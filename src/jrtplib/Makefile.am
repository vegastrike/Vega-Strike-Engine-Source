if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libjrtp.a

libjrtp =						\
	rtpconfig.h					\
	rtpconfig_unix.h			\
	rtpconfig_win.h				\
	rtpconnection.cpp			\
	rtpconnection.h				\
	rtpcontributingsources.cpp	\
	rtpcontributingsources.h	\
	rtpdebug.cpp				\
	rtpdebug.h					\
	rtpdefines.h				\
	rtpdestlist.cpp				\
	rtpdestlist.h				\
	rtperror.cpp				\
	rtperror.h					\
	rtpexceptionstructs.h		\
	rtphandlers.cpp				\
	rtphandlers.h				\
	rtpiptable.cpp				\
	rtpiptable.h				\
	rtplocalinfo.cpp			\
	rtplocalinfo.h				\
	rtpmcasttable.cpp			\
	rtpmcasttable.h				\
	rtppacket.cpp				\
	rtppacket.h					\
	rtppacketprocessor.cpp		\
	rtppacketprocessor.h		\
	rtprandom.cpp				\
	rtprandom.h					\
	rtprtcpmodule.cpp			\
	rtprtcpmodule.h				\
	rtpsession.cpp				\
	rtpsession.h				\
	rtpsourcedata.cpp			\
	rtpsourcedata.h				\
	rtpsourcedescription.cpp	\
	rtpsourcedescription.h		\
	rtpsources.cpp				\
	rtpsources.h				\
	rtpsrclist.cpp				\
	rtpsrclist.h				\
	rtpstructs.h				\
	rtptimeutil.cpp				\
	rtptimeutil.h

if WIN32
libjrtp_a_SOURCES = $(libjrtp) rtpconfig_win.h
else
libjrtp_a_SOURCES = $(libjrtp) rtpconfig_unix.h
endif

