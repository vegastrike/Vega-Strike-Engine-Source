AC_INIT(vegastrike, 0.4.3)
AC_CONFIG_SRCDIR(src/vegastrike.h)
AC_PREREQ(2.53)

AC_LANG(C++)

AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CPP

AC_MSG_NOTICE([Using compiler: ${CXX}.])

dnl Check system type
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
VS_CHECK_OS


dnl Initialize AutoMake and AutoHeader systems
AM_INIT_AUTOMAKE([1.6])
AM_CONFIG_HEADER(config.h)


dnl Check for compilers.  We set CXXFLAGS to null if unset, so
dnl that these macros won't set them to default values that we don't want.
CPPFLAGS=""
CXXFLAGS=""
dnl unfortunately most users (eg hellcatv) are too stupid to figure out where that environment variable is set (it's not in env or set or echo $CPPFLAGS gives nothing) so we have to force it to null
if test "x${CXXFLAGS:-notset}" = "xnotset" ; then
   export CXXFLAGS
   CXXFLAGS=""
fi

if test "x${CPPFLAGS:-notset}" = "xnotset" ; then
   export CPPFLAGS
   CPPFLAGS=""
fi


dnl AC_GNU_SOURCE()
AC_AIX()
AC_MINIX()
AC_ISC_POSIX()


saved_CXXFLAGS="${CXXFLAGS}"
CXXFLAGS="-falign-loops=4"
AC_TRY_COMPILE(, , [falign=yes], [falign=no])
CXXFLAGS="-malign-loops=4"
AC_TRY_COMPILE(, , [malign=yes], [malign=no])
CXXFLAGS="${saved_CXXFLAGS}"

case $target in
  *Darwin* | *darwin*)
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

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

dnl Checks for header files.
dnl AC_HEADER_STDC
AC_HEADER_DIRENT
AC_FUNC_ALLOCA
AC_HEADER_SYS_WAIT

AC_CHECK_HEADERS([OS.h arpa/inet.h fcntl.h float.h limits.h math.h netdb.h netinet/in.h stddef.h sys/ioctl.h sys/param.h sys/time.h sys/timeb.h values.h wchar.h wctype.h])

if test "x$slabcheck" = "xyes"; then
AC_CHECK_HEADERS([linux/slab.h linux/malloc.h])
else
AC_CHECK_HEADERS([malloc.h])
fi

AC_CHECK_FUNCS(vfscanf)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T

AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF


dnl Checks for library functions.

AC_CHECK_FUNCS([access atexit bzero clock_gettime floor getcwd gethostbyname gethostname gettimeofday inet_ntoa lstat memmove memset mkdir munmap pow select socket sqrt strcasecmp strchr strdup strerror strstr strtol])

dnl Check for non-standard floating point functions and headers

AC_CHECK_FUNCS([finite _finite isnan _isnan])

AC_CHECK_HEADERS([ieeefp.h zlib.h])

dnl --------------------------------------------------------------------------
dnl General options
dnl --------------------------------------------------------------------------
VS_CPPFLAGS=""
VS_CXXFLAGS="-pipe "
VS_LIBS=""

if test "x${falign}" = "xyes"; then
VS_ALIGN="-falign-loops=2 -falign-jumps=2 -falign-functions=2"
else if test "x${malign}" = "xyes"; then
VS_ALIGN="-malign-loops=2 -malign-jumps=2 -malign-functions=2"
else VS_ALIGN=""
fi
fi


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
	DEBUG_FLAGS="-gdwarf-2 -g3"
else
    saved_CXXFLAGS="${CXXFLAGS}"
    CXXFLAGS="-g3"
    AC_TRY_COMPILE(, , [g3_supported=yes], [g3_supported=no])
    CXXFLAGS="${saved_CXXFLAGS}"

    if test "x$g3_supported" = "xyes"; then
	    DEBUG_FLAGS="-ggdb3"
    else
	    DEBUG_FLAGS="-g"
    fi
fi

AC_ARG_ENABLE(ggdb3, AC_HELP_STRING([--enable-ggdb3], [Add the -ggdb3 compiler flag.]), DEBUG_FLAGS="-ggdb3")

AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug], [Produce an executable with debugging symbols]), [VS_CXXFLAGS="${VS_CXXFLAGS} ${DEBUG_FLAGS}"])

AC_ARG_ENABLE(release, AC_HELP_STRING([--enable-release], [Produce an executable with optimizations]), [VS_CXXFLAGS="${VS_CXXFLAGS} -O2 -ffast-math"])

AC_ARG_ENABLE(profile, AC_HELP_STRING([--enable-profile], [Produce an executable with profiling code]),  [VS_CXXFLAGS="${VS_CXXFLAGS} -pg"])
AC_ARG_ENABLE(wall, AC_HELP_STRING([--enable-wall], [Use the -Wall flag with the compiler.]), [VS_CXXFLAGS="${VS_CXXFLAGS} -Wall"])

AC_ARG_ENABLE(pedantic, AC_HELP_STRING([--enable-pedantic], [Enable -pedantic (and -ansi for C) modes]), [VS_CXXFLAGS="${VS_CXXFLAGS} -pedantic"])

AC_ARG_ENABLE(macosx_bundle, AC_HELP_STRING([--enable-macosx-bundle], [Produce a MacOSX bundle executable]),  [VS_CPPFLAGS="${VS_CPPFLAGS} -DWITH_MACOSX_BUNDLE=1"])

VS_CXXFLAGS="${VS_CXXFLAGS} ${VS_ALIGN} ${USER_FLAGS}"

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



if test "x$is_macosx" = "xyes" ; then
	CXXFLAGS="$CXXFLAGS -I/Developer/SDKs/MacOSX10.2.8sdk/System/Library/Frameworks/Carbon.framework/Libraries/CIncludes"
	CPPFLAGS="$CPPFLAGS -Wno-long-double"
    dnl GL_CPPFLAGS="$GL_CPPFLAGS -framework OpenGL"
    dnl GLUT_CPPFLAGS="$GLUT_CPPFLAGS -framework GLUT"
	VS_LIBS="$VS_LIBS -F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks  -framework OpenGL -framework GLUT"
    with_gl_inc=/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/Carbon.framework/Libraries/CIncludes
    with_gl_libs=/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/OpenGL.framework/Libraries
    with_al_inc="/Developer/SDKs/MacOSX10.2.8.sdk/Library/Frameworks/OpenAL.framework/Headers -I/sw/include"
    with_al_libs="/Developer/SDKs/MacOSX10.2.8.sdk/Library/Frameworks/OpenAL.framework/ -L/sw/lib"
    with_expat_libs=/sw/lib
    with_png_libs=/sw/lib
    with_jpeg_libs=/sw/lib
    with_jpeg_inc=/sw/include
    with_expat_inc=/sw/include
    with_png_inc=/sw/include
    PYTHON_LDOPTS="-L/Library/Frameworks/Python.framework"
    with_python_inc=/Library/Frameworks/Python.framework/Headers
    VS_LIBS="$VS_LIBS -lobjc"
else
    VS_LIBS="$VS_LIBS -lutil"
fi

VS_CHECK_PYTHON

VS_CHECK_SDL

dnl --------------------------------------------------------------------------
dnl Check for car sim
dnl --------------------------------------------------------------------------
AC_ARG_ENABLE(car, AC_HELP_STRING([--enable-car], [Produce an executable for car simulation]), [VS_CPPFLAGS="${VS_CPPFLAGS} -DCAR_SIM"; echo "making car simulator"])



VS_CHECK_GL




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
if test "x$is_macosx" = "xyes" ; then
    have_glext_h=yes
else
    AC_CHECK_HEADERS([GL/gl.h GL/glext.h], [have_glext_h=yes], [have_glext_h=no],
    [[
    #include <GL/gl.h>    
    #include <GL/glext.h>
    #if (!defined(GL_ARB_vertex_buffer_object) ) && GL_GLEXT_VERSION<21
    #error "Download http://oss.sgi.com/projects/ogl-sample/ABI/glext.h; Place it in the directory /usr/include/GL."
    #endif
    ]])
fi

if test "x$have_glext_h" = "xno" ; then
    AC_MSG_ERROR([
*****************************************************************
Your file glext.h is too old. (Must be Version 21 or greater)
You can get a new copy at
http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
Place it in the directory /usr/include/GL.
*****************************************************************])
fi

dnl Check that glext.h version is recent enough
AC_MSG_CHECKING([whether glext.h is recent enough]);
if test "x$is_macosx" = "xno" ; then
AC_EGREP_CPP(yes,
[
#include <GL/gl.h>
#include <GL/glext.h>
#if defined(GL_GLEXT_VERSION) && GL_GLEXT_VERSION >= 6
 yes
#endif
], is_glext_h_recent=yes, is_glext_h_recent=no )
else
    is_glext_h_recent=yes
fi

AC_MSG_RESULT([$is_glext_h_recent])

if test "x$is_glext_h_recent" = "xno" ; then
    AC_MSG_ERROR([
*****************************************************************
Your copy of glext.h is out of date.
You can get a copy of the latest version at
http://oss.sgi.com/projects/ogl-sample/ABI/glext.h
Place it in the directory /usr/include/GL.
*****************************************************************])
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

AC_DEFUN([CHECK_FOR_GLUT_LIB], [
    AC_MSG_CHECKING([for $GLUT_LIB_NAME library])
    if test "x$is_macosx" = "xyes" ; then
        LIBS="$saved_LIBS $VS_LIBS $GLUT_LDOPTS -framework GLUT"
    else
	saved_LDFLAGS=$LDFLAGS
	LDFLAGS="-L/usr/X11R6/lib $saved_LDFLAGS"
        LIBS="$saved_LIBS $VS_LIBS $GLUT_LDOPTS -l$GLUT_LIB_NAME -lXi -lXmu"
    fi
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
    AC_MSG_ERROR([GLUT library not found or too old version. 3.7 (beta) or later required.])
fi

LIBS="$saved_LIBS -lXi -lXmu"
if test "x$is_macosx" = "xyes" ; then
    VS_LIBS="$VS_LIBS $GLUT_LDOPTS -framework GLUT"
else
    VS_LIBS="$VS_LIBS $GLUT_LDOPTS -l$GLUT_LIB_NAME  -lXi -lXmu"
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

VS_CHECK_OPENAL

VS_CHECK_VORBIS

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

BOOST_CPPFLAGS="-DUSE_BOOST_128=1"' -I$(top_srcdir)/src/boost'
BOOST_LIBS="../src/boost/libpython_boost.a"
TESTUSEBOOST131="1"
TESTUSEBOOST128="0"
AC_ARG_ENABLE(boost-128, AC_HELP_STRING([--enable-boost-128], [Use if your compiler has trouble with boost-131 (such as gcc 2.9x)]),TESTUSEBOOST131="0")

if test "$TESTUSEBOOST131" = "1" ; then
   BOOST_CPPFLAGS="-DUSE_BOOST_131=1 "'-I$(top_srcdir)/src/boost129'
   BOOST_LIBS="../src/boost129/libpython_boost.a"
fi

if test "$TESTUSEBOOST131" = "0" ; then
TESTUSEBOOST128="1"
fi

AC_SUBST([BOOST_LIBS])
AC_SUBST([BOOST_CPPFLAGS])
AM_CONDITIONAL(USEBOOST128, test "$TESTUSEBOOST128" = "1")
AM_CONDITIONAL(USEBOOST131, test "$TESTUSEBOOST131" = "1")
AM_CONDITIONAL(MACOSX, test "x$is_macosx" = "xyes")

VS_WITH_NET_THREADS

VS_WITH_COLLIDE_MAP

VS_CONFIG_VSLAUNCHER

CPPFLAGS="${CPPFLAGS} ${VS_CPPFLAGS} ${PYTHON_CPPFLAGS} ${BOOST_CPPFLAGS} ${CG_CPPFLAGS}"' -I$(top_srcdir)/src '
CXXFLAGS="${CXXFLAGS} ${VS_CXXFLAGS} ${PTHREAD_CXXFLAGS} -pipe"

LIBS="${LIBS} ${VS_LIBS} ${PYTHON_LIBS} ${PTHREAD_LIBS} ${CG_LIBS}"


AC_CONFIG_FILES(Makefile
                 launcher/Makefile
                 mission/Makefile
                 objconv/Makefile
                 src/Makefile
                 src/aldrv/Makefile
                 src/boost/Makefile
                 src/boost129/Makefile
                 src/cmd/Makefile
                 src/cmd/ai/Makefile
                 src/cmd/collide/Makefile
                 src/cmd/collide/csgeom/Makefile
                 src/cmd/script/Makefile
                 src/cmd/script/c_alike/Makefile
                 src/common/Makefile
                 src/gfx/Makefile
                 src/gfx/nav/Makefile
                 src/gldrv/Makefile
                 src/gui/Makefile
                 src/networking/Makefile
                 src/networking/lowlevel/Makefile
                 src/python/Makefile
		 objconv/mesher/Makefile
                 tools/Makefile
                 )
AC_OUTPUT()
