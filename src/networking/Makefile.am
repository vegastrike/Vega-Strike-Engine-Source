if MACOSX
incl2 = \
	 -I/System/Library/Frameworks/QuickTime.framework/Headers \
	 -I/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
endif

if NETCOMM_JVOIP
incl = \
	-I/usr/local/include/jvoiplib	\
	-I/usr/local/include/jrtplib	\
	-I/usr/local/include/jthread
else
incl =
endif

if USEBOOST128
INCLUDES= $(incl) $(incl2)	\
	 -I$(top_srcdir)/src	\
	 -I$(top_srcdir)/src/boost
else
INCLUDES= $(incl) $(incl2)	\
	 -I$(top_srcdir)/src	\
	 -I$(top_srcdir)/src/boost129
endif

# noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
bin_PROGRAMS = soundserver

soundserver_LDADD = -L/usr/local/lib -lSDL -lSDL_mixer
soundserver_SOURCES = inet.cpp inet.h soundserver.cpp

liblowlevel = \
	GCPtr.h		\
	vsnet_dloadmgr.cpp	\
	vsnet_dloadmgr.h	\
	vsnet_dloadenum.cpp	\
	vsnet_dloadenum.h	\
	vsnet_notify.cpp	\
	vsnet_notify.h		\
	vsnet_clientstate.cpp		\
	vsnet_address.cpp		\
	vsnet_address.h		\
	vsnet_clientstate.h		\
	vsnet_cmd.cpp		\
	vsnet_cmd.h		\
	vsnet_err.h		\
	vsnet_pipe.cpp		\
	vsnet_pipe.h		\
	vsnet_queue.h		\
	vsnet_serversocket.cpp		\
	vsnet_serversocket.h		\
	vsnet_socket.cpp		\
	vsnet_socket.h		\
	vsnet_socketset.cpp		\
	vsnet_socketset.h		\
	vsnet_sockettcp.cpp		\
	vsnet_sockettcp.h		\
	vsnet_socketudp.cpp		\
	vsnet_socketudp.h		\
	vsnet_thread.h		\
	vsnet_thread_non.cpp		\
	vsnet_thread_p.cpp		\
	vsnet_thread_sdl.cpp		\
	netui.cpp		\
	netui.h		\
	packet.cpp		\
	packet.h		\
	packetmem.cpp		\
	packetmem.h		\
	netclass.h

libacctserver = \
	client.cpp		\
	client.h		\
	const.h		\
	accountsxml.cpp		\
	accountsxml.h		\
	acctserver.h		\
	savenet_util.cpp		\
	savenet_util.h		\
	fileutil.h			\
	fileutil.cpp		\
	netbuffer.cpp		\
	netbuffer.h

libnetgeneric = \
	$(liblowlevel)   \
	$(libacctserver) \
	cubicsplines.cpp		\
	cubicsplines.h		\
	prediction.cpp		\
	prediction.h		\
	mangle.cpp		\
	mangle.h		\
	netclient_clients.cpp	\
	netclient_login.cpp	\
	netclient_devices.cpp \
	netclient.cpp		\
	netclient.h		\
	netserver_acct.cpp \
	netserver_clients.cpp \
	netserver_devices.cpp \
	netserver_net.cpp \
	netserver_login.cpp \
	netserver.cpp		\
	netserver.h		\
	zonemgr.cpp		\
	zonemgr.h

libnetclientserver =	$(libnetgeneric) \
	netserver.cpp		\
	netserver.h			\
	netclient.cpp		\
	netclient.h			\
	cubicsplines.cpp	\
	cubicsplines.h		\
	zonemgr.cpp			\
	zonemgr.h

libnet_a_SOURCES =	\
	inet.cpp		\
	inet.h

libnetclient = $(libnetclientserver) \
	networkcomm.cpp		\
	networkcomm.h		\
	webcam_support.cpp	\
	webcam_support.h

libnetclient_a_SOURCES = $(libnetclient)

libnetacct_a_SOURCES = $(libacctserver) $(liblowlevel) \
	acctserver.cpp \
	acctserver.h

libnetserver_a_SOURCES = $(libnetclientserver) \
	networkcomm_server.cpp

