gcc -Xlinker -force_flat_namespace  -Xlinker -nomultidefs  -o vegastrike debug_vs.o			\
	gfxlib_struct.o		\
	in_joystick.o			\
	force_feedback.o		\
	faction_util.o		\
	in_kb.o				\
	in_sdl.o				\
	in_mouse.o			\
	in_main.o				\
	main.o				\
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
	savegame.o				\
	vs_globals.o				\
	vsfilesystem.o					\
	pk3.o					\
aldrv/libaldrv.a common/libvscommon.a networking/libnetclient.a cmd/script/script_call_briefing.o cmd/script/libscript.a cmd/script/c_alike/libc_alike.a python/briefing_wrapper.o cmd/libcmd.a cmd/base_init.o python/libpython.a gfx/nav/libnav.a gfx/libgfx.a cmd/ai/libai.a gldrv/libgldrv.a gui/libgui.a networking/libnet.a networking/lowlevel/libnetlowlevel.a cmd/collide/libcollide.a  boost/libboost_python.a  -lpthread   -framework OpenGL -framework GLUT /sw/lib/libexpat.a /sw/lib/libjpeg.a /Users/daniel/Install/libpng/libpng.a /users/daniel/install/zlib/libz.a  /usr/lib/gcc/darwin/3.1/libstdc++.a -framework AppKit /users/daniel/Vega/openal/linux/src/libopenal.a  -ldl /Users/daniel/Vega/Python-2.2.3/libpython2.2.a  -framework IOKit -framework Foundation -framework CoreFoundation -framework ApplicationServices -framework CoreServices  -framework Carbon -lcups -lbsm -framework SystemConfiguration -framework CoreAudio

gcc -Xlinker -force_flat_namespace  -Xlinker -nomultidefs  -o soundserver -DHAVE_SDL=1 -D__APPLE -I.. networking/soundserver.cpp networking/inet.cpp  -I/sw/include /Users/daniel/Install/SDL_mixer-1.2.5/.libs/libSDL_mixer.a /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /Users/daniel/Install/SDL-1.2.7/src/.libs/libSDL.a  /usr/lib/gcc/darwin/3.1/libstdc++.a -framework AppKit -framework OpenGL  -framework Quicktime

#/sw/lib/python2.2/config/libpython2.2.a 
#/usr/lib/dyld
#
#/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/dylib1.o 
