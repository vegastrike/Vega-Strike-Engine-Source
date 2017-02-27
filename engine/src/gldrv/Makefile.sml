MAIN=libgldrv.a

CC=g++


CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I ../

LFLAGS= 

SRCS = 	gl_globals.cpp				\
	gl_init.cpp				\
	gl_light.cpp				\
	gl_light_state.cpp			\
	gl_light_pick.cpp			\
	gl_matrix.cpp				\
	gl_material.cpp				\
	gl_misc.cpp				\
	gl_quad_list.cpp			\
	gl_state.cpp				\
	gl_texture.cpp				\
	gl_vertex_list.cpp			


OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	ar cru libgldrv.a $(OBJS) 
	ranlib libgldrv.a
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
