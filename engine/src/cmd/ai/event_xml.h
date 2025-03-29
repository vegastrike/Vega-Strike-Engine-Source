/**
 * event_xml.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_EVENT_XML_H
#define VEGA_STRIKE_ENGINE_CMD_AI_EVENT_XML_H

#include "root_generic/xml_support.h"
#include <string>
#include <vector>
#include <list>
/**
 * General namespace that does nothing on its own, but
 * Deals with the parsing of an XML file that contains a number of
 * event "if" statements
 * Each statement can have a minimum, max value and a "not" flag to invert it
 * and the type references the enum in the class using this one, and
 * is eventually only used for the "not" tag
 */
namespace AIEvents {
///A struct indicating an event that may or may not be executed
struct AIEvresult {
    ///will never be zero...negative indicates "not"
    int type;
    ///The maximum/minimum values that will cause this event
    float max, min;
    float timetofinish;
    float timetointerrupt;
    float priority;
    ///The string indicating what type of thing this event evaluates
    std::string script;
    AIEvresult(int type,
            float const min,
            const float max,
            float timetofinish,
            float timetointerrupt,
            float priority,
            const std::string &aiscript);

    bool Eval(const float eval) const {
        if (eval >= min) {
            if (eval < max) {
                if (type > 0) {
                    return true;
                }
            }
        }
        if (eval < min) {
            if (eval >= max) {
                if (type < 0) {
                    return true;
                }
            }
        }
        return false;
    }
};
struct ElemAttrMap {
    XMLSupport::EnumMap element_map;
    int level;
    float curtime;
    float maxtime;
    float obedience;                                              //short fix
    std::vector<std::list<AIEvresult> > result;

    ElemAttrMap(const XMLSupport::EnumMap &el) :
            element_map(el), level(0) {
    }
};
void LoadAI(const char *filename, ElemAttrMap &result, const std::string &faction);  //num seconds
}

#endif //VEGA_STRIKE_ENGINE_CMD_AI_EVENT_XML_H
