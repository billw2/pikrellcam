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
		<br>After two or three seconds, the preview image from the camera should appear.
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
		After the video ends, view it by going to the Thumbs (or Videos) page by clicking:
		<span style='font-weight:700'>Media:</span> <span class='btn-control'>Thumbs</span>
		<br>Manually recorded videos cannot yet be viewed from the Thumbs page, so for them
		go to the Videos page.
	</li>
	<li>On the button bar, click the buttons
		<span style='font-weight:700'>Show:</span>
		<span class='btn-control'>Timelapse</span>
		<span class='btn-control'>Regions</span>
		<span class='btn-control'>Vectors</span>
		<br>to toggle showing information PiKrellCam can display on the OSD.
		This information shows real time motion detection information and gives a feel
		for motion magnitudes and counts which can be configured to tune motion detection.
	</li>
	<li>A basic first configuration to consider is enabling motion detection to be turned on
		each time PiKrellCam is started.  To do this, use the OSD menu system:<br>
		<ul>
			<li>
			Expand the <span style='font-weight:700'>Setup</span> panel.
			</li>
			<li>In the
			<span style='font-weight:700'>Motion</span> group,
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
the showing of Regions and Vectors.  Watching this display will allow you to tune
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
	<li>Interpreting the two vector count status lines:
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

<span style='font-size: 1.5em; font-weight: 650;'>Motion Regions Panel</span><hr>
<img src="images/motion-regions.jpg" alt="motion-regions.jpg"> 
<div class='indent0'>
Motion regions outline areas of the camera view that will be
sensitive to motion and provides for excluding from motion detection areas such
as wind blown vegetation.
Motion regions may be added, deleted, resized or moved.  After the
motion regions have been edited, they may be saved and loaded by name.  Typically, a region
will be configured for a particular camera setup and then saved by name.  Then that motion region
setup can be automatically loaded when PiKrellCam starts - see the example in the at commands
section.  If you save an edited motion regions to the special name
<span style='font-weight:700'>default</span>, then your modified regions will load
at startup without having to configure an at command motion regions load.

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
<span style='font-size: 1.2em; font-weight: 680;'>Camera Config</span>
	<ul>
		<li><span style='font-weight:700'>Video Presets</span> - selects the video resolution for
		motion and manual videos.  Different resolutions may have different fields of view. So
		one reason for selecting
		<span style='font-weight:700'>720p</span> over
		<span style='font-weight:700'>1080p</span> would be to get a wider field of view.  Resolutions
		will have either 16:9 or 4:3 aspect ratio.
		</li>
		<li><span style='font-weight:700'>Still Presets</span> - selecting different resolutions
		gives different fields of view and aspect ratios.
		</li>
		<li><span style='font-weight:700'>Adjustments</span>
		<ul>
			<li><span style='font-weight:700'>video_bitrate</span> - determines the size of a video
			and its quality.  Adjust up if it improves video quality.  Adjust down if you want
			to reduce the size of the videos.
			</li>
			<li><span style='font-weight:700'>video_fps</span> - typically this should be no higher
			than 24 or the motion detecting preview jpeg camera stream may start dropping frames.
			(I have no data on the effect GPU overclocking might have on this limitation).
			</li>
			<li><span style='font-weight:700'>video_mp4box_fps</span> - keep this value the same
			as video_fps unless you want to create fast or slow motion videos.
			</li>
			<li><span style='font-weight:700'>mjpeg_divider</span> - this value is divided into
			the video_fps value to get the preview jpeg rate.  The preview is updated at this rate
			and it is the rate that motion vector frames are checked for motion.
			</li>
			<li><span style='font-weight:700'>still_quality</span> - adjust up if it improves
			still jpeg quality.  Adjust down if you want to reduce the size of still jpegs.
			</li>
		</ul>
		</li>
	</ul>
<span style='font-size: 1.2em; font-weight: 650;'>Camera Params</span>
	<div class='indent1'>
	The camera parameters menus can set the hardware parameters of the Pi camera.
	</div>
	<p>
<span style='font-size: 1.2em; font-weight: 650;'>Motion</span>
	<ul>
		<li><span style='font-weight:700'>Vector Limits</span>
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
			Post_Capture time.  Set this higher for animals or walking people that may pause for
			periods of time before resuming motion.  Set lower for active scenes where events
			are frequent and you want to bias towards shorter videos
			that capture more events separately.
			</li>
			<li><span style='font-weight:700'>Post_Capture</span> - seconds of video
			that will be recorded after the last occurring motion event.  This time must be
			less than or equal to the Event_Gap time.
			</li>
			<li><span style='font-weight:700'>Time_Limit</span> - range is 10 - 1800
			seconds (30 minutes) and is the seconds of motion video
			that will be recorded after the first occurring motion event.  So the total
			video length will be the Pre_Capture time + the Time_Limit.
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
		<li><span style='font-weight:700'>Settings</span>
			<ul>
			<li><span style='font-weight:700'>Startup_Motion</span> - set to
			<span style='font-weight:700'>ON</span> for motion detection to be enabled each time
			PiKrellCam starts.  If set to
			<span style='font-weight:700'>OFF</span>, motion detection will need to be manually
			enabled from the web page or a script.
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
	</ul>
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
		<li><span style='font-weight:700'>video_filename</span>,
			<span style='font-weight:700'>still_filename</span> and
			<span style='font-weight:700'>timelapse_video_name</span>
		- these name formats are configurable, but they probably should not be because
		changing them can cause problems... expand on this.
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
	Read this file for configuration options to change the web pages background image,
	text colors and scroll area heights.
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
This assumes there is a single USB disk plugged into the Pi and it appears as <nobr>/dev/sda</nobr>.  This
USB disk should have a linux filesystem on <nobr>/dev/sda1</nobr> so that pikrellcam can create directories with the
needed permissions.  With a disk mounted, you can see:
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
The main media directory can be considered a temporary staging directory where media
files are reviewed for keeping or deleting.  Over time the number of files to keep can become
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
The archive directory tree is by year, month and day with each day two digits:
<pre>
.../archive/2015/11/13
or
.../archive/2015/07/04
</pre>
Where 13 is a media directory for November 13.  It has the media file sub directories videos,
thumbs, and stills.  When you archive a single file or day or thumb selections from the web page,
the web server sends a command to pikrellcam to archive those files and the pikrellcam program
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
motion trigger
motion load_regions name
motion save_regions name
motion list_regions
motion show_regions [on|off|toggle]
motion show_vectors [on|off|toggle]
motion [command] - other commands sent by the web page to edit motion regions not
	intented for script or command line use.

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
archive_video
archive_still
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
	plus a 5 second margin.
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
	Still jpegs are created when a still command is sent to the FIFO.
<pre>
	echo "still" > ~/pikrellcam/www/FIFO"
</pre>
	<li>
	You have a script or program that looks at a GPIO output from a infrared motion
	detector.  The script can trigger a motion record event that uses all of the
	configured motion detect times with:
<pre>
	echo "motion trigger" > ~/pikrellcam/www/FIFO"
</pre>
	</li>
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

<span style='font-size: 1.2em; font-weight: 650;'>Examples</span>
<div class='indent1'>
<ul>
<li>
At each PiKrellCam startup, load a motion detect regions file:
<pre>
daily   start   "@motion load_regions driveway"
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
</ul>
</div>
</div>



</div>

