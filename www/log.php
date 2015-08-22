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

$log_file = LOG_FILE;

function dump_log()
	{
	$file = fopen(LOG_FILE, "r");
	if ($file)
		{
		echo "<pre>";
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

  <body onload="scroll_to_anchor()" background="images/paper1.png">
    <div>
		<?php
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

		echo "<div style='margin-top:16px;'>";
		dump_log();
		echo   "</div>";

		echo "<div style='margin-top:16px;'>";
		echo "<a href='index.php' id='anchor' class='btn-control'
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
