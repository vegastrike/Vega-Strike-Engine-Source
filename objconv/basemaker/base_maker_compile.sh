#!/bin/sh

g++ base_maker.cpp -c -I../.. -I../../src -I../../src/boost129 -I/usr/include/python2.3 -g && \
g++ base_maker_texture.cpp -c -g -I../../src && \
g++ -o base_maker base_maker.o -lGL -lGLU -lSDL -lglut base_maker_texture.o -ljpeg -lpng -g -lpthread -L/usr/X11R6/lib -lXi -lXmu

