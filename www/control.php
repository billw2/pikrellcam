<?php
// Mock up to show control buttons - in development.

	echo "<input type='image' src='images/arrow-left.png'
			style='margin-left:40px; vertical-align: bottom;'
			onclick='fifo_command('pan left');'>
		<input type='image' src='images/arrow-right.png'
			style='vertical-align: bottom;'
			onclick='fifo_command('pan right');'>
		<input type='image' src='images/arrow-up.png'
			style='vertical-align: bottom;'
			onclick='fifo_command('tilt up');'>
		<input type='image' src='images/arrow-down.png'
			style='vertical-align: bottom;'
			onclick='fifo_command('tilt down');'>";

?>
