AC_DEFUN([VS_CHECK_NETCOMM], 
[
dnl --------------------------------------------------------------------------
dnl Check for network communication system
dnl --------------------------------------------------------------------------

TESTNETCOMM="0"
TESTNETCOMM_NOSOUND="1"
TESTNETCOMM_WEBCAM="0"
TESTNETCOMM_JVOIP="0"
TESTNETCOMM_PORTAUDIO="0"

	dnl NOTE : CLIENT WITH JVOIP SUPPORT WILL ONLY HEAR OTHER JVOIP CLIENTS
	dnl        AND IT GOES THE SAME FOR PORTAUDIO
	AC_ARG_ENABLE(	netcomm,
				AC_HELP_STRING([--enable-netcomm=TYPE],
				[Enable hazardous network communications (TYPE=nosound (default), jvoip, portaudio)]),
				[
    				AC_DEFINE(NETCOMM,1,[enable hazardous network communications (Webcam and VoIP)])
					case "$enableval" in
						"n" | "none" | "nosound")
							TESTNETCOMM="1"
					;;
						"j" | "jvoip")
							TESTNETCOMM_NOSOUND="0"
							TESTNETCOMM_JVOIP="1"
							TESTNETCOMM="1"
							echo "Trying to enable JVoIP support... "
					;;
						"pa" | "portaudio")
							TESTNETCOMM_NOSOUND="0"
							TESTNETCOMM_PORTAUDIO="1"
							TESTNETCOMM="1"
							echo "Trying to enable PortAudio support... "
					;;
					esac
				],
				[])

if test "$TESTNETCOMM" = "1" ; then
	dnl check for oss headers under Linux

	if test "$TESTNETCOMM_NOSOUND" = "0"; then
		AC_DEFUN([CHECK_FOR_OSS_H], [
		    AC_MSG_CHECKING([for sys/soundcard.h])
		    AC_TRY_CPP( [ #include <sys/soundcard.h> ], have_oss_h=yes, have_oss_h=no )
		    AC_MSG_RESULT([$have_oss_h])
		])

		AC_DEFUN([CHECK_FOR_JVOIP_H], [
		    AC_MSG_CHECKING([for jvoiplib/jvoipsession.h])
		    AC_TRY_CPP( [ #include <jvoiplib/jvoipsession.h> ], have_jvoip_h=yes, have_jvoip_h=no )
		    AC_MSG_RESULT([$have_jvoip_h])
		])

		AC_DEFUN([CHECK_FOR_JTHREAD_H], [
		    AC_MSG_CHECKING([for jthread/jthread.h])
		    AC_TRY_CPP( [ #include <jthread/jthread.h> ], have_jthread_h=yes, have_jthread_h=no )
		    AC_MSG_RESULT([$have_jthread_h])
		])

		AC_DEFUN([CHECK_FOR_JRTP]_H, [
		    AC_MSG_CHECKING([for jrtp/rtpsession.h])
		    AC_TRY_CPP( [ #include <jrtp/rtpsession.h> ], have_jrtp_h=yes, have_jrtp_h=no )
		    AC_MSG_RESULT([$have_jrtp_h])
		])

		AC_DEFUN([CHECK_FOR_COREAUDIO_H], [
		    AC_MSG_CHECKING([for CoreAudio/CoreAudio.h])
		    AC_TRY_CPP( [ #include <CoreAudio/CoreAudio.h> ], have_core_audio_h=yes, have_core_audio_h=no )
		    AC_MSG_RESULT([$have_core_audio_h])
		])

		if test "$LINUX" = "1" ; then
			CHECK_FOR_OSS_H
			if test "x$have_oss_h" = "xno" ; then
				TESTNETCOMM_JVOIP="0"
				TESTNETCOMM_PORTAUDIO="0"
			fi
		fi

		if test "x$is_macosx" = "xyes" ; then
			if test "$TESTNETCOMM_JVOIP" = "1" ; then
				CHECK_FOR_COREAUDIO_H
				TESTNETCOMM_JVOIP="0"
				echo "JVoIP lib does not support MacOS X yet"
			else
				if test "x$have_core_audio_h" = "xno" ; then
					TESTNETCOMM_PORTAUDIO="0"
				fi
			fi
		fi

		saved_CPPFLAGS="$CPPFLAGS"
		dnl Add the CFLAGS that correspond to sound choice
		if test "$TESTNETCOMM_JVOIP" = "1" ; then
		dnl THIS DOESN'T WORK BECAUSE JVOIP HEADERS INCLUDE C++ IOSTREAM AND STRING HEADERS
		dnl	JLIBS_FLAGS="-I/usr/include -I/usr/local/include -I/usr/local/include/jrtplib -I/usr/local/include/jthread -I/usr/local/include/jvoiplib"
		dnl	CPPFLAGS="$saved_CPPFLAGS $JLIBS_FLAGS"
		dnl	CHECK_FOR_JVOIP_H
		dnl	if test "x$have_jvoip_h" = "xno" ; then
		dnl		TESTNETCOMM_JVOIP=0
		dnl		echo "JVoIP lib headers not found : disabling JVoIP support"
		dnl	else
		dnl		CHECK_FOR_JTHREAD_H
		dnl		if test "x$have_jthread_h" = "xno" ; then
		dnl			TESTNETCOMM_JVOIP=0
		dnl			echo "JThread lib headers not found : disabling JVoIP support"
		dnl		else
		dnl			CHECK_FOR_JRTP_H
		dnl			if test "x$have_jrtp_h" = "xno" ; then
		dnl				TESTNETCOMM_JVOIP=0
		dnl				echo "JRtp lib headers not found : disabling JVoIP support"
		dnl			else
					  	VS_CPPFLAGS="$VS_CPPFLAGS -DNETCOMM_JVOIP"
						VS_LIBS="$VS_LIBS -L/usr/local/lib -ljvoip -ljrtp -ljthread"
					   	AC_DEFINE(NETCOMM_JVOIP,1,[enable JVoIP library for sound support with NETCOMM])
						echo "VoIP support enabled."
		dnl			fi
		dnl		fi
		dnl	fi
		else
			if test "$TESTNETCOMM_PORTAUDIO" = "1" ; then
				VS_LIBS="$VS_LIBS -L/usr/local/lib -lportaudio"
	   			AC_DEFINE(NETCOMM_PORTAUDIO,1,[enable PortAudio library for sound support with NETCOMM])
				echo "PortAudio support enabled."
			else
				TESTNETCOMM_PORTAUDIO="0"
				echo "PortAudio support disabled."
			fi
		fi
		CPPFLAGS="$saved_CPPFLAGS"
	else
		echo "No sound support for networking communications"
		TESTNETCOMM_JVOIP="0"
		TESTNETCOMM_PORTAUDIO="0"
	fi

	dnl Check for video for linux headers under linux
	AC_DEFUN([CHECK_FOR_VFL_H], [
	    AC_MSG_CHECKING([for linux/videodev.h])
	    AC_TRY_CPP( [ #include <linux/videodev.h> ], have_vfl_h=yes, have_vfl_h=no )
	    AC_MSG_RESULT([$have_vfl_h])
	])

	echo "Trying to enable webcam support... "

	dnl Test to check VFL headers
	if test "$LINUX" = "1" ; then
		CHECK_FOR_VFL_H
		if test "x$have_vfl_h" = "xyes" ; then
			echo "Webcam support enabled."
			TESTNETCOMM_WEBCAM="1"
    		AC_DEFINE(NETCOMM_WEBCAM,1,[enable video with NETCOMM])
		else
			echo "*** Webcam support disabled."
		fi
	fi

	dnl Check for QuickTime headers under MacOS X
	dnl saved_CPPFLAGS="$CPPFLAGS"
	AC_DEFUN([CHECK_FOR_QT_H], [
	    AC_MSG_CHECKING([for QuickTimeComponents.h])
	    AC_TRY_CPP( [ #include <QuickTimeComponents.h> ], have_qt_h=yes, have_qt_h=no )
	    AC_MSG_RESULT([$have_qt_h])
	])

	if test "$is_macosx" = "yes" ; then
		CPPFLAGS="$CPPFLAGS -I/System/Library/Frameworks/QuickTime.framework/Versions/A/Headers"
		CHECK_FOR_QT_H
		if test "x$have_qt_h" = "xyes" ; then
			TESTNETCOMM_WEBCAM="1"
    		AC_DEFINE(NETCOMM_WEBCAM,1,[enable video with NETCOMM])
   			LIBS="$LIBS -framework QuickTime -framework Carbon"
			echo "Webcam support enabled."
		else
			echo "*** Webcam support disabled."
		fi

	fi
fi

AM_CONDITIONAL(NETCOMM, test "$TESTNETCOMM" = "1")
AM_CONDITIONAL(NETCOMM_NOSOUND, test "$TESTNETCOMM_NOSOUND" = "1")
AM_CONDITIONAL(NETCOMM_NOWEBCAM, test "$TESTNETCOMM_NOWEBCAM" = "1")
AM_CONDITIONAL(NETCOMM_JVOIP, test "$TESTNETCOMM_JVOIP" = "1")
AM_CONDITIONAL(NETCOMM_PORTAUDIO, test "$TESTNETCOMM_PORTAUDIO" = "1")
])
