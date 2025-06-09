/*
 * general.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: David Ranger
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
// Version 2.2
#ifndef VEGA_STRIKE_ENGINE_MISSION_GENERAL_H
#define VEGA_STRIKE_ENGINE_MISSION_GENERAL_H

/* Don't forget, these defines can be set at compile time with the compile flag -D */

#define _G_ALL        // Enable everything except _G_DEBUG

//#define _G_DEBUG		// Debug Messaging
//#define _G_ERROR		// Error Messaging (internal and external)
//#define _G_GLIB		// use GLIB for string allocation
//#define _G_GLOB		// Enable the GLOB file searching
//#define _G_NUMBER		// Number processing
//#define _G_RANDOM		// Enable Random Number
//#define _G_STRING_MANAGE	// Enable String Management
//#define _G_STRING_PARSE	// Enable the String Parsing
//#define _G_XML          	// Enable the XML string parsing

// If memory can't be allocated to perform a function, and EXIT_ON_FATAL is not defined,
// the function will return NULL. With this defined, functions which return pointers to
// new memory will return a valid pointer. Only remove this if you want to handle error
// control yourself

// Also, any functions which you write that use ShowError() to produce a fatal error also won't
// cause the program to exit

#define EXIT_ON_FATAL -1    // exit(EXIT_ON_FATAL) on a fatal error. If _G_ERROR is undefined, there will be no message

#ifdef _G_ALL
#define _G_ERROR
#define _G_GLIB
#define _G_GLOB
#define _G_NUMBER
#define _G_RANDOM
#define _G_STRING_MANAGE
#define _G_STRING_PARSE
#define _G_XML
#endif    // _G_ALL

#define SEPERATOR '/'

#ifdef _G_GLIB
#include <glib.h>
#endif    // _G_GLIB

#ifdef _G_GLOB
#include <glob.h>
#include <stdio.h>
#endif    // _G_GLIB

#ifdef _G_ERROR
#include <stdio.h>
#endif    // _G_ERROR

#include <stdlib.h>

#ifdef _G_STRING_MANAGE
#include <string.h>
#endif    // _G_STRING_MANAGE

#ifdef _G_RANDOM
#include <time.h>
#endif    // _G_RANDOM

#ifdef _G_DEBUG
#ifdef __cplusplus		//iostream is only used in debugging
#include <iostream.h>
#endif    // __cplusplus
#endif    // _G_DEBUG

#ifndef MAX_READ
#define MAX_READ 1024
#endif    // MAX_READ

#ifndef MAX_ELEM
#define MAX_ELEM 15
#endif    // MAX_ELEM

#ifdef _G_STRING_PARSE
char *next_parm(char *string);
//char *split_words(char *string, int max_words);
char *ptr_copy(char *string);
void chomp(char *line);
char *pre_chomp(char *line);
char *replace(char *line, char *search, char *replace, int LENGTH);
char *strmov(char *to, char *from);
void lower(char *line);
void strappend(char *dest, char *source, int length);
char *StripPath(char *filename);
void StripExtension(char *filename);
#endif    // _G_STRING_PARSE

#ifdef _G_RANDOM
int randnum(int start, int end);
void randcode(char *line, int length);
#endif    // _G_STRING_RANDOM

#ifdef _G_NUMBER
void itoa(char *line, int number, int length);
int pwer(int start, int end);
int pwr(int start, int end);
int do_power(int start, int end, int multiply);
#ifdef __cplusplus
double pwer(double start, long end);
#endif    // __cplusplus
void btoa(char *dest, char *string);
#endif    // _G_NUMBER

#ifdef _G_STRING_MANAGE
#ifdef _G_GLIB
char *GetString(GString *line);
void SetString(GString **ptr, char *line);
#endif    // G_GLIB

#ifdef __cplusplus
char *GetString(char *line);
void SetString(char **ptr, char *line);
char *NewString(char *string);
#endif    // __cplusplus
#endif    // _G_STRING_MANAGE

void ShowError(char *error_msg, char *error_code, int is_fatal);

#ifdef _G_XML
char *xml_pre_chomp_comment(char *string);
char *xml_chomp_comment(char *string);
#endif    // _G_XML

#ifdef _G_GLOB
glob_t *FindFiles(char *path, char *extension);
glob_t *FindDirs(char *path);
#endif    // _G_GLOB

#endif // VEGA_STRIKE_ENGINE_MISSION_GENERAL_H
