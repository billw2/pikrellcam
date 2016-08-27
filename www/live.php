<?php
    require_once(dirname(__FILE__) . '/config.php');
    include_once(dirname(__FILE__) . '/config-user.php');
    include_once(dirname(__FILE__) . '/config-defaults.php');
?>

<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title><?php echo TITLE_STRING; ?></title>
  <script src="js-css/jquery.min.js"></script>
  <link rel="stylesheet" href="js-css/pikrellcam.css" />
  <script src="js-css/pikrellcam.js"></script>
  <script src="js-css/expandable-panels.js"></script>
  
  <script type="text/javascript">
   var mjpeg;
   var url = window.location.search;
   var refreshtime = url.substring(url.lastIndexOf("=")+1);
   if (refreshtime == "") { refreshtime = 150; }

	function mjpeg_read()
        {
        setTimeout("mjpeg.src = 'mjpeg_read.php?time=' + new Date().getTime();", refreshtime);
        }

	function mjpeg_start()
        {
        mjpeg = document.getElementById("mjpeg_image");
        mjpeg.onload = mjpeg_read;
        mjpeg.onerror = mjpeg_read;
        mjpeg_read();
        }
  </script>
  
  <link rel="stylesheet" href="js-css/expandable-panels.css" />
  <style>
	*{margin:0;padding:0;}
</style>
</head>

<?php
echo "<body background='' onload=\"mjpeg_start();\">";
?>
<img id="mjpeg_image" style="width:100%" onclick='window.open("index.php","_blank");'>
</body>
</html>
