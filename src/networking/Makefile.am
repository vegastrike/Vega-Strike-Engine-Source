noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
bin_PROGRAMS = soundserver
INCLUDES = -I ../

soundserver_LDADD = -L/usr/local/lib -lSDL -lSDL_mixer
soundserver_SOURCES = inet.cpp inet.h soundserver.cpp

libnet_a_SOURCES =	\
	inet.cpp		\
	inet.h

libnetclient_a_SOURCES = $(libnetgeneric)	\
	netclient.cpp	\
	netclient.h	\
	cubicsplines.cpp	\
	cubicsplines.h

libnetgeneric =	\
	accountsxml.cpp	\
	accountsxml.h	\
	client.h	\
	clientstructs.cpp	\
	const.h		\
	netui.h		\
	netclass.h	\
	packet.cpp	\
	packet.h

libnetacct_a_SOURCES = $(libnetgeneric)	\
	acctserver.cpp \
	acctserver.h

libnetserver_a_SOURCES = $(libnetgeneric)	\
	netserver.cpp	\
	netserver.h	\
	cubicsplines.cpp	\
	cubicsplines.h		\
	zonemgr.cpp	\
	zonemgr.h
