bin_PROGRAMS = vegastrike vegaserver accountserver
SUBDIRS = cmd gfx gldrv aldrv python gui networking boost common
vegastrike_LDADD = cmd/script/libscript.a cmd/libcmd.a cmd/ai/libai.a cmd/script/c_alike/libc_alike.a gfx/libgfx.a gldrv/libgldrv.a aldrv/libaldrv.a python/libpython.a cmd/collide/libcollide.a gui/libgui.a -lutil networking/libnet.a boost/libboost_python.a common/libvscommon.a networking/libnetclient.a -lpthread

vegaserver_LDADD = networking/libnetserver.a
#cmd/unit.o cmd/unit_xml.o cmd/unit_cargo.o cmd/unit_factory.o cmd/unit_interface.o cmd/container.o cmd/unit_collide.o \
#gfx/animation.o gfx/mesh.o gfx/vsbox.o
accountserver_LDADD = networking/libnetserver.a
INCLUDES = -Iboost/
#/usr/lib/python2.2/config/libpython2.2.a 
vegaserver_SOURCES =				\
	hashtable.h				\
	hashtable.cpp			\
	configxml.cpp			\
	configxml.h			\
	xml_support.h  			\
	xml_support.cpp			\
	easydom.cpp				\
	easydom.h				\
	lin_time.cpp			\
	lin_time.h				\
	vegaserver.cpp

accountserver_SOURCES =				\
	hashtable.h				\
	hashtable.cpp			\
	configxml.cpp			\
	configxml.h			\
	xml_support.h  			\
	xml_support.cpp			\
	easydom.cpp				\
	easydom.h				\
	lin_time.cpp			\
	lin_time.h				\
	accountserver.cpp

vegastrike_SOURCES =				\
	debug_vs.h				\
	debug_vs.cpp				\
	endianness.h				\
	file_main.h				\
	galaxy.cpp				\
	galaxy_xml.cpp				\
	galaxy_xml.h				\
	galaxy_gen.cpp				\
	galaxy_gen.h				\
	gfxlib.h				\
	gfxlib_struct.h				\
	hashtable.h				\
	hashtable.cpp				\
	linecollide.h				\
	in.h					\
	in_joystick.cpp				\
	in_joystick.h				\
	force_feedback.cpp			\
	force_feedback.h			\
	in_kb.cpp				\
	in_kb.h					\
	in_sdl.cpp				\
	in_mouse.cpp				\
	in_mouse.h				\
	in_main.cpp				\
	in_main.h				\
	in_handler.cpp				\
	in_handler.h				\
	lin_time.h				\
	lin_time.cpp				\
	load_mission.cpp			\
	main.cpp				\
	main_loop.h				\
	main_loop.cpp				\
	savegame.cpp				\
	savegame.h				\
	physics.h				\
	physics.cpp				\
	resizable.h				\
	star_system.cpp 			\
	star_system_jump.cpp			\
	star_system_xml.cpp			\
	star_system.h 				\
	universe.cpp				\
	universe.h				\
	universe_util.h				\
	universe_util.cpp				\
	xml_support.h  				\
	xml_support.cpp				\
	xml_serializer.cpp			\
	xml_serializer.h			\
	config_xml.cpp				\
	config_xml.h				\
	easydom.cpp				\
	easydom.h				\
	profile.h 				\
	vegastrike.h				\
	vs_globals.cpp 				\
	vs_globals.h 				\
	vs_path.cpp				\
	vs_path.h				\
	audiolib.h				\
        macosx_math.cpp				\
        macosx_math.h


