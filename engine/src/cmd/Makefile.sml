MAIN=libcmd.a

CC=g++

SUBDIRS = ai

CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I ../

LFLAGS=  -L/usr/X11R6/lib -lXi -lXmu -lGL -lGLU -lglut -lexpat 

LTWOFLAGS = -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread -lSDL_mixer  -lGL -lGLU  -lglut  -lexpat

SRCS = 	beam.cpp				\
	bolt.cpp				\
	unit_collide.cpp				\
	faction_xml.cpp			\
	hud.cpp				\
	click_list.cpp			\
	unit.cpp				\
	unit_bsp.cpp			\
	unit_click.cpp			\
	unit_damage.cpp			\
	unit_xml.cpp  			\
	unit_physics.cpp				\
	weapon_xml.cpp			\
	planet.cpp 				\
	collection.cpp			\
	container.cpp			


OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	for dir in $(SUBDIRS); do \
                (cd $$dir &&  $(MAKE) -f Makefile.sml); \
        done
	ar cru $(MAIN) $(OBJS) ai/libai.a
	ranlib $(MAIN)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	for dir in $(SUBDIRS); do \
                (cd $$dir && $(MAKE) clean); \
        done
	rm -f *.o *~ $(MAIN)
