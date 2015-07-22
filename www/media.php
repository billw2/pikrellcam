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
	$file_array = array_slice(scandir($dir), 2);
	$n_files = count($file_array);

	for ($i = 0; $i < $n_files; $i++)
		{
		if (!is_file($dir . "/" . $file_array[$i]))
			unset($file_array[$i]);
		}
	$file_array = array_values($file_array);
	return $file_array;
	}

function next_select($dir, $cur_file)
	{
	$next_file = "";
	$file_array = media_file_array($dir);
	$n_files = count($file_array);

	if ($n_files > 0)
		{
		$next_file = $file_array[0];
		if ($cur_file != "")
			{
			for ($i = 0; $i < $n_files; $i++)
				{
				if ($cur_file != $file_array[$i])
					continue;
				if ($i == $n_files - 1 && $i > 0)
					$next_file = $file_array[$i - 1];
				else
					{
					$i++;
					while ($i < $n_files)
						{
						if (is_file("$dir" . "/" . $file_array[$i]))
							{
							$next_file = $file_array[$i];
							break;
							}
						$i++;
						}
					break;
					}
				}
			}
		}
	return $next_file;
	}
?>

<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>PiKrellCam Download Media</title>
    <link rel="stylesheet" href="js-css/pikrellcam.css" />
  </head>

  <body background="images/paper1.png">
    <div>
		<?php

		echo "<div class='text-center'>";
		echo   "<div><a class='text-shadow-large' style='text-decoration: none' href='index.php'>";
		echo     TITLE_STRING;
		echo   "</a>";
		echo   "</div>";

		$media_dir = $_GET["dir"];

		if (isset($_GET["file"]))
			$selected = $_GET["file"];
		else if (isset($_GET["delete"]))
			{
			$del_file = $_GET["delete"];
			$selected = next_select($media_dir, $del_file);
//			echo "<script type='text/javascript'>alert('$media_dir . \"/\" . $del_file');</script>";
			unlink("$media_dir" . "/" . $del_file);
			}
		else if (isset($_GET["delete_all"]))
			{
			array_map('unlink', glob("$media_dir" . "/*.mp4"));
			array_map('unlink', glob("$media_dir" . "/*.h264"));
			array_map('unlink', glob("$media_dir" . "/*.jpg"));
			$selected = "";
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
				echo "<video controls width='640' style='border:6px groove silver;'>
					    <source src='$media_dir" . '/' . $selected . "' type='video/mp4'>
					    Your browser does not support the video tag.
					  </video>";
			echo "<div>
                    <input type='button' value='Delete'
                      class='btn-control alert-control'
				      onclick='window.location=\"media.php?dir=$media_dir&delete=" . $selected . "\";'
                  > &nbsp;";
            $wopen = "download.php?file=" . $media_dir . "/". $selected;
			echo "<input type='button' value='Download'
                    class='btn-control'
					onclick='window.open(\"$wopen\", \"_blank\");'
                  > ";
			echo "<selected>&nbsp; $selected</selected>
                  </div>";
			}
		else
			{
			echo "<p style='margin-top:20px; margin-bottom:20px;'>
					<h4>------</h4>
				  </p>";
			}
		echo "</div>";
		?>

      <h3>Files in <?php echo "$media_dir" ?></h3>
      <?php
		$file_array = media_file_array($media_dir);
		$n_files = count($file_array);

		if ($n_files == 0)
			echo "<p>No files.</p>";
		else
			{
			if ($n_files < 11)
				$columns = 1;
			else
				$columns = 2;
			$rows = ceil($n_files / $columns);

			echo "<table width='95%' cellpadding='2'>";
			for ($row = 0; $row < $rows; $row++)
				{
				echo "<tr>";
				foreach ($file_array as $k => $file)
					{
					if (($k % $rows) == $row)
						{
						echo "<td>";
						if(is_file("$media_dir" . "/" . $file))
							{
							$fsize = eng_filesize(filesize("$media_dir" . "/" . $file));
							if ($file == $selected)
								echo "<a href='media.php?dir=$media_dir&file=$file' style='color: #400808; text-decoration: none'>$file</a> ($fsize)";
							else
								echo "<a href='media.php?dir=$media_dir&file=$file' style='text-decoration: none'>$file</a> ($fsize)";
							}
						echo "</td>";
						}
					}
				echo "</tr>";
				}
			echo "</table>";
			echo "<p><input type='button' value='Delete All'
					class='btn-control alert-control'
					onclick='if (confirm(\"Delete all?\"))
							{window.location=\"media.php?dir=$media_dir&delete_all\";}'>
				  </p>";
			}
	  ?>
	  <p style="margin-top:20px;">
		<a href="index.php">
			<span>
			</span>Back to <?php echo TITLE_STRING; ?>
		</a>
      </p>
    </div>
  </body>
</html>
