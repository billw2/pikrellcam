<?php
	require_once(dirname(__FILE__) . '/config.php');
?>
<?php
	$fifo = fopen(FIFO_FILE,"w");
	fwrite($fifo, $_GET["cmd"]);
	fclose($fifo);
?>
