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

#include "pikrellcam.h"

boolean
config_boolean_value(char *value)
	{
	if (   (*value == '1' && *(value + 1) == '\0')
	    || !strcasecmp(value, "on")
	    || !strcasecmp(value, "true")
	    || !strcasecmp(value, "high")
	   )
		return TRUE;
	return FALSE;
	}

int
find_param(char *name, ParameterTable *table, int table_size)
	{
	ParameterTable	*entry;

	for (entry = table; entry < table + table_size; ++entry)
		{
		if (!strcmp(entry->name, name))
			return entry->param;
		}
	return -1;
	}


ParameterTable	exposure_mode_table[] =
	{
	{ MMAL_PARAM_EXPOSUREMODE_OFF,          "off" },
	{ MMAL_PARAM_EXPOSUREMODE_AUTO,         "auto" },
	{ MMAL_PARAM_EXPOSUREMODE_NIGHT,        "night" },
	{ MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW, "nightpreview" },
	{ MMAL_PARAM_EXPOSUREMODE_BACKLIGHT,    "backlight" },
	{ MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT,    "spotlight" },
	{ MMAL_PARAM_EXPOSUREMODE_SPORTS,       "sports" },
	{ MMAL_PARAM_EXPOSUREMODE_SNOW,         "snow" },
	{ MMAL_PARAM_EXPOSUREMODE_BEACH,        "beach" },
	{ MMAL_PARAM_EXPOSUREMODE_VERYLONG,     "verylong" },
	{ MMAL_PARAM_EXPOSUREMODE_FIXEDFPS,     "fixedfps" },
	{ MMAL_PARAM_EXPOSUREMODE_ANTISHAKE,    "antishake" },
	{ MMAL_PARAM_EXPOSUREMODE_FIREWORKS,    "fireworks" }
	};

#define	EXPOSURE_MODE_TABLE_SIZE	\
			(sizeof(exposure_mode_table) / sizeof(ParameterTable))

MMAL_STATUS_T
exposure_mode_set(char *option, char *setting)
	{
	MMAL_PARAM_EXPOSUREMODE_T		value;
	MMAL_PARAMETER_EXPOSUREMODE_T	exp_mode =
										{ { MMAL_PARAMETER_EXPOSURE_MODE,
										sizeof(exp_mode) }, 0};
	MMAL_STATUS_T					status = MMAL_EINVAL;

	if ((value = find_param(setting,
				exposure_mode_table, EXPOSURE_MODE_TABLE_SIZE)) >= 0)
		{
		exp_mode.value = value;
		status = mmal_port_parameter_set(camera.control_port,
										&exp_mode.hdr);
		}
	return status;
	}

ParameterTable	metering_mode_table[] =
	{
	{ MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE,	"average"},
	{ MMAL_PARAM_EXPOSUREMETERINGMODE_SPOT,		"spot" },
	{ MMAL_PARAM_EXPOSUREMETERINGMODE_BACKLIT,	"backlit" },
	{ MMAL_PARAM_EXPOSUREMETERINGMODE_MATRIX,	"matrix" }
	};

#define	METERING_MODE_TABLE_SIZE	\
			(sizeof(metering_mode_table) / sizeof(ParameterTable))

MMAL_STATUS_T
metering_mode_set(char *option, char *setting)
	{
	MMAL_PARAMETER_EXPOSUREMETERINGMODE_T	meter_mode =
			{{MMAL_PARAMETER_EXP_METERING_MODE,sizeof(meter_mode)}, 0};
	int				value;
	MMAL_STATUS_T	status = MMAL_EINVAL;

	if ((value = find_param(setting, metering_mode_table,
							METERING_MODE_TABLE_SIZE)) >= 0)
		{
		meter_mode.value = value;
		status = mmal_port_parameter_set(camera.control_port,
							&meter_mode.hdr);
		}
	return status;
	}

ParameterTable awb_mode_table[] =
	{
	{ MMAL_PARAM_AWBMODE_OFF,           "off" },
	{ MMAL_PARAM_AWBMODE_AUTO,          "auto" },
	{ MMAL_PARAM_AWBMODE_SUNLIGHT,      "sunlight" },
	{ MMAL_PARAM_AWBMODE_CLOUDY,        "cloudy" },
	{ MMAL_PARAM_AWBMODE_SHADE,         "shade" },
	{ MMAL_PARAM_AWBMODE_TUNGSTEN,      "tungsten" },
	{ MMAL_PARAM_AWBMODE_FLUORESCENT,   "flourescent" },
	{ MMAL_PARAM_AWBMODE_INCANDESCENT,  "incandescent" },
	{ MMAL_PARAM_AWBMODE_FLASH,         "flash" },
	{ MMAL_PARAM_AWBMODE_HORIZON,       "horizon" }
	};

#define	AWB_MODE_TABLE_SIZE	\
			(sizeof(awb_mode_table) / sizeof(ParameterTable))

MMAL_STATUS_T
awb_mode_set(char *option, char *setting)
	{
	MMAL_PARAMETER_AWBMODE_T	awb_mode =
				{ { MMAL_PARAMETER_AWB_MODE, sizeof(awb_mode) }, 0 };
	int				value;
	MMAL_STATUS_T	status = MMAL_EINVAL;

	if ((value = find_param(setting, awb_mode_table,
						AWB_MODE_TABLE_SIZE)) >= 0)
		{
		awb_mode.value = value;
		status = mmal_port_parameter_set(camera.control_port,
						&awb_mode.hdr);
		}
	return status;
	}


ParameterTable image_fx_table[] =
	{
	{ MMAL_PARAM_IMAGEFX_NONE,          "none" },
	{ MMAL_PARAM_IMAGEFX_NEGATIVE,      "negative" },
	{ MMAL_PARAM_IMAGEFX_SOLARIZE,      "solarize" },
//	{ MMAL_PARAM_IMAGEFX_POSTERIZE,     "posterize" },
//	{ MMAL_PARAM_IMAGEFX_WHITEBOARD,    "whiteboard" },
//	{ MMAL_PARAM_IMAGEFX_BLACKBOARD,    "blackboard" },
	{ MMAL_PARAM_IMAGEFX_SKETCH,        "sketch" },
	{ MMAL_PARAM_IMAGEFX_DENOISE,       "denoise" },
	{ MMAL_PARAM_IMAGEFX_EMBOSS,        "emboss" },
	{ MMAL_PARAM_IMAGEFX_OILPAINT,      "oilpaint" },
	{ MMAL_PARAM_IMAGEFX_HATCH,         "hatch" },
	{ MMAL_PARAM_IMAGEFX_GPEN,          "gpen" },
	{ MMAL_PARAM_IMAGEFX_PASTEL,        "pastel" },
	{ MMAL_PARAM_IMAGEFX_WATERCOLOUR,   "watercolor" },
	{ MMAL_PARAM_IMAGEFX_FILM,          "film" },
	{ MMAL_PARAM_IMAGEFX_BLUR,          "blur" },
	{ MMAL_PARAM_IMAGEFX_SATURATION,    "saturation" },
	{ MMAL_PARAM_IMAGEFX_COLOURSWAP,    "colorswap" },
	{ MMAL_PARAM_IMAGEFX_WASHEDOUT,     "washedout" },
	{ MMAL_PARAM_IMAGEFX_POSTERISE,     "posterise" },
//	{ MMAL_PARAM_IMAGEFX_COLOURPOINT,   "colorpoint" },
//	{ MMAL_PARAM_IMAGEFX_COLOURBALANCE, "colourbalance" },
	{ MMAL_PARAM_IMAGEFX_CARTOON,       "cartoon" }
	};

#define	IMAGE_FX_TABLE_SIZE	\
			(sizeof(image_fx_table) / sizeof(ParameterTable))

MMAL_STATUS_T
image_effect_set(char *option, char *setting)
	{
	MMAL_PARAMETER_IMAGEFX_T	image_fx =
				{ { MMAL_PARAMETER_IMAGE_EFFECT, sizeof(image_fx) }, 0 };
	MMAL_PARAM_IMAGEFX_T		value;
	MMAL_STATUS_T				status = MMAL_EINVAL;

	if ((value = (MMAL_PARAM_IMAGEFX_T) find_param(setting, image_fx_table,
							IMAGE_FX_TABLE_SIZE)) >= 0)
		{
		image_fx.value = value;
		status = mmal_port_parameter_set(camera.control_port,
							&image_fx.hdr);
		}
	return status;
	}

ParameterTable parameter_table[] =
	{
	{ MMAL_PARAMETER_SHARPNESS,		"sharpness" },		/* -100 to 100 */
	{ MMAL_PARAMETER_CONTRAST,		"contrast" },		/* -100 to 100 */
	{ MMAL_PARAMETER_BRIGHTNESS,	"brightness" },		/*    0 to 100 */
	{ MMAL_PARAMETER_SATURATION,	"saturation" },		/* -100 to 100 */

	{ MMAL_PARAMETER_ISO, 			"iso" },					/* uint */
	{ MMAL_PARAMETER_SHUTTER_SPEED,	"shutter_speed" },			/* uint */

	{ MMAL_PARAMETER_EXPOSURE_COMP,
 									"exposure_compensation" },	/* int -25 to 25 */
	{ MMAL_PARAMETER_ROTATION,		"rotation" },				/* int */

	{ MMAL_PARAMETER_VIDEO_STABILISATION,	"video_stabilisation" }, /* bool */
	{ MMAL_PARAMETER_ENABLE_RAW_CAPTURE,	"raw_capture" },		/* bool */
	};


#define	PARAMETER_TABLE_SIZE	\
			(sizeof(parameter_table) / sizeof(ParameterTable))

MMAL_STATUS_T
rational_control_set(char *option, char *setting)
	{
	MMAL_RATIONAL_T		rational;
	int					param, value, min;
	MMAL_STATUS_T		status = MMAL_EINVAL;

	min = !strcmp(option, "brightness") ? 0 : -100;
	value = atoi(setting);
	if (value < min)
		value = min;
	if (value > 100)
		value = 100;

	param = find_param(option, parameter_table, PARAMETER_TABLE_SIZE);

	if (param >= 0)
		{
		rational.num = value;
		rational.den = 100;
		status = mmal_port_parameter_set_rational(camera.control_port,
								param, rational);
		}
	return status;
	}

MMAL_STATUS_T
boolean_control_set(char *option, char *setting)
	{
	int				value, param;
	MMAL_STATUS_T	status = MMAL_EINVAL;

	value = config_boolean_value(setting);
	param = find_param(option, parameter_table, PARAMETER_TABLE_SIZE);
	if (param >= 0)
		{
		status = mmal_port_parameter_set_boolean(camera.control_port,
							param, value);
		}
	return status;
	}

MMAL_STATUS_T
int32_control_set(char *option, char *setting)
	{
	int				param, value;
	MMAL_STATUS_T	status = MMAL_EINVAL;

	value = atoi(setting);
	param = find_param(option, parameter_table, PARAMETER_TABLE_SIZE);
	if (param >= 0)
		{
		status = mmal_port_parameter_set_int32(camera.control_port,
							param, value);
		}
	return status;
	}

MMAL_STATUS_T
uint32_control_set(char *option, char *setting)
	{
	int				param;
	uint32_t		value;
	MMAL_STATUS_T	status = MMAL_EINVAL;

	value = (uint32_t) atoi(setting);
	param = find_param(option, parameter_table, PARAMETER_TABLE_SIZE);
	if (param >= 0)
		{
		status = mmal_port_parameter_set_boolean(camera.control_port,
							param, value);
		}
	return status;
	}

MMAL_STATUS_T
rotation_control_set(char *option, char *setting)
	{
	int				i,
					value = atoi(setting);
	MMAL_STATUS_T	status = MMAL_EINVAL;

	value = ((value % 360 ) / 90) * 90;
	for (i = 0; i < MAX_CAMERA_PORTS; ++i)
		{
		status = mmal_port_parameter_set_int32(camera.component->output[i],
						MMAL_PARAMETER_ROTATION, value);
		}
	return status;
	}

MMAL_STATUS_T
flip_control_set(char *option, char *setting)
	{
	MMAL_PARAMETER_MIRROR_T mirror =
                    {{MMAL_PARAMETER_MIRROR, sizeof(MMAL_PARAMETER_MIRROR_T)},
                    MMAL_PARAM_MIRROR_NONE};
	CameraParameter *param;
	int             hflip = FALSE,
	                vflip = FALSE;
	int             i;
	MMAL_STATUS_T   status = MMAL_EINVAL;

	if (!strcmp(option, "hflip"))
		{
		hflip = config_boolean_value(setting);
		if (!hflip)
			setting = "off";	/* config transition */
		param = mmalcam_config_parameter_get("hflip");
		dup_string(&param->arg, setting);

		param = mmalcam_config_parameter_get("vflip");
		vflip = config_boolean_value(param->arg);
		}
	else
		{
		vflip = config_boolean_value(setting);
		param = mmalcam_config_parameter_get("vflip");
		dup_string(&param->arg, setting);

		param = mmalcam_config_parameter_get("hflip");
		hflip = config_boolean_value(param->arg);
		}
	if (hflip && vflip)
		mirror.value = MMAL_PARAM_MIRROR_BOTH;
	else if (hflip)
		mirror.value = MMAL_PARAM_MIRROR_HORIZONTAL;
	else if (vflip)
		mirror.value = MMAL_PARAM_MIRROR_VERTICAL;

	for (i = 0; i < MAX_CAMERA_PORTS; ++i)
		if ((status = mmal_port_parameter_set(camera.component->output[i],
				&mirror.hdr)) != MMAL_SUCCESS)
			break;

	return status;
	}

MMAL_STATUS_T
crop_control_set(char *option, char *setting)
	{
	MMAL_PARAMETER_INPUT_CROP_T crop =
			{{MMAL_PARAMETER_INPUT_CROP, sizeof(MMAL_PARAMETER_INPUT_CROP_T)}};
	MMAL_STATUS_T				status = MMAL_EINVAL;

	if (sscanf(setting, "%d %d %d %d", &crop.rect.x, &crop.rect.y,
				&crop.rect.width, &crop.rect.height) == 4)
		status = mmal_port_parameter_set(camera.control_port, &crop.hdr);
	return status;
	}

MMAL_STATUS_T
color_effect_set(char *option, char *setting)
	{
	MMAL_PARAMETER_COLOURFX_T colfx =
			{{MMAL_PARAMETER_COLOUR_EFFECT,sizeof(colfx)}, 0, 0, 0};
	char			enable[8];
	MMAL_STATUS_T	status = MMAL_EINVAL;

	if (sscanf(setting, "%7s %d %d", enable, &colfx.u, &colfx.v) == 3)
		{
		colfx.enable = config_boolean_value(enable);
		status = mmal_port_parameter_set(camera.control_port, &colfx.hdr);
		}
	return status;
	}


/* ========================================== */

static CameraParameter  camera_parameters[] =
	{
	{ "sharpness",  "0",   rational_control_set,  &pikrellcam.sharpness },
	{ "contrast",   "0",   rational_control_set,  &pikrellcam.contrast },
	{ "brightness", "50",  rational_control_set,  &pikrellcam.brightness },
	{ "saturation", "0",   rational_control_set,  &pikrellcam.saturation },

	{ "iso",           "0",  uint32_control_set,  &pikrellcam.iso },
	{ "shutter_speed", "0",  uint32_control_set,  &pikrellcam.shutter_speed },

	{ "exposure_compensation", "0", int32_control_set, &pikrellcam.exposure_compensation},

	{ "video_stabilisation", "false",  boolean_control_set,  &pikrellcam.video_stabilisation },
	{ "raw_capture",         "false",  boolean_control_set,  &pikrellcam.raw_capture },

	{ "rotation",      "0",    rotation_control_set, &pikrellcam.rotation },
	{ "exposure_mode", "auto", exposure_mode_set,    &pikrellcam.exposure_mode },
	{ "image_effect",  "none", image_effect_set,     &pikrellcam.image_effect },

	{ "hflip",  "off",    flip_control_set,  &pikrellcam.hflip },
	{ "vflip",  "off",    flip_control_set,  &pikrellcam.vflip },
	{ "crop",   "0 0 65536 65536", crop_control_set,  &pikrellcam.crop },

	{ "metering_mode", "average",       metering_mode_set, &pikrellcam.metering_mode },
	{ "white_balance", "auto",          awb_mode_set,      &pikrellcam.white_balance },
	{ "color_effect",  "false 128 128", color_effect_set,  &pikrellcam.color_effect }
	};

#define CAMERA_PARAMETERS_SIZE \
		(sizeof(camera_parameters) / sizeof(CameraParameter))


CameraParameter *
mmalcam_config_parameter_get(char *name)
	{
	CameraParameter	*param;

	for (param = &camera_parameters[0];
				param < &camera_parameters[CAMERA_PARAMETERS_SIZE]; ++param)
		{
		if (!strcmp(param->name, name))
			return param;
		}
	return NULL;
	}

boolean
mmalcam_config_parameter_set(char *name, char *arg, boolean set_camera)
	{
	CameraParameter	*param;
	boolean			found = FALSE;
	MMAL_STATUS_T	status = MMAL_SUCCESS;

	for (param = &camera_parameters[0];
				param < &camera_parameters[CAMERA_PARAMETERS_SIZE]; ++param)
		{
		if (!strcmp(param->name, name) && param->func)
			{
			found = TRUE;
			if (set_camera)
				status = (param->func)(name, arg);
			if (status == MMAL_SUCCESS)
				dup_string(&param->arg, arg);		/* replace in config table */
			break;
			}
		}
	if (status != MMAL_SUCCESS)
		log_printf("mmalcam_config_parameter_set: %s %s [%s]\n", name, arg,
				!found ? "not found" :
					(set_camera ? mmal_status[status] : "config set (not camera)"));
	return (found && (status == MMAL_SUCCESS));
	}

void
mmalcam_config_parameters_set_camera(void)
	{
	CameraParameter		*param;
	MMAL_STATUS_T		status;

	for (param = &camera_parameters[0];
				param < &camera_parameters[CAMERA_PARAMETERS_SIZE]; ++param)
		{
		if (param->func)
			{
			status = (param->func)(param->name, param->arg);
			if (status != MMAL_SUCCESS)
				log_printf("mmalcam_all_parameter_set: %s %s  \t[%s]\n",
							param->name, param->arg, mmal_status[status]);
			}
		}
	}


/* ========================================== */

static boolean
config_string_set(char *arg, ConfigResult *result)
	{
	dup_string(result->string, arg);
	return TRUE;
	}

void
config_set_boolean(boolean *result, char *arg)
	{
	if (!strcasecmp(arg, "toggle"))
			*result = *result ? 0 : 1;
	else *result = config_boolean_value(arg);
	}

static boolean
config_value_bool_set(char *arg, ConfigResult *result)
	{
	*result->value = config_boolean_value(arg);
	return TRUE;
	}

static int
config_value_int_set(char *arg, ConfigResult *result)
	{
	int	valid = TRUE;

	if (isdigit(*arg))
		*result->value = atoi(arg);
	else
		{
		printf("    Bad config_value_int_set: %s\n", arg);
		valid = FALSE;
		}
	return valid;
	}


static Config  config[] =
	{
	{ "# ----------------------------------------------------------\n"
	  "#\n"
	  "# The PiKrellCam installation directory.\n"
	  "# This must match where PiKrellCam is installed and it is checked by\n"
	  "# the install-pikrellcam.sh script.  This should not need to be edited.\n"
	  "#",
	"install_dir", "/home/pi/pikrellcam", TRUE, { .string = &pikrellcam.install_dir }, config_string_set },

	{ "# Directory for the stream mjpeg file and info file. These files\n"
	  "# are frequently updated so this directory should be in a tmpfs.\n"
	  "# This could be a directory in /tmp if your /tmp is a tmpfs.\n"
	  "# Avoid putting this directory under /run/shm or /dev/shm because these\n"
	  "# directories are subject to automatic cleanups which could delete the\n"
	  "# tmpfs_dir out from under a running pikrellcam if running headless.\n"
	  "#",
	"tmpfs_dir",  "/run/pikrellcam", TRUE, { .string = &pikrellcam.tmpfs_dir }, config_string_set },

	{ "# If media_dir has no leading '/' it will be a sub directory in install_dir.\n"
	  "# Otherwise it is a full pathname to the media directory.\n"
	  "# So the default media dir is /home/pi/pikrellcam/media.\n"
	  "# An alternate choice could be /home/pi/media if you set the full path.\n"
	  "# Even /tmp/media with /tmp a tmpfs can be an option for a setup that\n"
	  "# will manage by hand or script the limited space available.\n"
	  "# A file system may be mounted on the media dir in the startup script.\n"
	  "#",
	"media_dir", "media", TRUE, { .string = &pikrellcam.media_dir }, config_string_set },

	{ "# If archive_dir has no leading '/' it will be a sub directory under media_dir.\n"
	  "# Otherwise it is a full pathname to the archive directory.\n"
	  "# So the default archive dir is /home/pi/pikrellcam/media/archive.\n"
	  "# When media files are archived, they will be moved to sub directories:\n"
	  "#     archive_dir/year/month/day/[videos|thumbs|stills]\n"
	  "# A file system may be mounted on the archive dir in the startup script.\n"
	  "#",
	"archive_dir", "archive", TRUE, { .string = &pikrellcam.archive_dir }, config_string_set },

	{ "# Log file.\n"
	  "#",
	"log_file",  "/tmp/pikrellcam.log", TRUE, { .string = &pikrellcam.log_file }, config_string_set },

	{ "# At startup and at each new day, trim the log file number of lines\n"
	  "# to log_lines.  If log_lines is 0 the log file is not trimmed.\n"
	  "#",
	"log_lines", "1000", FALSE, {.value = &pikrellcam.log_lines}, config_value_int_set},

	{ "# Command to run at PiKrellCam startup.  This is run after the config\n"
	  "# files are loaded but before the camera is started or directories\n"
	  "# are checked.  If you want a start command that runs after the camera\n"
	  "# is started, add a command to the file: ~/.pikrellcam/at-commands.conf.\n"
	  "# The default on_startup script can mount a drive on the media directory.\n"
	  "# Edit MOUNT_DISK in the startup script in $C (the scripts directory) to\n"
	  "# enable mounting.\n"
	  "#",
	"on_startup", "$C/startup $I $m $G",  TRUE, {.string = &pikrellcam.on_startup_cmd}, config_string_set },


	{ "\n# -------------------- Motion Detect Options -----------------------\n"
	  "#\n"
	  "# Enable pikrellcam motion detection at startup\n"
	  "#",
	"motion_enable",        "off", FALSE, {.value = &pikrellcam.motion_enable},       config_value_bool_set},

	{ "# Motion vectors must be at least this magnitude for a motion detect.\n"
	  "# Minimum is 3 for detecting the slowest moving objects possible.\n"
	  "#",
	"motion_magnitude_limit",  "5", FALSE, {.value = &pikrellcam.motion_magnitude_limit},      config_value_int_set},

	{ "# The count of vectors required for a motion detect.\n"
	  "# Minimum is 2 for detecting the smallest objects possible.\n"
	  "#",
	"motion_magnitude_limit_count",  "4", FALSE, {.value = &pikrellcam.motion_magnitude_limit_count},      config_value_int_set},

	{ "# Motion vector count minimum for a burst motion detect.\n"
	  "# For large/close object detection.\n"
	  "#",
	"motion_burst_count",  "400", FALSE, {.value = &pikrellcam.motion_burst_count},      config_value_int_set},

	{ "# The number of sustained frames for a burst motion detect.\n"
	  "#",
	"motion_burst_frames",  "3", FALSE, {.value = &pikrellcam.motion_burst_frames},      config_value_int_set},

	{ "# Time length limit of motion video record excluding pre_capture time.\n"
	  "# If zero, there is no time limit or else the minimum is 10 seconds.\n"
	  "#",
	"motion_record_time_limit",  "0", FALSE, {.value = &pikrellcam.motion_record_time_limit},      config_value_int_set},

	{ "# Percent to dim image when drawing motion vectors.  Range 30 - 60\n"
	  "#",
	"motion_vectors_dimming", "45", FALSE, {.value = &pikrellcam.motion_vectors_dimming}, config_value_int_set},


	{ "# Require a second motion detect within this period of seconds before\n"
	  "# triggering a real motion detect event.  Set to zero to not require a\n"
	  "# second confirming motion detect.\n"
	  "#",
	"motion_confirm_gap",   "4", TRUE, {.value = &pikrellcam.motion_times.confirm_gap},  config_value_int_set },

	{ "# event_gap seconds since the last motion detect event must pass\n"
	  "# before a motion video record can end.  Each motion detect within\n"
	  "# an event_gap resets a new full event_gap period.  When an event gap period\n"
	  "# does expire without a new motion event occurring, the video will end with\n"
	  "# an end time of the last motion detect time plus the post_capture time.\n"
	  "#",
	"motion_event_gap", "30", TRUE, {.value = &pikrellcam.motion_times.event_gap},    config_value_int_set },

	{ "# Seconds of video that will be recorded in a motion video before the\n"
	  "# time of the first motion event.\n"
	  "#",
	"motion_pre_capture",   "5", TRUE, {.value = &pikrellcam.motion_times.pre_capture},  config_value_int_set },

	{ "# Seconds of video that will be recorded after the last motion event.\n"
	  "# motion_post_caputure must be <= motion_event_gap.\n"
	  "#",
	"motion_post_capture",  "5", TRUE, {.value = &pikrellcam.motion_times.post_capture}, config_value_int_set },

	{ "# Command/script to run when a motion detect event begins.\n"
	  "#",
	"on_motion_begin",  "", TRUE, {.string = &pikrellcam.on_motion_begin_cmd}, config_string_set },

	{ "# Command/script to run when a motion detect event ends.\n"
	  "# The motion_end script uses scp to immediately archive motion detect\n"
	  "# videos to a different computer.\n"
	  "# To enable this, add your machine information to the motion-end script\n"
	  "# and make this the on_motion_end command:\n"
	  "#   on_motion_end $C/motion-end $v $P $G\n"
	  "#",
	"on_motion_end",    "", TRUE, {.string = &pikrellcam.on_motion_end_cmd}, config_string_set },

	{ "# When to save the motion preview file.\n"
	  "#     first  - when motion is first detected.\n"
	  "#              The on_motion_preview_save command runs immediately.\n"
	  "#     best   - best motion based on vector count and position.\n"
	  "#              The on_motion_preview_save command runs at motion end.\n"
	  "#",
	"motion_preview_save_mode", "best", FALSE, {.string = &pikrellcam.motion_preview_save_mode}, config_string_set },

	{ "# Command to run on the motion preview jpeg file.\n"
	  "# Specify the preview jpeg file name with $F.\n"
	  "# A jpeg thumb of the motion area is automatically extracted from the\n"
	  "# preview jpeg and saved to the thumbs directory.  However, if you\n"
	  "# want to do something else with the motion area, the motion detect\n"
	  "# area in the jpeg can be passed to this command with these substitution\n"
	  "# variables\n"
	  "#     $i  width of the area\n"
	  "#     $J  height of the area\n"
	  "#     $K  x coordinate of the area center\n"
	  "#     $Y  y coordinate of the area center\n"
	  "# Example command to email the motion detect preview jpeg:\n"
	  "#     on_motion_preview_save mpack -s pikrellcam@$H $F myuser@gmail.com\n"
	  "# Or, example command to run the default preview-save script which you\n"
	  "# can edit to enable emailing or copying the jpeg.\n"
	  "#     on_motion_preview_save $C/preview-save  $F $m $P $G\n"
	  "#",
	"on_motion_preview_save", "", TRUE, {.string = &pikrellcam.on_motion_preview_save_cmd}, config_string_set },

	{ "# Set to off if you want jpeg preview files to show the OSD overlay text\n"
	  "# or graphics.  This can be considered a debug or instructional option.\n"
	  "# Normally it is nicer to have clean preview jpegs and thumbs.\n"
	  "#",
	"motion_preview_clean",  "on", FALSE, {.value = &pikrellcam.motion_preview_clean}, config_value_bool_set },

	{ "# Minimum width and height in pixels for the substitution width and height\n"
	  "# variables for motion detect areas in the preview jpeg.\n"
	  "# This minimum helps with possible frame skew for smaller relatively\n"
	  "# faster moving objects.\n"
	  "#",
	"motion_area_min_side",  "60", FALSE, {.value = &pikrellcam.motion_area_min_side}, config_value_int_set },

	{ "# Enable writing a motion statistics .csv file for each motion video.\n"
	  "# For users who have a need for advanced video post processing.\n"
	  "#",
	"motion_stats",  "off", FALSE, {.value = &pikrellcam.motion_stats}, config_value_bool_set },


	{ "\n# --------------------- Video Record Options -----------------------\n"
	  "#\n"
	  "# Motion record video name format.\n"
	  "# PHP web page code depends on parsing this name format so there is\n"
	  "# very little flexibility for changing it.\n"
	  "# strftime() specifiers must not be changed except for possibly using %T.\n"
	  "# But only a recent gpac release can handle ':' in a video filename\n"
	  "# and you may not be able to use the %T specifier which is %H:%M:%S\n"
	  "# The %F specifier is the same as %Y-%m-%d and should not be changed.\n"
	  "# The final name must be of the form:\n"
	  "#   hhh_date_time_ttt.mp4\n"
	  "# where the hhh and ttt fields must not contain the '_' character.\n"
	  "# The format may use substitution variables in the hhh and ttt fields:\n"
	  "#     $N - The motion video sequence number\n"
	  "#     $H - The hostname\n"
	  "# \n"
	  "# A possible edit you can make to the default would be to add using the\n"
	  "# hostname variable or replace the $N sequence number with the hostname\n"
	  "# variable.  For example, if your hostname is rpi0, the current sequence\n"
	  "# number is 99, these formats produce these motion video file names:\n"
	  "#   motion_%F_%H.%M.%S_$N.mp4    => motion_2016-01-09_10.44.08_99.mp4\n"
	  "#   motion-$H_%F_%H.%M.%S_$N.mp4 => motion-rpi0_2016-01-09_10.44.08_99.mp4\n"
	  "#   motion_%F_%H.%M.%S_$H.mp4    => motion_2016-01-09_10.44.08_rpi0.mp4\n"
	  "#   $H_%F_%H.%M.%S_$N.mp4        => rpi0_2016-01-09_10.44.08_99.mp4\n"
	  "# or if ':' in names is supported by your version of gpac:\n"
	  "#   motion_%F_%T_$N.mp4          => motion_2016-01-09_10:44:08_99.mp4\n"
	  "# \n"
	  "# Unsupported option:\n"
	  "# By default video files are boxed into a .mp4 video but it is possible\n"
	  "# to leave the video in the Pi camera raw video .h264 format if the .mp4\n"
	  "# in the name format is replaced with .h264.\n"
	  "# But h264 videos cannot be played from the web page or programs\n"
	  "# like vlc and mplayer so changing to h264 should be done only if you\n"
	  "# have a way to manage the videos.  PiKrellCam webpages do not support\n"
	  "# or manage .h264 videos.\n"
	  "#",
	"video_motion_name_format", "motion_%F_%H.%M.%S_$N.mp4", TRUE,
		{.string = &pikrellcam.video_motion_name_format}, config_string_set },

	{ "# Manual record video name format.\n"
	  "# This format is similar to the video_motion_name_format except it has\n"
	  "# the added restriction that it must begin with \"man\" so that the\n"
	  "# web page can differentiate motion videos from manual videos.\n"
	  "# The final name must be of the form:\n"
	  "#   manhhh_date_time_ttt.mp4\n"
	  "# where the hhh and ttt fields must not contain the '_' character.\n"
	  "# The format may use substitution variables in the hhh and ttt fields:\n"
	  "#     $N - The manual video sequence number\n"
	  "#     $H - The hostname\n"
	  "# \n"
	  "# So some possibilities are:\n"
	  "#   manual_%F_%H.%M.%S_$N.mp4    => manual_2016-01-09_10.44.08_99.mp4\n"
	  "#   man-$H_%F_%H.%M.%S_$N.mp4    => man-rpi0_2016-01-09_10.44.08_99.mp4\n"
	  "#   manual_%F_%H.%M.%S_$H.mp4    => manual_2016-01-09_10.44.08_rpi0.mp4\n"
	  "#   man_%F_%H.%M.%S_$H-$N.mp4    => man_2016-01-09_10.44.08_rpi0-99.mp4\n"
	  "#",
	"video_manual_name_format", "manual_%F_%H.%M.%S_$N.mp4", TRUE,
		{.string = &pikrellcam.video_manual_name_format}, config_string_set },

	{ "# Pixel width of videos recorded.\n"
	  "#",
	"video_width",    "1920", TRUE, {.value = &pikrellcam.camera_config.video_width},      config_value_int_set },

	{ "# Pixel height of videos recorded.\n"
	  "#",
	"video_height",   "1080", TRUE, {.value = &pikrellcam.camera_config.video_height},     config_value_int_set },

	{ "# Video frames per second.  The processing required to implement the\n"
	  "# multiple video paths in PiKrellCam limits this fps to about 24.\n"
	  "# Above that may cause web page mjpeg frames to be dropped.  But if\n"
	  "# you are overclocking the GPU you may be able to set higher.\n"
	  "#",
	"video_fps",      "24", FALSE, {.value = &pikrellcam.camera_adjust.video_fps},        config_value_int_set },

	{ "# MP4Box output frames per second if video filename is a .mp4\n"
	  "# If this is different from video_fps, the final mp4 will be a\n"
	  "# slow or fast motion video.\n"
	  "#",
	"video_mp4box_fps", "24", FALSE, {.value = &pikrellcam.camera_adjust.video_mp4box_fps},        config_value_int_set },

	{ "# Video bitrate affects the quality and size of a video recording.\n"
	  "# Along with pre_capture and event_gap times, it also determines the\n"
	  "# PiKrellCam video circular buffer memory usage.\n"
	  "# Set the bitrate lower or higher as you wish to trade off video size\n"
	  "# and memory usage with video quality.  The Pi camera lens and sensor\n"
	  "# pixel size combination also can be a limiting factor on video quality\n"
	  "# and may limit the benefit of the highest bitrate settings.\n"
	  "#",
	"video_bitrate",  "6000000", TRUE, {.value = &pikrellcam.camera_adjust.video_bitrate},    config_value_int_set },

	{ "# Pixel width of the stream jpeg file. Aspect ratio is determined by the video.\n"
	  "#",
	"mjpeg_width",    "640",  TRUE, {.value = &pikrellcam.mjpeg_width},      config_value_int_set },

	{ "# Quality factor (up to 100) affects the quality and size of the stream jpeg.\n"
	  "# Set this lower if you need to reduce the stream bandwidth.\n"
	  "#",
	"mjpeg_quality",  "14",  TRUE, {.value = &pikrellcam.mjpeg_quality},    config_value_int_set },

	{ "# Divide the video_fps by this to get the stream jpeg file update rate.\n"
	  "# This will also be the motion frame check rate for motion detection.\n"
	  "# For example if video_fps is 24 and this divider is 4, the stream jpeg file\n"
	  "# is updated and motion is checked 6 times/sec.\n"
	  "#",
	"mjpeg_divider",  "4", FALSE, {.value = &pikrellcam.mjpeg_divider},    config_value_int_set },


	{ "\n# ------------------ Still Capture Options -----------------------\n"
	  "#\n"
	  "# Still file name format.\n"
	  "# This name is parsed by the PHP web page so restrictions are similar\n"
	  "# to the video_motion_name_format described above and must be of the form:\n"
	  "#   hhh_date_time_ttt.mp4\n"
	  "# where the hhh and ttt fields must not contain the '_' character.\n"
	  "#\n"
	  "# still_name_format may use substitution variables in hhh and ttt fields:\n"
	  "#     $N - The still capture sequence number\n"
	  "#     $H - The hostname\n"
	  "#\n"
	  "# Examples:\n"
	  "#   image_%F_%H.%M.%S_$N.jpg    => image_2016-01-09_10.44.08_99.jpg\n"
	  "#   im-$H_%F_%H.%M.%S_$N.jpg    => im-rpi0_2016-01-09_10.44.08_99.jpg\n"
	  "#   still_%F_%H.%M.%S_$H.jpg    => still_2016-01-09_10.44.08_rpi0.jpg\n"
	  "#",
	"still_name_format", "image_%F_%H.%M.%S_$N.jpg", TRUE, {.string = &pikrellcam.still_name_format}, config_string_set },

	{ "# Width of a still capture in pixels.  Max value 2592\n"
	  "#",
	"still_width",   "1920", TRUE, {.value = &pikrellcam.camera_config.still_width}, config_value_int_set },

	{ "# Height of a still capture in pixels.  Max value 1944\n"
	  "#",
	"still_height",  "1080", TRUE, {.value = &pikrellcam.camera_config.still_height}, config_value_int_set },

	{ "# This quality factor affects the size and quality of still captures.\n"
	  "#",
	"still_quality", "30", TRUE, {.value = &pikrellcam.camera_adjust.still_quality},   config_value_int_set },

	{ "# Command to run after a still capture.\n"
	  "# email the still somewhere with:\n"
	  "#   on_still_capture mpack -s pikrellcam@$H $s myuser@gmail.com\n"
	  "# Or do an email and/or something like copy or move the still somewhere\n"
	  "# with a script you write ($s is the still file pathname):\n"
	  "#   on_still_capture $C/still-capture $s $P $G\n"
	  "#",
	"on_still_capture", "", TRUE, {.string = &pikrellcam.on_still_capture_cmd}, config_string_set },

	{ "# Timelapse name format.\n"
	  "# PHP web page code depends on parsing this name format so there is\n"
	  "# very little flexibility for changing it.\n"
	  "# The name must begin with the characters \"tl\" and the strftime()\n"
	  "# specifiers should not be changed except for possibly using %T as\n"
	  "# described above.  The $n variable should not be changed and is:\n"
	  "#     $n - a timelapse id which defaults to the period in seconds.\n"
	  "# So about the only possible edit you can make here is to add in the\n"
	  "# hostname with a format like:\n"
	  "#     tl-$H_%F_%H.%M.%S_$n.mp4\n"
	  "# This video name is used in $T in the timelapse_convert command.\n"
	  "#",
	"video_timelapse_name_format", "tl_%F_%H.%M.%S_$n.mp4", TRUE, {.string = &pikrellcam.video_timelapse_name_format}, config_string_set },

	{ "# Command to run when a time lapse series is ended.\n"
	  "# The default timelapse_convert command is in the scripts-dist directory\n"
	  "# and converts captures in the media_dir/timelapse directory to a video\n"
	  "# which is saved in the media_dir/videos directory.\n"
	  "# If you want to use your own timelapse convert script, create the\n"
	  "# script in the scripts directory and set this timelapse_convert to\n"
	  "# use it (change the $c to $C and the script name to your script).\n"
	  "# NOTE: $l embeds a '%' qualifier in the command string so it\n"
	  "# can only be used as the last $X variable.\n"
	  "#",
	"timelapse_convert", "$c/_timelapse-convert $m $T $n $G $P $l", TRUE,
						{.string = &pikrellcam.timelapse_convert_cmd}, config_string_set },


	{ "\n# ------------------- Miscellaneous Options  -----------------------\n"
	  "#\n"
	  "# How long in seconds a notify string should stay on the stream jpeg file.\n"
	  "#",
	"notify_duration", "4", FALSE, {.value = &pikrellcam.notify_duration}, config_value_int_set },

	{ "# Your latitude used to calculate sun rise, set, dawn, and dusk times.\n"
	  "#",
	"latitude",  "30.12N", FALSE, {.string = &pikrellcam.latitude },   config_string_set },

	{ "# Your longitude used to calculate sun rise, set, dawn, and dusk times.\n"
	  "#",
	"longitude", "97.88W", FALSE, {.string = &pikrellcam.longitude },   config_string_set },

	{ "# Do not edit.  Used internally to force startup config write if needed.\n"
	  "#",
	"config_sequence", "0", FALSE, {.value = &pikrellcam.config_sequence}, config_value_int_set },


	{ "\n# ------------------- Annotate Text Options  -----------------------\n"
	  "#\n"
	  "# Format for a date string that can be superimposed on videos and stills.\n"
	  "# Custom strings from scripts can be prepended or appended to this date string\n"
	  "# using the annotate_string FIFO command.  See the Help web page.\n"
	  "#",
	"annotate_format_string",  "%a %b %e, %l:%M.%S %p", FALSE,
	                           {.string = &pikrellcam.annotate_format_string}, config_string_set },

	{ "# Enables drawing the annotate date string.\n"
	  "#",
	"annotate_enable",      "on", FALSE, {.value = &pikrellcam.annotate_enable},      config_value_bool_set },

	{ "# Extra information drawn on the video.\n"
	  "#",
	"annotate_show_motion", "off", FALSE, {.value = &pikrellcam.annotate_show_motion}, config_value_bool_set },

	{ "# Extra information drawn on the video.\n"
	  "#",
	"annotate_show_frame",  "off", FALSE, {.value = &pikrellcam.annotate_show_frame},  config_value_bool_set },

	{ "# Annotate text background color.  Set to \"none\" for no background.\n"
	  "# Otherwise, set to a hex rgb value, eg \"000000\" for black or \"808080\" for gray.\n"
	  "#",
	"annotate_text_background_color",    "none", FALSE, {.string = &pikrellcam.annotate_text_background_color },   config_string_set },

	{ "# Annotate text brightness. Range: integer from 0 - 255\n"
	  "# Text cannot be set to a color, only to a brightness..\n"
	  "#",
	"annotate_text_brightness",    "255", FALSE, {.value = &pikrellcam.annotate_text_brightness },   config_value_int_set },

	{ "# Annotate text size. Range: integer from 6 - 160\n"
	  "#",
	"annotate_text_size",    "32", FALSE, {.value = &pikrellcam.annotate_text_size },   config_value_int_set },
	};

#define CONFIG_SIZE (sizeof(config) / sizeof(Config))


boolean
config_set_option(char *opt, char *arg, boolean set_safe)
	{
	Config	*cfg;
	boolean	result = FALSE;

	if (!opt || !*opt || !arg || !*arg)
		return FALSE;

	for (cfg = &config[0]; cfg < &config[CONFIG_SIZE]; ++cfg)
		{
		if (   (cfg->safe && !set_safe)
		    || strcmp(opt, cfg->option)
		   )
			continue;

		dup_string(&cfg->arg, arg);
		result = (*cfg->config_func)(arg, &cfg->result);	/* Allocates new storage for strings */

		if (pikrellcam.verbose && !set_safe)
			printf("config_set_option: %s %s\n", cfg->option, arg);
		break;
		}
	return result;
	}

void
config_set_defaults(void)
	{
	CameraParameter	*param;
	Config			*cfg;
	char			*home_dir;
	boolean			valid;

	/* Camera parameter table and pikrellcam config table have initial value pointers
	|  to static storage.  Replace these pointers to an allocated copy of the
	|  initial value so that when later config changes are made the changes
	|  can be dup_string() replaced into the pointers.
	*/
	for (param = &camera_parameters[0];
				param < &camera_parameters[CAMERA_PARAMETERS_SIZE]; ++param)
		param->arg = strdup(param->arg);

	for (cfg = &config[0]; cfg < &config[CONFIG_SIZE]; ++cfg)
		{
		cfg->arg = strdup(cfg->arg);
		valid = (*cfg->config_func)(cfg->arg, &cfg->result);
		if (!valid)
			printf("config_set_default%s: %s %s\n", valid ? "" : " FAIL",
							cfg->option, cfg->arg);
		}

	pikrellcam.version = strdup(PIKRELLCAM_VERSION);
	pikrellcam.timelapse_format = strdup("tl_$n_$N.jpg");
	pikrellcam.preview_filename = strdup("");
	gethostname(pikrellcam.hostname, HOST_NAME_MAX);	

	/* If pikrellcam started by rc.local or web page, need to get correct
	|  home directory.  Makefile does a setuid/setgid on executable.
	*/
	home_dir = getpwuid(geteuid())->pw_dir;
	asprintf(&pikrellcam.config_dir, "%s/%s", home_dir, PIKRELLCAM_CONFIG_DIR);

	if (make_directory(pikrellcam.config_dir))
		{
		asprintf(&pikrellcam.config_file, "%s/%s",
					pikrellcam.config_dir, PIKRELLCAM_CONFIG);
		asprintf(&pikrellcam.motion_regions_config_file, "%s/%s",
					pikrellcam.config_dir, PIKRELLCAM_MOTION_REGIONS_CONFIG);
		asprintf(&pikrellcam.at_commands_config_file, "%s/%s",
					pikrellcam.config_dir, PIKRELLCAM_AT_COMMANDS_CONFIG);
		asprintf(&pikrellcam.timelapse_status_file, "%s/%s",
					pikrellcam.config_dir, PIKRELLCAM_TIMELAPSE_STATUS);
		}

	/* Make sure some motion regions exist.  These will be replaced if there
	|  is a motion regions config file
	*/
	motion_command("add_region 0.042 0.159 0.224 0.756");
	motion_command("add_region 0.266 0.159 0.233 0.756");
	motion_command("add_region 0.500 0.150 0.233 0.750");
	motion_command("add_region 0.734 0.156 0.224 0.753");
	motion_frame.show_regions = FALSE;
	}

boolean
config_load(char *config_file)
	{
	FILE	*f;
	char	linebuf[128], opt[64], args[128];
	int		n;
	char	*s;

	if ((f = fopen(config_file, "r")) == NULL)
		return FALSE;

	pikrellcam.config_sequence_new = 15;

	while (fgets(linebuf, sizeof(linebuf), f))
		{
		n = sscanf(linebuf, "%63s %[^\n]", opt, args);
		s = args + strlen(args) - 1;
		if (n == 2)
			while (*s == ' ' || *s == '\t' || *s == '\r')
				*s-- = '\0';
		if (n < 1 || opt[0] == '#')
			continue;
		if (   n != 2
		    || (   !config_set_option(opt, args, TRUE)
		        && !mmalcam_config_parameter_set(opt, args, FALSE)
		       )
		   )
			printf("Bad config file option: %s\n", linebuf);
		}
	fclose(f);

	if (pikrellcam.motion_magnitude_limit < 3)
		pikrellcam.motion_magnitude_limit = 3;
	if (pikrellcam.motion_magnitude_limit_count < 2)
		pikrellcam.motion_magnitude_limit_count = 2;

	if (pikrellcam.motion_burst_count < 20)
		pikrellcam.motion_burst_count = 20;
	if (pikrellcam.motion_burst_frames < 2)
		pikrellcam.motion_burst_frames = 2;

	if (   pikrellcam.motion_record_time_limit != 0
	    && pikrellcam.motion_record_time_limit < 10
	   )
		pikrellcam.motion_record_time_limit = 10;

	if (pikrellcam.motion_vectors_dimming < 30)
		pikrellcam.motion_vectors_dimming = 30;
	if (pikrellcam.motion_vectors_dimming > 60)
		pikrellcam.motion_vectors_dimming = 60;

	if (pikrellcam.motion_times.post_capture > pikrellcam.motion_times.event_gap)
		pikrellcam.motion_times.event_gap = pikrellcam.motion_times.post_capture;

	if (pikrellcam.annotate_text_size < 6)
		pikrellcam.annotate_text_size = 6;
	else if (pikrellcam.annotate_text_size > 160)
		pikrellcam.annotate_text_size = 160;
	if (pikrellcam.annotate_text_brightness < 0)
		pikrellcam.annotate_text_size = 0;
	else if (pikrellcam.annotate_text_brightness > 255)
		pikrellcam.annotate_text_size = 255;

	pikrellcam.annotate_string_space_char = '_';

	camera_adjust_temp = pikrellcam.camera_adjust;
	motion_times_temp = pikrellcam.motion_times;

	if (pikrellcam.config_sequence_new != pikrellcam.config_sequence)
		{
		pikrellcam.config_sequence = pikrellcam.config_sequence_new;
		pikrellcam.config_modified = TRUE;
		}
	return TRUE;
	}

void
config_save(char *config_file)
	{
	FILE			*f;
	Config			*cfg;
	CameraParameter	*param;

	if (!strcmp(config_file, "-"))
		f = stdout;
	else
		f = fopen(config_file, "w");
	if (!f)
		{
		printf("Failed to save pikrellcam config file: %s.  %m\n", config_file);
		return;
		}


	fprintf(f,
"# ==================================================================\n"
"#               pikrellcam Program Configuration\n"
"#\n"
"# When editing this file, stop and then restart pikrellcam so your changes\n"
"# will be in effect.  Otherwise, pikrellcam may overwrite this file and your\n"
"# changes will be lost.\n"
"# Commands in this file should not be enclosed in quotes and commands may\n"
"# have substitution variables.\n"
"# See ~/.pikrellcam/at_commands.conf for a list of the substitution variables.\n"
"# The web interface can modify many settings here but caonnot modify any\n"
"# commands or filename templates.  If a setting is modified from the web\n"
"# interface, this file will be written.\n"
"#\n"
		);
	for (cfg = &config[0]; cfg < &config[CONFIG_SIZE]; ++cfg)
		{
		fprintf(f, "%s\n", cfg->description);
		if (cfg->config_func == config_value_int_set)
			fprintf(f, "%s %d\n\n", cfg->option, *(cfg->result.value));
		else if (cfg->config_func == config_value_bool_set)
			fprintf(f, "%s %s\n\n", cfg->option, *(cfg->result.value) ? "on" : "off");
		else
			fprintf(f, "%s%s %s\n\n",
					(*cfg->arg == '\0') ? "#" : "",	/* comment out if no arg */
					cfg->option, cfg->arg);
		}

	fprintf(f,
	    "# ==================================================================\n");
	fprintf(f,
		"#               Raspberry Pi Initial Camera Parameters\n");
	fprintf(f, "#\n");
	for (param = &camera_parameters[0];
				param < &camera_parameters[CAMERA_PARAMETERS_SIZE]; ++param)
		{
		fprintf(f, "%s %s\n", param->name, param->arg);
		}
	fclose(f);
	pikrellcam.config_modified = FALSE;
	}


void
config_timelapse_save_status(void)
	{
	FILE	*f;
	
	f = fopen(pikrellcam.timelapse_status_file, "w");
	if (f)
		{
		fprintf(f, "period activated daylight series sequence\n");
		fprintf(f, "%d %d %d %d %d\n",
			time_lapse.period, time_lapse.activated, time_lapse.on_hold,
			time_lapse.series, time_lapse.sequence);
		fclose(f);
		}
	}

void
config_timelapse_load_status(void)
	{
	FILE	*f;
	int		n;
	char	buf[100];

	f = fopen(pikrellcam.timelapse_status_file, "r");
	if (f)
		{
		fgets(buf, sizeof(buf), f);
		fgets(buf, sizeof(buf), f);
		n = sscanf(buf, "%d %d %d %d %d\n",
			&time_lapse.period, &time_lapse.activated, &time_lapse.on_hold,
			&time_lapse.series, &time_lapse.sequence);
		fclose(f);
		if (n == 5 && time_lapse.activated)
			time_lapse.event = event_add("timelapse",
						pikrellcam.t_now, time_lapse.period,
						timelapse_capture, NULL);
		}
	if (!f || time_lapse.period < 1)
		{
		time_lapse.period = 60;
		config_timelapse_save_status();
		}
	}

