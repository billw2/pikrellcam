/* PiKrellCam
|
|  Copyright (C) 2015-2017 Bill Wilson    billw@gkrellm.net
|
|  PiKrellCam is free software: you can redistribute it and/or modify it
|  under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  PiKrellCam is distributed in the hope that it will be useful, but WITHOUT
|  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
|  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
|  License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with this program. If not, see http://www.gnu.org/licenses/
|
|  This file is part of PiKrellCam.
*/

#include "pikrellcam.h"


AudioCircularBuffer	audio_circular_buffer;

static pthread_t	audio_thread_ref;
static int			thread_ret = 1;
static float		gain;

#define FORMAT_CODE_PCM			1
#define N_PERIODS				8


typedef struct
	{
	char     RIFF[4];
	uint32_t file_size;
	char     type[4];
	char     format_chunk_id[4];
	uint32_t format_chunk_size;
	uint16_t format_code;
	uint16_t n_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t bytes_per_sample;
	uint16_t bits_per_sample;
	char	 data[4];
	uint32_t data_size;
	} WaveHeader;

static WaveHeader	wave_header;

static FILE	*debug_wave_file;
static char	*debug_wave_path = "/tmp/debug.wav";
static int	max_mp3_encode_periods;

  /* Video buffer is initialized first, so vcb->seconds is right.
  |  Usually called after video_circular_buffer_init() while vcb is locked,
  |  but also called from audio_mic_open() before audio_thread is started.
  */
void
audio_circular_buffer_init(void)
	{
	VideoCircularBuffer *vcb = &video_circular_buffer;
	AudioCircularBuffer *acb = &audio_circular_buffer;
	int					size;

	if (!acb->pcm)
		return;
	acb->head = acb->tail = 0;
	acb->channels = pikrellcam.audio_channels;
	acb->n_frames = acb->rate * vcb->seconds;
	size = SND_FRAMES_TO_BYTES(acb, acb->n_frames);

	if (acb->data_size != size)
		{
		if (acb->data)
			free(acb->data);
		acb->data = (int16_t *) malloc(size);
		if (acb->data)
			{
			acb->data_size = size;
//			memset(acb->data, 0, acb->data_size);
			log_printf("audio circular buffer - %.2f KB  %.2f KFrames (%d sec, %d samples/sec, %d channels)\n",
				(float) size / 1000.0, (float) acb->n_frames / 1000.0,
				vcb->seconds, acb->rate, pikrellcam.audio_channels);
			}
		else
			{
			acb->data_size = 0;
			acb->n_frames = 0;
			log_printf("Audio circular buffer malloc() failed: %m\n");
			}
		}
	}

static void
audio_record_write(AudioCircularBuffer *acb, void *buf, int n_frames)
	{
	int		n;

	if (!acb->mp3_file)
		return;
	if (acb->channels == 1)
		n = lame_encode_buffer(acb->lame_record, buf, NULL, n_frames,
				acb->mp3_record_buffer, acb->mp3_record_buffer_size);
	else
		n = lame_encode_buffer_interleaved(acb->lame_record, buf, n_frames,
				acb->mp3_record_buffer, acb->mp3_record_buffer_size);
	if (n > 0)
		fwrite(acb->mp3_record_buffer, n, 1, acb->mp3_file);

	if ((pikrellcam.audio_debug & 0x1) && debug_wave_file)
		fwrite(buf, SND_FRAMES_TO_BYTES(acb, n_frames), 1, debug_wave_file);
	}

  /* When set head/tail functions are called from h264 encoder callback,
  |  vcb mutex is locked.
  */
void
audio_buffer_set_record_head(AudioCircularBuffer *acb, int head)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;

	if (!acb->pcm || !acb->lame_record)
		return;
	acb->record_head = head;	/* don't need mutex */

	if (pikrellcam.audio_debug & 0x4)
		{
		printf("%.2f audio_set_record_head:%d (tail:%d frames:%d) %.2f video (frames:%d)\n",
				(float) acb->frame_count / (float) acb->rate,
				head, acb->tail, acb->frame_count,
				(float) (vcb->last_pts - pikrellcam.video_start_pts) / 1e6,
				vcb->frame_count);
		}
	}

void
audio_buffer_set_tail(AudioCircularBuffer *acb, int position)
	{
	pthread_mutex_lock(&acb->mutex);
	acb->tail = position;
	if (pikrellcam.audio_debug & 0x4)
		printf("set_tail tail:%d  head: %d\n", acb->tail, acb->head);
	pthread_mutex_unlock(&acb->mutex);
	}

  /* If tail < 0, tail is delta to set behind head. Otherwise it is absolute
  |  tail.
  */
void
audio_buffer_set_record_head_tail(AudioCircularBuffer *acb, int head, int tail)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;

	pthread_mutex_lock(&acb->mutex);
	acb->record_head = head;
	if (tail < 0)
		{
		acb->tail = head + tail;
		if (acb->tail < 0)
			acb->tail += acb->n_frames;
		}
	else
		acb->tail = tail;

	if (pikrellcam.audio_debug & 0x4)
		{
		printf("%.2f audio_set_record_head:%d_tail:%d (tail:%d frames:%d) %.2f video (frames:%d)\n",
				(float) acb->frame_count / (float) acb->rate,
				head, tail, acb->tail, acb->frame_count,
				(float) (vcb->last_pts - pikrellcam.video_start_pts) / 1e6,
				vcb->frame_count);
		}
	pthread_mutex_unlock(&acb->mutex);
	}

int
audio_frames_offset_from_video(AudioCircularBuffer *acb)
	{
	VideoCircularBuffer	*vcb = &video_circular_buffer;
	double		vid_time;
	int			pending, offset;

	if (vcb->video_frame_count < 2)
		return 0;

	vid_time = (double)(vcb->last_pts - pikrellcam.video_start_pts) / 1e6;
	vid_time /= vcb->video_frame_count - 1;
	vid_time *= vcb->video_frame_count;

	pthread_mutex_lock(&acb->mutex);
	pending = acb->record_head - acb->tail;
	if (pending < 0)
		pending += acb->n_frames;

	offset = pending + acb->frame_count
					- (int) (vid_time * (double) acb->rate);

	if (pikrellcam.audio_debug & 0x4)
		{
		printf("  rhead:%d tail:%d pending:%d frames:%d vid_time_usec:%d\n",
				acb->record_head, acb->tail, pending, acb->n_frames,
				(int) (vid_time * 1e6));
		}
	pthread_mutex_unlock(&acb->mutex);
	return offset;
	}

static void
wave_header_init(WaveHeader *header, uint32_t sample_rate, uint16_t bit_depth,
				uint16_t n_channels)
	{
	if (!header)
		return;

	memcpy(&header->RIFF, "RIFF", 4);
	header->file_size = 0xefffffff;
	memcpy(&header->type, "WAVE", 4);
	memcpy(&header->format_chunk_id, "fmt ", 4);
	header->format_chunk_size = 16;		/* Size of following format data */
	header->format_code = FORMAT_CODE_PCM;
	header->n_channels = n_channels;
	header->sample_rate = sample_rate;	/* data blocks / sec */
	header->byte_rate = sample_rate * n_channels * bit_depth / 8;
	header->bytes_per_sample = n_channels * bit_depth / 8;
	header->bits_per_sample = bit_depth;

	memcpy(&header->data, "data", 4);
	header->data_size = header->file_size - sizeof(WaveHeader);
	}

static int
audio_gain(AudioCircularBuffer *acb, int16_t *pcm_buf, int frames)
	{
	int		i, peak, pcm, n_pcm;

	n_pcm = frames * acb->channels;
	for (i = 0, peak = 0; i < n_pcm; ++i)
		{
		pcm = (int) ((float) pcm_buf[i] * gain);
		if (pcm > INT16_MAX)
			pcm = INT16_MAX;
		else if (pcm < INT16_MIN)
			pcm = INT16_MIN;
		pcm_buf[i] = (int16_t) pcm;
		if (pcm < 0)
			pcm = -pcm;
		if (pcm > peak)
			peak = pcm;
		}
	return peak;
	}

static void
audio_record_files_close(void)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	int		n = 0;

	pthread_mutex_lock(&acb->mutex);
	if (acb->lame_record)
		{
		n = lame_encode_flush(acb->lame_record,
					acb->mp3_record_buffer, acb->mp3_record_buffer_size);
		lame_close(acb->lame_record);
		acb->lame_record = NULL;	
		}
	if (acb->mp3_file)
		{
		if (n > 0)
			fwrite(acb->mp3_record_buffer, n, 1, acb->mp3_file);
		fclose(acb->mp3_file);
		acb->mp3_file = NULL;
		}
	pthread_mutex_unlock(&acb->mutex);

	if (debug_wave_file)
		fclose(debug_wave_file);
	debug_wave_file = NULL;
	}


static void
audio_stream_close(int error)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	int		n;

	pthread_mutex_lock(&acb->mutex);
	if (acb->lame_stream)
		{
		n = lame_encode_flush(acb->lame_stream,
					acb->mp3_stream_buffer, acb->mp3_stream_buffer_size);
		lame_close(acb->lame_stream);
		acb->lame_stream = NULL;
		if (acb->mp3_stream_fd > 0)
			{
			if (n > 0 && !error)
				write(acb->mp3_stream_fd, acb->mp3_stream_buffer, n);
			close(acb->mp3_stream_fd);
			acb->mp3_stream_fd = -1;
			}
		}
	pthread_mutex_unlock(&acb->mutex);

	if (error && pikrellcam.verbose)
		log_printf("audio stream closed, error: %s\n", strerror(error));
	}

static void *
audio_thread(void *ptr)
	{
	AudioCircularBuffer		*acb = &audio_circular_buffer;
	static struct timeval	encode_timer;
	int						n, err, end_frames, t_usec, frames, rhead, peak,
							n_frames, avail_record_frames, use_record_head;
	int16_t					*buf = acb->buffer;

	while (1)
		{
		err = 0;
		if ((frames = snd_pcm_readi(acb->pcm, buf, acb->period_frames)) < 0)
			{
			usleep(5000);
			if (frames == -EAGAIN)
				continue;
			if ((err = snd_pcm_recover(acb->pcm, frames, 1)) == 0)
				err = snd_pcm_start(acb->pcm);
			}
		if (err < 0)
			{
			log_printf("Audio recover failed: %s\n", snd_strerror(err));
			pthread_mutex_lock(&acb->mutex);
			snd_pcm_close(acb->pcm);
			acb->pcm = NULL;
			pthread_mutex_unlock(&acb->mutex);
			audio_record_files_close();
			audio_stream_close(0);
			if (pikrellcam.audio_pathname)
				{
				unlink(pikrellcam.audio_pathname);
				free(pikrellcam.audio_pathname);
				}
			pikrellcam.audio_pathname = NULL;
			break;
			}

		if (frames <= 0)
			continue;

		if ((peak = audio_gain(acb, buf, frames)) > acb->vu_meter)
			acb->vu_meter = peak;

		end_frames = acb->n_frames - acb->head;
		if (frames <= end_frames)
			memcpy(acb->data + acb->head, buf,
						SND_FRAMES_TO_BYTES(acb, frames));
		else
			{
			memcpy(acb->data + acb->head, buf,
						SND_FRAMES_TO_BYTES(acb, end_frames));
			memcpy(acb->data, buf + end_frames,
						SND_FRAMES_TO_BYTES(acb, frames - end_frames));
			}

		pthread_mutex_lock(&acb->mutex);
		rhead = acb->record_head;
		acb->head = (acb->head + frames) % acb->n_frames;

		err = 0;
		if (acb->mp3_stream_fd > 0 && acb->lame_stream)
			{
			if (pikrellcam.audio_debug & 0x2)
				micro_elapsed_time(&encode_timer);

			if (acb->channels == 1)
				n = lame_encode_buffer(acb->lame_stream, buf, NULL,
						frames, acb->mp3_stream_buffer, acb->mp3_stream_buffer_size);
			else
				n = lame_encode_buffer_interleaved(acb->lame_stream, buf,
						frames, acb->mp3_stream_buffer, acb->mp3_stream_buffer_size);

			if (pikrellcam.audio_debug & 0x2)
				{
				t_usec = micro_elapsed_time(&encode_timer);
				printf("  audio thread stream frames: %d  encode_usec: %d\n",
						frames, t_usec);
				}

			if (n > 0)
				err = write(acb->mp3_stream_fd, acb->mp3_stream_buffer, n);
			}

		/* Need to write from tail to record_head, but first write of
		|  pre-capture time or motion re-triggers after post-capture in
		|  event-gap can be for many seconds -> long  MP3 convert time. So
		|  convert only chunks of max_record_frames and catch up in event-gap.
		*/
		if (acb->lame_record && acb->mp3_file && acb->tail != rhead)
			{
			avail_record_frames = rhead - acb->tail;
			if (avail_record_frames < 0)
				avail_record_frames += acb->n_frames;
			if (avail_record_frames > acb->max_record_frames)
				use_record_head =
						(acb->tail + acb->max_record_frames) % acb->n_frames;
			else
				use_record_head = rhead;

			if (pikrellcam.audio_debug & 0x4)
				micro_elapsed_time(&encode_timer);

			if (acb->tail < use_record_head)
				{
				n_frames = use_record_head - acb->tail;
				audio_record_write(acb, acb->data + acb->tail, n_frames);
				acb->frame_count += n_frames;
				}
			else
				{
				n_frames = acb->n_frames - acb->tail;
				audio_record_write(acb, acb->data + acb->tail, n_frames);
				if (use_record_head > 0)
					audio_record_write(acb, acb->data, use_record_head);
				acb->frame_count += n_frames + use_record_head;
				}
			acb->tail = use_record_head;
			if (pikrellcam.audio_debug & 0x4)
				{
				t_usec = micro_elapsed_time(&encode_timer);
				printf("  audio thread record frames: %d  encode_usec: %d\n",
						acb->frame_count, t_usec);
				}
			}
		pthread_mutex_unlock(&acb->mutex);

		if (err < 0 && errno == EPIPE)
			audio_stream_close(EPIPE);

		if (acb->force_thread_exit)
			{
			acb->force_thread_exit = FALSE;
			break;
			}
		}
	return NULL;
	}

boolean
audio_record_start(void)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	int		quality;

	if (!acb->pcm)
		return FALSE;

	if (debug_wave_file)
		fclose(debug_wave_file);
	debug_wave_file = NULL;

	acb->mp3_file = fopen(pikrellcam.audio_pathname, "w");
	if (!acb->mp3_file)
		{
		log_printf("Failed to create mp3 file: %s\n", pikrellcam.audio_pathname);
		return FALSE;
		}

	acb->frame_count = 0;
	acb->lame_record = lame_init();
	lame_set_in_samplerate(acb->lame_record, acb->rate);
	lame_set_num_channels(acb->lame_record, acb->channels);
	quality = (pikrellcam.pi_model == 2)
		? pikrellcam.audio_mp3_quality_Pi2 : pikrellcam.audio_mp3_quality_Pi1;
	lame_set_quality(acb->lame_record, quality);
	lame_set_VBR_q(acb->lame_record, quality); 
	lame_init_params(acb->lame_record);

	if (pikrellcam.audio_debug & 0x1)
		{
		debug_wave_file = fopen(debug_wave_path, "w");
		if (debug_wave_file)
			fwrite(&wave_header, sizeof(WaveHeader), 1, debug_wave_file);
		}
	return TRUE;
	}

void
audio_record_stop(void)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	int					t_usec, offset;

	if (!acb->pcm)
		{
		pikrellcam.audio_last_frame_count = 0;
		pikrellcam.audio_last_rate = 0;
		return;
		}
	offset = audio_frames_offset_from_video(acb);
	if (offset < 0)
		{
		/* Wait enough time for head to move so have space to move record_head.
		*/
		t_usec = (2 - offset / (int) acb->period_frames) * acb->period_usec;
		usleep(t_usec);

		pthread_mutex_lock(&acb->mutex);
		acb->record_head -= offset;
		if (acb->record_head > acb->n_frames)
			acb->record_head -= acb->n_frames;
		pthread_mutex_unlock(&acb->mutex);

		/* And wait more so audio thread can write from tail to record_head.
		*/
		usleep(t_usec);
		}
	acb->tail = acb->record_head;

	if (pikrellcam.audio_debug & 0x2)
		printf("audio_record_stop: audio_frames:%d  offset:%d\n",
				acb->frame_count, offset);

	audio_record_files_close();
	pikrellcam.audio_last_frame_count = acb->frame_count;
	pikrellcam.audio_last_rate = (int) ((pikrellcam.audio_last_frame_count
						/ pikrellcam.video_last_time + 0.5));
	}

static int
audio_error(char *msg, int err)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	char	buf[128], *err_msg;

	if (acb->pcm)
		snd_pcm_close(acb->pcm);
	acb->pcm = NULL;

	err_msg = (char *) snd_strerror(err);

	display_inform("\"Audio error - cannot open microphone.\" 3 3 1");
	snprintf(buf, sizeof(buf), "\"%s failed: %s\" 4 3 1", msg, err_msg);
	display_inform(buf);
	display_inform("timeout 2");

	log_printf("Audio error - %s failed: %s\n", msg, snd_strerror(err));
	return FALSE;
	}

void
audio_retry_open(void)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	static int			tries;

	log_printf("audio open retry: %d of 8\n", ++tries);
	if (acb->pcm || audio_mic_open(TRUE) || tries > 8)
		{
		tries = 0;
		event_remove_name("audio retry open");
		}
	}

boolean
audio_mic_open(boolean inform)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	int					t, err, dir, size;
	snd_pcm_hw_params_t	*params;

	if (acb->pcm)
		{
		display_inform("\"Microphone is already open.\" 3 3 1");
		display_inform("timeout 2");
		return TRUE;
		}

	acb->format = SND_PCM_FORMAT_S16_LE;
	max_mp3_encode_periods = (pikrellcam.pi_model == 1) ? 2 : 3;

	if ((err = snd_pcm_open(&acb->pcm, pikrellcam.audio_device,
					SND_PCM_STREAM_CAPTURE, 0)) < 0)
		return audio_error("pcm open", err);

	if ((err = snd_pcm_hw_params_malloc(&params)) < 0)
		return audio_error("params malloc", err);

	if ((err = snd_pcm_hw_params_any(acb->pcm, params)) < 0)
		return audio_error("params any", err);

	if ((err = snd_pcm_hw_params_set_access(acb->pcm, params,
					SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
		return audio_error("set access", err);

	if ((err = snd_pcm_hw_params_set_format(acb->pcm, params, acb->format)) < 0)
		return audio_error("set format", err);

	acb->rate = (pikrellcam.pi_model == 2) ?
						pikrellcam.audio_rate_Pi2 : pikrellcam.audio_rate_Pi1;
	if ((err = snd_pcm_hw_params_set_rate_near(acb->pcm, params, &acb->rate, 0)) < 0)
		return audio_error("set rate", err);
	if (pikrellcam.pi_model == 2)
		pikrellcam.audio_rate_Pi2 = acb->rate;
	else
		pikrellcam.audio_rate_Pi1 = acb->rate;

	if ((err = snd_pcm_hw_params_set_channels(acb->pcm, params,
				pikrellcam.audio_channels)) < 0)
		return audio_error("set channels", err);

	/* If audio period time < 1/video_fps, audio latency can be < 1 vid frame.
	*/
	t = 1000000 / pikrellcam.camera_adjust.video_fps / 2;
	t %= 1000;
	if (t < 25000)
		t = 25000;
	else if (t > 62000)
		t = 62000;
	if ((err = snd_pcm_hw_params_set_period_time(acb->pcm, params, t, 0)) < 0)
		return audio_error("set period time", err);

	if (snd_pcm_hw_params_set_periods(acb->pcm, params, N_PERIODS, 0) < 0)
		return audio_error("set periods", err);

	if ((err = snd_pcm_hw_params(acb->pcm, params)) < 0)
		return audio_error("hw params", err);


	if ((err = snd_pcm_hw_params_get_channels(params, &acb->channels)) < 0)
		return audio_error("get channels", err);
	pikrellcam.audio_channels = acb->channels;

	if ((err = snd_pcm_hw_params_get_period_size(params, &acb->period_frames, &dir)) < 0)
		return audio_error("get period size", err);

	if ((err = snd_pcm_hw_params_get_period_time(params, &acb->period_usec, &dir)) < 0)
		return audio_error("get period time", err);

	if ((err = snd_pcm_hw_params_get_buffer_size(params, &acb->buffer_frames)) < 0)
		return audio_error("get buffer size", err);

	snd_pcm_hw_params_free(params);

	audio_circular_buffer_init();

	acb->mp3_file = NULL;
	acb->mp3_stream_fd = -1;

	size = SND_FRAMES_TO_BYTES(acb, acb->period_frames);
	if (size != acb->buffer_size)
		{
		if (acb->buffer)
			free(acb->buffer);
		acb->buffer = (int16_t *) malloc(size);
		acb->buffer_size = acb->buffer ? size : 0;
		if (!acb->buffer)
			log_printf("malloc() of period frame buffer failed: %m\n");

		if (acb->mp3_record_buffer)
			free(acb->mp3_record_buffer);
		acb->max_record_frames = max_mp3_encode_periods * (int) acb->period_frames;
		acb->mp3_record_buffer_size = acb->max_record_frames * 5 / 4 + 8000;
		acb->mp3_record_buffer = (uint8_t *) malloc(acb->mp3_record_buffer_size);
		if (!acb->mp3_record_buffer)
			log_printf("malloc() of mp3 record buffer failed: %m\n");

		if (acb->mp3_stream_buffer)
			free(acb->mp3_stream_buffer);
		acb->mp3_stream_buffer_size = (int) acb->period_frames * 5 / 4 + 8000;
		acb->mp3_stream_buffer = (uint8_t *) malloc(acb->mp3_stream_buffer_size);
		if (!acb->mp3_stream_buffer)
			log_printf("malloc() of mp3 stream buffer failed: %m\n");
		}

	wave_header_init(&wave_header, acb->rate, 16, acb->channels);
	gain = powf(10.0, pikrellcam.audio_gain_dB / 20.0);

	thread_ret = pthread_create(&audio_thread_ref, NULL, audio_thread, NULL);

	if (pikrellcam.audio_debug & 0x2)
		printf("period_frames=%d period_buffer_size=%d rate: %d channels: %d\n",
			(int) acb->period_frames, (int) acb->buffer_size,
			acb->rate, acb->channels);

	log_printf("mic opened: period_frames:%d period_buffer_size:%d rate:%d channels:%d  MP3 quality:%d\n",
			(int) acb->period_frames, (int) acb->buffer_size,
			acb->rate, acb->channels,
			(pikrellcam.pi_model == 2)
				? pikrellcam.audio_mp3_quality_Pi2 : pikrellcam.audio_mp3_quality_Pi1);
	if (inform)
		{
		display_inform("\"Microphone opened.\" 3 3 1");
		display_inform("timeout 2");
		}
	return TRUE;
	}

static boolean
audio_fifo_write_open(void *data)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	boolean		abort_if_fail = (boolean) data;

	acb->mp3_stream_fd = open(pikrellcam.audio_fifo, O_WRONLY|O_NONBLOCK);
	if (acb->mp3_stream_fd >= 0)
		{
		display_inform("\"Audio stream opened.\" 3 3 1");
		display_inform("timeout 2");
		return TRUE;
		}
	else if (abort_if_fail)
		{
		if (acb->lame_stream)
			lame_close(acb->lame_stream);
		acb->lame_stream = NULL;
		display_inform("timeout 0");
		display_inform("\"Failed to open audio FIFO (no reader?).\" 3 3 1");
		display_inform("timeout 2");
		log_printf("Failed to open audio FIFO (no reader?): %s.  %m\n", pikrellcam.audio_fifo);
		}
	else
		{
		display_inform("\"Trying to open audio FIFO.\" 2 3 1");
		display_inform("timeout 1");
		}
	return FALSE;
	}

static boolean
audio_mic_close(void)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	VideoCircularBuffer *vcb = &video_circular_buffer;
	int		i;

	if (!acb->pcm)
		{
		display_inform("\"Microphone is already closed.\" 3 3 1");
		display_inform("timeout 2");
		return FALSE;
		}
	if (vcb->state != VCB_STATE_NONE || acb->lame_record)
		{
		display_inform("\"Cannot close microphone\" 3 3 1");
		display_inform("\"while audio/video is recording.\" 4 3 1");
		display_inform("timeout 2");
		return FALSE;
		}
	if (thread_ret == 0)
		{
		thread_ret = 1;
		acb->force_thread_exit = TRUE;
		i = 20;
		while (acb->force_thread_exit && --i > 0)
			usleep(50000);
		}
	audio_record_files_close();
	audio_stream_close(0);

	if (acb->pcm)
		snd_pcm_close(acb->pcm);
	acb->pcm = NULL;

	display_inform("\"Microphone closed.\" 3 3 1");
	display_inform("timeout 2");

	return TRUE;
	}

#define	AUDIO_MIC_OPEN		0
#define	AUDIO_MIC_CLOSE		1
#define	AUDIO_MIC_TOGGLE	2
#define	AUDIO_GAIN			3
#define	AUDIO_STREAM_OPEN	4
#define	AUDIO_STREAM_CLOSE	5

typedef struct
    {
    char *name;
    int  id,
         n_args;
    }
AudioCommand;

static AudioCommand audio_commands[] =
    {
        { "mic_open",	AUDIO_MIC_OPEN,  0 },
        { "mic_close",	AUDIO_MIC_CLOSE, 0 },
        { "mic_toggle",	AUDIO_MIC_TOGGLE, 0 },
        { "gain",		AUDIO_GAIN, 1 },
        { "stream_open",  AUDIO_STREAM_OPEN,  0 },
        { "stream_close", AUDIO_STREAM_CLOSE, 0 },
    };

#define N_AUDIO_COMMANDS    (sizeof(audio_commands) / sizeof(AudioCommand))

void
audio_command(char *cmd_line)
	{
	AudioCircularBuffer	*acb = &audio_circular_buffer;
	AudioCommand	*acmd;
	char			buf[64], arg1[32];
	int				i, n, quality, id = -1;

    arg1[0] = '\0';
    n = sscanf(cmd_line, "%63s %31s", buf, arg1);
    if (n < 1)
        return;

    for (i = 0; i < N_AUDIO_COMMANDS; ++i)
        {
        acmd = &audio_commands[i];
        if (!strcmp(acmd->name, buf))
            {
            if (acmd->n_args <= n - 1)
                id = acmd->id;
            break;
            }
        }
    if (id == -1)
        {
//      inform_message("Bad audio command.");
        return;
        }

	switch (id)
		{
		case AUDIO_MIC_TOGGLE:
			n = pikrellcam.audio_enable;
			if (acb->pcm)
				{
				if (audio_mic_close())
					pikrellcam.audio_enable = FALSE;
				}
			else
				pikrellcam.audio_enable = audio_mic_open(TRUE);

			if (n != pikrellcam.audio_enable)
				pikrellcam.config_modified = TRUE;
			break;

		case AUDIO_MIC_OPEN:
			n = pikrellcam.audio_enable;
			pikrellcam.audio_enable = audio_mic_open(TRUE);
			if (n != pikrellcam.audio_enable)
				pikrellcam.config_modified = TRUE;
			break;

		case AUDIO_MIC_CLOSE:
			if (audio_mic_close())
				{
				if (pikrellcam.audio_enable)
					pikrellcam.config_modified = TRUE;
				pikrellcam.audio_enable = FALSE;
				}
			break;

		case AUDIO_GAIN:
			n = pikrellcam.audio_gain_dB;
			if (!strcmp(arg1, "up"))
				n += 2;
			else if (!strcmp(arg1, "down"))
				n -= 2;
			else if (isdigit(*arg1))
				n = atoi(arg1);
			n &= ~1;
			if (n < 0)
				n = 0;
			if (n > 30)
				n = 30;
			pikrellcam.audio_gain_dB = n;
			gain = powf(10.0, pikrellcam.audio_gain_dB / 20.0);
			pikrellcam.config_modified = TRUE;
			break;

		case AUDIO_STREAM_OPEN:
			if (!acb->pcm)
				{
				display_inform("\"Cannot audio stream, microphone is closed.\" 3 3 1");
				display_inform("timeout 2");
				break;
				}
			if (acb->mp3_stream_fd > 0)
				{
				display_inform("\"Audio stream already open.\" 3 3 1");
				display_inform("timeout 2");
				break;
				}
			if (acb->lame_stream)
				lame_close(acb->lame_stream);
			acb->lame_stream = lame_init();
			lame_set_in_samplerate(acb->lame_stream, acb->rate);
			lame_set_num_channels(acb->lame_stream, acb->channels);
			quality = (pikrellcam.pi_model == 2)
				? pikrellcam.audio_mp3_quality_Pi2 : pikrellcam.audio_mp3_quality_Pi1;
			lame_set_quality(acb->lame_stream, quality);
			lame_set_VBR_q(acb->lame_stream, quality);
			lame_init_params(acb->lame_stream);

			/* Reader must open before write NONBLOCK open can succeed, so
			|  wait first and then try once more before giving up.
			*/
			usleep(20000);
			if (!audio_fifo_write_open((void *) FALSE))
				event_count_down_add("audio stream open",
						12 * EVENT_LOOP_FREQUENCY,
						(void *) audio_fifo_write_open, (void *) TRUE);
			break;

		case AUDIO_STREAM_CLOSE:
			if (acb->mp3_stream_fd >= 0)
				{
				audio_stream_close(0);
				display_inform("\"Audio stream closed.\" 3 3 1");
				display_inform("timeout 2");
				}
			else
				{
				display_inform("\"Cannot close, audio stream was not open.\" 3 3 1");
				display_inform("timeout 2");
				}
			break;
		}
	}
