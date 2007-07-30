AC_DEFUN([VS_CONFIG_GTKUTILS], 
[CHECK_GTK=2
HAVE_GTK=no
HAVE_GTK1=no
HAVE_GTK2=no
ALLOW_GTK1=yes

AC_ARG_ENABLE(oldgtk, AC_HELP_STRING([--enable-oldgtk], [use gtk 1.2 even if
GTK 2 is installed. ]),  [oldgtk=$enableval], [oldgtk=])

if test "x$oldgtk" = "xyes"; then
  CHECK_GTK=1
elif test "x$oldgtk" = "xno"; then
  ALLOW_GTK1=no
fi

dnl AM_CONDITIONAL(USE_GTK1,test "x$ACENABLE" = "xyes")

AC_ARG_ENABLE(gtk, AC_HELP_STRING([--disable-gtk], [Disable building of
vssetup (disable GTK)]), [CHECK_GTK=0])

if test x"$CHECK_GTK" = x"2"; then
  PKG_CHECK_MODULES([GTK2], [gtk+-2.0], [HAVE_GTK2=yes], [AC_MSG_RESULT([no])] )
  if test "x$HAVE_GTK2" = "xyes"; then
    GTK_CFLAGS=$GTK2_CFLAGS
    GTK_LIBS=$GTK2_LIBS
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
    HAVE_GTK=yes
  elif test "x$ALLOW_GTK1" = "xno"; then
    HAVE_GTK=no
    AC_MSG_WARN([[GTK 2 was not found, 1.2 disabled.  VSSETUP will not build.]])
  else
    CHECK_GTK=1
  fi
fi

if test x"$CHECK_GTK" = x"1"; then
  AM_PATH_GTK([1.2.0], [HAVE_GTK1=yes], [HAVE_GTK1=no])
  HAVE_GTK="$HAVE_GTK1"
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
fi


AM_CONDITIONAL([VS_MAKE_GTKUTILS], [test x$HAVE_GTK = xyes]) 
if test x$HAVE_GTK = xno; then
  if test x$CHECK_GTK = x1; then
    AC_MSG_WARN([[GTK 1.2 Was not found.  VSSETUP will not be built.]])
  fi
else
  echo -n 'Using GTK version '
  if test "x$HAVE_GTK1" = "xyes"; then
    echo '1.2'
  elif test "x$HAVE_GTK2" = "xyes"; then
    echo '2.0'
  fi
fi
])
