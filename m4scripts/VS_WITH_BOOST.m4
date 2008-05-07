AC_DEFUN([VS_WITH_BOOST], 
[dnl
#--------------------------------------------------------------------------
#Choosing Version of Boost Python library to use
#--------------------------------------------------------------------------
AC_ARG_WITH(boost, AC_HELP_STRING([[--with-boost[=VERSION]]], 
[Currently supported versions are 1.33,1.35, and system]))
case "$with_boost" in
   "") with_boost=1.33 ;;
   "1.33" | "1.35" | "system") ;;
   *) AC_MSG_ERROR([${with_boost} is not a valid boost version]) ;;
esac

if (test "x${with_boost}" = "xsystem"); then 
BOOST_CPPFLAGS='-I/usr/include'
with_boost_ver=$with_boost



AC_MSG_NOTICE([Using System Boost])
else
dnl the stuff between [ ] is treated as a string by autoconf
dnl and placed verbatim into the configure script, where we want it.
[BOOSTDIR=`grep '^BOOSTDIR=' Makefile.am|awk 'BEGIN{FS="="}{print $(2)}'`
with_boost_ver=`echo $with_boost|sed -e s/[.]/_/`
BOOST_CPPFLAGS='-I$(top_srcdir)/$(BOOSTDIR)/'$with_boost_ver]
if (test "x${with_boost}" = "x1.35"); then
[BOOST_CPPFLAGS='-DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_PYTHON_STATIC_LIB -I$(top_srcdir)/$(BOOSTDIR)/'$with_boost_ver]
AC_DEFINE(BOOST_PYTHON_NO_PY_SIGNATURES,1,[needed for get_pytype])
AC_DEFINE(BOOST_PYTHON_STATIC_LIB,1, [needed so we create .a])
AC_MSG_NOTICE([Using Boost Version ${with_boost}.])
fi
fi
AC_SUBST(BOOST_CPPFLAGS)
AM_CONDITIONAL([BOOST1_33], [test x$with_boost = x1.33])
AM_CONDITIONAL([BOOST1_35], [test x$with_boost = x1.35])
AM_CONDITIONAL([BOOSTSYSTEM], [test x$with_boost = xsystem])
])
