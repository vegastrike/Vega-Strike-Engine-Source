/*
 * script_expression.cpp
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

#include "cmd/unit_generic.h"
#include "mission.h"
#include "easydom.h"

/* *********************************************************** */

varInst *Mission::checkObjectExpr(missionNode *node, int mode) {
    varInst *res = NULL;
    if (node->tag == DTAG_VAR_EXPR) {
        res = doObjectVar(node, mode);
    } else if (node->tag == DTAG_CALL) {
        varInst *vi = doCall(node, mode);
        if (vi->type == VAR_OBJECT) {
            res = vi;
        } else if (vi->type == VAR_ANY && mode == SCRIPT_PARSE) {
            res = vi;
        } else {
            fatalError(node, mode, "expected a object call, got a different one");
            assert(0);
        }
    } else if (node->tag == DTAG_EXEC) {
        varInst *vi = doExec(node, mode);
        if (vi == NULL) {
            fatalError(node, mode, "doExec returned NULL");
            assert(0);
        } else if (node->script.vartype == VAR_OBJECT) {
            res = vi;
        } else {
            fatalError(node, mode, "expected a object exec, got a different one");
            assert(0);
        }
    } else if (node->tag == DTAG_CONST) {
        varInst *vi = doConst(node, mode);
        if (vi->type == VAR_OBJECT && vi->objectname == "string") {
            res = vi;
        } else {
            fatalError(node, mode, "expected a string const, got a different one: " + vi->objectname);
            assert(0);
        }
    } else {
        fatalError(node, mode, "no such object expression tag");
        assert(0);
    }
    return res;
}

/* *********************************************************** */

varInst *Mission::doMath(missionNode *node, int mode) {
    string mathname = node->attr_value("math");

    int len = node->subnodes.size();
    if (len < 2) {
        fatalError(node, mode, "math needs at least 2 arguments");
        assert(0);
    }
    varInst *res_vi = newVarInst(VI_TEMP);

    varInst *res1_vi = checkExpression((missionNode *) node->subnodes[0], mode);
    if (res1_vi->type != VAR_INT && res1_vi->type != VAR_FLOAT && res1_vi->type != VAR_ANY) {
        printf("res1_vi=%d\n", res1_vi->type);
        fatalError(node, mode, "only int or float expr allowed for math");
        assert(0);
    }
    res_vi->type = res1_vi->type;
    assignVariable(res_vi, res1_vi);
    if (res_vi->type == VAR_ANY) {
        res_vi->type = VAR_FLOAT;
    }
    deleteVarInst(res1_vi);
    for (int i = 1; i < len; i++) {
        varInst *res2_vi = checkExpression((missionNode *) node->subnodes[i], mode);
        var_type res2_type = res2_vi->type;
        if (res2_type == VAR_INT && res_vi->type == VAR_FLOAT) {
            res2_type = VAR_FLOAT;
            if (mode == SCRIPT_RUN) {
                float res2 = (float) res2_vi->int_val;
                float res = floatMath(mathname, res_vi->float_val, res2);
                res_vi->float_val = res;
            }
        } else if (res2_type == VAR_FLOAT && res_vi->type == VAR_INT) {
            res_vi->type = VAR_FLOAT;
            if (mode == SCRIPT_RUN) {
                res_vi->float_val = (float) res_vi->int_val;
                float res2 = res2_vi->float_val;
                float res = floatMath(mathname, res_vi->float_val, res2);
                res_vi->float_val = res;
            }
        } else {
            if (res_vi->type != res2_type) {
                fatalError(node, mode, "can't do math on such types");
                assert(0);
            }
            if (mode == SCRIPT_RUN) {
                if (res_vi->type == VAR_INT) {
                    int res = intMath(mathname, res_vi->int_val, res2_vi->int_val);
                    res_vi->int_val = res;
                } else if (res_vi->type == VAR_FLOAT) {
                    float res = floatMath(mathname, res_vi->float_val, res2_vi->float_val);
                    res_vi->float_val = res;
                } else if (res_vi->type != res2_type) {
                    fatalError(node, mode, "can't do math on such types");
                    assert(0);
                }
            }             //of SCRIPT_RUN
        }         //else
        deleteVarInst(res2_vi);
    }     //for arguments

    return res_vi;
}

int Mission::intMath(string mathname, int res1, int res2) {
    int res = res1;
    if (mathname == "+") {
        res = res + res2;
    } else if (mathname == "-") {
        res = res - res2;
    } else if (mathname == "*") {
        res = res * res2;
    } else if (mathname == "/") {
        res = res / res2;
    } else {
        fatalError(NULL, SCRIPT_RUN, "no such intmath expression");
        assert(0);
    }
    return res;
}

/* *********************************************************** */
double Mission::floatMath(string mathname, double res1, double res2) {
    double res = res1;
    if (mathname == "+") {
        res = res + res2;
    } else if (mathname == "-") {
        res = res - res2;
    } else if (mathname == "*") {
        res = res * res2;
    } else if (mathname == "/") {
        res = res / res2;
    } else {
        fatalError(NULL, SCRIPT_RUN, "no such floatmath expression");
        assert(0);
    }
    return res;
}

/* *********************************************************** */

double Mission::doFMath(missionNode *node, int mode) {
    varInst *math_vi = doMath(node, mode);
    if (math_vi->type != VAR_FLOAT) {
        fatalError(node, mode, "fmath expected float");
        assert(0);
    }
    double ret = math_vi->float_val;
    deleteVarInst(math_vi);

    return ret;

    string mathname = node->attr_value("math");

    int len = node->subnodes.size();
    if (len < 2) {
        fatalError(node, mode, "fmath needs at least 2 arguments");
        assert(0);
    }
    double res = checkFloatExpr((missionNode *) node->subnodes[0], mode);

    char buffer[200];
    sprintf(buffer, "fmath: 1st expr returns %f", res);
    debug(4, node, mode, buffer);
    for (int i = 1; i < len; i++) {
        double res2 = checkFloatExpr((missionNode *) node->subnodes[i], mode);
        if (mode == SCRIPT_RUN) {
            if (mathname == "+") {
                res = res + res2;
            } else if (mathname == "-") {
                res = res - res2;
            } else if (mathname == "*") {
                res = res * res2;
            } else if (mathname == "/") {
                res = res / res2;
            } else {
                fatalError(node, mode, "no such fmath expression");
                assert(0);
            }
        }
    }
    if (mode == SCRIPT_RUN) {
        return res;
    }
    return 0.0;
}

/* *********************************************************** */

int Mission::doIMath(missionNode *node, int mode) {
    varInst *math_vi = doMath(node, mode);
    if (math_vi->type != VAR_INT) {
        fatalError(node, mode, "fmath expected int");
        assert(0);
    }
    int res = math_vi->int_val;
    deleteVarInst(math_vi);

    return res;

#if 0
    //if(mode==SCRIPT_PARSE){
    string mathname = node->attr_value( "math" );

    int    len = node->subnodes.size();
    if (len < 2) {
        fatalError( node, mode, "imath needs at least 2 arguments" );
        assert( 0 );
    }
    int  res = checkIntExpr( (missionNode*) node->subnodes[0], mode );

    char buffer[200];
    sprintf( buffer, "imath: 1st expr returns %d", res );
    debug( 4, node, mode, buffer );
    for (int i = 1; i < len; i++) {
        int res2 = checkIntExpr( (missionNode*) node->subnodes[i], mode );
        if (mode == SCRIPT_RUN) {
            if (mathname == "+") {
                res = res+res2;
            } else if (mathname == "-") {
                res = res-res2;
            } else if (mathname == "*") {
                res = res*res2;
            } else if (mathname == "/") {
                res = res/res2;
            } else {
                fatalError( node, mode, "no such imath expression" );
                assert( 0 );
            }
        }
    }
    if (mode == SCRIPT_RUN)
        return res;
    return 0;
#endif
}

/* *********************************************************** */

double Mission::checkFloatExpr(missionNode *node, int mode) {
    double res = 0.0;
    if (node->tag == DTAG_VAR_EXPR) {
        res = doFloatVar(node, mode);
    } else if (node->tag == DTAG_FMATH) {
        res = doFMath(node, mode);
    } else if (node->tag == DTAG_CONST) {
        varInst *vi = doConst(node, mode);
        if (vi && vi->type == VAR_FLOAT) {
            res = vi->float_val;
        } else {
            fatalError(node, mode, "expected a float const, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_CALL) {
        varInst *vi = doCall(node, mode);
        if (vi->type == VAR_FLOAT) {
            res = vi->float_val;
        } else if (vi->type == VAR_ANY && mode == SCRIPT_PARSE) {
            res = vi->float_val;
        } else {
            fatalError(node, mode, "expected a float call, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_EXEC) {
        varInst *vi = doExec(node, mode);
        if (vi == NULL) {
            fatalError(node, mode, "doExec returned NULL");
            assert(0);
        } else if (node->script.vartype == VAR_FLOAT) {
            res = vi->float_val;
        } else {
            fatalError(node, mode, "expected a float exec, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else {
        fatalError(node, mode, "no such float expression tag");
        assert(0);
    }
    return res;
}

/* *********************************************************** */

int Mission::checkIntExpr(missionNode *node, int mode) {
    int res = 0;
    if (node->tag == DTAG_VAR_EXPR) {
        res = doIntVar(node, mode);
    } else if (node->tag == DTAG_FMATH) {
        res = doIMath(node, mode);
    } else if (node->tag == DTAG_CONST) {
        varInst *vi = doConst(node, mode);
        if (vi && vi->type == VAR_INT) {
            res = vi->int_val;
        } else {
            fatalError(node, mode, "expected a float const, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_CALL) {
        varInst *vi = doCall(node, mode);
        if (vi->type == VAR_INT) {
            res = vi->int_val;
        } else if (vi->type == VAR_ANY && mode == SCRIPT_PARSE) {
            res = vi->int_val;
        } else {
            fatalError(node, mode, "expected a int call, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_EXEC) {
        varInst *vi = doExec(node, mode);
        if (vi == NULL) {
            fatalError(node, mode, "doExec returned NULL");
            assert(0);
        } else if (node->script.vartype == VAR_INT) {
            res = vi->int_val;
        } else {
            fatalError(node, mode, "expected a int exec, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else {
        fatalError(node, mode, "no such int expression tag");
        assert(0);
    }
    return res;
}

/* *********************************************************** */

bool Mission::checkBoolExpr(missionNode *node, int mode) {
    bool ok = false;
    //no difference between parse/run
    if (node->tag == DTAG_AND_EXPR) {
        ok = doAndOr(node, mode);
    } else if (node->tag == DTAG_OR_EXPR) {
        ok = doAndOr(node, mode);
    } else if (node->tag == DTAG_NOT_EXPR) {
        ok = doNot(node, mode);
    } else if (node->tag == DTAG_TEST_EXPR) {
        ok = doTest(node, mode);
    } else if (node->tag == DTAG_VAR_EXPR) {
        ok = doBooleanVar(node, mode);
    } else if (node->tag == DTAG_CONST) {
        varInst *vi = doConst(node, mode);
        if (vi->type == VAR_BOOL) {
            ok = vi->bool_val;
        } else {
            fatalError(node, mode, "expected a bool const, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_CALL) {
        varInst *vi = doCall(node, mode);
        if (vi->type == VAR_BOOL) {
            ok = vi->bool_val;
        } else if (vi->type == VAR_ANY && mode == SCRIPT_PARSE) {
            ok = vi->bool_val;
        } else {
            fatalError(node, mode, "expected a bool call, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_EXEC) {
        varInst *vi = doExec(node, mode);
        if (vi == NULL) {
            fatalError(node, mode, "doExec returned NULL");
            assert(0);
            //parsing?
        } else if (node->script.vartype == VAR_BOOL) {
            ok = vi->bool_val;
        } else {
            fatalError(node, mode, "expected a bool exec, got a different one");
            assert(0);
        }
        deleteVarInst(vi);
    } else {
        fatalError(node, mode, "no such boolean expression tag");
        assert(0);
    }
    return ok;
}

/* *********************************************************** */

bool Mission::doAndOr(missionNode *node, int mode) {
    bool ok; //FIXME !! Not all branches result in ok being initialized
    ok = true; //this line added temporarily by chuck_starchaser
    //no difference between parse/run
    if (node->tag == DTAG_AND_EXPR) {
        ok = true;
    } else if (node->tag == DTAG_OR_EXPR) {
        ok = false;
    }
    vector<easyDomNode *>::const_iterator siter;

    int i = 0;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++, i++) {
        missionNode *snode = (missionNode *) *siter;
        bool res = checkBoolExpr(snode, mode);
        if (node->tag == DTAG_AND_EXPR) {
            ok = ok && res;
        } else if (node->tag == DTAG_OR_EXPR) {
            ok = ok || res;
        }
    }
    if (mode == SCRIPT_PARSE) {
        if (i < 2) {
            warning("less than two arguments for and/or");
        }
    }
    return ok; //FIXME ok not initialized by all paths --chuck_starchaser
}

/* *********************************************************** */

bool Mission::doNot(missionNode *node, int mode) {
    bool ok;

    //no difference between parse/run

    missionNode *snode = (missionNode *) node->subnodes[0];
    if (snode) {
        ok = checkBoolExpr(snode, mode);

        return !ok;
    } else {
        fatalError(node, mode, "no subnode in not");
        assert(0);
        return false;         //we'll never get here
    }
}

/* *********************************************************** */

bool Mission::doTest(missionNode *node, int mode) {
    if (mode == SCRIPT_PARSE) {
        string teststr = node->attr_value("test");
        if (teststr.empty()) {
            fatalError(node, mode, "you have to give test an argument what to test");
            assert(0);
        }
        if (teststr == "gt") {
            node->script.tester = TEST_GT;
        } else if (teststr == "lt") {
            node->script.tester = TEST_LT;
        } else if (teststr == "eq") {
            node->script.tester = TEST_EQ;
        } else if (teststr == "ne") {
            node->script.tester = TEST_NE;
        } else if (teststr == "ge") {
            node->script.tester = TEST_GE;
        } else if (teststr == "le") {
            node->script.tester = TEST_LE;
        }
#if 0
            else if (teststr == "between") {
                node->script.tester = TEST_BETWEEN;
            }
#endif

        else {
            fatalError(node, mode, "unknown test argument for test");
            assert(0);
        }

        vector<easyDomNode *>::const_iterator siter;
#if 0
        int i = 0;
        for (siter = node->subnodes.begin(); siter != node->subnodes.end() && i < 2; siter++) {
            missionNode *snode = (missionNode*) *siter;
            (node->script.test_arg)[i] = snode;
        }
        if (i < 2) {
            fatalError( node, mode, "a test-expr needs exact two subnodes" );
            assert( 0 );
        }
#endif

        int len = node->subnodes.size();
        if (len != 2) {
            fatalError(node, mode, "a test-expr needs exact two subnodes");
            assert(0);
        }
        node->script.test_arg[0] = (missionNode *) node->subnodes[0];
        node->script.test_arg[1] = (missionNode *) node->subnodes[1];
    }     //end of parse

    varInst *arg1_vi = checkExpression(node->script.test_arg[0], mode);
    varInst *arg2_vi = checkExpression(node->script.test_arg[1], mode);
    bool res = false;
    if (arg1_vi->type != arg2_vi->type) {
        fatalError(node, mode, "test is getting not the same types");
        assert(0);
    }
    if (mode == SCRIPT_RUN) {
        if (arg1_vi->type == VAR_FLOAT) {
            double arg1 = arg1_vi->float_val;
            double arg2 = arg2_vi->float_val;
            switch (node->script.tester) {
                case TEST_GT:
                    res = (arg1 > arg2);
                    break;
                case TEST_LT:
                    res = (arg1 < arg2);
                    break;
                case TEST_EQ:
                    res = (arg1 == arg2);
                    break;
                case TEST_NE:
                    res = (arg1 != arg2);
                    break;
                case TEST_GE:
                    res = (arg1 >= arg2);
                    break;
                case TEST_LE:
                    res = (arg1 <= arg2);
                    break;
                default:
                    fatalError(node, mode, "no valid tester");
                    assert(0);
            }
        } else if (arg1_vi->type == VAR_INT) {
            int arg1 = arg1_vi->int_val;
            int arg2 = arg2_vi->int_val;
            switch (node->script.tester) {
                case TEST_GT:
                    res = (arg1 > arg2);
                    break;
                case TEST_LT:
                    res = (arg1 < arg2);
                    break;
                case TEST_EQ:
                    res = (arg1 == arg2);
                    break;
                case TEST_NE:
                    res = (arg1 != arg2);
                    break;
                case TEST_GE:
                    res = (arg1 >= arg2);
                    break;
                case TEST_LE:
                    res = (arg1 <= arg2);
                    break;
                default:
                    fatalError(node, mode, "no valid tester");
                    assert(0);
            }
        } else {
            fatalError(node, mode, "no such type allowed for test");
            assert(0);
        }
    }     //SCRIPT_RUN

    deleteVarInst(arg1_vi);
    deleteVarInst(arg2_vi);

    return res;
}

/* *********************************************************** */

varInst *Mission::checkExpression(missionNode *node, int mode) {
    varInst *ret = NULL;
    debug(3, node, mode, "checking expression");
    switch (node->tag) {
        case DTAG_AND_EXPR:
        case DTAG_OR_EXPR:
        case DTAG_NOT_EXPR:
        case DTAG_TEST_EXPR: {
            bool res = checkBoolExpr(node, mode);
            ret = newVarInst(VI_TEMP);
            ret->type = VAR_BOOL;
            ret->bool_val = res;
            break;
        }
        case DTAG_CONST:
            ret = doConst(node, mode);
            break;
        case DTAG_VAR_EXPR:
            ret = doVariable(node, mode);
            break;
        case DTAG_FMATH:
            ret = doMath(node, mode);
            break;
        case DTAG_CALL:
            ret = doCall(node, mode);
            break;
        case DTAG_EXEC:
            ret = doExec(node, mode);
            break;
        default:
            fatalError(node, mode, "no such expression");
            assert(0);
            break;
    }
    return ret;
}

