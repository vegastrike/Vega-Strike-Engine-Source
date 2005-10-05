g++ -o replace ../objconv/replace.cpp
#sed -e 's/__ZNSs6assignEPKcm/__ZNSs6assi6nEPKcm/g' cmd/ai/libai.a >cmd/ai/libai.bak
#sed -e 's/__ZNSs6assignEPKcm/__ZNSs6assi7nEPKcm/g' cmd/libcmd.a >cmd/libcmd.bak#mv cmd/ai/libai.bak cmd/ai/libai.a
#mv cmd/libcmd.bak cmd/libcmd.a
./replace __ZNSs6assignEPKcm __ZNSs6assi6nEPKcm cmd/ai/libai.a cmd/ai/libai.a
#./replace __ZNSs6assignEPKcm __ZNSs6assi7nEPKcm cmd/libcmd.a cmd/libcmd.a
./replace _png_get_uint_31 _png_get_uiNt_31 /Users/daniel/Install/libpng/libpng.a /Users/daniel/Install/libpng/libpng.a 
if grep inflate /Users/daniel/Install/libpng/libpng.a ; then
  nm /Users/daniel/install/zlib/libz.a | grep \[TS\]\  | grep flate | cut -c12- | xargs python replaceall.py /Users/daniel/Install/libpng/libpng.a 
  #first replace user, then replace provider
  nm /Users/daniel/install/zlib/libz.a | grep \[TSts\]\  | cut -c12- | xargs python replaceall.py /Users/daniel/install/zlib/libz.a 
fi
ranlib /Users/daniel/Install/libpng/libpng.a 
ranlib /Users/daniel/install/zlib/libz.a
ranlib cmd/ai/libai.a >&/dev/null
ranlib cmd/libcmd.a >&/dev/null
export MACOSX_DEPLOYMENT_TARGET=10.1
export NEXT_ROOT=/Developer/SDKs/MacOSX10.1.5.sdk/
gcc -Xlinker -Y -Xlinker 16384 -Xlinker -force_flat_namespace  -Xlinker -multiply_defined -Xlinker warning  -o vegastrike debug_vs.o			\
	ship_commands.o \
	command.o \
        macquartz.cpp \
	rendertext.o \
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
aldrv/libaldrv.a common/libvscommon.a networking/libnetclient.a cmd/script/script_call_briefing.o cmd/script/libscript.a cmd/script/c_alike/libc_alike.a python/briefing_wrapper.o cmd/libcmd.a cmd/base_init.o python/libpython.a gfx/nav/libnav.a gfx/libgfx.a cmd/ai/libai.a gldrv/libgldrv.a gui/libgui.a networking/libnet.a networking/lowlevel/libnetlowlevel.a cmd/collide/libcollide.a  boost/libboost_python.a -L/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.1 -lgcc -lpthread   /Developer/SDKs/MacOSX10.1.5.sdk/System/Library/Frameworks/OpenGL.framework/OpenGL /Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib /Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/OpenGL.framework/Libraries/libGLU.dylib -framework GLUT /sw/lib/libexpat.a /sw/lib/libjpeg.a /Users/daniel/Install/libpng/libpng.a   /Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.1/libstdc++.a -framework AppKit /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /users/daniel/Vega/openal/linux/src/libopenal.a   /Users/daniel/Vega/Python-2.2.3/libpython2.2.a  -framework IOKit -framework Foundation -framework CoreFoundation -framework ApplicationServices -framework CoreServices  -framework Carbon -framework SystemConfiguration -framework CoreAudio /users/daniel/install/zlib/libz.a 
#/usr/lib/libz.dylib
#
gcc -Xlinker -Y -Xlinker 16384 -Xlinker -force_flat_namespace  -Xlinker -nomultidefs  -o soundserver.child -DHAVE_SDL=1 -D__APPLE -I.. networking/soundserver.cpp networking/inet.cpp -I. lin_time.cpp networking/inet_file.cpp  -I/sw/include /Users/daniel/Install/SDL_mixer-1.2.5/.libs/libSDL_mixer.a /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /Users/daniel/Install/SDL-1.2.7/src/.libs/libSDL.a  /Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.1/libstdc++.a -framework AppKit   -framework Quicktime -framework AudioUnit -framework AudioToolbox /Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib -framework OpenGL -framework ApplicationServices macquartz.cpp
gcc -Xlinker -Y -Xlinker 16384 -Xlinker -force_flat_namespace -Xlinker -nomultidefs -o soundserver -DHAVE_GETTIMEOFDAY networking/respawning_soundserver.cpp networking/inet.cpp networking/inet_file.cpp lin_time.cpp -I.. -I. -Inetworking /Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.1/libstdc++.a
cp vegastrike /Users/daniel/Vega/Priv-Package/priv,exe.app/Contents/MacOS/vegastrike
#cp soundserver /Users/daniel/Vega/Priv-Package/priv,exe.app/Contents/Resources/bin/soundserver
cp soundserver.child /Users/daniel/Vega/Priv-Package/priv,exe.app/Contents/Resources/bin/soundserver
#/sw/lib/python2.2/config/libpython2.2.a 
#/usr/lib/dyld
#
#/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/dylib1.o 
