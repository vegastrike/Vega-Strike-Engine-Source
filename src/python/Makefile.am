noinst_LIBRARIES = libpython.a libpythonserver.a

INCLUDES = -I ../  -I/usr/include/python2.2

libpythongeneric =				\
	init.cpp \
	init.h	\
	python_class.h	\
	python_compile.h	\
	python_compile.cpp	\
	unit_wrapper.cpp	\
	unit_exports.cpp	\
	unit_exports1.cpp	\
	unit_exports2.cpp	\
	unit_exports3.cpp	\
	unit_method_defs.cpp	\
	universe_util_exports.cpp	\
	unit_wrapper_class.h	\
	unit_exports.h		\
	star_systeM_exports.h

libpythonserver_a_SOURCES =$(libpythongeneric)

libpython_a_SOURCES =$(libpythongeneric)				\
	briefing_wrapper.cpp	

