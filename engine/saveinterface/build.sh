#!/bin/sh
#thank you Thunderbird@#nvidia
g++ -I/usr/lib/glib/include/ -I/usr/include/gtk-1.2 -I/usr/include/gtk-2.0 -I/usr/include/glib-1.2 -I/usr/include/glib-2.0 -olauncher saveinterface.cpp -lgtk -lglib -lgdk -g
