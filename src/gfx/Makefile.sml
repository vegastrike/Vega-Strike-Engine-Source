MAIN=libgfx.a

CC=g++


CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I ../

LFLAGS= 

SRCS =		\
	animation.cpp	\
	aux_logo.cpp	\
	aux_palette.cpp	\
	aux_texture.cpp	\
	background.cpp	\
	bounding_box.cpp	\
	box.cpp		\
	bsp.cpp		\
	bsp_gent.cpp	\
	camera.cpp	\
	coord_select.cpp	\
	env_map_gent.cpp	\
	halo.cpp	\
	lerp.cpp	\
	loc_select.cpp	\
	mesh.cpp	\
	mesh_bin.cpp	\
	mesh_fx.cpp	\
	mesh_poly.cpp	\
	mesh_xml.cpp	\
	primitive.cpp	\
	primitive.h	\
	sphere.cpp	\
	sprite.cpp	\
	matrix.cpp	\
	vec.cpp	\
	quaternion.cpp


OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	ar cru $(MAIN) $(OBJS) 
	ranlib $(MAIN)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
