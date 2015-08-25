<?php
// display JPEG images in directory in name order, with clickable links  v0.2 7/31/2015

require_once(dirname(__FILE__) . '/config.php');
include_once(dirname(__FILE__) . '/config-user.php');
include_once(dirname(__FILE__) . '/config-defaults.php');

function showGallery( $pathToVideo, $pathToThumbs )
{
  global $background_image, $default_text_color, $n_thumb_scroll_pixels;

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
      $vSize = round(filesize($vFile) / (1000*1000) ); // filesize in MB
      $fModTime = filemtime($tFile);
      $fDate = @date('m/d', $fModTime);
      $fTime = @date('H:i:s', $fModTime);

      $output .= "<fieldset style=\"display:inline; padding:2px; vertical-align:middle;\">";
      $output .= "<span style=\"color: $default_text_color\">$fDate &nbsp;<b>$fTime</b>&nbsp";
      $output .= "<div style=float:right>{$vSize}M</div></span><br>";
      $output .= "<a href=\"{$vFile}\">";
      $output .= "<img src=\"{$tFile}\" border=\"0\" /></a></fieldset>";
      $counter += 1;
  }
  $fDur = ($tNow - $fModTime) / 3600;
  $hours = number_format($fDur,1);  // hours with 1 decimal place
  $tString = @date('Y/m/d H:i:s', $tNow);

  echo "<html><head><title>Thumbnails</title>";
  echo "<link rel=\"stylesheet\" href=\"js-css/pikrellcam.css\" />";
  echo "</head>";
  echo "<body background=\"$background_image\">";

  $title = TITLE_STRING;
  echo "<div class='text-center'>";
  echo "<div>";
  echo "<a class='text-shadow-large' style='text-decoration: none;' href='index.php'>";
  echo   "$title";;
  echo   "</a>";
  echo   "</div>";
  echo   "</div>";

  echo "<fieldset> ";
  diskSpace($pathToVideo);  // print disk usage
  echo "<span style=\"margin-top: 16px; font-size: 1.2em; color: $default_text_color\">&nbsp &nbsp {$counter} in {$hours} hr</span>";

  echo "<div style=\"float:right; font-size: 1.2em; color: $default_text_color;\">{$tString}</div><hr>";

//  $div_style = "overflow-y: scroll; height:${n_thumb_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
//  $div_style = "overflow-y: scroll; height:${n_thumb_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
  echo "<div style=\"$div_style\">";
  echo $output;  // send out the completed HTML web page
  echo "</div>";
  echo "</fieldset>";

  echo "<div style='margin-top:12px;'>";
  $title = TITLE_STRING;
  echo "<a href='index.php' class='btn-control'
  style='margin-left:8px;'>
  $title</a>";

  $video_dir = VIDEO_DIR;
  echo "<a href=\"media.php?dir=${video_dir}\" class='btn-control' style='margin-left:8px;'> Videos</a>";

  $still_dir = STILL_DIR;
  echo "<a href=\"media.php?dir=${still_dir}\" class='btn-control' style='margin-left:8px;'> Stills</a>";

  echo "</body> </html>"; // end of HTML file
}
function diskSpace( $vdir ){ // http://www.thecave.info/display-disk-free-space-percentage-in-php/
  global $default_text_color;

  $df = disk_free_space($vdir); // total free
  $dt = disk_total_space($vdir); // total on disk
  $du = sprintf('%.0f',($dt - $df) / (1024*1024));;  // total used (Mbytes)
  $dp = sprintf('%.1f',(($dt - $df) / $dt) * 100); // percent used
  echo "<span style=\"color: $default_text_color\">{$du} MB &nbsp;(${dp} %)</span>";
}
showGallery("media/videos/","media/thumbs/");  // video and thumbnail directories
?>
