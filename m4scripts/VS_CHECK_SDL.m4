AC_DEFUN([VS_CHECK_SDL], 
[
#--------------------------------------------------------------------------
# Check for SDL
#--------------------------------------------------------------------------
AC_ARG_ENABLE(sdl, AC_HELP_STRING([--disable-sdl], [Use GLUT instead of SDL]), _sdl=$enableval)
if test "x$_sdl" != "xno" ; then

SDL_VERSION=1.0.1
AM_PATH_SDL($SDL_VERSION, have_SDL=yes, have_SDL=no)

if test "x$have_SDL" = "xyes" ; then
    VS_CPPFLAGS="${VS_CPPFLAGS} -DHAVE_SDL=1"
    VS_CXXFLAGS="${VS_CXXFLAGS} ${SDL_CFLAGS}"
    VS_LIBS="${VS_LIBS} ${SDL_LIBS}"

    dnl Check for joystick support in SDL
    saved_LIBS="$LIBS"
    LIBS="$saved_LIBS $VS_LIBS"

    AC_CHECK_FUNCS( SDL_JoystickOpen, 
                    have_SDL_joystick=yes, have_SDL_joystick=no )

    if test "x$have_SDL_joystick" = "xno" ; then
        echo "*** This version of SDL doesn't have joystick support."
        echo "*** Configuring without joystick support."
    fi

AC_ARG_ENABLE(sdl-windowing, AC_HELP_STRING([--disable-sdl-windowing], [Disable SDL's window management options]), sdl_windowing=$enableval)
	if test "x$sdl_windowing" != "xno" ; then
		VS_CPPFLAGS="${VS_CPPFLAGS} -DSDL_WINDOWING=1" 
		echo "Using SDL Windowing System"
	else
		echo "Using GLUT Windowing System"
	fi
dnl Check for SDL_mixer
    AC_CHECK_LIB( SDL_mixer, Mix_OpenAudio, 
                  have_SDL_mixer=yes, have_SDL_mixer=no )

    if test "x$have_SDL_mixer" = "xyes" ; then 
        AC_DEFINE(HAVE_SDL_MIXER,1,[Can we use the SDL mixer])
	SDL_MIXER_LIBS="-lSDL_mixer"
    else
        echo "*** SDL_mixer not found.  Configuring without audio support."
    fi
    AC_SUBST(SDL_MIXER_LIBS)

    LIBS="$saved_LIBS"
else
    echo "*** SDL not found.  Configuring without audio or joystick support."
fi
else
    echo "*** SDL disabled. Configuring without audio or joystick support."
fi
])
