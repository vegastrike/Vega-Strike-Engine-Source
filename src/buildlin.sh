gcc -Xlinker -export-dynamic -Xlinker   -ovegastrike debug_vs.o			\
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
aldrv/libaldrv.a gfx/nav/libnav.a common/libvscommon.a networking/libnetclient.a cmd/script/script_call_briefing.o cmd/script/libscript.a cmd/script/c_alike/libc_alike.a python/briefing_wrapper.o cmd/libcmd.a cmd/base_init.o python/libpython.a gfx/libgfx.a cmd/ai/libai.a gldrv/libgldrv.a gui/libgui.a networking/libnet.a networking/lowlevel/libnetlowlevel.a  cmd/collide/libcollide.a  boost/libboost_python.a    /usr/lib/libglut.a /usr/lib/libexpat.a /usr/lib/libjpeg.a /usr/lib/python2.2/config/libpython2.2.a /usr/lib/libpng.a /usr/lib/libz.a  /usr/lib/libSDL.a /usr/lib/libopenal.a /usr/lib/libGLU.a /usr/lib/libutil.a /usr/lib/gcc-lib/i386-linux/2.95.4/libstdc++.a /usr/lib/libGL.so -ldl -lpthread  /usr/lib/gcc-lib/i386-linux/2.95.4/libgcc.a 
gcc -DHAVE_SDL -osoundserver networking/soundserver.cpp networking/inet.cpp networking/inet_file.cpp -I.. -I. -Inetworking  /usr/lib/libSDL_mixer.a /usr/lib/libSDL.a /usr/lib/libsmpeg.a  /usr/lib/libvorbisfile.a /usr/lib/libvorbis.a  /usr/lib/libogg.a -lpthread -lm -lX11 -lXext -ldl /usr/lib/gcc-lib/i386-linux/2.95.4/libstdc++.a /usr/lib/gcc-lib/i386-linux/2.95.4/libgcc.a

#/usr/lib/gcc-lib/i386-linux/2.95.4/libstdc++.a  /usr/lib/libdl.a   -lpthread
# /usr/lib/gcc-lib/i386-linux/2.95.4/crtbegin.o /usr/lib/gcc-lib/i386-linux/2.95.4/crtend.o 
#-nostartfiles -nostdlib