#SUBDIRS = gldrv

bindir = $(prefix)/games/@PACKAGE@/bin

bin_PROGRAMS = vegastrike
SUBDIRS = cmd gfx gldrv aldrv
vegastrike_LDADD = cmd/libcmd.a cmd/ai/libai.a gfx/libgfx.a gldrv/libgldrv.a aldrv/libaldrv.a

vegastrike_SOURCES =				\
	endianness.h				\
	file_main.h				\
	gfxlib.h				\
	gfxlib_struct.h				\
	hashtable.h				\
	hashtable.cpp				\
	hashtable_3d.h				\
	linecollide.h				\
	in.h					\
	in_joystick.cpp				\
	in_joystick.h				\
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
	main.cpp				\
	main_loop.h				\
	main_loop.cpp				\
	physics.h				\
	physics.cpp				\
	star_system.cpp 			\
	star_system_xml.cpp			\
	star_system.h 				\
	universe.cpp				\
	universe.h				\
	xml_support.h  				\
	xml_support.cpp				\
	vegaconfig.cpp				\
	vegaconfig.h				\
	easydom.cpp				\
	easydom.h				\
	profile.h 				\
	vegastrike.h				\
	vs_globals.h 				

