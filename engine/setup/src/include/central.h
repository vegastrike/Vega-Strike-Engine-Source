/*
 * central.h
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
#ifndef VEGA_STRIKE_ENGINE_SETUP_CENTRAL_H
#define VEGA_STRIKE_ENGINE_SETUP_CENTRAL_H

// Which interface to use. Only have one of these uncommented
// If you're using 'build' to compile, you will need to move any files not being linked to dont_link
// If you're using make or VC++, your results may vary
//#define GTK
//#define CONSOLE

#define VERSION "1.1"

#ifdef GTK
#include <gtk/gtk.h>
#endif

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include "general.h"
#include "file.h"

#ifndef MAX_READ
#define MAX_READ 1024	// Maximum number of characters to read from a line
#endif

#define CONFIG_FILE "setup.config"

void Start(int *, char ***);
void SetGroup(char *group, char *setting);
void SetInfo(char *catagory, char *info);
char *GetInfo(char *catagory);
char *GetSetting(char *group);
struct catagory *GetCatStruct(char *name);
struct group *GetGroupStruct(char *name);
struct catagory *GetNameFromInfo(char *info);

typedef struct _GtkWidget GtkWidget;
struct catagory {
    char *group;
    char *name;
    char *info;
    GtkWidget *button;
    struct catagory *next;
};

struct group {
    char *name;
    char *setting;
    struct group *next;
};

struct global_settings {
    char *program_name;
    char *config_file;
    char *temp_file;
    char *data_path;
    int columns;
};

// The structs are used primarily for the interface
extern struct catagory CATS;
extern struct group GROUPS;
extern struct global_settings CONFIG;

#endif    //VEGA_STRIKE_ENGINE_SETUP_CENTRAL_H
