<style>
pre
	{
	font-family: monospace, "courier new", courier, serif;
	font-size: 1.0em;
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

</style>

<?php
require_once(dirname(__FILE__) . '/config.php');
include_once(dirname(__FILE__) . '/config-user.php');
include_once(dirname(__FILE__) . '/config-defaults.php');	

	ini_set('display_errors',1);
	ini_set('display_startup_errors',1);
	error_reporting(-1);

	$header = "<!DOCTYPE html><html><head>";
	$header .= "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    $header .= "<title>PiKrellCam Archive</title>";
	$header .= "<link rel=\"stylesheet\" href=\"js-css/pikrellcam.css\" />";
	$header .= "</head>";
	$header .= "<body background=\"$background_image\">";
	$header .= "<div><div class='text-center'>";
	$title = TITLE_STRING;
	$header .= "<div><a class='text-shadow-large'
			style='text-decoration: none;' href='index.php'>$title</a></div></div></div>";
	echo $header;
?>
<?php echo "<div style='color: $default_text_color; margin-left: 1cm; margin-right: 2cm'>" ?>

<span style='font-size: 1.4em; font-weight: 500;'>Note</span><hr>
This help page is just started and so far only has info related to the new pikrellcam archiving.
</p><br>


<span style='font-size: 1.4em; font-weight: 500;'>Media Files</span><hr>
A media file is a manually or motion recorded video, a motion thumbnail jpeg, or a still jpeg.
For each motion video recorded in a videos directory, there is a corresponding thumb jpeg
of the hopefully best motion extracted from the video.
By reviewing the thumbs, you can get a quick idea of what caused the motion video.
Motion videos and their corresponding thumbs track each other.  They are archived
together and deleted together whenever the action is done on a web page video view or thumb view.
<p><br>



<span style='font-size: 1.4em; font-weight: 500;'>Media Directories</span><hr>
PiKrellCam uses media directories for storing media files.  A media directory is a directory
with media file sub directories videos, thumbs, and stills.  All media directories are created
automatically when pikrellcam runs.  There are two links in the web page www directory
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
<p>
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
This assumes there is a single USB disk plugged into the Pi and it appears as <nobr>/dev/sda</nobr>.  This
USB disk should have a linux filesystem on <nobr>/dev/sda1</nobr> so that pikrellcam can create directories with the
needed permissions.  With a disk mounted, you can see:
<pre>
pi@rpi2: ~$ df -h
Filesystem      Size  Used Avail Use% Mounted on
...
/dev/sda1       3.7G  1.9G  1.6G  54% /home/pi/pikrellcam/media
</pre>
and the media links in <nobr>~/pikrellcam/www</nobr> will now be pointing into the mounted USB disk.
The media links may be changed to point to some other part of the filesystem which can be
mounted with a USB disk or NAS.  Just change the media_dir or archive_dir values in
pikrellcam.conf to reference an absolute path.
</p><br>



<span style='font-size: 1.4em; font-weight: 500;'>Archiving</span><hr>
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
Stills may be script archived with the archive_still FIFO command.

</div>

