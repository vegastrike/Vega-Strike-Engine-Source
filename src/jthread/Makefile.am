if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libjthread.a

if WIN32
libjthread_a_SOURCES =	\
	jthread.h			\
	jmutex.h			\
	jthread_win32.cpp	\
	jmutex_win32.cpp
else
libjthread_a_SOURCES =	\
	jthread.h			\
	jmutex.h			\
	jthread_pthread.cpp	\
	jmutex_pthread.cpp
endif
