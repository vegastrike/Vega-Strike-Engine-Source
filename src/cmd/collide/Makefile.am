if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libcollide.a
SUBDIRS = csgeom 
libcollide_a_SOURCES =				\
	prapid.cpp			\
	collider.h			\
	cs_compat.h			\
	garray.h			\
	qint.h				\
	qsqrt.h				\
	pbuild.cpp			\
	peigen.cpp			\
	prapid.h			\
	rapcol.h			\
	box.cpp				\
	matrix3.cpp			\
	vector3.cpp			

