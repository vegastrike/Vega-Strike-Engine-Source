/*
 * configxml.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
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
 *  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include "root_generic/xml_support.h"
#include <assert.h>
#include "root_generic/configxml.h"
#include "root_generic/easydom.h"
#include "src/vs_logging.h"
#include "src/vs_exit.h"

/* *********************************************************** */

VegaConfig::VegaConfig(const char *configfile) {
    configNodeFactory domf;
    configNode *top = (configNode *) domf.LoadXML(configfile);
    if (top == nullptr) {
        VS_LOG_AND_FLUSH(fatal, "Panic exit - no configuration");
        VSExit(0);
    }
    variables = nullptr;
    colors = nullptr;
    checkConfig(top);
}

VegaConfig::~VegaConfig() {
    if (variables != nullptr) {
        delete variables;
        variables = nullptr;
    }
    if (colors != nullptr) {
        delete colors;
        colors = nullptr;
    }
    if (bindings != nullptr) {
        delete bindings;
        bindings = nullptr;
    }
}

/* *********************************************************** */

bool VegaConfig::checkConfig(configNode *node) {
    if (node->Name() != "vegaconfig") {
        VS_LOG(warning, "this is no Vegastrike config file");
        return false;
    }
    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        if (cnode->Name() == "variables") {
            doVariables(cnode);
        } else if (cnode->Name() == "colors") {
            doColors(cnode);
        } else if (cnode->Name() == "bindings") {
            bindings = cnode;              //delay the bindings until keyboard/joystick is initialized
        } else {
            VS_LOG(warning, (boost::format("Unknown tag: %1%") % cnode->Name()));
        }
    }
    return true;
}

/* *********************************************************** */

void VegaConfig::doVariables(configNode *node) {
    if (variables != nullptr) {
        VS_LOG(error, "only one variable section allowed");
        return;
    }
    variables = node;

    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        checkSection(cnode, SECTION_VAR);
    }
}

/* *********************************************************** */

void VegaConfig::doSection(string prefix, configNode *node, enum section_t section_type) {
    string section = node->attr_value("name");
    if (section.empty()) {
        VS_LOG(warning, "no name given for section");
    }
    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        if (section_type == SECTION_COLOR) {
            checkColor(prefix, cnode);
        } else if (section_type == SECTION_VAR) {
            if (cnode->Name() == "var") {
                doVar(prefix, cnode);
            } else if (cnode->Name() == "section") {
                doSection(prefix + cnode->attr_value("name") + "/", cnode, section_type);
            } else {
                VS_LOG(warning, "neither a variable nor a section");
            }
        }
    }
}

/* *********************************************************** */

void VegaConfig::checkSection(configNode *node, enum section_t section_type) {
    if (node->Name() != "section") {
        VS_LOG(warning, "not a section");
        node->printNode(std::cout, 0, 1);

        return;
    }
    doSection(node->attr_value("name") + "/", node, section_type);
}

/* *********************************************************** */

void VegaConfig::doVar(string prefix, configNode *node) {
    string name = node->attr_value("name");
    string value = node->attr_value("value");
    string hashname = prefix + name;
    map_variables[hashname] = value;
    if (name.empty()) {
        VS_LOG(warning, (boost::format("no name given for variable %1% %2% ") % name % value));
    }
}

/* *********************************************************** */

void VegaConfig::checkVar(configNode *node) {
    if (node->Name() != "var") {
        VS_LOG(warning, "not a variable");
        return;
    }
    doVar("", node);
}

/* *********************************************************** */

bool VegaConfig::checkColor(string prefix, configNode *node) {
    if (node->Name() != "color") {
        VS_LOG(warning, "no color definition");
        return false;
    }
    if (node->attr_value("name").empty()) {
        VS_LOG(warning, "no color name given");
        return false;
    }
    string name = node->attr_value("name");
    string hashname = prefix + name;

    vColor *color;
    color = new vColor;
    vColor &vc = map_colors[hashname];

    if (node->attr_value("ref").empty()) {
        string r = node->attr_value("r");
        string g = node->attr_value("g");
        string b = node->attr_value("b");
        string a = node->attr_value("a");
        if (r.empty() || g.empty() || b.empty() || a.empty()) {
            VS_LOG(warning, (boost::format("neither name nor r,g,b given for color %1%") % node->Name()));
            delete color;
            color = nullptr;
            return false;
        }
        float rf = atof(r.c_str());
        float gf = atof(g.c_str());
        float bf = atof(b.c_str());
        float af = atof(a.c_str());

        vc.name.erase();
        vc.r = rf;
        vc.g = gf;
        vc.b = bf;
        vc.a = af;

        color->r = rf;
        color->g = gf;
        color->b = bf;
        color->a = af;
    } else {

        string ref_section = node->attr_value("section");
        string ref_name = node->attr_value("ref");
        if (ref_section.empty()) {
            VS_LOG(warning, "you have to give a referenced section when referencing colors");
            ref_section = "default";
        }
        GFXColor refcol;
        refcol = getColor(ref_section, ref_name, refcol);

        vc.name = ref_section + "/" + ref_name;
        vc.r = refcol.r;
        vc.g = refcol.g;
        vc.b = refcol.b;
        vc.a = refcol.a;

        color->r = refcol.r;
        color->g = refcol.g;
        color->b = refcol.b;
        color->a = refcol.a;
    }
    color->name = name;
    node->color = color;

    return true;
}

/* *********************************************************** */

void VegaConfig::doColors(configNode *node) {
    if (colors != NULL) {
        VS_LOG(warning, "only one variable section allowed");
        return;
    }
    colors = node;

    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        checkSection(cnode, SECTION_COLOR);
    }
}

/* *********************************************************** */

string VegaConfig::getVariable(string section, string subsection, string name, string defaultvalue) {
    string hashname = section + "/" + subsection + "/" + name;
    std::map<string, string>::iterator it;
    if ((it = map_variables.find(hashname)) != map_variables.end()) {
        return (*it).second;
    } else {
        return defaultvalue;
    }
}

/* *********************************************************** */

string VegaConfig::getVariable(string section, string name, string defaultval) {
    string hashname = section + "/" + name;
    std::map<string, string>::iterator it;
    if ((it = map_variables.find(hashname)) != map_variables.end()) {
        return (*it).second;
    } else {
        return defaultval;
    }
}

/* *********************************************************** */

string VegaConfig::getVariable(configNode *section, string name, string defaultval) {
    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = section->subnodes.begin(); siter != section->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        if ((cnode)->attr_value("name") == name) {
            return (cnode)->attr_value("value");
        }
    }
    static bool foundshouldwarn = false;
    static bool shouldwarn = true;
    if (!foundshouldwarn) {
        if (name != "debug_config") {
            shouldwarn = XMLSupport::parse_bool(getVariable("general", "debug_config", "true"));
            foundshouldwarn = true;
        }
    }
    if (shouldwarn) {
        VS_LOG(warning,
                (boost::format("WARNING: no var named %1% in section %2% using default: %3%") % name
                        % section->attr_value("name") % defaultval));
    }
    return defaultval;
}

GFXColor VegaConfig::getColor(string section, string name, GFXColor default_color) {
    string hashname = section + "/" + name;
    std::map<string, vColor>::iterator it;
    if ((it = map_colors.find(hashname)) != map_colors.end()) {
        return GFXColor((*it).second.r, (*it).second.g, (*it).second.b, (*it).second.a);
    } else {
        return default_color;
    }
}

/* *********************************************************** */

GFXColor VegaConfig::getColor(configNode *node, string name, GFXColor default_color) {
    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = node->subnodes.begin(); siter != node->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        if ((cnode)->attr_value("name") == name) {
            return GFXColor((cnode)->color->r, (cnode)->color->g, (cnode)->color->b, (cnode)->color->a);
        }
    }
    VS_LOG(warning, (boost::format("WARNING: color %1% not defined, using default") % name));
    return default_color;
}

/* *********************************************************** */

configNode *VegaConfig::findEntry(string name, configNode *startnode) {
    return findSection(name, startnode);
}

/* *********************************************************** */

configNode *VegaConfig::findSection(string section, configNode *startnode) {
    std::vector<easyDomNode *>::const_iterator siter;
    for (siter = startnode->subnodes.begin(); siter != startnode->subnodes.end(); siter++) {
        configNode *cnode = (configNode *) (*siter);
        string scan_name = (cnode)->attr_value("name");
        if (scan_name == section) {
            return cnode;
        }
    }
    VS_LOG(warning, (boost::format("WARNING: no section/variable/color named %1%") % section));

    return nullptr;
}

/* *********************************************************** */

void VegaConfig::setVariable(configNode *entry, string value) {
    entry->set_attribute("value", value);
}

/* *********************************************************** */

bool VegaConfig::setVariable(string section, string name, string value) {
    configNode *sectionnode = findSection(section, variables);
    if (sectionnode != NULL) {
        configNode *varnode = findEntry(name, sectionnode);
        if (varnode != NULL) {
            //now set the thing
            setVariable(varnode, value);
        }
    }
    string hashname = section + "/" + name;
    map_variables[hashname] = value;
    return true;
}

bool VegaConfig::setVariable(string section, string subsection, string name, string value) {
    configNode *sectionnode = findSection(section, variables);
    if (sectionnode != NULL) {
        configNode *subnode = findSection(name, sectionnode);
        if (subnode != NULL) {
            configNode *varnode = findEntry(name, subnode);
            if (varnode != NULL) {
                //now set the thing
                setVariable(varnode, value);
            }
        }
    }
    string hashname = section + "/" + subsection + "/" + name;
    map_variables[hashname] = value;
    return true;
}

