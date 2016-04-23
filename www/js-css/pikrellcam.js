var move_mode = "move_coarse";

function move_region_mode($cb)
	{
	if ($cb.checked)
		move_mode = "move_coarse";
	else
		move_mode = "move_fine";
	}

function move_region(where)
	{
//	alert("motion " + move_mode +  " " + where);
	fifo_command("motion " +  move_mode +  " " + where);
	}

function tl_start()
	{
	var period;

	period = document.getElementById('tl_period').value;
	fifo_command("tl_start " +  period);
	}


function image_expand_toggle()
	{
	var mjpeg_img = document.getElementById("mjpeg_image");

	mjpeg_img.classList.toggle("expanded-image");
	}

function new_region()
	{
	fifo_command("motion new_region 0.3 0.3 0.3 0.3");
//	alert("Two consecutive fifo_command() not working.");
//	fifo_command("motion select_region last\n");
	}

function list_regions()
	{
	fifo_command("motion list_regions");
	}

function load_regions()
	{
	fifo_command('motion load_regions_show ' + document.getElementById('load_regions').value);
	document.getElementById('load_regions').value = "";
	}

function save_regions()
	{
	fifo_command('motion save_regions ' + document.getElementById('save_regions').value);
	document.getElementById('save_regions').value = "";
	}


var mjpeg;

function mjpeg_read()
	{
	setTimeout("mjpeg.src = 'mjpeg_read.php?time=' + new Date().getTime();", 150);
	}

function mjpeg_start()
	{
	mjpeg = document.getElementById("mjpeg_image");
	mjpeg.onload = mjpeg_read;
	mjpeg.onerror = mjpeg_read;
	mjpeg_read();
	}


function create_XMLHttpRequest()
	{
	if (window.XMLHttpRequest)
		return new XMLHttpRequest();	// IE7+, Firefox, Chrome, Opera, Safari
	else
		return new ActiveXObject("Microsoft.XMLHTTP");	// IE6, IE5
	}


var sys_cmd = create_XMLHttpRequest();

function pikrellcam(start_stop)
	{
	sys_cmd.open("PUT", "sys_command.php?cmd=pikrellcam_" + start_stop, true);
	sys_cmd.send();
	}


var fifo_cmd = create_XMLHttpRequest();

function fifo_command (cmd)
	{
	fifo_cmd.open("PUT", "fifo_command.php?cmd=" + cmd, true);
	fifo_cmd.send();
	}

