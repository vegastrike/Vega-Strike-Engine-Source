MAIN=libai.a

CC=g++


CFLAGS= -DUNIX -Wall -g  -DHAVE_FINITE -DHAVE_GETTIMEOFDAY

INCLUDES= -I/usr/X11R6/include -I ../../

LFLAGS= 

SRCS =		\
 	event_xml.cpp	\
	navigation.cpp	\
	flykeyboard.cpp	\
	firekeyboard.cpp \
	fire.cpp	\
	order.cpp	\
	aggressive.cpp	\
	input_dfa.cpp	\
	flybywire.cpp	\
	script.cpp


OBJS = $(SRCS:.cpp=.o)

$(MAIN):  $(OBJS)
	ar cru $(MAIN) $(OBJS) 
	ranlib $(MAIN)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o *~ $(MAIN)
