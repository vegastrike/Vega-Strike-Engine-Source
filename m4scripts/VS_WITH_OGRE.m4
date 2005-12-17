AC_DEFUN([VS_WITH_OGRE], 
[dnl
#--------------------------------------------------------------------------
# Checking to see if OGRE is available and if it should be used 
#--------------------------------------------------------------------------
AC_BEFORE([$0], [VS_WITH_OGRE])dnl
[no_ogre=0]
AC_ARG_ENABLE(ogre, AC_HELP_STRING([[--disable-ogre]], 
[Disables ogre (currently disables mesher)]), [no_ogre=1])
if (test x$no_ogre = x1); then
  AC_MSG_CHECKING([for OGRE])
  AC_MSG_RESULT([no (Disabled)])
[else]
  PKG_CHECK_MODULES([OGRE], [OGRE], 
    [], 
    [AC_MSG_RESULT([no])
    no_ogre=1])
[fi]
AM_CONDITIONAL([HAVE_OGRE], [test x$no_ogre = 0])
])
