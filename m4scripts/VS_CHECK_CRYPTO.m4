AC_DEFUN([VS_CHECK_CRYPTO], 
[
dnl --------------------------------------------------------------------------
dnl Check for crypto++ library
dnl --------------------------------------------------------------------------

CRYPTO="0"
dnl AC_ARG_ENABLE(crypto, AC_HELP_STRING([--enable-crypto], [Enable crypto++ library support]), [CRYPTO="1"])

	AC_ARG_ENABLE(	crypto,
				AC_HELP_STRING([--enable-crypto=VERSION],
				[Enable crypto++ library support (VERSION=51 (5.1, default))]),
				[
   					CRYPTO="1"
					CRYPTO_VERSION="$enableval"
					if test "x$enableval" = "xyes" ; then
							CRYPTO_VERSION="51"
					fi
				],
				[])

if test "$CRYPTO" = "1" ; then
	dnl AC_DEFUN([CHECK_FOR_CRYPTOLIB_H], [
	dnl	CPPFLAGS="$CPPFLAGS $CRYPTO_FLAGS"
	dnl    dnl AC_MSG_CHECKING([for crypto++/cryptlib.h])
    dnl    AC_CHECK_HEADERS(crypto++/cryptlib.h, have_crypto_h=yes, have_crypto_h=no )
	    dnl AC_TRY_CPP( [ #include <crypto++/cryptlib.h> ], have_crypto_h=yes, have_crypto_h=no )
	dnl    AC_MSG_RESULT([$have_crypto_h])
	dnl ])

	dnl CHECK_FOR_CRYPTOLIB_H
	dnl if test "x$have_crypto_h" = "xyes" ; then
	dnl	VS_CPPFLAGS="$VS_CPPFLAGS -DCRPYTO"
	dnl else
	dnl	echo "checking in /usr/local/include..."
	dnl	CRYPTO_FLAGS="-I/usr/local/include"
	dnl	CHECK_FOR_CRYPTOLIB_H
	dnl	if test "x$have_crypto_h" = "xyes" ; then
	dnl		VS_CPPFLAGS="$VS_CPPFLAGS -DCRPYTO"
	dnl	else
	dnl		echo "No cryptolib support"
	dnl	fi
	dnl fi

    AC_DEFUN([CHECK_FOR_CRYPTO_LIB], [
        AC_MSG_CHECKING([for crypto++ library with $CRYPTO_LIB])
		VS_LIBS="$VS_LIBS $CRYPTO_LIB"
		LIBS="$VS_LIBS"
        AC_TRY_LINK( , , have_crypto_lib=yes, have_crypto_lib=no )
        AC_MSG_RESULT([$have_crypto_lib])
    ]) 
	saved_VSLIBS="$VS_LIBS"
	CRYPTO_LIB="-lcrypto++"
	CHECK_FOR_CRYPTO_LIB
	if test "x$have_crypto_lib" = "xno" ; then
		CRYPTO_LIB="-L/usr/local/lib -lcrypto++"
		CHECK_FOR_CRYPTO_LIB
		dnl Test for debian
		if test "x$have_crypto_lib" = "xno" ; then
			VS_LIBS="$saved_VSLIBS"
			CRYPTO_LIB="-lcrypto++5.1"
			CHECK_FOR_CRYPTO_LIB
			if test "x$have_crypto_lib" = "xno" ; then
				VS_LIBS="$saved_VSLIBS"
				CRYPTO_LIB="-lcryptopp"
				CHECK_FOR_CRYPTO_LIB
				if test "x$have_crypto_lib" = "xno" ; then
					VS_LIBS="$saved_VSLIBS"
					CRYPTO_LIB="-L/usr/local/lib -lcryptopp"
					CHECK_FOR_CRYPTO_LIB
					if test "x$have_crypto_lib" = "xno" ; then
						VS_LIBS="$saved_VSLIBS"
					fi
				fi
			fi
		fi
	fi
	LIBS="$VSLIBS"
	if test "x$have_crypto_lib" = "xyes" ; then
		VS_CPPFLAGS="$VS_CPPFLAGS -DCRYPTO=$CRYPTO_VERSION"
		CPPFLAGS="$CPPFLAGS -I/usr/local/include"
		echo "Crypto++ library support enabled"
	else
		echo "No Crypto++ support"
	fi
else
	echo "No Crypto++ support"
fi
])
