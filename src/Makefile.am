bin_PROGRAMS = vegastrike vegaserver accountserver
noinst_LIBRARIES = libroot.a librootgeneric.a librootserver.a

SUBDIRS = cmd gfx gldrv aldrv python gui networking boost common
vegastrike_LDADD =  \
aldrv/libaldrv.a \
common/libvscommon.a \
networking/libnetclient.a \
cmd/script/script_call_briefing.o \
cmd/script/libscript.a cmd/script/libscriptgeneric.a \
cmd/script/c_alike/libc_alike.a \
python/briefing_wrapper.o \
cmd/libcmd.a cmd/libcmdgeneric.a \
cmd/base_init.o \
cmd/unit_util.o \
python/libpython.a python/libpythongeneric.a \
boost/libboost_python.a \
gfx/libgfx.a gfx/libgfxgeneric.a \
cmd/ai/libaigeneric.a cmd/ai/libai.a \
gldrv/libgldrv.a \
gui/libgui.a \
networking/libnet.a \
cmd/collide/libcollide.a \
-lpthread 

vegaserver_LDADD =  \
networking/libnetserver.a \
cmd/libcmdserver.a cmd/libcmdgeneric.a \
cmd/ai/libaigeneric.a \
cmd/script/libscriptserver.a cmd/script/libscriptgeneric.a \
cmd/script/c_alike/libc_alike.a \
gfx/libgfxgeneric.a \
python/libpython.a python/libpythongeneric.a \
boost/libboost_python.a \
librootserver.a librootgeneric.a 




accountserver_LDADD =  \
networking/libnetserver.a

INCLUDES = -Iboost/

libroot_a_SOURCES =			\
	debug_vs.h				\
	debug_vs.cpp			\
	gfxlib.h				\
	gfxlib_struct.h			\
	linecollide.h			\
	in_joystick.cpp			\
	in_joystick.h			\
	faction.cpp				\
	faction.h				\
	force_feedback.cpp		\
	force_feedback.h		\
	in_kb.cpp				\
	in_kb.h					\
	in_sdl.cpp				\
	in_mouse.cpp			\
	in_mouse.h				\
	in_main.cpp				\
	in_main.h				\
	in_handler.cpp			\
	in_handler.h			\
	main_loop.h				\
	main_loop.cpp			\
	physics.h				\
	physics.cpp				\
	resizable.h				\
	star_system_jump.cpp	\
	star_system_xml.cpp		\
	star_system.cpp 		\
	star_system.h 			\
	universe.cpp			\
	universe.h				\
	universe_util.h			\
	universe_util.cpp		\
	config_xml.cpp			\
	config_xml.h			\
	profile.h 				\
	audiolib.h				\
    macosx_math.cpp			\
    macosx_math.h

librootgeneric_a_SOURCES =			\
	star_system_generic.cpp 	\
	star_system_generic.h 		\
	universe_generic.cpp		\
	universe_generic.h			\
	universe_util_generic.cpp 	\
	universe_util_generic.h 	\
	galaxy.cpp					\
	galaxy_xml.cpp				\
	galaxy_xml.h				\
	galaxy_gen.cpp				\
	galaxy_gen.h				\
	faction_generic.cpp			\
	faction_generic.h			\
	hashtable.h					\
	hashtable.cpp				\
	configxml.cpp				\
	configxml.h					\
	easydom.cpp					\
	easydom.h					\
	xml_serializer.cpp		\
	xml_serializer.h		\
	xml_support.h  			\
	xml_support.cpp			\
	lin_time.cpp				\
	lin_time.h					\
	endianness.cpp			\
	endianness.h			\
	file_main.h				\
	in.h					\
	faction_util.cpp		\
	load_mission.cpp		\
	savegame.cpp			\
	savegame.h				\
	universe_generic.cpp	\
	universe_generic.h		\
	universe_util_generic.h	\
	universe_util_generic.cpp	\
	star_system_generic.cpp \
	star_system_generic.h 	\
	vegastrike.h			\
	vs_path.cpp				\
	vs_path.h

librootserver_a_SOURCES =			\
	libserver.cpp				\
	faction_util_server.cpp

vegaserver_SOURCES =			\
	vs_globals.cpp			\
	vegaserver.cpp

accountserver_SOURCES =		\
	configxml.cpp				\
	configxml.h					\
	easydom.cpp					\
	easydom.h					\
	xml_support.h  			\
	xml_support.cpp			\
	lin_time.cpp				\
	lin_time.h					\
	vs_globals.cpp			\
	accountserver.cpp

vegastrike_SOURCES =		\
	vs_globals.cpp			\
	main.cpp	\
		\
	debug_vs.h				\
	debug_vs.cpp			\
	gfxlib.h				\
	gfxlib_struct.h			\
	linecollide.h			\
	in_joystick.cpp			\
	in_joystick.h			\
	faction.cpp				\
	faction.h				\
	force_feedback.cpp		\
	force_feedback.h		\
	in_kb.cpp				\
	in_kb.h					\
	in_sdl.cpp				\
	in_mouse.cpp			\
	in_mouse.h				\
	in_main.cpp				\
	in_main.h				\
	in_handler.cpp			\
	in_handler.h			\
	main_loop.h				\
	main_loop.cpp			\
	physics.h				\
	physics.cpp				\
	resizable.h				\
	star_system_jump.cpp	\
	star_system_xml.cpp		\
	star_system.cpp 		\
	star_system.h 			\
	universe.cpp			\
	universe.h				\
	universe_util.h			\
	universe_util.cpp		\
	config_xml.cpp			\
	config_xml.h			\
	profile.h 				\
	audiolib.h				\
    macosx_math.cpp			\
    macosx_math.h	\
	star_system_generic.cpp 	\
	star_system_generic.h 		\
	universe_generic.cpp		\
	universe_generic.h			\
	universe_util_generic.cpp 	\
	universe_util_generic.h 	\
	galaxy.cpp					\
	galaxy_xml.cpp				\
	galaxy_xml.h				\
	galaxy_gen.cpp				\
	galaxy_gen.h				\
	faction_generic.cpp			\
	faction_generic.h			\
	hashtable.h					\
	hashtable.cpp				\
	configxml.cpp				\
	configxml.h					\
	easydom.cpp					\
	easydom.h					\
	xml_serializer.cpp		\
	xml_serializer.h		\
	xml_support.h  			\
	xml_support.cpp			\
	lin_time.cpp				\
	lin_time.h					\
	endianness.cpp			\
	endianness.h			\
	file_main.h				\
	in.h					\
	faction_util.cpp		\
	load_mission.cpp		\
	savegame.cpp			\
	savegame.h				\
	vegastrike.h			\
	vs_path.cpp				\
	vs_path.h


