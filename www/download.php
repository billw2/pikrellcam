<?php
	require_once(dirname(__FILE__) . '/config.php');
?>
<?php
	$filename = $_GET["file"];
	$extension = strtolower(substr(strrchr($filename, "."), 1));

	switch ($extension)
		{
		case "h264":
		case "mp4":
			$content_type="video/mp4";
			break;
		case "png":
			$content_type="image/png";
			break;
		case "jpeg":
		case "jpg":
			$content_type="image/jpeg";
			break;
		default:
			$content_type="application/force-download";
		}

	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
	header("Cache-Control: private",false);
	header("Content-Type: $content_type");
	header("Content-Disposition: attachment; filename=\"".basename($filename)."\";");
	header("Content-Transfer-Encoding: binary");
	header("Content-Length: ".@filesize($filename));
	set_time_limit(0);
	readfile("$filename");
?>
