AC_DEFUN([VS_CHECK_PYTHON], 
[
AC_REQUIRE([AC_PROG_CXX])dnl
#
# check for Python
#
dnl for now support automake versions < 1.5 for 2 reasons:
dnl  - it is not available on some distros (SuSE, RedHat, ...)
dnl  - it can not find python 2.2 in many other cases (debian, ...)
dnl 
dnl AM_PATH_PYTHON(2.2)
dnl
AC_MSG_CHECKING([for python])

FOUND_PYTHON=no
PYTHON_CXXFLAGS=""
PYTHON_LIBS=""

PYTHON_binchk="python python2 python2.2 python2.3 python2.4"
for i in ${PYTHON_binchk};
do
    PYTHON_check=`$i -V 2>/dev/null; echo $?`
    if test ${PYTHON_check} -eq 0;
    then
        FOUND_PYTHON=yes
    fi

    dnl
    dnl Change autoconf quote characters temporarily.
    dnl
    changequote(<<, >>)dnl

    if test "x${FOUND_PYTHON}" = "xyes";
    then
        PYTHON_VERSION=`$i -V 2>&1 | awk "{print $NF}"`
        if test `echo ${PYTHON_VERSION} | sed -e 's/\./ /g; s/[a-z|A-Z|+]/ /g' | awk '{print $<<1>>$<<2>>}'` -ge 22;
        then
            FOUND_PYTHON=yes
            PYTHON_SHORT=`echo ${PYTHON_VERSION} | sed -e 's/\./ /g; s/[a-z|A-Z|+]/ /g' | awk '{print $<<1>>"."$<<2>>}'`
            PYTHON_incchk="/usr/include/python /usr/include/python${PYTHON_SHORT} /usr/local/include/python /usr/local/include/python${PYTHON_SHORT} /sw/include/python /sw/include/python${PYTHON_SHORT}"

	    PYTHON_incdir=""
	    if test "x${FOUND_PYTHON}" = "xyes";
	    then
    	        for i in ${PYTHON_incchk};
    	        do
                    if test -f "$i/Python.h";
	            then
            	        PYTHON_incdir=$i
            	        break
                    fi
    	        done
	    fi

	    if test "x${PYTHON_incdir}" = "x";
	    then
    	        FOUND_PYTHON=no
	    fi
        else
            FOUND_PYTHON=no
        fi
    fi

    dnl
    dnl Reset autoconf quote characters to brackets.
    dnl
    changequote([, ])dnl

    if test "x${FOUND_PYTHON}" = "xyes";
    then
        PYTHON=$i
        break
    fi
done

if test "x${FOUND_PYTHON}" = "xno";
then
    AC_MSG_ERROR([*** Python version 2.2 or later not found!])
fi
AC_MSG_RESULT([python ${PYTHON_VERSION}])

dnl Simple check for libpython2.2.so
if test "x${FOUND_PYTHON}" = "xyes";
then
    FOUND_LIBPYTHON_SO=no
    PYTHON_libchk="${PYTHON_LIBPATH} /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64 /usr/lib64/python${PYTHON_SHORT}/config /usr/local/lib64/python${PYTHON_SHORT}/config /sw/lib/python${PYTHON_SHORT}/config /usr/lib/python${PYTHON_SHORT} /usr/lib/python${PYTHON_SHORT}/config /usr/local/lib/python${PYTHON_SHORT} /usr/local/lib/python${PYTHON_SHORT}/config /lib/python2.2/config"
    for i in ${PYTHON_libchk};
    do
	
        if test -f $i/libpython${PYTHON_SHORT}.so;
        then
	    echo "$i/libpython${PYTHON_SHORT}.so yes"
	    PYTHON_CXXFLAGS="-I${PYTHON_incdir}"
	    if test "x$is_macosx" = "xyes" ; then
	    	    PYTHON_LIBS="-L$i -lpython${PYTHON_SHORT} --export-dynamic"
	    else
		    PYTHON_LIBS="-L$i -lpython${PYTHON_SHORT} -Xlinker -export-dynamic"
	    fi
            FOUND_LIBPYTHON_SO=yes
            break
	else
          if test -f $i/libpython${PYTHON_SHORT}.a;	   
	  then
            echo "$i/libpython${PYTHON_SHORT}.a yes"
    	    PYTHON_CXXFLAGS="-I${PYTHON_incdir}"
	    if test "x$is_macosx" = "xyes" ; then
	    	    PYTHON_LIBS="$i/libpython${PYTHON_SHORT}.a --export-dynamic"
	    else
	    	    PYTHON_LIBS="$i/libpython${PYTHON_SHORT}.a  -Xlinker -export-dynamic"
	    fi
            FOUND_LIBPYTHON_SO=yes
	    break
          else
            if test -f $i/libpython${PYTHON_SHORT}.dll.a;	   
	    then
    	      PYTHON_CXXFLAGS="-I${PYTHON_incdir}"
              echo "$i/libpython${PYTHON_SHORT}.dll.a yes"
	      PYTHON_LIBS="$i/libpython${PYTHON_SHORT}.dll.a  -Xlinker -export-dynamic"
              FOUND_LIBPYTHON_SO=yes
            else
              echo "$i/libpython${PYTHON_SHORT}.so no"
            fi
	  fi
        fi
    done
fi

dnl if test "x${FOUND_LIBPYTHON_SO}" = "xyes";
dnl then
dnl    PYTHON_CFLAGS="-I${PYTHON_incdir}"
dnl    PYTHON_LIBS="-lpython${PYTHON_SHORT}"
dnl fi

if test "x${FOUND_LIBPYTHON_SO}" = "xno";
then
    echo
    echo "Missing {python-prefix}/lib/libpython${PYTHON_SHORT}.so"
    echo "Try to create the shared library using (root priviledge required):"
    echo "    su -c 'sh ./build_libpython_so.sh'"
    echo
    AC_MSG_WARN([*** Python shared library not found!])
else
    PYTHON_CPPFLAGS="$PYTHON_CPPFLAGS $PYTHON_CXXFLAGS -DHAVE_PYTHON=1 "
fi
AC_SUBST(PYTHON_LIBS)
])
