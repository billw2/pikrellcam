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

echo "<div style='color: $default_text_color; margin-left: 1cm; margin-right: 2cm'>"
?>

<span style='font-size: 1.5em; font-weight: 650;'>Introduction</span><hr>
<div class='indent0'>
For an overview description of PiKrellCam, visit the
<a href="http://billw2.github.io/pikrellcam/pikrellcam.html">PiKrellCam website</a><br>
And there is a Raspberry Pi
<a href="https://www.raspberrypi.org/forums/viewtopic.php?f=43&t=115583">PiKrellCam forum</a>
<p>
Under construction...
</div>

<span style='font-size: 1.5em; font-weight: 650;'>Release Notes</span><hr>
<div class='indent0'>
Version 3.1
<div class='indent1'>
<a href="help.php#MULTICAST_INTERFACE">multicast interface</a><br>
<a href="help.php#MOTION_EVENTS">motion-events file</a><br>
</div>

</div>
Version 3.0
<div class='indent1'>
The upgrade from PiKrellcam V2.x to PiKrellCam V3.0 adds presets and servo control.<br>
This is documented below on this page, but there are changes in how
motion regions usage is handled that is important to be aware of up front:
<ul>
	<li> The use of saving and loading of motion regions by name is no longer the primary
	way to change the motion regions in effect.  Saving and loading regions by name now
	has a new role of maintaining a set of motion regions as temporaries for backup or
	using as an initial condition to be loaded when creating a new preset.
	</li>
	<li> If motion regions are edited or a new set loaded by name, the changes are
	automatically stored into the current preset (unless you have servos and are off
	a preset - see below).  So if you edit motion regions the current preset is
	changed and there is no need to save by name unless you want the backup.
	</li>
	<li> When pikrellcam is restarted, it loads the preset you were on when pikrellcam
	stopped.  If for example, you have a preset 1 set up for default use and a preset 2
	for windy conditions, then if you want to be sure that at program start preset 1 is
	selected, you should have in at-commands.conf:
<pre>
daily  start  "@preset goto 1 1"
</pre>
	If you have servos, you might want to have a position number other than "1".<br>
	This would replace the use of an at command to load regions and if you have such
	a startup motion load_regions command, you probably want to take that out.  If you
	don't take it out and don't have a startup preset goto, the regions will be loaded
	into whatever preset you restart with which can be not what you want.
	If you do use a startup
	preset goto command, also having a startup motion load_regions is likely redundant
	because the preset remembers its motion regions.
	</li>
</ul>
</div>
</div>


<span style='font-size: 1.5em; font-weight: 650;'>Install</span><hr>
<div class='indent0'>
PiKrellCam is installed from a github git repository using the command line. The install
is cloning the repository in the /home/pi directory and running the install script
(an install for a user other than pi is possible):
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
	<li>Auto start: if enabled a line will be added to
	<span style='font-weight:700'>/etc/rc.local</span> so that pikrellcam will be
	auto started at boot.  If this is not enabled, pikrellcam will need to be started from
	the web page or from a terminal after each boot.
	</li>
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
	<li> After the preview image appears, turn on motion detection by clicking the button
		<span style='font-weight:700'>Enable:</span> <span class='btn-control'>Motion</span>
	</li>
	<li>The OSD then shows that motion detection is
		<span style='font-weight:700'>ON</span>
		and PiKrellCam is now operating with its default settings.
	</li>
	<li>Wait for motion to be detected and watch the OSD for the video record progress.<br>
		After the video ends, view it by going to the Videos page by clicking:
		<span style='font-weight:700'>Media:</span> <span class='btn-control'>Videos</span>
	</li>
	<li>On the button bar, click the buttons
		<span style='font-weight:700'>Show:</span>
		<span class='btn-control'>Preset</span>
		<span class='btn-control'>Timelapse</span>
		<span class='btn-control'>Vectors</span>
		<br>to toggle showing information PiKrellCam can display on the OSD.
		When you show Preset, you see the currently configured motion detection vector
		and burst values and the motion detect regions in effect.  See below.
	</li>
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
	<li>PiKrellCam runs as user pi for simplified install and other benefits
	(but a default user other than pi is possible).
	PHP files are run via the web server nginx and so run as the user www-data.
	The install script created a <nobr><span style='font-weight:700'>/etc/sudoers.d/pikrellcam</span></nobr>
	file to give the user www-data permission to start pikrellcam as user pi from the web page.
	If the option to run pikrellcam at boot was selected, a line to start pikrellcam as user pi
	was added to <nobr><span style='font-weight:700'>/etc/rc.local</span></nobr>
	</li>
	<li>The install script creates
	<span style='font-weight:700'>/etc/nginx/sites-available/pikrellcam</span> where listen port
	number and password protection for the web pages are configured.
	</li>
	<li>Several PiKrellCam internal control paths are not shown in the above diagram.
	The "command processing"
	block can control the video record, still, time lapse and other sub systems.
	</li>
	<li>A simple feed of the preview jpeg can be viewed without control panels or button/control bars.<br>
	Just point your browser to:
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://your_pi:port_num/live.php</span>
	<br>or
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://pi:password@your_pi:port_num/live.php</span>
	<br>where port_num is the nginx port configured in the install.  If the port was left at
	the default 80, you can omit the <span style='font-weight:700'>:port_num</span> from the URL.
	</li>
	<li>An alternate way to view the preview is with a tcp stream connection which additionally
	allows viewing using vlc or Android MJPEG viewer apps such as tinycam monitor, etc.
	Open the MJPEG network stream using the URL:
	<br>&nbsp;&nbsp;&nbsp;<span style='font-weight:700'>http://pi:password@your_pi:port_num/mjpeg_stream.php</span>
	</li>
	<li>To be able to view the h264 tcp video stream some extra install and
	setup are required.  Follow the
	<nobr><a href="https://www.raspberrypi.org/forums/viewtopic.php?p=862399#p862399">
	rtsp live video setup instructions</a></nobr> on the forum and you can view the stream
	with vlc.
	</li>
	</ul>
</div>
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
<img src="images/vector0.jpg" alt="vector0.jpg"> 
<p>
<span style='font-size: 1.2em; font-weight: 680;'>Notes:</span>
<ul>
	<li>Image 2 fails the overall motion detect because PiKrellCam tries to be
		discriminating against noise.  This is how it can run with very low magnitude
		and count limits and be good at detecting small animals.  The low density
		distribution of vectors in region 2 that caused the failure can be typical of
		scenes with wind blown trees and grass.
	</li>
	<li>Image 4 shows a burst detect overriding a direction detect failure.
		The point to take from this is that if the burst count limit is set
		too low for a given camera environment, the burst detect method is likely
		to detect more noise than the direction detect method.
	</li>
	<li>Sparkles are camera motion vectors that have no neighbors and PiKrellCam
		considers them noise and excludes them from the composite vectors.
	</li>
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
			<li><span style='font-weight:700'>rej:22</span> There were 22 total frame vectors
			that passed the magnitude limit count but failed the direction compare test to
			their region composite vector.  They were excluded from any region final motion
			direction detect test.
			They were not excluded from the burst detect count test.
			</li>
			<li><span style='font-weight:700'>spkl:52 (53.5)</span> there were 52 vectors
			that passed the magnitude test but did not have any neighbors.  So they were
			ecluded from any motion detect test.  The sparkle moving average (53.5) is
			currently not used in motion detect testing.
			</li>
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
			<ul>
			<li><span style='font-weight:700'>Vector_Magnitude</span> - sets the minimum magnitude
			of a motion vector.  Individual motion vector and region composite vector
			magnitudes less than this value will not be considered valid motion.
			Set the value lower to increase sensitivity to slower moving objects.  Slow walking
			cats or people at a distance will probably require low settings of 7 or less while
			faster objects like cars or closer people can detect with higher settings.
			This value applies to composite vectors used for both direction and burst detects.
			</li>
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
			</ul>
		</li>
		<li><span style='font-weight:700'>Move: One</span><br>
			You will have this button only if servos are configured.<br>
			If the servos are moved off a preset, click this if you want to move
			the preset you were on to the current servo position.
		</li>
		<li><span style='font-weight:700'>Move: All</span><br>
			You will have this button only if servos are configured.<br>
			If the servos are moved off a preset, click this if you want to move
			the preset you were on to the current servo position and move all the other preset
			positions by the same amount.  If the camera installation is disturbed or serviced,
			this allows a quick adjustment for restoring position presets.  The other presets
			may still need small adjustments if servo positioning is non linear.  All presets
			cannot be moved if the move would move any preset past a servo position limit.
		</li>
		<li><span style='font-weight:700'>Del</span><br>
			If servos are not configured or if the servo position is on an existing preset, delete
			the current Settings.  If servos are configured and the servo position is on a preset
			and the Settings are the last Settings for the preset, then delete the position preset
			unless it is the only existing position preset.  There must always be at least one
			preset and you cannot delete down to zero presets.
		</li>
		<li><span style='font-weight:700'>Copy</span><br>
			You will have this button only if servos are configured.<br>
			If the pan servo is moved off a preset, click this to create a
			new preset at the servo position which is initiallized by copying all of
			the preset settings (motion detect limits and regions) from the preset you
			were on into the new preset.
		</li>
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
	<ul>
		<li><span style='font-weight:700'>Video Res</span> - selects the video resolution for
		motion and manual videos.  Different resolutions may have different fields of view. So
		one reason for selecting
		<span style='font-weight:700'>720p</span> over
		<span style='font-weight:700'>1080p</span> would be to get a wider field of view.  Resolutions
		will have either 16:9 or 4:3 aspect ratio.
		</li>
		<li><span style='font-weight:700'>Still Res</span> - selecting different resolutions
		gives different fields of view and aspect ratios.
		</li>
		<li><span style='font-weight:700'>Settings</span>
			<ul>
			<li><span style='font-weight:700'>Startup_Motion</span> - set to
			<span style='font-weight:700'>ON</span> for motion detection to be enabled each time
			PiKrellCam starts.  If set to
			<span style='font-weight:700'>OFF</span>, motion detection will need to be manually
			enabled from the web page or a script.
			</li>
			<li><span style='font-weight:700'>video_bitrate</span> - determines the size of a video
			and its quality.  Adjust up if it improves video quality.  Adjust down if you want
			to reduce the size of the videos.
			</li>
			<li><span style='font-weight:700'>video_fps</span> - typically this should be no higher
			than 24 or the motion detecting preview jpeg camera stream may start dropping frames.
			(I have no data on the effect GPU overclocking might have on this limitation).
			</li>
			<li><span style='font-weight:700'>video_mp4box_fps</span> - keep this value set to zero
			unless you want to create fast or slow motion videos.  When zero, mp4 boxing fps will be
			the same as video_fps which is normally what you want.  But this value can be set to a
			non zero value different from video_fps if you want fast or slow motion videos.
			</li>
			<li><span style='font-weight:700'>mjpeg_divider</span> - this value is divided into
			the video_fps value to get the preview jpeg rate.  The preview is updated at this rate
			and it is the rate that motion vector frames are checked for motion.
			</li>
			<li><span style='font-weight:700'>still_quality</span> - adjust up if it improves
			still jpeg quality.  Adjust down if you want to reduce the size of still jpegs.
			</li>
			<li><span style='font-weight:700'>Vector_Counts</span> - enable showing of vector count
			statistics when showing a Preset.  This information may help when setting motion detect
			limits.
			</li>
			<li><span style='font-weight:700'>Vector_Dimming</span> - sets a percentage dimming
			of the preview jpeg image when the
			<span style='font-weight:700'>Vectors</span> display is enabled.  This is to improve
			the contrast of the drawn motion vectors.
			</li>
			<li><span style='font-weight:700'>Preview_Clean</span> - if set to
			<span style='font-weight:700'>OFF</span>, whatever text or graphics that happen to be
			drawn on the preview jpeg at the time a motion preview save or thumb save occurs will
			also appear on the saved preview or thumb.  This might help with some debugging, but
			is normally not desirable, so the option should be set
			<span style='font-weight:700'>ON</span>.
			</li>
			</ul>
		</li>
		<li><span style='font-weight:700'>Times</span>
			<ul>
			<li><span style='font-weight:700'>Confirm_Gap</span> - for motion direction detects,
			require a second motion detect within this period of seconds before triggering a
			motion detect event.  This adds a level of noise rejection to motion direction detecting
			but may be set to zero to disable a second detect requirement if fast detection is desired.
			This setting does not apply to motion burst detects because the Burst_Frames setting
			provides a confirm time for that method.
			</li>
			<li><span style='font-weight:700'>Pre_Capture</span> - seconds of video to record prior
			to the first motion detect event.  This value should be greater than or equal to the
			Confirm_Gap.
			</li>
			<li><span style='font-weight:700'>Event_Gap</span> - number of seconds that must pass
			since the last motion detect event before a motion video record can end.
			When an Event_Gap period does expire without a new motion event occurring,
			the video will end with an end time of the last motion detect time plus the
			Post_Capture time (but see Post_Capture).
			Set this higher for animals or walking people that may pause for
			periods of time before resuming motion.  Set lower for active scenes where events
			are frequent and you want to bias towards shorter videos
			that capture more events separately.
			</li>
			<li><span style='font-weight:700'>Post_Capture</span> - seconds of video
			that will be recorded after the last occurring motion event.  This time must be
			less than or equal to the Event_Gap time because post capture time is accumulated
			in the circular buffer while the video is recording.  An expiring Event_Gap time
			ends the video immediately and no more Post_Capture time can be accumulated.
			</li>
			<li><span style='font-weight:700'>Time_Limit</span> - range is 10 - 1800
			seconds (30 minutes) and is the maximum seconds of motion video
			that will be recorded after the first occurring motion event.  So the total
			video length max will be the Pre_Capture time + the Time_Limit.
			If this is set to zero, there will be no time limit enforced.  This limit
			does not apply to manual recordings - see FIFO examples for that.
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
		<li><span style='font-weight:700'>Servo</span><br>
			The Servo menu is shown only if servos have been configured.
			<ul>
			<li><span style='font-weight:700'>Motion_Off_Preset</span> - if
			<span style='font-weight:700'>OFF</span>, do not detect motion when the servo postion
			is off a preset.  If configured motion regions are suitable for any servo position,
			this can be set
			<span style='font-weight:700'>ON</span> if you do not want motion detection to be
			put on hold when a servo is manually moved off a preset.
			</li>
			<li><span style='font-weight:700'>Move_Step_msec</span> - delay in milliseconds between
			servo steps when a servo is moved using the
			<span style='font-weight:700'>Servo</span> arrow buttons.  A servo step changes the
			pulse width of the servo control line by 1/100 of a millisecond (10 usec).
			</li>
			<li><span style='font-weight:700'>Preset_Step_msec</span> - delay
			in milliseconds between servo steps when a servo is moved using the
			<span style='font-weight:700'>Preset</span>
			left/right arrow buttons.
			</li>
			<li><span style='font-weight:700'>Servo_Settle_msec</span> - delay in milliseconds before
			motion detection is taken out of its hold state after servos stop moving.
			</li>
			<li><span style='font-weight:700'>Move_Steps</span> - number of steps to move when the
			<span style='font-weight:700'>Servo</span> arrow buttons are cycled to the second step
			mode.  The other step modes are single step and scan and are selected by clicking the
			<span style='font-weight:700'>Servo</span> button.  When in scan mode, the scan can be
			stopped by clicking the double arrow button again.
			</li>
			<li><span style='font-weight:700'>Pan_Left_Limit</span><br> 
				<span style='font-weight:700'>Pan_Right_Limit</span><br>
				<span style='font-weight:700'>Tilt_Up_Limit</span><br>
				<span style='font-weight:700'>Tilt_Down_Limit</span> - Sets the servo limits.
			</li>
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
		<li><span style='font-weight:700'>video_motion_name_format</span><br>
			<span style='font-weight:700'>video_manual_name_format</span><br>
			<span style='font-weight:700'>video_timelapse_name_format</span><br>
			<span style='font-weight:700'>still_name_format</span>
		- these file name formats are configurable with restrictions which are described in
		detail in pikrellcam.conf where they are configured.
		</li>
		<li><span style='font-weight:700'>motion_record_time_limit</span>
		- This value limits the time in seconds of motion video recordings and can be
		set in seconds from 10 to 1800 (30 minutes) or set to zero for no record time limit.
		This time limit does not apply to manual recordings, but see the FIFO command
		examples for how to have a time limited manual record.
		</li>
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
	Edit this file to change web page appearance and some web page behavior.  If the file
	is edited, reload web pages to see the results.
	<ul>
	<li> The image used for the web page background can be changed to another PiKrellCam
	distribution image or to your custom background image.  Any custom background image name
	must begin with <span style='font-weight:700'>bg_</span>
	or else the image will be deleted by git when you do an upgrade.
	</li>
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
	<li> The height of various scrolled views can be changed to match your normal browser
	size.
	</li>
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
lrwxrwxrwx 1 pi       www-data    33 Nov 16 09:07 archive -> /home/pi/pikrellcam/media/archive/
lrwxrwxrwx 1 pi       www-data    25 Nov 16 09:07 media -> /home/pi/pikrellcam/media/
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
be stored on an external USB disk by editing the <nobr>~/pikrellcam/scripts/startup</nobr> file to uncomment
the line:
<pre>
MOUNT_DISK=sda1
</pre>
<p>
This assumes there is a single USB disk plugged into the Pi and it appears as
<nobr>/dev/sda</nobr>.  If this USB disk has a linux filesystem on
<nobr>/dev/sda1</nobr>, pikrellcam can create directories with the
needed permissions.  However, if the filesystem is not a linux filesystem
(eg. VFAT or FAT32) then pikrellcam cannot set up the needed permissions for the
web interface to work and media files will not be shown
unless the proper permissions are specified when
the partition is mounted.  For this case, the mount command or fstab entry
should specify umask or dmask/fmask permissions of 0002.  For example,
use a mount command like:
<pre>
sudo mount -t vfat /dev/sda1 /media/mountdir -o rw,user,umask=0002
</pre>
or, if using fstab, the entry should be like:
<pre>
/dev/sda1   /media/mountdir    vfat    rw,user,umask=0002   0   0
</pre>
You can use dmask=0002,fmask=0002 in place of umask=0002.<br>
With a disk mounted, you can see:
<pre>
pi@rpi2: ~$ df -h
Filesystem      Size  Used Avail Use% Mounted on
...
/dev/sda1       3.7G  1.9G  1.6G  54% /home/pi/pikrellcam/media
</pre>
<p>
and the media links in <nobr>~/pikrellcam/www</nobr> will now be pointing into the mounted USB disk.
The media links may be changed to point to some other part of the filesystem which can be
mounted with a USB disk or NAS.  Just change the media_dir or archive_dir values in
pikrellcam.conf to reference an absolute path.
</div><br><br>

<span style='font-size: 1.5em; font-weight: 500;'>Archiving</span><hr>
<div class='indent0'>
The main media directory can be where you always manage media files, or it
can be considered a temporary staging directory where media
files are reviewed for archiving or deleting.  Over time the number of files to keep can become
large, difficult to review, and the web page may become sluggish with large numbers of files
to load at a time.  So an archiving interface is provided in pikrellcam so that media files
can be managed in smaller groups of days.  
<p>
Archiving means to move media files out of the main media flat directory and into the archive
tree directory where files are stored by day.
After archiving, media files may be viewed by day, week, or month by clicking links on the
calendar accessed through a web page "Archive Calendar" button.
<p>
The archive is a hierarchical directory tree where leaf day directories
contain media files only for a single day.  When viewing files on the web page, you will see
a "Media" label for the flat media files view or an "Archive" label when viewing media of
days from the archive directory tree.



Pikrellcam does not provide for automatic mounting
of the archive_dir as it does for the media_dir, but archiving could be set up to be on a separate
disk.  You could simply mount a disk on <nobr>~/pikrellcam/media/archive</nobr> or mount a disk on
/mnt/somedisk and edit archive_dir in pikrellcam.conf:
<pre>
archive_dir /mnt/somedisk/archive
</pre>
The archive directory tree is by year, month and day with each month and day two digits:
<pre>
.../archive/2015/11/13
or
.../archive/2015/07/04
</pre>
Where 13 is a media directory for November 13.  It has the media file sub directories videos,
thumbs, and stills.  When you archive files from the web page,
the web server sends a command to pikrellcam to archive the files and the pikrellcam program
does the archiving.  Since pikrellcam runs as the user pi, pikrellcam has the sudo permission to
create the appropriate directory structure for archiving.
Since the web server runs as the user www-data, pikrellcam creates directories with write permission
for the user www-data so files can be deleted from the web page. All directories in the archive
path have permissions like:
<pre>
pi@rpi2: ~/pikrellcam/www$ ls -Rl /tmp/media/archive/
/tmp/media/archive/:
total 0
drwxrwxr-x 3 pi www-data 60 Nov 12 12:16 2015/

/tmp/media/archive/2015:
total 0
drwxrwxr-x 6 pi www-data 120 Nov 15 15:28 11/

/tmp/media/archive/2015/11:
total 0
drwxrwxr-x 4 pi www-data 80 Nov 15 15:29 13/

/tmp/media/archive/2015/11/13:
total 0
drwxrwxr-x 2 pi www-data 80 Nov 14 22:08 thumbs/
drwxrwxr-x 2 pi www-data 80 Nov 14 22:08 videos/
</pre>
Keep these permissions in mind if you manage the directory structure outside of pikrellcam.  This
example listing is from my configuration where I set media_dir to <nobr>/tmp/media</nobr> in pikrellcam.conf
and my /tmp is a tmpfs
so my archive file testing would not wear on my SD card or a mounted USB drive.
<p>
The archive directory tree in the above listing was created by clicking on a web page Archive button.  It is also
possible to automate archiving from a script.  From a script or command line, the same archiving could
be done with:
<pre>
echo "archive_video day 2015-11-13" > ~/pikrellcam/www/FIFO
</pre>
or a specific video (including its thumb) can be archived with:
<pre>
echo "archive_video motion_2015-11-05_14.46.14_456.mp4 2015-11-05" > ~/pikrellcam/www/FIFO
</pre>
To archive all videos for today or yesterday:
<pre>
echo "archive_video day today" > ~/pikrellcam/www/FIFO
echo "archive_video day yesterday" > ~/pikrellcam/www/FIFO
</pre>
Stills may be script archived using the same set of arguments with the archive_still FIFO command.
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
record on
record on pre_capture_time
record on pre_capture_time time_limit
record pause
record off
still
tl_start period
tl_end
tl_hold [on|off|toggle]
tl_show_status [on|off|toggle]
motion_enable [on|off|toggle]
motion limits magnitude count
motion burst count frames
motion trigger enable
motion trigger enable pre_capture time_limit
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
archive_video [day|today|yesterday|video.mp4] yyyy-mm-dd
archive_still [day|today|yesterday|video.mp4] yyyy-mm-dd
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
	The motion trigger command is used to trigger a motion event from a script.
	It has two usages and the first is
	<nobr><span style='font-weight:700'>motion trigger enable</span></nobr>
	where <span style='font-weight:700'>enable</span> can be
	<span style='font-weight:700'>0</span> to use the configured motion enable,
	or it can be <span style='font-weight:700'>1</span> to force motion enable on
	for this trigger.  If
	<span style='font-weight:700'>enable</span> is omitted, it defaults to
	<span style='font-weight:700'>0</span>.  This is a trigger event that
	works in parallel with motion direction and burst detection and it uses
	all the configured motion times and motion commands.
	The event gap time applies, so
	detects or triggers after an initial FIFO trigger can keep the video going:
<pre>
echo "motion trigger" > ~/pikrellcam/www/FIFO"
# or force motion enable on
echo "motion trigger 1" > ~/pikrellcam/www/FIFO"
</pre>
	The second usage
	<nobr><span style='font-weight:700'>motion trigger enable pre_capture time_limit</span></nobr>
	is a special case motion trigger event that records a
	one shot motion video with a custom pre capture and time limit.  This usage
	does not use the configured motion times but does run configured motion
	commands.  If this FIFO command is given when there is already
	a motion video in progress, the configured motion times will be in use and the
	custom times cannot be applied.  The trigger command then reduces to its first
	use case as if no custom times were given.  For this example, assume that the
	configured motion enable is either off or, if on, that no other video is
	in progress.  Then issue a motion trigger command that
	forces motion enable on and records a
	motion video with a 4 second pre_capture and a 5 second time_limit for
	a total video length of 9 seconds (subject to the same circular buffer
	constraints as the record FIFO command):
<pre>
echo "motion trigger 1 4 5" > ~/pikrellcam/www/FIFO"
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
		<span style='font-weight:700'>/var/run/pikrellcam/state</span>
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
	<li>
		<span style='font-weight:700'>command</span>
		- a system command/script or an internal pikrellcam command if the command
		is preceeded with the '@' character.  Commands must be enclosed in quotes.
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
	to add labels to temperature values.
<pre>
daily minute "$C/ds18b20.py F fifo"
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
	This is new in PiKrellCam 3.1.0 and it's possible there will be small
	changes in its format in coming versions.
	<p>
	Motion detect events are written to this file during each motion video
	recording.  An on_motion_begin command configured to run in pikrellcam.conf
	can read this file to get
	a reasonably fast notification of motion
	events as they occur while the video is recording.  Scripts can determine
	where motion is in the video frame (by x,y position or motion region
	number) and then take some action such as sending multicast alarms and/or
	moving servos.
	<p>
	This file is overwritten with new event data for each new motion video
	recording so
	it is intended for use by on_motion_begin commands which are run
	immediately after the motion-events file writing begins.
	The output to the file is
	flushed after data for each motion detect is written.
	See script-dist/example-motion-send-alarm2 for an example reading of this
	file in an on_motion_begin script.
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
b   0
f  49  43  57  -2  57  263
1  44  42  53  -4  53  144
2  55  44  61   0  61  119
&lt;/motion&gt;
...
&lt;end&gt;
</pre>
	shows data for a detect at 3.667 seconds into the video
	(including precapture).
	Each line inside the motion block begins with a single character code:
	<ul>
	<li> <span style='font-weight:700'>b</span> - this line shows burst counts.
	For this detect the burst count did not exceed the burst count limit, so zero
	is reported.
	</li>
	<li> <span style='font-weight:700'>f</span> - this line is the data for
	the total frame vector (composite of all the motion region vectors)
	and has the format:
<pre>
code x y dx dy magnitude count
</pre>
	</li>
	<li> <span style='font-weight:700'>n</span> - where n is a digit.  These
	lines are for motion vectors for a motion region and have the same
	format as the frame vector.
	There is one
	line for each region having motion and no line for regions not having
	motion.  Just like the overall frame vector, for a motion region to have
	motion, the configured magnitude and count limits must be met.
	For this detect there was motion in regions 1 and 2.
	</li>
	</div>
The end tag is written when the motion video ends.
</div
<a name="MULTICAST_INTERFACE">
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
	<br>
	On each Pi you want PiKrellCam to send alarm messages from, the setup is:
	<ol>
		<li> Copy the example-motion-send-alarm1 or example-motion-send-alarm2
		from the scripts-dist directory to
		<nobr>scripts/motion-send-alarm.</nobr>
		If using the alarm2 example, edit it to select which region you want
		to detect motion in and edit it to set the motion magnitude and count
		limits you want.
		</li>
		<li> Edit pikrellcam.conf and set the on_motion_begin command to:
<pre>on_motion_begin  $C/motion-send-alarm
</pre>
		</li>
	</ol>
	Note: the example-motion-send-alarm2 is likely a work in progress.
	It is an example of reading the
	<nobr>/run/pikrellcam/motion-events</nobr> file
	while a motion video recording is in progress and
	sending an alarm only if motion exceeding a magnitude and count limit
	is detected in a particular region.
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
	<li><span style='font-weight:700'>to_hosts</span> - a single hostname
	or a comma separated
	list of hostnames that the message is being sent to.
	Use the keyword <span style='font-weight:700'>all</span> to send to
	all programs listening on the PKC multicast group.  Every PiKrellCam
	program is always listening for messages of certain message types
	addressed to its hostname.
	</li>
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

