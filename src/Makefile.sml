MAIN=vegastrike

CC=g++

SUBDIRS = gldrv gfx

CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I.

LFLAGS=  -L/usr/X11R6/lib -lXi -lXmu -lGL -lGLU -lglut -lexpat 

LTWOFLAGS = -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -lSDL_mixer  -lGL -lGLU  -lglut  -lexpat

SRCS = 	cmd_beam.cpp				\
	cmd_bolt.cpp				\
	cmd_aggressive_ai.cpp			\
	cmd_fireat.cpp				\
	star_system_xml.cpp			\
	cmd_navigation_orders.cpp		\
	cmd_collide.cpp				\
	cmd_faction_xml.cpp			\
	cmd_aiscript.cpp			\
	cmd_input_dfa.cpp			\
	cmd_flybywire.cpp			\
	cmd_hud.cpp				\
	cmd_click_list.cpp			\
	cmd_order.cpp				\
	cmd_unit.cpp				\
	cmd_unit_click.cpp			\
	cmd_unit_damage.cpp			\
	cmd_unit_xml.cpp  			\
	cmd_physics.cpp				\
	cmd_weapon_xml.cpp			\
	cmd_click_list.cpp			\
	hashtable.cpp				\
	in_ai.cpp				\
	in_kb.cpp				\
	in_mouse.cpp				\
	in_main.cpp				\
	in_handler.cpp				\
	lin_time.cpp				\
	main.cpp				\
	main_loop.cpp				\
	planet.cpp 				\
	physics.cpp				\
	star_system.cpp 			\
	UnitCollection.cpp			\
	UnitContainer.cpp			\
	universe.cpp				\
	xml_support.cpp				


OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	for dir in $(SUBDIRS); do \
                (cd $$dir &&  $(MAKE) -f Makefile.sml); \
        done
	-$(CC) $(CFLAGS) $(OBJS) gldrv/libgldrv.a gfx/libgfx.a -o $(MAIN) $(LFLAGS)
	-$(CC) $(CFLAGS) $(OBJS) gldrv/libgldrv.a gfx/libgfx.a -o $(MAIN) $(LTWOFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	for dir in $(SUBDIRS); do \
                (cd $$dir && $(MAKE) clean); \
        done
	rm -f *.o *~ $(MAIN)
