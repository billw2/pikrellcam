<?php
	define("PROGRAM", "PiKrellCam");
	define("HOST", php_uname("n"));
  	define("TITLE_STRING", PROGRAM . "@" . HOST);

// The 'media' directory in these defines is a link to the media_dir
// which is configured in ~/.pikrellcam/pikrellcam.conf and the link is
// automatically altered by the startup script if media_dir is changed.
// The videos, stills and timelapse subdirs are fixed to match PiKrellCam.
// Do not change these here.
//
	define("VIDEO_DIR", "media/videos");
	define("STILL_DIR", "media/stills");
	define("TIMELAPSE_DIR", "media/timelapse");

// The mjpeg file can be changed by editing ~/.pikrellcam/pikrellcam.conf
// The others are fixed by the install and enforced by the startup script.
// It is no use to change these here.
//
	define("LOG_FILE", "/tmp/pikrellcam.log");
	define("MJPEG_FILE", "/run/pikrellcam/mjpeg.jpg");
	define("PIKRELLCAM", "/home/pi/pikrellcam/pikrellcam");
	define("FIFO_FILE", "/home/pi/pikrellcam/www/FIFO");

	define("VERSION", "1.0");
?>
