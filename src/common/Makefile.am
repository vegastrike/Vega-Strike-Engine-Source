if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libvscommon.a

libvscommon_a_SOURCES =	\
	common.cpp	\
	common.h
