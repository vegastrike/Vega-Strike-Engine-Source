if USEBOOST128
includes=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost \
	 -I$(top_srcdir)/src/jvoiplib/libcomponents/base	\
	 -I$(top_srcdir)/src/jvoiplib/libcomponents/impl	\
	 -I$(top_srcdir)/src/jvoiplib/componentframework	\
	 -I$(top_srcdir)/src/jvoiplib/thirdparty/lpc 		\
	 -I$(top_srcdir)/src/jvoiplib/thirdparty/gsm/inc	\
	 -I$(top_srcdir)/src/jthread \
	 -I$(top_srcdir)/src/jrtplib
else
includes=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129 \
	 -I$(top_srcdir)/src/jvoiplib/libcomponents/base	\
	 -I$(top_srcdir)/src/jvoiplib/libcomponents/impl	\
	 -I$(top_srcdir)/src/jvoiplib/componentframework	\
	 -I$(top_srcdir)/src/jvoiplib/thirdparty/lpc 		\
	 -I$(top_srcdir)/src/jvoiplib/thirdparty/gsm/inc 	\
	 -I$(top_srcdir)/src/jthread \
	 -I$(top_srcdir)/src/jrtplib
endif

if WIN32
INCLUDES=$(includes) -I$(top_srcdir)/src/jvoiplib/libcomponents/impl/win32
else
INCLUDES=$(includes) -I$(top_srcdir)/src/jvoiplib/libcomponents/impl/oss
endif

noinst_LIBRARIES = libjvoip.a libgsm.a liblpc.a

libgsm_a_SOURCES = \
	thirdparty/gsm/src/gsm_add.cpp			\
	thirdparty/gsm/src/gsm_code.cpp			\
	thirdparty/gsm/src/gsm_create.cpp		\
	thirdparty/gsm/src/gsm_debug.cpp		\
	thirdparty/gsm/src/gsm_decode.cpp		\
	thirdparty/gsm/src/gsm_decode2.cpp		\
	thirdparty/gsm/src/gsm_destroy.cpp		\
	thirdparty/gsm/src/gsm_encode.cpp		\
	thirdparty/gsm/src/gsm_explode.cpp		\
	thirdparty/gsm/src/gsm_implode.cpp		\
	thirdparty/gsm/src/gsm_long_term.cpp	\
	thirdparty/gsm/src/gsm_lpc.cpp			\
	thirdparty/gsm/src/gsm_option.cpp		\
	thirdparty/gsm/src/gsm_preprocess.cpp	\
	thirdparty/gsm/src/gsm_print.cpp		\
	thirdparty/gsm/src/gsm_rpe.cpp			\
	thirdparty/gsm/src/gsm_short_term.cpp	\
	thirdparty/gsm/src/gsm_table.cpp

liblpc_a_SOURCES =	\
	thirdparty/lpc/lpccodec.cpp	\
	thirdparty/lpc/lpccodec.h

libjvoip_generic =			\
	jvoiperrors.cpp			\
	jvoiperrors.h			\
	jvoipexception.h		\
	jvoipsession.cpp		\
	jvoipsession.h			\
	jvoipsessionparams.cpp	\
	jvoipsessionparams.h	\
	jvoipsigwait.cpp		\
	jvoipsigwait.h			\
	jvoipthread.cpp			\
	jvoipthread.h			\
	jvoiptypes.h			\
	libcomponents/base/jvoipcompression.cpp			\
	libcomponents/base/jvoipsessionlocalisation.cpp	\
	libcomponents/impl/jvoipdpcmcompression.cpp		\
	libcomponents/impl/jvoipgsmcompression.cpp		\
	libcomponents/impl/jvoiphrtfdata.cpp			\
	libcomponents/impl/jvoiphrtflocalisation.cpp	\
	libcomponents/impl/jvoiplpccompression.cpp		\
	libcomponents/impl/jvoipnoinput.cpp				\
	libcomponents/impl/jvoipnooutput.cpp			\
	libcomponents/impl/jvoipnormalmixer.cpp			\
	libcomponents/impl/jvoiprtptransmission.cpp		\
	libcomponents/impl/jvoipsampleconverter.cpp		\
	libcomponents/impl/jvoipsilencesuppression.cpp	\
	libcomponents/impl/jvoipsimplelocalisation.cpp	\
	libcomponents/impl/jvoipsimpletimer.cpp			\
	libcomponents/impl/jvoipulawcompression.cpp		\
	componentframework/voiceblock.cpp				\
	componentframework/voicecall.cpp

if WIN32
libjvoip_a_SOURCES = $(libjvoip_generic) \
	libcomponents/impl/win32/jvoipmixerstatus.cpp		\
	libcomponents/impl/win32/jvoipsoundcardinput_win32.cpp	\
	libcomponents/impl/win32/jvoipsoundcardoutput_win32.cpp
else
libjvoip_a_SOURCES = $(libjvoip_generic) \
	libcomponents/impl/oss/jvoipsoundcardparams.cpp			\
	libcomponents/impl/oss/jvoipsoundcardinput.cpp			\
	libcomponents/impl/oss/jvoipsoundcardoutput.cpp			\
	libcomponents/impl/oss/jvoipsounddevdrvthread.cpp		\
	libcomponents/impl/oss/jvoipsounddevice.cpp				\
	libcomponents/impl/oss/jvoipsounddevicedriver.cpp
endif

