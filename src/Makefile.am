
bin_PROGRAMS = vegastrike vegaserver accountserver

SUBDIRS = \
    cmd gfx gldrv aldrv python gui networking boost common boost129

vegastrike_LDADD =  				\
networking/netserver_devices.o		\
networking/libnetclient.a 			\
networking/lowlevel/libnetlowlevel.a \
cmd/script/script_call_briefing.o 	\
cmd/script/libscript.a 				\
cmd/script/c_alike/libc_alike.a 	\
python/briefing_wrapper.o 			\
cmd/libcmd.a 						\
cmd/base_init.o 					\
python/libpython.a 					\
gfx/libgfx.a 						\
gldrv/libgldrv.a 					\
cmd/ai/libai.a 						\
gui/libgui.a 						\
networking/libnet.a 				\
cmd/collide/libcollide.a 			\
boost129/libboost_python.a          \
boost/libboost_python.a             \
gfx/nav/libnav.a					\
aldrv/libaldrv.a 					\
-lz

vegaserver_LDADD =  			\
networking/netserver_devices.o	\
cmd/script/director_server.o 	\
cmd/unit_util_generic.o 		\
cmd/unit_util_server.o 			\
networking/netclient.o 			\
networking/libnetserver.a 		\
networking/lowlevel/libnetlowlevel.a \
cmd/libcmdserver.a 				\
cmd/script/libscriptserver.a 	\
cmd/ai/libaiserver.a 			\
cmd/script/c_alike/libc_alike.a \
python/libpythonserver.a 		\
gfx/libgfxserver.a 				\
gldrv/libgldrvserver.a 			\
cmd/collide/libcollide.a 		\
boost129/libboost_python.a      \
boost/libboost_python.a	        \
-lz

accountserver_LDADD =           \
networking/libnetacct.a 		\
networking/lowlevel/libnetlowlevel.a \
-lz


libroot =		\
	debug_vs.h				\
	debug_vs.cpp			\
	gfxlib.h				\
	gfxlib_struct.h			\
	gfxlib_struct.cpp		\
	linecollide.h			\
	in_joystick.cpp			\
	in_joystick.h			\
	force_feedback.cpp		\
	force_feedback.h		\
	faction_util.cpp		\
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
    macosx_math.h			\
	cg_global.cpp

librootgeneric =				\
	vsfilesystem.cpp			\
	vsfilesystem.h				\
	posh.cpp					\
	posh.h						\
	stardate.cpp				\
	stardate.h					\
	star_system_xml.cpp			\
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
	xml_serializer.cpp			\
	xml_serializer.h			\
	xml_support.h  				\
	xml_support.cpp				\
	lin_time.cpp				\
	lin_time.h					\
	endianness.cpp				\
	endianness.h				\
	file_main.h					\
	in.h						\
	faction_util_generic.cpp	\
	load_mission.cpp			\
	savegame.cpp				\
	savegame.h					\
	vegastrike.h				\
	pk3.cpp						\
	pk3.h						\
	vs_globals.cpp

librootserver =					\
	gfxlib_struct_server.cpp	\
	libaudioserver.cpp			\
	libserver.cpp				\
	universe_util_server.cpp	\
	force_feedback_server.cpp	\
	faction_util_server.cpp

vegaserver_SOURCES =					\
	$(librootgeneric) $(librootserver)	\
	vegaserver.cpp

accountserver_SOURCES =		\
	vsfilesystem.cpp		\
	vsfilesystem.h			\
	pk3.cpp					\
	pk3.h					\
	posh.cpp				\
	libserver.cpp			\
	configxml.cpp			\
	configxml.h				\
	easydom.cpp				\
	easydom.h				\
	xml_support.h  			\
	xml_support.cpp			\
	lin_time.cpp			\
	lin_time.h				\
	endianness.cpp			\
	endianness.h			\
	accountserver.cpp

#	galaxy_gen.cpp
#	galaxy_gen.h

vegastrike_SOURCES =		\
	$(librootgeneric) $(libroot) \
	main.cpp

