<?php
// display JPEG images in directory in name order, with clickable links
function showGallery( $pathToVideo, $pathToThumbs )
{
  $cols = 4;  // how many columns on the HTML page

  $output = "<html>";
  $output .= "<head><title>Thumbnails</title></head>";
  $output .= "<body>";
  $output .= "<table cellspacing=\"0\" cellpadding=\"2\" width=\"500\">";
  $output .= "<tr>";

  $dir = opendir( $pathToThumbs );  // open the directory
  $a=array();  // hold filenames in array so we can sort them in order
  while (false !== ($fname = readdir($dir)))  // loop through directory
  {
    if (substr($fname, -4) == '.jpg') // select only *.jpg filenames
    { $a[]=$fname; } // add this filename to the array
  }
  closedir( $dir ); // close the directory, all done reading
  $counter = 0;     // count how many images total so far
  sort($a);         // sorts the array of filenames in-place
  foreach($a as $fname){
      $vname = rtrim($fname, ".th.jpg") . ".mp4"; // filename X.th.jpg becomes X.mp4
      $output .= "<td valign=\"middle\" align=\"center\"><a href=\"{$pathToVideo}{$vname}\">";
      $output .= "<img src=\"{$pathToThumbs}{$fname}\" border=\"0\" />";
      $output .= "</a></td>";
      $counter += 1;
      if ( $counter % $cols  == 0 ) { $output .= "</tr><tr>"; }
  }
  $output .= "</tr> </table> </body> </html>"; // end of HTML file
  echo $output;  // send out the completed HTML web page
}
showGallery("media/videos/","media/thumbs/");  // video and thumbnail directories
?>
