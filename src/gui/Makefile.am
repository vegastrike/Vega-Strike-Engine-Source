if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libgui.a

libgui_a_SOURCES =				\
	button.cpp 		\
	button.h		\
	general.cpp		\
	general.h		\
	glut_support.cpp	\
	glut_support.h		\
	text_area.cpp		\
	text_area.h
