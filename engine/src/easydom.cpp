/*
 * easydom.cpp
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
 *  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <expat.h>
#include "easydom.h"

#include <assert.h>     /// needed for assert() calls.

easyDomNode::easyDomNode() {
}

void easyDomNode::set(easyDomNode *_parent, std::string _name, const XML_Char **atts) {
    parent = _parent;
    if (atts != nullptr) {
        for (; *atts != nullptr; atts += 2) {
#if 0
            att_name.push_back( (*iter).name );
            att_value.push_back( (*iter).value );
#endif
            attribute_map[atts[0]] = atts[1];
        }
    }
    name = _name;
}

void easyDomNode::addChild(easyDomNode *child) {
    subnodes.push_back(child);
}

std::string easyDomNode::attr_value(std::string search_name) {
    return attribute_map[search_name];
}

void easyDomNode::printNode(std::ostream &out, int recurse_level, int level) {
    vsUMap<std::string, std::string>::const_iterator iter;

    out << "<" << name;
    for (iter = attribute_map.begin(); iter != attribute_map.end(); iter++) {
        out << " " << (*iter).first << "=\"" << (*iter).second << "\"";
    }
    out << ">" << std::endl;

    std::vector<easyDomNode *>::const_iterator siter;
    if (recurse_level > 0) {
        for (siter = subnodes.begin(); siter != subnodes.end(); siter++) {
            (*siter)->printNode(out, recurse_level - 1, level + 1);
        }
    }
    if (!(recurse_level == 0 && level == 0)) {
        out << "</" << name << ">" << std::endl;
    }
}

const char *textAttr = "Text_Attr";

