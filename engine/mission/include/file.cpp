/*
 * file.cpp
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

#if defined(_WIN32) && _MSC_VER > 1300
#define __restrict
#endif
#include "file.h"

void FindMissions(char *path) {
    MISSIONS = FindFiles(path, EXT_MISSION);
    return;
}

void LoadMission(char *filename) {
    char *file, *name;
    easyDomFactory<missionNode> *domf = new easyDomFactory<missionNode>();
    missionNode *top = domf->LoadXML(filename);
    easyDomNode *read;
    string scan_name;
//	vector<easyDomNode *>::const_iterator siter;
//	easyDomNode *sub = 0;

    file = _strdup(filename);
    name = StripPath(file);
    StripExtension(name);

    SetString(&DATA.name, name);
    SetString(&DATA.author, "Anonymous");
    SetString(&DATA.description, "A standard mission");
    SetString(&DATA.briefing, "Destroy all hostiles");
    SetString(&DATA.path, filename);
    SetString(&DATA.numplayers, "1");
    free(file);

    if (top == NULL) {
        cout << "Unable to load mission file\n";
        return;
    }

    read = top;

    ScanNode(NULL, top, filename);
}

string ClassName(string pythonfilename) {
    string::size_type where = pythonfilename.find_last_of("/");
    if (where != string::npos) {
        pythonfilename = pythonfilename.substr(where + 1, pythonfilename.length() - (where + 1));
    }
    while ((where = pythonfilename.find_first_of("_")) != string::npos) {
        string first = pythonfilename.substr(0, where);
        string last = pythonfilename.substr(where + 1, pythonfilename.length() - (where + 1));
        printf("chopping %s %s", first.c_str(), last.c_str());
        first[0] = toupper(first[0]);
        last[0] = toupper(last[0]);
        pythonfilename = first + last;
    }
    return pythonfilename;
}

using std::string;

std::string PrintArg(easyDomNode *node) {
    printf("%s\n", (node->Name() == "const") ? "" : "ASSERT(node->Name()==\"const\")");
    string type = node->attr_value("type");
    string def;
    string retval = node->attr_value("value");
    if (type == "int") {
        def = "0";
    } else if (type == "float") {
        def = "0.0";
    } else if (type == "bool") {
        def = "0";
        if (retval == "false") {
            retval = "0";
        } else if (retval == "true") {
            retval = "1";
        }
    } else if (type == "object") {
        retval = string("'") + retval + string("'");
        def = "''";
    }
    if (retval.empty()) {
        printf("\n[WARNING: attr_value with type \"%s\" is empty... Using default:\"%s\"\n", type.c_str(), def.c_str());
        retval = def;
    }
    return retval;
}

void PrintArgs(string &pythontxt, easyDomNode *node) {
    for (unsigned int i = 0; i < node->subnodes.size(); i++) {
        pythontxt += PrintArg(node->subnodes[i]);
        if (i + 1 < node->subnodes.size()) {
            pythontxt += ", ";
        }
    }
}

void LoadPythonModule(string fn, string pythn) {
    FILE *fp = fopen(fn.c_str(), "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp) + 1;
        fseek(fp, 0, SEEK_SET);
        char *temp = (char *) malloc(size);
        temp[fread(temp, 1, size - 1, fp)] = '\0';
        fclose(fp);
        string wholefile(temp);
        free(temp);
        string::size_type where;
        if (wholefile.find("python") != string::npos) {
            return;
        }
        fp = fopen(fn.c_str(), "w");
        if (!fp) {
            return;
        }
        string tofind("module");
        if ((where = wholefile.find(tofind)) != string::npos) {
            *(wholefile.begin() + where + 1) = 'a';//madule
            for (; where > 0 && wholefile[where] != '<'; where--) {
            }
            fwrite(wholefile.c_str(), where, 1, fp);
//	  string INBETTEXT;
            for (; where > 0 && wholefile[where] != '>'; where--) {
//		  if (wholefile[where]=='\n'||wholefile[where]=='\r'||wholefile[where]==' '||wholefile[where]=='\t')
//			  INBETTEXT+=wholefile[where];
            }
            where += 1;
            string addition("<python>\n" + pythn + "\n</python>");
            fwrite(addition.c_str(), addition.length(), 1, fp);
            wholefile = wholefile.substr(where, wholefile.length() - (where + 1));
        }
        fwrite(wholefile.c_str(), wholefile.length(), 1, fp);

        fclose(fp);

    }
}

void PrintPython(easyDomNode *node, string filename) {
    string pythontxt;
    string module = node->attr_value("module");
    string classname = module;//ClassName (module);
    string myname = node->attr_value("name");
    if (module.length()) {
        pythontxt = string("import ") + module
                + "\nnewmission = " + module + "." + ((myname == "init" || myname == "") ? classname : myname) + " (";
    } else {
        pythontxt = "newmission = " + classname + " (";
    }

    if (node->subnodes.size() > 0) {
        PrintArgs(pythontxt, node);
    }
    pythontxt += ")\nnewmission=0\n";
    LoadPythonModule(filename, pythontxt);
}

void ScanNode(string *parent, easyDomNode *node, string filename) {
    vector<easyDomNode *>::const_iterator siter;
    string scan_name, current;

    current = node->Name();
    if (current == "script") {
        if (node->attr_value("name") == "initgame") {
            fprintf(stderr, "initgame found");
            for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {

                if ((*siter)->Name() == "exec") {
                    PrintPython((*siter), filename);
                }
            }

        }
    }
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {

        ScanNode(&current, *siter, filename);
    }
    if (parent == NULL) {
        return;
    }        // We're at the top node, probably <mission></mission>
    CheckVar(*parent, current, node->attr_value("name"), node->attr_value("value"));
}

// If the line is  <mission><variable><var name="alpha" value="beta"/></variable></mission>, then
// the parameters will be:  variable, var, alpha, beta
void CheckVar(string parent, string current, string name, string value) {
    char *param;
    int i;
    if (name.empty() || value.empty()) {
        return;
    }
    param = _strdup(value.c_str());
    if (name == "mission_name") {
        SetString(&DATA.name, param);
    }
    if (name == "author") {
        SetString(&DATA.author, param);
    }
    if (name == "description") {
        for (i = 0; param[i] != '\0'; i++) {
            if (param[i] == '\\') {
                param[i] = '\n';
            }
        }
        SetString(&DATA.description, param);
    }
    if (name == "briefing") {
        for (i = 0; param[i] != '\0'; i++) {
            if (param[i] == '\\') {
                param[i] = '\n';
            }
        }
        SetString(&DATA.briefing, param);
    }
    if (name == "num_players") {
        SetString(&DATA.numplayers, param);
    }
    free(param);
}
