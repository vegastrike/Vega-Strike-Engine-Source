#!/usr/bin/perl

$root = "./";
$name = "Vega Strike Mission Selector";

$defines = "-D_G_GLOB -D_G_STRING_PARSE -D_G_STRING_MANAGE -D_G_ERROR";
#$defines = "-D_G_ALL";

$exec_bin = "g++";
$exec_include = "-Wall -O3 `gtk-config --cflags` $defines -g";
$exec_compile = "-O3 `gtk-config --libs` -lexpat $defines";

$do_c = 1;
$do_cpp = 1;

@BUILD = (
	{
		"name" => "Vega Strike Mission Selector",
		"include" => $root."include/",
		"src" => $root."c/",
		"build" => $root."./",
		"install" => $root."./"
	}
);
