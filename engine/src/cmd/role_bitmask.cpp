/*
 * role_bitmask.cpp
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


#include "role_bitmask.h"
#include "xml_support.h"
#include <gnuhash.h>

#include "vs_globals.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "csv.h"
#include "vs_exit.h"
using std::string;
using std::pair;
using namespace VSFileSystem;   // FIXME -- Shouldn't import an entire namespace like this, at least according to Google Style Guide

namespace ROLES {

int discreteLog(int bitmask) {
    for (unsigned char i = 0; i < sizeof(int) * 8; i++) {
        if (bitmask & (1 << i)) {
            return i;
        }
    }
    VS_LOG(warning, "undefined discrete log.");
    return 0;
}

std::vector<std::vector<char>> buildroles();

std::vector<std::vector<char>> &getAllRolePriorities() {
    static std::vector<std::vector<char> > allrolepriority = buildroles();
    return allrolepriority;
}

std::vector<char> &getPriority(unsigned char rolerow) {
    if (rolerow > getAllRolePriorities().size()) {
        VS_LOG_FLUSH_EXIT(fatal, "FATAL ERROR ROLE OUT OF RANGE", 1);
    }
    return getAllRolePriorities()[rolerow];
}

static vsUMap<string, int> rolemap;
static vsUMap<int, string> irolemap;

unsigned char InternalGetRole(const std::string &s) {
    vsUMap<string, int>::const_iterator i = rolemap.find(strtoupper(s));
    if (i != rolemap.end()) {
        return (*i).second;
    }
    return 0;
}

const std::string &InternalGetStrRole(unsigned char c) {
    static const std::string empty;

    vsUMap<int, string>::const_iterator i = irolemap.find(c);
    if (i != irolemap.end()) {
        return (*i).second;
    }
    return rolemap.size() ? rolemap.begin()->first : empty;
}

std::vector<std::vector<std::string> > buildscripts() {
    std::vector<std::vector<std::string> > scripts;
    getAllRolePriorities();

    VSFile f;
    VSError err = f.OpenReadOnly("VegaEvents.csv", AiFile);
    if (err <= Ok) {
        int len = f.Size();
        char *temp = (char *) malloc(len + 1);
        memset(temp, 0, len + 1);
        f.ReadLine(temp, len);
        size_t siz = getAllRolePriorities().size();

        std::vector<std::string> vec = readCSV(temp);
        if (siz && getAllRolePriorities()[0].size() != vec.size()) {
            VS_LOG_FLUSH_EXIT(fatal,
                    (boost::format(
                            "FATAL error in hash map... column %1% in ai/VegaEvents.csv does not line up with that item in ai/VegaPriorities.csv\n")
                            % ((unsigned int) vec.size())), -5);
        }
        if (vec.size()) {
            vec.erase(vec.begin());
        }
        for (unsigned int j = 0; j < vec.size(); j++) {
            if (getRole(vec[j]) != j) {
                VS_LOG_FLUSH_EXIT(fatal,
                        (boost::format(
                                "FATAL error in hash map... column %1% in ai/VegaEvents.csv does not line up with that item in ai/VegaPriorities.csv\n")
                                % j), -5);
            }
        }
        unsigned int i = 0;
        for (i = 0; i < siz; i++) {
            scripts.push_back(std::vector<std::string>());
            for (unsigned int j = 0; j < vec.size(); j++) {
                scripts[i].push_back("default");
            }
        }
        for (i = 0; i < vec.size(); i++) {
            f.ReadLine(temp, len);
            std::vector<std::string> strs = readCSV(temp);
            if (strs.size()) {
                string front = strs.front();
                unsigned int scriptind = getRole(front);
                while (scripts.size() <= scriptind) {
                    scripts.push_back(std::vector<std::string>());
                }
                for (unsigned int j = 1; j < strs.size() && j <= vec.size(); j++) {
                    unsigned int index = getRole(vec[j - 1]);
                    while (scripts[scriptind].size() <= index) {
                        scripts[scriptind].push_back("default");
                    }
                    scripts[scriptind][index] = strs[j];
                }
            }
        }
        free(temp);
        f.Close();
    }
    return scripts;
}

const std::string &getRoleEvents(unsigned char ourrole, unsigned char theirs) {
    static std::vector<std::vector<std::string> > script = buildscripts();
    const static std::string def = "default";
    if (ourrole >= script.size()) {
        VS_LOG(error, "bad error with getRoleEvents (no event specified)");
        return def;
    }
    if (theirs >= script[ourrole].size()) {
        VS_LOG(error, "bad error || with getRoleEvents (no event specified)");
        return def;
    }
    return script[ourrole][theirs];
}

std::vector<std::vector<char> > buildroles() {
    std::vector<std::vector<char> > rolePriorities;
    VSFile f;
    VSError err = f.OpenReadOnly("VegaPriorities.csv", AiFile);
    if (err <= Ok) {
        int len = f.Size();
        char *temp = (char *) malloc(len + 1);
        memset(temp, 0, len + 1);
        f.ReadLine(temp, len);
        std::vector<std::string> vec = readCSV(temp);
        unsigned int i;
        for (i = 1; i < vec.size(); i++) {
            rolemap.insert(pair<string, int>(strtoupper(vec[i]), i - 1));
            irolemap.insert(pair<int, string>(i - 1, strtoupper(vec[i])));
        }
        std::vector<std::vector<char> > tmprolepriorities;
        std::vector<std::string> tmpnamelist;
        while (f.ReadLine(temp, len) == Ok) {
            std::vector<std::string> priority = readCSV(temp);
            if (priority.size() > 0) {
                tmpnamelist.push_back(strtoupper(priority[0]));
                tmprolepriorities.push_back(std::vector<char>());
                for (unsigned int j = 1; j < priority.size(); j++) {
                    tmprolepriorities.back().push_back(XMLSupport::parse_int(priority[j]));
                }
                while (tmprolepriorities.back().size() < vec.size()) {
                    tmprolepriorities.back().push_back(31);
                }
            }
        }
        for (int k = 0; k < 2; ++k) {
            for (i = 0; i < tmpnamelist.size(); ++i) {
                vsUMap<string, int>::iterator iter = rolemap.find(tmpnamelist[i]);
                int j = -1;
                if (iter != rolemap.end()) {
                    if (k == 0) {
                        j = iter->second;
                    }
                } else if (k == 1) {
                    for (j = 0; j < (int) rolePriorities.size(); ++j) {
                        if (rolePriorities[j].size() == 0) {
                            break;
                        }
                    }
                    rolemap[tmpnamelist[i]] = j;
                    irolemap[j] = tmpnamelist[i];
                }
                if (j != -1) {
                    while (rolePriorities.size() <= (unsigned int) j) {
                        rolePriorities.push_back(std::vector<char>());
                    }
                    rolePriorities[j].swap(tmprolepriorities[i]);
                }
            }
        }
        size_t a = rolePriorities.size();
        if (rolePriorities.size()) {
            size_t b = rolePriorities[0].size();
            while (b > a) {
                rolePriorities.push_back(rolePriorities[0]);
                a++;
            }
            if (a > b) {
                for (size_t i = 0; i < rolePriorities.size(); ++i) {
                    rolePriorities[i].resize(a);
                }
            }
            //this is just to square out the table and make it safe to access with *any * input string
        }
        free(temp);
        f.Close();
    } else {
        rolePriorities.push_back(std::vector<char>());
        rolePriorities[0].push_back(0);
    }
    return rolePriorities;
}

unsigned char getRole(const std::string &s) {
    return InternalGetRole(s);
}

const std::string &getRole(unsigned char c) {
    return InternalGetStrRole(c);
}

// TODO: remove and examine entire file
unsigned int readBitmask(const std::string &ss) {
    string s = ss;
    std::string::size_type loc = string::npos;
    int ans = 0;
    do {
        loc = s.find(" ");
        ans |= (1 << getRole(s.substr(0, loc)));
        if (loc != string::npos) {
            s = s.substr(loc + 1);
        }
    } while (loc != string::npos);
    return ans;
}

unsigned int getCapitalRoles() {
    static string defaultcapshipvalues = vs_config->getVariable("data",
            "capship_roles",
            "ESCORTCAP CAPITAL CARRIER BASE TROOP");
    unsigned int retval = 0;
    string inp = defaultcapshipvalues;
    string::size_type where;
    while ((where = inp.find(" ")) != string::npos) {
        string tmp = inp.substr(0, where);
        unsigned char logrole = getRole(tmp);
        if (tmp == getRole(logrole)) {
            retval |= (1 << logrole);
        }
        inp = inp.substr(where + 1);
    }
    if (inp.length()) {
        unsigned char logrole = getRole(inp);
        string tmp = getRole(logrole);
        if (tmp == inp) {
            retval |= (1 << logrole);
        }
    }
    return retval;
}
}

