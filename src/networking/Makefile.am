noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a
bin_PROGRAMS = soundserver
INCLUDES = -I ../

soundserver_LDADD = -lSDL_mixer
soundserver_SOURCES = inet.cpp inet.h soundserver.cpp

libnet_a_SOURCES =	\
	inet.cpp		\
	inet.h

libnetclient_a_SOURCES =	\
	client.h	\
	clientstructs.cpp	\
	const.h		\
	netclient.cpp	\
	netclient.h	\
	netui.h		\
	netclass.h	\
	packet.cpp	\
	packet.h	\
	cubicsplines.cpp	\
	cubicsplines.h

libnetserver_a_SOURCES =	\
	accountsxml.cpp	\
	accountsxml.h	\
	acctserver.cpp	\
	acctserver.h	\
	client.h	\
	clientstructs.cpp	\
	const.h		\
	netserver.cpp	\
	netserver.h	\
	netui.h		\
	netclass.h	\
	packet.cpp	\
	packet.h	\
	cubicsplines.cpp	\
	cubicsplines.h		\
	zonemgr.cpp	\
	zonemgr.h
