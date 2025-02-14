/*
 * central.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; David Ranger specifically
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

#include "central.h"
struct catagory CATS;
struct group GROUPS;
struct global_settings CONFIG;

static char EMPTY_STR[] = "";

// Primary initialization function. Sets everything up and takes care of the program
void Start(int *argc, char ***argv) {
    LoadMainConfig();
    InitGraphics(argc, argv);
    LoadConfig();
    ShowMain();
}

void SetGroup(char *group, char *setting) {
    struct group *CURRENT;
    CURRENT = &GROUPS;
    do {
        if (CURRENT->name == NULL) {
            continue;
        }
        if (strcmp(group, CURRENT->name) == 0) {
            CURRENT->setting = NewString(setting);
            return;
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
}

void SetInfo(char *catagory, char *info) {
    struct catagory *CURRENT;
    CURRENT = &CATS;
    do {
        if (CURRENT->name == NULL) {
            continue;
        }
        if (strcmp(catagory, CURRENT->name) == 0) {
            CURRENT->info = NewString(info);
            return;
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
}

char *GetInfo(char *catagory) {
    struct catagory *CURRENT;
    CURRENT = &CATS;
    do {
        if (CURRENT->name == NULL) {
            continue;
        }
        if (strcmp(catagory, CURRENT->name) == 0) {
            if (CURRENT->info) {
                return CURRENT->info;
            } else {
                return catagory;
            }
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
    return catagory;
}

char *GetSetting(char *group) {
    struct group *CUR;
    CUR = &GROUPS;
    do {
        if (CUR->name == NULL) {
            continue;
        }
        if (strcmp(CUR->name, group) == 0) {
            return CUR->setting;
        }
    } while ((CUR = CUR->next) != nullptr);
    return EMPTY_STR;
}

struct catagory *GetCatStruct(char *name) {
    struct catagory *CUR;
    CUR = &CATS;
    do {
        if (CUR->name == NULL) {
            continue;
        }
        if (strcmp(CUR->name, name) == 0) {
            return CUR;
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}

struct group *GetGroupStruct(char *name) {
    struct group *CUR;
    CUR = &GROUPS;
    do {
        if (CUR->name == NULL) {
            continue;
        }
        if (strcmp(CUR->name, name) == 0) {
            return CUR;
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}

struct catagory *GetNameFromInfo(char *info) {
    struct catagory *CUR;
    CUR = &CATS;
    do {
        if (CUR->name == NULL) {
            continue;
        }
        if (CUR->info != NULL) {
            if (strcmp(CUR->info, info) == 0) {
                return CUR;
            }
        } else {
            if (strcmp(CUR->name, info) == 0) {
                return CUR;
            }
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}
