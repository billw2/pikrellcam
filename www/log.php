<script>
function scroll_to_anchor()
    {
    document.getElementById("anchor").scrollIntoView(true);
    }
</script>

<style type="text/css">
a.anchor
    {
    display: block; position: relative; top: -250px; visibility: hidden;
    }
</style>


<?php
require_once(dirname(__FILE__) . '/config.php');
include_once(dirname(__FILE__) . '/config-user.php');
include_once(dirname(__FILE__) . '/config-defaults.php');	

$log_file = LOG_FILE;


function dump_log()
	{
	global $log_text_color;

	$file = fopen(LOG_FILE, "r");
	if ($file)
		{
		echo "<pre style=\"color:${log_text_color}\">";
		while (($line = fgets($file, 1024)) !== false)
			echo "$line";
		fclose($file);
		echo "</pre>";
		}
	}
?>

<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>PiKrellCam Log</title>
    <link rel="stylesheet" href="js-css/pikrellcam.css" />
  </head>

  <?php
  echo "<body onload=\"scroll_to_anchor()\" background=\"$background_image\">";
    echo "<div>";
//ini_set('display_errors',1);
//ini_set('display_startup_errors',1);
//error_reporting(-1);

		echo "<div class='text-center'>";
		$title = TITLE_STRING;

		echo "<div>";
		echo "<a class='text-shadow-large'
				style='text-decoration: none;'
				href='index.php'>";
		echo   "$title";;
		echo   "</a>";
		echo   "</div>";
		echo   "</div>";

		if (isset($_GET["delete_log"]))
			{
			$fifo = fopen(FIFO_FILE,"w");
    		fwrite($fifo, "delete_log");
    		fclose($fifo);
			sleep(1);
//			www-data cannot delete a pi /tmp file even if group write permission!
//			unlink("$log_file");
			}

		$div_style = "overflow-y: scroll; height:${n_log_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
		echo "<div style='$div_style'>";
		echo "<div style='color: $default_text_color; margin-left: 0.2cm; margin-right: 0.2cm; margin-top: 0.2cm;'>";
//		echo "<div style=\"$div_style\">";
//		echo '<div id="" style="overflow-y: scroll; height:770px; overflow-x: auto; border:4px groove silver">';

		dump_log();
		echo "<a id='anchor'></a>";
		echo   "</div>";
		echo   "</div>";

		echo "<div style='margin-top:16px;'>";
		echo "<a href='index.php' class='btn-control'
			style='margin-left:8px;'>
			$title</a>";

		echo "<input type='button' value='Delete Log'
			class='btn-control alert-control'
			style='margin-left:32px;'
			onclick='if (confirm(\"Delete Log File?\"))
			 {window.location=\"log.php?delete_log\";}'>";

		echo "</div>";

		?>
    </div>
  </body>
</html>
