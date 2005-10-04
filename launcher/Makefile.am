INCLUDES = -I$(top_srcdir) \
	   `gtk-config --cflags`

GCF =
AM_LDFLAGS  =`gtk-config --libs`

bin_PROGRAMS=vslauncher
commonLIB = ../src/common/libvscommon.a
vslauncher_LDADD = ${commonLIB}
vslauncher_SOURCES= saveinterface.cpp general.cpp
