if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libaldrv.a

libaldrv_a_SOURCES =				\
	al_init.cpp				\
	al_listen.cpp				\
	al_sound.cpp				\
	al_globals.cpp				\
	al_globals.h				



