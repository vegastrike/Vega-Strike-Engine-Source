/*
 * script_callbacks.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, alexannika, ace123, surfdargent,
 * khepri, klaussfriere, pyramid3d, dan_w, pheonixstorm, Roy Falk,
 * Stephen G. Tuggy, and other Vega Strike Contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

/// Parses functions for python modules

#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "root_generic/xml_support.h"
#include "src/vegastrike.h"
#include "root_generic/lin_time.h"

#include "cmd/unit_generic.h"
#include "mission.h"
#include "root_generic/easydom.h"

#include "msgcenter.h"

#include "src/star_system.h"
#include "src/universe.h"

using std::cout;
using std::cerr;
using std::endl;

string varToString(varInst *vi) {
    switch (vi->type) {
        case VAR_FLOAT:
            return XMLSupport::tostring((float) vi->float_val);
        case VAR_INT:
            return XMLSupport::tostring(vi->int_val);
        case VAR_BOOL:
            return XMLSupport::tostring(vi->bool_val);
        case VAR_OBJECT:
        default:
            if (vi->objectname == "string") {
                return *((string *) vi->object);
            } else {
                return XMLSupport::tostring((long) vi->object);
            }
    }
}

void Mission::doCall_toxml(string module, varInst *ovi) {
    if (module == "_olist") {
        call_olist_toxml(NULL, SCRIPT_RUN, ovi);
    } else if (module == "_unit") {
        call_unit_toxml(NULL, SCRIPT_RUN, ovi);
    }
}

varInst *Mission::doCall(missionNode *node, int mode, string module, string method) {
    varInst *vi = NULL;
    callback_module_type module_id = node->script.callback_module_id;
    if (module_id == CMT_UNIT) {
        vi = call_unit(node, mode);
    } else if (module_id == CMT_STD) {
        if (mode == SCRIPT_PARSE) {
            node->script.method_id = module_std_map[method];
        }
        callback_module_std_type method_id = (callback_module_std_type) node->script.method_id;
        if (method_id == CMT_STD_Rnd) {
            vi = callRnd(node, mode);
        } else if (method_id == CMT_STD_getGameTime) {
            vi = callGetGameTime(node, mode);
        } else if (method_id == CMT_STD_ResetTimeCompression) {
            vi = callResetTimeCompression(node, mode);
        } else if (0) {
            vi = callGetSystemName(node, mode);
        } else if (method_id == CMT_STD_getSystemFile) {
            vi = callGetSystemFile(node, mode);
        } else if (method_id == CMT_STD_getCurrentAIUnit) {
            vi = callGetCurrentAIUnit(node, mode);
        } else if (method_id == CMT_STD_getCurrentAIOrder) {
            vi = callGetCurrentAIOrder(node, mode);
        } else if (method_id == CMT_STD_isNull) {
            vi = call_isNull(node, mode);
        } else if (method_id == CMT_STD_setNull) {
            vi = call_setNull(node, mode);
        } else if (method_id == CMT_STD_equal) {
            vi = call_isequal(node, mode);
        } else if (method_id == CMT_STD_Float) {
            vi = call_float_cast(node, mode);
        } else if (method_id == CMT_STD_Int) {
            vi = call_int_cast(node, mode);
        } else if (method_id == CMT_STD_getGalaxyProperty) {
            vi = callGetGalaxyProperty(node, mode);
        } else if (method_id == CMT_STD_musicAddList) {
            vi = call_musicAddList(node, mode);
        } else if (method_id == CMT_STD_musicPlaySong) {
            vi = call_musicPlaySong(node, mode);
        } else if (method_id == CMT_STD_musicPlayList) {
            vi = call_musicPlayList(node, mode);
        } else if (method_id == CMT_STD_getDifficulty) {
            vi = newVarInst(VI_TEMP);
            vi->type = VAR_FLOAT;
            vi->float_val = g_game.difficulty;
        } else if (method_id == CMT_STD_setDifficulty) {
            float diff = getFloatArg(node, mode, 0);
            if (mode == SCRIPT_RUN) {
                g_game.difficulty = diff;
            }
            vi = newVarInst(VI_TEMP);
            vi->type = VAR_VOID;
        } else if (method_id == CMT_STD_getNumAdjacentSystems) {
            vi = callGetNumAdjacentSystems(node, mode);
        } else if (method_id == CMT_STD_getAdjacentSystem) {
            vi = callGetAdjacentSystem(node, mode);
        } else if (method_id == CMT_STD_terminateMission) {
            vi = call_terminateMission(node, mode);
        } else if (method_id == CMT_STD_playSound) {
            std::string soundName = getStringArgument(node, mode, 0);
            QVector loc;
            loc.i = getFloatArg(node, mode, 1);
            loc.j = getFloatArg(node, mode, 2);
            loc.k = getFloatArg(node, mode, 3);
            Vector speed(0, 0, 0);
            if (node->subnodes.size() > 6) {
                speed.i = getFloatArg(node, mode, 4);
                speed.j = getFloatArg(node, mode, 5);
                speed.k = getFloatArg(node, mode, 6);
            }
            vi = newVarInst(VI_TEMP);
            vi->type = VAR_VOID;
        } else if (method_id == CMT_STD_playSoundCockpit) {
            std::string soundName = getStringArgument(node, mode, 0);
        } else if (method_id == CMT_STD_playAnimation) {
            std::string aniName = getStringArgument(node, mode, 0);
            QVector loc(0, 0, 0);
            loc.i = getFloatArg(node, mode, 1);
            loc.j = getFloatArg(node, mode, 2);
            loc.k = getFloatArg(node, mode, 3);
            vi = newVarInst(VI_TEMP);
            vi->type = VAR_VOID;
        }
    } else if (module_id == CMT_OLIST) {
        vi = call_olist(node, mode);
    } else if (module_id == CMT_OMAP) {
        vi = call_omap(node, mode);
    } else if (module_id == CMT_ORDER) {
        vi = call_order(node, mode);
    } else if (module_id == CMT_STRING) {
        vi = call_string(node, mode);
    } else if (module_id == CMT_IO) {
        if (method == "PrintFloats") {
            vi = callPrintFloats(node, mode);
        } else if (method == "printf") {
            vi = call_io_printf(node, mode);
        } else if (method == "sprintf") {
            vi = call_io_sprintf(node, mode);
        } else if (method == "message") {
            vi = call_io_message(node, mode);
        } else if (method == "printMsgList") {
            vi = call_io_printmsglist(node, mode);
        }
    } else if (module_id == CMT_BRIEFING) {
        vi = call_briefing(node, mode);
    }
    return vi;
}

varInst *Mission::doCall(missionNode *node, int mode) {
    trace(node, mode);
    if (mode == SCRIPT_PARSE) {
        string name = node->attr_value("name");
        string module = node->attr_value("module");
        string object = node->attr_value("object");
        if (object.empty() && module.empty()) {
            fatalError(node, mode, "you have to give a callback object or module");
            assert(0);
        }
        if (name.empty()) {
            fatalError(node, mode, "you have to give a callback name");
            assert(0);
        }
        node->script.name = name;
        callback_module_type module_id = module_map[module];
        node->script.callback_module_id = module_id;
    }
    //RUNTIME && PARSE
    string module = node->attr_value("module");
    if (module.empty()) {
        //does not work yet
        string object = node->attr_value("object");
        assert(0);
        varInst *ovi = NULL;
        if (ovi == NULL) {
            fatalError(node, mode, "no object found with name " + object);
            assert(0);
        }
        if (ovi->type != VAR_OBJECT) {
            fatalError(node, mode, "given variable" + object + " is not an object");
            assert(0);
        }
        module = ovi->objectname;
        if (module.empty()) {
            fatalError(node, mode, "object " + object + " not yet initialized");
            assert(0);
        }
        module = "_" + module;
    }
    string method = node->script.name;
    varInst *vi = NULL;
    vi = doCall(node, mode, module, method);
    if (vi == NULL) {
        fatalError(node, mode, "no such callback named " + module + "." + node->script.name);
        assert(0);
    }
    return vi;
}

varInst *Mission::call_isNull(missionNode *node, int mode) {
    varInst *ovi = getObjectArg(node, mode);
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_BOOL;
    viret->bool_val = (ovi->object == NULL);
    deleteVarInst(ovi);
    return viret;
}

varInst *Mission::call_setNull(missionNode *node, int mode) {
    varInst *ovi = getObjectArg(node, mode);
    ovi->object = NULL;
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    deleteVarInst(ovi);
    return viret;
}

varInst *Mission::call_terminateMission(missionNode *node, int mode) {
    getBoolArg(node, mode, 0);
    if (mode == SCRIPT_RUN) {
        terminateMission();
    }
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    return viret;
}

varInst *Mission::call_float_cast(missionNode *node, int mode) {
    missionNode *snode = getArgument(node, mode, 0);
    int intval = checkIntExpr(snode, mode);
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_FLOAT;
    viret->float_val = (float) intval;
    return viret;
}

varInst *Mission::call_int_cast(missionNode *node, int mode) {
    missionNode *snode = getArgument(node, mode, 0);
    double floatval = checkFloatExpr(snode, mode);
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_INT;
    viret->int_val = (int) floatval;
    return viret;
}

varInst *Mission::call_isequal(missionNode *node, int mode) {
    varInst *ovi = getObjectArg(node, mode);
    missionNode *other_node = getArgument(node, mode, 1);
    varInst *other_vi = checkObjectExpr(other_node, mode);
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_BOOL;
    bool res = false;
    if (mode == SCRIPT_RUN) {
        if (other_vi->objectname == ovi->objectname) {
            if (other_vi->object == ovi->object) {
                res = true;
            }
        }
    }
    deleteVarInst(ovi);
    deleteVarInst(other_vi);
    viret->bool_val = res;
    return viret;
}

varInst *Mission::callGetGameTime(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_FLOAT;
    if (mode == SCRIPT_RUN) {
        vi->float_val = gametime;
    }
    return vi;
}

varInst *Mission::callResetTimeCompression(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_VOID;
    if (mode == SCRIPT_RUN) {
        setTimeCompression(1.0);
    }
    return vi;
}

varInst *Mission::callGetSystemName(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "string";
    if (mode == SCRIPT_RUN) {
        deleteVarInst(vi);
        StarSystem *ssystem = _Universe->activeStarSystem();
        string sysname = ssystem->getName();
        vi = call_string_new(node, mode, sysname);
    }
    return vi;
}

varInst *Mission::callGetSystemFile(missionNode *node, int mode, StarSystem *ss) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "string";
    if (mode == SCRIPT_RUN) {
        deleteVarInst(vi);
        if (ss == NULL) {
            ss = _Universe->activeStarSystem();
        }
        string sysname = ss->getFileName();
        vi = call_string_new(node, mode, sysname);
    }
    return vi;
}

varInst *Mission::callGetAdjacentSystem(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "string";
    string str = getStringArgument(node, mode, 0);
    int which = (int) getIntArg(node, mode, 1);
    if (mode == SCRIPT_RUN) {
        deleteVarInst(vi);
        const string &sysname = _Universe->getAdjacentStarSystems(str)[which];
        vi = call_string_new(node, mode, sysname);
    }
    return vi;
}

varInst *Mission::callGetGalaxyProperty(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "string";
    string sys = getStringArgument(node, mode, 0);
    string prop = getStringArgument(node, mode, 1);
    if (mode == SCRIPT_RUN) {
        deleteVarInst(vi);
        string sysname = _Universe->getGalaxyProperty(sys, prop);
        vi = call_string_new(node, mode, sysname);
    }
    return vi;
}

varInst *Mission::callGetNumAdjacentSystems(missionNode *node, int mode) {
    string sysname = getStringArgument(node, mode, 0);
    int ret = 0;
    if (mode == SCRIPT_RUN) {
        ret = _Universe->getAdjacentStarSystems(sysname).size();
    }
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_INT;
    vi->int_val = ret;
    return vi;
}

varInst *Mission::call_io_printmsglist(missionNode *node, int mode) {
    int i = 0;
    if (mode == SCRIPT_RUN) {
        gameMessage msg;
        while (msgcenter->last(i, msg, std::vector<std::string>()) && i < 7.0) {
            cout << "MESSAGE" << msg.message << endl;
            i++;
        }
    }
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    return viret;
}

varInst *Mission::call_io_message(missionNode *node, int mode) {
    missionNode *args[3];
    varInst *args_vi[3];
    string args_str[3];
    int delay = (int) getIntArg(node, mode, 0);
    for (int i = 0; i < 3; i++) {
        args[i] = getArgument(node, mode, i + 1);
        args_vi[i] = checkObjectExpr(args[i], mode);
        if (mode == SCRIPT_RUN) {
            args_str[i] = call_string_getstring(node, mode, args_vi[i]);
        }
        deleteVarInst(args_vi[i]);
    }
    if (mode == SCRIPT_RUN) {
        msgcenter->add(args_str[0], args_str[1], args_str[2], delay);
    }
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    return viret;
}

#if 1

string Mission::replaceNewline(string origstr) {
    string ostr = origstr;
    int breakpos = ostr.find("\\n", 0);
    if (breakpos >= 0) {
        string newstr = ostr.replace(breakpos, 2, "\n");
        return replaceNewline(newstr);
    } else {
        return ostr;
    }
}

#endif

varInst *Mission::call_io_sprintf(missionNode *node, int mode) {
    missionNode *outstr_node = getArgument(node, mode, 0);
    varInst *outstr_vi = checkObjectExpr(outstr_node, mode);
    string *outstrptr = getStringObject(outstr_node, mode, outstr_vi);
    char outbuffer[1024];
    string outstring;
    missionNode *stringnode = getArgument(node, mode, 1);
    if (stringnode->tag != DTAG_CONST) {
        fatalError(node, mode, "only const string allowed for second arg of sprintf");
        assert(0);
    }
    varInst *str_vi = checkObjectExpr(stringnode, mode);
    if (str_vi->type != VAR_OBJECT || (str_vi->type == VAR_OBJECT && str_vi->objectname != "string")) {
        fatalError(node, mode, "io.sprintf needs string object as second arg");
        assert(0);
    }
    int nr_of_args = node->subnodes.size();
    int current_arg = 2;
    string *fullstringptr;
    string fullstring;
    fullstringptr = (string *) str_vi->object;
    fullstring = *fullstringptr;
    fullstring = replaceNewline(fullstring);
    string endstring = fullstring;
    while (current_arg < nr_of_args) {
        int breakpos = endstring.find("%", 0);
        string beforestring = endstring.substr(0, breakpos);
        string breakstring = endstring.substr(breakpos, 2);
        if (breakstring[1] == 'f') {
            missionNode *anode = getArgument(node, mode, current_arg);
            double res = checkFloatExpr(anode, mode);
            if (mode == SCRIPT_RUN) {
                sprintf(outbuffer, "%s", beforestring.c_str());
                outstring += outbuffer;
                sprintf(outbuffer, "%f", res);
                outstring += outbuffer;
            }
        } else if (breakstring[1] == 'd') {
            missionNode *anode = getArgument(node, mode, current_arg);
            int res = checkIntExpr(anode, mode);
            if (mode == SCRIPT_RUN) {
                sprintf(outbuffer, "%s", beforestring.c_str());
                outstring += outbuffer;
                sprintf(outbuffer, "%d", res);
                outstring += outbuffer;
            }
        } else if (breakstring[1] == 's') {
            missionNode *anode = getArgument(node, mode, current_arg);
            varInst *res_vi = doObjectVar(anode, mode);
            if (mode == SCRIPT_RUN) {
                if (res_vi->type != VAR_OBJECT || (res_vi->type == VAR_OBJECT && res_vi->objectname != "string")) {
                    fatalError(node, mode, "io.printf needs string object as some arg");
                    assert(0);
                }
                string *strptr = (string *) res_vi->object;
                sprintf(outbuffer, "%s", beforestring.c_str());
                outstring += outbuffer;
                sprintf(outbuffer, "%s", strptr->c_str());
                outstring += outbuffer;
            }
            deleteVarInst(res_vi);
        }
        endstring = endstring.substr(breakpos + 2, endstring.size() - (breakpos + 2));
        current_arg++;
    }
    if (mode == SCRIPT_RUN) {
        sprintf(outbuffer, "%s", endstring.c_str());
        outstring += outbuffer;
        (*outstrptr) = outstring;
    }
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    deleteVarInst(str_vi);
    deleteVarInst(outstr_vi);

    return viret;
}

varInst *Mission::call_io_printf(missionNode *node, int mode) {
    missionNode *stringnode = getArgument(node, mode, 0);
    if (stringnode->tag != DTAG_CONST) {
        fatalError(node, mode, "only const string allowed for first arg of printf");
        assert(0);
    }
    varInst *str_vi = checkObjectExpr(stringnode, mode);
    if (str_vi->type != VAR_OBJECT || (str_vi->type == VAR_OBJECT && str_vi->objectname != "string")) {
        fatalError(node, mode, "io.printf needs string object as first arg");
        assert(0);
    }
    int nr_of_args = node->subnodes.size();
    int current_arg = 1;
    string *fullstringptr;
    string fullstring;
    fullstringptr = (string *) str_vi->object;
    fullstring = *fullstringptr;
    fullstring = replaceNewline(fullstring);
    string endstring = fullstring;
    while (current_arg < nr_of_args) {
        int breakpos = endstring.find("%", 0);
        string beforestring = endstring.substr(0, breakpos);
        string breakstring = endstring.substr(breakpos, 2);
        if (breakstring[1] == 'f') {
            missionNode *anode = getArgument(node, mode, current_arg);
            double res = checkFloatExpr(anode, mode);
            if (mode == SCRIPT_RUN) {
                printf("%s", beforestring.c_str());
                printf("%f", res);
            }
        } else if (breakstring[1] == 'd') {
            missionNode *anode = getArgument(node, mode, current_arg);
            int res = checkIntExpr(anode, mode);
            if (mode == SCRIPT_RUN) {
                printf("%s", beforestring.c_str());
                printf("%d", res);
            }
        } else if (breakstring[1] == 'b') {
            missionNode *anode = getArgument(node, mode, current_arg);
            bool res = checkBoolExpr(anode, mode);
            if (mode == SCRIPT_RUN) {
                printf("%s", beforestring.c_str());
                if (res == true) {
                    printf("true");
                } else {
                    printf("false");
                }
            }
        } else if (breakstring[1] == 's') {
            missionNode *anode = getArgument(node, mode, current_arg);
            varInst *res_vi = doObjectVar(anode, mode);
            if (mode == SCRIPT_RUN) {
                if (res_vi->type != VAR_OBJECT || (res_vi->type == VAR_OBJECT && res_vi->objectname != "string")) {
                    fatalError(node, mode, "io.printf needs string object as some arg");
                    assert(0);
                }
                string *strptr = (string *) res_vi->object;

                printf("%s", beforestring.c_str());
                printf("%s", strptr->c_str());
            }
            deleteVarInst(res_vi);
        }
        endstring = endstring.substr(breakpos + 2, endstring.size() - (breakpos + 2));
        current_arg++;
    }
    if (mode == SCRIPT_RUN) {
        printf("%s", endstring.c_str());
    }
    varInst *viret = newVarInst(VI_TEMP);
    viret->type = VAR_VOID;
    deleteVarInst(str_vi);
    fflush(stdout);
    return viret;
}

varInst *Mission::call_musicAddList(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_INT;
    vi->int_val = 0;
    return vi;
}

varInst *Mission::call_musicPlaySong(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_VOID;
    return vi;
}

varInst *Mission::call_musicPlayList(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_VOID;
    return vi;
}

varInst *Mission::callPrintFloats(missionNode *node, int mode) {
    string s1 = node->attr_value("s1");
    string s2 = node->attr_value("s2");
    if (mode == SCRIPT_RUN) {
        cout << "print: " << s1;
    }
    int len = node->subnodes.size();
    for (int i = 0; i < len; i++) {
        double res = checkFloatExpr((missionNode *) node->subnodes[i], mode);
        if (mode == SCRIPT_RUN) {
            cout << " " << res << " ,";
        }
    }
    if (mode == SCRIPT_RUN) {
        cout << " " << s2 << endl;
    }
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_VOID;

    return vi;
}

varInst *Mission::callGetCurrentAIUnit(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "unit";
    vi->object = (void *) current_ai_unit;

    return vi;
}

varInst *Mission::callGetCurrentAIOrder(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_OBJECT;
    vi->objectname = "order";
    vi->object = (void *) current_ai_order;

    return vi;
}

varInst *Mission::callRnd(missionNode *node, int mode) {
    varInst *vi = newVarInst(VI_TEMP);
    vi->type = VAR_FLOAT;
    vi->float_val = ((float) rand()) / (((float) RAND_MAX) + 1);

    char buffer[100];
    sprintf(buffer, "rnd returning %f", (vi->float_val));
    debug(7, node, mode, buffer);

    return vi;
}

varInst *Mission::getObjectArg(missionNode *node, int mode) {
    if (node->subnodes.size() < 1) {
        fatalError(node, mode, method_str(node) + " needs an object as first  argument");
        assert(0);
    }
    missionNode *snode = (missionNode *) node->subnodes[0];
    varInst *ovi = doObjectVar(snode, mode);

    debug(3, node, mode, node->attr_value("module") + "." + node->attr_value("name") + " object: ");
    printVarInst(3, ovi);

    return ovi;
}

string Mission::method_str(missionNode *node) {
    return node->attr_value("module") + "." + node->attr_value("name");
}

bool Mission::getBoolArg(missionNode *node, int mode, int arg_nr) {
    missionNode *val_node = getArgument(node, mode, arg_nr);
    bool res = checkBoolExpr(val_node, mode);
    return res;
}

double Mission::getFloatArg(missionNode *node, int mode, int arg_nr) {
    missionNode *val_node = getArgument(node, mode, arg_nr);
    return checkFloatExpr(val_node, mode);
}

int Mission::getIntArg(missionNode *node, int mode, int arg_nr) {
    missionNode *val_node = getArgument(node, mode, arg_nr);
    int res = checkIntExpr(val_node, mode);
    return res;
}

Unit *Mission::getUnitArg(missionNode *node, int mode, int arg_nr) {
    Unit *ret = NULL;

    missionNode *unit_node = getArgument(node, mode, arg_nr);
    varInst *unit_vi = checkObjectExpr(unit_node, mode);
    if (mode == SCRIPT_RUN) {
        if (unit_vi->type == VAR_OBJECT && unit_vi->objectname == "unit") {
            ret = getUnitObject(unit_node, mode, unit_vi);
        } else {
            printf("Error: Unit died prematurely\n");
            return NULL;             //never reach
        }
    }
    deleteVarInst(unit_vi);
    return ret;
}

QVector Mission::getVec3Arg(missionNode *node, int mode, int arg_nr) {
    missionNode *pos_node = getArgument(node, mode, arg_nr);
    varInst *pos_vi = checkObjectExpr(pos_node, mode);

    QVector vec3;
    if (mode == SCRIPT_RUN) {
        vec3 = call_olist_tovector(pos_node, mode, pos_vi);
    }
    deleteVarInst(pos_vi);
    return vec3;
}

missionNode *Mission::getArgument(missionNode *node, int mode, int arg_nr) {
    if (node->subnodes.size() < (unsigned int) (arg_nr + 1)) {
        char buf[200];
        sprintf(buf, " needs at least %d arguments", arg_nr + 1);
        fatalError(node, mode, method_str(node) + buf);
        assert(0);
    }
    missionNode *snode = (missionNode *) node->subnodes[arg_nr];

    return snode;
}

void Mission::initCallbackMaps() {
    module_map["_io"] = CMT_IO;
    module_map["_std"] = CMT_STD;
    module_map["_string"] = CMT_STRING;
    module_map["_olist"] = CMT_OLIST;
    module_map["_omap"] = CMT_OMAP;
    module_map["_order"] = CMT_ORDER;
    module_map["_unit"] = CMT_UNIT;
    module_map["_briefing"] = CMT_BRIEFING;
    module_briefing_map["addShip"] = CMT_BRIEFING_addShip;
    module_briefing_map["removeShip"] = CMT_BRIEFING_removeShip;
    module_briefing_map["enqueueOrder"] = CMT_BRIEFING_enqueueOrder;
    module_briefing_map["replaceOrder"] = CMT_BRIEFING_replaceOrder;
    module_briefing_map["setShipPosition"] = CMT_BRIEFING_setShipPosition;
    module_briefing_map["getShipPosition"] = CMT_BRIEFING_getShipPosition;
    module_briefing_map["setCamPosition"] = CMT_BRIEFING_setCamPosition;
    module_briefing_map["setCamOrientation"] = CMT_BRIEFING_setCamOrientation;
    module_briefing_map["setCloak"] = CMT_BRIEFING_setCloak;
    module_briefing_map["terminate"] = CMT_BRIEFING_terminate;

    module_std_map["Rnd"] = CMT_STD_Rnd;
    module_std_map["getGameTime"] = CMT_STD_getGameTime;
    module_std_map["ResetTimeCompression"] = CMT_STD_ResetTimeCompression;
    module_std_map["GetSystemName"] = CMT_STD_getSystemFile;
    module_std_map["getNumAdjacentSystems"] = CMT_STD_getNumAdjacentSystems;
    module_std_map["getGalaxyProperty"] = CMT_STD_getGalaxyProperty;
    module_std_map["getAdjacentSystem"] = CMT_STD_getAdjacentSystem;
    module_std_map["GetSystemFile"] = CMT_STD_getSystemFile;
    module_std_map["getSystemFile"] = CMT_STD_getSystemFile;
    module_std_map["getCurrentAIUnit"] = CMT_STD_getCurrentAIUnit;
    module_std_map["getCurrentAIOrder"] = CMT_STD_getCurrentAIOrder;
    module_std_map["isNull"] = CMT_STD_isNull;
    module_std_map["setNull"] = CMT_STD_setNull;
    module_std_map["equal"] = CMT_STD_equal;
    module_std_map["Int"] = CMT_STD_Int;
    module_std_map["Float"] = CMT_STD_Float;
    module_std_map["getDifficulty"] = CMT_STD_getDifficulty;
    module_std_map["setDifficulty"] = CMT_STD_setDifficulty;
    module_std_map["playSound"] = CMT_STD_playSound;
    module_std_map["playSoundCockpit"] = CMT_STD_playSoundCockpit;
    module_std_map["terminateMission"] = CMT_STD_terminateMission;
    module_std_map["playAnimation"] = CMT_STD_playAnimation;
    module_std_map["musicAddList"] = CMT_STD_musicAddList;
    module_std_map["musicPlaySong"] = CMT_STD_musicPlaySong;
    module_std_map["musicPlayList"] = CMT_STD_musicPlayList;

    module_order_map["newAggressiveAI"] = CMT_ORDER_newAggressiveAI;
    module_order_map["newMoveTo"] = CMT_ORDER_newMoveTo;
    module_order_map["newChangeHeading"] = CMT_ORDER_newChangeHeading;
    module_order_map["newFaceTarget"] = CMT_ORDER_newFaceTarget;
    module_order_map["newFireAt"] = CMT_ORDER_newFireAt;
    module_order_map["newExecuteFor"] = CMT_ORDER_newExecuteFor;
    module_order_map["newCloakFor"] = CMT_ORDER_newCloakFor;
    module_order_map["newMatchVelocity"] = CMT_ORDER_newMatchVelocity;
    module_order_map["newMatchAngularVelocity"] = CMT_ORDER_newMatchAngularVelocity;
    module_order_map["newMatchLinearVelocity"] = CMT_ORDER_newMatchLinearVelocity;
    module_order_map["newFlyToWaypoint"] = CMT_ORDER_newFlyToWaypoint;
    module_order_map["newFlyToWaypointDefend"] = CMT_ORDER_newFlyToWaypointDefend;
    module_order_map["newFlyToJumppoint"] = CMT_ORDER_newFlyToJumppoint;
    module_order_map["newPatrol"] = CMT_ORDER_newPatrol;
    module_order_map["newOrderList"] = CMT_ORDER_newOrderList;
    module_order_map["newSuperiority"] = CMT_ORDER_newSuperiority;

    module_order_map["enqueueOrder"] = CMT_ORDER_enqueueOrder;
    module_order_map["enqueueOrderFirst"] = CMT_ORDER_enqueueOrderFirst;
    module_order_map["eraseOrder"] = CMT_ORDER_eraseOrder;
    module_order_map["findOrder"] = CMT_ORDER_findOrder;
    module_order_map["SteerUp"] = CMT_ORDER_SteerUp;
    module_order_map["SteerRight"] = CMT_ORDER_SteerRight;
    module_order_map["SteerRollRight"] = CMT_ORDER_SteerRollRight;
    module_order_map["SteerStop"] = CMT_ORDER_SteerStop;
    module_order_map["SteerAccel"] = CMT_ORDER_SteerAccel;
    module_order_map["SteerAfterburn"] = CMT_ORDER_SteerAfterburn;
    module_order_map["SteerSheltonSlide"] = CMT_ORDER_SteerSheltonSlide;
    module_order_map["print"] = CMT_ORDER_print;
    module_order_map["setActionString"] = CMT_ORDER_setActionString;

    module_olist_map["new"] = CMT_OLIST_new;
    module_olist_map["delete"] = CMT_OLIST_delete;
    module_olist_map["push_back"] = CMT_OLIST_push_back;
    module_olist_map["pop_back"] = CMT_OLIST_pop_back;
    module_olist_map["back"] = CMT_OLIST_back;
    module_olist_map["at"] = CMT_OLIST_at;
    module_olist_map["erase"] = CMT_OLIST_erase;
    module_olist_map["set"] = CMT_OLIST_set;
    module_olist_map["toxml"] = CMT_OLIST_toxml;
    module_olist_map["size"] = CMT_OLIST_size;
    module_omap_map["new"] = CMT_OMAP_new;
    module_omap_map["delete"] = CMT_OMAP_delete;
    module_omap_map["set"] = CMT_OMAP_set;
    module_omap_map["get"] = CMT_OMAP_get;
    module_omap_map["toxml"] = CMT_OMAP_toxml;
    module_omap_map["size"] = CMT_OMAP_size;

    module_string_map["new"] = CMT_STRING_new;
    module_string_map["delete"] = CMT_STRING_delete;
    module_string_map["print"] = CMT_STRING_print;
    module_string_map["equal"] = CMT_STRING_equal;
    module_string_map["begins"] = CMT_STRING_begins;

    module_unit_map["getContainer"] = CMT_UNIT_getContainer;
    module_unit_map["getUnitFromContainer"] = CMT_UNIT_getUnitFromContainer;
    module_unit_map["deleteContainer"] = CMT_UNIT_deleteContainer;
    module_unit_map["getUnit"] = CMT_UNIT_getUnit;
    module_unit_map["getTurret"] = CMT_UNIT_getTurret;
    module_unit_map["getCredits"] = CMT_UNIT_getCredits;
    module_unit_map["getRandCargo"] = CMT_UNIT_getRandCargo;
    module_unit_map["addCredits"] = CMT_UNIT_addCredits;
    module_unit_map["getPlayer"] = CMT_UNIT_getPlayer;
    module_unit_map["getPlayerX"] = CMT_UNIT_getPlayerX;
    module_unit_map["launch"] = CMT_UNIT_launch;
    module_unit_map["launchNebula"] = CMT_UNIT_launchPlanet;
    module_unit_map["launchPlanet"] = CMT_UNIT_launchNebula;
    module_unit_map["launchJumppoint"] = CMT_UNIT_launchJumppoint;
    module_unit_map["getPosition"] = CMT_UNIT_getPosition;
    module_unit_map["getFaction"] = CMT_UNIT_getFaction;
    module_unit_map["getVelocity"] = CMT_UNIT_getVelocity;
    module_unit_map["getTarget"] = CMT_UNIT_getTarget;
    module_unit_map["getName"] = CMT_UNIT_getName;
    module_unit_map["setName"] = CMT_UNIT_setName;
    module_unit_map["equal"] = CMT_UNIT_equal;
    module_unit_map["getThreat"] = CMT_UNIT_getThreat;
    module_unit_map["setTarget"] = CMT_UNIT_setTarget;
    module_unit_map["setPosition"] = CMT_UNIT_setPosition;
    module_unit_map["addCargo"] = CMT_UNIT_addCargo;
    module_unit_map["removeCargo"] = CMT_UNIT_removeCargo;
    module_unit_map["incrementCargo"] = CMT_UNIT_incrementCargo;
    module_unit_map["decrementCargo"] = CMT_UNIT_decrementCargo;
    module_unit_map["getThreat"] = CMT_UNIT_getThreat;
    module_unit_map["getDistance"] = CMT_UNIT_getDistance;
    module_unit_map["getMinDis"] = CMT_UNIT_getMinDis;
    module_unit_map["getAngle"] = CMT_UNIT_getAngle;
    module_unit_map["getAngleToPos"] = CMT_UNIT_getAngleToPos;
    module_unit_map["getFShieldData"] = CMT_UNIT_getFShieldData;
    module_unit_map["getRShieldDat"] = CMT_UNIT_getRShieldData;
    module_unit_map["getLShieldData"] = CMT_UNIT_getLShieldData;
    module_unit_map["getBShieldData"] = CMT_UNIT_getBShieldData;
    module_unit_map["getEnergyData"] = CMT_UNIT_getEnergyData;
    module_unit_map["getHullData"] = CMT_UNIT_getHullData;
    module_unit_map["getRSize"] = CMT_UNIT_getRSize;
    module_unit_map["isStarShip"] = CMT_UNIT_isStarShip;
    module_unit_map["isPlanet"] = CMT_UNIT_isPlanet;
    module_unit_map["isSun"] = CMT_UNIT_isSun;
    module_unit_map["isSignificant"] = CMT_UNIT_isSignificant;
    module_unit_map["isJumppoint"] = CMT_UNIT_isJumppoint;
    module_unit_map["getRelation"] = CMT_UNIT_getRelation;
    module_unit_map["Jump"] = CMT_UNIT_Jump;
    module_unit_map["getOrientationP"] = CMT_UNIT_getOrientationP;
    module_unit_map["getOrder"] = CMT_UNIT_getOrder;
    module_unit_map["removeFromGame"] = CMT_UNIT_removeFromGame;
    module_unit_map["getFgDirective"] = CMT_UNIT_getFgDirective;
    module_unit_map["setFgDirective"] = CMT_UNIT_setFgDirective;
    module_unit_map["getFgLeader"] = CMT_UNIT_getFgLeader;
    module_unit_map["setFgLeader"] = CMT_UNIT_setFgLeader;
    module_unit_map["getFgID"] = CMT_UNIT_getFgId;
    module_unit_map["getFgId"] = CMT_UNIT_getFgId;
    module_unit_map["getFgName"] = CMT_UNIT_getFgName;
    module_unit_map["getFgSubnumber"] = CMT_UNIT_getFgSubnumber;
    module_unit_map["scanSystem"] = CMT_UNIT_scanSystem;
    module_unit_map["scannerNearestEnemy"] = CMT_UNIT_scannerNearestEnemy;
    module_unit_map["scannerNearestFriend"] = CMT_UNIT_scannerNearestFriend;
    module_unit_map["scannerNearestShip"] = CMT_UNIT_scannerNearestShip;
    module_unit_map["scannerLeader"] = CMT_UNIT_scannerLeader;
    module_unit_map["scannerNearestEnemyDist"] = CMT_UNIT_scannerNearestEnemyDist;
    module_unit_map["scannerNearestFriendDist"] = CMT_UNIT_scannerNearestFriendDist;
    module_unit_map["scannerNearestShipDist"] = CMT_UNIT_scannerNearestShipDist;
    module_unit_map["toxml"] = CMT_UNIT_toxml;
    module_unit_map["getSaveData"] = CMT_UNIT_getSaveData;
    module_unit_map["upgrade"] = CMT_UNIT_upgrade;
    module_unit_map["frameOfReference"] = CMT_UNIT_frameOfReference;
    module_unit_map["communicateTo"] = CMT_UNIT_communicateTo;
    module_unit_map["commAnimation"] = CMT_UNIT_commAnimation;
    module_unit_map["correctStarSystem"] = CMT_UNIT_correctStarSystem;     //useful when comparing _jumps_
    module_unit_map["switchFg"] = CMT_UNIT_switchFg;
}

