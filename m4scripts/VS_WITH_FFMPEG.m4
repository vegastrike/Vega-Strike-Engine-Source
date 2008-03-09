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
  AC_CHECK_HEADERS([ffmpeg/avcodec.h ffmpeg/avformat.h ffmpeg/avio.h], 
    no_ffmpeg=0; VS_LIBS="${VS_LIBS} -lavcodec -lavformat" , 
    no_ffmpeg=1)
[fi]
AM_CONDITIONAL([HAVE_FFMPEG], [test x$no_ffmpeg = 0])
])
