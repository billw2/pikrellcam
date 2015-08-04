<!DOCTYPE html>

<?php
	require_once(dirname(__FILE__) . '/config.php');
?>

<?php
function eng_filesize($bytes, $decimals = 1)
	{
	$sz = 'BKMGTP';
	$factor = floor((strlen($bytes) - 1) / 3);
	return sprintf("%.{$decimals}f", $bytes / pow(1000, $factor)) . @$sz[$factor];
	}

function media_file_array($dir)
	{
	$media_array = array();
	$file_array = array_slice(scandir($dir), 2);
	$n_files = count($file_array);

	foreach($file_array as $name)
		{
		$mtime = filemtime($dir.'/'.$name);
		$ymd = date("Y_m_d", $mtime);
		$media_array[] = array('name'  => $name,
		                       'mtime' => $mtime,
		                       'date'  => $ymd);
		}

	usort($media_array, create_function('$a, $b',
				'return strcmp($a["mtime"], $b["mtime"]);'));
	krsort($media_array);
	$media_array = array_values($media_array);

	return $media_array;
	}

function next_select($dir, $cur_file)
	{
	$next_file = "";
	$file_array = media_file_array($dir);
	$n_files = count($file_array);

	if ($n_files > 0)
		{
		if ($cur_file == "")
			$i = 0;
		else
			{
			for ($i = 0; $i < $n_files; $i++)
				{
				if ($cur_file == $file_array[$i]['name'])
					break;
				}
			if ($i == $n_files)
				$i = 0;
			else if ($i > 0)
				--$i;
			}
		$next_file = $file_array[$i]['name'];
		}
	return $next_file;
	}
?>

<script>
function scroll_to_selected()
	{
	document.getElementById("selected").scrollIntoView(true);
	}
</script>

<style type="text/css">
a.anchor
	{
	display: block; position: relative; top: -250px; visibility: hidden;
	}
</style>

<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>PiKrellCam Media</title>
    <link rel="stylesheet" href="js-css/pikrellcam.css" />
  </head>

  <body onload="scroll_to_selected()" background="images/paper1.png">
    <div>
		<?php
		echo "<div class='text-center'>";
		// Offset TITLE_STRING to left of center 120px (compensate for 150px thumb)
		echo "<div style='margin: auto; overflow: visible;'>";
		echo   "<div style='margin-right:120px;'>";
		echo "<a class='text-shadow-large'
				style='text-decoration: none;'
				href='index.php'>";
		echo     TITLE_STRING;
		echo   "</a></div>";
		echo   "</div>";

		$media_dir = $_GET["dir"];

		if (isset($_GET["file"]))
			$selected = $_GET["file"];
		else if (isset($_GET["delete"]))
			{
			$del_file = $_GET["delete"];
//			echo "<script type='text/javascript'>alert('$media_dir . \"/\" . $del_file');</script>";
			$selected = next_select($media_dir, $del_file);
			unlink("$media_dir" . "/" . $del_file);
			if ($selected == $del_file)
				$selected = next_select($media_dir, "");
			if ("$media_dir" == "media/videos")
				{
				$base = str_replace(".mp4", "", $del_file);
				unlink("media/thumbs/" . "$base" . ".th.jpg");
				}
			}
		else if (isset($_GET["delete_day"]))
			{
			$ymd = $_GET["day"];
			$file_array = media_file_array($media_dir);
			$n_files = count($file_array);
			$selected = "";
			$searching = 1;
			for ($i = 0; $i < $n_files; $i++)
				{
				if ($ymd != $file_array[$i]['date'])
					{
					if ($searching == 1 || "$selected" == "")
						$selected = $file_array[$i]['name'];
					continue;
					}
				$searching = 0;
				$del_file = $file_array[$i]['name'];
//	echo "<script type='text/javascript'>alert('$media_dir . \"/\" . $del_file');</script>";
				unlink("$media_dir" . "/" . $del_file);
				if ("$media_dir" == "media/videos")
					{
					$base = str_replace(".mp4", "", $del_file);
					unlink("media/thumbs/" . "$base" . ".th.jpg");
					}
				}
			}
		else if (isset($_GET["delete_all"]))
			{
			array_map('unlink', glob("$media_dir" . "/*.mp4"));
			array_map('unlink', glob("$media_dir" . "/*.h264"));
			array_map('unlink', glob("$media_dir" . "/*.jpg"));
			$selected = "";
			if ("$media_dir" == "media/videos")
				{
				array_map('unlink', glob("media/thumbs" . "/*.th.jpg"));
				}
			}
		else
			$selected = next_select($media_dir, "");

		if ($selected != "" && is_file("$media_dir" . "/" . $selected))
			{
			$extension = strtolower(substr(strrchr($selected, "."), 1));
			if ($extension == "jpg")
				echo "<a href='$media_dir" . '/' . $selected . "' target='_blank'>
                        <img src='$media_dir" . '/' . $selected . "'
						style='max-width:100%;'
					    style='border:6px groove silver;'>
				      </a>";
			else
				{
				echo "<video controls width='640' style='border:6px groove silver;'>
					    <source src='$media_dir" . '/' . $selected . "' type='video/mp4'>
					    Your browser does not support the video tag.
					  </video>";
				$base = str_replace(".mp4", "", $selected);
				$thumb = "media/thumbs/" . "$base" . ".th.jpg";
				if (is_file($thumb))
					echo "<img src='$thumb'
						style='border:6px groove silver;'>";
				else
                   echo "<img src='images/paper1.png'
						style='width:150px; height:150px; border:6px groove silver;'>";
				}
			// Offset to left of center 120px (compensate for 150px thumb)
			echo "<div style='margin: auto; overflow: visible;'>";
			echo   "<div style='margin-right:120px;'>";
			  echo "<selected>&nbsp; $selected</selected>";
              $wopen = "download.php?file=" . $media_dir . "/". $selected;
			  echo "<input type='button' value='Download'
                    class='btn-control'
                    style='margin-left: 12px;'
					onclick='window.open(\"$wopen\", \"_blank\");'
                  > ";
              echo "<input type='button' value='Delete'
                      class='btn-control alert-control'
                      style='margin-left: 32px;'
				      onclick='window.location=\"media.php?dir=$media_dir&delete=" . $selected . "\";'
                  > &nbsp;";
            echo "</div>";
            echo "</div>";
			}
		else
			{
			echo "<p style='margin-top:20px; margin-bottom:20px;'>
					<h4>------</h4>
				  </p>";
			}
		echo "</div>";

		echo "<div style='margin-left:8px; margin-top:8px; margin-bottom:6px;'>";
		echo "<span style='font-size: 1.2em; font-weight: bold;'>
				$media_dir</span>";
		echo "</div>";

		$file_array = media_file_array($media_dir);
		$n_files = count($file_array);

		if ($n_files == 0)
			echo "<p>No files.</p>";
		else
			{
			$ymd_header = "";
			echo '<div id="" style="overflow-y: scroll; height:380px; overflow-x: auto; border:4px groove silver">';
			echo "<table width='98%' cellpadding='2'>";
			for ($k = 0; $k < $n_files; $k = $last)
				{
				$ymd = $file_array[$k]['date'];
				if ("$ymd_header" != "$ymd")
					{
					$date_string = date('D - M j Y', $file_array[$k]['mtime']);
					echo "<tr><td><span style='margin-left: 8px; font-size: 1.1em; font-weight: bold;'>
							$date_string</span>";
					$ymd_header = $ymd;

					echo "<input type='button' value='Delete Day'
						class='btn-control alert-control'
						style='margin-left: 32px; margin-bottom:6px; margin-top:24px;'
						onclick='if (confirm(\"Delete day $ymd?\"))
						{window.location=\"media.php?dir=$media_dir&delete_day&day=$ymd\";}'>";
					echo "</td></tr><br>";

					$n_rows = 1;
					for ($last = $k; $file_array[$last]['date'] == $ymd; ++$last)
						;
					$n_rows = intval(($last - $k) / 2 + 1);
//					echo "<br><tr><td>rows=$n_rows last=$last</td></tr><br>";
					}
				for ($row = 0; $row < $n_rows; ++$row)
					{
					echo "<tr>";
					for ($col = 0; $col < 2; ++$col)
						{
						$idx = $k + $row + $col * $n_rows;
						if ($idx < $last)
							{
							echo "<td style='font-size: 0.95em;'>";
							$fname = $file_array[$idx]['name'];
							$path = "$media_dir" . "/" . "$fname";
							$fsize = eng_filesize(filesize("$path"));
							if ($fname == $selected)
								{
								echo "<a id='selected' class='anchor'></a>";
								echo "<a href='media.php?dir=$media_dir&file=$fname' style='color: #400808; text-decoration: none'>$fname</a> ($fsize)";
								}
							else
								{
								echo "<a href='media.php?dir=$media_dir&file=$fname' style='text-decoration: none'>$fname</a> ($fsize)";
								}
							echo "</td>";
							}
						}
					echo "</tr>";
					}
				}

			echo "</table>";
			echo "</div>";

			echo "<div style='margin-top:12px;'>";
			$title = TITLE_STRING;
			echo "<a href='index.php' class='btn-control'
				style='margin-left:8px;'>
				$title</a>";
			echo "<a href='thumbs.php' class='btn-control'
				style='margin-left:8px;'>
				Thumbs</a>";
			echo "<input type='button' value='Delete All'
				class='btn-control alert-control'
				style='margin-left:32px;'
				onclick='if (confirm(\"Delete all?\"))
				 {window.location=\"media.php?dir=$media_dir&delete_all\";}'>";
			echo "</div>";
			}
	  ?>

    </div>
  </body>
</html>
