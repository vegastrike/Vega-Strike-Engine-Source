noinst_LIBRARIES = libnet.a
bin_PROGRAMS = soundserver
INCLUDES = -I ../

soundserver_LDADD = -lSDL_mixer
soundserver_SOURCES = inet.cpp inet.h soundserver.cpp

libnet_a_SOURCES =				\
	inet.cpp	\
	inet.h	