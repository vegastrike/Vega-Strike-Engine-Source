INCLUDES = -I$(top_srcdir) \
	   `gtk-config --cflags`

GCF = 

bin_PROGRAMS=vslauncher
commonLIB = ../src/common/libvscommon.a
vslauncher_LDFLAGS  =`gtk-config --libs`
vslauncher_LDADD = ${commonLIB} 
vslauncher_SOURCES= saveinterface.cpp general.cpp
