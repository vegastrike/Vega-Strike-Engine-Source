AC_DEFUN([CHECK_FOR_AL_H], [
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$saved_CPPFLAGS $AL_CPPFLAGS"
    if test "x$is_macosx" = "xno" ; then
        CPPFLAGS="$VS_CPPFLAGS $AL_CPPFLAGS"
    fi

    AC_MSG_CHECKING([for AL/al.h])
    AC_TRY_CPP( [ #include <AL/al.h> ], have_al_h=yes, have_al_h=no )
    AC_MSG_RESULT([$have_al_h])
    AC_MSG_CHECKING([for al.h])
    AC_TRY_CPP( [ #include <al.h> ], have_al_h=yes, )
    AC_MSG_RESULT([$have_al_h])
    CPPFLAGS="$saved_CPPFLAGS" 
])

AC_DEFUN([VS_CHECK_OPENAL], 
[
#--------------------------------------------------------------------------
#Check for openal library
#--------------------------------------------------------------------------
AC_ARG_ENABLE(sound, AC_HELP_STRING([--disable-sound], [Disable allsound support]),sdl_sound=$enableval)

AC_ARG_WITH(openal-libs, AC_HELP_STRING([--with-openal-libs=DIR], [Location of openal library]))
if test "x$sdl_sound" != "xno" ; then
    if test "x$with_openal_libs" = "x" ; then
        OPENAL_LDOPTS=""
    else
        OPENAL_LDOPTS="-L$with_openal_libs"
    fi

    AC_DEFUN([CHECK_FOR_OPENAL_LIB], [
        AC_MSG_CHECKING([for $OPENAL_LIB_NAME library])
	LIBS="$MACORPCLIBS $OPENAL_LDOPTS"
        AC_TRY_LINK( , , have_openal_lib=yes, have_openal_lib=no )
        AC_MSG_RESULT([$have_openal_lib])
    ]) 

    saved_LIBS="$RRLIBS"
    
    OPENAL_LIB_LIST="openal"
    
    for OPENAL_LIB_NAME in $OPENAL_LIB_LIST ; do

    if test "x$is_macosx" = "xyes" ; then
      MACORPCLIBS="$OPENAL_LDOPTS -framework OpenAL"
      CHECK_FOR_OPENAL_LIB
    fi
    if test "x$have_openal_lib" = "xyes" ; then
      EPEE_DAY="september 30"
    else
      MACORPCLIBS="$OPENAL_LDOPTS -l$OPENAL_LIB_NAME"
      CHECK_FOR_OPENAL_LIB
    fi

    
        if test "x$have_openal_lib" = "xyes" ; then
            LIBS="$saved_LIBS"
    	    VS_LIBS="$VS_LIBS $MACORPCLIBS"
			if test "x${is_win32}" = "xyes" ; then
    	    VS_LIBS="$VS_LIBS -lwinmm"
			fi
            break;
        else
	    LIBS=""
	    saved_LIBS=""
        fi
    done

fi

#--------------------------------------------------------------------------
#Check for OpenAL headers
#--------------------------------------------------------------------------

AC_ARG_WITH(al-inc, AC_HELP_STRING([--with-al-inc=DIR], [OpenAL header file location]))

if test "x$with_al_inc" = "x" ; then
    AL_CPPFLAGS=""
else 
    AL_CPPFLAGS="-I$with_al_inc"
fi

dnl check for al.h
saved_CPPFLAGS="$CPPFLAGS"

CHECK_FOR_AL_H

if test "x$have_al_h" = "xno" -a "x$AL_CPPFLAGS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenAL headers installed in the standard"
    echo "*** location (/usr/include).  I'll check in /usr/local/include, since"
    echo "*** many distributions put OpenAL headers there."
    AL_CPPFLAGS="-I/usr/local/include"
    CHECK_FOR_AL_H
fi
if test "x$have_openal_lib" = "xno" ; then
    echo "Cannot find openal library"
    echo "Error::Configuring without OpenAL support!"
else
    if test "x$have_al_h" = "xno" ; then
        echo "Cannot find AL/al.h"
        echo "Error::Configuring without OpenAL support!"
    else
      dnl Check for alext.h 
      AC_CHECK_HEADERS(AL/alext.h, have_alext_h=yes, have_alext_h=no )

      if test "x$sdl_sound" != "xno" ; then
        echo "Configuring with OpenAL support!"
        VS_CPPFLAGS="${VS_CPPFLAGS} -DHAVE_AL=1" 
      else
        echo "Disabling OpenAL on user request"
      fi   
    fi
fi
VS_CPPFLAGS="${VS_CPPFLAGS} ${AL_CPPFLAGS}"
])
