if MACOSX
incl2 = \
	 -I/System/Library/Frameworks/QuickTime.framework/Headers \
	 -I/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
endif

incl = \
	-I/usr/local/include/jvoiplib	\
	-I/usr/local/include/jrtplib	\
	-I/usr/local/include/jthread
#    -I$(top_srcdir)/src/jvoiplib \
#	 -I$(top_srcdir)/src/jvoiplib/libcomponents/base	\
#	 -I$(top_srcdir)/src/jvoiplib/libcomponents/impl	\
#	 -I$(top_srcdir)/src/jvoiplib/componentframework	\
#	 -I$(top_srcdir)/src/jvoiplib/thirdparty/lpc 		\
#	 -I$(top_srcdir)/src/jvoiplib/thirdparty/gsm/inc	\
#	 -I$(top_srcdir)/src/jthread \
#	 -I$(top_srcdir)/src/jrtplib

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

libnetgeneric = \
	vsnet_dloadmgr.cpp	\
	vsnet_dloadmgr.h	\
	vsnet_dloadenum.cpp	\
	vsnet_dloadenum.h	\
	vsnet_notify.cpp	\
	vsnet_notify.h		\
	accountsxml.cpp		\
	accountsxml.h		\
	acctserver.h		\
	client.h		\
	clientstructs.cpp		\
	const.h		\
	cubicsplines.cpp		\
	cubicsplines.h		\
	GCPtr.h		\
	inet.cpp		\
	inet.h		\
	inet_file.cpp		\
	inet_file.h		\
	mangle.cpp		\
	mangle.h		\
	netbuffer.cpp		\
	netbuffer.h		\
	netclass.h		\
	netclient.cpp		\
	netclient.h		\
	netserver.cpp		\
	netserver.h		\
	netui.cpp		\
	netui.h		\
	packet.cpp		\
	packet.h		\
	packetmem.cpp		\
	packetmem.h		\
	savenet_util.cpp		\
	savenet_util.h		\
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

libnetacct_a_SOURCES = $(libnetgeneric) \
	acctserver.cpp \
	acctserver.h

libnetserver_a_SOURCES = $(libnetclientserver) \
	networkcomm_server.cpp

