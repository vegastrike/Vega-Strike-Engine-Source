# noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
bin_PROGRAMS = soundserver
INCLUDES = -I ../

soundserver_LDADD = -L/usr/local/lib -lSDL -lSDL_mixer
soundserver_SOURCES = inet.cpp inet.h soundserver.cpp

libnetgeneric =	\
	accountsxml.cpp	\
	accountsxml.h	\
	client.h		\
	clientstructs.cpp	\
	const.h			\
	netui.cpp		\
	netui.h			\
	packetmem.cpp	\
	packetmem.h		\
	packet.cpp		\
	packet.h		\
	vsnet_address.cpp		\
	vsnet_address.h			\
	vsnet_socket.cpp		\
	vsnet_socket.h			\
	vsnet_socketset.cpp		\
	vsnet_socketset.h		\
	vsnet_cmd.cpp			\
	vsnet_cmd.h				\
	vsnet_clientstate.h		\
	vsnet_serversocket.cpp	\
	vsnet_serversocket.h

libnet_a_SOURCES =	$(libnetgeneric) \
	inet.cpp		\
	inet.h

libnetclient_a_SOURCES = \
	netclient.cpp	\
	netclient.h	\
	cubicsplines.cpp	\
	cubicsplines.h

libnetacct_a_SOURCES = $(libnetgeneric) \
	acctserver.cpp \
	acctserver.h

libnetserver_a_SOURCES = $(libnetgeneric) \
	netserver.cpp	\
	netserver.h	\
	cubicsplines.cpp	\
	cubicsplines.h		\
	savenet_util.h		\
	savenet_util.cpp	\
	zonemgr.cpp	\
	zonemgr.h

