#!/usr/bin/perl

$USE_GTK = 1;

$root = "../";
$name = "Vega Strike Setup";

$exec_bin = "g++";

$exec_include = "-Wall -g3 -I/Users/daniel/Vega/dialog-1.0-20040920 -I/home/daniel/install/dialog-1.0-20040920";
$exec_compile = "-g3 -L/Users/daniel/Vega/dialog-1.0-20040920 -L/home/daniel/install/dialog-1.0-20040920 -ldialog -lcurses";
$exec_defines = "-DCONSOLE";

if ($USE_GTK) {
	$exec_include = "-Wall -O3 `gtk-config --cflags`";
	$exec_compile = "-O3 `gtk-config --libs`";
	$exec_defines = "-DGTK";
}

$do_c = 1;
$do_cpp = 1;

@BUILD = (
	{
		"name" => $name,
		"include" => $root."src/include/",
		"src" => $root."src/c/",
		"build" => $root."src/bin/",
		"install" => $root."bin/"
	}
);
