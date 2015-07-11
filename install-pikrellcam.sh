#!/bin/bash

PGM=`basename $0`

if [ `id -u` == 0 ]
then
    echo -e "$PGM should not be run as root.\n"
    exit 1
fi

bad_install()
	{
	echo "Cannot find $1 in $PWD"
	echo "Are you running $PGM in the install directory?"
	exit 1
	}

if [ ! -x $PWD/pikrellcam ]
then
	bad_install "program pikrellcam"
fi

if [ ! -d $PWD/www ]
then
	bad_install "directory www"
fi


echo ""
echo "Set the port for the nginx web server."
echo "If you already have a web server configuration using the default"
echo "port 80, you should enter an alternate port for PiKrellCam."
echo "Otherwise you can use the default port 80 or an alternate as you wish."
echo "The port number will be set in: /etc/nginx.sites-available/pikrellcam."
echo -n "Enter web server port: "
read resp
if [ "$resp" == "" ]
then
	PORT=80
else
	PORT=$resp
fi

echo ""
echo "For auto starting at boot, a PiKrellCam start command must be in rc.local."
echo "If you don't start at boot, PiKrellCam can always be started and stopped"
echo "from the web page."
echo -n "Do you want PiKrellCam to be auto started at boot? (yes/no): "
read resp
if [ "$resp" == "y" ] || [ "$resp" == "yes" ]
then
	AUTOSTART=yes
else
	AUTOSTART=no
fi

echo ""
echo "Enter a password if you want a password login for the web page."
echo "Enter a blank entry if you do not want the password login."
echo "The password will be configured in: $PWD/www/password.php"
echo -n "Enter password :"
read resp
if [ "$resp" == "" ]
then
	PASSWORD="unset"
else
	PASSWORD=$resp
fi


echo ""
echo "Starting PiKrellCam install..."

WHEEZY=7.8
DEB_VERSION=`cat /etc/debian_version`

# =============== apt install needed packages ===============
#
PACKAGE_LIST=""
for PACKAGE in gpac php5 php5-common php5-fpm nginx libav-tools bc sshpass mpack
do
	if ! dpkg-query -l $PACKAGE &> /dev/null
	then
		PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
	fi
done

if [ "$PACKAGE_LIST" != "" ]
then
	echo "Installing packages: $PACKAGE_LIST"
	echo "Running: apt-get update"
	sudo apt-get update
	echo "Installing packages: $PACKAGE_LIST"
	sudo apt-get install -y $PACKAGE_LIST
else
	echo "No packages need to be installed."
fi


IS_WHEEZY=`echo "$DEB_VERSION <= $WHEEZY" | bc`

if [ $IS_WHEEZY -gt 0 ]
then
	if ! dpkg-query -l realpath &> /dev/null
	then
		echo "Installing package: realpath"
		sudo apt-get install -y realpath
	fi
fi


if [ ! -h /usr/local/bin/pikrellcam ]
then
    echo "Making /usr/local/bin/pikrellcam link."
	sudo rm -f /usr/local/bin/pikrellcam
    sudo ln -s $PWD/pikrellcam /usr/local/bin/pikrellcam
else
    CURRENT_BIN=`realpath /usr/local/bin/pikrellcam`
    if [ "$CURRENT_BIN" != "$PWD/pikrellcam" ]
    then
    echo "Replacing /usr/local/bin/pikrellcam link"
        sudo rm /usr/local/bin/pikrellcam
        sudo ln -s $PWD/pikrellcam /usr/local/bin/pikrellcam
    fi
fi


# =============== create initial ~/.pikrellcam configs ===============
#
./pikrellcam -quit


# =============== set install_dir in pikrellcam.conf ===============
#
PIKRELLCAM_CONF=$HOME/.pikrellcam/pikrellcam.conf
if [ ! -f $PIKRELLCAM_CONF ]
then
	echo "Unexpected failure to create config file $HOME/.pikrellcam/pikrellcam.conf"
	exit 1
fi

if ! grep -q "install_dir $PWD" $PIKRELLCAM_CONF
then
	echo "Setting install_dir config line in $PIKRELLCAM_CONF:"
	echo "install_dir $PWD"
	sed -i  "/install_dir/c\install_dir $PWD" $PIKRELLCAM_CONF
fi


# =============== pikrellcam autostart to rc.local  ===============
#
#CMD="su pi -c '(sleep 5; \/home\/pi\/pikrellcam\/pikrellcam)  \&'"
CMD="su pi -c '(sleep 5; $PWD/pikrellcam) \&'"

if [ "$AUTOSTART" == "yes" ]
then
    if ! fgrep -q "$CMD" /etc/rc.local
    then
		if grep -q pikrellcam /etc/rc.local
		then
			sudo sed -i "/pikrellcam/d" /etc/rc.local
		fi
		echo "Adding a pikrellcam autostart command to /etc/rc.local:"
        sudo sed -i "s|^exit.*|$CMD\n&|" /etc/rc.local
		grep pikrellcam /etc/rc.local
    fi
else
	if grep -q pikrellcam /etc/rc.local
	then
		echo "Removing pikrellcam autostart line from /etc/rc.local."
		sudo sed -i "/pikrellcam/d" /etc/rc.local
	fi
fi


# ===== sudoers permission for www-data to run pikrellcam as pi ======
#
CMD=$PWD/pikrellcam
if ! grep -q "$cmd" /etc/sudoers.d/pikrellcam 2>/dev/null
then
	echo "Adding to /etc/sudoers.d: www-data permission to run pikrellcam as user pi:"
	cp etc/pikrellcam.sudoers /tmp/pikrellcam.sudoers.tmp
	sed -i "s|pikrellcam|$CMD|" /tmp/pikrellcam.sudoers.tmp
	sudo chown root.root /tmp/pikrellcam.sudoers.tmp
	sudo chmod 440 /tmp/pikrellcam.sudoers.tmp
	sudo mv /tmp/pikrellcam.sudoers.tmp /etc/sudoers.d/pikrellcam
#	sudo cat /etc/sudoers.d/pikrellcam
fi


# =============== nginx install ===============
#
# Logging can eat many tens of megabytes of SD card space per day
# with the mjpeg.jpg streaming
#
if ! grep -q "access_log off" /etc/nginx/nginx.conf
then
	echo "Turning off nginx access_log."
	sudo sed -i  '/access_log/c\	access_log off;' /etc/nginx/nginx.conf
	NGINX_RESTART="yes"
fi

WHEEZY=7.8
DEB_VERSION=`cat /etc/debian_version`

IS_JESSIE=`echo "$DEB_VERSION > $WHEEZY" | bc`

if [ $IS_JESSIE -gt 0 ]
then
	NGINX_SITE=etc/nginx-jessie-site-default
else
	NGINX_SITE=etc/nginx-wheezy-site-default
fi

if     ! grep -q $PORT /etc/nginx/sites-available/pikrellcam 2>/dev/null \
	|| ! grep -q "root $PWD/www" /etc/nginx/sites-available/pikrellcam 2>/dev/null
then
	echo "Installing /etc/nginx/sites-available/pikrellcam"
	echo "    nginx web server port: $PORT"
	echo "    nginx web server root: $PWD/www"
	sudo cp $NGINX_SITE /etc/nginx/sites-available/pikrellcam
	sudo sed -i "s|PIKRELLCAM_WWW|$PWD/www|; \
				s/PORT/$PORT/" \
			/etc/nginx/sites-available/pikrellcam
	NGINX_RESTART="yes"
fi

NGINX_SITE=/etc/nginx/sites-available/pikrellcam

if [ "$PORT" == "80" ]
then
	NGINX_LINK=/etc/nginx/sites-enabled/default
	CURRENT_SITE=`realpath $NGINX_LINK`
	if [ "$CURRENT_SITE" != "$NGINX_SITE" ]
	then
		echo "Changing $NGINX_LINK link to pikrellcam"
		sudo rm -f $NGINX_LINK
		sudo ln -s $NGINX_SITE $NGINX_LINK
		NGINX_RESTART="yes"
	fi
else
	NGINX_LINK=/etc/nginx/sites-enabled/pikrellcam
fi

if [ ! -h $NGINX_LINK 2>/dev/null ]
then
	echo "Adding $NGINX_LINK link to sites-available/pikrellcam."
	sudo ln -s $NGINX_SITE $NGINX_LINK
	NGINX_RESTART="yes"
fi

if [ "$NGINX_RESTART" == "yes" ]
then
	echo "Restarting nginx"
	sudo service nginx restart
fi


# =============== Setup FIFO  ===============
#
fifo=$PWD/www/FIFO

if [ ! -p "$fifo" ]
then
	rm -f $fifo
	mkfifo $fifo
fi
sudo chown pi.www-data $fifo
sudo chmod 664 $fifo

# =============== Setup Password  ===============
#
SESSION_PATH=$PWD/www/session

if [ ! -f www/password.php ]
then
	WRITE_PASSWORD=yes
else
	if    ! grep -q $PASSWORD www/password.php 2>/dev/null \
       || ! grep -q $SESSION_PATH www/password.php 2>/dev/null
	then
		WRITE_PASSWORD=yes
	fi
fi


if [ "$WRITE_PASSWORD" == "yes" ]
then
	if [ $$PASSWORD == "unset" ]
	then
		echo "Setting web page password to not require a password."
	else
		echo "Setting web page password file with password: $PASSWORD"
	fi
	cat << EOF > www/password.php
<?php
define("PASSWORD", "$PASSWORD");
define("SESSION_PATH", "$SESSION_PATH");
?>
EOF
fi

if [ ! -d $SESSION_PATH ]
then
	mkdir $SESSION_PATH
fi
sudo chown pi.www-data $SESSION_PATH
sudo chmod 775 $SESSION_PATH


# =============== copy scripts-dist into scripts  ===============
#
if [ ! -d scripts ]
then
	mkdir scripts
fi

cd scripts-dist

for script in *
do
	if [ ! -f ../scripts/$script ] && [ "$script" != "init" ]
	then
		cp $script ../scripts 
	fi
done

