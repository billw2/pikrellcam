<?php
set_time_limit(0);

$filename = 'audio_FIFO';

$buffer = '';
$readsize = 4096;

header("Content-Type: audio/mpeg");
header('content-type: application/octet-stream');
header ("Content-Transfer-Encoding: binary");
header ("Pragma: no-cache");
header("Cache-Control: no-cache, must-revalidate");
header("Expires: Sat, 26 Jul 1997 05:00:00 GMT");

$fifo = fopen($filename, 'r');
if ($fifo === false)
	{
	return false;
	}

while (!feof($fifo))
	{
	$buffer = fread($fifo, $readsize);
	echo $buffer;
	ob_flush();
	flush();
	}

$status = fclose($fifo);

return true;
?>

