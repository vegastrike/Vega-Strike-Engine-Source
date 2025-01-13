/*
 * script_call_olist.cpp
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
#include "xml_support.h"

#include "vegastrike.h"

#include "mission.h"
#include "easydom.h"
#include "cmd/unit_generic.h"

varInst *Mission::call_olist(missionNode *node, int mode) {
    varInst *viret = NULL;
    if (mode == SCRIPT_PARSE) {
        string cmd = node->attr_value("name");
        node->script.method_id = module_olist_map[cmd];
    }
    callback_module_olist_type method_id = (callback_module_olist_type) node->script.method_id;
    if (method_id == CMT_OLIST_new) {
        viret = call_olist_new(node, mode);
        return viret;
    } else {
        varInst *ovi = getObjectArg(node, mode);
        olist_t *my_object = getOListObject(node, mode, ovi);
        if (method_id == CMT_OLIST_delete) {
            if (mode == SCRIPT_RUN) {
                while (my_object->size() > 0) {
                    varInst *content_vi = my_object->back();
                    deleteVarInst(content_vi, true);
                    my_object->pop_back();
                }
                delete my_object;
                olist_counter--;
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OLIST_push_back) {
            missionNode *snode = getArgument(node, mode, 1);
            varInst *vi = checkExpression(snode, mode); //should be getObjExpr
            debug(3, node, mode, "olist.push_back pushing variable");
            printVarInst(3, vi);
            if (mode == SCRIPT_RUN) {
                call_olist_push_back(node, mode, ovi, vi);
            }
            deleteVarInst(vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OLIST_pop_back) {
            debug(3, node, mode, "olist.pop");
            if (mode == SCRIPT_RUN) {
                call_olist_pop_back(node, mode, ovi);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OLIST_back) {
            debug(3, node, mode, "olist.back");
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_OBJECT;
            if (mode == SCRIPT_RUN) {
                varInst *back_vi = call_olist_back(node, mode, ovi);
                assignVariable(viret, back_vi);
                deleteVarInst(back_vi); //this won't delete it
            }
        } else if (method_id == CMT_OLIST_at) {
            debug(3, node, mode, "olist.at");
            missionNode *snode = getArgument(node, mode, 1);
            int index = checkIntExpr(snode, mode);
            debug(3, snode, mode, "index is in that node");
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_ANY;
            if (mode == SCRIPT_RUN) {
                varInst *back_vi = call_olist_at(node, mode, ovi, index);
                assignVariable(viret, back_vi);
                deleteVarInst(back_vi);
            }
        } else if (method_id == CMT_OLIST_erase) {
            debug(3, node, mode, "olist.erase");
            missionNode *snode = getArgument(node, mode, 1);
            int index = checkIntExpr(snode, mode);
            debug(3, snode, mode, "index is in that node");
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
            if (mode == SCRIPT_RUN) {
                olist_t *mylist = getOListObject(node, mode, ovi);
                mylist->erase(mylist->begin() + index);
            }
        } else if (method_id == CMT_OLIST_set) {
            debug(3, node, mode, "olist.set");
            missionNode *snode = getArgument(node, mode, 1);
            int index = checkIntExpr(snode, mode);
            debug(3, snode, mode, "index is in that node");
            missionNode *newvar_node = getArgument(node, mode, 2);
            varInst *new_vi = checkExpression(newvar_node, mode);             //should be getObjExpr
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
            if (mode == SCRIPT_RUN) {
                call_olist_set(node, mode, ovi, index, new_vi);
            }
            //we have to delete the old - or not?
        } else if (method_id == CMT_OLIST_toxml) {
            if (node->subnodes.size() != 1) {
                fatalError(node, mode, "olist.toxml needs no arguments");
                assert(0);
            }
            debug(3, node, mode, "olist.toxml");
            if (mode == SCRIPT_RUN) {
                call_olist_toxml(node, mode, ovi);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_OLIST_size) {
            if (node->subnodes.size() != 1) {
                fatalError(node, mode, "olist.size needs one arguments");
                assert(0);
            }
            debug(3, node, mode, "olist.size");
            viret = newVarInst(VI_TEMP);
            if (mode == SCRIPT_RUN) {
                int len = my_object->size();
                viret->int_val = len;
            }
            viret->type = VAR_INT;
        } else {
            fatalError(node, mode, "unknown command " + node->script.name + " for callback olist");
            assert(0);
        }
        deleteVarInst(ovi);
        return viret;
    }
    return NULL; //never reach
}

olist_t *Mission::getOListObject(missionNode *node, int mode, varInst *ovi) {
    olist_t *my_object = NULL;
    if (mode == SCRIPT_RUN) {
        my_object = (olist_t *) ovi->object;
        if (my_object == NULL) {
            fatalError(node, mode, "olist: no object");
            assert(0);
        }
    }
    return my_object;
}

void Mission::call_olist_set(missionNode *node, int mode, varInst *ovi, int index, varInst *new_vi) {
    olist_t *olist = getOListObject(node, mode, ovi);
    if (((unsigned int) index) >= olist->size()) {
        char buffer[200];
        sprintf(buffer, "olist.set: index out of range size=%u, index=%u\n", (unsigned int) olist->size(), index);
        fatalError(node, mode, buffer);
        assert(0);
    }
    varInst *push_vi = newVarInst(VI_IN_OBJECT);
    push_vi->type = new_vi->type;
    assignVariable(push_vi, new_vi);
    (*olist)[index] = push_vi;
}

varInst *Mission::call_olist_at(missionNode *node, int mode, varInst *ovi, int index) {
    olist_t *olist = getOListObject(node, mode, ovi);
    if (((unsigned int) index) >= olist->size()) {
        char buffer[200];
        sprintf(buffer, "olist.at: index out of range size=%u, index=%u\n", (unsigned int) olist->size(), index);
        fatalError(node, mode, buffer);
        assert(0);
    }
    varInst *back_vi = (*olist)[index];
    return back_vi;
}

varInst *Mission::call_olist_back(missionNode *node, int mode, varInst *ovi) {
    return getOListObject(node, mode, ovi)->back();
}

void Mission::call_olist_pop_back(missionNode *node, int mode, varInst *ovi) {
    getOListObject(node, mode, ovi)->pop_back();
}

void Mission::call_olist_push_back(missionNode *node, int mode, varInst *ovi, varInst *push) {
    varInst *push_vi = newVarInst(VI_IN_OBJECT);
    push_vi->type = push->type;
    assignVariable(push_vi, push);
    getOListObject(node, mode, ovi)->push_back(push_vi);
}

void Mission::call_olist_toxml(missionNode *node, int mode, varInst *ovi) {
    olist_t *my_object = getOListObject(node, mode, ovi);
    int len = my_object->size();
    for (int i = 0; i < len; i++) {
        char buffer[200];
        sprintf(buffer, "<olist index=\"%d\"", i);
        debug(3, node, mode, buffer);
    }
}

void Mission::call_vector_into_olist(varInst *vec_vi, QVector vec3) {
    olist_t *my_object = new olist_t;
    olist_counter++;

    vec_vi->type = VAR_OBJECT;
    vec_vi->objectname = "olist";
    vec_vi->object = (void *) my_object;

    varInst *push_vi;

    push_vi = newVarInst(VI_IN_OBJECT);
    push_vi->type = VAR_FLOAT;
    push_vi->float_val = vec3.i;
    my_object->push_back(push_vi);

    push_vi = newVarInst(VI_IN_OBJECT);
    push_vi->type = VAR_FLOAT;
    push_vi->float_val = vec3.j;
    my_object->push_back(push_vi);

    push_vi = newVarInst(VI_IN_OBJECT);
    push_vi->type = VAR_FLOAT;
    push_vi->float_val = vec3.k;
    my_object->push_back(push_vi);
}

QVector Mission::call_olist_tovector(missionNode *node, int mode, varInst *ovi) {
    varInst *x_vi = call_olist_at(node, mode, ovi, 0);
    varInst *y_vi = call_olist_at(node, mode, ovi, 1);
    varInst *z_vi = call_olist_at(node, mode, ovi, 2);
    if (x_vi->type != VAR_FLOAT || y_vi->type != VAR_FLOAT || z_vi->type != VAR_FLOAT) {
        fatalError(node, mode, "that's not a vec3 object");
        assert(0);
    }
    QVector pos;
    pos.i = x_vi->float_val;
    pos.j = y_vi->float_val;
    pos.k = z_vi->float_val;
    return pos;
}

varInst *Mission::call_olist_new(missionNode *node, int mode) {
    varInst *viret = newVarInst(VI_TEMP);
    olist_t *my_object = new olist_t;
    olist_counter++;
    viret->type = VAR_OBJECT;
    viret->objectname = "olist";
    viret->object = (void *) my_object;
    debug(3, node, mode, "olist new object: ");
    printVarInst(3, viret);
    return viret;
}

