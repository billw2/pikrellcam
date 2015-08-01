<?php
// display JPEG images in directory in name order, with clickable links  v0.2 7/31/2015
function showGallery( $pathToVideo, $pathToThumbs )
{
  $output = "";
  $dir = opendir( $pathToThumbs );  // open the directory
  $a=array();  // hold filenames in array so we can sort them in order
  while (false !== ($fname = readdir($dir)))  // loop through directory
  { if (substr($fname, -4) == '.jpg') // select only *.jpg filenames
    { $a[]=$fname; } // add this filename to the array
  }
  closedir( $dir ); // close the directory, all done reading
  $counter = 0;     // count how many images total so far
  rsort($a);         // sorts the array of filenames in-place, in reverse order (newest at top)
  $tNow = time();
  $fModTime = $tNow; // just in case there were no images at at all
  foreach($a as $fname){
      $vname = rtrim($fname, ".th.jpg") . ".mp4"; // filename X.th.jpg becomes X.mp4
      $tFile = "{$pathToThumbs}{$fname}"; // full pathname of thumbnail image
      $vFile = "{$pathToVideo}{$vname}";  // full pathname of video file
      $vSize = round(filesize($vFile) / (1024*1024) ); // filesize in MB
      $fModTime = filemtime($tFile);
      $fDate = @date('m/d', $fModTime);
      $fTime = @date('H:i:s', $fModTime);

      $output .= "<fieldset style=\"display:inline; padding:2px; vertical-align:middle;\">";
      $output .= "$fDate &nbsp;<b>$fTime</b>";
      $output .= "<div style=float:right>{$vSize}M</div><br>";
      $output .= "<a href=\"{$vFile}\">";
      $output .= "<img src=\"{$tFile}\" border=\"0\" /></a></fieldset>";
      $counter += 1;
  }
  $fDur = ($tNow - $fModTime) / 3600;
  $hours = number_format($fDur,1);  // hours with 1 decimal place
  $tString = @date('Y/m/d H:i:s', $tNow);
  echo "<html><head><title>Thumbnails</title></head><body>";
  echo "<fieldset> ";
  diskSpace($pathToVideo);  // print disk usage
  echo "<big><big><b>  &nbsp; &nbsp;<a href=index.php>PiKrellCam</a></b></big> &nbsp; &nbsp; ";
  echo "{$counter} in {$hours} hr</big></center>";
  echo "<div style=float:right>{$tString}</div><hr>";

  echo $output;  // send out the completed HTML web page
  echo "</fieldset></body> </html>"; // end of HTML file
}
function diskSpace( $vdir ){ // http://www.thecave.info/display-disk-free-space-percentage-in-php/
  $df = disk_free_space($vdir); // total free
  $dt = disk_total_space($vdir); // total on disk
  $du = sprintf('%.0f',($dt - $df) / (1024*1024));;  // total used (Mbytes)
  $dp = sprintf('%.1f',(($dt - $df) / $dt) * 100); // percent used
  echo "{$du} MB &nbsp;(${dp} %)";
}
showGallery("media/videos/","media/thumbs/");  // video and thumbnail directories
?>
