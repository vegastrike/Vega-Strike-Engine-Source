#!/bin/bash

until [ "x" = "x$1" ]; do
	if [ "x-p" = "x$1" -o "x--prefix" = "x$1" ]; then
		shift
		INSTALL_PATH="$1"
	elif [ "x$1" = "x-32" -o "x$1" = "x-64" ]; then
		BINARY_APPEND="$1"
	else
		echo "Usage: $0 [--prefix PATH] [-32|-64]"
		echo "    Installs Vega Strike binaries into PATH (default /usr/bin)"
		echo "    Specifying -32 or -64 will install binaries for that architecture."
		exit 
	fi
	shift
done

if [ "x" = "x$INSTALL_PATH" ]; then
	INSTALL_PATH=/usr/bin
fi

if [ \! -e Version.txt ]; then
	echo 'I could not find the Vega Strike package.'
	echo 'Make sure you are inside of the vegastrike folder when you run this.'
	read -p 'Press any key to exit.'
	exit 1
fi


SOURCE_PATH="$PWD"
cd "$SOURCE_PATH"

echo '                     ==== VEGA STRIKE 0.5.0 ===='
echo ''
echo ''
if [ "x" = "x$BINARY_APPEND" ]; then
	if uname -a | grep -q x86_64 ; then
		echo 'I am installing 64-bit binaries.'
		BINARY_APPEND=-64
	else
		echo 'I am installing 32-bit binaries.'
		BINARY_APPEND=-32
	fi
fi

BINARIES="vegastrike vegaserver vssetup mesher"

echo ''
echo 'This script needs your password to link the following commands '
echo " into '$INSTALL_PATH': $BINARIES"
echo ''
echo "Do you wish to grant administrator access to install these binaries?"
read -p "Type yes or no: " LOGIN

SUCCESS=no

if [ "x$LOGIN" = "xy" -o "x$LOGIN" = "xyes" ]; then
	DOSUDO=yes
	if [ "`whoami`" = "root" ]; then
		# Use no command for installing.
		mysudo=''
	else
		mysudo='sudo'
		echo "Trying to using sudo..."
		sudo -v || DOSUDO=no
	fi
	if [ "$DOSUDO" = "yes" ]; then
		SUCCESS=1
		for bin in $BINARIES; do
			src="${SOURCE_PATH}/bin/${bin}${BINARY_APPEND}"
			dest="${INSTALL_PATH}/${bin}"
			cmd="ln -f -s -v"
			if [ -e $src ]; then
				$mysudo $cmd "$src" "$dest" || SUCCESS=0
			fi
		done
	else
		echo ''
		echo "Trying to use su (type in the root password)..."
		tries=""
		SUCCESS=0
		until [ "$tries" = "a" ]; do #try once
			su -c 'bash -c '\'"`(for bin in $BINARIES; do
				src="${SOURCE_PATH}/bin/${bin}${BINARY_APPEND}"
				dest=${INSTALL_PATH}/${bin}
				cmd='ln -f -s -v'
				if [ -e $src ]; then
					echo -n $cmd $src $dest ';'
				fi
			done)`"\'
			if [ "$?" = "1" ]; then
				echo "Failed to login with su."
				tries="a${tries}"
			else
				SUCCESS=1
				break
			fi
		done
	fi
	if [ "$SUCCESS" = 1 ];then
		RUNSETUP=y
		echo "Installation successful!"
		echo ""
		read -p "Do you want to run 'vssetup'? [yes]/no? " RUNSETUP
		if [ "x$RUNSETUP" = "xn" -o "x$RUNSETUP" = "xno" ]; then
			true
		else
			vssetup
			echo "Settings should now be configured."

		fi
		echo ""
		read -p "Type 'vegastrike' at a terminal to play VS!  Press any key to exit." TMP
		exit 0
	else
		echo ""
		echo "Installation failed."
		read -p "Re-run this script to try again.  Press any key to exit." TMP
		exit 1
	fi
else
	VSSETUP="./bin/vssetup${BINARY_APPEND}"
	echo ""
	echo "I am not installing the executables at this time."
	echo "Binaries can be found in the 'bin' folder."
	echo ""
	read -p "Do you want to run '$VSSETUP' to configure the game? [yes]/no? " RUNSETUP
	if [ "x$RUNSETUP" = "xn" -o "x$RUNSETUP" = "xno" ]; then
		true
	else
		"$VSSETUP"
	fi
	echo ""
	echo "To install the executables, either rerun this script,"
	echo " or you can make aliases or symbolic links (not copies)"
	echo " of the executables in the 'bin' folder"
	echo ""
	read -p "Have fun playing VS.  Press any key to exit." TMP
	exit 0
fi

