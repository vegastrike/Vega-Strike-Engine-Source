MAIN=vegastrike

CC=g++

SUBDIRS = cmd gldrv gfx

CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I.

LFLAGS=  -L/usr/X11R6/lib -lXi -lXmu -lGL -lGLU -lglut -lexpat 

LTWOFLAGS = -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -lSDL_mixer  -lGL -lGLU  -lglut  -lexpat

SRCS = 	star_system_xml.cpp			\
	hashtable.cpp				\
	in_kb.cpp				\
	in_mouse.cpp				\
	in_main.cpp				\
	in_handler.cpp				\
	lin_time.cpp				\
	main.cpp				\
	main_loop.cpp				\
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
	-$(CC) $(CFLAGS) $(OBJS) gldrv/gl_quad_list.o gldrv/libgldrv.a gfx/libgfx.a -o $(MAIN) $(LFLAGS)
	-$(CC) $(CFLAGS) $(OBJS) gldrv/gl_quad_list.o gldrv/libgldrv.a gfx/libgfx.a -o $(MAIN) $(LTWOFLAGS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	for dir in $(SUBDIRS); do \
                (cd $$dir && $(MAKE) clean); \
        done
	rm -f *.o *~ $(MAIN)
