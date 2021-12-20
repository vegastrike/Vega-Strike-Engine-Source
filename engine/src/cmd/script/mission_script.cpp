/*
 * mission_script.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
 * Copyright (C) 2021 Stephen G. Tuggy
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "vs_globals.h"
#include "config_xml.h"

#include "msgcenter.h"
#include "cmd/unit_generic.h"

/* *********************************************************** */

/* *********************************************************** */

void Mission::doImport( missionNode *node, int mode )
{
    if (mode == SCRIPT_PARSE && parsemode == PARSE_DECL) {
        string name = node->attr_value( "name" );
        if ( name.empty() ) {
            fatalError( node, mode, "you have to give a name to import" );
            assert( 0 );
        }
        node->script.name = name;
        import_stack.push_back( name );
    }
}

/* *********************************************************** */

void Mission::doModule( missionNode *node, int mode )
{
    if (mode == SCRIPT_PARSE) {
        string name = node->attr_value( "name" );
        if (parsemode == PARSE_DECL) {
            if ( name.empty() ) {
                fatalError( node, mode, "you have to give a module name" );
                assert( 0 );
            }
            if (runtime.modules[name] != NULL) {
                fatalError( node, mode, "there can only be one module with name "+name );
                assert( 0 );
            }
            if (name == "director")
                director = node;
            node->script.name = name;

            varInstMap *cvmap = new varInstMap;
            node->script.classvars.push_back( cvmap );
            node->script.classinst_counter = 0;
            debug( 10, node, mode, "created classinst 0" );

            runtime.modules[name] = node;             //add this module to the list of known modules
        }
        scope_stack.push_back( node );

        current_module = node;

        debug( 5, node, mode, "added module "+name+" to list of known modules" );
    }
    if (mode == SCRIPT_RUN) {
        //SCRIPT_RUN
        runtime.cur_thread->module_stack.push_back( node );
        runtime.cur_thread->classid_stack.push_back( 0 );
    }
    vector< easyDomNode* >::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        missionNode *snode = (missionNode*) *siter;
        if (snode->tag == DTAG_SCRIPT) {
            varInst *vi = doScript( snode, mode );
            deleteVarInst( vi );
        } else if (snode->tag == DTAG_DEFVAR) {
            doDefVar( snode, mode );
        } else if (snode->tag == DTAG_GLOBALS) {
            doGlobals( snode, mode );
        } else if (snode->tag == DTAG_IMPORT) {
            doImport( snode, mode );
        } else {
            fatalError( snode, mode, "unkown node type for module subnodes" );
            assert( 0 );
        }
    }
    if (mode == SCRIPT_PARSE) {
        scope_stack.pop_back();
    } else {
        runtime.cur_thread->module_stack.pop_back();
        runtime.cur_thread->classid_stack.pop_back();
    }
}

/* *********************************************************** */

scriptContext* Mission::makeContext( missionNode *node )
{
    scriptContext *context = new scriptContext;

    context->varinsts   = new varInstMap;

    context->block_node = node;

    return context;
}

/* *********************************************************** */

void Mission::removeContextStack()
{
    contextStack *cstack = runtime.cur_thread->exec_stack.back();
    runtime.cur_thread->exec_stack.pop_back();
    if (cstack->return_value != NULL)
        deleteVarInst( cstack->return_value, true );
    delete cstack;
}

void Mission::addContextStack( missionNode *node )
{
    contextStack *cstack = new contextStack;

    cstack->return_value = NULL;

    runtime.cur_thread->exec_stack.push_back( cstack );
}

scriptContext* Mission::addContext( missionNode *node )
{
    scriptContext *context = makeContext( node );
    contextStack  *stack   = runtime.cur_thread->exec_stack.back();
    stack->contexts.push_back( context );

    debug( 5, node, SCRIPT_RUN, "added context for this node" );
    printRuntime();

    return context;
}

/* *********************************************************** */

void Mission::removeContext()
{
    contextStack *stack = runtime.cur_thread->exec_stack.back();

    int lastelem = stack->contexts.size()-1;

    scriptContext *old = stack->contexts[lastelem];
    stack->contexts.pop_back();
#if 0
    vsUMap< string, varInst* >::const_iterator iter;
    for (iter = old->varinsts->begin(); iter != old->varinsts->end(); iter++) {
        varInst *vi = (*iter).second;
        deleteVarInst( vi );
    }
#endif
    deleteVarMap( old->varinsts );
    delete old->varinsts;
    delete old;
}

/* *********************************************************** */

varInst* Mission::doScript( missionNode *node, int mode, varInstMap *varmap )
{
    trace( node, mode );
    if (mode == SCRIPT_PARSE) {
        current_script = node;
        if (parsemode == PARSE_DECL) {
            node->script.name = node->attr_value( "name" );
            if ( node->script.name.empty() )
                fatalError( node, mode, "you have to give a script name" );
            current_module->script.scripts[node->script.name] = node;
            debug( 5, node, mode, "added to module "+current_module->script.name+" : script ="+node->script.name );

            node->script.nr_arguments = 0;

            string retvalue = node->attr_value( "type" );
            if (retvalue.empty() || retvalue == "void")
                node->script.vartype = VAR_VOID;
            else
                node->script.vartype = vartypeFromString( retvalue );
        }
        scope_stack.push_back( node );
    }
    debug( 5, node, mode, "executing script name="+node->script.name );
    if (mode == SCRIPT_RUN) {
        addContextStack( node );
        addContext( node );
    }
    vector< easyDomNode* >::const_iterator siter;
    if (mode == SCRIPT_PARSE && parsemode == PARSE_DECL) {
        node->script.nr_arguments  = 0;
        node->script.argument_node = NULL;
    }
    for (siter = node->subnodes.begin(); siter != node->subnodes.end() && !have_return( mode ); siter++) {
        missionNode *snode = (missionNode*) *siter;
        if (snode->tag == DTAG_ARGUMENTS) {
            doArguments( snode, mode, varmap );
            if (mode == SCRIPT_PARSE && parsemode == PARSE_DECL)
                node->script.argument_node = snode;
            char buffer[200];
            sprintf( buffer, "nr of arguments=%d", node->script.nr_arguments );
            debug( 3, node, mode, buffer );
        } else {
            if (mode == SCRIPT_PARSE && parsemode == PARSE_DECL) {
                //do nothing, break here
            } else {
                checkStatement( snode, mode );
            }
        }
    }
    if (mode == SCRIPT_RUN) {
        removeContext();
        contextStack *cstack = runtime.cur_thread->exec_stack.back();
        varInst *vi = cstack->return_value;
        if (vi != NULL) {
            if (node->script.vartype != vi->type) {
                fatalError( node, mode, "doScript: return type not set correctly" );
                assert( 0 );
            }
        } else
        if (node->script.vartype != VAR_VOID) {
            fatalError( node, mode, "no return set from doScript" );
            assert( 0 );
        }
        varInst *viret = NULL;
        if (vi) {
            viret = newVarInst( VI_TEMP );
            viret->type = vi->type;
            assignVariable( viret, vi );
        }
        removeContextStack();

        return viret;
    } else {
        scope_stack.pop_back();
        return NULL;
    }
}

/* *********************************************************** */

void Mission::doArguments( missionNode *node, int mode, varInstMap *varmap )
{
    int nr_arguments = 0;
    if (mode == SCRIPT_PARSE) {
        if (parsemode == PARSE_DECL) {
            vector< easyDomNode* >::const_iterator siter;
            for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
                missionNode *snode = (missionNode*) *siter;
                if (snode->tag == DTAG_DEFVAR) {
                    doDefVar( snode, mode );
                    nr_arguments++;
                } else {
                    fatalError( node, mode, "only defvars allowed below argument node" );
                    assert( 0 );
                }
            }
            node->script.nr_arguments = nr_arguments;
        }
    }
    nr_arguments = node->script.nr_arguments;
    if (mode == SCRIPT_RUN) {
        if (varmap) {
            int nr_called = varmap->size();
            if (nr_arguments != nr_called) {
                fatalError( node, mode, "wrong number of args in doScript " );
                assert( 0 );
            }
            for (int i = 0; i < nr_arguments; i++) {
                missionNode *defnode = (missionNode*) node->subnodes[i];

                doDefVar( defnode, mode );
                varInst     *vi = doVariable( defnode, mode );

                varInst     *call_vi = (*varmap)[defnode->script.name];
                if (call_vi == NULL) {
                    fatalError( node, mode, "argument var "+node->script.name+" no found in varmap" );
                    assert( 0 );
                }
                assignVariable( vi, call_vi );
            }
        } else
        //no varmap == 0 args
        if (nr_arguments != 0) {
            fatalError( node, mode, "doScript expected to be called with arguments" );
            assert( 0 );
        }
    }
    if (mode == SCRIPT_PARSE) {
        if (parsemode == PARSE_DECL) {
            missionNode *exec_scope = scope_stack.back();
            exec_scope->script.nr_arguments = nr_arguments;
            node->script.nr_arguments = nr_arguments;
        }
    }
}

/* *********************************************************** */

void Mission::doReturn( missionNode *node, int mode )
{
    trace( node, mode );
    if (mode == SCRIPT_PARSE) {
        missionNode *script = current_script;

        node->script.exec_node = script;
    }
    int len = node->subnodes.size();
    varInst     *vi     = newVarInst( VI_LOCAL );

    missionNode *script = node->script.exec_node;
    if (script->script.vartype == VAR_VOID) {
        if (len != 0) {
            fatalError( node, mode, "script returning void, but return statement with node" );
            assert( 0 );
        }
    } else {
        //return something non-void
        if (len != 1) {
            fatalError( node, mode, "return statement needs only one subnode" );
            assert( 0 );
        }
        missionNode *expr = (missionNode*) node->subnodes[0];
        if (script->script.vartype == VAR_BOOL) {
            bool res = checkBoolExpr( expr, mode );
            vi->bool_val = res;
        } else if (script->script.vartype == VAR_FLOAT) {
            double res = checkFloatExpr( expr, mode );
            vi->float_val = res;
        } else if (script->script.vartype == VAR_INT) {
            int res = checkIntExpr( expr, mode );
            vi->int_val = res;
        } else if (script->script.vartype == VAR_OBJECT) {
            varInst *vi2 = checkObjectExpr( expr, mode );
            vi->type = VAR_OBJECT;
            assignVariable( vi, vi2 );
        } else {
            fatalError( node, mode, "unkown variable type" );
            assert( 0 );
        }
    }
    if (mode == SCRIPT_RUN) {
        contextStack *cstack = runtime.cur_thread->exec_stack.back();

        vi->type = script->script.vartype;

        cstack->return_value = vi;
    }
}

/* *********************************************************** */

void Mission::doBlock( missionNode *node, int mode )
{
    trace( node, mode );
    if (mode == SCRIPT_PARSE)
        scope_stack.push_back( node );
    if (mode == SCRIPT_RUN)
        addContext( node );
    vector< easyDomNode* >::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end() && !have_return( mode ); siter++) {
        missionNode *snode = (missionNode*) *siter;
        checkStatement( snode, mode );
    }
    if (mode == SCRIPT_RUN)
        removeContext();
    if (mode == SCRIPT_PARSE)
        scope_stack.pop_back();
}

/* *********************************************************** */

varInst* Mission::doExec( missionNode *node, int mode )
{
    trace( node, mode );
    if (mode == SCRIPT_PARSE) {
        string name = node->attr_value( "name" );
        if ( name.empty() ) {
            fatalError( node, mode, "you have to give name to exec" );
            assert( 0 );
        }
        node->script.name = name;

        string use_modstr   = node->attr_value( "module" );
        missionNode *module = NULL;
        missionNode *script = NULL;
        if ( !use_modstr.empty() ) {
            module = runtime.modules[use_modstr];
        } else {
            module = current_module;
        }
        if (module == NULL) {
            fatalError( node, mode, "module "+use_modstr+" not found" );
            assert( 0 );
        }
        script = module->script.scripts[name];
        if (script == NULL) {
            fatalError( node, mode, "script "+name+" not found in module "+use_modstr );
            assert( 0 );
        }
        node->script.exec_node   = script;
        node->script.vartype     = script->script.vartype;
        node->script.module_node = module;
    }
    missionNode *arg_node = node->script.exec_node->script.argument_node;

    int nr_arguments;
    if (arg_node == NULL)
        nr_arguments = 0;
    else
        nr_arguments = arg_node->script.nr_arguments;
    int nr_exec_args = node->subnodes.size();
    if (nr_arguments != nr_exec_args) {
        char buffer[200];
        sprintf( buffer, "wrong nr of arguments in doExec=%d doScript=%d", nr_exec_args, nr_arguments );
        fatalError( node, mode, buffer );
        assert( 0 );
    }
    varInstMap *varmap = NULL;
    if (nr_arguments > 0) {
        varmap = new varInstMap;
        for (int i = 0; i < nr_arguments; i++) {
            missionNode *defnode  = (missionNode*) arg_node->subnodes[i];
            missionNode *callnode = (missionNode*) node->subnodes[i];

            varInst     *vi = newVarInst( VI_LOCAL );
            vi->type = defnode->script.vartype;
            if (defnode->script.vartype == VAR_FLOAT) {
                debug( 4, node, mode, "doExec checking floatExpr" );
                double res = checkFloatExpr( callnode, mode );
                vi->float_val = res;
            } else if (defnode->script.vartype == VAR_INT) {
                debug( 4, node, mode, "doExec checking intExpr" );
                int res = checkIntExpr( callnode, mode );
                vi->int_val = res;
            } else if (defnode->script.vartype == VAR_BOOL) {
                debug( 4, node, mode, "doExec checking boolExpr" );
                bool ok = checkBoolExpr( callnode, mode );
                vi->bool_val = ok;
            } else if (defnode->script.vartype == VAR_OBJECT) {
                debug( 3, node, mode, "doExec checking objectExpr" );
                varInst *ovi = checkObjectExpr( callnode, mode );
                vi->type = VAR_OBJECT;
                if (mode == SCRIPT_RUN)
                    assignVariable( vi, ovi );
                deleteVarInst( ovi );
            } else {
                fatalError( node, mode, "unsupported vartype in doExec" );
                assert( 0 );
            }
            (*varmap)[defnode->script.name] = vi;
        }
    }
    if (mode == SCRIPT_RUN) {
        //SCRIPT_RUN

        debug( 4, node, mode, "executing "+node->script.name );

        missionNode *module     = node->script.module_node;

        missionNode *old_module = runtime.cur_thread->module_stack.back();

        unsigned int classid    = 0;
        if (old_module == module)
            classid = runtime.cur_thread->classid_stack.back();
        runtime.cur_thread->module_stack.push_back( module );
        runtime.cur_thread->classid_stack.push_back( classid );

        varInst *vi = doScript( node->script.exec_node, mode, varmap );

        runtime.cur_thread->module_stack.pop_back();
        runtime.cur_thread->classid_stack.pop_back();
        if (varmap) {
            deleteVarMap( varmap );
            delete varmap;
        }
        return vi;
    }
    //SCRIPT_PARSE

    varInst *vi = newVarInst( VI_TEMP );

    vi->type = node->script.exec_node->script.vartype;

    return vi;
}

varInst* Mission::newVarInst( scope_type scopetype )
{
    varInst *vi = new varInst( scopetype );
    vi_counter++;

    return vi;
}

void Mission::deleteVarInst( varInst *vi, bool del_local )
{
    if (vi == NULL)
        return;
    if (vi->scopetype == VI_GLOBAL || vi->scopetype == VI_MODULE || vi->scopetype == VI_CLASSVAR) {
        debug( 12, NULL, 0, "reqested to delete global/module vi\n" );
    } else if (vi->scopetype == VI_ERROR) {
        debug( 2, NULL, 0, "reqested to delete vi_error\n" );
    } else if (del_local == false && vi->scopetype == VI_IN_OBJECT) {
        //debug(2,NULL,0,"reqested to delete vi in object\n");
    } else if (vi->scopetype == VI_CONST) {
        debug( 12, NULL, 0, "reqested to delete const vi\n" );
    } else if (del_local == false && vi->scopetype == VI_LOCAL) {
        debug( 12, NULL, 0, "reqested to delete local vi\n" );
    } else {
        delete vi;
        vi_counter--;
    }
}

void Mission::deleteVarMap( varInstMap *vmap )
{
    vsUMap< string, varInst* >::const_iterator iter;
    for (iter = vmap->begin(); iter != vmap->end(); iter++) {
        varInst *vi = (*iter).second;
        if (vi == NULL)
            debug( 12, NULL, 0, "NULLVAR "+(*iter).first+"\n" );
        else
            deleteVarInst( vi, true );
    }
}

unsigned int Mission::createClassInstance( string modulename )
{
    missionNode *module_node = runtime.modules[modulename];
    if (module_node == NULL) {
        fatalError( NULL, SCRIPT_RUN, "module "+modulename+" not found" );
        assert( 0 );
    }
    module_node->script.classinst_counter++;

    char buf[200];
    sprintf( buf, "class counter for module %s : %d\n", modulename.c_str(), module_node->script.classinst_counter );
    debug( 1, NULL, 0, buf );

    varInstMap *cvmap = new varInstMap();

    module_node->script.classvars.push_back( cvmap );

    varInstMap *cvmap0 = module_node->script.classvars[0];

    vsUMap< string, varInst* >::const_iterator iter;
    for (iter = cvmap0->begin(); iter != cvmap0->end(); iter++) {
        varInst *vi0 = (*iter).second;
        string   vi0_name = (*iter).first;

        varInst *vi  = newVarInst( VI_CLASSVAR );
        vi->type = vi0->type;
        assignVariable( vi, vi0 );

        (*cvmap)[vi0_name] = vi;
    }
    return module_node->script.classinst_counter;
}

void Mission::destroyClassInstance( string modulename, unsigned int classid )
{
    missionNode *module = runtime.modules[modulename];
    if (module == NULL) {
        fatalError( NULL, SCRIPT_RUN, "module "+modulename+" not found" );
        assert( 0 );
    }
    if ( classid >= module->script.classvars.size() ) {
        fatalError( module, SCRIPT_RUN, "illegal classvar nr." );
        assert( 0 );
    }
    printf( "destroying class instance %s:%d\n", modulename.c_str(), classid );
    varInstMap *cvmap = module->script.classvars[classid];

    deleteVarMap( cvmap );

    module->script.classvars[classid] = NULL;
}

