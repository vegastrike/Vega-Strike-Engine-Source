/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "cmd/unit_generic.h"
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"

varInst *Mission::call_string(missionNode *node, int mode) {
    varInst *viret = NULL;
    if (mode == SCRIPT_PARSE) {
        string cmd = node->attr_value("name");
        node->script.method_id = module_string_map[cmd];
    }
    callback_module_string_type method_id = (callback_module_string_type) node->script.method_id;
    if (method_id == CMT_STRING_new) {
        viret = call_string_new(node, mode, "");

        return viret;
    } else {
        varInst *ovi = getObjectArg(node, mode);
        string *my_string = getStringObject(node, mode, ovi);
        if (method_id == CMT_STRING_delete) {
            if (mode == SCRIPT_RUN) {
                delete my_string;
                string_counter--;
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_STRING_print) {
            if (mode == SCRIPT_RUN) {
                call_string_print(node, mode, ovi);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_STRING_equal) {
            missionNode *other_node = getArgument(node, mode, 1);
            varInst *other_vi = checkObjectExpr(other_node, mode);

            bool res = false;
            if (mode == SCRIPT_RUN) {
                string s1 = call_string_getstring(node, mode, ovi);
                string s2 = call_string_getstring(node, mode, other_vi);
                if (s1 == s2) {
                    res = true;
                }
            }
            deleteVarInst(other_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_BOOL;
            viret->bool_val = res;
        } else if (method_id == CMT_STRING_begins) {
            //test if s1 begins with s2
            missionNode *other_node = getArgument(node, mode, 1);
            varInst *other_vi = checkObjectExpr(other_node, mode);

            bool res = false;
            if (mode == SCRIPT_RUN) {
                string s1 = call_string_getstring(node, mode, ovi);
                string s2 = call_string_getstring(node, mode, other_vi);
                if (s1.find(s2, 0) == 0) {
                    res = true;
                }
            }
            deleteVarInst(other_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_BOOL;
            viret->bool_val = res;
        } else {
            fatalError(node, mode, "unknown command " + node->script.name + " for callback string");
            assert(0);
        }
        deleteVarInst(ovi);
        return viret;
    }     //else objects
    return NULL;     //never reach
}

string Mission::getStringArgument(missionNode *node, int mode, int arg_nr) {
    missionNode *arg_node = getArgument(node, mode, arg_nr);
    varInst *arg_vi = checkObjectExpr(arg_node, mode);

    string retstr;
    if (mode == SCRIPT_RUN) {
        retstr = call_string_getstring(arg_node, mode, arg_vi);
    }
    return retstr;
}

string Mission::call_string_getstring(missionNode *node, int mode, varInst *ovi) {
    if (ovi->type != VAR_OBJECT || (ovi->type == VAR_OBJECT && ovi->objectname != "string")) {
        fatalError(node, mode, "call_string_getstring needs string object as arg");
        assert(0);
    }
    string *my_string = getStringObject(node, mode, ovi);

    string ret = *my_string;

    return ret;
}

void Mission::call_string_print(missionNode *node, int mode, varInst *ovi) {
    string *my_string = getStringObject(node, mode, ovi);

    std::cout << *my_string;
}

varInst *Mission::call_string_new(missionNode *node, int mode, string initstring) {
    debug(10, node, mode, "call_string");

    varInst *viret = newVarInst(VI_TEMP);

    string *my_string = new string(initstring);
    string_counter++;

    viret->type = VAR_OBJECT;
    viret->objectname = "string";
    viret->object = (void *) my_string;

    return viret;
}

string *Mission::getStringObject(missionNode *node, int mode, varInst *ovi) {
    string *my_object = NULL;
    if (mode == SCRIPT_RUN) {
        my_object = (string *) ovi->object;
        if (my_object == NULL) {
            fatalError(node, mode, "string: no object");
            assert(0);
        }
    }
    return my_object;
}

