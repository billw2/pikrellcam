<?php
require_once("password.php");
require_once(dirname(__FILE__) . '/config.php');

session_cache_limiter('private');
session_cache_expire(86400);

ini_set('session.cookie_lifetime', 86400);
ini_set('session.gc_maxlifetime', 86400);
ini_set('session.gc_probability', 1);
ini_set('session.gc_divisor', 50);

$session_path = SESSION_PATH;
ini_set("session.save_path", $session_path);

session_start();

class passwdCheck
{
var $password = PASSWORD;
	
function showLoginForm()
	{
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<head>
  <title>PiKrellCam Login</title>
  <link href="js-css/pikrellcam.css" rel="stylesheet" type="text/css" />
  <style>
	#container
		{
    	margin: auto;
    	width: 400px;
		border-width:1px;
		border-style:solid;
    	background-color: #e8e8e8;
		}
  </style>
</head>

<body background="images/paper1.png" style="padding-top:40px">
  <div id="container">
    <div id="header" style="padding:6px; text-align:center; background-color:#d0d0d0;">
      <selected>
         <?php echo TITLE_STRING; ?> Login
      </selected>
    </div>
    <div>
      <form style="margin:18px;" action="<?php echo $_SERVER['PHP_SELF'];?>" method="post">
        <center>
          <p>Password:
            <input name="passwd" type="password" size="16" />
          </p>
          <p>
            <input type="submit" name="submitBtn" class="btn-control" value="Login" />
          </p>
        </center>
      </form>
    </div>
  </div>
</body>		 

<?php
	}
function login()
	{
	if ($this->password == "unset")
	   $_SESSION['loggedin'] = true;
	$loggedin = isset($_SESSION['loggedin']) ? $_SESSION['loggedin'] : false;
	if ( (!isset($_POST['submitBtn'])) && (!($loggedin)))
		{
		$_SESSION['loggedin'] = false;
		$this->showLoginForm();
		exit();
		}
	else if (isset($_POST['submitBtn']))
		{
		$pass = isset($_POST['passwd']) ? $_POST['passwd'] : '';
		if ($pass != $this->password)
			{
			$_SESSION['loggedin'] = false;
			$this->showLoginForm();
			exit();     
			}
		else
			$_SESSION['loggedin'] = true;
		}
	}
}
$protector = new passwdCheck();
$protector->login();
?>
