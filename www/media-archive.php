<script>
function scroll_to_selected()
	{
	document.getElementById("selected").scrollIntoView(true);
	}


function select_all(source)
	{
	checkboxes = document.getElementsByName('file_list[]');
	for (var c in checkboxes)
		{
		checkboxes[c].checked = source.checked;
		}
	checkboxlist = document.getElementsByName('checkbox_list[]');
	for (var c in checkboxlist)
		{
		checkboxlist[c].checked = source.checked;
		}
	}

function select_day(source, ymd)
	{
//alert("select_day" + ymd);
	checkboxes = document.getElementsByName('file_list[]');
	for (var c in checkboxes)
		{
		var val = checkboxes[c].value;
		if (val.substring(0, 10) == ymd)
			checkboxes[c].checked = source.checked;
		}
	}
</script>

<style type="text/css">
a.anchor
	{
	display: block; position: relative; top: -250px; visibility: hidden;
	}
</style>


<?php
// ini_set('display_errors',1);
// ini_set('display_startup_errors',1);
// error_reporting(-1);

include(dirname(__FILE__) . '/config.php');

include(dirname(__FILE__) . '/config-user.php');
include(dirname(__FILE__) . '/config-defaults.php');


function eng_filesize($bytes, $decimals = 1)
	{
	$sz = 'BKMGTP';
	$factor = floor((strlen($bytes) - 1) / 3);
	return sprintf("%.{$decimals}f", $bytes / pow(1000, $factor)) . @$sz[$factor];
	}

function media_dir_array_create($media_dir)
	{
	global	$archive_root, $media_mode, $media_type, $media_subdir, $media_view;

	$media_array = array();
	$file_dir = "$media_dir/$media_subdir";		// videos, thumbs, or stills

	if (is_dir($file_dir))
		{
		$tmp_array = array_slice(scandir($file_dir), 2);
		$n_files = count($tmp_array);
		foreach($tmp_array as $file_name)
			{
			$extension = substr(strrchr($file_name, "."), 0);
			if ("$extension" != ".mp4" && "$extension" != ".jpg" && "$extension" != ".h264")
				continue;
			if ("$extension" == ".h264" && "$media_mode" == "loop")
				continue;

			// Try to use an mtime from file name ccc_yyyy-mm-dd_hh.mm.ss_ccc.[mp4|jpg] which
			// has date embedded in name instead of filesystem mtime which can change if file
			// is copied. Otherwise (name format changed) use the mtime from the filesystem.
			//
			$mtime = 0;
			$parts = explode("_", $file_name);
			if (count($parts) == 4)
				{
				$date = explode("-", $parts[1]);	// $parts[1] is yyyy-mm-dd
				if (count($date) == 3)
					{
					$parts[2] = str_replace(':', '.', $parts[2]);
					$time = explode(".", $parts[2]); // $parts[2] is hh.mm.ss or hh.mm
					$n = count($time);
					if ($n == 3)
						$mtime = mktime($time[0], $time[1], $time[2], $date[1], $date[2], $date[0]);
					else if ($n == 2)
						$mtime = mktime($time[0], $time[1], 0, $date[1], $date[2], $date[0]);
					}
				}
			if ($mtime == 0)
				$mtime = filemtime("$file_dir" . "/" . "$file_name");
			$ymd = date("Y-m-d", $mtime);
			if ("$media_type" == "videos")
				{
				if ("$media_view" == "thumbs")
					{
					$thumb_name = $file_name;
					$file_name = str_replace(".th.jpg", ".mp4", $thumb_name);
					$short_name = date('H:i:s', $mtime);
					}
				else
					{
					$thumb_name = str_replace(".mp4", ".th.jpg", $file_name);
					$short_name = date('H:i:s', $mtime) . "$extension";
					}
				$media_array[] = array('file_name' => $file_name,
								'media_dir'        => "$media_dir",
								'file_path'        => "$media_dir" . "/videos/" . "$file_name",
								'thumb_path'       => "$media_dir" . "/thumbs/" . "$thumb_name",
								'mtime'            => $mtime,
								'date'             => $ymd,
								'short_name'       => $short_name);
				}
			else	// stills
				{
				if ("$media_view" == "thumbs")
					$short_name = date('H:i:s', $mtime);
				else
					$short_name = date('H:i:s', $mtime) . "$extension";
				$media_array[] = array('file_name' => $file_name,
								'media_dir'        => "$media_dir",
								'file_path'        => "$media_dir" . "/stills/" . "$file_name",
								'thumb_path'       => "$media_dir" . "/stills/" . "$file_name",
								'mtime'            => $mtime,
								'date'             => $ymd,
								'short_name'       => $short_name);
				}
			}
		}
	return $media_array;
	}

function archive_media_dir($year, $month, $day)
	{
	global	$archive_root;

	$month_dir = str_pad($month, 2, "0", STR_PAD_LEFT);
	$day_dir = str_pad($day, 2, "0", STR_PAD_LEFT);
	return "$archive_root/$year/$month_dir/$day_dir";
	}

function media_array_create()
	{
	global	$media_array, $media_array_size, $media_mode, $media_dir;
	global	$year, $month0, $day0, $month1, $day1;

	$media_array = array();
	if ("$media_mode" == "archive")
		{
		if ($month0 == $month1)
			$dlast = $day1;
		else
			$dlast = 31;

		for ($day = $day0; $day <= $dlast; $day++)
			{
			$dir = archive_media_dir($year, $month0, $day);
			$media_array = array_merge($media_array, media_dir_array_create($dir));
			}
		if ($month1 > $month0)
			for ($day = 1; $day <= $day1; $day++)
				{
				$dir = archive_media_dir($year, $month1, $day);
				$media_array = array_merge($media_array, media_dir_array_create($dir));
				}
		}
	else
		$media_array = media_dir_array_create($media_dir);

	usort($media_array, create_function('$a, $b',
				'return strcmp($a["mtime"], $b["mtime"]);'));
	krsort($media_array);
	$media_array = array_values($media_array);
	$media_array_size = count($media_array);
	}

function media_array_index($name)
	{
	global	$media_array, $media_array_size;

	if ($media_array_size == 0)
		return -1;
	else if ("$name" == "")
		return 0;

	for ($i = 0; $i < $media_array_size; $i++)
		if ("$name" == $media_array[$i]['file_name'])
			return $i;
	return 0;
	}

function delete_file($media_dir, $fname)
	{
	global	$media_mode, $media_subdir;

	if (!is_dir($media_dir))
		return;

	if ("$media_subdir" == "stills")
		{
		unlink("$media_dir/stills/$fname");
		$thumb = str_replace(".jpg", ".th.jpg", $fname);
		unlink("$media_dir/stills/.thumbs/$thumb");
		}
	else
		{
		unlink("$media_dir/videos/$fname");

		$thumb = str_replace(".mp4", ".th.jpg", $fname);
		unlink("$media_dir/thumbs/$thumb");

		$h264 = "$media_dir/videos/$fname" . ".h264";
		if (is_file("$h264"))	// stray xxx.mp4.h264 possible if MP4Box failed or no space
			unlink("$h264");

		$csv = "$media_dir/videos/". str_replace(".mp4", ".csv", $fname);
		if (is_file("$csv"))
			unlink("$csv");
		}
	if ("$media_mode" == "archive")
		delete_empty_media_dir($media_dir);
	}

function delete_day($media_dir, $ymd)
	{
	global	$media_mode, $media_type;

	//echo "<script type='text/javascript'>alert('$media_type $media_dir $ymd');</script>";
	if (!is_dir($media_dir))
		return;

	if ("$media_type" == "videos")
		{
		array_map('unlink', glob("$media_dir/videos/*$ymd*.mp4"));
		array_map('unlink', glob("$media_dir/videos/*$ymd*.csv"));
		array_map('unlink', glob("$media_dir/videos/*$ymd*.h264"));
		array_map('unlink', glob("$media_dir/thumbs/*$ymd*.th.jpg"));
		}
	else if ("$media_type" == "stills")
		{
		array_map('unlink', glob("$media_dir/stills/*$ymd*.jpg"));
		array_map('unlink', glob("$media_dir/stills/.thumbs/*$ymd*.th.jpg"));
		}
	if ("$media_mode" == "archive")
		delete_empty_media_dir($media_dir);
	}

function delete_all_files($media_dir)
	{
	global	$media_type;

	if (!is_dir($media_dir))
		return;
	if ("$media_type" == "videos")
		{
		array_map('unlink', glob("$media_dir/videos/*.mp4"));
		array_map('unlink', glob("$media_dir/videos/*.csv"));
		array_map('unlink', glob("$media_dir/videos/*.h264"));
		array_map('unlink', glob("$media_dir/thumbs/*.th.jpg"));
		}
	else if ("$media_type" == "stills")
		{
		array_map('unlink', glob("$media_dir/stills/.thumbs/*.th.jpg"));
		array_map('unlink', glob("$media_dir/stills/*.jpg"));
		}
	}

function delete_empty_media_dir($media_dir)
	{
	if (!is_dir($media_dir))
		return;

	$subdir = "$media_dir/videos";
	if (is_dir($subdir) && count(glob("$subdir/*")) == 0)
		rmdir($subdir);

	$subdir = "$media_dir/thumbs";
	if (is_dir($subdir) && count(glob("$subdir/*")) == 0)
		rmdir($subdir);

	$subdir = "$media_dir/stills/.thumbs";
	if (is_dir($subdir) && count(glob("$subdir/*")) == 0)
		rmdir($subdir);
	$subdir = "$media_dir/stills";
	if (is_dir($subdir) && count(glob("$subdir/*")) == 0)
		rmdir($subdir);

	if (count(glob("$media_dir/*")) == 0)
		rmdir($media_dir);
	}

function delete_archive_range($year, $month0, $day0, $month1, $day1)
	{
	global	$archive_root, $media_mode;

	if ("$media_mode" != "archive")
		return;

	if ($month0 == $month1)
		$dlast = $day1;
	else
		$dlast = 31;

	$month_dir = str_pad($month0, 2, "0", STR_PAD_LEFT);
	for ($day = $day0; $day <= $dlast; $day++)
		{
		$day_dir = str_pad($day, 2, "0", STR_PAD_LEFT);
		$del_dir = "$archive_root/$year/$month_dir/$day_dir";
		delete_day($del_dir, "$year-$month_dir-$day_dir");
		delete_empty_media_dir("$del_dir");
		delete_empty_media_dir("$archive_root/$month_dir");
		}
	if ($month1 > $month0)
		{
		$month_dir = str_pad($month1, 2, "0", STR_PAD_LEFT);
		for ($day = 1; $day <= $day1; $day++)
			{
			$day_dir = str_pad($day, 2, "0", STR_PAD_LEFT);
			$del_dir = "$archive_root/$year/$month_dir/$day_dir";
			delete_day($del_dir, "$year-$month_dir-$day_dir");
			delete_empty_media_dir("$del_dir");
			delete_empty_media_dir("$archive_root/$month_dir");
			}
		}
	}

function still_thumb($idx)
	{
	global	$media_array, $stills_thumb_rescan;

	$fname = $media_array[$idx]['file_name'];
	$thumb = str_replace(".jpg", ".th.jpg", $fname);

	$stills_dir = $media_array[$idx]['media_dir'] . "/stills";
	$stills_thumb_dir = "$stills_dir/.thumbs";

	$thumb_path = "$stills_thumb_dir/$thumb";
	if (!is_file($thumb_path))
		{
		if ($stills_thumb_rescan)
			{
			$fifo = fopen(FIFO_FILE,"w");
			fwrite($fifo, "stills_thumbs_rescan $stills_dir");
			fclose($fifo);
			$stills_thumb_rescan = false;
			}
		$thumb_path = "$stills_dir/$fname";
		}
	return "<img src=\"$thumb_path\" width='150' style='padding:1px 5px 2px 5px'/></a></fieldset>";
	}

// wait for archive script to move files
// media_mode is always "media" when called
//
function wait_files_gone($key, $pat)
	{
	global $media_dir, $media_type;

	for ($i = 0; $i < 16; ++$i)
		{
		usleep(100000);
		if ("$key" == "file" && !file_exists($pat))
			break;
		else if ("$key" == "day")
			{
			if ("$media_type" == "videos")
				{
				if (   count(glob("$media_dir/videos/*$pat*")) == 0
				    && count(glob("$media_dir/thumbs/*$pat*")) == 0
				   )
					break;
				}
			else if ("$media_type" == "stills")
				{
				if (count(glob("$media_dir/stills/*$pat*")) == 0)
					break;
				}
			}
			break;
		}
	usleep(400000);
	if ($i == 16)
		echo "<script type='text/javascript'>alert('Archive may have failed. Is pikrellcam running?');</script>";
	}

function restart_page($selected)
	{
	global $env, $name_style;

	echo "</body></html>";
	if ("$selected" == "")
		echo "<script>window.location=\"media-archive.php?$env\";</script>";
	else
		echo "<script>window.location=\"media-archive.php?$env&file=$selected\";</script>";
	exit(0);
	}

//echo "<script type='text/javascript'>alert('$name_style $n_columns');</script>";

	$media_mode = "archive";
	if (isset($_GET["mode"]))
		$media_mode = $_GET["mode"];	// "archive", "media" or "loop"
	$title = TITLE_STRING;

	$header = "<!DOCTYPE html><html><head>";
	$header .= "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
	if ("$media_mode" == "archive")
		$header .= "<title>$title Archive</title>";
	else if ("$media_mode" == "loop")
		$header .= "<title>$title Loop</title>";
	else
		$header .= "<title>$title Media</title>";
	$header .= "<link rel=\"stylesheet\" href=\"js-css/pikrellcam.css\" />";
	$header .= "</head>";
	$header .= "<body onload=\"scroll_to_selected()\" background=\"$background_image\">";
	$header .= "<div><div class='text-center'>";

		// Offset TITLE_STRING to left of center 120px (compensate for 150px thumb)
	$header .= "<div style='margin: auto; overflow: visible;'>";
	$header .= "<div style='margin-right:120px;'>";
	$header .= "<a class='text-shadow-large' style='text-decoration: none;'
			href='index.php'>$title</a></div></div>";
	echo $header;

	$archive_root = ARCHIVE_DIR;

	$media_type = "videos";
	$selected = "";
	$media_dir = "";
	$prev_index = 0;
	$label = "??";
	$env = "";
	$year = "";
	$stills_thumb_rescan = true;

	if (isset($_GET["newtype"]))
		$media_type = $_GET["newtype"];	// "videos" or "stills"
	else if (isset($_GET["type"]))
		$media_type = $_GET["type"];	// "videos" or "stills"
	if ("$media_type" == "thumbs")
		$media_type = "videos";			// transition from old version

	if (isset($_GET["label"]))			// Descriptive label of media_mode
		$label = $_GET["label"];

	if (isset($_GET["file"]))
		$selected = $_GET["file"];

	if (isset($_GET["dir"]))
		$media_dir = $_GET["dir"];

	if ("$media_mode" == "archive")
		{
		$year = $_GET["year"];
		$month0 = $_GET["m0"];
		$day0 = $_GET["d0"];
		$month1 = $_GET["m1"];
		$day1 = $_GET["d1"];
		$env = "mode=$media_mode&type=$media_type&label=$label&year=$year&m0=$month0&d0=$day0&m1=$month1&d1=$day1";
		}
	else if ("$media_mode" == "loop")
		{
		$media_dir = "loop";
		$env = "mode=$media_mode&type=$media_type";
		}
	else
		{
		$media_dir = "media";
		$env = "mode=$media_mode&type=$media_type";
		}

	if (isset($_GET["videos_mode_list"]))
		{
		$videos_mode = "list";
		config_user_save();
		}
	if (isset($_GET["videos_mode_thumbs"]))
		{
		$videos_mode = "thumbs";
		config_user_save();
		}
	if (isset($_GET["loop_mode_list"]))
		{
		$loop_mode = "list";
		config_user_save();
		}
	if (isset($_GET["loop_mode_thumbs"]))
		{
		$loop_mode = "thumbs";
		config_user_save();
		}
	if (isset($_GET["stills_mode_list"]))
		{
		$stills_mode = "list";
		config_user_save();
		}
	if (isset($_GET["stills_mode_thumbs"]))
		{
		$stills_mode = "thumbs";
		config_user_save();
		}

	if ("$media_type" == "stills")
		{
		$media_view = $stills_mode;
		$media_subdir = "stills";
		}
	else if ("$media_mode" == "loop")
		{
		$media_view = $loop_mode;
		if ("$loop_mode" == "thumbs")
			$media_subdir = "thumbs";
		else
			$media_subdir = "videos";
		}
	else
		{
		$media_view = $videos_mode;
		if ("$videos_mode" == "thumbs")
			$media_subdir = "thumbs";
		else
			$media_subdir = "videos";
		}

//echo "<script type='text/javascript'>alert('$media_dir $media_type $media_subdir $videos_mode');</script>";
//echo "<script type='text/javascript'>alert('$env');</script>";


	if (isset($_GET["toggle_scroll"]))
		{
		if ("$media_mode" == "archive")
			{
			if ("$archive_thumbs_scrolled" == "yes")
				$archive_thumbs_scrolled = "no";
			else
				$archive_thumbs_scrolled = "yes";
			}
		else if ("$media_mode" == "loop")
			{
			if ("$loop_thumbs_scrolled" == "yes")
				$loop_thumbs_scrolled = "no";
			else
				$loop_thumbs_scrolled = "yes";
			}
		else
			{
			if ("$media_thumbs_scrolled" == "yes")
				$media_thumbs_scrolled = "no";
			else
				$media_thumbs_scrolled = "yes";
			}
		config_user_save();
//		restart_page($selected);
		}
	if (isset($_GET["toggle_name_style"]))
		{
		if ("$name_style" == "short")
			$name_style = "full";
		else
			$name_style = "short";
		config_user_save();
//		restart_page($selected);
		}

	if (isset($_GET["inc_columns"]))
		{
		if ($n_columns < 10)
			$n_columns += 1;;
		config_user_save();
//		restart_page($selected);
		}
	if (isset($_GET["dec_columns"]))
		{
		if ($n_columns > 2)
			$n_columns -= 1;;
		config_user_save();
//		restart_page($selected);
		}

	if (isset($_POST['action']) && !empty($_POST['file_list']))
		{
		$action = $_POST['action'];
		foreach ($_POST['file_list'] as $file)
			{
			$parts = explode("/", $file);
			$ymd = $parts[0];
			$date = explode("-", $ymd);
			$fname = $parts[1];				// mp4 name is passed in thumb mode
			if ($action == "delete_selected")
				{
				if ("$media_mode" == "archive")
					$media_dir = archive_media_dir($date[0], $date[1], $date[2]);
				delete_file($media_dir, $fname);
				}
			else if ($action == "archive_selected")
				{
				$fifo = fopen(FIFO_FILE,"w");
				if ("$media_type" == "videos")
					{
					fwrite($fifo, "archive_video $fname $ymd");
					$path = "$media_dir/videos/$fname";
					}
				else if ("$media_type" == "stills")
					{
					fwrite($fifo, "archive_still $fname $ymd");
					$path = "$media_dir/stills/$fname";
					}
				fclose($fifo);
				wait_files_gone("file", "$path");
				}
			}
		restart_page($selected);
		}

	if (isset($_GET["archive"]))
		{
		$fname = $_GET["archive"];	// mp4 passed for videos or thumbs type
		$ymd = $_GET["date"];
		$fifo = fopen(FIFO_FILE,"w");
		if ("$media_type" == "videos")
			{
			fwrite($fifo, "archive_video $fname $ymd");
			$subdir = "videos";
			}
		else
			{
			fwrite($fifo, "archive_still $fname $ymd");
			$subdir = "stills";
			}
		fclose($fifo);
		wait_files_gone("file", "$media_dir/$sub_dir/$fname");
		restart_page($selected);
		}

	if (isset($_GET["archive_date"]))
		{
		$ymd = $_GET["archive_date"];
		$fifo = fopen(FIFO_FILE,"w");
		if ("$media_type" == "videos")
			fwrite($fifo, "archive_video day $ymd");
		else
			fwrite($fifo, "archive_still day $ymd");
		fclose($fifo);
		wait_files_gone("day", "$ymd");
		restart_page($selected);
		}

	if (isset($_GET["delete"]))
		{
		$del_name = $_GET["delete"];
		delete_file($media_dir, $del_name);
		restart_page($selected);
		}

	if (isset($_GET["delete_day"]))
		{
		$ymd = $_GET["delete_day"];
		delete_day($media_dir, $ymd);
		restart_page($selected);
		}

	if (isset($_GET["delete_all"]))
		{
		if ("$media_mode" == "archive")
			delete_archive_range($year, $month0, $day0, $month1, $day1);
		else
			delete_all_files($media_dir);
		restart_page("");
		}

	media_array_create();
	$index = media_array_index("$selected");

	if (   "$media_view" == "thumbs" &&
	       (   ("$media_mode" == "archive" && "$archive_thumbs_scrolled" == "no")
	        || ("$media_mode" == "media" && "$media_thumbs_scrolled" == "no")
	        || ("$media_mode" == "loop" && "$loop_thumbs_scrolled" == "no")
	       )
	   )
		$scrolled = "no";
	else
		$scrolled = "yes";

	if ($index >= 0)
		{
		$file_path = $media_array[$index]['file_path'];
		$file_name = $media_array[$index]['file_name'];

		if ("$media_type" == "stills")
			echo "<a href=$file_path target='_blank'>
					<img src=\"$file_path\"
					style='max-width:100%;'
				    style='border:4px groove silver;'>
			      </a>";
		else if ("$scrolled" == "yes")
			{
			$thumb_path = $media_array[$index]['thumb_path'];
			echo "<video controls width='640' style='border:4px groove silver;'>
				    <source src=\"$file_path\" type='video/mp4'>
				    Your browser does not support the video tag.
				  </video>";
			if (is_file($thumb_path))
				echo "<img src=\"$thumb_path\" style='border:4px groove silver;'>";
			else
				echo "<img src=\"$background_image\"
					style='width:150px; height:150px; border:4px groove silver;'>";
			}
		if ("$scrolled" == "yes")
			{
			// Offset a div under the thumb a bit to the left
			echo "<div style='margin: auto; overflow: visible;'>";
			echo   "<div style='margin-right:6px; margin-top: 6px'>";
			echo "<selected>&nbsp; $file_name</selected>";

			$idx = $index + 1;
			if ($idx >= $media_array_size)
				$idx = $media_array_size - 1;
			$next_select = $media_array[$idx]['file_name'];
			echo "<input type='image' src='images/arrow-left.png'
					style='margin-left:16px; vertical-align: bottom;'
					onclick='window.location=\"media-archive.php?$env&file=$next_select\";'>";

			$idx = $index - 1;
			if ($idx < 0)
				$idx = 0;
			$next_select = $media_array[$idx]['file_name'];
			echo "<input type='image' src='images/arrow-right.png'
					style='margin-left:3px; vertical-align: bottom;'
					onclick='window.location=\"media-archive.php?$env&file=$next_select\";'>";

			$wopen = "download.php?file=$file_path";
			echo "<input type='button' value='Download'
					class='btn-control'
					style='margin-left: 16px;'
					onclick='window.open(\"$wopen\", \"_blank\");'
				  > ";
			$media_dir = $media_array[$index]['media_dir'];
			if ($index > 0)
				$next_file = "&file=$next_select";
			else
				$next_file = "";
			$left_margin = 100;
			if ("$media_mode" != "archive")
				{
				$ymd = $media_array[$index]['date'];
				echo "<input type='button' value='Archive'
						class='btn-control'
						style='margin-left: 100px;'
						onclick='window.location=\"media-archive.php?$env&date=$ymd&dir=$media_dir&archive=$file_name$next_file\";'
				  	>";
				$left_margin = 10;
				}
			echo "<input type='button' value='Delete'
					class='btn-control alert-control'
					style=\"margin-left: $left_margin;\"
					onclick='window.location=\"media-archive.php?$env&dir=$media_dir&delete=$file_name$next_file\";'
				  >";
			echo "</div>";
			echo "</div>";
			}
		}
	else
		{
		echo "<p style='margin-top:20px; margin-bottom:20px;'>
				<h4>------</h4>
			  </p>";
		}
	echo "</div>";

	echo "<div style='color: $default_text_color; margin-left:8px; margin-top:8px; margin-bottom:6px;'>";
	if ("$media_mode" == "archive")
		$media_label = "Archive: $year $label";
	else
		$media_label = "";

//	echo "<span style=\"font-size: 1.2em; font-weight: 500;\">
//			$media_label</span>";
	if ("$media_mode" == "loop")
		{
		echo "<span style=\"margin-left: 4px; font-size: 1.4em; font-weight: 500;\">Loop</span> &nbsp;&nbsp;";
		echo "<a href=\"media-archive.php?mode=media&type=videos\"
					class='btn-control' style='margin-left:8px;'>Videos</a>";
		echo "<a href=\"media-archive.php?mode=media&type=stills\"
					class='btn-control' style='margin-left:4px;'>Stills</a>";
		}
	else
		{
		if ("$media_type" == "videos")
			{
			echo "<span style=\"margin-left: 4px; font-size: 1.4em; font-weight: 500;\">Videos $media_label</span> &nbsp;&nbsp;";
			echo "<a href=\"media-archive.php?newtype=stills&$env\"
					class='btn-control' style='margin-left:8px;'>Stills</a>";
			echo "<a href=\"media-archive.php?mode=loop&type=videos\"
					class='btn-control' style='margin-left:4px;'>Loop</a>";
			}
		else if ("$media_type" == "stills")
			{
			echo "<span style=\"margin-left: 4px; font-size: 1.4em; font-weight: 500;\">Stills $media_label</span> &nbsp; &nbsp;";
			echo "<a href=\"media-archive.php?newtype=videos&$env\"
					class='btn-control' style='margin-left:8px;'>Videos</a>";
			echo "<a href=\"media-archive.php?mode=loop&newtype=videos\"
					class='btn-control' style='margin-left:4px;'>Loop</a>";
			}
		}

	if ("$media_mode" == "archive")
		{
		$disk_total = disk_total_space($archive_root);
		$disk_free = disk_free_space($archive_root);
		}
	else
		{
		$disk_total = disk_total_space($media_dir);
		$disk_free = disk_free_space($media_dir);
		}

	$free_percent = sprintf('%.1f',($disk_free / $disk_total) * 100);

	$total = eng_filesize($disk_total);
	$free = eng_filesize($disk_free);

	echo "<span style=\"float: top; margin-left:30px; font-size: 0.96em; font-weight:550; color: $default_text_color\">
		Disk:&thinsp;${total}B &nbsp Free:&thinsp;${free}B&thinsp;($free_percent %)</span>";

	echo "<span style='float:right;'>";
	if ("$media_mode" == "loop")
		{
		if ("$loop_mode" == "thumbs")
			echo "<a href='media-archive.php?$env&loop_mode_list'>List View</a>";
		else
			echo "<a href='media-archive.php?$env&loop_mode_thumbs'>Thumbs View</a>";
		}
	else if ("$media_type" == "stills")
		{
		if ("$stills_mode" == "thumbs")
			echo "<a href='media-archive.php?$env&stills_mode_list'>List View</a>";
		else
			echo "<a href='media-archive.php?$env&stills_mode_thumbs'>Thumbs View</a>";
		}
	else
		{
		if ("$videos_mode" == "thumbs")
			echo "<a href='media-archive.php?$env&videos_mode_list'>List View</a>";
		else
			echo "<a href='media-archive.php?$env&videos_mode_thumbs'>Thumbs View</a>";
		}
	echo "</span>";
	echo "</div>";

	if ($media_array_size == 0)
		echo "<p>No files.</p>";
	else
		{
		$ymd_header = "";

		if ("$scrolled" == "yes")
			{
			if ("$media_type" == "videos")
				$div_style = "overflow-y: scroll; height:${n_video_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
			else
				$div_style = "overflow-y: scroll; height:${n_still_scroll_pixels}px; overflow-x: auto; border:4px groove silver";
			}
		else
			$div_style = "margin: 20px; border: 4px";

		if ("$media_view" == "thumbs")
			echo "<form method=\"POST\" action=\"media-archive.php?$env\">";
		echo "<div style=\"$div_style\">";
		if ("$scrolled" == "yes")
			echo "<table width='100%' cellpadding='2'>";
		else
			echo "<table width='100%' cellpadding='2' frame='box'>";
		$next_select = "";
		for ($k = 0; $k < $media_array_size; $k = $last)
			{
			$ymd = $media_array[$k]['date'];
			if ($k > 0)
				$next_select = $media_array[$k - 1]['file_name'];	// look back one
			if ("$ymd_header" != "$ymd")
				{
				echo "<td style='vertical-align: bottom; padding-bottom:6px; padding-top:18px'>";
				$date_string = date('D - M j Y', $media_array[$k]['mtime']);
				echo "<span style='margin-left: 4px; font-size: 1.0em; font-weight: 500; color: $default_text_color;'>
						$date_string</span>";
				$ymd_header = $ymd;
				$dir = $media_array[$k]['media_dir'];
				if ($n_columns > 2 && "$media_view" != "thumbs")
					echo "</td><td>";
				if ("$next_select" != "")
					$next_file = "&file=$next_select";
				else
					$next_file = "";
				if ("$media_view" == "thumbs" && "$media_mode" != "loop")
					echo "<input style='margin-left: 16px' type='checkbox' name='checkbox_list[]'
						onClick=\"select_day(this, '$ymd')\"/>";
				else if ("$media_mode" != "loop")
					{
					if ("$media_mode" != "archive")
						{
						echo "<input type='button' value='Archive Day'
							class='btn-control'
							style='margin-left: 32px; margin-bottom:4px; margin-top:24px; font-size: 0.82em; text-align: left;'
							onclick='if (confirm(\"Archive day $ymd?\"))
							  {window.location=\"media-archive.php?$env&dir=$dir&archive_date=$ymd$next_file\";}'>";
						if ($n_columns > 2 && "$media_view" != "thumbs")
							echo "</td><td>";
						}
					echo "<input type='button' value='Delete Day'
						class='btn-control alert-control'
						style='margin-left: 32px; margin-bottom:4px; margin-top:24px; font-size: 0.82em; text-align: left;'
						onclick='if (confirm(\"Delete day $ymd?\"))
						  {window.location=\"media-archive.php?$env&dir=$dir&delete_day=$ymd$next_file\";}'>";
					}
				echo "</td>";

				for ($last = $k; $last < $media_array_size && $media_array[$last]['date'] == $ymd; ++$last)
					;
				$n_rows = ceil(($last - $k) / $n_columns);
				}

			if ("$media_view" == "thumbs")
				{
				echo "<tr><td>";
				for ($idx = $k; $idx < $last; ++$idx)
					{
					$thumb_path = $media_array[$idx]['thumb_path'];
					$fname = $media_array[$idx]['file_name'];
					$path = $media_array[$idx]['file_path'];
					$fsize = eng_filesize(filesize("$path"));
					$display_name = $media_array[$idx]['short_name'];
					$color = $media_text_color;
					$border_color = "";
					if ("$scrolled" == "yes" && $fname == $media_array[$index]['file_name'])
						{
						$color = $selected_text_color;
						$border_color = "border-color: $selected_text_color;";
						echo "<a id='selected' class='anchor' style='display:inline'></a>";
						}
					$out = "<fieldset style=\"display:inline; $border_color margin:1px; padding:2px 0px 1px 1px; vertical-align:top; font-size: 0.88em\">";
					$out .= "<span style=\"color: $color;\">$display_name &nbsp</span>";
					$out .= "<span style='float:right'><input type='checkbox' name='file_list[]' value=\"$ymd/$fname\"/></span>";
					if ("$media_mode" == "loop")
						{
						if (substr($fname, -5, 1) == "0")
							$fsize = "";
						else
							$fsize = "motion";
						}
					$out .= "<span style='float:right; color: $default_text_color;'>$fsize</span><br>";
					if (substr($fname, 0, 3) == "man")
						$out .= "<span style=\"color: $color;\">Manual</span><br>";
					else if (substr($fname, 0, 2) == "tl")
						{
						$period = "---";
						$parts = explode("_", $fname);
						if (count($parts) == 4)
							{
							$tail = explode(".", $parts[3]);
							if (count($tail) == 2)
								$period = $tail[0];
							}
						if (substr($period, 0, 1) == "0")
							$period = "---";
						$out .= "<span style=\"color: $color;\">Timelapse: $period</span><br>";
						}
					else if (substr($fname, 0, 4) == "loop")
						{
						if ("$media_mode" == "archive")
							$out .= "<span style=\"color: $color;\">Loop</span><br>";
						}
					else if (   "$media_type" == "stills"
					         && substr($fname, 0, 6) != "image_")
						{
						$pos = strpos($fname, "_");
						$prefix = substr($fname, 0, $pos);
						$out .= "<span style=\"color: $color;\">$prefix</span><br>";
						}
					echo "$out";
					if ("$scrolled" == "yes")
						{
						echo "<a href=\"media-archive.php?$env&file=$fname\">";
						if ("$media_type" == "stills")
							echo still_thumb($idx);
						else
							echo "<img src=\"$thumb_path\" style='padding:1px 5px 2px 5px'/></a></fieldset>";
						}
					else
						{
						if ("$video_url" == "")
							echo "<a href=\"$path\">";
						else
							echo "<a href=\"$video_url$fname\">";
						if ("$media_type" == "stills")
							echo still_thumb($idx);
						else
							echo "<img src=\"$thumb_path\" style='padding:1px 10px 2px 10px'/></a></fieldset>";
						}
					}
				echo "</td></tr>";
				}
			else
				{
				for ($row = 0; $row < $n_rows; ++$row)
					{
					echo "<tr>";
					for ($col = 0; $col < $n_columns; ++$col)
						{
						echo "<td style='font-size: 0.92em;'>";
						$idx = $k + $row + $col * $n_rows;
						if ($idx < $last)
							{
							$fname = $media_array[$idx]['file_name'];
							$path = $media_array[$idx]['file_path'];
							$fsize = eng_filesize(filesize("$path"));
							if ($name_style == "short")
								$display_name = $media_array[$idx]['short_name'];
							else
								$display_name = $fname;

							$border_color = "";
							if ($fname == $media_array[$index]['file_name'])
								{
								$color = $selected_text_color;
								echo "<a id='selected' class='anchor'></a>";
								}
							else if (substr($fname, 0, 3) == "man")
								$color = $manual_video_text_color;
							else
								$color = $media_text_color;

							if ("$media_mode" == "loop")
								echo "<a href='media-archive.php?$env&file=$fname'
									style=\"color: $color; text-decoration: none;\">$display_name</a>";
							else
								echo "<a href='media-archive.php?$env&file=$fname'
									style=\"color: $color; text-decoration: none;\">$display_name</a>
									<span style='font-size: 0.86em; color: $default_text_color;'>
										($fsize)</span>";
							}
						else
							echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
						echo "</td>";
						}
					echo "</tr>";
					}
				}
			}
		echo "</table>";
		echo "</div>";
		}

	echo "<div style='margin-top:12px;'>";
	$title = TITLE_STRING;
	echo "<a href='index.php' class='btn-control'
		style='margin-left:8px;'>
		$title</a>";

	echo "<a href='archive.php' class='btn-control'
		style=\"margin-left: 8px;\">
		Archive Calendar</a>";

	echo "<span style=\"color: $default_text_color;\">";
	if ("$media_view" == "thumbs")
		{
		echo "<span style='margin-left: 50px;'>Selections:</span>";
		if ("$media_mode" != "archive")
			echo "<button type='submit' class='btn-control' style='margin-left: 8px';
				value='archive_selected' name='action'
				onclick=\"return confirm('Archive selected media?');\">
				Archive</button>";
		echo "<button type='submit' class='btn-control alert-control' style='margin-left: 8px'
			value='delete_selected' name='action'
				onclick=\"return confirm('Delete selected media?');\">
			Delete</button>";
		echo "<span style='float:right;'>";
		echo "Select All";
		echo "<input style='margin-right:16px;' type='checkbox' onClick='select_all(this)'/>";
		echo "Files:&thinsp;$media_array_size";
		echo "<a style='margin-left:32px;' href='media-archive.php?$env&toggle_scroll'>
				Toggle Scrolled</a>";
		echo "</span>";
		}
	else
		{
		echo "<span style='margin-left: 70px;'>";
		echo "<input type='button' value='Delete All'
			class='btn-control alert-control' style='margin-right:40px;'
			onclick='if (confirm(\"Delete all $year $label?\"))
			  {window.location=\"media-archive.php?$env&delete_all\";}'>";
		echo "</span>";

		echo "<span style='float:right;'>";
		echo "Files:&thinsp;$media_array_size";
		echo "<a style='margin-left: 32px; margin-right:24px;' href='media-archive.php?$env&toggle_name_style'>
			Name Style</a>";
		echo "&nbsp;&nbsp;Columns $n_columns:";
		if ($n_columns > 2)
			echo "<input type='button' value='-'
				class='btn-control' style='margin-left:6px;'
				onclick='window.location=\"media-archive.php?$env&dec_columns\";'>";
		if ($n_columns < 10)
			echo "<input type='button' value='+'
				class='btn-control' style='margin-left:6px;'
				onclick='window.location=\"media-archive.php?$env&inc_columns\";'>";
		echo "</span>";
		}
	echo "</span>";

	echo "</div>";
	if ("$media_view" == "thumbs")
		echo "</form>";

	echo "</div></body></html>";
?>
