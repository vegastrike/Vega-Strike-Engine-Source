AC_DEFUN([VS_WITH_FFMPEG], 
[dnl
#--------------------------------------------------------------------------
# Checking to see if OGRE is available and if it should be used 
#--------------------------------------------------------------------------
AC_BEFORE([$0], [VS_WITH_FFMPEG])dnl
[no_ffmpeg=0]
AC_ARG_ENABLE(ffmpeg, AC_HELP_STRING([[--disable-ffmpeg]], 
[Disables ffmpeg (video file support)]), [no_ffmpeg=1])
if (test x$no_ffmpeg = x1); then
  AC_MSG_CHECKING([for ffmpeg])
  AC_MSG_RESULT([no (Disabled)])
[else]
  AC_CHECK_HEADERS([ffmpeg/avcodec.h ffmpeg/avformat.h ffmpeg/avio.h], no_ffmpeg=0, no_ffmpeg=1)
  if (test x$no_ffmpeg = x0); then
    AC_MSG_CHECKING([for libavcodec and libavformat])
    NEW_LIBS="-lavcodec -lavformat"
    saved_LIBS="${LIBS}"
    LIBS="${LIBS} $NEW_LIBS"
    AC_TRY_LINK(, , [haveavcodec=yes], [haveavcodec=no; no_ffmpeg=1])
    LIBS="$saved_LIBS"
    AC_MSG_RESULT($haveavcodec)
    
    if (test x$haveavcodec = xyes); then
      VS_LIBS="${VS_LIBS} ${NEW_LIBS}"
      
      AC_MSG_CHECKING([for libswscale])
      NEW_LIBS="-lswscale"
      saved_LIBS="${LIBS}"
      LIBS="${LIBS} $NEW_LIBS"
      AC_TRY_LINK(, , [haveswscale=yes], [haveswscale=no])
      LIBS="$saved_LIBS"
      AC_MSG_RESULT($haveswscale)
      
      if test "x$haveswscale" = "xyes"; then
        AC_CHECK_HEADERS([ffmpeg/swscale.h], haveswscale=yes, haveswscale=no)
      fi
      if test "x$haveswscale" = "xyes"; then
        VS_LIBS="${VS_LIBS} ${NEW_LIBS}"
      else
        AC_MSG_CHECKING([for img_convert in libavcodec])
        saved_LIBS="${LIBS}"
        LIBS="-lavcodec -lavformat"
        AC_TRY_LINK([extern "C" {
		#include <ffmpeg/libavcodec.h>
		#include <ffmpeg/libavformat.h>
		#include <ffmpeg/libavio.h>
		}], [ img_convert(NULL,PIX_FMT_RGB24,NULL,0,0,0);], [ result=yes; no_ffmpeg=0], [result=no; no_ffmpeg=1])
        AC_MSG_RESULT($result)
        LIBS="$saved_LIBS"
      fi
      if test "x$no_ffmpeg" = "x0"; then
        VS_CPPFLAGS="${VS_CPPFLAGS} -DHAVE_FFMPEG"
      fi
    fi
  fi
[fi]
AM_CONDITIONAL([HAVE_FFMPEG], [test x$no_ffmpeg = 0])
])
