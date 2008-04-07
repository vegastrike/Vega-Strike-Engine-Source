AC_INIT(VSU: Tales of the Void, 20050128, vegastrike-users@lists.sourceforge.net, vegastrike)
AC_CONFIG_SRCDIR(Version.txt)

AC_CONFIG_HEADER(config.h)

AC_PREREQ(2.50)
AC_PROG_MAKE_SET()
AC_PROG_INSTALL()

dnl Define the optional parameters

PK3="0"

AC_ARG_ENABLE(	pk3,
				AC_HELP_STRING([--enable-pk3=COMPRESSION],
				[Enable compression of pk3 volumes (COMPRESSION=0, default)]),
				[PK3="$enableval"])

HOMEDIR="vegastrike"

AC_ARG_ENABLE(	homedir,
				AC_HELP_STRING([--enable-homedir=HOMEDIR],
				[Change the location of vegastrike's homedir folder (HOMEDIR=vegastrike, default)]),
				[HOMEDIR="$enableval"])

VALID="0"
#FIXME use proper range expansion
for LEVEL [ in 0 1 2 3 4 5 6 7 8 9 ] ; do
  if [[ $LEVEL = $PK3 ]] ; then
    VALID="1"
    break
  fi ; done

if test "$VALID" = "0" ; then
  AC_MSG_ERROR([Compression level must be in range 0 (0% compression) to 9 (maximum compression).])
fi


AC_MSG_NOTICE(Checking for zip)

AC_PATH_PROG(ZIP, zip)
if test ! -x $ZIP; then
  AC_MSG_ERROR(Required binary not found)
fi

AC_SUBST(ZIP)
AC_SUBST(PK3)
AC_SUBST(HOMEDIR)

AC_OUTPUT(Makefile)
