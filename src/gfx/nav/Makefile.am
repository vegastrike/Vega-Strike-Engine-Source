if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libnav.a

libnav_a_SOURCES =	\
	drawlist.cpp drawlist.h navgetxmldata.cpp navgetxmldata.h navitemstodraw.h navitemtypes.h navparse.h navscreen.cpp navscreen.h navscreenoccupied.cpp navscreenoccupied.h drawgalaxy.cpp
