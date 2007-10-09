AC_INIT(vegastrike, 0.4.4)
AC_CONFIG_SRCDIR(src/vegastrike.h)
AC_PREREQ(2.53)



AC_LANG(C++)
dnl AWK is used in configure
AC_PROG_AWK
dnl LN_S is used by the helper makefile in src
AC_PROG_LN_S
dnl We are a C++ program, so of course we use a c++ compiler and preprocessor
AC_PROG_CXX
AC_PROG_CXXCPP
dnl Install is used by make install
AC_PROG_INSTALL

dnl Lexx used to be used for a parser we use

dnl dnl pmx - 22/09/2006
dnl AC_CHECK_TOOL(BISON,bison)
dnl dnl AS the converter code in src/cmd/script/c_alike relies on bison, 
dnl dnl we MUST be sure to use flex in bison compatibility mode
dnl dnl (instad of the traditional 'lex')  pmx - 23/sept./2006
dnl AC_CHECK_TOOL(LEX, flex)
dnl AC_SUBST(LFLAGS,"$LFLAGS --yylineno --bison-bridge")
dnl AC_SUBST(FLIB,"-lfl")

dnl Old lex tool:
dnl AC_PROG_LEX
dnl AM_PROG_LEX
dnl Lex isn't used any more since the switch to Python.

dnl We use some static libraries to make building this program easier.
AC_PROG_RANLIB

dnl Most C++ programs also use C files.  We use some
dnl Libraries where the testing for them needs done in C
AC_PROG_CC
AC_PROG_CPP

AC_MSG_NOTICE([Using C++ compiler: ${CXX}.])
AC_MSG_NOTICE([Using C   compiler: ${CC}.])

dnl Check system type
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl Take the information from BUILD and use it to figure out what we are building for.
VS_CHECK_OS


dnl Initialize AutoMake and AutoHeader systems
AM_INIT_AUTOMAKE([1.6 subdir-objects dist-bzip2 ])
AM_CONFIG_HEADER(config.h)


dnl unfortunately most users (eg hellcatv) are too stupid to figure out where that environment variable is set (it's not in env or set or echo $CPPFLAGS gives nothing) so we have to force it to null
if test "x${CXXFLAGS:-notset}" = "xnotset" ; then
   export CXXFLAGS
   CXXFLAGS=""
fi

if test "x${CPPFLAGS:-notset}" = "xnotset" ; then
   export CPPFLAGS
   CPPFLAGS=""
fi


case $target in
  *Darwin* | *darwin*)
    is_macosx=yes
    bsdthread=no
    ;;
  *cygwin*)
    bsdthread=no
    ;;
  *mingw*)
    bsdthread=no
    ;;
  *)
    saved_CXXFLAGS="${CXXFLAGS}"
    CXXFLAGS="-Werror -pthread"
    AC_TRY_COMPILE(, , [bsdthread=yes], [bsdthread=no])
    CXXFLAGS="${saved_CXXFLAGS}"
    ;;
esac


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h fenv.h float.h inttypes.h limits.h malloc.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h values.h wchar.h wctype.h])


if test "x$slabcheck" = "xyes"; then
AC_CHECK_HEADERS([linux/slab.h linux/malloc.h])
else
AC_CHECK_HEADERS([malloc.h])
fi

AC_CHECK_FUNCS([vfscanf lstat])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])


# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRCOLL
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit bzero fesetround floor getcwd gethostbyname gettimeofday inet_ntoa iswprint localtime_r mblen memchr memmove memset mkdir munmap pow putenv rint rmdir select setenv socket sqrt strcasecmp strchr strdup strerror strstr strtol strtoul])


dnl Check for non-standard floating point functions and headers
AC_CHECK_FUNCS([finite _finite isnan _isnan])

dnl --------------------------------------------------------------------------
dnl Check if compiling for MacOSX
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING([for MacOSX platform])
AC_EGREP_CPP(yes,
[ 
#if defined(__APPLE__) || defined(MACOSX)
  yes
#endif
], [is_macosx=yes], [is_macosx=no] )
AC_MSG_RESULT([${is_macosx}])

AC_CHECK_HEADERS([ieeefp.h zlib.h])

dnl --------------------------------------------------------------------------
dnl General options
dnl --------------------------------------------------------------------------
VS_CPPFLAGS=""
VS_CXXFLAGS="-pipe "
VS_LIBS=""



if test "x${bsdthread}" = "xyes"; then
PTHREAD_CXXFLAGS="-pthread"
PTHREAD_LIBS="-pthread"
else
PTHREAD_CXXFLAGS="-D_REENTRANT"
PTHREADS_LIBS="-lpthread"
fi

AC_ARG_ENABLE(flags, AC_HELP_STRING([--enable-flags=opts], [Set specific flags for the compiler]), userflags=${enableval})


if !(test "x${userflags}" = "xno" || test "x${userflags}" = "") ; then

saved_CXXFLAGS="${CXXFLAGS}"
CXXFLAGS="$userflags"
AC_TRY_COMPILE(, , [userflagged=yes], [userflagged=no])
CXXFLAGS="${saved_CXXFLAGS}"

if test "x${userflagged}" = "xyes"; then
	USER_FLAGS="${userflags}"
	echo "Enabling user-specified flags: $userflags"
else
	echo "Invalid flags specified. Ignoring."
fi
fi

saved_CXXFLAGS="${CXXFLAGS}"
case $target in
  *freebsd* | *netbsd* | *openbsd* | *linux*)
    CXXFLAGS="-gdwarf-2 -g3 -Werror"
    AC_TRY_COMPILE(, , [gwarf2g3_supported=yes], [gwarf2g3_supported=no])
    ;;
  *)
    CXXFLAGS="-g3 -Werror"
    gwarf2g3_supported=no
    ;;
esac
CXXFLAGS="${saved_CXXFLAGS}"

if test "x$gwarf2g3_supported" = "xyes"; then
	DEBUG_FLAGS="-O0 -gdwarf-2 -g3"
else
    saved_CXXFLAGS="${CXXFLAGS}"
    CXXFLAGS="-g3"
    AC_TRY_COMPILE(, , [g3_supported=yes], [g3_supported=no])
    CXXFLAGS="${saved_CXXFLAGS}"

    if test "x$g3_supported" = "xyes"; then
	    DEBUG_FLAGS="-O0 -g3"
    else
	    DEBUG_FLAGS="-O0 -g"
    fi
fi

# Remove these flags beforehand... this should not remove functionality.
CXXFLAGS=${CXXFLAGS#-g}
CXXFLAGS=${CXXFLAS#-O2}
VS_CXXFLAGS=${VS_CXXFLAGS#-g}
VS_CXXFLAGS=${VS_CXXFLAGS#-O2}
CFLAGS=${CFLAGS#-g}
CFLAGS=${CFLAGS#-O2}

AC_ARG_ENABLE(ggdb3, AC_HELP_STRING([--enable-ggdb3], [Add the -ggdb3 compiler flag.]), 
	if test $enableval = yes ; then
		DEBUG_FLAGS="-O0 -ggdb3"
	fi)

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Produce an executable with debugging symbols]), 
	if test $enableval = yes ; then
		AC_MSG_NOTICE(Enabling debug mode with $DEBUG_FLAGS)
		VS_CXXFLAGS="${VS_CXXFLAGS} ${DEBUG_FLAGS}"
	fi)

AC_ARG_ENABLE(release, AC_HELP_STRING([--enable-release=LEVEL], [Produce an executable with optimizations (LEVEL=2)]), 
	if test $enableval = no -o $enableval = 0; then
		optimizationlevel=0
		VS_CXXFLAGS="${VS_CXXFLAGS} -O0"
	else
		optimizationlevel=$enableval
		if test $enableval = yes ; then
			optimizationlevel=2
		fi
		AC_MSG_NOTICE(Enabling optimizations with -O$optimizationlevel)
		VS_CXXFLAGS="${VS_CXXFLAGS} -O$optimizationlevel -ffast-math"
	fi
	)

AC_ARG_ENABLE(client, AC_HELP_STRING([--disable-client], [Only build vegaserver binary]), 
	build_client=$enableval
	if test $enableval = no; then
		VS_CPPFLAGS="${VS_CPPFLAGS} -DNO_GFX"
	fi
	AC_MSG_NOTICE(Building client: $enableval))

AC_ARG_ENABLE(static, AC_HELP_STRING([--enable-static], [Produce a statically linked executable]), 
	if test $enableval = yes ; then
		VS_CXXFLAGS="${VS_CXXFLAGS} -static"
		AC_MSG_NOTICE(Linking statically)
	else
		AC_MSG_NOTICE(Linking dynamically)
	fi)

AC_ARG_ENABLE(profile, AC_HELP_STRING([--enable-profile], [Produce an executable with profiling code]),  
	if test $enableval = yes ; then
		VS_CXXFLAGS="${VS_CXXFLAGS} -pg"
		AC_MSG_NOTICE(Enabling profiling)
	fi)

AC_ARG_ENABLE(wall, AC_HELP_STRING([--enable-wall], [Use the -Wall flag with the compiler.]), 
	if test $enableval = yes ; then
		VS_CXXFLAGS="${VS_CXXFLAGS} -Wall"
	fi)

AC_ARG_ENABLE(pedantic, AC_HELP_STRING([--enable-pedantic], [Enable -pedantic (and -ansi for C) modes]), 
	if test $enableval = yes ; then
		AC_MSG_NOTICE(Setting compiler to pedantic (don't expect a successful build))
		VS_CXXFLAGS="${VS_CXXFLAGS} -pedantic"
	fi)

AC_ARG_ENABLE(macosx_bundle, AC_HELP_STRING([--enable-macosx-bundle], [Produce a MacOSX bundle executable]),  
	if test $enableval = yes ; then
		AC_MSG_NOTICE(Producing a MacOSX bundle executable)
		VS_CPPFLAGS="${VS_CPPFLAGS} -DWITH_MACOSX_BUNDLE=1"
	fi)

if test "x${build_client}" = "xno" ; then
    VS_CXXFLAGS="${VS_CXXFLAGS} -DNO_GFX"
fi

AM_CONDITIONAL([VS_MAKE_NO_CLIENT], [test "x$build_client" = "xno"]) 

VS_CXXFLAGS="${VS_CXXFLAGS} ${USER_FLAGS}"

dnl --------------------------------------------------------------------------
dnl Check if compiling for Win32
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING([for Win32 platform])
AC_EGREP_CPP(yes,
[ 
#if defined(WIN32) || defined(_WIN32)
  yes
#endif
], is_win32=yes, is_win32=no )
AC_MSG_RESULT(${is_win32})

dnl Add libs required for Windows
dnl I don't use -mwindows since I like having the console around
dnl If the console annoys you then remove "-luser32 -lgdi32" and replace
dnl with "-mwindows"
WIN32="0"
if test "x${is_win32}" = "xyes" ; then
	VS_LIBS="${VS_LIBS} -luser32 -lgdi32"
	VS_CPPFLAGS="${VS_CXXFLAGS} -DNOMINMAX"
	WIN32="1"
fi

AC_MSG_CHECKING([for CygWin platform])
AC_EGREP_CPP(yes,
[ 
#if defined(__CYGWIN__)
  yes
#endif
], is_cygwin=yes, is_cygwin=no )
AC_MSG_RESULT(${is_cygwin})

AC_MSG_CHECKING([for Mingw32 platform])
AC_EGREP_CPP(yes,
[ 
#if defined(__MINGW32__)
  yes
#endif
], is_mingw=yes, is_mingw=no )
AC_MSG_RESULT(${is_mingw})
if test "x${is_mingw}" = "xyes" ; then
	VS_LIBS="${VS_LIBS} -L/usr/lib/w32api -lwsock32"
fi

dnl --------------------------------------------------------------------------
dnl Check if compiling for Linux
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING([for Linux platform])
AC_EGREP_CPP(yes,
[ 
#if defined(linux)
  yes
#endif
], is_linux=yes, is_linux=no )
AC_MSG_RESULT(${is_linux})

LINUX="0"
if test "x$is_linux" = "xyes" ; then
	LINUX="1"
fi

dnl --------------------------------------------------------------------------
dnl Check if compiling for FreeBSD 
dnl --------------------------------------------------------------------------
AC_MSG_CHECKING([for FreeBSD platform])
AC_EGREP_CPP(yes,
[ 
#if defined(__FreeBSD__)
  yes
#endif
], [is_freebsd=yes], [is_freebsd=no] )
AC_MSG_RESULT([${is_freebsd}])
#if test "x$is_freebsd" = "xyes" ; then
#    VS_LIBS="$VS_LIBS -lutil"
#fi


if test "x$is_macosx" = "xyes" ; then
	CXXFLAGS="$CXXFLAGS -I/Developer/SDKs/MacOSX10.2.8sdk/System/Library/Frameworks/Carbon.framework/Libraries/CIncludes -DHAVE_PYTHON=1 -I$with_python_inc "
	CPPFLAGS="$CPPFLAGS -Wno-long-double"
    dnl GL_CPPFLAGS="$GL_CPPFLAGS -framework OpenGL"
    dnl GLUT_CPPFLAGS="$GLUT_CPPFLAGS -framework GLUT"
	VS_LIBS="$VS_LIBS -F/Developer/SDKs/MacOSX10.3.0.sdk/System/Library/Frameworks  -framework OpenGL -framework GLUT -framework CoreAudio"
    with_gl_inc=/Developer/SDKs/MacOSX10.3.0.sdk/System/Library/Frameworks/Carbon.framework/Libraries/CIncludes
    with_gl_libs=/Developer/SDKs/MacOSX10.3.0.sdk/System/Library/Frameworks/OpenGL.framework/Libraries
    with_al_inc="/System/Library/Frameworks/OpenAL.framework/Headers -I/sw/include"
    with_al_libs="/System/Library/Frameworks/OpenAL.framework/ -L/sw/lib"
#    PYTHON_LDOPTS="-L/System/Library/Frameworks/Python.framework"
#    with_python_inc="/System/Library/Frameworks/Python.framework/Headers"
    VS_LIBS="$VS_LIBS -lobjc -L$with_python_libs "
else
    VS_LIBS="$VS_LIBS -lutil"
fi


VS_CHECK_PYTHON

AC_LANG(C++)

if test "x$build_client"  = "xno"; then
    AM_CONDITIONAL([VS_MAKE_SOUNDSERVER], [false]) 
else
    VS_CHECK_SDL
    
    dnl Only build soundserver if we have SDL Mixer (this is defined in CHECK_SDL)
    dnl AM_CONDITIONAL([VS_MAKE_SOUNDSERVER], [test "x$have_SDL_mixer" = "xyes"]) 
    dnl soundserver not used any more.
    AM_CONDITIONAL([VS_MAKE_SOUNDSERVER], [false]) 
    
    AC_LANG(C++)
  if test "x$is_macosx" = "xno" ; then 
    dnl all macs have openGL  and this test no longer works for mac
    VS_CHECK_GL
  fi
fi

dnl --------------------------------------------------------------------------
dnl Check for car sim
dnl --------------------------------------------------------------------------
AC_ARG_ENABLE(car, AC_HELP_STRING([--enable-car], [Produce an executable for car simulation]), [VS_CPPFLAGS="${VS_CPPFLAGS} -DCAR_SIM"; echo "making car simulator"])


dnl --------------------------------------------------------------------------
dnl Make sure that the scripts in m4scripts are available
dnl --------------------------------------------------------------------------
m4_ifdef([VS_CHECK_CRYPTO], [], 
[AC_FATAL([MISSING script(s) from m4scripts directory.
An easy way to get these scripts is to use ./bootstrap-sh to create configure.
If you get the message and are running bootstrap-sh, that probably means that 
the m4scripts directory does not exist.  If you are using SVN to get 
vegastrike, change to vegastrike directory, type 'svn update' and this 
directory should be created.])])  
m4_pattern_forbid([VS_CHECK_CRYPTO])



if test "x$build_client"  = "xno"; then
    echo "*** Building server without OpenGL support as requested."
else
dnl ==============================
dnl Disabling all checks for OpenGL if build_client is "no".
dnl ==============================

dnl --------------------------------------------------------------------------
dnl Check for OpenGL headers
dnl --------------------------------------------------------------------------

AC_ARG_WITH(gl-inc, AC_HELP_STRING([--with-gl-inc=DIR], [OpenGL header file location]))

if test "x$with_gl_inc" = "x" ; then
    GL_CPPFLAGS=""
else 
    GL_CPPFLAGS="$GL_CPPFLAGS -I$with_gl_inc"
fi

dnl check for gl.h
saved_CPPFLAGS="$CPPFLAGS"
AC_DEFUN([CHECK_FOR_GL_H], [
    CPPFLAGS="$saved_CPPFLAGS $GL_CPPFLAGS $VS_CPPFLAGS"

    AC_MSG_CHECKING([for GL/gl.h])
    if test "x$is_macosx" = "xyes" ; then
        have_gl_h=yes
    else
        AC_TRY_CPP( [ #include <GL/gl.h> ], have_gl_h=yes, have_gl_h=no )
    fi
    AC_MSG_RESULT([$have_gl_h])
])

CHECK_FOR_GL_H

if test "x$have_gl_h" = "xno" -a "x$GL_CPPFLAGS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenGL headers installed in the standard"
    echo "*** location (/usr/include).  I'll check in /usr/X11R6/include, since"
    echo "*** many distributions (incorrectly) put OpenGL headers there."
    GL_CPPFLAGS="-I/usr/X11R6/include"
    CHECK_FOR_GL_H
fi

if test "x$have_gl_h" = "xno" ; then
    AC_MSG_ERROR([Cannot find GL/gl.h])
fi



dnl Check for glext.h
AC_LANG(C++)

if test "x$is_macosx" != "xyes" ; then
AC_COMPILE_IFELSE(
[[
#include <GL/gl.h>    
#include <GL/glext.h>
#if (!defined(GL_ARB_vertex_buffer_object) ) && GL_GLEXT_VERSION<21
#error "Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h; Place it in the directory /usr/include/GL."
#endif
]], [], [AC_MSG_ERROR([
*****************************************************************
Your file glext.h is too old. (Must be Version 21 or greater)
You can get a new copy at
http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
Place it in the directory /usr/include/GL.
*****************************************************************])])
fi

CPPFLAGS="$saved_CPPFLAGS"
VS_CPPFLAGS="${VS_CPPFLAGS} $GL_CPPFLAGS"

dnl --------------------------------------------------------------------------
dnl Check for glut library
dnl --------------------------------------------------------------------------
AC_ARG_WITH(glut-libs, AC_HELP_STRING([--with-glut-libs=DIR], [Location of GLUT library]))

if test "x$with_glut_libs" = "x" ; then
    GLUT_LDOPTS="$GLUT_LDOPTS"
else
    GLUT_LDOPTS="-L$with_glut_libs"
fi

dnl check for glut and prerequesites to the glut-test: libXmu and libXi
AC_CHECK_LIB([Xmu], [XmuGetHostname], [have_Xmu="yes"], [have_Xmu="no"])
AC_CHECK_LIB([Xi], [XListInputDevices], [have_Xi="yes"], [have_Xi="no"])
if test "x$have_Xmu" = "xyes"; then
    VS_LIBS="$VS_LIBS -lXmu"
fi
if test "x$have_Xi" = "xyes"; then
    VS_LIBS="$VS_LIBS -lXi"
fi

AC_DEFUN([CHECK_FOR_GLUT_LIB], [
    AC_MSG_CHECKING([for $GLUT_LIB_NAME library])
    if test "x$is_macosx" = "xyes" ; then
        LIBS="$saved_LIBS $VS_LIBS $GLUT_LDOPTS -framework GLUT"
    else
	saved_LDFLAGS=$LDFLAGS
	LDFLAGS="-L/usr/X11R6/lib $saved_LDFLAGS"
        LIBS="$saved_LIBS $VS_LIBS $GLUT_LDOPTS -l$GLUT_LIB_NAME"
    fi
    echo $LIBS
    AC_TRY_LINK( , , have_glut_lib=yes, have_glut_lib=no )
    AC_MSG_RESULT([$have_glut_lib])
])
LDFLAGS="$saved_LDFLAGS"
saved_LIBS="$LIBS"

GLUT_LIB_LIST="glut32 glut"

for GLUT_LIB_NAME in $GLUT_LIB_LIST ; do
    CHECK_FOR_GLUT_LIB

    if test "x$have_glut_lib" = "xyes" ; then
        break;
    fi
done

if test "x$have_glut_lib" = "xno" ; then
    if test "x$have_Xmu" = "xno"; then
   	AC_MSG_WARN([Glut might fail because of missing libXmu-dev and/or headers!])
    fi
    if test "x$have_Xi" = "xno"; then
   	AC_MSG_WARN([Glut might fail because of missing libXi-dev and/or headers!])
    fi
    AC_MSG_ERROR([GLUT library not found or too old version. 3.7 (beta) or later required.])
fi

LIBS="$saved_LIBS"
if test "x$is_macosx" = "xyes" ; then
    VS_LIBS="$VS_LIBS $GLUT_LDOPTS -framework GLUT"
else
    LIBS="$saved_LIBS -lXi -lXmu"
    VS_LIBS="$VS_LIBS $GLUT_LDOPTS -l$GLUT_LIB_NAME"
fi


dnl --------------------------------------------------------------------------
dnl Check for glut.h
dnl --------------------------------------------------------------------------

AC_ARG_WITH(glut-inc, AC_HELP_STRING([--with-glut-inc=DIR], [Specify GLUT header file location]))

if test "x$with_glut_inc" = "x" ; then
    GLUT_CPPFLAGS="$GLUT_CPPFLAGS"
else
    GLUT_CPPFLAGS="-I$with_glut_inc"
fi

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$saved_CPPFLAGS $VS_CPPFLAGS $GLUT_CPPFLAGS"
if test "x$is_macosx" = "xyes" ; then
    have_glut_h=yes
else
    AC_CHECK_HEADER([GL/glut.h], have_glut_h=yes, have_glut_h=no )
fi
if test "x$have_glut_h" = "xno" ; then
    AC_MSG_ERROR([Cannot find GL/glut.h])
fi

CPPFLAGS="$saved_CPPFLAGS"
VS_CPPFLAGS="${VS_CPPFLAGS} $GLUT_CPPFLAGS"

fi
dnl ==============================
dnl END if "$build_client" = "no" 
dnl ==============================

dnl --------------------------------------------------------------------------
dnl Check for expat library
dnl --------------------------------------------------------------------------
AC_ARG_WITH(expat-libs, AC_HELP_STRING([--with-expat-libs=DIR], [Location of expat library]))

if test "x$with_expat_libs" = "x" ; then
    EXPAT_LDOPTS=""
else
    EXPAT_LDOPTS="-L$with_expat_libs"
fi

AC_DEFUN([CHECK_FOR_EXPAT_LIB], [
    AC_MSG_CHECKING([for $EXPAT_LIB_NAME library])
    LIBS="$EXPAT_LDOPTS -l$EXPAT_LIB_NAME"
    AC_TRY_LINK( , , have_expat_lib=yes, have_expat_lib=no )
    AC_MSG_RESULT([$have_expat_lib])
]) 

saved_LIBS="$LIBS"

EXPAT_LIB_LIST="expat"

for EXPAT_LIB_NAME in $EXPAT_LIB_LIST ; do
    CHECK_FOR_EXPAT_LIB

    if test "x$have_expat_lib" = "xyes" ; then
        break;
    fi
done

if test "x$have_expat_lib" = "xno" ; then
    AC_MSG_ERROR([Expat library not found])
fi

LIBS="$saved_LIBS"
VS_LIBS="$VS_LIBS $EXPAT_LDOPTS -l$EXPAT_LIB_NAME"


dnl --------------------------------------------------------------------------
dnl Check for expat.h
dnl --------------------------------------------------------------------------

AC_ARG_WITH(expat-inc, AC_HELP_STRING([--with-expat-inc=DIR], [Specify expat header file location]))

if test "x$with_expat_inc" = "x" ; then
    EXPAT_CPPFLAGS=""
else
    EXPAT_CPPFLAGS="-I$with_expat_inc"
fi

saved_CPPFLAGS="$CPPFLAGS"
if test "x$is_macosx" = "xyes" ; then
    CPPFLAGS="$saved_CPPFLAGS $EXPAT_CPPFLAGS "
else
    CPPFLAGS="$saved_CPPFLAGS $VS_CPPFLAGS $EXPAT_CPPFLAGS "
fi
AC_CHECK_HEADER([expat.h], have_expat_h=yes, have_expat_h=no )

if test "x$have_expat_h" = "xno" ; then
    AC_MSG_ERROR([Cannot find expat.h])
fi

CPPFLAGS="$saved_CPPFLAGS"
VS_CPPFLAGS="${VS_CPPFLAGS} ${EXPAT_CPPFLAGS}"



dnl --------------------------------------------------------------------------
dnl Check for png library
dnl --------------------------------------------------------------------------
AC_ARG_WITH(png-libs, AC_HELP_STRING([--with-png-libs=DIR], [Location of png library]))

if test "x$with_png_libs" = "x" ; then
    PNG_LDOPTS=""
else
    PNG_LDOPTS="-L$with_png_libs"
fi

AC_DEFUN([CHECK_FOR_PNG_LIB], [
    AC_MSG_CHECKING([for $PNG_LIB_NAME library])
    LIBS="$PNG_LDOPTS -l$PNG_LIB_NAME"
    AC_TRY_LINK( , , have_png_lib=yes, have_png_lib=no )
    AC_MSG_RESULT([$have_png_lib])
]) 

saved_LIBS="$LIBS"

PNG_LIB_LIST="png png12"

for PNG_LIB_NAME in $PNG_LIB_LIST ; do
    CHECK_FOR_PNG_LIB

    if test "x$have_png_lib" = "xyes" ; then
        break;
    fi
done

if test "x$have_png_lib" = "xno" ; then
     PNG_LIB_NAME="png -lz -lm"
     CHECK_FOR_PNG_LIB
     if test "x$have_png_lib" = "xno" ; then
       AC_MSG_ERROR([Png library not found])
     fi
fi

LIBS="$saved_LIBS"
VS_LIBS="$VS_LIBS $PNG_LDOPTS -l$PNG_LIB_NAME"


dnl --------------------------------------------------------------------------
dnl Check for png.h
dnl --------------------------------------------------------------------------

AC_ARG_WITH(png-inc, AC_HELP_STRING([--with-png-inc=DIR], [Specify png header file location]))

if test "x$with_png_inc" = "x" ; then
    PNG_CPPFLAGS=""
else
    PNG_CPPFLAGS="-I$with_png_inc"
fi

saved_CPPFLAGS="$CPPFLAGS"
if test "x$is_macosx" = "xyes" ; then
    CPPFLAGS="$saved_CPPFLAGS $PNG_CPPFLAGS "
else
    CPPFLAGS="$saved_CPPFLAGS $VS_CPPFLAGS $PNG_CPPFLAGS "
fi
AC_CHECK_HEADER([png.h], have_png_h=yes, have_png_h=no )

if test "x$have_png_h" = "xno" ; then
    AC_MSG_ERROR([Cannot find png.h])
fi

CPPFLAGS="$saved_CPPFLAGS"
VS_CPPFLAGS="${VS_CPPFLAGS} ${PNG_CPPFLAGS}"


VS_CHECK_JPEG

if test "x$build_client" = "xno"; then
    echo "Not checking for audio libraries."
else
    VS_CHECK_OPENAL
    
    VS_CHECK_VORBIS
fi

VS_CHECK_CRYPTO

VS_CHECK_NETCOMM

dnl --------------------------------------------------------------------------

dnl Check for win32 winsock 2 support
dnl --------------------------------------------------------------------------
if test "$WIN32" = "1" ; then
AC_ARG_ENABLE(netcomm, AC_HELP_STRING([--enable-winsock2], [Enable winsock2 support instead of winsock1 under win32]), [VS_CPPFLAGS="${VS_CPPFLAGS} -D_WIN32_WINNT=0x0400"; echo "Using winsock 2 support"])
fi

dnl --------------------------------------------------------------------------
dnl Other options
dnl --------------------------------------------------------------------------

AC_ARG_ENABLE(boost,    AC_HELP_STRING([--disable-boost], [Build without included boost]),[ACENABLE=$enableval], [ACENABLE=])
AM_CONDITIONAL(BOOSTSYSTEM,test "x$ACENABLE" != "xno")

dnl Turn use of stencil buffer on/off
AC_ARG_ENABLE(stencil-buffer, AC_HELP_STRING([--enable-stencil-buffer], [Use if your hardware has a stencil buffer]),VS_CPPFLAGS="${VS_CPPFLAGS} -DUSE_STENCIL_BUFFER=1")

AC_ARG_ENABLE(nvidia-cg, AC_HELP_STRING([--enable-nvidia-cg], 
	[Use if you have the NVidia SDK and CG compiler or compatible to get shader support (very experimental)]))
if test "x$nvidia-cg" = "xyes"; then
saved_LIBS="$LIBS"
LIBS="-lCg -lCgGL"
AC_TRY_LINK( , , cg_enable=yes, cg_enable=no)
LIBS="${saved_LIBS}"

if test "x$cg_enable" = "xyes"; then
CG_CPPFLAGS="-DCG_SUPPORT=1" 
CG_LIBS="-lCg -lCgGL"
fi

fi

dnl Specify data directory
AC_ARG_WITH(data-dir, AC_HELP_STRING([--with-data-dir=PATH], [Default vegastrike data directory]),VS_CPPFLAGS="${VS_CPPFLAGS} -DDATA_DIR=\\\"$withval\\\"")

AC_SUBST(VS_CPPFLAGS)
AC_SUBST(VS_CXXFLAGS)
AC_SUBST(VS_LIBS)
AC_SUBST(PYTHON_CPPFLAGS)
AC_SUBST(CG_CPPFLAGS)
AC_SUBST(CG_LIBS)


VS_WITH_BOOST
VS_WITH_NET_THREADS
VS_WITH_COLLIDE_MAP
VS_WITH_OGRE
VS_WITH_CEGUI

AC_LANG(C)
VS_CONFIG_GTKUTILS
AC_LANG(C++)

# The boost include must come first to override includes from elsewhere
CPPFLAGS="${BOOST_CPPFLAGS} ${CPPFLAGS} ${VS_CPPFLAGS} ${PYTHON_CPPFLAGS} ${CG_CPPFLAGS} ${CXXFLAGS} "' -I$(top_srcdir)/src '
CXXFLAGS="${CXXFLAGS} ${VS_CXXFLAGS} ${PTHREAD_CXXFLAGS}"

LIBS="${LDFLAGS} ${LIBS} ${VS_LIBS} ${PYTHON_LIBS} ${PTHREAD_LIBS} ${CG_LIBS}"

AC_MSG_NOTICE([Compiling with CPPFLAGS: ${CPPFLAGS}.])
AC_MSG_NOTICE([Compiling with CXXFLAGS: ${CXXFLAGS}.])
AC_MSG_NOTICE([Compiling with LIBS: ${LIBS}.])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT()
