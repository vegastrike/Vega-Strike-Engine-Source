/***************************************************************************
 *                           general.h  -  description
 *                           ----------------------------
 *                           begin                : December 28, 2001
 *                           copyright            : (C) 2001 by David Ranger
 *                           email                : reliant@canshell.com
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

// Version 2.1

#ifndef GENERAL_H
#define GENERAL_H

//#define GLIB

#ifdef GLIB
#include <glib.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus		//iostream is only used in debugging
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
char *NewString(char *string);
#endif    //GENERAL_H

char *xml_pre_chomp_comment(char *string);
char *xml_chomp_comment(char *string);
#ifdef __cplusplus		//iostream is only used in debugging
}
#endif
