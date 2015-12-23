<?php

$host="localhost";
$port=9999;

// open connection to local socket
$fp = fsockopen($host, $port, $error, $errstr);
if (!$fp)
        return;

// create multipart header
header("Cache-Control: no-cache");
header("Cache-Control: private");
header("Pragma: no-cache");
header("Content-type: multipart/x-mixed-replace; boundary=fooboundary");

// Set this so PHP doesn't timeout
set_time_limit(0);

// read the stream
while(!feof($fp)) {
        echo fgets($fp, 1024);
}

?>

