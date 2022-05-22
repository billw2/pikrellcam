#!/bin/bash

PGM=$(basename "$0")

if [ "$(id -u)" == 0 ]
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

if [ ! -x "$PWD"/pikrellcam ]
then
	bad_install "program pikrellcam"
fi

if [ ! -d "$PWD"/www ]
then
	bad_install "directory www"
fi

if [ -f "/etc/arch-release" ]; then
	DISTRO="ARCH"
	WWW_USER=http
	WWW_GROUP=http
else
	DISTRO="DEBIAN"
	WWW_USER=www-data
	WWW_GROUP=www-data
	V=$(cat /etc/debian_version)
	#DEB_VERSION="${V:0:1}"
	# Strip all chars after decimal point
	DEB_VERSION="${V%.*}"
	JESSIE=8
	STRETCH=9
	BUSTER=10
	BULLSEYE=11
fi

sudo chown .$WWW_GROUP "$PWD"/www
sudo chmod 775 "$PWD"/www

if [ ! -d media ]
then
	mkdir media media/archive media/videos media/thumbs media/stills
	sudo chown .$WWW_GROUP media media/archive media/videos media/thumbs media/stills
	sudo chmod 775 media media/archive media/videos media/thumbs media/stills
fi

if [ ! -h www/media ]
then
	ln -s "$PWD"/media www/media
fi

if [ ! -h www/archive ]
then
	ln -s "$PWD"/media/archive www/archive
fi

echo ""
echo "Set the port for the nginx web server."
echo "If you already have a web server configuration using the default"
echo "port 80, you should enter an alternate port for PiKrellCam."
echo "Otherwise you can use the default port 80 or an alternate as you wish."
echo "The port number will be set in: /etc/nginx.sites-available/pikrellcam."
echo -n "Enter web server port: "
read -r resp
if [ "$resp" == "" ]
then
	PORT=80
else
	PORT=$resp
fi

echo ""
echo "For auto starting at boot, a PiKrellCam start command must be in rc.local or systemd service."
echo "If you don't start at boot, PiKrellCam can always be started and stopped"
echo "from the web page."
echo -n "Do you want PiKrellCam to be auto started at boot? (yes/no): "
read -r resp
if [ "$resp" == "y" ] || [ "$resp" == "yes" ]
then
	AUTOSTART=yes
else
	AUTOSTART=no
fi


HTPASSWD=www/.htpasswd
PASSWORD=""

echo ""
if [ -f $HTPASSWD ]
then
	echo "A web password is already set."
	echo -n "Do you want to change the password (yes/no)? "
	read -r resp
	if [ "$resp" == "y" ] || [ "$resp" == "yes" ]
	then
		SET_PASSWORD=yes
		rm -f $HTPASSWD
	else
		SET_PASSWORD=no
	fi
else
	SET_PASSWORD=yes
fi

if [ "$SET_PASSWORD" == "yes" ]
then
	echo "Enter a password for a web page login for user: $USER"
	echo "Enter a blank entry if you do not want the password login."
	echo -n "Enter password: "
	read -r PASSWORD
fi




echo ""
echo "Starting PiKrellCam install..."

# =============== apt install needed packages ===============
#

if [ "$DISTRO" == "DEBIAN" ]
then
	PACKAGE_LIST=""

	if ((DEB_VERSION >= BUSTER))
	then
		AV_PACKAGES="ffmpeg"
		PHP_PACKAGES="php8.0 php8.0-common php8.0-fpm"
	elif ((DEB_VERSION >= BUSTER))
	then
		AV_PACKAGES="ffmpeg"
		PHP_PACKAGES="php7.3 php7.3-common php7.3-fpm"
	elif ((DEB_VERSION >= STRETCH))
	then
		AV_PACKAGES="libav-tools"
		PHP_PACKAGES="php7.0 php7.0-common php7.0-fpm"
	else
		AV_PACKAGES="libav-tools"
		PHP_PACKAGES="php5 php5-common php5-fpm"
	fi

	for PACKAGE in $PHP_PACKAGES $AV_PACKAGES
	do
		if ! dpkg -s "$PACKAGE" 2>/dev/null | grep Status | grep -q installed
		then
			PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
		fi
	done

	for PACKAGE in gpac nginx bc \
		sshpass mpack imagemagick libasound2 libasound2-dev \
		libmp3lame0 libmp3lame-dev openssl
	do
		if ! dpkg -s $PACKAGE 2>/dev/null | grep Status | grep -q installed
		then
			PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
		fi
	done

	if [ "$PACKAGE_LIST" != "" ]
	then
		echo "Installing packages: $PACKAGE_LIST"
		echo "Running: apt-get update"
		sudo apt-get update
		sudo apt-get install -y --no-install-recommends "$PACKAGE_LIST"
	else
		echo "No packages need to be installed."
	fi


	if ((DEB_VERSION < JESSIE))
	then
		if ! dpkg -s realpath 2>/dev/null | grep Status | grep -q installed
		then
			echo "Installing package: realpath"
			sudo apt-get install -y --no-install-recommends realpath
		fi
	fi
	
elif [ "$DISTRO" == "ARCH" ]
then
	PACKAGE_LIST=""
	AV_PACKAGES="ffmpeg"
	PHP_PACKAGES="php php-fpm"
	
	for PACKAGE in $PHP_PACKAGES $AV_PACKAGES
	do
		if ! pacman -Q 2>/dev/null | grep -q "$PACKAGE"
		then
			PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
		fi
	done
	
	for PACKAGE in sudo gpac nginx-mainline bc lame \
		sshpass libmpack imagemagick alsa-lib openssl
	do
		if ! pacman -Q 2>/dev/null | grep -q $PACKAGE
		then
			PACKAGE_LIST="$PACKAGE_LIST $PACKAGE"
		fi
	done
	
	if [ "$PACKAGE_LIST" != "" ]
	then
		echo "Installing packages: $PACKAGE_LIST"
		echo "Running: pacman"
		sudo pacman -Sy --noconfirm
		sudo pacman -S pacman --needed --noconfirm
		sudo pacman-db-upgrade
		sudo pacman -S --noconfirm --needed "$PACKAGE_LIST"
	else
		echo "No packages need to be installed."
	fi

fi

if [ ! -h /usr/local/bin/pikrellcam ]
then
    echo "Making /usr/local/bin/pikrellcam link."
	sudo rm -f /usr/local/bin/pikrellcam
    sudo ln -s "$PWD"/pikrellcam /usr/local/bin/pikrellcam
else
    CURRENT_BIN=$(realpath /usr/local/bin/pikrellcam)
    if [ "$CURRENT_BIN" != "$PWD/pikrellcam" ]
    then
    echo "Replacing /usr/local/bin/pikrellcam link"
        sudo rm /usr/local/bin/pikrellcam
        sudo ln -s "$PWD"/pikrellcam /usr/local/bin/pikrellcam
    fi
fi


# =============== create initial ~/.pikrellcam configs ===============
#
./pikrellcam -quit

if [ "$USER" == "pi" ]
then
	rm -f www/user.php
else
	printf "<?php
    \$e_user = \"$USER\";
?>
" > www/user.php
fi

if [ "$DISTRO" == "ARCH" ]
then
	setfacl -m u:http:rwx "$HOME"
fi

# =============== set install_dir in pikrellcam.conf ===============
#
PIKRELLCAM_CONF=$HOME/.pikrellcam/pikrellcam.conf
if [ ! -f "$PIKRELLCAM_CONF" ]
then
	echo "Unexpected failure to create config file $HOME/.pikrellcam/pikrellcam.conf"
	exit 1
fi

# shellcheck disable=SC2086
if ! grep -q "install_dir $PWD" $PIKRELLCAM_CONF
then
	echo "Setting install_dir config line in $PIKRELLCAM_CONF:"
	echo "install_dir $PWD"
	sed -i  "/install_dir/c\install_dir $PWD" $PIKRELLCAM_CONF
fi

sed -i  "s/NGINX_GROUP/$WWW_GROUP/" "$PIKRELLCAM_CONF"

# =============== pikrellcam autostart to rc.local  ===============
#
#CMD="su $USER -c '(sleep 5; \/home\/pi\/pikrellcam\/pikrellcam)  \&'"
CMD="su $USER -c '(sleep 5; $PWD/pikrellcam) \&'"

if [ "$DISTRO" == "DEBIAN" ]
then
	if [ "$AUTOSTART" == "yes" ]
	then
		if ((DEB_VERSION >= BULLSEYE))
		then
			if [ ! -f /etc/systemd/user/pikrellcam.service ]
			then
				cp etc/pikrellcam.service /tmp/pikrellcam.service.tmp
				sed -i "s/USER/$USER/" /tmp/pikrellcam.service.tmp
				sed -i "s|PWD|$PWD|" /tmp/pikrellcam.service.tmp
				sudo cp /tmp/pikrellcam.service.tmp /etc/systemd/system/pikrellcam.service
			fi
			sudo systemctl enable pikrellcam
		else
		    if ! grep -F -q "$CMD" /etc/rc.local
		    then
				if grep -q pikrellcam /etc/rc.local
				then
					sudo sed -i "/pikrellcam/d" /etc/rc.local
				fi
				echo "Adding a pikrellcam autostart command to /etc/rc.local:"
			sudo sed -i "s|^exit.*|$CMD\n&|" /etc/rc.local
				if ! [ -x /etc/rc.local ]
				then
					echo "Added execute permission to /etc/rc.local"
					sudo chmod a+x /etc/rc.local
				fi
				grep pikrellcam /etc/rc.local
		    fi
		fi
	else
		if ((DEB_VERSION >= BULLSEYE))
		then
			sudo systemctl disable pikrellcam
		else
			if grep -q pikrellcam /etc/rc.local
			then
				echo "Removing pikrellcam autostart line from /etc/rc.local."
				sudo sed -i "/pikrellcam/d" /etc/rc.local
			fi
		fi
	fi
elif [ "$DISTRO" == "ARCH" ]
then
	if [ ! -f /etc/systemd/user/pikrellcam.service ]
	then
		cp etc/pikrellcam.service /tmp/pikrellcam.service.tmp
		sed -i "s/USER/$USER/" /tmp/pikrellcam.service.tmp
		sed -i "s|PWD|$PWD|" /tmp/pikrellcam.service.tmp
		sudo cp /tmp/pikrellcam.service.tmp /etc/systemd/system/pikrellcam.service
	fi
		
	if [ "$AUTOSTART" == "yes" ]
	then
		sudo systemctl enable pikrellcam
	else
		sudo systemctl disable pikrellcam
	fi
fi


# ===== sudoers permission for $WWW_USER to run pikrellcam as pi ======
#
CMD=$PWD/pikrellcam
if ! grep -q "$CMD" /etc/sudoers.d/pikrellcam 2>/dev/null
then
	echo "Adding to /etc/sudoers.d: $WWW_USER permission to run pikrellcam as user pi:"
	cp etc/pikrellcam.sudoers /tmp/pikrellcam.sudoers.tmp
	sed -i "s|pikrellcam|$CMD|g" /tmp/pikrellcam.sudoers.tmp
	sed -i "s/NGINX_USER/$WWW_USER/g" /tmp/pikrellcam.sudoers.tmp
	if [ "$DISTRO" == "ARCH" ]
	then
		sed -i "s/#USER/$USER/g" /tmp/pikrellcam.sudoers.tmp
	fi
	sed -i "s/USER/$USER/g" /tmp/pikrellcam.sudoers.tmp
	sudo chown root.root /tmp/pikrellcam.sudoers.tmp
	sudo chmod 440 /tmp/pikrellcam.sudoers.tmp
	sudo mv /tmp/pikrellcam.sudoers.tmp /etc/sudoers.d/pikrellcam
#	sudo cat /etc/sudoers.d/pikrellcam
fi

# =============== Setup Password  ===============
#
OLD_SESSION_PATH=www/session
if [ -d $OLD_SESSION_PATH ]
then
	sudo rm -rf $OLD_SESSION_PATH
fi

OLD_PASSWORD=www/password.php
if [ -f $OLD_PASSWORD ]
then
	rm -f $OLD_PASSWORD
fi

if [ "$PASSWORD" != "" ]
then
	printf "$USER:$(openssl passwd -6 "$PASSWORD")\n" > $HTPASSWD
	sudo chown "$USER".$WWW_GROUP $HTPASSWD
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
fi

if [ "$DISTRO" == "DEBIAN" ]
then
	if ((DEB_VERSION < JESSIE))
	then
		NGINX_SITE=etc/nginx-wheezy-site-default
	elif ((DEB_VERSION >= BULLSEYE))
	then
		NGINX_SITE=etc/nginx-arch-site-default
	else
		NGINX_SITE=etc/nginx-jessie-site-default
	fi
elif [ "$DISTRO" == "ARCH" ]
then
	if [ ! -d "/etc/nginx/sites-enabled" ]; then
		sudo mkdir /etc/nginx/sites-available
		sudo mkdir /etc/nginx/sites-enabled
		if ! grep -q "sites-enabled" /etc/nginx/nginx.conf 2>/dev/null
		then
			sudo sed -i '/include.*mime.types;/s|$|\n    include       sites-enabled/*;|' /etc/nginx/nginx.conf
		fi
	fi
	NGINX_SITE=etc/nginx-arch-site-default
fi

echo "Installing /etc/nginx/sites-available/pikrellcam"
echo "    nginx web server port: $PORT"
echo "    nginx web server root: $PWD/www"
sudo cp $NGINX_SITE /etc/nginx/sites-available/pikrellcam
sudo sed -i "s|PIKRELLCAM_WWW|$PWD/www|; \
			s/PORT/$PORT/" \
			/etc/nginx/sites-available/pikrellcam

if [ "$DISTRO" == "DEBIAN" ]
	then
	if ((DEB_VERSION >= BULLSEYE))
	then
		sudo sed -i "s/php5/php\/php8.0/" /etc/nginx/sites-available/pikrellcam
	elif ((DEB_VERSION >= BUSTER))
	then
		sudo sed -i "s/php5/php\/php7.3/" /etc/nginx/sites-available/pikrellcam
	elif ((DEB_VERSION >= STRETCH))
	then
		sudo sed -i "s/php5/php\/php7.0/" /etc/nginx/sites-available/pikrellcam
	fi
elif [ "$DISTRO" == "ARCH" ]
then
	sudo sed -i "s/php5/php\/php8.0/" /etc/nginx/sites-available/pikrellcam
	sudo systemctl enable --now php-fpm
fi

NGINX_SITE=/etc/nginx/sites-available/pikrellcam

if [ "$PORT" == "80" ]
then
	NGINX_LINK=/etc/nginx/sites-enabled/default
	CURRENT_SITE=$(realpath $NGINX_LINK)
	if [ "$CURRENT_SITE" != "$NGINX_SITE" ]
	then
		echo "Changing $NGINX_LINK link to pikrellcam"
		sudo rm -f $NGINX_LINK
		sudo ln -s $NGINX_SITE $NGINX_LINK
	fi
else
	NGINX_LINK=/etc/nginx/sites-enabled/pikrellcam
fi

#if [ ! -h $NGINX_LINK 2>/dev/null ]
#then
#	echo "Adding $NGINX_LINK link to sites-available/pikrellcam."
#	sudo ln -s $NGINX_SITE $NGINX_LINK
#fi

if [ ! -f $HTPASSWD ]
then
	echo "A password for the web page is not set."
	sudo sed -i 's/auth_basic/\# auth_basic/' $NGINX_SITE
fi

if [ "$DISTRO" == "DEBIAN" ]
then
	sudo service nginx restart
elif [ "$DISTRO" == "ARCH" ]
then
	sudo systemctl enable nginx
	sudo systemctl restart nginx
fi


# =============== Setup FIFO  ===============
#
fifo=$PWD/www/FIFO

if [ ! -p "$fifo" ]
then
	rm -f "$fifo"
	mkfifo "$fifo"
fi
# shellcheck disable=SC2086
sudo chown $USER.$WWW_GROUP $fifo
sudo chmod 664 "$fifo"



# =============== copy scripts-dist into scripts  ===============
#
if [ ! -d scripts ]
then
	mkdir scripts
fi

cd scripts-dist || exit

for script in *
do
	if [ ! -f ../scripts/"$script" ] && [ "${script:0:1}" != "_" ]
	then
		cp "$script" ../scripts
	fi
done

echo ""
echo "Install finished."
echo "This install script does not automatically start pikrellcam."
echo "To start pikrellcam, open a browser page to:"
if [ "$PORT" == "80" ]
then
	echo "    http://your_pi"
else
	echo "    http://your_pi:$PORT"
fi
echo "and click on the \"System\" panel and then the \"Start PiKrellCam\" button."
echo "PiKrellCam can also be run from a Pi terminal for testing purposes."
if [ "$AUTOSTART" == "yes" ]
then
	echo "Automatic pikrellcam starting at boot is enabled."
fi
echo ""
