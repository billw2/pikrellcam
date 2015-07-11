/* PiKrellCam
|
|  Copyright (C) 2015 Bill Wilson    billw@gkrellm.net
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
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "utils.h"

#ifndef MAX
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define PIKRELLCAM_CONFIG_DIR					".pikrellcam"
#define PIKRELLCAM_CONFIG						"pikrellcam.conf"
#define PIKRELLCAM_MOTION_REGIONS_CONFIG		"motion-regions.conf"
#define PIKRELLCAM_MOTION_REGIONS_CUSTOM_CONFIG	"motion-regions-%s.conf"
#define PIKRELLCAM_AT_COMMANDS_CONFIG			"at-commands.conf"
#define PIKRELLCAM_TIMELAPSE_STATUS				"timelapse.status"

  /* These subdirs must match what is in www/config.php
  */
#define PIKRELLCAM_VIDEO_SUBDIR					"videos"
#define PIKRELLCAM_STILL_SUBDIR					"stills"
#define PIKRELLCAM_TIMELAPSE_SUBDIR				"timelapse"


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
	pid_t	child_pid;	/* > 0 if event to be executed after child exit */

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


  /* ------------------ Motion Detection ---------------
  */
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
			len2,		/*   and the length^2 */
			mag2_count;	/* Number of MotionVectors added into this composite. */
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
	int		box_w,			/* A box around the composite center for testing */
			box_h;			/*  the vector concentration composing the cvec  */
	int		reject_count,	/* vectors not pointing in composite direction.  */
			sparkle_count;	/* number of isolated vectors */
	boolean	motion;
	}
	MotionRegion;


#define	MOTION_NONE				0
#define	MOTION_LIGHTSWITCH		1
#define	MOTION_DETECTED			2

#define EVENT_MOTION_BEGIN            1
#define EVENT_MOTION_END              2
#define EVENT_PREVIEW_SAVE            4
#define EVENT_MOTION_PREVIEW_SAVE_CMD 8

typedef struct
	{
	int				motion_status;
	boolean			motion_enable,
					show_vectors,
					show_regions;

	boolean			do_preview_save,
					do_preview_save_cmd;

	int				width,			/* width in macroblocks */
					height;			/* height in macroblocks */
	int				vectors_size;
	MotionVector	*vectors;
	CompositeVector	best_frame_vector,
					best_motion_vector;
	int16_t			*trigger;
	int				n_regions,
					selected_region,
					prev_selected_region;

	pthread_mutex_t	region_list_mutex;
	SList			*motion_region_list;
	int				mag2_limit;      /* magnitude^2 limit for a vector to add to region_vector */
	int				mag2_limit_count; /* Threshold number of vectors added into */
			                          /* region_vector to make it a valid motion event */
	int		trigger_count,	/* Total # of vectors > mag2_limit */
			reject_count,	/* # of frame vectors not pointing same as composite vector */
			sparkle_count,	/* nuber of isolated vectors */
			vertical_count;

	float	sparkle_expma;

	int		frame_window;
	}
	MotionFrame;

#define MF_TRIGGER_SIZE	(motion_frame.width * motion_frame.height \
							* sizeof(*motion_frame.trigger))

#define	VCB_STATE_NONE					0
#define	VCB_STATE_MOTION_RECORD_START	1
#define	VCB_STATE_MOTION_RECORD			2
#define	VCB_STATE_MANUAL_RECORD_START	4
#define	VCB_STATE_MANUAL_RECORD			8
#define VCB_STATE_RESTARTING			0x100

typedef struct
	{
	int		position;
	time_t	t_frame;
	}
	KeyFrame;


#define	H264_HEADER_SIZE	29	/* What the GPU gives us */

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

	FILE		*file;
	int			state;

	int8_t	   h264_header[H264_HEADER_SIZE];
	int		   h264_header_position;

	int8_t	   *data; 		/* h.264 video data array      */
	int			size;		/* size in bytes of data array */
	int			head,
				tail;
				
	KeyFrame	key_frame[KEYFRAME_SIZE];
	int			pre_frame_index,
				cur_frame_index;
	boolean		in_keyframe,
				pause;

	time_t		record_start,
				motion_last_detect_time,
				motion_sync_time;
	}
	VideoCircularBuffer;


  /* -------------- The Global PiKrellCam Environment -----------
  */
typedef struct
	{
	int		event_gap,
			pre_capture,
			post_capture,
			confirm_gap;
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
			mp4_box_fps,
			still_quality;
	boolean	modified;
	}
	CameraAdjust;

extern CameraAdjust	camera_adjust_temp;


typedef struct
	{

	time_t	t_now,
			t_start;
	struct tm tm_local;
	int		second_tick;

	char	*install_dir,
			*media_dir,
			*video_dir,
			*still_dir,
			*timelapse_dir,
			*script_dir,
			*command_fifo;

	char	*config_dir,
			*config_file,
			*motion_regions_config_file,
			*at_commands_config_file,
			*log_file,
			*on_startup_cmd;

	int		verbose,
			verbose_motion;
	char	hostname[HOST_NAME_MAX],
			*effective_user;


	char	*latitude,
			*longitude;

	boolean	motion_enable,
			config_modified;

	MotionTimes
			motion_times;
	int		motion_vectors_dimming,
			motion_magnitude_limit,
			motion_magnitude_limit_count;
	char	*on_motion_begin_cmd,
			*on_motion_end_cmd;
	char	*preview_filename;
	char	*motion_preview_save_mode,
			*on_motion_preview_save_cmd;
	boolean	motion_preview_clean,
			motion_vertical_filter;

	CameraConfig
			camera_config;
	CameraAdjust
			camera_adjust;

	char	*video_filename,
			*video_h264,
			*video_last_save,
			*video_pathname,
			*video_manual_tag,
			*video_motion_tag;
	int		video_manual_sequence,
			video_motion_sequence;
	FILE	*video_file;
	
	boolean	video_mp4box;


	char	*mjpeg_dir,
			*mjpeg_filename;
	int		mjpeg_width,
			mjpeg_height,
			mjpeg_quality,
			mjpeg_divider;

	char	*still_filename,
			*still_last_save;
	int		still_sequence;

	char	*timelapse_video_name,
			*timelapse_format,
			*timelapse_last_save,
			*timelapse_status_file;
	char	*on_timelapse_end_cmd;

	char	*annotate_format_string;
	boolean	annotate_enable,
			annotate_show_motion,
			annotate_show_frame,
			annotate_black_bg;

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
			*exposure_compensation,
			*exposure_mode,
			*white_balance,
			*image_effect,
			*color_effect,
			*rotation,
			*flip,
			*crop,
			*shutter_speed,
			*raw_capture;
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
boolean		still_capture(char *fname);
void		still_jpeg_callback(MMAL_PORT_T *port,
					MMAL_BUFFER_HEADER_T *buffer);
void		video_h264_encoder_callback(MMAL_PORT_T *port,
					MMAL_BUFFER_HEADER_T *mmalbuf);
boolean		camera_create(void);
void		camera_object_destroy(CameraObject *obj);
void		circular_buffer_init(void);

void		mmalcam_config_parameters_set_camera(void);
boolean 	mmalcam_config_parameter_set(char *name, char *value, boolean set_camera);
CameraParameter
			*mmalcam_config_parameter_get(char *name);

extern boolean	config_load(char *config_file);
extern void		config_save(char *config_file);
extern void 	config_set_defaults(void);
extern boolean	config_set_option(char *opt, char *arg, boolean set_safe);
boolean			config_boolean_value(char *value);
void			config_set_boolean(boolean *result, char *arg);
void			config_timelapse_save_status(void);
void			config_timelapse_load_status(void);

char		*fname_base(char *path);
void		log_printf(char *fmt, ...);
void		video_record_start(VideoCircularBuffer *vcb, int);
void		video_record_stop(VideoCircularBuffer *vcb);
void		camera_start(void);
void		camera_stop(void);
void		timelapse_capture(void);
char		*substitute_var(char *str, char V, char *fmt_arg);
char		*media_pathname(char *dir, char *fname,
							char var1, char *arg1, char var2, char *arg2);
void	command_process(char *command_line);

void	motion_init(void);
void	motion_command(char *cmd_line);
void	motion_frame_process(VideoCircularBuffer *vcb, MotionFrame *mf);
void	motion_regions_config_save(char *config_file);
boolean	motion_regions_config_load(char *config_file);
void	motion_preview_file_event(void);

/* On Screen Display
*/
void	display_init(void);
boolean	display_is_default();
void	display_set_default(void);
void	display_command(char *cmd_line);
void	display_draw(uint8_t *i420);
void	display_quit(void);

/* Events */

Event	*event_add(char *name, time_t time, time_t period,
						void (*func)(), void *data);
Event	*event_count_down_add(char *name, int count,
						void (*func)(), void *data);

Event	*event_find(char *name);
void	event_remove(Event *event);
void	event_process(void);
void	event_preview_save(void);
void	event_preview_save_cmd(char *cmd);
void	event_preview_dispose(void);
void	event_motion_end_cmd(char *cmd);

void	event_notify_expire(boolean *notify);
void	event_child_signal(int sig_num);
int		exec_wait(char *command, char *arg);
void	exec_no_wait(char *command, char *arg);
Event	*exec_child_event(char *event_name, char *command, char *arg);

void	sun_times_init(void);
void	at_commands_config_save(char *config_file);
boolean	at_commands_config_load(char *config_file);


#endif			/* _PIKRELLCAM_H		*/
