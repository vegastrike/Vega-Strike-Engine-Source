if MACOSX
incl2 = \
	 -I/System/Library/Frameworks/QuickTime.framework/Headers \
	 -I/System/Library/Frameworks/Carbon.framework/Versions/A/Headers
endif

incl = \
	 -I$(top_srcdir)/src


noinst_LIBRARIES = libnet.a libnetclient.a libnetserver.a libnetacct.a
bin_PROGRAMS = soundserver

soundserver_LDADD = -L/usr/local/lib $(SDL_LIBS) $(SDL_MIXER_LIBS)

SUBDIRS = lowlevel

soundserver_SOURCES = inet.cpp inet_file.cpp inet_file.h inet.h soundserver.cpp

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
	fileutil.cpp

libnetgeneric = \
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
	inet_file.cpp		\
	inet_file.h		\
	inet.h

libnetclient = $(libnetclientserver) \
	networkcomm.cpp		\
	networkcomm.h		\
	webcam_suport.h		\
	webcam_support.cpp

libnetclient_a_SOURCES = $(libnetclient)

libnetacct_a_SOURCES = $(libacctserver) \
	acctserver.cpp \
	acctserver.h

libnetserver = $(libnetclientserver) \
	networkcomm_server.cpp

libnetserver_a_SOURCES = $(libnetserver)
