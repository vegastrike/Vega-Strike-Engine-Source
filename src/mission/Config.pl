#!/usr/bin/perl

$root = "/home/reliant/vsmission/";
$name = "New GUI";

$exec_include = "-Wall -O3 -g -D_G_PATH -D_G_STRING_PARSE -D_G_STRING_MANAGE -D_G_ERROR";
$exec_compile = "-O3 -L/usr/X11R6/lib -lGL -lGLU -lpng -lglut -g -lexpat";

$exec_bin = "g++";
#$exec_bin = "i586-mingw32msvc-c++";

@BUILD = (
	{
		"name" => "New GUI",
		"include" => $root."include/",
		"src" => $root."c/",
		"build" => $root."bin/",
		"install" => $root
	}
);
