AC_DEFUN([VS_CONFIG_GTKUTILS], 
[CHECK_GTK=2
HAVE_GTK=no
HAVE_GTK1=no
HAVE_GTK2=no
ALLOW_GTK1=yes

AC_ARG_ENABLE(curses, AC_HELP_STRING([--disable-curses], [Check for ncurses Setup if unable to use GTK.]),  [oldgtk=$enableval], [ncurses=yes])

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
    GTK_CFLAGS="-DGTK $GTK2_CFLAGS"
    GTK_LIBS=$GTK2_LIBS
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
    HAVE_GTK=yes
  elif test "x$ALLOW_GTK1" = "xno"; then
    HAVE_GTK=no
  else
    CHECK_GTK=1
  fi
fi

if test x"$CHECK_GTK" = x"1"; then
  AM_PATH_GTK([1.2.0], [HAVE_GTK1=yes], [HAVE_GTK1=no])
  HAVE_GTK="$HAVE_GTK1"
  GTK_CFLAGS="$GTK_CFLAGS -DGTK"
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
fi

AC_DEFUN([VS_CHECK_DIALOG],
[
    AC_ARG_WITH(dialog-libs, AC_HELP_STRING([--with-dialog-libs=DIR], [Location of dialog library]))
    if test "x$with_dialog_libs" = "x" ; then
      DIALOG_LIBS=""
    else
      DIALOG_LIBS="-L$with_dialog_libs"
    fi
    AC_MSG_CHECKING([for libdialog and libncurses])
    DIALOG_LIBS="$DIALOG_LIBS -lncurses -ldialog"
    saved_LIBS="${LIBS}"
    LIBS="${LIBS} $DIALOG_LIBS"
    AC_TRY_LINK(, , [havedialog=yes], [havedialog=no])
    LIBS="$saved_LIBS"
    AC_MSG_RESULT($havedialog)
    if test "x$havedialog" = "xyes"; then
      AC_ARG_WITH(dialog-inc, AC_HELP_STRING([--with-dialog-inc=DIR], [Location of dialog includes]))
      if test "x$with_dialog_inc" = "x" ; then
        DIALOG_CPPFLAGS=""
      else
        DIALOG_CPPFLAGS="-I$with_dialog_inc"
      fi
      saved_CPPFLAGS="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS $DIALOG_CPPFLAGS"
      AC_CHECK_HEADER([dialog.h], have_dialog_h=yes, have_dialog_h=no )
      CPPFLAGS="$saved_CPPFLAGS"
      if test "x$have_dialog_h" = "xyes"; then
        GTK_CFLAGS="-DCONSOLE $DIALOG_CPPFLAGS"
        GTK_LIBS="$DIALOG_LIBS"
        AC_SUBST(GTK_CFLAGS)
        AC_SUBST(GTK_LIBS)
        HAVE_GTK=yes
      fi
    fi
])

if test x$HAVE_GTK = xno; then
  if test x$ncurses = xyes; then
    VS_CHECK_DIALOG
  fi
  if test x$HAVE_GTK = xyes; then
    echo 'Using console interface for the setup utility.'
  else
    if test x$CHECK_GTK = x1; then
      AC_MSG_WARN([[No version of GTK was found.  VSSETUP will not be built.]])
    elif test "x$ALLOW_GTK1" = "xno"; then
      AC_MSG_WARN([[GTK 2 was not found, 1.2 disabled.  VSSETUP will not build.]])
    fi
  fi
else
  echo -n 'Using GTK version '
  if test "x$HAVE_GTK1" = "xyes"; then
    echo '1.2'
  elif test "x$HAVE_GTK2" = "xyes"; then
    echo '2.0'
  fi
fi

AM_CONDITIONAL([VS_MAKE_GTKUTILS], [test x$HAVE_GTK = xyes]) 

])
