AC_DEFUN([CHECK_FOR_JPEG_LIB], [
    AC_MSG_CHECKING([for $JPEG_LIB_NAME library])
    LIBS="$JPEG_LDOPTS -l$JPEG_LIB_NAME"
    AC_TRY_LINK( , , have_jpeg_lib=yes, have_jpeg_lib=no )
    AC_MSG_RESULT([$have_jpeg_lib])
]) 

AC_DEFUN([VS_CHECK_JPEG], 
[
#--------------------------------------------------------------------------
#Check for jpeg library
#--------------------------------------------------------------------------
AC_ARG_WITH(jpeg-libs, AC_HELP_STRING([--with-jpeg-libs=DIR], [Location of jpeg library]))

if test "x$with_jpeg_libs" = "x" ; then
    JPEG_LDOPTS=""
else
    JPEG_LDOPTS="-L$with_jpeg_libs"
fi

saved_LIBS="$LIBS"

JPEG_LIB_LIST="jpeg"

for JPEG_LIB_NAME in $JPEG_LIB_LIST ; do
    CHECK_FOR_JPEG_LIB

    if test "x$have_jpeg_lib" = "xyes" ; then
        break;
    fi
done

if test "x$have_jpeg_lib" = "xno" ; then
    AC_MSG_ERROR([Jpeg library not found])
fi

LIBS="$saved_LIBS"
VS_LIBS="$VS_LIBS $JPEG_LDOPTS -l$JPEG_LIB_NAME"

#--------------------------------------------------------------------------
#Check for jpeg.h
#--------------------------------------------------------------------------

AC_ARG_WITH(jpeg-inc, AC_HELP_STRING([--with-jpeg-inc=DIR], [Specify jpeg header file location]))

if test "x$with_jpeg_inc" = "x" ; then
    JPEG_CPPFLAGS=""
else
    JPEG_CPPFLAGS="-I$with_jpeg_inc -DJPEG_SUPPORT"
fi

saved_CPPFLAGS="$CPPFLAGS"
if test "x$is_macosx" = "xyes" ; then
    CPPFLAGS="$saved_CPPFLAGS $JPEG_CPPFLAGS "
else
    CPPFLAGS="$saved_CPPFLAGS $VS_CPPFLAGS $JPEG_CPPFLAGS "
fi
AC_CHECK_HEADER([jpeglib.h], have_jpeg_h=yes, have_jpeg_h=no )
if test "x$have_jpeg_h" = "xno" ; then
    AC_MSG_WARN([Cannot find jpeg.h])
fi

CPPFLAGS="$saved_CPPFLAGS"
VS_CPPFLAGS="${VS_CPPFLAGS} ${JPEG_CPPFLAGS}"
])
