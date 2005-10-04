
AC_DEFUN([CHECK_FOR_GL_LIB], [
    AC_MSG_CHECKING([for $GL_LIB_NAME library])
    LIBS="$saved_LIBS $VS_LIBS $GL_LDOPTS -l$GL_LIB_NAME"
    AC_TRY_LINK( , , have_GL=yes, have_GL=no)
    AC_MSG_RESULT([${have_GL}])

    if test "x$have_GL" = "xyes" ; then
	VS_LIBS="$VS_LIBS $GL_LDOPTS -l$GL_LIB_NAME"

    else
	dnl Try with ${PTHREAD_LIBS}

	AC_MSG_CHECKING([for $GL_LIB_NAME library (with pthreads)])
	LIBS="$saved_LIBS $VS_LIBS $GL_LDOPTS -l$GL_LIB_NAME ${PTHREAD_LIBS}"
	AC_TRY_LINK( , , have_GL=yes, have_GL=no)
	AC_MSG_RESULT([$have_GL])

	if test "x$have_GL" = "xyes" ; then
	    VS_LIBS="$VS_LIBS $GL_LDOPTS -l$GL_LIB_NAME ${PTHREAD_LIBS}"
	fi
    fi
])

AC_DEFUN([FIND_GL_LIB], [
    if test "x$with_gl_lib_name" = "x" ; then
        GL_LIB_NAME="GL"
    else
        GL_LIB_NAME="$with_gl_lib_name"
    fi

    CHECK_FOR_GL_LIB

    if test "x$have_GL" = "xno" -a "x$with_gl_lib_name" = "x" ; then
        GL_LIB_LIST=`grep -v -E "^$GL_LIB_NAME\$" <<EOF
GL
MesaGL
opengl32
EOF
`

        for GL_LIB_NAME in $GL_LIB_LIST ; do
            CHECK_FOR_GL_LIB

            if test "x$have_GL" = "xyes" ; then
                break;
            fi
        done
    fi
])

AC_DEFUN([CHECK_FOR_GLU_LIB], [
    AC_MSG_CHECKING([for $GLU_LIB_NAME library])
    LIBS="$saved_LIBS $VS_LIBS $GLU_LDOPTS -l$GLU_LIB_NAME"
    AC_TRY_LINK( , , have_GLU=yes, have_GLU=no)
    AC_MSG_RESULT([$have_GLU])

    if test "x$have_GLU" = "xyes" ; then
	VS_LIBS="$VS_LIBS $GLU_LDOPTS -l$GLU_LIB_NAME"
    else
	dnl Try with ${PTHREAD_LIBS}

	AC_MSG_CHECKING([for $GLU_LIB_NAME library (with pthreads)])
	LIBS="$saved_LIBS $VS_LIBS $GLU_LDOPTS -l$GLU_LIB_NAME"
	AC_TRY_LINK( , , have_GLU=yes, have_GLU=no)
	AC_MSG_RESULT([$have_GLU])

	if test "x$have_GLU" = "xyes" ; then
	    VS_LIBS="$VS_LIBS $GLU_LDOPTS -l$GLU_LIB_NAME ${PTHREAD_LIBS}"
	fi
    fi
])

AC_DEFUN([FIND_GLU_LIB], [
    if test "x$with_glu_lib_name" = "x" ; then
        GLU_LIB_NAME="GLU"
    else
        GLU_LIB_NAME="$with_glu_lib_name"
    fi

    CHECK_FOR_GLU_LIB

    if test "x$have_GLU" = "xno" -a "x$with_glu_lib_name" = "x" ; then
        GLU_LIB_LIST=`grep -v -E "^$GLU_LIB_NAME\$" <<EOF
GLU
MesaGLU
openglu32
EOF
`

        for GLU_LIB_NAME in $GLU_LIB_LIST ; do
            CHECK_FOR_GLU_LIB

            if test "x$have_GLU" = "xyes" ; then
                break;
            fi
        done
    fi
])


AC_DEFUN([VS_CHECK_GL], 
[
#--------------------------------------------------------------------------
#Check for GL library
#--------------------------------------------------------------------------
AC_ARG_WITH(gl-libs, AC_HELP_STRING([--with-gl-libs=DIR], [GL/MesaGL library location]))
AC_ARG_WITH(gl-lib-name, AC_HELP_STRING([--with-gl-lib-name=NAME], [GL library name]))

if test "x$with_gl_libs" = "x" ; then
    GL_LDOPTS=""
else
    GL_LDOPTS="-L$with_gl_libs"
fi

saved_LIBS="$LIBS"

FIND_GL_LIB

if test "x$have_GL" = "xno" -a "x$GL_LDOPTS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenGL libraries installed in the standard"
    echo "*** location (/usr/lib).  I'll check in /usr/X11R6/lib, since"
    echo "*** many distributions (incorrectly) put OpenGL libs there."
    GL_LDOPTS="-L/usr/X11R6/lib"
    FIND_GL_LIB
fi

if test "x$have_GL" = "xno" ; then
    AC_MSG_ERROR([Cannot find GL library])
fi

if test "x${is_win32}" = "xno" -a x$is_macosx = xno; then
    dnl Make sure GL lib is up-to-date
    AC_CHECK_FUNCS( glXGetProcAddressARB, gl_up_to_date=yes, gl_up_to_date=no )

    if test "x$gl_up_to_date" = "xno" ; then
	AC_MSG_ERROR([Your copy of OpenGL is out of date. If using Mesa, make sure you have version >= 3.2.])
    fi
fi

LIBS="$saved_LIBS"

#--------------------------------------------------------------------------
#Check for GLU library
#--------------------------------------------------------------------------
AC_ARG_WITH(glu-libs, AC_HELP_STRING([--with-glu-libs=DIR], [GLU/MesaGLU library location]))
AC_ARG_WITH(glu-lib-name, AC_HELP_STRING([--with-glu-lib-name=NAME], [GLU library name]))

if test "x$with_glu_libs" = "x" ; then
    GLU_LDOPTS=""
else
    GLU_LDOPTS="-L$with_glu_libs"
fi

saved_LIBS="$LIBS"

FIND_GLU_LIB

if test "x$have_GLU" = "xno" -a "x$GLU_LDOPTS" = "x" ; then
    echo "*** Hmm, you don't seem to have OpenGLU libraries installed in the standard"
    echo "*** location (/usr/lib).  I'll check in /usr/X11R6/lib, since"
    echo "*** many distributions (incorrectly) put OpenGLU libs there."
    GLU_LDOPTS="-L/usr/X11R6/lib"
    FIND_GLU_LIB
fi

if test "x$have_GLU" = "xno" ; then
    AC_MSG_ERROR([Cannot find GLU library])
fi

LIBS="$saved_LIBS"
])
