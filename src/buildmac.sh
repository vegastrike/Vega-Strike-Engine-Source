g++    -pipe -O2  -Wall  -ffast-math -fomit-frame-pointer -fexpensive-optimizations    -o vegastrike 	debug_vs.o	main.o		\
	gfxlib_struct.o		\
	in_joystick.o			\
	force_feedback.o		\
	faction_util.o		\
	in_kb.o				\
	in_sdl.o				\
	in_mouse.o			\
	in_main.o				\
	in_handler.o			\
	main_loop.o			\
	physics.o				\
	star_system_jump.o	\
	star_system.o 		\
	universe.o			\
	universe_util.o		\
	config_xml.o			\
    macosx_math.o			\
	cg_global.o                   \
	posh.o						\
	stardate.o				\
	star_system_xml.o		\
	star_system_generic.o 	\
	universe_generic.o		\
	universe_util_generic.o 	\
	galaxy.o					\
	galaxy_xml.o				\
	galaxy_gen.o				\
	faction_generic.o			\
	hashtable.o				\
	configxml.o				\
	easydom.o					\
	xml_serializer.o			\
	xml_support.o				\
	lin_time.o				\
	endianness.o				\
	faction_util_generic.o	\
	load_mission.o			\
	savegame.o	pk3.o			\
	vs_globals.o	vsfilesystem.o			\
	cmd/planet.o cmd/planet_generic.o cmd/ai/firekeyboard.o			\
aldrv/libaldrv.a common/libvscommon.a networking/libnetclient.a networking/lowlevel/libnetlowlevel.a cmd/script/script_call_briefing.o cmd/script/libscript.a cmd/script/c_alike/libc_alike.a python/briefing_wrapper.o cmd/libcmd.a cmd/base_init.o python/libpython.a gfx/nav/libnav.a gfx/libgfx.a cmd/ai/libai.a gldrv/libgldrv.a gui/libgui.a networking/libnet.a cmd/collide/libcollide.a boost129/libboost_python.a boost/libboost_python.a -L/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/ -F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks -lpthread  /Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/libobjc.dylib -L/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/OpenGL.framework/Libraries -l GL -l GLU  -framework GLUT /sw/lib/libexpat.a /sw/lib/libjpeg.a /sw/lib/python2.2/config/libpython2.2.a   /sw/lib/libopenal.a -framework Carbon -framework CoreAudio -framework IOKit /Users/daniel/Install/libpng/libpng.a /Users/daniel/Install/zlib/libz.a -framework AppKit -framework Foundation -framework OpenGL -framework Quicktime -framework AudioUnit -framework AudioToolbox  -framework System

#-framework IOKit -framework AudioUnit -framework AudioToolbox  -framework OpenGL -framework Foundation -framework AppKit /sw/lib/libpng.a -framework Quicktime

#/users/daniel/install/zlib/libz.a -s 

