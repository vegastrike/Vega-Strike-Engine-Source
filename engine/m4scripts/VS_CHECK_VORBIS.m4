AC_DEFUN([VS_CHECK_VORBIS], 
[
AC_ARG_ENABLE(vorbis_sound, AC_HELP_STRING([--disable-vorbis], [Disable vorbis support]),vorbis_sound=$enableval)

AC_ARG_WITH(vorbis-libs, AC_HELP_STRING([--with-vorbis-libs=DIR], [Location of vorbis library]))
AC_ARG_WITH(vorbis-inc, AC_HELP_STRING([--with-vorbis-inc=DIR], [Location of vorbis includes]))
if test "x$vorbis_sound" != "xno" ;then
    if test "x$with_vorbis_libs" = "x" ; then
        VORBIS_LDOPTS=""
    else
        VORBIS_LDOPTS="-L$with_vorbis_libs"
    fi
    saved_lIBS="$RRLIBS"
    
    AC_MSG_CHECKING([for vorbis library])
    LIBS=" $VORBIS_LDOPTS -lvorbisfile -lvorbis -logg "
    AC_TRY_LINK( , , have_vorbis=yes, have_vorbis=no)
    AC_MSG_RESULT([$have_vorbis])
    

    if test "x$have_openal_lib" = "xyes" ; then
       LIBS="$saved_LIBS"
       VS_LIBS="$VS_LIBS $VORBIS_LDOPTS -lvorbisfile -lvorbis -logg "
       if test "x$with_vorbis_libs" = "x" ; then
           VORBIS_CFLAGS=""
       else
           VORBIS_CFLAGS="-I$with_vorbis_inc"
       fi
       	   LIBS=" $VORBIS_LDOPTS -lvorbisfile -lvorbis -logg "
	   saved_CPPFLAGS="$CPPFLAGS"
	   CPPFLAGS="$saved_CPPFLAGS $VORBIS_CFLAGS"
	   AC_MSG_CHECKING([for vorbis header])
	   AC_TRY_CPP([#include <vorbis/vorbisfile.h> ] ,have_vorbis_h=yes , have_vorbis_h=no)
	   AC_MSG_RESULT([$have_vorbis_h])
	   if test "x$have_vorbis_h" = "xyes" ; then
	      VS_CPPFLAGS="${VS_CPPFLAGS} ${VORBIS_CFLAGS} -DHAVE_OGG "
	      echo "found VORBIS header: Enabling ingame ogg support:: ${VS_CPPFLAGS}"
	   else
	      echo "failed to locate VORBIS  install vorbis/vorbisfile.h"
	   fi
	   
       
    else
	echo "failed to locate VORBIS library"
    fi
fi
])
