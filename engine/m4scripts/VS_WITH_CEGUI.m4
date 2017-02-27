AC_DEFUN([VS_WITH_CEGUI], 
[dnl
#--------------------------------------------------------------------------
# Checking to see if Cegui and Cegui Renders are available 
# and if they should be used 
#--------------------------------------------------------------------------
[no_cegui=0]
[no_cegui_opengl=0]
[no_cegui_ogre=0]
AC_ARG_ENABLE(cegui, AC_HELP_STRING([[--disable-cegui]], 
[Disables Cegui]), [no_cegui=1])
[if (test x$no_cegui = x1); then]
  AC_MSG_CHECKING([for CEGUI])
  AC_MSG_RESULT([no (Disabled)])
  AC_MSG_CHECKING([for CEGUI-OPENGL])
  AC_MSG_RESULT([no (Disabled)])
  AC_MSG_CHECKING([for CEGUI-OGRE])
  AC_MSG_RESULT([no (Disabled)])
[else]
  PKG_CHECK_MODULES([CEGUI], [CEGUI], 
    [], 
    [AC_MSG_RESULT([no])
    no_cegui=1])
  PKG_CHECK_MODULES([CEGUI_OPENGL], [CEGUI-OPENGL],
    [], 
    [AC_MSG_RESULT([no])
    no_cegui_opengl=1])
  PKG_CHECK_MODULES([CEGUI_OGRE], [CEGUI-OGRE],
    [], 
    [AC_MSG_RESULT([no])
    no_cegui_ogre=1])
[fi]
AM_CONDITIONAL([HAVE_CEGUI], [test x$no_cegui = 0])
AM_CONDITIONAL([HAVE_CEGUI_OGRE], [test x$no_cegui_ogre = 0])
AM_CONDITIONAL([HAVE_CEGUI_OPENGL], [test x$no_cegui_opengl = 0])
])
