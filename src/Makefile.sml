MAIN=vegastrike

CC=g++

SUBDIRS = gldrv

CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include 

LFLAGS=  -L/usr/X11R6/lib -lXi -lXmu -lGL -lGLU -lglut -lexpat 

LTWOFLAGS = -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -lSDL_mixer  -lGL -lGLU  -lglut  -lexpat

SRCS = 	cmd_beam.cpp				\
	cmd_bolt.cpp				\
	star_system_xml.cpp			\
	cmd_navigation_orders.cpp		\
	cmd_collide.cpp				\
	cmd_faction_xml.cpp			\
	cmd_aiscript.cpp			\
	cmd_input_dfa.cpp			\
	cmd_flybywire.cpp			\
	cmd_order.cpp				\
	cmd_unit.cpp				\
	cmd_unit_click.cpp			\
	cmd_unit_damage.cpp			\
	cmd_unit_xml.cpp  			\
	cmd_physics.cpp				\
	cmd_weapon_xml.cpp			\
	gfx_animation.cpp			\
	gfx_aux_logo.cpp			\
	gfx_aux_palette.cpp			\
	gfx_aux_texture.cpp			\
	gfx_background.cpp			\
	gfx_bounding_box.cpp			\
	gfx_box.cpp 				\
	gfx_bsp.cpp				\
	gfx_bsp_gent.cpp			\
	gfx_camera.cpp				\
	gfx_click_list.cpp			\
	gfx_env_map_gent.cpp			\
	gfx_coordinate_select.cpp		\
	gfx_halo.cpp				\
	gfx_hud.cpp				\
	gfx_lerp.cpp 				\
	gfx_location_select.cpp			\
	gfx_mesh.cpp				\
	gfx_mesh_fx.cpp				\
	gfx_mesh_bin.cpp			\
	gfx_mesh_xml.cpp 			\
	gfx_mesh_poly.cpp			\
	gfx_primitive.cpp			\
	gfx_sprite.cpp				\
	gfx_transform_vector.cpp		\
	gfx_transform_matrix.cpp		\
	gfx_sphere.cpp				\
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
	quaternion.cpp				\
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
	-$(CC) $(CFLAGS) $(OBJS) gldrv/libgldrv.a -o $(MAIN) $(LFLAGS)
	-$(CC) $(CFLAGS) $(OBJS) gldrv/libgldrv.a -o $(MAIN) $(LTWOFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	for dir in $(SUBDIRS); do \
                (cd $$dir && $(MAKE) clean); \
        done
	rm -f *.o *~ $(MAIN)
