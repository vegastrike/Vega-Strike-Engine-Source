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
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "cmd/unit_generic.h"
#include "mission.h"
#include "easydom.h"

#include "vs_globals.h"
#include "config_xml.h"

using std::cout;
using std::cerr;
using std::endl;

/* *********************************************************** */

string Mission::modestring(int mode) {
    if (mode == SCRIPT_PARSE) {
        if (parsemode == PARSE_DECL) {
            return "parsedecl";
        } else {
            return "parsefull";
        }
    } else {
        return "run";
    }
}

/* *********************************************************** */

void Mission::trace(missionNode *node, int mode) {
    if (!do_trace || mode == SCRIPT_PARSE) {
        return;
    }
    cout << "trace: ";
    missionNode *module_node = runtime.cur_thread->module_stack.back();
    if (module_node) {
        cout << module_node->script.name << ":" << node->attr_value("line");
    }
    cout << " tag " << node->Name() << "name " << node->attr_value("name") << " module " << node->attr_value("module")
            << endl;
}

/* *********************************************************** */

void Mission::fatalError(missionNode *node, int mode, string message) {
    cout << "fatal (" << modestring(mode) << ") " << message << " : ";
    printNode(node, mode);
    if (node) {
        cout << "semantic error at line " << node->attr_value("line") << endl;
    }
    if (mode == SCRIPT_RUN) {
        missionNode *module_node = runtime.cur_thread->module_stack.back();
        if (module_node) {
            cout << "ERROR: " << module_node->script.name << ":" << node->attr_value("line") << endl;
        }
        cout << endl << "Stackdump: " << endl;
        for (unsigned int i = 0; i < runtime.cur_thread->exec_stack.size(); i++) {
            contextStack *cstack = runtime.cur_thread->exec_stack[i];
            missionNode *mnode = runtime.cur_thread->module_stack[i];
            unsigned int classid = runtime.cur_thread->classid_stack[i];
            for (unsigned int j = 0; j < cstack->contexts.size(); j++) {
                scriptContext *context = cstack->contexts[j];
                missionNode *bnode = context->block_node;
                if (bnode) {
                    cout << mnode->script.name << ":" << classid << " line " << bnode->attr_value("line") << " "
                            << bnode->script.name << " ";

                    printNode(bnode, mode);
                }
            }
        }
    }
    cout << endl;
}

/* *********************************************************** */

void Mission::runtimeFatal(string message) {
    cout << "runtime fatalError: " << message << endl;
}

/* *********************************************************** */

void Mission::warning(string message) {
    cout << "warning: " << message << endl;
}

/* *********************************************************** */

void Mission::debug(int level, missionNode *node, int mode, string message) {
    if (level <= debuglevel) {
        debug(node, mode, message);
    }
}

/* *********************************************************** */

void Mission::debug(missionNode *node, int mode, string message) {
    cout << "debug (" << modestring(mode) << ") " << message << " : ";
    printNode(node, mode);
}

/* *********************************************************** */

void Mission::printNode(missionNode *node, int mode) {
    if (node) {
        node->printNode(cout, 0, 0);
    }
}

/* *********************************************************** */

bool Mission::have_return(int mode) {
    if (mode == SCRIPT_PARSE) {
        return false;
    }
    contextStack *cstack = runtime.cur_thread->exec_stack.back();
    if (cstack->return_value == NULL) {
        return false;
    }
    return true;
}

/* *********************************************************** */

void Mission::initTagMap() {
    tagmap["module"] = DTAG_MODULE;
    tagmap["script"] = DTAG_SCRIPT;
    tagmap["if"] = DTAG_IF;
    tagmap["block"] = DTAG_BLOCK;
    tagmap["setvar"] = DTAG_SETVAR;
    tagmap["exec"] = DTAG_EXEC;
    tagmap["call"] = DTAG_CALL;
    tagmap["while"] = DTAG_WHILE;
    tagmap["and"] = DTAG_AND_EXPR;
    tagmap["or"] = DTAG_OR_EXPR;
    tagmap["not"] = DTAG_NOT_EXPR;
    tagmap["test"] = DTAG_TEST_EXPR;
    tagmap["fmath"] = DTAG_FMATH;
    tagmap["vmath"] = DTAG_VMATH;
    tagmap["var"] = DTAG_VAR_EXPR;
    tagmap["defvar"] = DTAG_DEFVAR;
    tagmap["const"] = DTAG_CONST;
    tagmap["arguments"] = DTAG_ARGUMENTS;
    tagmap["globals"] = DTAG_GLOBALS;
    tagmap["return"] = DTAG_RETURN;
    tagmap["import"] = DTAG_IMPORT;
}

/* *********************************************************** */

void Mission::printVarInst(varInst *vi) {
}

/* *********************************************************** */

void Mission::printVarInst(int dbg_level, varInst *vi) {
    if (dbg_level <= debuglevel) {
        saveVarInst(vi, cout);
    }
}

/* *********************************************************** */

void Mission::printVarmap(const varInstMap &vmap) {
    vsUMap<string, varInst *>::const_iterator iter;
    for (iter = vmap.begin(); iter != vmap.end(); iter++) {
        cout << "variable " << (*iter).first;
        varInst *vi = (*iter).second;

        printVarInst(vi);
    }
}

/* *********************************************************** */

void Mission::printModules() {
    vsUMap<string, missionNode *>::iterator iter;
    for (iter = runtime.modules.begin(); iter != runtime.modules.end(); iter++) {
        cout << "  module " << (*iter).first;
        missionNode *mnode = (*iter).second;
        printNode(mnode, 0);
        cout << "        scripts" << endl;

        vsUMap<string, missionNode *>::iterator iter2;
        for (iter2 = mnode->script.scripts.begin(); iter2 != mnode->script.scripts.end(); iter2++) {
            cout << "  script " << (*iter2).first;
            missionNode *snode = (*iter2).second;
            printNode(snode, 0);
        }
    }
}

/* *********************************************************** */

void Mission::printRuntime() {
    return;

    cout << "RUNTIME" << endl;
    cout << "MODULES:" << endl;

    vsUMap<string, missionNode *>::iterator iter;
    for (iter = runtime.modules.begin(); iter != runtime.modules.end(); iter++) {
        cout << "  module " << (*iter).first;
        printNode((*iter).second, 0);
    }
    cout << "CURRENT THREAD:" << endl;

    printThread(runtime.cur_thread);
}

void Mission::printGlobals(int dbg_level) {
    if (dbg_level > debuglevel) {
        return;
    }
    vsUMap<string, missionNode *>::iterator iter;
    for (iter = runtime.global_variables.begin(); iter != runtime.global_variables.end(); iter++) {
        cout << "  global var " << (*iter).first;
        printNode((*iter).second, 0);
    }
}

/* *********************************************************** */

void Mission::printThread(missionThread *thread) {
    return;

    vector<contextStack *>::const_iterator siter;
    for (siter = thread->exec_stack.begin(); siter != thread->exec_stack.end(); siter++) {
        contextStack *stack = *siter;

        vector<scriptContext *>::const_iterator iter2;

        cout << "SCRIPT CONTEXTS" << endl;
        for (iter2 = stack->contexts.begin(); iter2 != stack->contexts.end(); iter2++) {
            scriptContext *context = *iter2;

            cout << "VARMAP " << endl;
            printVarmap(*(context->varinsts));
        }
    }
}

/* *********************************************************** */

varInst *Mission::searchScopestack(string name) {
    int elem = scope_stack.size() - 1;
    varInst *vi = NULL;
    while (vi == NULL && elem >= 0) {
        missionNode *scope = scope_stack[elem];

        vi = scope->script.variables[name];
        if (vi == NULL) {
            if (scope->script.classvars.size() > 0) {
                varInstMap *cvmap = scope->script.classvars[0];
                vi = (*cvmap)[name];
                debug(10, scope, 0, "found var " + name + " as classvar");
            }
            if (vi == NULL) {
                debug(5, scope, 0, "variable " + name + " not found in that scope");
            }
        } else {
            debug(5, scope, 0, "variable " + name + " FOUND in that scope");
        }
        elem--;
    }
    return vi;
}

/* *********************************************************** */

missionNode *Mission::lookupScript(string scriptname, string modulename) {
    missionNode *module = runtime.modules[modulename];
    if (module == NULL) {
        fatalError(module, SCRIPT_PARSE, "module " + modulename + " not found - maybe you forgot to import it?");
        assert(0);
    }
    missionNode *scriptnode = module->script.scripts[scriptname];
    if (scriptnode == NULL) {
        fatalError(module, SCRIPT_PARSE, "script " + scriptname + " not found in module " + modulename);
        assert(0);
    }
    return scriptnode;
}

