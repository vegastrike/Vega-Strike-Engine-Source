noinst_LIBRARIES = libpython.a libpythonserver.a

INCLUDES = -I ../ -I../boost  -I/usr/include/python2.2

libpythongeneric =				\
	init.cpp \
	init.h	\
	python_class.h	\
	python_compile.h	\
	python_compile.cpp	\
	unit_wrapper.cpp

libpythonserver_a_SOURCES =$(libpythongeneric)

libpython_a_SOURCES =$(libpythongeneric)				\
	briefing_wrapper.cpp	

