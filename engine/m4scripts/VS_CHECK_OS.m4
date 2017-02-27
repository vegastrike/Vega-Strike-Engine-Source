AC_DEFUN([VS_CHECK_OS], 
[
AC_REQUIRE([AC_CANONICAL_TARGET]) dnl
# Vega Strike OS Checks
# Conditionally sets the following AC_DEFINES
# depending on the value of target
# BSD, POSIX, UNIX, LINUX, WIN32_POSIX, WIN32, MACOSX
AH_TEMPLATE(BSD, [Is this a BSD variant?])
AH_TEMPLATE(POSIX, [Is this a known POSIX-compliant system?])
AH_TEMPLATE(UNIX, [Is this a known UNIX variant?])
AH_TEMPLATE(WIN32, [Is this Windows?])
AH_TEMPLATE(WIN32_POSIX, [Is this MingW, Cygwin, or a variant POSIX layer?])
AH_TEMPLATE(LINUX, [Is this Linux?])
AH_TEMPLATE(MACOSX, [Is this MacOS X?])

case $target in
*freebsd* | *netbsd* | *openbsd*)
	AC_DEFINE(BSD, 1)
	AC_DEFINE(POSIX,1)
	AC_DEFINE(UNIX,1)
;;
*linux*)
	AC_DEFINE(LINUX, 1)
	AC_DEFINE(POSIX,1)
	AC_DEFINE(UNIX,1)
	slabcheck=yes;
;;
*mingw* | *cygwin*)
	AC_DEFINE(WIN32_POSIX, 1)
	AC_DEFINE(WIN32, 1)
;;
*win32*)
	AC_DEFINE(WIN32, 1)
;;
*Darwin* | *darwin*)
	AC_DEFINE(MACOSX, 1)
	AC_DEFINE(POSIX, 1)
	AC_DEFINE(BSD, 199999)
;;
*)
	slabcheck=no;
;;
esac                                             
AM_CONDITIONAL(WIN32, test "$WIN32" = "1")
AM_CONDITIONAL(LINUX, test "$LINUX" = "1")
AM_CONDITIONAL(MACOSX, test "$MACOSX" = "1")
])
