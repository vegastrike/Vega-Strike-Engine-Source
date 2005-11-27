AC_DEFUN([VS_WITH_OGRE], 
[dnl
#--------------------------------------------------------------------------
# Checking to see if OGRE is available and if it should be used 
#--------------------------------------------------------------------------
[no_ogre=0]
AC_ARG_ENABLE(ogre, AC_HELP_STRING([[--disable-ogre]], 
[Disables ogre (currently disables mesher)]), [no_ogre=1])
if (test x$no_ogre = x1); then
AC_MSG_NOTICE([OGRE Support Disabled!])
fi
[if (test x$no_ogre = x0); then]
  PKG_CHECK_MODULES([OGRE], [OGRE], [AC_MSG_NOTICE([[OGRE Support Enabled]])], 
  [no_ogre=1
  AC_MSG_WARN([[OGRE Not Found - Support Disabled]])])
[fi]
AM_CONDITIONAL([HAVE_OGRE], [test x$no_ogre = 0])
])
