if USEBOOST128
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost
else
INCLUDES=-I$(top_srcdir)/src \
	 -I$(top_srcdir)/src/boost129
endif

noinst_LIBRARIES = libscript.a libscriptserver.a
SUBDIRS = c_alike

libscriptgeneric =			\
	mission.cpp					\
	mission.h					\
	mission_script.cpp			\
	pythonmission.cpp			\
	pythonmission.h             \
	director_generic.cpp			\
	script_call_olist.cpp			\
	script_call_omap.cpp			\
	script_call_order.cpp			\
	script_call_string.cpp			\
	script_variables.cpp			\
	script_util.cpp					\
	script_statement.cpp			\
	script_expression.cpp			\
	script_callbacks.cpp			\
	script_generic.cpp				\
	msgcenter.cpp					\
	msgcenter.h						\
	script_call_unit_generic.cpp

libscriptserver_a_SOURCES =	$(libscriptgeneric)		\
	flightgroup_server.cpp		\
	flightgroup.h				\
	director_server.cpp			\
	script_call_unit_server.cpp

libscript_a_SOURCES = $(libscriptgeneric) \
	flightgroup.cpp					\
	flightgroup.h					\
	director.cpp					\
	script_call_briefing.cpp		\
	script_call_unit.cpp

