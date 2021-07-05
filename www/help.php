<style>
pre
	{
	font-family: monospace, "courier new", courier, serif;
	font-size: 0.9em;
	margin-left: 1cm;
    border-radius: 4px;
    color: #004668;
    padding: 4px 8px 4px 8px;
    border: solid #1f628d 1px;

	background: #9aa1a2;
	}

p
	{
	margin-left: 0cm;
	}

.indent0
	{
	margin: 0.1cm 0cm 0.8cm 0.4cm;
	}

.indent1
	{
	margin: 0.1cm 0cm 0.1cm 1.0cm;
	}


</style>

<?php
require_once(dirname(__FILE__) . '/config.php');
include_once(dirname(__FILE__) . '/config-user.php');
include_once(dirname(__FILE__) . '/config-defaults.php');	

ini_set('display_errors',1);
ini_set('display_startup_errors',1);
error_reporting(-1);

$title = TITLE_STRING;
$header = "<!DOCTYPE html><html><head>";
$header .= "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
$header .= "<title>$title Help</title>";
$header .= "<link rel=\"stylesheet\" href=\"js-css/pikrellcam.css\" />";
$header .= "</head>";
$header .= "<body background=\"$background_image\">";
$header .= "<div><div class='text-center'>";
$header .= "<div><a class='text-shadow-large'
			style='text-decoration: none;' href='index.php'>$title</a></div></div></div>";
echo $header;

//echo "<div style='color: $default_text_color; margin-left: 1cm; margin-right: 2cm'>";
// Browser back button does not work in a scroll.
//$div_style = "color: $default_text_color; overflow-y: scroll; height:${n_log_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
$div_style = "color: $default_text_color";
echo "<div style='$div_style'>";
echo "<div style='color: $default_text_color; margin-left: 0.5cm; margin-right: 0.5cm;'>";
?>

<span style='font-size: 1.5em; font-weight: 650;'>Introduction</span><hr>
<div class='indent0'>
For an overview description of PiKrellCam, visit the
<a href="http://billw2.github.io/pikrellcam/pikrellcam.html">PiKrellCam website</a><br>
And there is a Raspberry Pi
<a href="https://www.raspberrypi.org/forums/viewtopic.php?f=43&t=115583">PiKrellCam forum</a>
</div>

<span style='font-size: 1.5em; font-weight: 650;'>Recent Release Notes</span><hr>
<div class='indent0'>

Version 4.3.2
<div class='indent1'>
Detect camera to determine resolution options shown in GUI and added
resolutions for HQ camera.<br>
Setup->Preset->Settings-><a href="help.php#ZOOM_PERCENT">Zoom_Percent.</a><br>
Setup->Config->Settings-><a href="help.php#STALL_WARNING">Preview_Stall_Warning.</a>
</div>
<br>
Version 4.3.1 - video_mp4box_fps can be fractional. For audio/video drift
tuning.
<br>

Version 4.3.0
<div class='indent1'>
Motion events can record stills instead of videos. See
<a href="help.php#MEDIA_TYPES">Media Types</a> section and config
options in
<a href="help.php#CONFIG_MOTION">Config->Motion.</a><br>
Removed motion_preview_save_mode. Preview/thumb now always saved as "first".
</div>

<br>
Version 4.2.1 - Changes for Pi 4 peripheral base and Buster installs.
<br>

Version 4.2.0
<div class='indent1'>
<a href="help.php#MOTION_EVENTS_FIFO">motion_detects_FIFO</a> can be read
to get all motion detects regardless of motion videos enabled state.<br>
Moved Setup->Config->Times/* and Setup->Settings->Startup_Motion to Setup->Config->Motion.
</div>

Version 4.1.5
<div class='indent1'>
Archive directory <a href="help.php#ARCHIVING">NFS mount examples.</a><br>
Archive Calendar can view by year.<br>
Updated scripts-dist/startup script has NFS archive mounting.
</div>

Version 4.1.4
<div class='indent1'>
Bug fix for pikrellcam Start from web page: use installing user instead
of hard wired "pi".<br>
New scripts-dist/example-motion-events demo of processing
<nobr>/run/pikrellcam/motion-events.</nobr><br>
The <a href="help.php#MOTION_TRIGGER">motion trigger</a> FIFO command can
encode for user defined trigger type (PIR, laser, etc).
Previously web page showed "Extern" for videos with only a FIFO trigger, but
now shows either "FIFO" or the user defined trigger type string.
</div>

Version 4.1.3
<div class='indent1'>
An <a href="help.php#AUDIO_TRIGGER">audio trigger</a>
event can start a motion video record.<br>
This help section describes <a href="help.php#MEDIA_TYPES">Media Types</a><br>
Fix 2 channel audio recording bug.
</div>

Version 4.1.2 - Fix record to end of event_gap bug. Add day_loop arg to archive_video command.
<br>
Version 4.1.0
<div class='indent1'>
<a href="help.php#LOOP">Loop recording</a> with percent
<a href="help.php#DISKUSAGE">diskusage limit</a>
checking to auto delete oldest loop videos.<br>
Disk free percent checking for stills/timelapse.<br>
pikrellcam.conf: on_manual_end on_loop_end on_motion_enable.
</div>

Version 4.0.5
<div class='indent1'>
<a href="help.php#DISKFREE">Disk free limit for media videos</a><br>
Stills have a thumbs view.
</div>

</div>


<span style='font-size: 1.5em; font-weight: 650;'>Install</span><hr>
<div class='indent0'>
PiKrellCam is installed from a github git repository using the command line. The install
is cloning the repository in the /home/pi directory and running the install script.
An install for a user other than pi should be possible.  The user needs sudo permission at
least for commands chown, chmod, mkdir and should be in groups audio and video
(and gpio if using servos).
<pre>
cd /home/pi
git clone https://github.com/billw2/pikrellcam.git
cd pikrellcam
./install-pikrellcam.sh
</pre>
The install-pikrellcam.sh script installs needed packages and prompts for
three things to configure:
	<ul>
	<li>Port number for the nginx web server
	to listen on.  The default is 80, but an alternate non standard port number can
	be used in which case the PiKrellCam web page would be accessed with the URL:<br>
	&nbsp; &nbsp; &nbsp;<span style='font-weight:700'>http://your_pi:port_number</span>
	</li>
	<p>
	<li>Auto start: if enabled a line will be added to
	<span style='font-weight:700'>/etc/rc.local</span> so that pikrellcam will be
	auto started at boot.  If this is not enabled, pikrellcam will need to be started from
	the web page or from a terminal after each boot.
	</li>
	<p>
	<li>Password protection: if set a login will be required to access the
	PiKrellCam web pages.
	</li>
	</ul>
Any time you want to change how you configured these three configurations,
just rerun the install script.
<p>
<span style='font-size: 1.2em; font-weight: 680;'>First Usage</span>
<div class='indent1'>
Go to the PiKrellCam web page in your browser (omit the port number if it was left at default 80):<br>
&nbsp; &nbsp; &nbsp;<span style='font-weight:700'>http://your_pi:port_number</span>
	<ul>
	<li>Expand the
		<span style='font-weight:700'>System</span> panel and start the PiKrellCam prgram
		by clicking the button
		<span class='btn-control'>Start</span>
		<br>After two or three seconds (startup can be slower on a Pi 1),
		the preview image from the camera should appear.
		If it does not you should get in its place an error image indicating that the
		camera could not be started.  This can happen if the camera is busy (another program
		is using it) or if there is a problem with the ribbon cable camera connection.
		If this happens, you should fix the issue with the camera and restart PiKrellCam.
	</li>
	<p>
	<li> After the preview image appears, turn on motion detection by clicking the button
		<span style='font-weight:700'>Enable:</span> <span class='btn-control'>Motion</span>
	</li>
	<p>
	<li>The OSD then shows that motion detection is
		<span style='font-weight:700'>ON</span>
		and PiKrellCam is now operating with its default settings.
	</li>
	<p>
	<li>Wait for motion to be detected and watch the OSD for the video record progress.<br>
		After the video ends, view it by going to the Videos page by clicking:
		<span style='font-weight:700'>Media:</span> <span class='btn-control'>Videos</span>
	</li>
	<p>
	<li>On the button bar, click the buttons
		<span style='font-weight:700'>Show:</span>
		<span class='btn-control'>Preset</span>
		<span class='btn-control'>Timelapse</span>
		<span class='btn-control'>Vectors</span>
		<br>to toggle showing information PiKrellCam can display on the OSD.
		When you show Preset, you see the currently configured motion detection vector
		and burst values and the motion detect regions in effect.  See below.
	</li>
	<p>
	<li>A basic first configuration to consider is enabling motion detection to be turned on
		each time PiKrellCam is started.  To do this, use the OSD menu system:<br>
		<ul>
			<li>
			Expand the <span style='font-weight:700'>Setup</span> panel.
			</li>
			<li>In the
			<span style='font-weight:700'>Config</span> group,
			click the button <span class='btn-control'>Settings</span>
			</li>
			<li>The OSD will show a horizontal menu with
			<span style='font-weight:700'>Startup_Motion</span> highlighted (underlined).
			</li>
			<li>Click the button: <span class='btn-control'>Sel</span>
			</li>
			<li>Turn the option ON by clicking
			<input type="image" src="images/arrow-right.png">
			</li>
			<li>Finalize the startup motion new option setting by clicking:
			<span class='btn-control'>Sel</span>
			</li>
			<li>Back out of the menu by clicking
			<span class='btn-control'>Back</span>
			</li>
		</ul>
	</li>
</div>
</div>




<span style='font-size: 1.5em; font-weight:650'>Upgrades</span><hr>
<div class='indent0'>
	Expand the front page
	<span style='font-weight:700'>System</span> panel and click the
	<span <span class='btn-control'>Upgrade</span> button.<br>
	After an upgrade to a new version PiKrellCam should be stopped
	and restarted from the
	<span style='font-weight:700'>System</span> panel and the web pages
	should be reloaded to pick up any possible web page changes.
</div>





<span style='font-size: 1.5em; font-weight: 650;'>Architecture</span><hr>
<div class='indent0'>
<img src="images/architecture.jpg" alt="architecture.jpg"> 
<div class='indent1' style='margin-top:8px;'>
	Architecture notes:
	<ul>
	<li>
	<span style='font-weight:700'>Pi GPU</span> is the Pi video processor and its
	components (encoders and resizer) run as threads separate from the
	<span style='font-weight:700'>PiKrellCam</span> program which runs on the Pi Arm
	microprocessor.<br>
	</li>
	<p>
	<li>PiKrellCam runs as user pi for simplified install and other benefits
	(but a default user other than pi is possible).
	PHP files are run via the web server nginx and so run as the user that nginx workers use.
	The install script created a <nobr><span style='font-weight:700'>/etc/sudoers.d/pikrellcam</span></nobr>
	file to give the user that nginx workers use permission to start pikrellcam as user pi from the web page.
	If the option to run pikrellcam at boot was selected, a line to start pikrellcam as user pi
	was added to <nobr><span style='font-weight:700'>/etc/rc.local</span></nobr>
	</li>
	<p>
	<li>The install script creates
	<span style='font-weight:700'>/etc/nginx/sites-available/pikrellcam</span> where listen port
	number and password protection for the web pages are configured.
	</li>
	<p>
	<li>Several PiKrellCam internal control paths are not shown in the above diagram.
	The "command processing"
	block can control the video record, still, time lapse and other sub systems.
	</li>
	<p>
	<li>A simple feed of the preview jpeg can be viewed without control panels or button/control bars.<br>
	Just point your browser to:
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://your_pi:port_num/live.php</span>
	<br>or
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://pi:password@your_pi:port_num/live.php</span>
	<br>where port_num is the nginx port configured in the install.  If the port was left at
	the default 80, you can omit the <span style='font-weight:700'>:port_num</span> from the URL.
	</li>
	<p>
	<li>An alternate way to view the preview is with a tcp stream connection which additionally
	allows viewing using vlc or Android MJPEG viewer apps such as tinycam monitor, etc.
	Open the MJPEG network stream using the URL:
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://pi:password@your_pi:port_num/mjpeg_stream.php</span>
	</li>
	<p>
	<li>To be able to view the h264 tcp video stream some extra install and
	setup are required.  Follow the
	<nobr><a href="https://www.raspberrypi.org/forums/viewtopic.php?p=862399#p862399">
	rtsp live video setup instructions</a></nobr> on the forum and you can view the stream
	with vlc.  See also
	<nobr><a href="https://www.raspberrypi.org/forums/viewtopic.php?f=43&t=115583&start=1200#p1239346">.
	</li>
	</ul>
</div>
</div>

<a name="MEDIA_TYPES">
<span style='font-size: 1.5em; font-weight: 650;'>Media Types</span><hr>
<div class='indent0'>
<p>
<nobr><span style=\"color: $default_text_color\"> Media:</span>
<span class='btn-control'>Videos</span></nobr>
<div class='indent1'>
These are manual or motion videos of variable length with the recording triggered by some
event.  Motion videos have a length defined by the configured
<a href="help.php#CONFIG_MOTION">motion times</a>.
and can have on_motion_begin, on_preview_save and on_motion_end
commands configured in
<span style='font-weight:700'>~/.pikrellcam/pikrellcam.conf</span>
run for the motion event.
<ul>
	<li>
		<span style='font-weight:700'>Manual</span> - videos are triggered by a
		<span style='font-weight:700'>record on|off</span>
		command sent into the command FIFO (by script or clicking the web page
		video record button).
		The web page thumb will have a "Manual" label.
	</li>
	<li>
		<span style='font-weight:700'>Motion</span> - videos have
		motion vector direction or burst detects.
		The web page thumb will have no extra label and the thumb image will be
		an image of the area of a motion detect.
	</li>
	<li>
		<span style='font-weight:700'>FIFO</span> - videos are motion videos triggered by a
		<span style='font-weight:700'>motion trigger</span>
		command sent into the command FIFO and will have no motion
		vector or burst detects.  The web page
		thumb will have a "FIFO" or "code" label where "code" is the optional
		user supplied code string (for example PIR or laser) in the
		<nobr><span style='font-weight:700'>motion trigger</span></nobr>
		command.
	</li>
	<li>
		<span style='font-weight:700'>Audio</span> - videos are motion videos triggered by
		an audio trigger and will have no motion vector/burst detects or FIFO
		command triggers.  The web page thumb will have an "Audio" label.
	</li>
</ul>
</div>
<p>
<nobr><span style=\"color: $default_text_color\"> Media:</span>
<span class='btn-control'>Stills</span></nobr>
<div class='indent1'>
Stills are single image snapshots.  Motion
stills instead of videos are taken if
<span style='font-weight:700'>Motion_Stills_(no_videos)</span>
is enabled.  If recording motion stills, a record event defined by the
configured motion times will have one or more motion stills taken.
However, only the motion times confirm_gap and event_gap are applied for
a motion still record event and pre_capture, post_capture and video_time_limit
are not used.  Also any custom pre_capture or time_limit in a
<span style='font-weight:700'>motion trigger</span>
command will not be used.
The motion commands on_motion_begin, on_motion_preview_save, and
on_motion_end will be run for the record event.

A motion detect area from the video stream has to clear the preview/thumb
jpg conversion and then the camera has to switch to capture mode, so the
capture of a motion still image will be slightly delayed from its thumb image.

<ul>
	<li>
		<span style='font-weight:700'>Manual</span> - stills are triggered by a
		<span style='font-weight:700'>still</span>
		command sent into the command FIFO (by script or clicking the web page
		still snapshot button) and have no web page thumb extra label.
	</li>
	<li>
		<span style='font-weight:700'>Motion</span> - stills are triggered by a
		motion vector or burst detect.
		The web page thumb will have a "Motion" label and a motion record
		event sequence number that is reset to 1 each day.
	</li>
	<li>
		<span style='font-weight:700'>FIFO</span> - stills are motion stills triggered by a
		<span style='font-weight:700'>motion trigger</span>
		command sent into the command FIFO. The web page
		thumb will have a "FIFO" or "code" label where "code" is the optional
		user supplied code string (for example PIR or laser) in the
		<nobr><span style='font-weight:700'>motion trigger</span></nobr>
		command.
	</li>
	<li>
		<span style='font-weight:700'>Audio</span> - stills are motion stills triggered by
		an audio detect trigger and will have an "Audio" label on the web page thumb.
	</li>
</ul>
</div>
<p>
<nobr><span style=\"color: $default_text_color\"> Media:</span>
<span class='btn-control'>Loop</span></nobr>
<div class='indent1'>
These are continuously recorded videos of a fixed configurable length
(configured motion times do not apply).
If motion is enabled while loop recording, any motion, FIFO or audio trigger
event will cause the video to be tagged as having a motion event and the web
page thumb will be labeled to show that.  When that happens, any user
configured motion begin/end event commands will also be run.
<ul>
	<li>
	Loop videos with no motion detect will end with
	<span style='font-weight:700'>_0.mp4</span>.
	</li>
	<li>
	Loop videos with a motion vector or burst detect will end with
	<span style='font-weight:700'>_m.mp4</span> (previous versions used
	 _1.mp4) and will have a "Motion" label on the web page thumb.
	The thumb image will be a motion detect area.
	</li>
	<li>
	Loop videos with motion externally triggered (a
	<span style='font-weight:700'>motion trigger</span>
	command was sent into the FIFO) and no motion vector/burst detects
	will end with
	<span style='font-weight:700'>_e-ID.mp4</span> and will have a "ID" label
	on the web page thumb where "ID" is "FIFO" or the user defined ID
	string in the
	<span style='font-weight:700'>motion trigger</span>
	command. The thumb image will be of the full preview image.
	</li>
	<li>
	Loop videos with an audio trigger and no motion vector detects or FIFO
	triggers will end with
	<span style='font-weight:700'>_a.mp4</span> and will have an "Audio" label
	on the web page thumb. The thumb image will be of the full preview image.
	</li>
</ul>
</div>
Configuration of motion record events is in
<a href="#CONFIG_MOTION">Config->Motion.</a>

</div>


<span style='font-size: 1.5em; font-weight: 650;'>Motion Vectors</span><hr>
<div class='indent0'>
Interleaved with the h264 video frames from the Pi camera are motion vector arrays which
have data on pixel blocks which move from frame to frame in the video.  Each motion vector
has movement data for one 16x16 pixel block from the video frame.  So for a 1080p video,
for example, the vector array will be 120x68.  PiKrellCam analyzes the vector array in
multiple passes to build composite vectors for motion regions and a composite vector for
the total frame.  Motion detecting is comparing the resulting composite vector magnitudes
and counts against configured limits.  PiKrellCam uses two methods in parallel to detect
motion:

		<div class='indent1'>
		<p>
		<span style='font-weight:700'>Motion Vector Direction Detects</span><br>
		After the motion region composite vectors are calculated, the component motion vectors are
		filtered for direction and density.  This provides high sensitivity and noise
		immunity for small to medium sized objects.  Motion detection is tested independently for
		each motion region.
		To adjust direction detects, set the
		Vector_Magnitude and Vector_Count values using the OSD.  The values should be set
		lower in environments where small animal detection is desired or they can be
		adjusted higher for moderate sized object detection.  The combination of direction
		filtering and density checks make this detection method resistant to
		camera noise and is why it is good for small object detection.
		However, when PiKrellCam detects
		camera sparkle noise and the configured Vector_Count is low, the Vector_Count
		does get a small dynamic adjustment higher to provide a noise safety margin.
		<p>
		<span style='font-weight:700'>Motion Vector Burst Detects</span><br>
		For large (or close) objects,
		the individual camera motion vectors can have large direction distributions.
		This is because individual vectors
		in larger areas can begin to match pixels in
		directions other than the overall object direction and direction detection can
		sometimes miss these events.
		So instead of using direction filtering, burst detection looks for a sudden
		large increase in the number of motion vectors sustained over a number of frames.
		This method uses the same region composite vectors as direction
		detection, but it does not exclude vectors failing the direction test.
		All region composite vectors are combined into an overall frame
		composite vector. Motion burst detection is to compare the frame composite vector
		count to the Burst_Count limit and to require at least one motion region
		to pass a density test which is used to help reduce false detects of
		the large noise counts the camera can generate in certain dim light situations.
		The burst detect configurations Burst_Count and Burst_Frames are a
		coarse adjustment for detection of relatively larger and faster moving
		objects and they should be set according to what the noise environment
		of the camera allows. For outdoor cameras this noise period occurs in the
		minutes before sunrise and after sunset.  For indoor cameras any noise
		periods will depend on lighting conditions.  Since burst detection does not
		have the benefit of direction filtering to reduce noise, an
		exponential moving average of background noise counts is calculated and
		used to provide additional noise margin.
		The compared to Burst_Count is dynamically adjusted higher by this average. 
		</div>
<p>
You can see the results of PiKrellCam's vector processing on the OSD by turning on
the showing of Preset and Vectors.  Watching this display will allow you to tune
your configured vector limit values to your camera environment.  To
get a better look at the vectors, you can temporarily raise the mjpeg_divider
value so the OSD will update more slowly.  
<p>
<span style='font-size: 1.2em; font-weight: 680;'>Example Motion Detects</span>
<div style='margin-left: 1.5cm;'>
<img src="images/vector0.jpg" alt="vector0.jpg"> 
</div>
<p>
<span style='font-size: 1.2em; font-weight: 680;'>Notes:</span>
<ul>
	<li>Image 2 fails the overall motion detect because PiKrellCam tries to be
		discriminating against noise.  This is how it can run with very low magnitude
		and count limits and be good at detecting small animals.  The low density
		distribution of vectors in region 2 that caused the failure can be typical of
		scenes with wind blown trees and grass.
	</li>
	<p>
	<li>Image 4 shows a burst detect overriding a direction detect failure.
		The point to take from this is that if the burst count limit is set
		too low for a given camera environment, the burst detect method is likely
		to detect more noise than the direction detect method.
	</li>
	<p>
	<li>Sparkles are camera motion vectors that have no neighbors and PiKrellCam
		considers them noise and excludes them from the composite vectors.
	</li>
	<p>
	<li>The vector count status line will be shown if you set
		Setup->Config->Settings->Vector_Counts on and Show: Preset.
		<br>
		Interpreting the vector count status line:
		<ul>
			<li><span style='font-weight:700'>any:47 (17.1)</span> This shows there was
			a frame total of 47 vectors excluding sparkles passing the magnitude limit test.
			So all of these vectors were clustered in a group of at least 2.  The 17.1 count
			is an exponential moving average of the <span style='font-weight:700'>any</span>
			count EXCLUDING any counts from a region when they were greater than the
			limit count.  So the moving average measures background noise over sparkles.
			A burst count must exceed the average by the Burst_Count limit for a burst detect.
			</li>
			<p>
			<li><span style='font-weight:700'>rej:22</span> There were 22 total frame vectors
			that passed the magnitude limit count but failed the direction compare test to
			their region composite vector.  They were excluded from any region final motion
			direction detect test.
			They were not excluded from the burst detect count test.
			</li>
			<p>
			<li><span style='font-weight:700'>spkl:52 (53.5)</span> there were 52 vectors
			that passed the magnitude test but did not have any neighbors.  So they were
			ecluded from any motion detect test.  The sparkle moving average (53.5) is
			currently not used in motion detect testing.
			</li>
			<p>
			<li><span style='font-weight:700'>confirming[4]:12</span> there was a direction
			detect but a positive confirm gap is configured so there is a count down in progress
			waiting for a confirm motion detect.  The <span style='font-weight:700'>:12</span> shows
			the current vector count being tested for a composite vector.  If there was no
			countdown in progress, the line would have shown
			<span style='font-weight:700'>counts:12</span>.  The counts on this line show
			when vector counts are greater than the limit count and a region composite vector
			is being tested for motion. 
			</li>
		</ul>
	</li>
	<p>
	<li>Most medium to large sized motion events can be detected by both the direction
		and burst methods.
		To help evaluate the effect of changing configuration values,
		look in the log file to find out which detect method (one or both) happened for
		a particular motion video.
	</li>
</ul>
</div>

<span style='font-size: 1.5em; font-weight: 650;'>Servos</span><hr>
<div class='indent0'>
PiKrellCam has built in servo control for cameras mounted on servos.
<ul>
	<li><span style='font-weight:700'>Hardware PWM</span>: If servos are connected to
	the hardware PWM GPIO pins, PiKrellCam can directly control the PWM signals and the
	only configuration needed is to set in pikrellcam.conf
	<span style='font-weight:700'>servo_pan_gpio</span> and
	<span style='font-weight:700'>servo_tilt_gpio</span>
	to the PWM GPIO pin numbers.  So the pan/tilt or tilt/pan gpio pairs must be one of<br>
	&nbsp;&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>12,13 &nbsp; 12,19 &nbsp; 18,13 &nbsp; 18,19</span><br>
	Stop pikrellcam before editing ~/.pikrellcam/pikrellcam.conf to set the GPIO values.
	Then restart pikrellcam, reload the web page and you will have new buttons to control
	position presets and moving the servos.
	</li>
	<p>
	<li><span style='font-weight:700'>ServoBlaster</span>: If your servos are connected to
	GPIOs that are not the hardware PWM pins you can use ServoBlaster.
	Set
	<span style='font-weight:700'>servo_pan_gpio</span> and
	<span style='font-weight:700'>servo_tilt_gpio</span>
	in pikrellcam.conf to the ServoBlaster servo numbers you are using and set
	<span style='font-weight:700'>servo_use_servoblaster</span> to
	<span style='font-weight:700'>on</span>.<br>
	For this a separate install of ServoBlaster is required according to ServoBlaster
	documentation.<br>
	Stop pikrellcam before editing ~/.pikrellcam/pikrellcam.conf to set the use
	servoblaster option and gpio values to ServoBlaster servo numbers.
	Then restart pikrellcam and reload the web page.
	</li>
</ul>
	After configuring for servos, the first thing to do is to check if the servos move
	in the right direction when the servo arrow buttons are clicked.  If they do not,
	then the directions can be inverted in
		<nobr><span style='font-weight:700'>Setup->Config->Servo</span></nobr>
</div>

<span style='font-size: 1.5em; font-weight: 650;'>Presets</span><hr>
<div class='indent0'>
<img src="images/preset-servos.jpg" alt="preset-servos.jpg"> 
<p>
A preset is a camera position with a group of motion detect settings
(vector magnitude / count and burst count / frames) and a set of motion regions.
Clicking the preset up/down
arrows moves to a new settings preset which single click loads a completely new set of
motion detect settings and motion regions.  So presets can be configured with
motion detect sensitivities and motion regions appropriate for different weather
or other conditions and quickly selected with single clicks.
<p>
Preset left/right arrow buttons are shown only if servos are configured and
move the servos to configured position presets.
<p>
The Servo button and arrows are shown only if servos are enabled.  Click the
<span class='btn-control'>Servo</span>
button to cycle the servo arrow direction buttons through three modes: step by one, step by
<span style='font-weight:700'>Move_Steps</span>, and scan.  When arrow buttons are in scan
mode, clicking an arrow will step the servo continuously at
<span style='font-weight:700'>Move_Step_msec</span>
rate until the arrow button is clicked
again or the servo reaches a pan/tilt limit.
<p>
<span style='font-weight:700'>Preset behavior without servos:</span>
<ul>
	<li> PiKrellCam considers the camera at a single fixed position and will never be
		off a preset.  There will be only preset up/down arrows and
		no preset left/right or servo arrows.  The pan/tilt graphics in the above image
		will not be shown.
	</li>
	<li> Any motion settings or regions edits will immediately apply to the currently
		selected settings preset (Preset up/down arrows).
	</li>
	<li> To create a new settings preset, click
		<nobr><span style='font-weight:700'>Setup->Preset->New</span></nobr>
		and a new settings preset will be created with the existing
		motion settings and regions which can then be edited.
	</li>
</ul>
<span style='font-weight:700'>Preset behavior with servos:</span>
<ul>
	<li> If <nobr><span style='font-weight:700'>Setup->Config->Servo->Motion_Off_Preset</span></nobr> is
		<span style='font-weight:700'>OFF</span>,
		motion detection applies only if the servos are on a preset and if the servos are
		moved off a position preset with the servo arrow buttons
		then motion detection is put on hold.  Set this option to
		<span style='font-weight:700'>ON</span>
		if you want to have motion detected even if a servo position is off a preset.
	</li>
	<li> Presets cannot be created with different tilt positions at the same pan
		position.
	</li>
	<li> When the servos are on a position preset, a new settings for the position
		is created with
		<nobr><span style='font-weight:700'>Setup->Preset->New</span></nobr>
	</li>
	<li> To create a preset at a new position:
	<ul>
		<li>Move the servos to the desired position and click <br>
		&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>Setup->Preset->New</span><br>
		and you can then edit the settings and motion regions for the preset.
		<br>
		Or you may move the servos off a preset and edit
		the settings or regions before creating the new preset and the OSD will warn you
		that you will need to create a new preset or else your edits will not be saved.
		If you move the servos back to an existing preset before creating a new one,
		your edits will be replaced with the preset settings.
		</li>
		<li>Copy an existing set of settings from a position preset to a new preset at
		a new position.<br>
		To do this, first move the servos to the existing preset you want to
		copy the settings from.
		Then, use the Servo arrows to move the camera to the new position (don't let
		the servo position fall on any other preset), and click<br>
		&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>Setup->Preset->Copy</span><br>
		</li>
	</ul>
	</li>
</ul>
</div>




<span style='font-size: 1.5em; font-weight: 650;'>Motion Regions Panel</span><hr>
<img src="images/motion-regions.jpg" alt="motion-regions.jpg"> 
<div class='indent0'>
As motion regions are edited, they are saved to the current preset unless servos are
configured and the servo position is not on a preset.  If the servo position is not on
a preset, motion region edits will be lost unless you create a new preset or save the
motion regions by name.
<p>
Motion regions outline areas of the camera view that will be
sensitive to motion and provides for excluding from motion detection areas such
as wind blown vegetation.
Motion regions may be added, deleted, resized or moved at each preset.

Motion regions may also be saved by name and this provides a way to maintain a set of
motion regions as a backup or a temporary.  For example, a backup motion region by
name can be loaded as an initial condition after creating a new preset.  Or temporary
motion regions by name can be loaded if you have a set of different motion regions you
want to load to a preset on demand for evaluation.
If a motion region is loaded by name it is automatically saved to the
current preset unless you have servos and are off a preset.
<p>
The increment of a region resize or move can be coarse or fine by selecting/deselecting the
<span style='font-weight:700'>Coarse Move</span> select button.  When the increment is fine,
the adjustment units are single motion vectors.  Motion vectors are 16x16 pixel areas of the
video so the motion vector array overlayed on the preview display for a 1080p video setting
will be 120x68.
<p>
A single region can be expanded to cover any motion area of interest, but if the area is
large relative to the total frame size, there is one advantage to splitting the area into more
than one region.  Each region is independently checked for direction motion and there is one
composite vector per region.  If there are two smaller separated objects moving in the
region the composite vector for the region will have a center away from the two objects
and the motion test can fail the density check.  However, in practice this might not be an
issue because the motions may be sporadic or otherwise not synchronous so
motion is likely to be detected regardless.  But if in doubt, go ahead and split areas into multiple
regions.  This is why the default motion regions configuration has four regions spanning the
width of the motion frame.
<p>
This should usually not be an issue, but the size of a motion region should not be so small
that it cannot hold the number of vectors that is set for the vector limit count.  For example,
if the limit count is set to 20 a small region of 10x6 size out of a total frame size of
120x68 will hold three times this limit and should be fine.  A safe rule of thumb when limit counts
are set higher would be to keep dimensions of motion regions not much smaller than 1/8 of the total
frame dimensions.
<p>
Motion burst detection considers the total frame composite vector so the total area of all
motion regions compared to the burst limit count is the factor to consider.
</div>


<span style='font-size: 1.5em; font-weight: 650;'>Setup Panel</span><hr>
<img src="images/setup.jpg" alt="setup.jpg"> 
<div class='indent0'>
After a menu is opened and an option or value from it is highlighted
by clicking the arrow buttons, the option or value must be finalized by clicking the
<span class='btn-control' >Sel</span>
button.  This is required for the change to be saved in the configuration file.
<p>
If servos are not configured, there will be no Move or Copy buttons in the
Preset group and there will be no Servo button in the Config group.
<p>
<span style='font-size: 1.2em; font-weight: 650;'>Preset</span>
	<ul>
		<li><span style='font-weight:700'>Settings</span><br>
		These values are part of a preset and editing them applies to the currently
		selected preset.  If you have servos and are off a preset, editing these values
		can be done in anticipation of creating a new preset.
		<p>
			<ul>
			<li><span style='font-weight:700'>Vector_Magnitude</span> - sets the minimum magnitude
			of a motion vector.  Individual motion vector and region composite vector
			magnitudes less than this value will not be considered valid motion.
			Set the value lower to increase sensitivity to slower moving objects.  Slow walking
			cats or people at a distance will probably require low settings of 7 or less while
			faster objects like cars or closer people can detect with higher settings.
			This value applies to composite vectors used for both direction and burst detects.
			</li>
			<p>
			<li><span style='font-weight:700'>Vector_Count</span> - sets the minimum count of
			individual motion vectors required for a region composite vector.
			Set the value lower for increased sensitivity to direction detects of smaller
			moving objects.  A value of 4 or 5 can detect small animals at moderate distances.
			A very low setting of 2 or 3 can detect small animals at greater distances, but with
			increased risk of false detects.
			For detecting
			people at medium distances or cars, the value can be set from 10 on up.
			Burst detects of larger objects have a separate count requirement.
			</li>
			<p>
			<li><span style='font-weight:700'>Burst_Count</span> - sets the minimum count of
			individual motion vectors required for a frame composite vector burst detect.
			For a range of object sizes, burst detection will overlap direction detection and
			it may be difficult to observe the effect as this value is changed.  If the camera
			view is noisy, the value probably should be adjusted higher, but if the view is
			normally quiet with well placed motion regions, the value can probably be left at
			a low value.  For a detect, the total frame vector count must exceed the
			background count exponential moving average by the Burst_Count, and at least
			one motion region must pass a density test.
			</li>
			<p>
			<li><span style='font-weight:700'>Burst_Frames</span> - sets the minimum number of
			frames of sustained burst counts required for a burst motion detect.  Frames are
			checked at a video_fps/mjpeg_divider rate.  The Pi camera will occasionally produce
			"glitches" of motion vectors which would generate many false detects if Burst_Frames
			was 1, so the minimum is 2.
			A value of 3 should filter out most glitches, but setting the value to 2 gives the
			fastest detects of large objects which may pass quickly through the camera
			field of view.  Set the value higher to decrease detection of large objects passing
			quickly through the field of view. 
			</li>
<a name="ZOOM_PERCENT">
			<p>
			<li><span style='font-weight:700'>Zoom_Percent</span> -
			is the percent width and height of the camera sensor used for
			imaging.  Set the percent lower to digitally zoom in for videos
			and stills.  Moderate zooming can tune the camera field of view
			at the expense of some resolution as fewer pixels may be used to
			convert to stills or videos.
			While viewing the web page preview, a larger zoom (lower percent
			value) magnifies the pixels and is useful for camera focusing.<br>
			If a zoom value is selected, it is saved as part of the current
			preset along with the motion limit and motion region settings.
			</li>
			</ul>
		</li>
		<p>
		<li><span style='font-weight:700'>Move: One</span><br>
			You will have this button only if servos are configured.<br>
			If the servos are moved off a preset, click this if you want to move
			the preset you were on to the current servo position.
		</li>
		<p>
		<li><span style='font-weight:700'>Move: All</span><br>
			You will have this button only if servos are configured.<br>
			If the servos are moved off a preset, click this if you want to move
			the preset you were on to the current servo position and move all the other preset
			positions by the same amount.  If the camera installation is disturbed or serviced,
			this allows a quick adjustment for restoring position presets.  The other presets
			may still need small adjustments if servo positioning is non linear.  All presets
			cannot be moved if the move would move any preset past a servo position limit.
		</li>
		<p>
		<li><span style='font-weight:700'>Del</span><br>
			If servos are not configured or if the servo position is on an existing preset, delete
			the current Settings.  If servos are configured and the servo position is on a preset
			and the Settings are the last Settings for the preset, then delete the position preset
			unless it is the only existing position preset.  There must always be at least one
			preset and you cannot delete down to zero presets.
		</li>
		<p>
		<li><span style='font-weight:700'>Copy</span><br>
			You will have this button only if servos are configured.<br>
			If the pan servo is moved off a preset, click this to create a
			new preset at the servo position which is initiallized by copying all of
			the preset settings (motion detect limits and regions) from the preset you
			were on into the new preset.
		</li>
		<p>
		<li><span style='font-weight:700'>New</span><br>
			Creates a new preset. If servos are not configured or if the servo position is on an
			existing position preset, this will create a new Settings preset which can then
			be edited.  If servos are configured and the servo position is not on an existing preset,
			then a new position preset is created with an initial single Settings.
		</li>
	</ul>

<p>
<span style='font-size: 1.2em; font-weight: 650;'>Time Lapse</span>
	<div class='indent1'>
	Enter a time period in the text box and click <span style='font-weight:700'>Start</span>
	to begin a time lapse run.  Entering a new period and clicking
	<span style='font-weight:700'>Start</span> will change the period for the current time
	lapse run and does not start a new time lapse run.  Click the
	<span style='font-weight:700'>Timelapse</span> button on the button bar to show the time
	lapse status on the preview OSD.  When the
	<span style='font-weight:700'>End</span> button is clicked PiKrellCam will run a time lapse
	end script which will convert the time lapse images into a video and store the final video
	in the media <span style='font-weight:700'>videos</span> directory and the video name
	will have a
	<span style='font-weight:700'>tl_</span> prefix.
	The progress of this
	conversion will be shown on the time lapse OSD display.  To better control start, end and
	overnight hold times, a time lapse can be controlled with at commands.  See that section for
	an example.
	</div>

<p>
<span style='font-size: 1.2em; font-weight: 680;'>Config</span>
<a name="CONFIG_MOTION">
	<ul>
		<li><span style='font-weight:700'>Motion</span>
			<ul>
			<li><span style='font-weight:700'>Startup_Motion</span> - set to
			<span style='font-weight:700'>ON</span> for motion detection to be enabled each time
			PiKrellCam starts.  Motion detection can be
			enabled from the web page or a script.
			</li>
<p>
			<li><span style='font-weight:700'>Confirm_Gap</span> - for motion direction detects,
			require a second motion detect within this period of seconds before triggering a
			motion record event.  This adds a level of noise rejection to motion direction detecting
			but may be set to zero to disable a second detect requirement if fast detection is desired.
			This setting does not apply to motion burst detects because the Burst_Frames setting
			provides a confirm time for that method.
			</li>
<p>
			<li><span style='font-weight:700'>Pre_Capture</span> - seconds of video to record prior
			to the first motion detect event.  This value should be greater than or equal to the
			Confirm_Gap.  Pre capture does not apply when motion stills recording.
			</li>
<p>
			<li><span style='font-weight:700'>Event_Gap</span> - number of seconds that must pass
			since the last motion detect before a motion event recording can end.
			When an Event_Gap period does expire without a new motion detect occurring,
			videos will end with an end time of the last motion detect time plus the
			Post_Capture time (but see Post_Capture).
			Set this higher for animals or walking people that may pause for
			periods of time before resuming motion.  Set lower for active scenes where events
			are frequent and you want to bias towards shorter videos
			that capture more events separately.
			</li>
<p>
			<li><span style='font-weight:700'>Post_Capture</span> - seconds of video
			that will be recorded after the last occurring motion event.  This time must be
			less than or equal to the Event_Gap time because post capture time is accumulated
			in the circular buffer while the video is recording.  An expiring Event_Gap time
			ends the video immediately and no more Post_Capture time can be accumulated.
			Post_Capture time does not apply when motion stills recording is enabled, so the
			last motion still captured will be for the last motion detect that extended the
			event gap and satisfied the Max_Motion_Stills_per_Minute constraint.
			</li>
<p>
			<li><span style='font-weight:700'>Video_Time_Limit</span> - range is 0 - 1800
			seconds (30 minutes) and is the maximum seconds of motion video
			that will be recorded after the first occurring motion detect.  So the total
			video length max will be the Pre_Capture time + the Time_Limit.
			If this is set to zero, there will be no time limit enforced.  This limit
			does not apply to manual recordings - see FIFO examples for that.
			</li>
<p>
			<li><span style='font-weight:700'>Motion_Stills_(no_videos)</span> - set to
			<span style='font-weight:700'>ON</span> to capture still images
			during a motion record event and a motion video will not be
			recorded.
			</li>
<p>
			<li><span style='font-weight:700'>Max_Motion_Stills_per_Minute</span> - can be 1 to 60
			stills per minute which sets a max still capture rate of 1 per minute to 1 per second.
			During motion record events, stills are taken at motion detects separated by at least
			this rate.  So stills will not be taken at motion detects more frequently than
			the max still capture rate. However, all motion
			detects during the motion record event are written to the
			<nobr><span style='font-weight: 700;'>/run/pikrellcam/motion-events</span></nobr>
			file.
			</li>

			</ul>
			<p>
			<div class='indent1'>
			<span style='font-weight:700'>Note:</span>
			Videos can be started only on key frame boundaries.  When waiting for a motion
			event to start, PiKrellCam requests key frames from the camera once per second,
			so a Pre_Capture time setting of T will actually record somewhere between
			T and T+1 seconds.
			Also, if there is a new motion event immediately following a previous motion
			video end, there may not be the configured Pre_Capture time accumulated in the
			circular buffer and so the actual pre capture time will be less than what
			is configured.
			</div>
		</li>
<p>
		<li><span style='font-weight:700'>Video Res</span> - selects the video resolution for
		motion and manual videos.  Different resolutions may have different fields of view. So
		one reason for selecting
		<span style='font-weight:700'>720p</span> over
		<span style='font-weight:700'>1080p</span> would be to get a wider field of view.  Resolutions
		will have either 16:9 or 4:3 aspect ratio.
		</li>
<p>
		<li><span style='font-weight:700'>Still Res</span> - selecting different resolutions
		gives different fields of view and aspect ratios.
		</li>
<p>
<a name="DISKFREE">
		<li><span style='font-weight:700'>Settings</span>
			<ul>

			<li><span style='font-weight:700'>Check_Media_Diskfree</span>
			- if set <span style='font-weight:700'>ON</span>, when new motion
			or manual videos or stills/timelapse are recorded, delete
			oldest videos or jpegs so that the configured minimum
			Diskfree_Percent will be maintained on the media file system.
			</li>
<p>
			<li><span style='font-weight:700'>Check_Archive_Diskfree</span>
			- if set <span style='font-weight:700'>ON</span>, when media
			motion or manual videos are archived, delete oldest archived videos
			so that the configured minimum Diskfree_Percent
			will be maintained on the archive file system.
			This is useful when the archive is on a file system
			separate from the media videos.  But if the archive and media
			directories are on the same file system, checking the archive
			has no or negligible effect since an archive operation is simply
			moving media videos and except for some directory structure overhead
			is not increasing disk usage.
			</li>
<p>
			<li><span style='font-weight:700'>Diskfree_Percent</span>
			- maintain this minimum free percent on media and archive
			file systems when checking is enabled for those file systems
			by deleting oldest videos or stills/timelapse.
			This is always enabled for loop videos and overrides
			Diskusage_Percent.
			</li>
<p>
			<li><span style='font-weight:700'>video_bitrate</span> - determines the size of a video
			and its quality.  Adjust up if it improves video quality.  Adjust down if you want
			to reduce the size of the videos.
			</li>
<p>
			<li><span style='font-weight:700'>video_fps</span> - if set higher
			than 24 the motion detecting preview camera stream can drop frames
			depending on Pi model (GPU clock speed).
			</li>
			<p>
			<li><span style='font-weight:700'>video_mp4box_fps</span> - when zero, the mp4
			boxing fps will track video_fps which is normally what you want. Set this to
			a non zero value different from video_fps for fast or slow motion videos.
			Or set to fractional values slightly different from video_fps to tune possible
			audio/video drift for longer videos.
			</li>
			<p>
			<li><span style='font-weight:700'>mjpeg_divider</span> - this value is divided into
			the video_fps value to get the preview jpeg rate.  The preview is updated at this rate
			and it is the rate that motion vector frames are checked for motion.
			</li>
			<p>
			<li><span style='font-weight:700'>still_quality</span> - adjust up if it improves
			still jpeg quality.  Adjust down if you want to reduce the size of still jpegs.
			</li>
			<p>
			<li><span style='font-weight:700'>Vector_Counts</span> - enable showing of vector count
			statistics when showing a Preset.  This information may help when setting motion detect
			limits.
			</li>
			<p>
			<li><span style='font-weight:700'>Vector_Dimming</span> - sets a percentage dimming
			of the preview jpeg image when the
			<span style='font-weight:700'>Vectors</span> display is enabled.  This is to improve
			the contrast of the drawn motion vectors.
			</li>
			<p>
			<li><span style='font-weight:700'>Preview_Clean</span> - if set to
			<span style='font-weight:700'>OFF</span>, whatever text or graphics that happen to be
			drawn on the preview jpeg at the time a motion preview save or thumb save occurs will
			also appear on the saved preview or thumb.  This might help with some debugging, but
			is normally not desirable, so the option should be set
			<span style='font-weight:700'>ON</span>.
			</li>
<a name="STALL_WARNING">
			<p>
			<li><span style='font-weight:700'>Preview_Stall_Warning</span>
			- default is
			<span style='font-weight:700'>ON</span>.
			Enable showing a warning on the web preview if the preview jpeg
			encoder stalls when hitting GPU performance limits which cause
			preview frames to be lost.
			The solution is to lower video_fps until the stalls stop or at
			least become infrequent.
			This strangely seems more an issue (at least with current
			firmware) with the HQ camera on the Pi 4.  With an HQ camera
			on a Pi 4 I needed to lower video_fps to 20 to avoid stalls,
			but on a P2 the HQ camera ran without stalls at video_fps 24.
			I've not seen this issue with V1 or V2 cameras.<br>
			If this option is
			<span style='font-weight:700'>OFF</span>,
			the warning is still enabled when showing preset information
			with the
			<span style='font-weight:700'>Show: Preset</span> button.
			</li>
			</ul>
		</li>
		<p>
<a name="DISKUSAGE">
		<li><span style='font-weight:700'>Loop</span><br>
			<ul>
			<li><span style='font-weight:700'>Startup_Loop</span> - set to
			<span style='font-weight:700'>ON</span> for loop recordings to
			be enabled each time PiKrellCam starts.  Loop recording can be
			enabled from the web page or a script or an at-command.
			</li>
			<p>
			<li><span style='font-weight:700'>Time_Limit</span>
			- loop video length in seconds.
			</li>
			<p>
			<li><span style='font-weight:700'>Diskusage_Percent</span>
			- Limit disk space used by loop videos to this percent
			by deleting oldest loop videos as new ones are recorded.
			Loop diskusage checks cannot be disabled.  If the free space on
			on the loop videos filesystem falls to the configured
			Diskfree_Percent then that will override this disk usage percent
			and allowed loop video disk usage can shrink below this value.
            </li>
			</ul>
		</li>
		<p>
<a name="AUDIO_TRIGGER">
		<li><span style='font-weight:700'>Audio</span><br>
			<ul>
			<li><span style='font-weight:700'>Audio_Trigger_Video</span>
			- set to <span style='font-weight:700'>ON</span>
			to enable audio detects to be treated as motion detects to trigger
			motion videos.
			</li>
			<p>
			<li><span style='font-weight:700'>Audio_Trigger_Level</span>
			- sets the audio level to be met or exceeded for triggering a
			motion video.
			</li>
			<p>
			<li><span style='font-weight:700'>Box_MP3_Only</span>
			- if set <span style='font-weight:700'>ON</span>
			and there are only audio triggers (no motion vector detects or
			external triggers)
			during a video recording, then do not include the h264 video
			in the MP4 boxing.  The resulting recording will be a .mp4
			boxed file containing only MP3 data and no video.  Use this option
			if you want to save disk space for recordings that have audio
			but may not have interesting video.
			</li>
			</ul>
		</li>
		<p>
		<li><span style='font-weight:700'>Servo</span><br>
		The Servo menu is shown only if servos have been configured.
		<p>
			<ul>
			<li><span style='font-weight:700'>Motion_Off_Preset</span> - if
			<span style='font-weight:700'>OFF</span>, do not detect motion when the servo postion
			is off a preset.  If configured motion regions are suitable for any servo position,
			this can be set
			<span style='font-weight:700'>ON</span> if you do not want motion detection to be
			put on hold when a servo is manually moved off a preset.
			</li>
			<p>
			<li><span style='font-weight:700'>Move_Step_msec</span> - delay in milliseconds between
			servo steps when a servo is moved using the
			<span style='font-weight:700'>Servo</span> arrow buttons.  A servo step changes the
			pulse width of the servo control line by 1/100 of a millisecond (10 usec).
			</li>
			<p>
			<li><span style='font-weight:700'>Preset_Step_msec</span> - delay
			in milliseconds between servo steps when a servo is moved using the
			<span style='font-weight:700'>Preset</span>
			left/right arrow buttons.
			</li>
			<p>
			<li><span style='font-weight:700'>Servo_Settle_msec</span> - delay in milliseconds before
			motion detection is taken out of its hold state after servos stop moving.
			</li>
			<p>
			<li><span style='font-weight:700'>Move_Steps</span> - number of steps to move when the
			<span style='font-weight:700'>Servo</span> arrow buttons are cycled to the second step
			mode.  The other step modes are single step and scan and are selected by clicking the
			<span style='font-weight:700'>Servo</span> button.  When in scan mode, the scan can be
			stopped by clicking the double arrow button again.
			</li>
			<p>
			<li><span style='font-weight:700'>Pan_Left_Limit</span><br> 
				<span style='font-weight:700'>Pan_Right_Limit</span><br>
				<span style='font-weight:700'>Tilt_Up_Limit</span><br>
				<span style='font-weight:700'>Tilt_Down_Limit</span> - Sets the servo limits.
			</li>
			<p>
			<li><span style='font-weight:700'>Servo_Pan_Invert</span><br> 
				<span style='font-weight:700'>Servo_Tilt_Invert</span> - Set these to ON or OFF
				to change the direction servos move when the direction arrows are clicked.
			</li>
			</ul>
	</ul>
<span style='font-size: 1.2em; font-weight: 650;'>Camera Params</span>
	<div class='indent1'>
	The camera parameters menus can set the hardware parameters of the Pi camera.
	</div>

</div>



<a name="AUDIO">
<span style='font-size: 1.5em; font-weight: 650;'>Audio</span><hr>
<div class='indent0'>
Pikrellcam can do two independent audio MP3 encodes.  One encode
is for recording audio with videos and is sourced from PCM sound data stored
in a circular buffer so there can be in sync pre-captured sound to track
video pre-capture.  The other encode is for streaming MP3 sound to a browser
(but see Issues) and is sourced from PCM sound data as it is read
from the sound capture device.
The streamed audio is not in sync with the mjpeg image stream
displayed by the browser because audio buffering will cause a
couple of seconds delay.
<p>
Connect an ALSA input capture device such as
USB sound card + microphone, USB mini microphone, or other which can
be recognized by running
<span style='font-weight:700'>arecord -l</span>.
If arecord can record from the device and aplay play the wav file,
then pikrellcam should work using the same sound device.  You may need to
run alsamixer to unmute the microphone input and set the capture level.
If you need more information than the basic setup listed here, the web
has many Pi microphone tutorials to look at.
<br>
So a microphone setup for PiKrellCam is:
<ul>
	<li>
	The user <span style='font-weight:700'>pi</span>
	must be in the audio group.<br>
	Connect the USB sound card + microphone and the kernel should
	load the USB sound modules.
	Verify the sound card number using arecord.
	I get card 1 and use that for the remaining examples:
<pre>
pi@rpi2: ~$ arecord -l | grep USB
card 1: Device [USB PnP Sound Device], device 0: USB Audio [USB Audio]
</pre>
	</li>
	<li> Run alsamixer on card 1 and make sure the microphone input is
	not muted.  Probably set the input sensitivy high.
	In alsamixer, press F4 to show controls
	for the microphone capture input for the card.
<pre>
pi@rpi2: ~$ alsamixer -c 1
</pre>
	</li>
	<li>
	For sound card 1, the ALSA hardware device is plughw:1.
	Test that device by recording and playing an audio wav file:
<pre>
  # Do a basic 5 second record:
pi@rpi2: ~$ arecord  -D plughw:1 -d 5  test.wav
  # And also check using parameters the same as pikrellcam recording defaults,
  # defaults are: device: plughw:1  channels: 1  rate: 24000 or 48000  16 bit audio.
arecord -D plughw:1 -d 5 -c 1 -f s16_LE -r 24000 test.wav
  # Play the sound:
pi@rpi2: ~$ aplay test.wav
</pre>
	</li>
	<li> If the USB sound card is not plughw:1, edit
	audio_device in pikrellcam.conf to be the correct device
	and restart pikrellcam.
	</li>
	<li> Enable/disable audio recording is done by clicking the microphone
		audio control toggle button on the web page.  An audio VU meter is
		drawn on the OSD when the microphone is successfully opened.
	</li>
</ul>

<p>
<span style='font-size: 1.2em; font-weight: 680;'>Audio Control Buttons</span><br>
<div class='indent1'>
<img src="images/audio-controls.jpg" alt="audio-controls.jpg"> 
Web page audio control buttons are left to right:
<ul>
	<li><span style='font-weight:700'>Audio Stream Stop</span> -
		The OSD moving stream indicator will disappear.
	</li>
	<li><span style='font-weight:700'>Audio Stream Play</span> - If the
		microphone is open, PCM audio is encoded into MP3 audio
		which can be played by a browser (see Issues). When streaming is on,
		the OSD shows a moving streaming indicator under the VU meter and
		gain value.
	</li>
	<li><span style='font-weight:700'>Microphone Toggle</span> - Opens and
		closes the microphone. When the microphone is open, the OSD shows a
		vertical audio VU meter with the current gain value printed under
		it. With the microphone opened, audio is recorded with videos and
		can be streamed.
	</li>
	<li><span style='font-weight:700'>Audio Gain Up</span> - Increment the
		audio gain up to 30dB.
		This is not the gain set by alsamixer but is an additional
		amplication of the PCM sound data read from ALSA to help boost audio
		at the expense of amplified noise and risk of distortion from clipping.
	</li>
	<li><span style='font-weight:700'>Audio Gain Down</span> - Decrement the
		audio gain in dB to a minimum of 0dB (amplification factor is 1).
	</li>
</ul>

</div>
<span style='font-size: 1.2em; font-weight: 680;'>Audio Parameters in pikrellcam.conf</span><br>
<div class='indent1'>
Edit pikrellcam.conf to change these settings:

<ul>
	<li><span style='font-weight:700'>audio_device</span> - default: plughw:1<br>
		Sets the ALSA hardware audio input (microphone) capture device.
	</li>
	<li><span style='font-weight:700'>audio_rate_Pi2</span> - default: 48000<br>
	    <span style='font-weight:700'>audio_rate_Pi1</span> - default: 24000<br>
		Audio sample rate used for a single core Pi and Pi model 2.
		Lame docs suggest using only MP3 supported sample rates:<br>
	&nbsp;&nbsp;&nbsp; 8000 11025 12000 16000 22050 24000 32000 44100 48000<br>
	</li>
	<li><span style='font-weight:700'>audio_channels</span> - default: 1<br>
		Set to 1 for mono or 2 for stereo.  If using a common
		USB sound card that supports only one channel, setting 2 will
		be reverted to 1 when pikrellcam opens the microphone.
	</li>
	<li><span style='font-weight:700'>audio_mp3_quality_Pi2</span> - default: 2<br>
	    <span style='font-weight:700'>audio_mp3_quality_Pi1</span> - default: 7<br>
		Value for quality of the lame lib encode of PCM to MP3 audio for
		a single core Pi and Pi model 2.
		Values range from 0 (best quality but very slow encode) to 9
		(worst qualilty but fast encode). Lame docs say 2 is near best and
		not too slow and 7 is OK quality and a really fast encode.
	</li>
</ul>
Pi single core and quad core models have separate settings
for sample rate and encode quality because two simultaneous audio MP3
encodings can push a single core to very high CPU usage.  A Pi2/3 does not have
a CPU usage issue.  This image shows CPU usage for a single core Pi
to give an idea of what to expect.  Streaming audio while recording
(R2 interval: two MP3 encodes) uses high CPU and causes
an extended video conversion time (C2 interval: one MP3 encode).
It is the particular Pi1 use case of expected video length/frequency and
any overclocking that determines what audio sample rate and encode
quality should be set and whether it is wise to stream audio
while videos are recording.
<p>
<img src="images/cpu-usage.jpg" alt="cpu-usage.jpg">

</div>

<p>
<span style='font-size: 1.2em; font-weight: 680;'>Limitations & Issues</span><br>
<div class='indent1'>
<ul>
	<li> Audio MP3 streaming works for me to Firefox but not to Chromium
		for some reason. Don't know if YMMV on this.
	</li>
	<li> FYI, sometimes clicking the microphone toggle button can fail to open
		the microphone because the device is busy, but clicking it
		some more eventually succeeds.
		Running arecord can similarly fail with device busy so it may
		be some issue with USB sound cards.
	<li> If a video has out of sync audio, check the log to see if
		the actual video fps and audio rate was what is configured.
		If these are off, then data has been lost during the record.  This
		is likely more a possible issue on a Pi1.
	</li>
	<li> Audio cannot be streamed to more than one web page at a time.
	</li>
	<li> ALSA audio capture devices cannot be opened by more than one
		application.  If a microphone is needed for another purpose, it
		cannot also be open in pikrellcam.
	</li>
</ul>
</div>

<p>
<span style='font-size: 1.2em; font-weight: 680;'>Microphones</span><br>
<div class='indent1'>
A high
<a href="https://geoffthegreygeek.com/microphone-sensitivity/">
microphone sensitivity</a> is likely needed for a PiKrellCam application
which wants to pick up related audio when recording events at some
distance from the camera.
A cheap way to experiment is to order some
<a href="http://www.hobby-hour.com/electronics/computer_microphone.php">
electret microphones</a>
from an electronics supplier and solder them to a 3.5mm plug.  I have been
using electrets with sensitivities from
<a href="http://www.mouser.com/ProductDetail/DB-Unlimited/MO093803-1/?qs=sGAEpiMZZMvxTCYhU%252bW9md6RLkZl0Nse48Qi7C4xp2w%3d">
-38dB</a> to
<a href="http://www.mouser.com/ProductDetail/CUI/CMC-6027-24T/?qs=sGAEpiMZZMuCv89HBVkAk5iC6ZN50VtrfpvYg8FFM2E%3d">
-24dB</a>
ordered from Mouser, but similar ones should be available from other
suppliers near you.  USB sound cards I have tried so far have the
microphone jack tip internally shorted to the ring.  To check, connect the
plug to the sound card and measure for zero resistance between the tip and
ring lugs.  So I simply solder the microphone (-) terminal to plug ground
and the microphone (+) terminal to either the plug tip or ring.
<p>
<img src="images/electret.jpg" alt="electret.jpg">
<p>
Noise is a possible issue when using sensitive omnidirectional microphones.
A couple of likely causes are power line hum from the surrounding
microphone environment or electrical noise getting into the USB sound
card through the power supply.
So microphone placement and a clean power supply can be important.
</div>

</div>



<a name="LOOP">
<span style='font-size: 1.5em; font-weight: 650;'>Loop Recording</span><hr>
<div class='indent0'>
Click the loop record button
<input type="image" src="images/loop.png">
to toggle recording continuous loop videos.
If motion is enabled during a loop video and motion
detected, the motion commands are run (on_motion_begin, on_motion_preview_save,
on_motion_end) and the web page thumbnail for the loop video will
show that motion occurred.
<p>
The default loop directory is under the ~/pikrellcam/media directory.
This directory can be mounted with a dedicated loop file system and mounting
can be done in fstab or in the pikrellcam startup script.  Or a
dedicated disk can be mounted to some other directory if the loop_dir
value in pikrellcam.conf is edited to reference that location.
<p>
Loop video recording continuously wears flash disks,
so if that is an issue, they can be enabled only for limited times
of interest with commands sent to the FIFO:
<pre>
echo "loop on" > ~/pikrellcam/www/FIFO
echo "loop off" > ~/pikrellcam/www/FIFO
echo "loop toggle" > ~/pikrellcam/www/FIFO
</pre>
or commands in at-commands.conf like:
<pre>
Mon-Fri 7:30   "@loop on"
Mon-Fri 9:30   "@loop off"
</pre>

Oldest loop videos are automatically deleted to enforce both a configured
maximum disk usage percent and configured minimum disk free percent with a
priority on minimum disk free percent.  The idea is to have a fixed maximum
diskusage percent for loop videos and then media videos (manual and motion)
and archived videos are allowed to grow until a minimum disk free percent
remains if disk free checking is enabled for those videos.
<p>
Examples on the interactions of loop recording, setting disk percent
limits and enabling disk free checking for motion videos:
        <ul>
            <li> Media videos, archived videos and loop videos
            are all on the same mounted file system that is different
            from the SD card OS file system.  If the
            Diskusage_Percent is set to say 50% and Diskfree_Percent
            is 10%, then there will be up to 40% of shared disk space
            available for media and archived videos. Setting
            Check_Archive_Diskfree ON has little to no effect because
            archiving is moving files within the same file system and
            not using more disk space.
            </li>
			<p>
            <li> Media videos and loop videos are all on
            the same file system that is shared with the OS on a
            SD card where the OS uses about 1/2 of the space. If
            Diskusage_Percent is set to 25% and Diskfree_Percent
            is set to 10%, then up to 15% disk space will be
            available for media videos.  Archiving has the same effect
            as above.
            </li>
			<p>
            <li> Media videos, archive videos and loop videos are each
            on separate file systems (disks mounted and dedicated to each
            video type).  If Diskfree_Percent is 10%, media videos
            can use up to 90% disk space, and Diskusage_Percent
            can be set high to 90% to use almost all of loop video disk
            space. In this case Check_Archive_Diskfree can be set ON
            and the oldest archived videos will be deleted to maintain a
            Diskfree_Percent minimum applied to the archive disk.
            </li>
        </ul>
Configure the loop time limit and max diskusage percent in the web page
Setup->Config->Loop.

</div>




<span style='font-size: 1.5em; font-weight: 650;'>Configuration Files</span><hr>
<div class='indent0'>
<span style='font-size: 1.2em; font-weight: 650;'>~/.pikrellcam/pikrellcam.conf</span>
	<div class='indent1'>
	Many options that are stored in this file can be set in the web page panels with the
	OSD menu system and were described above.
	Other options in this file are set by manually editing the file.
	If the file is manually edited, PiKrellCam must be stopped and restarted to pick up the changes.
	Configurations are described in the file, but some suggestions:
	<ul>
		<li><span style='font-weight:700'>mjpeg_width</span>
		- has an initial default value of 640, but for most browser views this can be set higher
		to get a larger preview image.  Values of 800 or 896 work OK.  If this is changed,
		the motion_area_min_side value in pikrellcam.conf should be correspondingly scaled.
		</li>
		<p>
		<li><span style='font-weight:700'>video_motion_name_format</span><br>
			<span style='font-weight:700'>video_manual_name_format</span><br>
			<span style='font-weight:700'>video_timelapse_name_format</span><br>
			<span style='font-weight:700'>still_name_format</span>
		- these file name formats are configurable with restrictions which are described in
		detail in pikrellcam.conf where they are configured.
		</li>
		<p>
		<li><span style='font-weight:700'>motion_record_time_limit</span>
		- This value limits the time in seconds of motion video recordings and can be
		set in seconds from 10 to 1800 (30 minutes) or set to zero for no record time limit.
		This time limit does not apply to manual recordings, but see the FIFO command
		examples for how to have a time limited manual record.
		</li>
		<p>
		<li> Todo
		</li>
		<li>
		</li>
		<li>
		</li>
	</ul>
	</div>
<p>
<span style='font-size: 1.2em; font-weight: 650;'>~/.pikrellcam/at-commands.conf</span>
	<div class='indent1'>
	This file is described below in its own section.
	</div>
<p>
<span style='font-size: 1.2em; font-weight: 650;'>~/pikrellcam/scripts/*</span>
	<div class='indent1'>
	Todo
	</div>
<p>
<span style='font-size: 1.2em; font-weight: 650;'>~/pikrellcam/www/config-user.php</span>
	<div class='indent1'>
	Edit this file to change web page appearance and some web page behavior.
	Some variables in this file can be modified by web page buttons and some
	require manual edits. If the file is edited, reload web pages
	to see the results.
	<ul>
	<li> The image used for the web page background can be changed to another
	PiKrellCam distribution image or to your custom background image you place
	in the <nobr>~/pikrellcam/www/images</nobr> directory. Any custom image
	name you create must begin with <span style='font-weight:700'>bg_</span>
	or else the image will be deleted by git when you do an upgrade.
	</li>
	<p>
	<li> Web page text colors can be changed.  If the background is changed to a
	darker image like <span style='font-weight:700'>images/passion.jpg</span> the text
	colors could be set brighter.  For example, you could try these changes:
<pre>
define("DEFAULT_TEXT_COLOR", "#ffffff");
define("SELECTED_TEXT_COLOR", "#500808");
define("MEDIA_TEXT_COLOR", "#0000EE");
define("MANUAL_VIDEO_TEXT_COLOR", "#085008");
define("LOG_TEXT_COLOR", "#ffffff");

define("BACKGROUND_IMAGE", "images/passion.jpg");
</pre>
	</li>
	<p>
	<li> The height of various scrolled views can be changed to match your normal browser
	size.
	</li>
	<p>
	<li> Other options such as <span style='font-weight:700'>VIDEO_URL</span>
	are not described here but are described in the file.
	</li>
	</ul>
	</div>
</div>


<span style='font-size: 1.5em; font-weight: 650;'>Media Files</span><hr>
<div class='indent0'>
PiKrellCam media files are videos or jpegs which are stored in media directories.
A media directory is a directory
with media file sub directories videos, thumbs, and stills.  All media directories are created
automatically when pikrellcam runs and media files are stored in the media directory
sub directories.
<p>
Each motion video will have a
corresponding thumbnail jpeg stored in the thumbs sub directory.  The motion video and its
thumb are created, deleted or archived as a unit.  The thumb jpegs are an image of motion
extracted from the video and provide a quick view of what caused the motion video.
<p>
<span style='font-size: 1.2em; font-weight: 650;'>Media Directories</span><br>
There are two links in the web page www directory
that pikrellcam uses to store and view media files and the defaults are:
<pre>
pi@rpi2: ~$ ls -lt pikrellcam/www
total 116
lrwxrwxrwx 1 $USER       $NGINX_GROUP    33 Nov 16 09:07 archive -> /home/pi/pikrellcam/media/archive/
lrwxrwxrwx 1 $USER       $NGINX_GROUP    25 Nov 16 09:07 media -> /home/pi/pikrellcam/media/
</pre>
The media link is the main media directory and its media file sub directories which contain a flat
list of files where media files for all days are initially stored.
Additionally, there is a timelapse and archive sub directory in the main media directory, but
these are not a requirement of a media directory and the archive directory location is configurable.
<br><br>
So the initial default main media directory contains:
<pre>
~/pikrellcam/media/videos
                  /stills
                  /thumbs
                  /timelapse
                  /archive
</pre>

The default media links
can be configured in <nobr>~/.pikrellcam/pikrellcam.conf</nobr>:
<pre>
media_dir media
archive_dir archive
</pre>

With this setup, all media files are stored on the Pi SD card.  Media files may be configured to
be stored on an external disk by editing the <nobr>~/pikrellcam/scripts/startup</nobr> file to uncomment
the line:
<pre>
MOUNT_DISK=sda1
</pre>
<p>
This assumes there is a single USB disk plugged into the Pi and it appears as
<nobr>/dev/sda</nobr>.
If this USB disk has a linux filesystem on
<nobr>/dev/sda1</nobr>, pikrellcam can create directories with the
needed permissions.  However, if the filesystem is not a linux filesystem
(eg. VFAT or FAT32) then pikrellcam cannot set up the needed permissions for
the web interface to work and media files will not be shown
unless the proper permissions are specified when
the partition is mounted.
For this case, the mount command or fstab entry
should specify umask or dmask/fmask permissions of 0002.
<p>
Also, the mount point can be somewhere else in the filesystem.  As an
example, you want a VFAT disk to be mounted on /media/mountdir.
For this, use absolute pathnames in pikrellcam.conf:
<pre>
media_dir /media/mountdir
</pre>
In the startup script, use a mount command like:
<pre>
sudo mount -t vfat /dev/sda1 /media/mountdir -o rw,user,umask=0002
</pre>
or, if using fstab instead of the startup script, the entry should be like:
<pre>
/dev/sda1   /media/mountdir    vfat    rw,user,umask=0002   0   0
</pre>
You can use dmask=0002,fmask=0002 in place of umask=0002.<br>
<p>
If mounting a large CIFS filesystem the nounix,noserverino options may
be needed in fstab so pikrellcam can make directories.  Look at the example
fstab entry forum
<a href="https://www.raspberrypi.org/forums/viewtopic.php?p=1123960#p1123960">
raspberry pi forum</a>
</div><br><br>

<a name="ARCHIVING">
<span style='font-size: 1.5em; font-weight: 500;'>Archiving</span><hr>
<div class='indent0'>
Archiving videos moves them out of the main media flat directory
into the archive tree directory where files are stored by day.
Archiving is useful for organizing large numbers of videos and can be a way
to move videos into a safer central location from one or more pikrellcam
camera installs.
After archiving, media files may be viewed by day, week, month or year by
clicking links on the calendar accessed through a web page "Archive Calendar"
button. Archiving of videos is done by clicking a web page "Archive" button to
operate on selected videos or by issuing archive commmands to the FIFO.
The archiving process may take time for large video files to be moved to new
directories, so videos may not immediately disappear from the
media videos page and appear on the archive pages.
<p>
By default, the archive directory is under the pikrellcam media directory
and so is on the same file system.  But archiving can be to either
a separate disk mounted on the archive directory (as described above
for mounting the media directory) or to another machine by network mounting
on the archive directory.
A network mount must have file system permissions set so that the pikrellcam
installing user and the nginx worker group have read/write permissions from the Pi.
<p>
<span style='font-size: 1.2em; font-weight: 650;'>NFS Archiving Example 1</span>
<div class='indent1'>
This is a minimal NFS setup for archiving from a Pi running pikrellcam to a
desktop machine on a LAN.  Setup is required on both the Pi and the desktop
and shown here is a specific example using gkrellm6 (my desktop), rpi5
(one of my Pis running pikrellcam) and my LAN IP addresses.  The same setup
as for rpi5 can be done for other Pis running pikrellcam and then multiple
Pis can store videos into the same archive.
You need your LAN working and change host names and IP addresses appropriate
for your setup.  nfs-kernel-server must be installed and the web has many
NFS tutorials you can refer to if you need more than these example steps.
<p>
<span style='font-weight:700'>On gkrellm6</span>
(Desktop running Linux - archiving to)
<ul>
	<li>In my home directory /home/bill, make an archive media directory
	to be NFS mounted by the Pi:
<pre>
$ cd
$ mkdir media-archive
</pre>
	</li>
	<li>Give permission for this directory to be exported to all
	other machines on my LAN by adding a line to /etc/exports:
<pre>
/home/bill/media-archive 192.168.0.0/25(rw,nohide,no_subtree_check,no_root_squash)
</pre>
After editing /etc/exports, restart nfs-kernel-server
(/home/bill/media-archive must exist):
<pre>
$ sudo systemctl restart nfs-kernel-server
</pre>
	</li>
</ul>

<span style='font-weight:700'>On rpi5</span>
(Pi running pikrellcam - archiving from)
<ul>
	<li> Edit /etc/hosts so from my Pi I can refer to my gkrellm6 desktop
	by name instead of IP address.  The line I use for my network:
<pre>
192.168.0.10    gkrellm6
</pre>
	</li>
	<li>
	For this example I will mount onto the default pikrellcam archive location
	so I don't have to edit archive_dir in pikrellcam.conf.  I leave it
	at its default which assumes
	<nobr>/home/pi/pikrellcam/media/archive</nobr>
	since there is no leading /:<br>
		&nbsp &nbsp <span style='font-weight:700'>archive_dir archive</span><br>
	</li>
	<p>
	<li>
	Add a line to /etc/fstab so I can NFS mount the gkrellm6 media-archive
	directory onto the pikrellcam archive directory. Use the archive_dir
	full path implied by the archive_dir value above:
<pre>
gkrellm6:/home/bill/media-archive /home/pi/pikrellcam/media/archive nfs users,noauto 0 0
</pre>
	</li>
	<li> NFS mount the gkrellm6 media-archive directory by hand or by script.
	The mount command will use the /etc/fstab line to mount the gkrellm6
	<nobr>/home/bill/media-archive</nobr> directory on the pikrellcam
	<nobr>/home/pi/pikrellcam/media/archive</nobr> directory.
	After mounting, running df will show the NFS mount and reloading
	web pages will show "NFS Archive Calendar" buttons.
<pre>
$ sudo mount gkrellm6:/home/bill/media-archive
</pre>
	</li>
	<li> You can use the pikrellcam ~/pikrellcam/scripts/startup script
	to mount the NFS archive directory when pikrellcam starts.
	Pikrellcam installs prior to V 4.1.5 did not have a NFS section in that
	startup script so you may have to copy the new
	<nobr>~/pikrellcam/scripts-dist/startup</nobr> over your existing
	<nobr>~/pikrellcam/scripts/startup.</nobr> and reconfigure
	MOUNT_DISK if you had previously done that.<br>
	Otherwise, configuration for NFS mounting must be as follows:
	<ul>
		<li> In ~/pikrellcam/scripts/startup, set NFS_ARCHIVE
		to match the /etc/fstab nfs mount line:<br>
		&nbsp &nbsp <span style='font-weight:700'>NFS_ARCHIVE=gkrellm6:/home/bill/media-archive</span>
		</li>
		<li> In pikrellcam.conf, set the on_startup command. The $a variable
		will be the archive_dir value configured in pikrellcam.conf and
		will become the archive_dir value in the script:<br>
		&nbsp &nbsp <span style='font-weight:700'>on_startup $C/startup $I $m $a $G</span>
		</li>
	</ul>
	</li>
</ul>
</div>

<p>
<span style='font-size: 1.2em; font-weight: 650;'>NFS Archiving Example 2</span>
<div class='indent1'>
This example is slightly more complicated and is my pikrellcam
archiving set up.  I have a USB SSD booted Pi3 desktop where I archive
videos from multiple Pis running pikrellcam.
My Pi3 desktop with a three partition USB SSD disk is rpi0,
and my Pis running pikrellcam are <nobr>rpi4, rpi5, ...</nobr>
The third partition on rpi0 is a large ext4 partition I use
for archiving various things.  Here I will archive my videos to that
partition into a media-archive subdirectory.
<p>
<span style='font-weight:700'>On rpi0</span> (Desktop Pi3 - archiving to)
<ul>
	<li> Make /mnt/archive and partition 3 /mnt/archive/media-archive directories:
<pre>
$ cd /mnt
$ sudo mkdir archive
$ sudo chown root.disk archive
$ sudo chmod 775 archive

# Mount partition 3 and make the media-archive subdirectory (my boot disk is sda).
$ sudo mount /dev/sda3 /mnt/archive
$ mkdir archive/media-archive
</pre>
	I want rpi0 partition 3 mounted at boot, so I have in /etc/fstab
	(I use PARTUUID in my fstab instead of sda for reliable mounting):
<pre>
PARTUUID=5d4064ac-01  /boot        vfat    defaults                  0       2
PARTUUID=5d4064ac-02  /            ext4    defaults,noatime,discard  0       1
PARTUUID=5d4064ac-03  /mnt/archive ext4    defaults,noatime,discard  0       2
</pre>
	</li>
	<li> Give export permission for /mnt/archive/media-archive to all on the LAN.
	Add to /etc/exports:
<pre>
/mnt/archive/media-archive 192.168.0.0/25(rw,nohide,no_subtree_check,no_root_squash)
</pre>
After editing /etc/exports, restart nfs-kernel-server (/mnt/archive/media-archive
must exist):
<pre>
$ sudo systemctl restart nfs-kernel-server
</pre>
</ul>

<span style='font-weight:700'>On rpi4, rpi5, ...</span> (Pis running pikrellcam - archiving from)
<ul>
	<li> Edit /etc/hosts so from each Pi I can refer to my rpi0 desktop
	by name instead of IP address.  The line I use for my network:
<pre>
192.168.0.30    rpi0
</pre>
	</li>
	<li> I don't want the pikrellcam archive directory left at the default
	location as <nobr>~/pikrellcam/media/archive</nobr> because
	I have all my pikrellcam media directories mounted with USB disks and
	I would rather not NFS mount into a USB mount.
	So, I set up similary to the structure I set up for rpi0 and
	have each pikrellcam archive to directory /mnt/archive/media-archive which
	will be NFS mounted (but this could be a directory in /home/pi if you
	prefer not to put it in /mnt).
<pre>
$ cd /mnt
$ sudo mkdir archive
$ sudo chown root.disk archive
$ sudo chmod 775 archive
$ mkdir archive/media-archive
</pre>
	Stop pikrellcam and edit archive_dir in ~/.pikrellcam/pikrellcam.conf,
	then restart pikrellcam.
<pre>
archive_dir  /mnt/archive/media-archive
</pre>
	</li>
	<li> Edit /etc/fstab with a line for NFS mounting the rpi0
	<nobr>/mnt/archive/media-archive</nobr> directory on the local
	<nobr>/mnt/archive/media-archive</nobr> directory.
	Add this line to /etc/fstab:
<pre>
rpi0:/mnt/archive/media-archive /mnt/archive/media-archive nfs users,noauto  0   0
</pre>
	</li>
	<li> NFS mount the rpi0 media-archive directory by hand or by script.
	The mount command will use the /etc/fstab line to mount the rpi0
	<nobr>/mnt/archive/media-archive</nobr> directory on the
	<nobr>/mnt/archive/media-archive</nobr> directory of the Pis
	running pikrellcam:
<pre>
$ sudo mount  rpi0:/mnt/archive/media-archive
</pre>
	</li>
	<li> You can use the pikrellcam startup script to mount the NFS
	directory as described in example 1.  In this case:
	<ul>
		<li> In ~/pikrellcam/scripts/startup, set NFS_ARCHIVE
		to match the mounting directory in the /etc/fstab line:<br>
		&nbsp &nbsp <span style='font-weight:700'>NFS_ARCHIVE=rpi0:/mnt/archive/media-archive</span>
		</li>
		<li> In pikrellcam.conf, in addition to the archive_dir set as
		above, set the on_startup command.  The $a variable
		will be the archive_dir value configured in pikrellcam.conf and
		will become the archive_dir value in the script:<br>
		&nbsp &nbsp <span style='font-weight:700'>on_startup $C/startup $I $m $a $G</span>
		</li>
	</ul>
	</li>
</ul>
</div>

<p>
<span style='font-size: 1.2em; font-weight: 650;'>NFS Archiving Notes</span>
<div class='indent1'>
<ul>
	<li> If the remote NFS server is slow to respond or down, a NFS mount
	command can appear to hang while the mount is retried.
	</li>
	<p>
	<li> A non responding NFS server can cause the web pages to be slow
	to load until either the server responds or the local kernel
	temporarily gives up and lists the mount as "Stale".  A stale or
	non responding NFS mount can be force unmounted with umnount -f or
	just wait until the remote file system comes back up.
	</li>
</ul>
</div>

<span style='font-size: 1.2em; font-weight: 650;'>Archive Directories</span>
<div class='indent1'>
The archive directory tree is by year, month and day with each month and day two digits:
<pre>
archive/2017/11/13
or
archive/2017/07/04
</pre>
Where 13 is a media directory for November 13.  It has the media file sub directories videos,
thumbs, and stills.  When you archive files from the web page,
the web server sends a command to pikrellcam to archive the files and the pikrellcam program
does the archiving.  Since pikrellcam runs as the user pi, pikrellcam has the sudo permission to
create the appropriate directory structure for archiving.
Since the web server runs as the user that nginx workers use, pikrellcam creates directories with write permission
for that user so files can be deleted from the web page. All directories in the archive
path have permissions like:
<pre>

pi@rpi2: ~/pikrellcam/www/archive$ ls -Rl
.:
total 0
drwxrwxr-x 3 $USER $NGINX_GROUP 60 Nov 12 12:16 2017/

./2017:
total 0
drwxrwxr-x 6 $USER $NGINX_GROUP 120 Nov 15 15:28 11/

./2017/11:
total 0
drwxrwxr-x 4 $USER $NGINX_GROUP 80 Nov 15 15:29 13/

./2017/11/13:
total 0
drwxrwxr-x 2 $USER $NGINX_GROUP 80 Nov 14 22:08 thumbs/
drwxrwxr-x 2 $USER $NGINX_GROUP 80 Nov 14 22:08 videos/
</pre>
Keep these permissions in mind if you manage the directory structure
outside of pikrellcam.
<br>
Archiving can be done by sending commands to the FIFO.  For example, to
archive all videos for Nov 13, 2017:
<pre>
echo "archive_video day 2017-11-13" > ~/pikrellcam/www/FIFO
</pre>
or a specific video (including its thumb) can be archived with:
<pre>
echo "archive_video motion_2017-11-05_14.46.14_456.mp4 2017-11-05" > ~/pikrellcam/www/FIFO
</pre>
To archive all media videos for today or yesterday:
<pre>
echo "archive_video day today" > ~/pikrellcam/www/FIFO
echo "archive_video day yesterday" > ~/pikrellcam/www/FIFO
</pre>
To archive all loop videos for today or yesterday:
<pre>
echo "archive_video day_loop today" > ~/pikrellcam/www/FIFO
echo "archive_video day_loop yesterday" > ~/pikrellcam/www/FIFO
</pre>
Stills may be script archived using the same set of arguments with the archive_still FIFO command.
</div>
</div>


<span style='font-size: 1.5em; font-weight: 650;'>FIFO Commands</span><hr>
<div class='indent0'>
From the web page, command line or scripts, PiKrellCam is controlled by sending commands to
a communication pipe named
<span style='font-weight:700'>FIFO</span> in the
<span style='font-weight:700'>~/pikrellcam/www</span> directory.
<p>
List of <span style='font-weight:700'>FIFO</span> commands:
<pre>
audio mic_open
audio mic_close
audio mic_toggle
audio gain [up|down|N]		# N: 0 - 30
audio stream_open
audio stream_close
audio_trigger_video [on|off]
audio_trigger_level N		# N: 2 - 100
box_MP3_only [on|off]
record on
record on pre_capture_time
record on pre_capture_time time_limit
record pause
record off
loop [on|off|toggle]
still
tl_start period
tl_end
tl_hold [on|off|toggle]
tl_show_status [on|off|toggle]
motion_enable [on|off|toggle]
motion_stills_enable [on|off|toggle]
motion_detects_fifo_enable [on|off|toggle]
motion limits magnitude count
motion burst count frames
motion trigger code    # code is digit N or N:ID    N is 0 or 1 and ID is a string (see Examples)
motion trigger code pre_capture time_limit
motion load_regions name
motion save_regions name
motion list_regions
motion show_regions [on|off|toggle]
motion show_vectors [on|off|toggle]
motion [command] - other commands sent by the web page to edit motion regions not
	intented for script or command line use.

preset prev_position
preset next_position
preset prev_settings
preset next_settings
preset goto position settings

zoom percent  - percent is 10-100

display [command] - commands sent by the web page to display OSD menus. Not intended for
	script or command line use.

tl_inform_convert
video_fps fps
video_bitrate bitrate
still_quality quality
video_mp4box_fps fps
inform "some string" row justify font xs ys
	echo inform \"Have a nice day.\" 3 3 1 > FIFO
	echo inform timeout 3
archive_video [day|day_loop|video.mp4] [today|yesterday|yyyy-mm-dd]
archive_still [day|video.mp4] [today|yesterday]yyyy-mm-dd]
annotate_text_background_color [none|rrggbb]   # rrggbb is hex color value 000000 - ffffff
annotate_text_brightness value   # value is integer 0 - 255, 255 default
annotate_text_size  value        # value is integer 6 - 160, 32 default
annotate_string [prepend|append] id string
annotate_string remove id
annotate_string spacechar c
fix_thumbs [fix|test]
delete_log
upgrade
quit

</pre>


<span style='font-size: 1.2em; font-weight: 650;'>Examples</span>
<div class='indent1'>
	<ul>
	<li>
	Motion detection
	can be toggled on/off from the web page or with the FIFO command:
<pre>
echo "motion_enable [on|off|toggle]" > ~/pikrellcam/www/FIFO
</pre>
	</li>
	<li>
	Still jpegs are created when a still command is sent to the FIFO.
<pre>
echo "still" > ~/pikrellcam/www/FIFO"
</pre>
	</li>
	<li>From the command line or a script, a manual video record can be managed with (a
	<span style='font-weight:700'>record on</span> after a <span style='font-weight:700'>record pause</span>
	resumes the existing record):
<pre>
echo "record on" > ~/pikrellcam/www/FIFO
...
echo "record pause" > ~/pikrellcam/www/FIFO
...
echo "record on" > ~/pikrellcam/www/FIFO
...
echo "record off" > ~/pikrellcam/www/FIFO
</pre>
	</li>
	<li>Start a manual record that will have up to 5 seconds of pre capture video.
	The pre capture time is subject to what is available in the video circular
	buffer, so if a record is started right after a previous manual or motion video
	has ended there may not be asked for pre capture time available.  Also the
	pre capture time is limited by the size of the video circular buffer which is
	a function of the greater of the motion detect Pre_Capture or Event_Gap times
	plus a 4 second margin.
	For example, if the Event_Gap is 10 seconds and Pre_Capture less than that,
	a manual record pre capture time of up to 14 seconds is possible.
<pre>
echo "record on 5" > ~/pikrellcam/www/FIFO
</pre>
	</li>
	<li>Start a manual record that will have up to 10 seconds of pre capture video and
	will have a maximum record time after the pre capture of 6 seconds.  So the total
	video will be 16 seconds long subject to pre capture time available in the video
	circular buffer.  The record time is wall time and does not consider pauses.
<pre>
echo "record on 10 6" > ~/pikrellcam/www/FIFO
</pre>
	</li>
	<li>
<a name="MOTION_TRIGGER">
	The motion trigger command is used to trigger a motion event from a script.
	It has two usages and the first is
	<nobr><span style='font-weight:700'>motion trigger code</span></nobr>
	where <span style='font-weight:700'>code</span> is a single digit
	<span style='font-weight:700'>N</span> motion enable code or
	<span style='font-weight:700'>N:ID</span>
	which adds a colon separated string to user identify
	the trigger type (PIR, laser interrupt, etc).
	If N is 0, the motion trigger is subject to the currently set
	motion enable.
	If N is 1, and the currently set motion enable is OFF, then force
	motion enable ON for this record event and recognize subsequent motion,
	external and audio triggers until the record event ends
	or there is a motion trigger command with N set to 0.
	If <span style='font-weight:700'>code</span> is omitted, the currently set motion
	enable applies and the code ID defaults to "FIFO" (for just a generic
	FIFO trigger).  The trigger code ID string is reported in
	the <nobr>/run/pikrellcam/motion-events</nobr> file so an on_motion_begin
	command can monitor for different external motion trigger types.
	See the file
	<nobr>scripts-dist/example-motion-events.</nobr><br>
	This is a trigger event that
	works in parallel with motion direction, burst and audio detection and
	it uses all the configured motion times and on_motion commands.
	The event gap time applies, so any detects or triggers after an
	initial FIFO trigger can keep the record event going.<br>
	Externally trigger a video or a motion still with FIFO commands:
<pre>
# Use the motion enable currently set, if it is OFF, no video or still.
echo "motion trigger" > ~/pikrellcam/www/FIFO"

# Force motion enable on for this FIFO command, trigger a record even if motion_enable is OFF.
#   If there are no other motion direction or burst detects for a motion video,
#   the web page thumb will be labeled with "FIFO".
#   A motion still thumb will be labled with "FIFO".
echo "motion trigger 1" > ~/pikrellcam/www/FIFO"

# Report code string "PIR" in motion-events file and use the motion enable currently set.
#   If there are no other motion direction or burst detects for a motion video,
#   the web page thumb will be labeled with "PIR".
$   A motion still thumb will be labeled with "PIR".
echo "motion trigger 0:PIR" > ~/pikrellcam/www/FIFO"
</pre>
	The second usage
	<nobr><span style='font-weight:700'>motion trigger code pre_capture time_limit</span></nobr>
	is a special case motion trigger event for motion videos (not motion stlls) that records
	a one shot motion video with a custom pre capture and time limit that overrides the
	configured motion times.  
	But If this FIFO command is given when there is already
	a motion video in progress (or if recording motion stills),
	the custom times are not used.  The trigger command then reduces to its first
	use case as if no custom times were given.  For this example, assume that the
	configured motion enable is either off or, if on, that no other video is
	in progress.  Then issue a motion trigger command that
	forces motion enable on and records a
	motion video with a 4 second pre_capture and a 5 second time_limit for
	a total video length of 9 seconds (subject to the same circular buffer
	constraints as the record FIFO command):
<pre>
echo "motion trigger 1 4 5" > ~/pikrellcam/www/FIFO"

# Report code string "laser" in the motion-events file.
#   If there are no other motion direction or burst detects for this video,
#   the web page thumb will be labeled with "laser".
echo "motion trigger 1:laser 4 5" > ~/pikrellcam/www/FIFO"
</pre>
	</li>
	<li>The <span style='font-weight:700'>fix_thumbs</span> command provides
	a consistency check/repair function for video and thumb files.  It creates
	missing thumbs for motion and manual videos and any motion thumbs will have
	"Recovered" text annotated on them to indicate that they are not original
	motion area thumbs.
	Also thumbs with no video and stray .h264 files will be deleted.
	<br>
	This command exists as a recovery path for
	<ul>
	<li>A bug around PiKrellCam 2.0.0: deleting a thumb did not delete its video.
	</li>
	<li> Before version 2.1.10: there could be stray h264 files if the media file system ran out of space.
	</li>
	<li>Before version 2.1.9: manual and timelapse videos did not generate thumbs.
	</li>
	</ul>
	First, use the "test" argument and watch
	the log page (reload it) until you see "DONE" to see if there are any problems.
	If the log lists any repair actions you want, redo the command with the
	"fix" argument.  If fixing, this command can take some time to complete.
<pre>
echo "fix_thumbs test" > ~/pikrellcam/www/FIFO
echo "fix_thumbs fix" > ~/pikrellcam/www/FIFO
</pre>
	</li>
	<li>The <span style='font-weight:700'>annotate_string</span> command dynamically
	appends, prepends or removes a string to the date string text drawn on videos.
	Here is a demo example script.  Copy the text to a file, make the file executable
	and run it while watching the OSD (a '_' is the default space character for strings
	and in this example adds a space after the hostname and before the temperature):
<pre>
#!/bin/bash
echo annotate_text_background_color 808080 > ~/pikrellcam/www/FIFO
echo annotate_text_size 42 > ~/pikrellcam/www/FIFO

# use the ds18b20 script if you actually have a ds18b20 connected to your pi.
# TEMP=`read_ds18b20`, otherwise simulate a temperature.
TEMP="29.1C"
echo annotate_string append id1 _$TEMP > ~/pikrellcam/www/FIFO; sleep 3
echo annotate_string prepend id2 ${HOSTNAME}_ > ~/pikrellcam/www/FIFO; sleep 3
TEMP="30.4C"
echo annotate_string append id1 _$TEMP > ~/pikrellcam/www/FIFO; sleep 3
echo annotate_string remove id2 > ~/pikrellcam/www/FIFO; sleep 3
echo annotate_string remove id1 > ~/pikrellcam/www/FIFO

echo annotate_text_background_color none > ~/pikrellcam/www/FIFO
echo inform \"End of annotate_string demo\" 3 3 1 > ~/pikrellcam/www/FIFO
echo inform timeout 3 > ~/pikrellcam/www/FIFO
</pre>
	</li>
	Multiple scripts may independently display their own data or information strings in the
	annotation text as long as they use unique ids.
	</ul>
</div>

<span style='font-size: 1.5em; font-weight: 650;'>At Commands</span><hr>
<div class='indent0'>
PiKrellCam has a built in command scheduler for periodic command execution at a particular
time.
The at commands are a list of commands in the configuration file
<span style='font-weight:700'>~/.pikrellcam/at-commands.conf.</span>
and if this file is modified, PiKrellCam will automatically reload it so there is no
need to restart.
<p>
Command lines in the file have the form:
<pre>
frequency  time   "command"
</pre>
	<ul>
	<li>
		<span style='font-weight:700'>frequency</span> - possible values:<br>
		&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>daily Mon-Fri Sat-Sun Mon Tue Wed Thu Fri Sat Sun
		</span
	</li>
	<p>
	<li>
		<span style='font-weight:700'>time</span> - possible values:<br>
		&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>hh:mm start dawn sunrise sunset dusk
			hour minute 5minute 10minute 15minute 30minute
		</span>
		<p>
		A minute offset may be added/subracted from the times
		<span style='font-weight:700'>dawn sunrise sunset dusk.</span><br>
		If the time is <span style='font-weight:700'>start</span>, the command is executed
		once at startup on the day specified
		(usually <span style='font-weight:700'>daily</span>)
		For all other times, the commands execute when the time occurs while PiKrellCam
		is running.  This means if you need an initial setting based on a specific
		time period (say started during the day versus started at night) when
		PiKrellCam is started, you need to have your own script that checks the time.
		Such a script can parse the PiKrellCam state file
		<span style='font-weight:700'>/run/pikrellcam/state</span>
		to get the
		<span style='font-weight:700'>current_minute</span>
		and compare it to todays sun times from the state file:
		<span style='font-weight:700'>dawn sunrise sunset dusk</span>.
		Other values in the state file can be used for special actions in at
		command scripts.
		<p>
		For the sun times to work, edit
		<span style='font-weight:700'>~/.pikrellcam/pikrellcam.conf</span> and set the
		<span style='font-weight:700'>latitude</span> and
		<span style='font-weight:700'>longitude</span> values to your location.  Check
		the state file or the log file to check the calculated times.
	</li>
	<p>
	<li>
		<span style='font-weight:700'>command</span>
		- a system command/script or an internal pikrellcam command if the command
		is preceeded with the '@' character.  Commands must be enclosed in quotes.
	</li>
	<p>
	<li> Prepend an '!' character to the command if you don't want it logged.
		See ds18b20.py example below.
	</li>
	</ul>

	Command strings may have substitution variables which are expanded by PiKrellCam
	prior to being executed.  Substitution variables are:

<pre>
C - script commands directory full path
I - the PiKrellCam install directory
a - archive directory full path
m - media directory full path
M - mjpeg file full path
P - command FIFO full path
G - log file full path
H - hostname
E - effective user running PiKrellCam
V - video files directory full path
t - thumb files directory full path
v - last video saved full path filename
S - still files directory full path
s - last still saved full path filename
L - timelapse files directory full path
l - timelapse current series filename format: tl_sssss_%%05d.jpg
    in timelapse sub directory.  If used in any script
    arg list, $l must be the last argument.
T - timelapse video full path filename in video sub directory
N - timelapse sequence last number
D - current_minute dawn sunrise sunset dusk
Z - pikrellcam version
</pre>
If the command string is an internal @command, only the $H substitution variable is
recognized.
<p>
<span style='font-size: 1.2em; font-weight: 650;'>Examples</span>
<div class='indent1'>
<ul>
<li>
At each PiKrellCam startup
	<ol>
	<li>Goto a preset.
	</li>
	<li> Prepend the hostname to the annotated text date string drawn on each video
	<br>
	(<span style='font-weight:700'>start1</span> is just an id string and
	the '_' character is replaced with a space).
	</li>
	</ol>
<pre>
# If no servos, goto position 1 (only 1 position possible with no servos) settings 1:
daily  start  "@preset goto 1 1"
# If servos, goto position 3 settings 1
daily  start  "@preset goto 3 1"
daily  start  "@annotate_string prepend start1 $H_"
</pre>
</li>
<li>
Turn motion detect off during a time of day when motion videos are not wanted:
<pre>
Mon-Fri 16:30   "@motion_enable off"
Mon-Fri 18:00   "@motion_enable on"
</pre>
</li>
<li>
Schedule a timelapse to run Monday through Friday with a period of 60 seconds.  Put
it on hold during the night and turn the OSD time lapse display on every morning
and at time lapse end as a reminder the time lapse is running or converting.
<pre>
Mon      sunrise-5 "@tl_start 60"
Fri      sunset+5  "@tl_end"
Mon-Fri  sunset+5  "@tl_hold on"
Mon-Fri  sunrise-5 "@tl_hold off"
Mon-Fri  sunrise   "@tl_show_status on"
Fri      sunset+5  "@tl_show_status on"
</pre>
</li>
<li>
Each day at 8:00 AM, archive all videos from yesterday:.
<pre>
daily    08:00  "@archive_video day yesterday"
</pre>
Or you could have a custom archive script that does something more complicated like
archiving and cleaning out old files. For example,
use something like this
<a href="https://www.raspberrypi.org/forums/viewtopic.php?p=847252#p847252">forum example</a>
and have the at-command:
<pre>
daily    23:00  "$C/do-archive"
</pre>
	The do-archive script could be run as an at-command as shown or cron could be used..
	</li>
	<li>
	If you have ds18b20 temperature chips connected, append temperature readings
	to the video annotated text date string.
	The ds18b20.py script is in the PiKrellCam scripts directory and can be edited
	to add labels to temperature values.  The 'F' reports fahrenheit.  Use
	'C' for centigrade.
	This example prepends an '!' to the
	command to disable logging, otherwise the log file gets spammed every
	minute.
<pre>
daily minute "$C/!ds18b20.py F fifo"
</pre>
	</li>
	</ul>
</div>
</div>

<span style='font-size: 1.5em; font-weight: 650;'>State Files</span><hr>
<div class='indent0'>
<span style='font-size: 1.2em; font-weight: 700;'>/run/pikrellcam/state</span>
	<div class='indent1'>
	PiKrellCam internal state can be read from this file.  Look at it to see
	what information is available.  For example, if you have a bash script that
	needs to know if the motion_enable current state is "on" or "off":
<pre>
line=`grep motion_enable /run/pikrellcam/state`
motion_state=${line#motion_enable}
</pre>
	The motion_state variable will then be "on" or "off".
	When motion_enable is FIFO changed, the new
	state should show up in the /run/pikrellcam/state file
	within around 100-200 msec.
	</div>

<a name="MOTION_EVENTS">
<span style='font-size: 1.2em; font-weight: 700;'>/run/pikrellcam/motion-events</span>
	<div class='indent1'>
	This file is for processing motion detects during a motion video
	or stills record event
	in real time as they occur while recording is in progress.  It is intended
	to be processed by an on_motion_begin command configured in
	pikrellcam.conf.
	<p>
	See the motion_detects_FIFO below if you want to process a continuous
	stream of motion detects whether or not a motion event is recording.
	<p>
	This file is overwritten with new detect data for each new motion
	recording and the on_motion_begin command is run 
	immediately after the motion-events file writing begins.
	The output to the file is
	flushed after data for each motion detect is written.
	See script-dist/example-motion-send-alarm2 for an example reading of this
	file in an on_motion_begin script.
	<p>
	Scripts can determine
	where motion is in the video frame (by x,y position or motion region
	number) and then take some action such as sending multicast alarms and/or
	moving servos.
	<p>
	The format of the file is a header block followed by one or more
	motion blocks and a final end tag.
	Inside of motion blocks are the data
	for each detect during the video.
	<p>
	For example:
<pre>
&lt;header&gt;
to be documented...
&lt;/header&gt;
...
&lt;motion  3.667&gt;
f  49  43  57  -2  57  263
1  44  42  53  -4  53  144
2  55  44  61   0  61  119
&lt;/motion&gt;
&lt;motion  4.100&gt;
f  10  36  69  52  86  290
b 949
&lt;/motion&gt;
&lt;motion  5.120&gt;
f   0   0   0   0   0    0
e PIR
&lt;/motion&gt;
&lt;motion  6.000&gt;
f   0	0   0	0   0	 0
a 45
&lt;/motion&gt;
...
&lt;end&gt;
</pre>
	shows data for a first detect at 3.667 seconds into the video
	(including precapture).
	Each line inside a motion block begins with a single character code:
	<ul>
	<li> <span style='font-weight:700'>f x y dx dy magnitude count</span>
	- where the first character code is 'f'.<br>
	This line is the data for
	the total frame vector (composite of all the motion region vectors).
	There is always a frame vector reported but may be a zero vector for
	audio or external only detects.  A non-zero frame vector passes
	the configured magnitude and count limits and there can be a passing
	frame vector without any passing region vectors.
	</li>
	<p>
	<li> <span style='font-weight:700'>n x y dx dy magnitude count</span>
	- where the first character code n is a digit.<br>
	These lines are for motion vectors for a motion region detects.
	There will be a line for each region having motion and no line for
	regions not having motion.  Just like the overall frame vector, for a
	motion region to have motion, the configured magnitude and count limits
	must be met.
	For this detect there was motion in regions 1 and 2.
	</li>
	<p>
	<li> <span style='font-weight:700'>b</span> - this line shows burst counts.
	If no regions individually passed detection magnitude and count limits,
	the overall frame vector must pass for a burst count detect so a burst
	detect with no region detects still always has a frame vector.
	</li>
	<p>
	<li> <span style='font-weight:700'>a level</span>
	- shows an audio trigger if an audio level exceeded the configured
	audio_trigger_level value.
	If there was only an audio trigger, then the overall
	motion frame vector 'f' line will show a zero vector.
	</li>
	<li> <span style='font-weight:700'>e code</span>
	- shows there was an external trigger (motion trigger command into the
	FIFO).  The code will either be "FIFO" or a custom code supplied in
	the external trigger command.
	If there was only an external trigger, then the overall
	motion frame vector 'f' line will show a zero vector.
	</li>
	</div>
The end tag is written when the motion video ends.
</div>

<a name="MOTION_EVENTS_FIFO">
<span style='font-size: 1.5em; font-weight: 650;'>Motion Detects FIFO</span><hr>
<div class='indent0'>
&nbsp;&nbsp;<span style='font-size: 1.2em; font-weight:700'>~/pikrellcam/www/motion_detects_FIFO</span>
<p>
This named pipe fifo is for near real time processing of all PiKrellCam
motion detects regardless of motion recording enabled state and so
can be a general purpose motion detect front end interface for a user
application with its own motion detect policy.  If motion video recordings are
enabled and there is a configured non-zero confirm gap, then this fifo will
report motion events that do not trigger a motion video if the second
confirming motion detect required for video recordings did not happen.
<p>
This fifo is not intended for use by an on_motion_begin command.
See the
<nobr><span style='font-weight:700'>/run/pikrellcam/motion-events</span></nobr>
file section above for motion detect processing on a per motion record event
basis via an on_motion_begin command.
<p>
To enable or disable writing all motion detects to the motion_detects_FIFO,
send to the command FIFO:

<pre>
echo "motion_detects_fifo_enable [on|off|toggle]" > ~/pikrellcam/www/FIFO
</pre>

Motion detect data blocks are written to the motion_detects_FIFO in the same
format as is written into the
<nobr><span style='font-weight:700'>/run/pikrellcam/motion-events</span></nobr>
file as described above except that the time in the motion tag will be
the current system time in seconds (with .1 second precision) instead of
the time elapsed into a video.
Also this is a continuous stream of motion blocks with no header blocks
written.
If the motion_detects_FIFO reading app needs any information other than
motion detects, it should read the
<nobr><span style='font-weight:700'>/run/pikrellcam/state</span></nobr>
file.
<p>
If the enable is "on" it will stay enabled across pikrellcam
restarts until an "off" command or pikrellcam.conf is edited.  Once enabled,
pikrellcam tries to write all motion detects into the motion_detects_FIFO
and an external app can read the detects from the motion_detects_FIFO.
The external app can be started by hand, by a command in the
at-commands.conf file, or by cron.
<br>
When a "motion_detects_fifo_enable off" command is sent to the command FIFO,
an &lt;off&gt; tag is written to the motion_detects_FIFO so the user app can
know the motion_detects_fifo_enable has been turned off.
<p>
Read the example script
<span style='font-weight:700'>~/pikrellcam/scripts-dist/example-motion-detects-fifo</span>
for more information. If this script is run by hand from a terminal,
a stream of all motion detects is printed.
<p>
To test the example script on a Pi running pikrellcam, open a terminal
(SSH terminal if the Pi is headless) and run the example script:<br>

<pre>
    $ ~/pikrellcam/scripts-dist/example-motion-detects-fifo
# The script enables motion_detects_FIFO and motion detects are printed here.
# Terminate the script with ^C or send an off command in another termial:
#    $ echo "motion_detects_fifo_enable off" > ~/pikrellcam/www/FIFO
</pre>
Commands in at-commands.conf can coordinate times when you want
PiKrellCam to have motion recordings and times when you might want to run
a custom motion detect app that reads from the motion_detects_FIFO.
As is done in the example-motion-detects-fifo script, your app can enable
the motion_detects_FIFO or it can be enabled with an at-commands.conf command
before or after your script starts.
And an at-command can turn the motion_detects_FIFO off at a certain time
if you want to signal your app to self terminate.
</pre>
</div>

<a name="MULTICAST_INTERFACE">
<span style='font-size: 1.5em; font-weight: 650;'>Multicast Interface</span><hr>
<div class='indent0'>
The multicast interface provides a means for PiKrellCams and desktops
on a LAN to coordinate state and motion events by one to many message
sending and script running.
The coordination control
implementation can be distributed among the PiKrellCams or the control can
be centralized on a desktop program that receives multicasts from the
PiKrellCams and issues commands or messages back to them.
A system could be as simple as PiKrellCam motion event alarm messages
sent to a desktop that then triggers an audio
alarm or some kind of GPIO event.   Or a more complex setup could
coordinate motion recording and moving cameras on servos to positions
depending on motion events throughout the network.
<p>
Multicasting is a network group communication protocol that
uses the UDP transport layer which is inherently unreliable
(messages may be lost).  To address this, the PiKrellCam multicast
implementation provides for message retransmission and tagging messages
with id numbers that can be acknowledged so loss detection can be implemented.
Using either of these mechanisms is optionally up to the user scripts or
programs that implement the PiKrellCam LAN multicast system.
<p>
PiKrellCam uses a fixed group network IP and port number for multicast
communication which user programs must use:
<pre>
PKC_MULTICAST_GROUP_IP    225.0.0.55
PKC_MULTICAST_GROUP_PORT  22555
</pre>
The scripts-dist directory has examples which can be used as is
or as a starting point for more complex programs.  To use these pkc-xxx
programs, copy them to a bin directory on desktops you will run them
from and then they can be run from a terminal or script:
<ul>
	<li>
	<span style='font-weight:700'>pkc-motion</span> - turn motion on or off
	for one or many PiKrellCams on a LAN.
<pre>
# Turn off motion detection for all PiKrellCams
pkc-motion all off
# or, "all" is assumed
pkc-motion off

# Turn on motion detection for two cameras
pkc-motion rpi2,rpi4 on
</pre>
	</li>
	<li>
	<span style='font-weight:700'>pkc-reboot</span> - if reboot/halt is
		enabled, reboot a Pi from a LAN desktop terminal.
<pre>
pkc-reboot rpi1
</pre>
	</li>
	<li>
	<span style='font-weight:700'>pkc-alarm</span> - This file must be
	edited so it can play an audio file on your machine.
	Then, if pkc-alarm is running on a desktop,
	it will play a sound when it receives a
	multicast alarm message from a PiKrellCam on your LAN.
	To test playing the audio file, run pkc-alarm in a terminal and then
	in another terminal run
	<nobr>scripts-dist/example-motion-send-alarm1.</nobr>
	This should work with the terminals on different machines on your LAN.
	And for further debugging, run pkc-recv in a third terminal.
	<p>
	On each Pi you want PiKrellCam to send alarm messages from, the setup is:
	<ol>
		<li> Copy the example-motion-send-alarm1 or example-motion-send-alarm2
		from the scripts-dist directory to
		<nobr>scripts/motion-send-alarm.</nobr>
		If using the alarm2 example, edit it to select which region you want
		to detect motion in and edit it to set the motion magnitude and count
		limits you want.
		<p>
		Note: the example-motion-send-alarm2 is likely a work in progress.
		It is an example of reading the
		<nobr>/run/pikrellcam/motion-events</nobr> file
		while a motion video recording is in progress and
		sending an alarm only if motion exceeding a magnitude and count limit
		is detected in a particular region.
		</li>
		<li> Edit pikrellcam.conf and set the on_motion_begin command to:
<pre>on_motion_begin  $C/motion-send-alarm
</pre>
		</li>
	</ol>
	</li>
	<li>
	<span style='font-weight:700'>pkc-recv</span> - run this in a terminal
		if you need to debug a PiKrellCam  multicasting installation or
		development.  It just prints all the network traffic
		sent in the PiKrellCam multicast group.
	</li>
</ul>
Other example scripts may be added over time and the pkc_xxx and example_xxx
scripts-dist scripts may or not be modified.
<p>
<span style='font-size: 1.2em; font-weight: 650;'>Protocol</span>
<div class='indent1'>
Programs that send information to the PKC multicast group should open
a sending socket (see example pkc-xxx programs) and send a message which is
a line of text characters ending in '\n'.  The format of that text message
should have at least four space separated fields with the message body
optionally having additional space separated fields:
<pre>
from_host to_hosts message_type message_body
</pre>
<ul>
	<li><span style='font-weight:700'>from_host</span> - the hostname of the
	sending machine.  A message id > 0 may be appended to the
	name after a colon - <span style='font-weight:700'>from_host:id</span>
	which is used when messages are repeat sent for transmission
	reliability.  Receiving programs should detect if the same id > 0
	from the same host is sent multiple times within a second or two
	and accept only one of the messages.  Multiple distinct messages that are
	sent quickly should have different message ids.
	</li>
	<p>
	<li><span style='font-weight:700'>to_hosts</span> - a single hostname
	or a comma separated
	list of hostnames that the message is being sent to.
	Use the keyword <span style='font-weight:700'>all</span> to send to
	all programs listening on the PKC multicast group.  Every PiKrellCam
	program is always listening for messages of certain message types
	addressed to its hostname.
	</li>
	<p>
	<li><span style='font-weight:700'>message_type</span> - a keyword that
	indicates what is being sent in the message_body.  User applications
	can create their own types and should ignore messages of types they are
	not interested in.  PiKrellcam recognizes receiving message types
	<span style='font-weight:700'>command</span> and
	<span style='font-weight:700'>pkc-message</span> and sends a message type
	<span style='font-weight:700'>ack</span> in response, which is so far
	not used in any of the example scripts..
	The pkc-alarm script recognizes the message type
	<span style='font-weight:700'>message</span> and that is the type sent
	by the example_motion_send_alarm scripts.
	</li>
	<p>
	<li><span style='font-weight:700'>message_body</span> - a text string
	appropriate for and interpreted according to the message type.
	The message body may contain additional space separated text fields
	as appropriate for the message type.
	</li>
</ul>
Use the message type
<span style='font-weight:700'>command</span> and the message body will
be interpreted as a command to execute by PiKrellCam.
If the '@' char is prepended, the message body will be a PiKrellCam
internal command as if it was a FIFO command.  Otherwise it will
be a script to run.
The <span style='font-weight:700'>pkc-message</span> type is a way to
get PiKrellCam to run a script that needs some internal variables passed
to it.  Its function is user defined and would be configured in
pikrellcam.conf.
</div>

</div>
</div>
</div>
</div>
<?php
echo "<div style='margin-top:12px;'>";
echo "<a href='index.php' class='btn-control'
		style='margin-left:8px;'>
		$title</a>";
echo "</div>";
?>
</body>
</html>
