<?php
//ini_set('display_errors',1);
//ini_set('display_startup_errors',1);
//error_reporting(-1);


	require_once(dirname(__FILE__) . '/config.php');
	include_once(dirname(__FILE__) . '/config-user.php');
	include_once(dirname(__FILE__) . '/config-defaults.php');	

function time_lapse_period()
	{
	$tl_status = "../../.pikrellcam/timelapse.status";
	$f = fopen($tl_status, 'r');
	$tl_period = 1;
	if ($f)
		{
		$input = fgets($f);
		$input = fgets($f);
		sscanf($input, "%d", $tl_period);
		fclose($f);
		}
	return $tl_period;
	}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title><?php echo TITLE_STRING; ?></title>

  <script src="js-css/jquery.min.js"></script>
  <link rel="stylesheet" href="js-css/pikrellcam.css" />
  <script src="js-css/pikrellcam.js"></script>
  <script src="js-css/expandable-panels.js"></script>
  <link rel="stylesheet" href="js-css/expandable-panels.css" />
</head>

<?php
echo "<body background=\"$background_image\" onload=\"mjpeg_start();\">";
    echo "<div class=\"text-center\">";
        echo "<div class='text-shadow-large'>";
        echo TITLE_STRING;
        echo "</div>";
?>
    </div>
      <div class="text-center">
        <img id="mjpeg_image"
          style="border:6px groove silver;"
          onclick="image_expand_toggle();"
        >
      </div>

    <div class="text-center top-margin">
      <input type="image" src="images/stop.png"
		style="vertical-align: bottom;"
        onclick="fifo_command('record off')"
        width="30" height="30"
      >
      <input type="image" src="images/pause.png"
		style="vertical-align: bottom;"
        onclick="fifo_command('pause')"
        width="30" height="30"
      >
      <input type="image" src="images/record.png"
		style="vertical-align: bottom;"
        onclick="fifo_command('record on')"
        width="30" height="30"
      >
      <input type="image" src="images/shutter.png"
        width="30" height="30"
        onclick="fifo_command('still')"
        style="margin-left:20px; vertical-align: bottom;"
      >

<?php
if (defined('INCLUDE_CONTROL'))
	{
	if ($include_control == "yes")
		{
		include 'control.php';
		}
	}

if (file_exists("custom-control.php"))
	{
	include 'custom-control.php';
	}
?>
    </div>

	<div id="container" class="top-margin">
      <a href="archive.php?year=<?php echo date('Y'); ?>"
        class="btn-control"
        style="margin-right:20px;"
      >Archive Calendar</a>
      <?php echo "<span style=\"color: $default_text_color\"> Media:</span>"; ?>
      <a href="media-archive.php?mode=media&type=videos"
        class="btn-control"
      >Videos</a>
      <a href="media-archive.php?mode=media&type=thumbs"
        class="btn-control"
      >Thumbs</a>
      <a href="media-archive.php?mode=media&type=stills"
        class="btn-control"
        style="margin-right:30px;"
      >Stills</a>

      <?php echo "<span style=\"color: $default_text_color\"> Enable:</span>"; ?>
      <input type="button" id="motion_button" value="Motion"
         onclick="fifo_command('motion_enable toggle')"
         class="btn-control motion-control"
      >
      <?php echo "<span style=\"float: right; color: $default_text_color\"> Show:"; ?>
        <input id="timelapse_button" type="button" value="Timelapse"
						onclick="fifo_command('tl_show_status toggle')"
						class="btn-control motion-control"
						>
        <input type="button" id="regions_button" value="Regions"
						onclick="fifo_command('motion show_regions toggle')"
						class="btn-control motion-control"
						>
        <input type="button" id="vectors_button" value="Vectors"
						onclick="fifo_command('motion show_vectors toggle')"
						class="btn-control motion-control"
						>
      </span>
    </div>

<div id="container">
    <div class="expandable-panel" id="cp-1">
        <div class="expandable-panel-heading">
            <h3>Motion Regions<span class="icon-close-open"></span></h3>
      </div>
        <div class="expandable-panel-content">

              <table class="table-container">
                <tr>
                  <td>
                    <table cellpadding="0" cellspacing="0" border="0" table-layout="fixed">
                    <tr>
                      <td style="border: 0;" >
                         <input type="button" value="List" style="margin-right: 12px;"
                           onclick="list_regions();"
                           class="btn-control"
                         >
                         <input type="text" id="load_regions" size=6>
                         <input type="button" value="Load" style="margin-right: 8px;"
                            onclick="load_regions();"
                            class="btn-menu"
                            >
                      </td>
                      <td style="border: 0;" align="right">
                         <input type="text" id="save_regions" size=6 >
                         <input type="button" value="Save"
                           onclick="save_regions();"
                           class="btn-menu"
                           >
                      </td>
                    </tr>

                    <tr>
                       <td style="border: 0;" align="left">
                       </td>
                       <td style="border: 0;" align="right">
                         <?php echo "<span style=\"color: $default_text_color\">
                           Coarse Move</span>"; ?>
                         <input type="checkbox" name="move_mode"
                           onclick='move_region_mode(this);' checked>
                       </td>
                    </tr>
 
                   <tr align="right">
                       <td style="border: 0;" align="right">
                         <input type="button" value="New"
                           onclick="new_region();"
                           class="btn-control"
                         >
                         <input type="button" value="Delete" style="margin-right: 8px;"
                           onclick="fifo_command('motion delete_regions selected');"
                           class="btn-control alert-control"
                         >
                       </td>
                       <td style="border: 0;" align="right">
                       <?php echo "<span style=\"color: $default_text_color\">Select</span>"; ?>
                         <input type="button" value="<"
                           onclick="fifo_command('motion select_region <');"
                           class="btn-control"
                         >
                         <input type="button" value=">"
                           onclick="fifo_command('motion select_region >');"
                           class="btn-control"
                         >
                       </td>
                    </tr>
                    </table>
                  </td>

                  <td>
                    <table cellpadding="0" cellspacing="0" border="0">
                    <tr>
                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="image" src="images/arrow-up.png"
                           onclick="move_region(' y m');"
                         >
                       </td>
                       <td style="border: 0;"> </td>

                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="image" src="images/arrow-up.png"
                           onclick="move_region(' dy p');"
                         >
                       </td>
                       <td style="border: 0;" align="right">
                       </td>
                    </tr>

                    <tr>
                       <td style="border: 0;">
                         <input type="image" src="images/arrow-left.png"
                           onclick="move_region(' x m');"
                         >
                       </td>
                       <td style="border: 0;">
                       <?php echo "<span style=\"color: $default_text_color\">Move</span>"; ?>
                       </td>
                       <td style="border: 0;">
                         <input type="image" src="images/arrow-right.png"
                           onclick="move_region(' x p');"
                         >
                       </td>

                       <td style="border: 0;">
                         <input type="image" src="images/arrow-left.png"
                           onclick="move_region(' dx m');"
                         >
                       </td>
                       <td style="border: 0;" align="center">
                       <?php echo "<span style=\"color: $default_text_color\">Size</span>"; ?>
                       </td>
                       <td style="border: 0;">
                         <input type="image" src="images/arrow-right.png"
                           onclick="move_region(' dx p');"
                         >
                       </td>
                    </tr>

                    <tr>
                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="image" src="images/arrow-down.png"
                           onclick="move_region(' y p');"
                         >
                       </td>
                       <td style="border: 0;"> </td>

                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="image" src="images/arrow-down.png"
                           onclick="move_region(' dy m');"
                         >
                       </td>
                       <td style="border: 0;"> </td>

                    </tr>
                    </table>
                  </td>
                </tr>
              </table>
        </div>
    </div>
     
    <div class="expandable-panel" id="cp-2">
        <div class="expandable-panel-heading">
            <h3>Setup<span class="icon-close-open"></span></h3>
      </div>
        <div class="expandable-panel-content">
              <table class="table-container">
                <tr>
                  <td style="border: 0;" align="right">
                    <input type="image" src="images/arrow2-left.png"
                      style="padding:0px 0px 0px 0px; margin:0;"
                      onclick="fifo_command('display <<');"
                    >
                    <input type="image" src="images/arrow-left.png"
                      style="padding:0px 0px 0px 0px; margin:0;"
                      onclick="fifo_command('display <');"
                    >
                  </td>
                  <td style="border: 0;" align="center">
                    <input type="button" value="SEL"
                      class="btn-control"
                      onclick="fifo_command('display sel');"
                    >
                  </td>
                  <td style="border: 0;" align="left">
                    <input type="image" src="images/arrow-right.png"
                      style="padding:0px 0px 0px 0px; margin:0;"
                      onclick="fifo_command('display >');"
                      class="btn-control"
                    >
                    <input type="image" src="images/arrow2-right.png"
                      style="padding:0px 0px 0px 0px; margin:0;"
                      onclick="fifo_command('display >>');"
                    >
                  </td>
                </tr>

                <tr>
                  <td style="border: 0;" align="right" >
                  </td>
                  <td style="border: 0;" align="center">
                    <input type="button" value="Back"
                      onclick="fifo_command('display back');"
                      class="btn-control"
                    >
                  </td>
                  <td style="border: 0;" align="left" >
                  </td>
                </tr>
              </table>


              <table class="table-container">
                <tr>
                  <td>
                    <?php echo "<span style=\"font-weight:600; color: $default_text_color\"> Camera Config </span>"; ?>
                    <div class="text-center">
                      <input type="button" value="Video Presets"
                        class="btn-menu"
                        onclick="fifo_command('display video_presets');"
                      >
                      <input type="button" value="Still Presets"
                        class="btn-menu"
                        onclick="fifo_command('display still_presets');"
                      >
                      <input type="button" value="Adjustments"
                        class="btn-menu"
                        onclick="fifo_command('display camera_adjustments');"
                      >
                    </div>
                  </td>
                </tr>
                <tr>
                  <td>
                    <?php echo "<span style=\"font-weight:600; color: $default_text_color\"> Camera Params </span>"; ?>
                    <div class="text-center">
                      <input type="button" value="Picture"
                        class="btn-menu"
                        onclick="fifo_command('display picture');"
                      >
                      <input type="button" value="Metering"
                        class="btn-menu"
                        onclick="fifo_command('display metering');"
                      >
                      <input type="button" value="Exposure"
                        class="btn-menu"
                        onclick="fifo_command('display exposure');"
                      >
                      <input type="button" value="White Balance"
                        class="btn-menu"
                        onclick="fifo_command('display white_balance');"
                      >
                      <input type="button" value="Image Effect"
                        class="btn-menu"
                        onclick="fifo_command('display image_effect');"
                      >
                    </div>
                  </td>
                </tr>
                <tr>
                  <td>
                    <?php echo "<span style=\"font-weight:600; color: $default_text_color\"> Motion </span>"; ?>
                    <div class="text-center" >
                      <input type="button" value="Vector Limits"
                        class="btn-menu"
                        onclick="fifo_command('display motion_limit');"
                      >
                      <input type="button" value="Settings"
                        class="btn-menu"
                        onclick="fifo_command('display motion_setting');"
                      >
                      <input type="button" value="Times"
                        class="btn-menu"
                        onclick="fifo_command('display motion_time');"
                      >
                    </div>
                  </td>
                </tr>
                <tr>
                  <td>
                    <?php echo "<span style=\"font-weight:600; color: $default_text_color\"> Time Lapse </span>"; ?>
                    <div>
                    <?php echo "<span style=\"margin-left:40px; font-weight:600; color: $default_text_color\"> Period </span>"; ?>
                      <input type="text" id="tl_period" value="<?php echo time_lapse_period(); ?>" size="3"
                      >
                    <?php echo "<span style=\"margin-left:4px; color: $default_text_color\"> sec </span>"; ?>
                      <input type="button" value="Start"
                        class="btn-menu"
                        onclick="tl_start();"
                        style="float: right; margin-left:10px;"
                      >
                      <input type="button" value="Hold"
                        class="btn-menu"
                        onclick="fifo_command('tl_hold toggle');"
                        style="float: right; margin-left:10px;"
                      >
                      <input type="button" value="End"
                        class="btn-menu alert-control"
                        onclick="fifo_command('tl_end');"
                        style="float: right;"
                      >
                    </div>
                  </td>
                </tr>
              </table>
        </div>
    </div>
     
    <div class="expandable-panel" id="cp-3">
        <div class="expandable-panel-heading">
            <h3>System<span class="icon-close-open"></span></h3>
      </div>
        <div class="expandable-panel-content text-center">
          <?php
            $version = VERSION;
            echo "<span style=\"font-weight:600; color: $default_text_color\">
             PiKrellCam V${version}: </span>";
          ?>
          <input id="stop_button" type="button" value="Stop"
            onclick="pikrellcam('stop');"
            class="btn-control alert-control"
          >

          <input id="start_button" type="button" value="Start"
            onclick="pikrellcam('start');"
            class="btn-control"
          >

          <input id="log_button" type="button" value="Log"
            style="margin-left:32px;"
            onclick="window.location='log.php';"
            class="btn-control"
          >
          <input id="upgrade_button" type="button" value="Upgrade"
            style="margin-left:16px;"
            onclick="fifo_command('upgrade')"
            class="btn-control"
          >
          <a href="help.php"
            class="btn-control" style="margin-left:16px;">Help</a>
        </div>
    </div>
<?php
if (file_exists("custom.php"))
	{
	include 'custom.php';
	}
?>
</div>
</body>
</html>
