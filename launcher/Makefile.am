INCLUDES = -I$(top_srcdir) \
	   `gtk-config --cflags`

GCF =
AM_LDFLAGS  =`gtk-config --libs`

commonLIB = ../src/common/libvscommon.a
if VS_MAKE_LAUNCHER
  bin_PROGRAMS=vslauncher
  vslauncher_LDADD = ${commonLIB}
  vslauncher_SOURCES= saveinterface.cpp general.cpp
endif
