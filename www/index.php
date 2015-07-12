<?php
if (file_exists('password.php'))
	{
	require_once("password_check.php");
	}
?>
<?php
	require_once(dirname(__FILE__) . '/config.php');
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

<body background="images/paper1.png" onload="mjpeg_start();">
    <div class="text-center">
      <?php
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
        onclick="fifo_command('record off')"
        width="30" height="30"
      >
      <input type="image" src="images/pause.png"
        onclick="fifo_command('pause')"
        width="30" height="30"
      >
      <input type="image" src="images/record.png"
        onclick="fifo_command('record on')"
        width="30" height="30"
      >
      <input type="image" src="images/shutter.png"
        width="30" height="30"
        onclick="fifo_command('still')"
        style="margin-left:40px;"
      >
    </div>

	<div id="container" class="top-margin">
      Files:
      <a href="media.php?dir=<?php echo VIDEO_DIR; ?>"
        class="btn-control"
      >Videos</a>
      <a href="media.php?dir=<?php echo STILL_DIR; ?>"
        class="btn-control"
        style="margin-right:20px;"
      >Stills</a>
      Enable:
      <input type="button" id="motion_button" value="Motion"
         onclick="fifo_command('motion_enable toggle')"
         class="btn-control motion-control"
      >
      <span style="float: right;">
        Show:
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
                         <input type="text" id="load_regions" size=6 >
                         <input type="button" value="Load"
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
                       <td style="border: 0;" align="center">
                         <input type="button" value="New"
                           onclick="new_region();"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;" align="right">
                         <input type="checkbox" name="move_mode"
                           onclick='move_region_mode(this);' checked> Coarse
                       </td>
                    </tr>
 
                   <tr align="right">
                       <td style="border: 0;" align="center">
                         <input type="button" value="Delete"
                           onclick="fifo_command('motion delete_regions selected');"
                           class="btn-control alert-control"
                         >
                       </td>
                       <td style="border: 0;" align="right">Select
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
                         <input type="button" value="+Y"
                           onclick="move_region(' y m');"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;"> </td>

                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="button" value="-DY"
                           onclick="move_region(' dy m');"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;" align="right">
                       </td>
                    </tr>

                    <tr>
                       <td style="border: 0;">
                         <input type="button" value="-X"
                           onclick="move_region(' x m');"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;">Move</td>
                       <td style="border: 0;">
                         <input type="button" value="+X"
                           onclick="move_region(' x p');"
                           class="btn-control"
                         >
                       </td>

                       <td style="border: 0;">
                         <input type="button" value="-DX"
                           onclick="move_region(' dx m');"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;" align="center">Size</td>
                       <td style="border: 0;">
                         <input type="button" value="+DX"
                           onclick="move_region(' dx p');"
                           class="btn-control"
                         >
                       </td>
                    </tr>

                    <tr>
                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="button" value="-Y" align="center"
                           onclick="move_region(' y p');"
                           class="btn-control"
                         >
                       </td>
                       <td style="border: 0;"> </td>

                       <td style="border: 0;"> </td>
                       <td style="border: 0;" align="center">
                         <input type="button" value="+DY"
                           onclick="move_region(' dy p');"
                           class="btn-control"
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
                    <input type="button" value="<<"
                      onclick="fifo_command('display <<');"
                      class="btn-control"
                    >
                    <input type="button" value="< "
                      onclick="fifo_command('display <');"
                      class="btn-control"
                    >
                  </td>
                  <td style="border: 0;" align="center">
                    <input type="button" value="SEL"
                      onclick="fifo_command('display sel');"
                      class="btn-control"
                    >
                  </td>
                  <td style="border: 0;" align="left">
                    <input type="button" value=" >"
                      onclick="fifo_command('display >');"
                      class="btn-control"
                    >
                    <input type="button" value=">>"
                      onclick="fifo_command('display >>');"
                      class="btn-control"
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
                    <span style="font-weight:600;">
                    Camera Config
                    </span>
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
                    <span style="font-weight:600;">
                    Camera Params
                    </span>
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
                    <span style="font-weight:600;">
                    Motion
                    </span>
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
                    <span style="font-weight:600;">Time Lapse</span>
                    <div>
                      <span style="margin-left:40px;">
                        Period
                      </span>
                      <input type="text" id="tl_period" value="0" size="3"
                      >sec
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
          <input id="stop_button" type="button" value="Stop PiKrellCam"
            onclick="pikrellcam('stop');"
            class="btn-control alert-control"
          >
          <input id="start_button" type="button" value="Start PiKrellCam"
            onclick="pikrellcam('start');"
            class="btn-control"
          >
        </div>
    </div>
</div>
</body>
</html>
