/* PiKrellCam
|
|  Copyright (C) 2015-2020 Bill Wilson    billw@gkrellm.net
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

#ifndef _PIKRELLCAM_H
#define _PIKRELLCAM_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include <alsa/asoundlib.h>
#include <lame/lame.h>

#include "utils.h"

#define	PIKRELLCAM_VERSION	"4.3.2"


//TCP Stream Server
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  

#define H264_TCP_WAIT_FOR_CONNECT 0
#define H264_TCP_SEND_HEADER      1
#define H264_TCP_SEND_DATA        2

extern int listenfd, connfd, num_sent;
extern socklen_t clilen;
extern struct sockaddr_in cliaddr, servaddr;
extern int h264_conn_status;


#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

/* Don't change, usleep code in main() assumes 10
*/
#define	EVENT_LOOP_FREQUENCY	10

#define PIKRELLCAM_CONFIG_DIR					".pikrellcam"
#define PIKRELLCAM_CONFIG						"pikrellcam.conf"
#define PIKRELLCAM_MOTION_REGIONS_CONFIG		"motion-regions.conf"
#define PIKRELLCAM_MOTION_REGIONS_CUSTOM_CONFIG	"motion-regions-%s.conf"
#define PIKRELLCAM_AT_COMMANDS_CONFIG			"at-commands.conf"
#define PIKRELLCAM_TIMELAPSE_STATUS				"timelapse.status"

  /* These subdirs must match what is in www/config.php
  */
#define PIKRELLCAM_VIDEO_SUBDIR					"videos"
#define PIKRELLCAM_THUMBS_SUBDIR				"thumbs"
#define PIKRELLCAM_STILL_SUBDIR					"stills"
#define PIKRELLCAM_TIMELAPSE_SUBDIR				"timelapse"

#define SERVO_MIN_WIDTH	50
#define SERVO_MAX_WIDTH	250


  /* ------------------ MMAL Camera ---------------
  */
#define CAMERA_PREVIEW_PORT	0
#define CAMERA_VIDEO_PORT	1
#define CAMERA_CAPTURE_PORT	2
#define	MAX_CAMERA_PORTS	3

/* Still and video need at least 2 buffers.
*/
#define BUFFER_NUMBER_MIN		3


/* A camera object is a RPi camera, encoder, resizer or splitter component
|  and its associated data we need to manage.
*/ 
typedef struct
	{
	char				*name;
	FILE				*file;

	MMAL_COMPONENT_T	*component;
	MMAL_PORT_T			*control_port;
	MMAL_CONNECTION_T	*input_connection;

	/* Record this objects output port if we are controlling its enable
	|  and its buffer pool if we created its pool.
	|  Maybe FIXME - what about a splitter with more than one output going
	|      to a callback?  This struct can only handle A splitter with one
	|      callback and the rest tunnel outs, or all tunnel outs.
	*/
	MMAL_PORT_T			*port_out;
	MMAL_POOL_T			*pool_out;

	/* If this camera object sends output data to a callback that will process
	|  and then send the data back into a destination camera object input, we
	|  record the destination port and pool in the sender object.  Then the
	|  sender object passes itself as a user data pointer to the callback so
	|  the callback can know who to send to.
	*/
	MMAL_PORT_T			*callback_port_in;
	MMAL_POOL_T			*callback_pool_in;
	}
	CameraObject;



  /* ------------------ Event/Command Scheduling ---------------
  */
typedef struct
	{
	char	*name;

	time_t	time,		/* For events at a time with optional repeat period */
			period;

	int		count;		/* For one shot events after count expires. */

	void	(*func)();
	void	*data;
	}
	Event;

typedef struct
	{
	char	*frequency,
			*at_time;
	char	*command;
	}
	AtCommand;


  /* ------------------ Annotate Strings ---------------
  */
typedef struct
	{
	char    *id,
	        *string;
	boolean prepend;
	}
	AnnotateString;

  /* ------------------ Motion Detection ---------------
  */

  /*  Scale a motion vector coordinate to a mjpeg frame coordinate.
  |  Motion vector coordinates are video frame coords / 16.
  */
#define	MOTION_VECTOR_TO_MJPEG_X(mvx) \
			((mvx) * 16 * pikrellcam.mjpeg_width / pikrellcam.camera_config.video_width)
#define	MOTION_VECTOR_TO_MJPEG_Y(mvy) \
			((mvy) * 16 * pikrellcam.mjpeg_height / pikrellcam.camera_config.video_height)

  /* Scale a mjpeg coordinate to a motion vector frame coordinate.
  */
#define	MJPEG_TO_MOTION_VECTOR_X(mjx) \
			((mjx) * pikrellcam.camera_config.video_width / 16 / pikrellcam.mjpeg_width)
#define	MJPEG_TO_MOTION_VECTOR_Y(mjy) \
			((mjy) * pikrellcam.camera_config.video_height / 16 / pikrellcam.mjpeg_height)


typedef struct
	{
	int	x0, y0,
		x1, y1;
	}
	Area;


typedef struct
	{
    int8_t	vx,
			vy;
	int16_t	sad;
	}
	MotionVector;

typedef struct
	{
	int		x,
			y;
	int		vx,
			vy;

	int		mag2,		/* Magnitude^2 of this composite vector */
			mag2_count;	/* Number of MotionVectors added into this composite. */

	int		box_w,			/* A box around the composite center for testing */
			box_h,			/*  the vector concentration composing the cvec  */
			in_box_count,	/* Count of motion vectors inside of box */
			in_box_rejects;
	boolean	vertical;
	}
	CompositeVector;

  /* A composite vector for a region is composed of a > limit count motion
  |  vectors that have a > than mag2_limit magnitude.  The component vectors
  |  may have other contstraints, eg individual vectors must all more or less
  |  point in the same direction.  See motion.c
  */
typedef struct
	{
	int		region_number;
	float	xf0, yf0,		/* 0 - 1.0 fraction of the screen */
			dxf, dyf;
	int		x, y,			/* Computed from the fractions. */
			dx, dy;

	CompositeVector  vector;

	int		reject_count,	/* vectors not pointing in composite direction.  */
			sparkle_count;	/* number of isolated vectors */
	boolean	motion;
	}
	MotionRegion;

/* Motion types for writing the /run/pikrellcam/motion-events file
*/
#define	MOTION_EVENTS_HEADER	1
#define MOTION_EVENTS_DETECT	2
#define MOTION_EVENTS_START		(MOTION_EVENTS_HEADER | MOTION_EVENTS_DETECT)
#define MOTION_EVENTS_STILL		4
#define	MOTION_EVENTS_END		8

/* Motion detect state/types for the MotionFrame
*/
#define	MOTION_NONE          0
#define	MOTION_DETECTED      1
#define	MOTION_DIRECTION     2
#define	MOTION_BURST         4
#define	MOTION_FIFO      8
#define	MOTION_PENDING_DIR   0x10
#define	MOTION_PENDING_BURST 0x20
#define	MOTION_AUDIO         0x40

/* Additional motion types for motion in regions.
*/
#define MOTION_TYPE_DIR_SMALL      1
#define MOTION_TYPE_DIR_NORMAL     2
#define MOTION_TYPE_BURST_DENSITY  4

#define FIFO_TRIG_MODE_DEFAULT   0
#define FIFO_TRIG_MODE_ENABLE    1
#define FIFO_TRIG_MODE_TIMES     2

typedef struct
	{
	int				motion_status;
	boolean			motion_enable,
					show_vectors,
					show_preset;

	int				width,			/* width in macroblocks */
					height;			/* height in macroblocks */
	int				vectors_size;
	MotionVector	*vectors;
	CompositeVector	frame_vector,
					preview_frame_vector,
					final_preview_vector;
	int				cvec_count;
	int16_t			*trigger;
	int				n_regions,
					selected_region,
					prev_selected_region;

	pthread_mutex_t	region_list_mutex;
	SList			*motion_region_list;
	int				mag2_limit;      /* magnitude^2 limit for a vector to add to region_vector */
	int				mag2_limit_count; /* Threshold number of vectors added into */
			                          /* region_vector to make it a valid motion event */
	int		reject_count,	/* # of frame vectors not pointing same as composite vector */
			sparkle_count,	/* nuber of isolated vectors */
			any_count,		/* mag2 count total of the frame (not frame vector) */
			vertical_count,
			first_detect,
			direction_detects,
			burst_detects,
			max_burst_count,
			audio_detects,
			fifo_detects;

	boolean	fifo_trigger;
	int		fifo_trigger_mode,
			fifo_trigger_pre_capture,
			fifo_trigger_time_limit;
	char	*fifo_trigger_code;

	Area	motion_area,	/* Geometric area convering passing vectors */
			preview_motion_area;	/* Copy to preserve values for preview */

	float	sparkle_expma,
			any_count_expma;	/* of the total frame vector */

	int		frame_window;
	}
	MotionFrame;

#define MF_TRIGGER_SIZE	(motion_frame.width * motion_frame.height \
							* sizeof(*motion_frame.trigger))

#define	VCB_STATE_NONE					0
#define	VCB_STATE_MOTION_RECORD_START	0x1
#define	VCB_STATE_MANUAL_RECORD_START	0x2
#define	VCB_STATE_LOOP_RECORD_START		0x4
#define	VCB_STATE_MOTION_RECORD			0x10
#define	VCB_STATE_LOOP_RECORD			0x20
#define	VCB_STATE_MANUAL_RECORD			0x40
#define VCB_STATE_RESTARTING			0x100

#define	VCB_STATE_MOTION  (VCB_STATE_MOTION_RECORD_START | VCB_STATE_MOTION_RECORD)
#define	VCB_STATE_MANUAL  (VCB_STATE_MANUAL_RECORD_START | VCB_STATE_MANUAL_RECORD)

typedef struct
	{
	int			position;
	int			audio_position;

	time_t		t_frame;
	int			frame_count;
	uint64_t	frame_pts;
	}
	KeyFrame;


#define	H264_MAX_HEADER_SIZE	29	/* Can be less */

  /* While waiting for a video record to start, the h264 callback requests
  |  keyframes once per second so we can have second resolution on pre_capture
  |  time.  So the KEYFRAME_SIZE needs to be at least pre_capture time in
  |  seconds.  For now, make it bigger than anybody would ever want to
  |  pre_capture...  But FIXME, make the size dynamic based on pre_capture.
  */
#define KEYFRAME_SIZE	(15 * 60)

typedef struct
	{
	pthread_mutex_t	mutex;

	time_t		t_cur;
	FILE		*file,
				*motion_stats_file;
	boolean		file_writing,
				record_hold,
				motion_stats_do_header;
	int			state,
				frame_count,
				video_frame_count;

	int8_t	   h264_header[H264_MAX_HEADER_SIZE];
	int		   h264_header_position;

	int8_t	   *data; 		/* h.264 video data array      */
	int			size;		/* size in bytes of data array */
	int         seconds;	/* max seconds in the buffer   */
	int			head,
				tail;

	KeyFrame	key_frame[KEYFRAME_SIZE];
	int			pre_frame_index,
				cur_frame_index;
	boolean		in_keyframe,
				pause;
	int         manual_pre_capture,
				max_record_time;

	time_t		record_start_time,
				loop_start_time,
				record_elapsed_time;	/* seconds since record start excluding pause time. */
	uint64_t	last_pts;

	int			record_start_frame_index;
	float		actual_fps;
	}
	VideoCircularBuffer;


/* Only do 16 bit sound samples
*/
#define SND_FRAMES_TO_BYTES(acb, n_frames) \
				((int)(n_frames) * sizeof(int16_t) * acb->channels)

typedef struct
	{
	pthread_mutex_t		mutex;
	boolean				force_thread_exit;

	snd_pcm_t			*pcm;
	lame_t				lame_record,
						lame_stream;
	FILE				*mp3_file;
	uint8_t				*mp3_record_buffer,
						*mp3_stream_buffer;
	int					mp3_record_buffer_size,
						mp3_stream_buffer_size;

	int					mp3_stream_fd;

	int16_t				*data;			/* circular buffer data */
	int					data_size;		/* and its size in bytes */

	int					head,			/* Frame ndices into circ buf data */
						record_head,
						record_tail,
						n_frames,		/* Size in frames of circ buf data */
						max_record_frames;

	uint32_t			rate,			/* ALSA parameters */
						channels;
	int					format,
						periods;
	unsigned int		period_usec;
	snd_pcm_uframes_t	period_frames,
						buffer_frames;

	int16_t				*buffer;		/* holds read of one period of frames */
	int					buffer_size;	/* which is copied to circular buffer */

	int					record_frame_count;

	int					vu_meter0,
						vu_meter1;
	}
	AudioCircularBuffer;


  /* -------------- The Global PiKrellCam Environment -----------
  */
typedef struct
	{
	double	x,
			y,
			width,
			height;
	}
	RoiRect;

typedef struct
	{
	int		event_gap,
			pre_capture,
			post_capture,
			confirm_gap,
			confirm_delay;
	boolean	modified;
	}
	MotionTimes;

extern MotionTimes	motion_times_temp;


typedef struct
	{
	int		video_width,
			video_height;
	int		still_width,
			still_height;
	boolean	modified;
	}
	CameraConfig;

typedef struct
	{
	int		video_fps,
			video_bitrate,
			still_quality,
			video_mp4box_fps_display;
	float	video_mp4box_fps;
	boolean	modified;
	}
	CameraAdjust;

extern CameraAdjust	camera_adjust_temp;

typedef struct
	{
	int     pan,
	        tilt,
			settings_index,
			n_settings;
	SList   *settings_list;
	}
	PresetPosition;

typedef struct
	{
	int     mag_limit,
			mag_limit_count,
	        burst_count,
	        burst_frames,
			zoom_percent;
	SList	*region_list;  /* string "xf0 yf0 dxf dyf" */
	}
	PresetSettings;


typedef struct
	{

	time_t	t_now,
			t_start,
			motion_last_detect_time;

	struct timeval
			tv_now;

	struct tm tm_local;
	int		second_tick;
	int		pi_model;

	char	*install_dir,
			*nginx_group,
			*version,
			*tmpfs_dir,		/* for mjpeg and info files */
			*archive_dir,
			*media_dir,
			*video_dir,
			*thumb_dir,
			*still_dir,
			*timelapse_dir,
			*scripts_dir,
			*scripts_dist_dir,
			*command_fifo,
			*state_filename,
			*motion_events_filename,
			*motion_detects_fifo,
			*video_converting,
			*loop_converting;

	char	*config_dir,
			*config_file,
			*motion_regions_config_file,
			*at_commands_config_file,
			*on_startup_cmd;

	char	*log_file;
	int		log_lines;

	int		verbose,
			verbose_log,
			verbose_motion,
			verbose_multicast;
	char	hostname[HOST_NAME_MAX],
			*effective_user;

	int		diskfree_percent;
	boolean	check_archive_diskfree,
			check_media_diskfree;
	uint64_t tmp_space_avail;

	char	*latitude,
			*longitude;

	boolean	startup_motion_enable,
			motion_detects_fifo_enable,
			external_motion_record_event,
			external_motion_still,
			state_modified,
			preset_state_modified,
			config_modified,
			thumb_convert_done;

	int		config_sequence,
			config_sequence_new;

	boolean	config_media_sequence_modified;
	char	*media_sequence_file;

	MotionTimes
			motion_times;

	int		motion_vectors_dimming,
			motion_magnitude_limit,
			motion_magnitude_limit_count,
			motion_burst_count,
			motion_burst_frames,
			motion_record_time_limit,
			zoom_percent,
			zoom_percent_prev,
			zoom_motion_holdoff;

	time_t	motion_sync_time;

	char	*motion_stills_name_format,
			*motion_still_pathname;
	int		motion_stills_sequence,
			motion_stills_per_minute,
			motion_stills_count;
	boolean	motion_still_capture_event;
	time_t	motion_stills_capture_time;
	struct timeval
			motion_stills_start_tv;

	struct timeval
			motion_still_tv;

	boolean	motion_stills_enable,
			motion_stills_record;

	char	*on_motion_begin_cmd,
			*on_motion_end_cmd,
			*on_motion_enable_cmd,
			*on_manual_end_cmd,
			*on_loop_end_cmd,
			*motion_regions_name;
	char	*preview_pathname,
			*thumb_name;
	char	*on_motion_preview_save_cmd;

	boolean	motion_preview_clean,
			motion_vertical_filter,
			motion_stats,
			motion_show_counts;
	int		motion_area_min_side;

	boolean	preview_stall_warning;

	CameraConfig
			camera_config;
	CameraAdjust
			camera_adjust;

	int		camera_width_max,
			camera_height_max;

	char	*video_motion_name_format,
			*video_manual_name_format,
			*video_h264,
			*video_last,
			*video_pathname,
			*video_description,
			*video_manual_tag,
			*video_motion_tag;
	int		video_manual_sequence,
			video_motion_sequence,
			video_header_size,
			video_size;

	int		video_last_frame_count,
			audio_last_frame_count,
			audio_last_rate;
	double	video_last_time,
			video_last_fps;
	uint64_t video_start_pts,
			video_end_pts;

	char	*loop_dir,
			*loop_video_dir,
			*loop_thumb_dir,
			*loop_name_format;
	int		loop_record_time_limit,
			loop_diskusage_percent,
			loop_next_keyframe;
	boolean	loop_enable,
			loop_startup_enable;

	boolean	video_mp4box,
			do_preview_save,
			do_thumb_convert,
			do_motion_still;

	char	*mjpeg_filename;
	int		mjpeg_width,
			mjpeg_height,
			mjpeg_quality,
			mjpeg_divider,
			mjpeg_stall_count;

	char	*still_name_format,
			*still_last,
			*still_thumb_dir;
	int		still_sequence;
	boolean	still_capture_event;
	char	*on_still_capture_cmd;

	char	*video_timelapse_name_format,
			*timelapse_video_last,
			*timelapse_video_pending,
			*timelapse_format,
			*timelapse_jpeg_last,
			*timelapse_status_file;
	boolean	timelapse_capture_event;
	char	*timelapse_convert_cmd;

	int		servo_pan_gpio,
			servo_tilt_gpio,
			servo_pan_min,
			servo_pan_max,
			servo_tilt_min,
			servo_tilt_max,
			servo_preset_step_msec,
			servo_move_step_msec,
			servo_move_steps,
			servo_settle_msec;
	boolean	servo_pan_invert,
			servo_tilt_invert,
			servo_moving,
			servo_use_servoblaster,
			have_servos;

	boolean	audio_enable,
			audio_trigger_video,
			audio_box_MP3_only,
			audio_debug;
	char	*audio_device,
			*audio_pathname,
			*audio_fifo;
	int		audio_rate_Pi2,
			audio_rate_Pi1,
			audio_channels,
			audio_gain_dB,
			audio_mp3_quality_Pi2,
			audio_mp3_quality_Pi1,
			audio_level,
			audio_trigger_level,
			audio_level_event,
			audio_confirm_gap,
			audio_confirm_delay;

	SList	*preset_position_list;
	int		preset_position_index,
			n_preset_positions;
	char	*preset_config_file,
			*preset_state_file;
	PresetPosition
			*preset_last_on;
	boolean	preset_modified,
			preset_modified_warning,
			preset_notify,
			motion_off_preset,
			on_preset;			/* modify in servo_control.mutex */

	int		multicast_group_port;
	char	*multicast_group_IP,
			*multicast_from_hostname,
			*on_multicast_message_cmd;
	boolean	multicast_enable;

	char	*annotate_format_string,
			annotate_string_space_char;
	SList	*annotate_list;
	boolean	annotate_enable,
			annotate_show_motion,
			annotate_show_frame;
	char	*annotate_text_background_color;
	int		annotate_text_brightness,
			annotate_text_size;

	boolean	video_notify,
			still_notify,
			timelapse_notify;
	int		notify_duration;

	char	*sharpness,
			*contrast,
			*brightness,
			*saturation,
			*iso,
			*metering_mode,
			*video_stabilisation,
//			*draw_focus_box,
			*exposure_compensation,
			*exposure_mode,
			*white_balance,
			*image_effect,
			*color_effect,
			*rotation,
			*hflip,
			*vflip,
			*crop,
			*shutter_speed,
			*raw_capture;

	char	*lc_time;
	int		debug,
			debug_fps;
	boolean	halt_enable;
	}
	PiKrellCam;


  /* -------------------- Timelapse -----------------
  */
typedef struct
	{
	int		series,
			sequence,
			period;
	boolean	activated,
			on_hold,
			show_status;
	char	*convert_name;
	int		convert_size;

	Event	*event,
			*inform_event;
	}
	TimeLapse;


  /* ------------------ Configuration ---------------
  */
typedef struct
	{
	int		param;
	char	*name;
	}
	ParameterTable;

typedef struct
	{
	char			*name,
					*arg;
	MMAL_STATUS_T	(*func)();
	char			**camera_option;
	}
	CameraParameter;


typedef union
	{
	int		*value;
	float	*value_float;
	char	**string;
	}
	ConfigResult;

typedef struct
	{
	char	*description,
			*option,
			*arg;
	boolean	safe;
	ConfigResult result;
	boolean	(*config_func)(char *, ConfigResult *result);
	}
	Config;


/* ========================== */


extern PiKrellCam	pikrellcam;

extern CameraObject	camera;
extern CameraObject	still_jpeg_encoder;
extern CameraObject	mjpeg_encoder;
extern CameraObject	video_h264_encoder;
extern CameraObject	stream_splitter;
extern CameraObject	stream_resizer;

extern VideoCircularBuffer video_circular_buffer;
extern AudioCircularBuffer audio_circular_buffer;
extern MotionFrame  motion_frame;
extern TimeLapse	time_lapse;

extern char  *mmal_status[];


/* ========================== */

boolean		out_port_callback(CameraObject *obj, int port_num,
					void callback());
boolean		ports_callback_connect(CameraObject *out, int port_num,
					CameraObject *in, void callback());
boolean		ports_tunnel_connect(CameraObject *out, int port_num,
					CameraObject *in);
boolean		h264_encoder_create(char *name, CameraObject *encoder,
					MMAL_PORT_T *src_port);
boolean		jpeg_encoder_create(char *name, CameraObject *encoder,
					MMAL_PORT_T *src_port, int quality);
boolean		splitter_create(char *name, CameraObject *splitter,
					MMAL_PORT_T *src_port);
boolean		resizer_create(char *name, CameraObject *resizer,
					MMAL_PORT_T *src_port,
					unsigned int resize_width, unsigned int resize_height);
void		mjpeg_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
void		I420_video_callback(MMAL_PORT_T *port,
					MMAL_BUFFER_HEADER_T *buffer);
boolean		still_capture(char *fname, boolean motion_still);
void		still_jpeg_callback(MMAL_PORT_T *port,
					MMAL_BUFFER_HEADER_T *buffer);
void		video_h264_encoder_callback(MMAL_PORT_T *port,
					MMAL_BUFFER_HEADER_T *mmalbuf);
boolean		camera_create(void);
void		camera_object_destroy(CameraObject *obj);
void		video_circular_buffer_init(void);
void		start_video_thread(void);

void		mmalcam_config_parameters_set_camera(void);
boolean 	mmalcam_config_parameter_set(char *name, char *value, boolean set_camera);
CameraParameter
			*mmalcam_config_parameter_get(char *name);


void			zoom_percent(int percent);

extern boolean	config_load(char *config_file);
extern void		config_save(char *config_file);
extern void 	config_set_defaults(char *homedir);
extern boolean	config_set_option(char *opt, char *arg, boolean set_safe);
boolean			config_boolean_value(char *value);
void			config_set_boolean(boolean *result, char *arg);
void			config_timelapse_save_status(void);
void			config_timelapse_load_status(void);
void			config_media_sequence_save(void);
void			config_media_sequence_load(void);

char		*fname_base(char *path);
void		log_printf_no_timestamp(char *fmt, ...);
void		log_printf(char *fmt, ...);
void		video_record_start(VideoCircularBuffer *vcb, int);
void		thumb_convert(void);
void		video_record_stop(VideoCircularBuffer *vcb);
void		camera_start(void);
void		camera_stop(void);
void		camera_restart(void);
void		timelapse_capture(void);
char		*substitute_var(char *str, char V, char *fmt_arg);
char		*media_pathname(char *dir, char *fname, time_t time,
							char var1, char *arg1, char var2, char *arg2);
void	command_process(char *command_line);

void	motion_init(void);
void	motion_command(char *cmd_line);
void	motion_frame_process(VideoCircularBuffer *vcb, MotionFrame *mf);
void	motion_regions_config_save(char *config_file, boolean inform);
boolean	motion_regions_config_load(char *config_file, boolean inform);
void	motion_preview_file_event(void);
void	motion_preview_area_fixup(void);
void	make_preview_pathname(char *media_pathname);
void	print_cvec(char *str, CompositeVector *cvec);
void	motion_events_write(MotionFrame *mf, int type, float detect_time);
void	motion_detects_fifo_write(MotionFrame *mf);

void	motion_stills_start(MotionFrame *mf);
void	motion_stills_stop(void);

/* On Screen Display
*/
void	display_init(void);
boolean	display_is_default();
void	display_set_default(void);
void	display_command(char *cmd_line);
void	display_draw(uint8_t *i420);
void	display_inform(char *args);
void	display_inform_clear(void);
void	display_motion_limit_adjustment_sync(PresetSettings *settings);

void	display_quit(void);

/* Events */

Event	*event_add(char *name, time_t time, time_t period,
						void (*func)(), void *data);
Event	*event_count_down_add(char *name, int count,
						void (*func)(), void *data);

Event	*event_find(char *name);
void	event_list_lock(void);
void	event_list_unlock(void);
void	event_remove(Event *event);
boolean	event_remove_name(char *name);
void	event_process(void);
void	event_motion_enable_cmd(char *cmd);
void	event_motion_begin_cmd(char *cmd);
void	event_still_capture_cmd(char *cmd);
void	event_motion_still_capture(void *path);

void	event_video_diskfree_percent(char *type);
void	event_archive_diskfree_percent(char *type);
void	event_still_diskfree_percent(char *subdir);
void	event_loop_diskusage_percent(void);

void	event_notify_expire(boolean *notify);
int		exec_wait(char *command, char *arg);
void	exec_no_wait(char *command, char *arg, boolean do_log);
void	event_shutdown_request(boolean reboot);
char	*expand_command(char *command, char *arg);

void	multicast_init(void);
void	multicast_recv(void);
void	multicast_send(char *seq, char *message);

void	preset_command(char *args);
void	preset_config_load(void);
void	preset_config_save(void);
void	preset_state_save(void);
void	preset_state_load(void);
void	preset_load_values(boolean do_pan);
void	preset_on_check(int pan, int tilt);
PresetPosition *preset_find_at_position(int pan, int tilt);
void	preset_settings_set_modified(void);
void	preset_regions_set_modified(void);
void	preset_pan_range(int *max, int *min);
void	preset_tilt_range(int *max, int *min);

void	gpio_alt_function(int pin, int altfn);
void	gpio_set_mode(int pin, int mode);
void	gpio_set_pud(int pin, int pud);
void	gpio_to_channel(int gpio, int *channel, int *altfn);
int		gpio_read(int pin);
void	gpio_write(int pin, int level);
void	gpio_hardware_pwm(int pin);
void	servo_get_position(int *pan, int *tilt);
void	servo_move(int pan, int tilt, int delay);
void	servo_command(char *args);
void	servo_init(void);

boolean	audio_mic_open(boolean inform);
void	audio_retry_open(void);
void	audio_buffer_set_record_head(AudioCircularBuffer *acb, int head);
void	audio_buffer_set_tail(AudioCircularBuffer *acb, int position);
boolean	audio_record_start(void);
void	audio_record_stop(void);
void	audio_buffer_set_record_head_tail(AudioCircularBuffer *acb, int head, int tail);
int		audio_frames_offset_from_video(AudioCircularBuffer *acb);
void	audio_command(char *args);
void	audio_circular_buffer_init(void);

void	sun_times_init(void);
void	at_commands_config_save(char *config_file);
boolean	at_commands_config_load(char *config_file);

void	setup_h264_tcp_server(void);
int		setup_mjpeg_tcp_server(void);
void	tcp_poll_connect(void);
void	tcp_send_h264_header(void *data, int len);
void	tcp_send_h264_data(char * what, void *data, int len);

void	pikrellcam_cleanup(void);

#endif			/* _PIKRELLCAM_H		*/
