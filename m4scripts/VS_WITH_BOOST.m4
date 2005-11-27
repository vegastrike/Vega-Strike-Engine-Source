AC_DEFUN([VS_WITH_BOOST], 
[dnl
#--------------------------------------------------------------------------
#Choosing Version of Boost Python library to use
#--------------------------------------------------------------------------
AC_ARG_WITH(boost, AC_HELP_STRING([[--with-boost[=VERSION]]], 
[Currently supported versions are 1.28, 1.31, and 1.33]))
case "$with_boost" in
   "") with_boost=1.33 ;;
   "1.28" | "1.31" | "1.33") ;;
   *) AC_MSG_ERROR([${with_boost} is not a valid boost version]) ;;
esac
dnl the stuff between [ ] is treated as a string by autoconf
dnl and placed verbatim into the configure script, where we want it.
[BOOSTDIR=`grep '^BOOSTDIR=' Makefile.am|awk 'BEGIN{FS="="}{print $(2)}'`
with_boost_ver=`echo $with_boost|awk '{gsub("[.]", "_", $(0)); print ($(0))}'`
BOOST_CPPFLAGS='-I$(top_srcdir)/$(BOOSTDIR)/'$with_boost_ver]
AC_MSG_NOTICE([Using Boost Version ${with_boost}.])
AC_SUBST(BOOST_CPPFLAGS)
AM_CONDITIONAL([BOOST1_28], [test x$with_boost = x1.28])
AM_CONDITIONAL([BOOST1_31], [test x$with_boost = x1.31])
AM_CONDITIONAL([BOOST1_33], [test x$with_boost = x1.33])

])
