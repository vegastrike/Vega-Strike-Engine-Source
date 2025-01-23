/*
 * general.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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

// Version 2.1

#ifndef VEGA_STRIKE_ENGINE_SETUP_GENERAL_H
#define VEGA_STRIKE_ENGINE_SETUP_GENERAL_H

//#define GLIB

#ifdef GLIB
#include <glib.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus        //iostream is only used in debugging
#include <iostream>
#endif

#if defined(MAX_READ)
/* Already Defined */
#else
#define MAX_READ 1024
#endif

#if defined(MAX_ELEM)
/* Already Defined */
#else
#define MAX_ELEM 15
#endif

//char *split_words(char *string, int max_words);
char *ptr_copy(char *string);
#ifdef __cplusplus
extern "C" {
#endif
char *next_parm(char *string);
void chomp(char *line);
#ifdef __cplusplus
}
#endif
char *pre_chomp(char *line);
char *replace(char *line, char *search, char *replace, int LENGTH);
char *strmov(char *to, char *from);
void lower(char *line);
void strappend(char *dest, char *source, int length);
int randnum(int start, int end);
void randcode(char *line, int length);
void vs_itoa(char *line, int number, int length);
int pwer(int start, int end);
int pwr(int start, int end);
#ifdef __cplusplus
double pwer(double start, long end);

extern "C" {
#endif
void btoa(char *dest, char *string);
#ifdef GLIB
char *GetString(GString *line);
void SetString(GString **ptr, char *line);
#endif    //GLIB
char *NewString(const char *string);

char *xml_pre_chomp_comment(char *string);
char *xml_chomp_comment(char *string);
#ifdef __cplusplus        //iostream is only used in debugging
}
#endif //__cplusplus

#endif    //VEGA_STRIKE_ENGINE_SETUP_GENERAL_H
