/*
 * script_call_omap.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "cmd/unit_generic.h"
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"

varInst *Mission::call_omap(missionNode *node, int mode) {
    varInst *viret = NULL;
    if (mode == SCRIPT_PARSE) {
        string cmd = node->attr_value("name");
        node->script.method_id = module_omap_map[cmd];
    }
    callback_module_omap_type method_id = (callback_module_omap_type) node->script.method_id;
    if (method_id == CMT_OMAP_new) {
        viret = call_omap_new(node, mode);

        return viret;
    } else {
        varInst *ovi = getObjectArg(node, mode);
        omap_t *my_object = getOMapObject(node, mode, ovi);
        if (method_id == CMT_OMAP_delete) {
            if (mode == SCRIPT_RUN) {
                omap_t::iterator iter;
                for (iter = my_object->begin(); iter != my_object->end(); iter++) {
                    string varname = (*iter).first;
                    varInst *vi = (*iter).second;
                    deleteVarInst(vi, true);
                }
                my_object->clear();
                delete my_object;
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OMAP_set) {
            missionNode *snode = getArgument(node, mode, 2);
            varInst *var_vi = checkExpression(snode, mode);             //should be getObjExpr

            string name = getStringArgument(node, mode, 1);

            debug(3, node, mode, "omap set");
            if (mode == SCRIPT_RUN) {
                varInst *push_vi = newVarInst(VI_IN_OBJECT);
                push_vi->type = var_vi->type;
                assignVariable(push_vi, var_vi);

                (*my_object)[name] = push_vi;
            }
            deleteVarInst(var_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OMAP_get) {
            debug(3, node, mode, "omap.get");

            string name = getStringArgument(node, mode, 1);

            viret = newVarInst(VI_TEMP);
            viret->type = VAR_ANY;
            if (mode == SCRIPT_RUN) {
                varInst *back_vi = (*my_object)[name];
                assignVariable(viret, back_vi);
                if (back_vi->type > 10)
                    assert(0);
                deleteVarInst(back_vi);                 //this won't delete it
            }
        } else if (method_id == CMT_OMAP_toxml) {
            if (node->subnodes.size() != 1) {
                fatalError(node, mode, "olist.toxml needs no arguments");
                assert(0);
            }
            debug(3, node, mode, "omap.toxml");
            if (mode == SCRIPT_RUN) {
                //call_olist_toxml(node,mode,ovi); DELETE ME FIXME ?
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OMAP_size) {
            if (node->subnodes.size() != 1) {
                fatalError(node, mode, "olist.size needs one arguments");
                assert(0);
            }
            debug(3, node, mode, "omap.size");

            viret = newVarInst(VI_TEMP);
            if (mode == SCRIPT_RUN) {
                int len = my_object->size();
                viret->int_val = len;
            }
            viret->type = VAR_INT;
        } else {
            fatalError(node, mode, "unknown command " + node->script.name + " for callback omap");
            assert(0);
        }
        deleteVarInst(ovi);
        return viret;
    }     //else (objects)

    return NULL;     //never reach
}

omap_t *Mission::getOMapObject(missionNode *node, int mode, varInst *ovi) {
    omap_t *my_object = NULL;
    if (mode == SCRIPT_RUN) {
        my_object = (omap_t *) ovi->object;
        if (my_object == NULL) {
            fatalError(node, mode, "omap: no object");
            assert(0);
        }
    }
    return my_object;
}

varInst *Mission::call_omap_new(missionNode *node, int mode) {
    varInst *viret = newVarInst(VI_TEMP);

    omap_t *my_object = new omap_t;

    viret->type = VAR_OBJECT;
    viret->objectname = "omap";

    viret->object = (void *) my_object;

    debug(3, node, mode, "omap new object: ");
    printVarInst(3, viret);

    return viret;
}

