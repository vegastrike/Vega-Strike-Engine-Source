/*
 * script_variables.cpp
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


/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "root_generic/xml_support.h"

#include "src/vegastrike.h"

#include "cmd/unit_generic.h"
#include "mission.h"
#include "root_generic/easydom.h"

#include "root_generic/vs_globals.h"
#include "src/config_xml.h"

using std::cout;
using std::cerr;
using std::endl;

/* *********************************************************** */

bool Mission::checkVarType(varInst *var, enum var_type check_type) {
    if (var->type == check_type) {
        return true;
    }
    return false;
}

/* *********************************************************** */

bool Mission::doBooleanVar(missionNode *node, int mode) {
    varInst *var = doVariable(node, mode);

    bool ok = checkVarType(var, VAR_BOOL);
    if (!ok) {
        fatalError(node, mode, "expected a bool variable - got a different type");
        assert(0);
    }
    deleteVarInst(var);

    return var->bool_val;
}

/* *********************************************************** */

double Mission::doFloatVar(missionNode *node, int mode) {
    varInst *var = doVariable(node, mode);

    bool ok = checkVarType(var, VAR_FLOAT);
    if (!ok) {
        fatalError(node, mode, "expected a float variable - got a different type");
        assert(0);
    }
    deleteVarInst(var);
    return var->float_val;
}

/* *********************************************************** */

int Mission::doIntVar(missionNode *node, int mode) {
    varInst *var = doVariable(node, mode);

    bool ok = checkVarType(var, VAR_INT);
    if (!ok) {
        fatalError(node, mode, "expected a int variable - got a different type");
        assert(0);
    }
    deleteVarInst(var);
    return var->int_val;
}

/* *********************************************************** */

varInst *Mission::doObjectVar(missionNode *node, int mode) {
    varInst *var = doVariable(node, mode);

    bool ok = checkVarType(var, VAR_OBJECT);

    debug(3, node, mode, "doObjectVar got variable :");
    printVarInst(3, var);
    if (!ok) {
        fatalError(node, mode, "expected a object variable - got a different type");
        assert(0);
    }
    return var;
}

/* *********************************************************** */

varInst *Mission::lookupLocalVariable(missionNode *asknode) {
    contextStack *cstack = runtime.cur_thread->exec_stack.back();
    varInst *defnode = NULL;
    //slow search/name lookup
    for (unsigned int i = 0; i < cstack->contexts.size() && defnode == NULL; i++) {
        scriptContext *context = cstack->contexts[i];
        varInstMap *map = context->varinsts;
        defnode = (*map)[asknode->script.name];
        if (defnode != NULL) {
            debug(5, defnode->defvar_node, SCRIPT_RUN, "FOUND local variable defined in that node");
        }
    }
#if 0
    //fast index lookup
    varInst *defnode2 = NULL;
    if (asknode->script.context_id == -1 || asknode->script.varId == -1)
        printf( "ERROR: no local variable with fast lookup\n" );
    scriptContext *context = cstack->contexts[asknode->script.context_id];
    varInstMap    *map2    = context->varinsts;
    defnode2 = map2->varVec[asknode->script.varId];
    if (defnode2 != defnode)
        printf( "ERROR: wrong local variable lookup\n" );
#endif
    if (defnode == NULL) {
        return NULL;
    }
    return defnode;
}

/* *********************************************************** */

varInst *Mission::lookupModuleVariable(string mname, missionNode *asknode) {
    //only when runtime
    missionNode *module_node = runtime.modules[mname];
    if (module_node == NULL) {
        fatalError(asknode, SCRIPT_RUN, "no such module named " + mname);
        assert(0);
        return NULL;
    }
    vector<easyDomNode *>::const_iterator siter;
    for (siter = module_node->subnodes.begin(); siter != module_node->subnodes.end(); siter++) {
        missionNode *varnode = (missionNode *) *siter;
        if (varnode->script.name == asknode->script.name) {
            char buffer[200];
            sprintf(buffer, "FOUND module variable %s in that node", varnode->script.name.c_str());
            debug(4, varnode, SCRIPT_RUN, buffer);
            printVarInst(varnode->script.varinst);

            return varnode->script.varinst;
        }
    }
    return NULL;
}

/* *********************************************************** */

varInst *Mission::lookupClassVariable(string modulename, string varname, unsigned int classid) {
    missionNode *module = runtime.modules[modulename];
    string mname = module->script.name;
    if (classid == 0) {
        //no class instance
        return NULL;
    }
    if (classid >= module->script.classvars.size()) {
        fatalError(module, SCRIPT_RUN, "illegal classvar nr.");
        assert(0);
    }
    varInstMap *cvmap = module->script.classvars[classid];

    varInst *var = (*cvmap)[varname];
    return var;
}

varInst *Mission::lookupClassVariable(missionNode *asknode) {
    missionNode *module = runtime.cur_thread->module_stack.back();
    unsigned int classid = runtime.cur_thread->classid_stack.back();
    string mname = module->script.name;
    string varname = asknode->script.name;
    if (classid == 0) {
        //no class instance
        return NULL;
    }
    if (classid >= module->script.classvars.size()) {
        fatalError(asknode, SCRIPT_RUN, "illegal classvar nr.");
        assert(0);
    }
    varInstMap *cvmap = module->script.classvars[classid];

    varInst *var = (*cvmap)[varname];
    return var;
}

varInst *Mission::lookupModuleVariable(missionNode *asknode) {
    //only when runtime
    missionNode *module = runtime.cur_thread->module_stack.back();

    string mname = module->script.name;

    varInst *var = lookupModuleVariable(mname, asknode);

    return var;
}

/* *********************************************************** */

varInst *Mission::lookupGlobalVariable(missionNode *asknode) {
    missionNode *varnode = runtime.global_variables[asknode->script.name];
    if (varnode == NULL) {
        return NULL;
    }
    return varnode->script.varinst;
}

/* *********************************************************** */

void Mission::doGlobals(missionNode *node, int mode) {
    if (mode == SCRIPT_RUN || (mode == SCRIPT_PARSE && parsemode == PARSE_FULL)) {
        //nothing to do
        return;
    }
    debug(3, node, mode, "doing global variables");

    vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end() && !have_return(mode); siter++) {
        missionNode *snode = (missionNode *) *siter;
        if (snode->tag == DTAG_DEFVAR) {
            doDefVar(snode, mode, true);
        } else {
            fatalError(node, mode, "only defvars allowed below globals node");
            assert(0);
        }
    }
}

/* *********************************************************** */

varInst *Mission::doVariable(missionNode *node, int mode) {
    if (mode == SCRIPT_RUN) {
        varInst *var = lookupLocalVariable(node);
        if (var == NULL) {
            var = lookupClassVariable(node);
            if (var != NULL) {
            }
            if (var == NULL) {
                //search in module namespace
                var = lookupModuleVariable(node);
                if (var == NULL) {
                    //search in global namespace
                    var = lookupGlobalVariable(node);
                    if (var == NULL) {
                        fatalError(node, mode, "did not find variable");
                        assert(0);
                    }
                }
            }
        }
        return var;
    } else {
        //SCRIPT_PARSE
        node->script.name = node->attr_value("name");
        if (node->script.name.empty()) {
            fatalError(node, mode, "you have to give a variable name");
            assert(0);
        }
        varInst *vi = searchScopestack(node->script.name);
        if (vi == NULL) {
            missionNode *global_var = runtime.global_variables[node->script.name];
            if (global_var == NULL) {
                fatalError(node, mode, "no variable " + node->script.name + " found on the scopestack (dovariable)");
                assert(0);
            }
            vi = global_var->script.varinst;
        }
        return vi;
    }
}

/* *********************************************************** */

void Mission::doDefVar(missionNode *node, int mode, bool global_var) {
    if (mode == SCRIPT_RUN) {
        missionNode *scope = node->script.context_block_node;
        if (scope->tag == DTAG_MODULE) {
            //this is a module variable - it has been initialized at parse time
            debug(0, node, mode, "defined module variable " + node->script.name);
            return;
        }
        debug(5, node, mode, "defining context variable " + node->script.name);

        contextStack *stack = runtime.cur_thread->exec_stack.back();
        scriptContext *context = stack->contexts.back();

        varInstMap *vmap = context->varinsts;

        varInst *vi = newVarInst(VI_LOCAL);
        vi->defvar_node = node;
        vi->block_node = scope;
        vi->type = node->script.vartype;
        vi->name = node->script.name;

        (*vmap)[node->script.name] = vi;

        printRuntime();

        return;
    }
    //SCRIPT_PARSE

    node->script.name = node->attr_value("name");
    if (node->script.name.empty()) {
        fatalError(node, mode, "you have to give a variable name");
        assert(0);
    }
    string value = node->attr_value("initvalue");

    debug(5, node, mode, "defining variable " + node->script.name);

    string type = node->attr_value("type");

    node->script.vartype = vartypeFromString(type);

    missionNode *scope = NULL;
    int scope_id = -1;
    if (global_var == false) {
        scope = scope_stack.back();
        scope_id = scope_stack.size() - 1;
    }
    varInst *vi = NULL;
    if (global_var) {
        vi = newVarInst(VI_GLOBAL);
    } else if (scope->tag == DTAG_MODULE) {
        vi = newVarInst(VI_MODULE);
    } else {
        vi = newVarInst(VI_LOCAL);
    }
    vi->defvar_node = node;
    vi->block_node = scope;
    vi->type = node->script.vartype;
    vi->name = node->script.name;
    if (global_var || scope->tag == DTAG_MODULE) {
        if (!value.empty()) {
            debug(4, node, mode, "setting init for " + node->script.name);
            if (vi->type == VAR_FLOAT) {
                vi->float_val = strtod(value.c_str(), NULL);
            } else if (vi->type == VAR_INT) {
                vi->int_val = atoi(value.c_str());
            } else if (vi->type == VAR_BOOL) {
                if (value == "true") {
                    vi->bool_val = true;
                } else if (value == "false") {
                    vi->bool_val = false;
                } else {
                    fatalError(node, mode, "wrong bool value");
                    assert(0);
                }
            } else {
                fatalError(node, mode, "this datatype can;t be initialized");
                assert(0);
            }
            printVarInst(vi);
        }
    } else
        //local variable
    if (!value.empty()) {
        fatalError(node, mode, "initvalue is not allowed for a local variable");
        assert(0);
    }
    node->script.varinst = vi;     //FIXME (not for local var)
    node->script.context_id = -1;
    node->script.varId = -1;

    int varId = -1;
    if (global_var) {
        //global var
        debug(3, node, mode, "defining global variable");
        runtime.global_variables[node->script.name] = node;
        varId = runtime.global_varvec.addVar(vi);
        printGlobals(3);
    } else {
        //module, class or local var
        string classvar = node->attr_value("classvar");
        if (classvar == "true") {
            //class var
            missionNode *module_node = scope_stack.back();
            if (module_node->script.classvars.size() != 1) {
                fatalError(node, mode, "no module node with classvars");
                assert(0);
            }
            varInstMap *vmap = module_node->script.classvars[0];
            (*vmap)[node->script.name] = vi;

            varId = vmap->varVec.addVar(vi);
        } else {
            //module or local var
            scope->script.variables[node->script.name] = vi;

            varId = scope->script.variables.varVec.addVar(vi);
        }
        node->script.context_block_node = scope;
        node->script.context_id = scope_id;

        debug(5, scope, mode, "defined variable in that scope");
    }
    node->script.varId = varId;
}

/* *********************************************************** */

void Mission::doSetVar(missionNode *node, int mode) {
    trace(node, mode);
    if (mode == SCRIPT_PARSE) {
        node->script.name = node->attr_value("name");
        if (node->script.name.empty()) {
            fatalError(node, mode, "you have to give a variable name");
            VS_LOG_FLUSH_EXIT(fatal, "you have to give a variable name", -1);
        }
    }
    debug(3, node, mode, "trying to set variable " + node->script.name);
    if (node->subnodes.size() != 1) {
        fatalError(node, mode, "setvar takes exactly one argument");
        VS_LOG_FLUSH_EXIT(fatal, "setvar takes exactly one argument", -1);
    }
    missionNode *expr = (missionNode *) node->subnodes[0];
    if (mode == SCRIPT_PARSE) {
        varInst *vi = searchScopestack(node->script.name);
        if (vi == nullptr) {
            missionNode *global_var = runtime.global_variables[node->script.name];
            if (global_var == nullptr) {
                fatalError(node, mode, "no variable " + node->script.name + " found on the scopestack (setvar)");
                VS_LOG_FLUSH_EXIT(fatal, (boost::format("no variable %1% found on the scopestack (setvar)") % node->script.name), -1);
            }
            vi = global_var->script.varinst;
        }
        switch (vi->type) {
            case VAR_FLOAT: // fall-through
            case VAR_INT:   // fall-through
            case VAR_BOOL:  // fall-through
            case VAR_OBJECT:// fall-through
                break;
            default:
                fatalError(node, mode, "unsupported type in setvar");
                VS_LOG_FLUSH_EXIT(fatal, "unsupported type in setvar", -1);
        }
    }
    if (mode == SCRIPT_RUN) {
        varInst *var_inst = doVariable(node, mode);         //lookup variable instance
        if (var_inst == nullptr) {
            fatalError(node, mode, "variable lookup failed for " + node->script.name);
            printRuntime();
            VS_LOG_FLUSH_EXIT(fatal, (boost::format("variable lookup failed for %1%") % node->script.name), -1);
        }
        if (var_inst->type == VAR_BOOL) {
            bool res = checkBoolExpr(expr, mode);
            var_inst->bool_val = res;
        } else if (var_inst->type == VAR_FLOAT) {
            double res = checkFloatExpr(expr, mode);
            var_inst->float_val = res;
        } else if (var_inst->type == VAR_INT) {
            int res = checkIntExpr(expr, mode);
            var_inst->int_val = res;
        } else if (var_inst->type == VAR_OBJECT) {
            debug(3, node, mode, "setvar object (before)");
            varInst *ovi = checkObjectExpr(expr, mode);
            assignVariable(var_inst, ovi);
            debug(3, node, mode, "setvar object left,right");
            printVarInst(3, var_inst);
            printVarInst(3, ovi);
            deleteVarInst(ovi);
        } else {
            fatalError(node, mode, "unsupported datatype");
            VS_LOG_FLUSH_EXIT(fatal, "unsupported datatype", -1);
        }
    }
}

/* *********************************************************** */

varInst *Mission::doConst(missionNode *node, int mode) {
    if (mode == SCRIPT_PARSE) {
        string typestr = node->attr_value("type");
        string valuestr = node->attr_value("value");
        if (typestr.empty()) {
            fatalError(node, mode, "no valid const declaration");
            assert(0);
        }
        debug(5, node, mode, "parsed const value " + valuestr);

        varInst *vi = newVarInst(VI_CONST);
        if (typestr == "float") {
            node->script.vartype = VAR_FLOAT;
            vi->float_val = atof(valuestr.c_str());
        } else if (typestr == "int") {
            node->script.vartype = VAR_INT;
            vi->int_val = atoi(valuestr.c_str());
        } else if (typestr == "bool") {
            node->script.vartype = VAR_BOOL;
            if (valuestr == "true") {
                vi->bool_val = true;
            } else if (valuestr == "false") {
                vi->bool_val = false;
            } else {
                fatalError(node, mode, "wrong bool value");
                assert(0);
            }
        } else if (typestr == "object") {
            string objecttype = node->attr_value("object");
            if (objecttype == "string") {
                varInst *svi = call_string_new(node, mode, valuestr);
                vi->type = VAR_OBJECT;
                assignVariable(vi, svi);
                vi->type = VAR_OBJECT;
                node->script.vartype = VAR_OBJECT;
                deleteVarInst(svi);
            } else {
                fatalError(node, mode, "you cant have a const object");
                assert(0);
            }
        } else {
            fatalError(node, mode, "unknown variable type");
            assert(0);
        }
        vi->type = node->script.vartype;

        node->script.varinst = vi;
    }
    return node->script.varinst;
}

/* *********************************************************** */

void Mission::assignVariable(varInst *v1, varInst *v2) {
    if (v1->type != v2->type && v1->type != VAR_ANY) {
        fatalError(NULL, SCRIPT_RUN, "wrong types in assignvariable");
        saveVarInst(v1, cout);
        saveVarInst(v2, cout);
        assert(0);
    }
    if (v1->type == VAR_OBJECT) {
        if (v1->objectname.empty()) {
            //the object has not been set
            //we set it below
        } else if (v2->objectname.empty()) {
            //printf("WARNING: assignVariable v2==empty\n");  FIXME ??
        } else if (v1->objectname != v2->objectname) {
            fatalError(NULL,
                    SCRIPT_RUN,
                    "wrong object types in assignment (" + v1->objectname + " , " + v2->objectname);
            assert(0);
        }
    }
    if (v1->type == VAR_ANY) {
        v1->type = v2->type;
    }
    v1->float_val = v2->float_val;
    v1->int_val = v2->int_val;
    v1->bool_val = v2->bool_val;
    if (!(v2->objectname.empty())) {
        v1->objectname = v2->objectname;
    }
    v1->object = v2->object;
}

/* *********************************************************** */

var_type Mission::vartypeFromString(string type) {
    var_type vartype;
    if (type == "float") {
        vartype = VAR_FLOAT;
    } else if (type == "bool") {
        vartype = VAR_BOOL;
    } else if (type == "int") {
        vartype = VAR_INT;
    } else if (type == "object") {
        vartype = VAR_OBJECT;
    } else {
        fatalError(NULL, SCRIPT_PARSE, "unknown var type " + type);
        vartype = VAR_FAILURE;
    }
    return vartype;
}

/* *********************************************************** */

void Mission::saveVariables(const std::ostream &out) {
}

/* *********************************************************** */

void Mission::saveVarInst(varInst *vi, std::ostream &aa_out) {
    char buffer[100];
    if (vi == NULL) {
    } else {
        if (vi->type == VAR_BOOL) {
            sprintf(buffer, "type=\"bool\" value=\"%d\" ", vi->bool_val);
        } else if (vi->type == VAR_FLOAT) {
            sprintf(buffer, "type=\"float\"  value=\"%f\" ", vi->float_val);
        } else if (vi->type == VAR_INT) {
            sprintf(buffer, "type=\"int\"  value=\"%d\" ", vi->int_val);
        } else if (vi->type == VAR_OBJECT) {
            if (vi->objectname == "string") {
                string *sptr = (string *) vi->object;
                sprintf(buffer,
                        "type=\"object\"  object=\"%s\" value=\"%s\" >\n ",
                        vi->objectname.c_str(),
                        sptr->c_str());
            } else {
                sprintf(buffer,
                        "type=\"object\"  object=\"%s\" value=\"0x%p\" >\n ",
                        vi->objectname.c_str(),
                        vi->object);

                string modname = "_" + vi->objectname;

                doCall_toxml(modname, vi);
            }
        }
    }
}
