/*
 * file.cpp
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

#include <string>
#include <sys/stat.h>

using std::string;
#include "file.h"
extern char origpath[65536];
bool origconfig = false;

void LoadMainConfig(void) {
    FILE *fp;
    char line[MAX_READ + 1];
    char *p, *n_parm, *parm;
    int got_name = 0;
    int got_config = 0;
    int got_temp = 0;
    int got_column = 0;
    if ((fp = fopen(CONFIG_FILE, "r")) == NULL) { //[MSVC-Warn]
        string opath(origpath);
        opath += string("/") + CONFIG_FILE;
        if ((fp = fopen(opath.c_str(), "r")) == NULL) { //[MSVC-Warn]
            fprintf(stderr, "Unable to read from %s\n", CONFIG_FILE);
            exit(-1);
        }
    }
    while ((p = fgets(line, MAX_READ, fp)) != NULL) {
        if (line[0] == '#') {
            continue;
        }
        chomp(line);
        if (line[0] == '\0') {
            continue;
        }
        parm = line;
        n_parm = next_parm(parm);
        if (strcmp("program_name", parm) == 0) {
            if (CONFIG.program_name != NULL) {
                fprintf(stderr, "Duplicate program_name in config file\n");
                continue;
            }
            if (n_parm[0] == '\0') {
                fprintf(stderr, "Missing parameter for program_name\n");
                continue;
            }
            CONFIG.program_name = NewString(n_parm);
            got_name = 1;
            continue;
        }
        if (strcmp("config_file", parm) == 0) {
            if (CONFIG.config_file != NULL) {
                fprintf(stderr, "Duplicate config_file in config file\n");
                continue;
            }
            if (n_parm[0] == '\0') {
                fprintf(stderr, "Missing parameter for config_file\n");
                continue;
            }
            CONFIG.config_file = NewString(n_parm);
            got_config = 1;
            continue;
        }
        if (strcmp("temp_file", parm) == 0) {
            if (CONFIG.temp_file != NULL) {
                fprintf(stderr, "Duplicate temp_file in config file\n");
                continue;
            }
            if (n_parm[0] == '\0') {
                fprintf(stderr, "Missing parameter for temp_file in config file\n");
                continue;
            }
            CONFIG.temp_file = NewString(n_parm);
            got_temp = 1;
            continue;
        }
        if (strcmp("columns", parm) == 0) {
            if (CONFIG.columns > 0) {
                fprintf(stderr, "Duplicate columns in config file\n");
                continue;
            }
            if (n_parm[0] == '\0') {
                fprintf(stderr, "Missing parameter for columns in config file\n");
                continue;
            }
            CONFIG.columns = atoi(n_parm);
            if (CONFIG.columns == 0) {
                fprintf(stderr, "Invalid parameter for column in config file\n");
                continue;
            }
            got_column = 1;
            continue;
        }
        fprintf(stderr, "Unknown line in config file: %s %s\n", parm, n_parm);
    }
    fclose(fp);
    if (got_name == 0) {
        fprintf(stderr, "Unable to find name of program. Using default (Application)\n");
        CONFIG.program_name = NewString("Application");
    }
    if (got_config == 0) {
        fprintf(stderr, "Fatal Error. Name of config file not found. What can I do without a config file to modify?\n");
        fprintf(stderr, "To specify a config file, edit setup.config and add the following line (without the <>):\n");
        fprintf(stderr, "config_file <Name of config file>\n");
        exit(-1);
    }
    if (got_temp == 0) {
        fprintf(stderr, "Unable to find name of temporary file. Using default (config.temp)\n");
        CONFIG.temp_file = NewString("config.temp");
    }
    if (got_column == 0) {
        fprintf(stderr, "Unable to find number of columns. Using default (3)\n");
        CONFIG.columns = 3;
    }
}

//Reads the config file to get the header information
//The program segfaults with incorrect header information
//I should add error checking for that
string mangle_config(string config) {
    return string(origpath) + string("/") + config;
}

bool useGameConfig(void) {
    struct stat st1, st2;
    if (stat(CONFIG.config_file, &st1) == 0 && stat(mangle_config(CONFIG.config_file).c_str(), &st2) == 0) {
        if (st2.st_mtime > st1.st_mtime) {
            return true;
        }
    }
    return false;
}

void LoadConfig(void) {
    FILE *fp;
    char line[MAX_READ + 1];
    char *p, *n_parm, *parm, *group;
    struct group *G_CURRENT, *G_NEXT;
    struct catagory *C_CURRENT, *C_NEXT;

    G_CURRENT = &GROUPS;
    C_CURRENT = &CATS;
    if (useGameConfig() || (fp = fopen(CONFIG.config_file, "r")) == NULL) { //[MSVC-Warn]
        origconfig = true;
        if ((fp = fopen(mangle_config(CONFIG.config_file).c_str(), "r")) == NULL) { //[MSVC-Warn]
            fprintf(stderr, "Unable to read from %s\n", CONFIG_FILE);
            exit(-1);
        }
    }
    while ((p = fgets(line, MAX_READ, fp)) != NULL) {
        parm = line;
        if (parm[0] == '<') {
            parm += 5;
        }                  //Line might start with '<!-- '. Our code is inside these comments
        if (parm[0] != '#') {
            continue;
        }                   //A line not starting with # can't be a config setting
        if (parm[1] == '#')
            continue;                   //A line with a 2nd # is an ignored line
        chomp(parm);                                  //Get rid of the \n at the end of the line
        parm++;
        n_parm = next_parm(parm);                     //next_parm is a line splitter included with general.c
        if (strcmp("groups", parm) == 0) {
            parm = n_parm;
            while ((n_parm = next_parm(parm)) != nullptr) {
                G_CURRENT->name = NewString(parm);
                G_NEXT = (struct group *) malloc(sizeof(struct group));
                if (G_NEXT == 0) {
                    fprintf(stderr, "Out of memory\n");
                    exit(-1);
                }
                G_NEXT->name = 0;
                G_NEXT->setting = 0;
                G_CURRENT->next = G_NEXT;
                G_CURRENT = G_NEXT;
                G_CURRENT->next = 0;
                parm = n_parm;
            }
            continue;
        }
        if (strcmp("cat", parm) == 0) {
            parm = n_parm;
            n_parm = next_parm(parm);
            group = NewString(parm);
            parm = n_parm;
            while ((n_parm = next_parm(parm)) != nullptr) {
                C_CURRENT->name = NewString(parm);
                C_NEXT = (struct catagory *) malloc(sizeof(struct catagory));
                if (C_NEXT == 0) {
                    fprintf(stderr, "Out of memory\n");
                    exit(-1);
                }
                C_CURRENT->next = C_NEXT;
                C_NEXT->name = 0;
                C_NEXT->group = 0;
                C_NEXT->info = 0;
                C_NEXT->button = 0;
                C_CURRENT->group = group;
                C_CURRENT = C_NEXT;
                C_CURRENT->next = 0;
                parm = n_parm;
            }
            continue;
        }
        if (strcmp("set", parm) == 0) {
            parm = n_parm;
            n_parm = next_parm(parm);
            SetGroup(parm, n_parm);
            continue;
        }
        if (strcmp("desc", parm) == 0) {
            parm = n_parm;
            n_parm = next_parm(parm);
            SetInfo(parm, n_parm);
            continue;
        }
        if (strcmp("endheader", parm) == 0) {
            fclose(fp);
            return;
        }
    }
}

void Modconfig(int setting, const char *name, const char *group) {
    FILE *rp, *wp;              //read and write
    char line[MAX_READ + 1], write[MAX_READ + 1], mid[MAX_READ + 1];
    char *p, *parm, *n_parm, *start_write, *end_write;
    int commenting = 0;               //0 if scanning, 1 if adding comments, 2 if removing comments
    int skip;
    if (useGameConfig() || (rp = fopen(CONFIG.config_file, "r")) == NULL) { //[MSVC-Warn]
        if ((rp = fopen(mangle_config(CONFIG.config_file).c_str(), "r")) == NULL) { //[MSVC-Warn]
            fprintf(stderr, "Unable to read from %s\n", CONFIG_FILE);
            exit(-1);
        }
    }
    if ((wp = fopen(CONFIG.temp_file, "w")) == NULL) { //[MSVC-Warn]
        if ((wp = fopen(mangle_config(CONFIG.temp_file).c_str(), "w")) == NULL) { //[MSVC-Warn]
            fprintf(stderr, "Unable to write to %s\n", CONFIG.temp_file);
            exit(-1);
        }
    }
    while ((p = fgets(line, MAX_READ, rp)) != NULL) {
        chomp(line);
        strncpy(write, line, MAX_READ + 1); //[MSVC-Warn]
        skip = 0;
        start_write = line;
        parm = xml_pre_chomp_comment(start_write);         //Gets the start of the comment block
        //If there's no <!--, we might still be in a comment block, but xml_pre_chomp_comment() wouldn't know that
        if (parm[0] == '\0' && start_write[0] != '\0') {
            n_parm = parm;
            parm = start_write;
            start_write = n_parm;
        }
        end_write = xml_chomp_comment(parm);               //Gets the end of the comment block
        //parm is everything inside <!-- -->, start_write and end_write
        //is everything else (excluding <!-- -->
        strncpy(mid,
                parm,
                MAX_READ
                        + 1);                      //Mid is used to keep the data inside the comments in memory //[MSVC-Warn]
        mid[strlen(parm)] = '\0';
        n_parm = next_parm(parm);
        //if (parm[0] == '#' && parm[1] == '#') { fprintf(wp, "%s\n", write); continue; }   We no longer use double # for comments
        if (parm[0] != '#' || (parm[1] == '#' && parm[0] == '#')) {
            fprintf(wp, "%s\n", write);
            continue;
        }
        if (strcmp("#endheader", parm) == 0) {
            fprintf(wp, "%s\n", write);
            continue;
        }
        if (strcmp("#end", parm) == 0) {
            if (commenting == 1) {
                fprintf(wp, "#end -->\n");
            } else if (commenting == 2) {
                fprintf(wp, "<!-- #end -->\n");
            } else {
                fprintf(wp, "%s\n", write);
            }
            commenting = 0;
            skip = 1;
            //fprintf(wp, "%s\n", write);
            continue;
        }
        if (strcmp("#groups", parm) == 0) {
            skip = 1;
        }
        if (strcmp("#cat", parm) == 0) {
            skip = 1;
        }
        if (strcmp("#set", parm) == 0) {
            parm = n_parm;
            n_parm = next_parm(parm);
            if (strcmp(parm, group) == 0) {
                if (setting == 1) {
                    fprintf(wp, "#set %s none\n", group);
                }
                if (setting == 2) {
                    fprintf(wp, "#set %s %s\n", group, name);
                }
            } else {
                fprintf(wp, "%s\n", write);
            }
            continue;
        }
        if (strcmp("#desc", parm) == 0) {
            skip = 1;
        }
        if (skip == 1) {
            fprintf(wp, "%s\n", write);
            continue;
        }
//Comments are now <!-- --> and are controlled at the start and end of the block. No longer need to comment each line
/*		if (commenting == 2) {
 *                       parm = write;
 *                       if (parm[0] == '#') { parm++; }
 *                       fprintf(wp, "%s\n", parm);
 *                       continue;
 *               }
 *               if (commenting == 1) {
 *                       fprintf(wp, "#%s\n", write);
 *                       continue;
 *               }
 */
        if (parm[0] != '#') {
            fprintf(wp, "%s\n", write);
            continue;
        }
        parm++;
        if (strcmp(name, parm) == 0) {
            commenting = setting;
        } else {
            parm = n_parm;
            while ((n_parm = next_parm(parm)) != nullptr) {
                if (parm[0] == '<') {
                    break;
                }
                if (strcmp(name, parm) == 0) {
                    commenting = setting;
                    break;
                }
                parm = n_parm;
            }
        }
        if (commenting == 0) {
            fprintf(wp, "%s\n", write);
            continue;
        }
        fprintf(wp, "%s", start_write);
        if (commenting == 1) {
            fprintf(wp, "<!-- %s", mid);
        } else if (commenting == 2) {
            fprintf(wp, "<!-- %s -->", mid);
        } else {
            fprintf(wp, "%s", mid);
        }
        fprintf(wp, "%s\n", end_write);
        //fprintf(wp, "%s\n", write);
    }
    fclose(wp);
    fclose(rp);
    //Now we commit the changes
    if ((rp = fopen(CONFIG.temp_file, "r")) == NULL) { //[MSVC-Warn]
        if ((rp = fopen(mangle_config(CONFIG.temp_file).c_str(), "r")) == NULL) { //[MSVC-Warn]
            fprintf(stderr, "Unable to read from %s\n", CONFIG.temp_file);

            exit(-1);
        }
    }
    string tmp1 = CONFIG.config_file;
/*
 *       if(origconfig) {
 *               tmp1 = mangle_config (CONFIG.config_file);
 *       }
 */
    if ((wp = fopen(tmp1.c_str(), "w")) == NULL) { //[MSVC-Warn]
        tmp1 = mangle_config(CONFIG.config_file);
        if ((wp = fopen(tmp1.c_str(), "w")) == NULL) { //[MSVC-Warn]
            tmp1 = CONFIG.config_file;
            if ((wp = fopen(tmp1.c_str(), "w")) == NULL) { //[MSVC-Warn]
                fprintf(stderr, "Unable to write  to %s\n", CONFIG.config_file);
                exit(1);
            }
        }
    }
    while ((p = fgets(line, MAX_READ, rp)) != NULL) {
        fprintf(wp, "%s", line);
    }
    fclose(rp);
    fclose(wp);
}

void EnableSetting(const char *name, const char *group) {
    Modconfig(2, name, group);
}

void DisableSetting(const char *name, const char *group) {
    Modconfig(1, name, group);
}
