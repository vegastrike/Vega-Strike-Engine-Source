/*
 * director_generic.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Nachum Barcohen, Roy Falk,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#ifdef HAVE_PYTHON
#include <boost/version.hpp>
#include <boost/python.hpp>
#include <boost/python/class.hpp>
#include <Python.h>
#endif

#include "cmd/unit_generic.h"
#include "cmd/ai/order.h"

#include "root_generic/configxml.h"
#include "gfx_generic/cockpit_generic.h"

#include "python/python_class.h"

#include "pythonmission.h"
#include "mission.h"
#include "root_generic/savegame.h"
#include "src/gnuhash.h"
#include "src/universe.h"
#include "src/vs_logging.h"

PYTHON_INIT_INHERIT_GLOBALS(Director, PythonMissionBaseClass);

float getSaveData(int whichcp, const string &key, unsigned int num) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<float> *ans = &(_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key));
    if (num >= ans->size()) {
        return 0;
    }
    return (*ans)[num];
}

const vector<float> &getSaveData(int whichcp, const string &key) {
    static vector<float> empty;
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return empty;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionData(key);
}

string getSaveString(int whichcp, const string &key, unsigned int num) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return "";
    }
    vector<std::string> *ans = &(_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key));
    if (num >= ans->size()) {
        return "";
    }
    return (*ans)[num];
}

unsigned int getSaveDataLength(int whichcp, const string &key) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionDataLength(key);
}

unsigned int getSaveStringLength(int whichcp, const string &key) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    return _Universe->AccessCockpit(whichcp)->savegame->getMissionStringDataLength(key);
}

unsigned int pushSaveData(int whichcp, const string &key, float val) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));

    ans->push_back(val);
    return ans->size() - 1;
}

unsigned int eraseSaveData(int whichcp, const string &key, unsigned int index) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    if (index < ans->size()) {

        ans->erase(ans->begin() + index);
    }
    return ans->size();
}

unsigned int clearSaveData(int whichcp, const string &key) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    int ret = ans->size();
    if (!ret) {
        return 0;
    }

    ans->clear();
    return ret;
}

unsigned int pushSaveString(int whichcp, const string &key, const string &value) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));

    ans->push_back(std::string(value));
    return ans->size() - 1;
}

void putSaveString(int whichcp, const string &key, unsigned int num, const string &val) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    if (num < ans->size()) {

        (*ans)[num] = val;
    }
}

void putSaveData(int whichcp, const string &key, unsigned int num, float val) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return;
    }
    vector<float> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionData(key)));
    if (num < ans->size()) {

        (*ans)[num] = val;
    }
}

unsigned int eraseSaveString(int whichcp, const string &key, unsigned int index) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    if (index < ans->size()) {

        ans->erase(ans->begin() + index);
    }
    return ans->size();
}

unsigned int clearSaveString(int whichcp, const string &key) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return 0;
    }
    vector<std::string> *ans = &((_Universe->AccessCockpit(whichcp)->savegame->getMissionStringData(key)));
    int ret = ans->size();
    if (!ret) {
        return 0;
    }

    ans->clear();
    return ret;
}

vector<string> loadStringList(int playernum, const string &mykey) {
    if (playernum < 0 || (unsigned int) playernum >= _Universe->numPlayers()) {
        return vector<string>();
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;
    vector<string> rez;

    const vector<string> &ans = savegame->readMissionStringData(mykey);
    if (ans.size() > 0) {
        /* This is the modern way to store string data: as strings */
        rez.reserve(ans.size());
        rez.insert(rez.end(), ans.begin(), ans.end());
    } else {
        /* This variant loads it from float data, converting to character data.
           It's a legacy variant, highly and unpleasantly convoluted and sub-performant,
           but we must support it to be compatible with old savegames */
        const vector<float> &ans = savegame->readMissionData(mykey);
        int lengt = ans.size();
        rez.reserve(ans.size());
        if (lengt >= 1) {
            string curstr;
            int length = (int) ans[0];
            for (int j = 0; j < length && j < lengt; j++) {
                char myint = (char) ans[j + 1];
                if (myint != '\0') {
                    curstr += myint;
                } else {
                    rez.push_back(curstr);
                    curstr = "";
                }
            }
        }
    }
    return rez;
}

const vector<string> &getStringList(int playernum, const string &mykey) {
    if (playernum < 0 || (unsigned int) playernum >= _Universe->numPlayers()) {
        static const vector<string> empty;
        return empty;
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;

    /* Should check old-style string lists, but it would defeat the purpose
       of this fast getter. Besides, any functionality that uses this one is
       using the new-style string lists, so we're cool (or ought to be) */
    return savegame->readMissionStringData(mykey);
}

void saveStringList(int playernum, const string &mykey, const vector<string> &names) {
    if (playernum < 0 || (unsigned int) playernum >= _Universe->numPlayers()) {
        return;
    }

    SaveGame *savegame = _Universe->AccessCockpit(playernum)->savegame;

    // Erase old-style string lists
    if (savegame->getMissionDataLength(mykey) != 0) {
        clearSaveData(playernum, mykey);
    }

    vector<string> &ans = savegame->getMissionStringData(mykey);
    clearSaveString(playernum, mykey);
    for (vector<string>::const_iterator i = names.begin(); i != names.end(); ++i) {

        ans.push_back(*i);
    }
}

void saveDataList(int whichcp, const string &key, const vector<float> &values) {
    if (whichcp < 0 || (unsigned int) whichcp >= _Universe->numPlayers()) {
        return;
    }

    clearSaveData(whichcp, key);

    vector<float> &ans = _Universe->AccessCockpit(whichcp)->savegame->getMissionData(key);
    for (vector<float>::const_iterator i = values.begin(); i != values.end(); ++i) {

        ans.push_back(*i);
    }
}

static float getSaveDataPy(int whichcp, string key, unsigned int num) {
    return getSaveData(whichcp, key, num);
}

static string getSaveStringPy(int whichcp, string key, unsigned int num) {
    return getSaveString(whichcp, key, num);
}

static unsigned int getSaveDataLengthPy(int whichcp, string key) {
    return getSaveDataLength(whichcp, key);
}

static unsigned int getSaveStringLengthPy(int whichcp, string key) {
    return getSaveStringLength(whichcp, key);
}

static unsigned int pushSaveDataPy(int whichcp, string key, float val) {
    return pushSaveData(whichcp, key, val);
}

static unsigned int eraseSaveDataPy(int whichcp, string key, unsigned int index) {
    return eraseSaveData(whichcp, key, index);
}

static unsigned int clearSaveDataPy(int whichcp, string key) {
    return clearSaveData(whichcp, key);
}

static unsigned int pushSaveStringPy(int whichcp, string key, string value) {
    return pushSaveString(whichcp, key, value);
}

static void putSaveStringPy(int whichcp, string key, unsigned int num, string val) {
    putSaveString(whichcp, key, num, val);
}

static void putSaveDataPy(int whichcp, string key, unsigned int num, float val) {
    putSaveData(whichcp, key, num, val);
}

static unsigned int eraseSaveStringPy(int whichcp, string key, unsigned int index) {
    return eraseSaveString(whichcp, key, index);
}

static unsigned int clearSaveStringPy(int whichcp, string key) {
    return clearSaveString(whichcp, key);
}

static vector<string> loadStringListPy(int playernum, string mykey) {
    return loadStringList(playernum, mykey);
}

static void saveStringListPy(int playernum, string mykey, vector<string> names) {
    saveStringList(playernum, mykey, names);
}

static bool isUtf8SaveGamePy(string savegame) {
    return isUtf8SaveGame(savegame);
}

BOOST_PYTHON_MODULE(Director) {
    PYTHON_BEGIN_INHERIT_CLASS(Director, pythonMission, PythonMissionBaseClass, "Mission")
        PYTHON_DEFINE_METHOD_DEFAULT(Class, &PythonMissionBaseClass::Pickle, "Pickle", pythonMission::default_Pickle);
        PYTHON_DEFINE_METHOD_DEFAULT(Class,
                &PythonMissionBaseClass::UnPickle,
                "UnPickle",
                pythonMission::default_UnPickle);
        PYTHON_DEFINE_METHOD_DEFAULT(Class,
                &PythonMissionBaseClass::Execute,
                "Execute",
                pythonMission::default_Execute);
    PYTHON_END_CLASS(Director, pythonMission)
    boost::python::def("putSaveData", &putSaveDataPy);
    boost::python::def("pushSaveData", &pushSaveDataPy);
    boost::python::def("eraseSaveData", &eraseSaveDataPy);
    boost::python::def("clearSaveData", &clearSaveDataPy);
    boost::python::def("getSaveData", &getSaveDataPy);
    boost::python::def("getSaveDataLength", &getSaveDataLengthPy);
    boost::python::def("putSaveString", &putSaveStringPy);
    boost::python::def("pushSaveString", &pushSaveStringPy);
    boost::python::def("getSaveString", &getSaveStringPy);
    boost::python::def("getSaveStringLength", &getSaveStringLengthPy);
    boost::python::def("eraseSaveString", &eraseSaveStringPy);
    boost::python::def("clearSaveString", &clearSaveStringPy);
    boost::python::def("loadStringList", &loadStringListPy);
    boost::python::def("saveStringList", &saveStringListPy);
    boost::python::def("isUtf8SaveGame", &isUtf8SaveGamePy);
}

void InitDirector() {
    PyImport_AppendInittab("Director", PYTHON_MODULE_INIT_FUNCTION(Director));
}

void InitDirector2() {
    Python::reseterrors();
    PYTHON_INIT_MODULE(Director);
    Python::reseterrors();
}

void Mission::loadModule(string modulename) {
    missionNode *node = director;

    debug(3, node, SCRIPT_PARSE, "loading module " + modulename);

    VS_LOG(trace, (boost::format("  loading module %1%") % modulename));

    string filename = "modules/" + modulename + ".module";
    missionNode *import_top = importf->LoadXML(filename.c_str());
    if (import_top == NULL) {
    }
    import_top->Tag(&tagmap);

    doModule(import_top, SCRIPT_PARSE);
}

void Mission::loadMissionModules() {
    missionNode *node = director;
    while (import_stack.size() > 0) {
        string importname = import_stack.back();
        import_stack.pop_back();

        missionNode *module = runtime.modules[importname];
        if (module == NULL) {
            loadModule(importname);
        } else {
            debug(3, node, SCRIPT_PARSE, "already have module " + importname);
        }
    }
}

void Mission::RunDirectorScript(const string &script) {
    runScript(director, script, 0);
}

bool Mission::runScript(missionNode *module_node, const string &scriptname, unsigned int classid) {
    if (module_node == NULL) {
        return false;
    }
    missionNode *script_node = module_node->script.scripts[scriptname];
    if (script_node == NULL) {
        return false;
    }
    runtime.cur_thread->module_stack.push_back(module_node);
    runtime.cur_thread->classid_stack.push_back(classid);

    varInst *vi = doScript(script_node, SCRIPT_RUN);
    deleteVarInst(vi);

    runtime.cur_thread->classid_stack.pop_back();
    runtime.cur_thread->module_stack.pop_back();
    return true;
}

bool Mission::runScript(string modulename, const string &scriptname, unsigned int classid) {
    return runScript(runtime.modules[modulename], scriptname, classid);
}

double Mission::getGametime() {
    return gametime;
}

void Mission::addModule(string modulename) {
    import_stack.push_back(modulename);
}

void Mission::DirectorStartStarSystem(StarSystem *ss) {
    RunDirectorScript("initstarsystem");
}

std::string Mission::Pickle() {
    if (!runtime.pymissions) {
        return "";
    } else {
        return runtime.pymissions->Pickle();
    }
}

void Mission::UnPickle(string pickled) {
    if (runtime.pymissions) {
        runtime.pymissions->UnPickle(pickled);
    }
}

void Mission::DirectorStart(missionNode *node) {
    VS_LOG(trace, "DIRECTOR START");

    static int st_debuglevel = atoi(vs_config->getVariable("interpreter", "debuglevel", "0").c_str());
    static bool st_start_game = XMLSupport::parse_bool(vs_config->getVariable("interpreter", "startgame", "true"));
    static bool st_do_trace = XMLSupport::parse_bool(vs_config->getVariable("interpreter", "trace", "false"));

    debuglevel = st_debuglevel;
    start_game = st_start_game;
    do_trace = st_do_trace;

    vi_counter = 0;
    old_vi_counter = 0;

    olist_counter = 0;
    old_olist_counter = 0;

    string_counter = 0;
    old_string_counter = 0;
    missionThread *main_thread = new missionThread;
    runtime.thread_nr = 0;
    runtime.threads.push_back(main_thread);
    runtime.cur_thread = main_thread;

    director = NULL;
    std::string doparse = node->attr_value("do_parse");
    if (!doparse.empty()) {
        if (XMLSupport::parse_bool(doparse) == false) {
            return;
        }
    }
    VS_LOG(trace, "parsing declarations for director");

    parsemode = PARSE_DECL;

    doModule(node, SCRIPT_PARSE);

    importf = new easyDomFactory<missionNode>();

    loadMissionModules();

    parsemode = PARSE_FULL;

    doModule(node, SCRIPT_PARSE);

    vsUMap<string, missionNode *>::iterator iter;
    for (iter = runtime.modules.begin(); iter != runtime.modules.end(); iter++) {
        string mname = (*iter).first;
        missionNode *mnode = (*iter).second;
        if (mname != "director") {
            VS_LOG(trace, (boost::format(" parsing full module %1%") % mname));
            doModule(mnode, SCRIPT_PARSE);
        }
    }
}

void Mission::DirectorInitgame() {
    this->player_num = _Universe->CurrentCockpit();
    if (nextpythonmission) {
        //CAUSES AN UNRESOLVED EXTERNAL SYMBOL FOR PythonClass::last_instance ?!?!
#ifndef _WIN32
        char *tmp = nextpythonmission;
        while (*tmp) {
            if (tmp[0] == '\r') {
                tmp[0] = '\n';
            }
            tmp++;
        }
#endif
        runtime.pymissions = (pythonMission::FactoryString(nextpythonmission));
        delete[] nextpythonmission;          //delete the allocated memory
        nextpythonmission = NULL;
        if (!this->unpickleData.empty()) {
            if (runtime.pymissions) {
                runtime.pymissions->UnPickle(unpickleData);
                unpickleData = "";
            }
        }
    }
    if (director == NULL) {
        return;
    }
    RunDirectorScript("initgame");
}

