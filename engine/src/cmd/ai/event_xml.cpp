/*
 * event_xml.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#include "cmd/unit_generic.h"
#include "xml_support.h"
#include "event_xml.h"
#include <string>
#include <vector>
#include <list>
#include <float.h>
#include <assert.h>
#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "vs_globals.h"
#include "configxml.h"
extern bool validateHardCodedScript(std::string s);
//serves to run through a XML file that nests things for "and".

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;
namespace AIEvents {
AIEvresult::AIEvresult(int type,
        float const min,
        const float max,
        float timetofinish,
        float timeuntilinterrupts,
        float priority,
        const std::string &aiscript) {
    this->timetointerrupt = timeuntilinterrupts;
    this->timetofinish = timetofinish;
    this->type = type;
    this->priority = priority;
    this->max = max;

    this->min = min;

    this->script = aiscript;
    if (!validateHardCodedScript(this->script)) {
        static int aidebug = XMLSupport::parse_int(vs_config->getVariable("AI", "debug_level", "0"));
        if (aidebug) {
            for (int i = 0; i < 20; ++i) {
                VS_LOG(serious_warning, (boost::format("SERIOUS WARNING %1%") % this->script.c_str()));
            }
        }
        VS_LOG(serious_warning, (boost::format(
                "SERIOUS WARNING in AI script: no fast method to perform %1$s when type %2$d is at least %3$f and at most %4$f with priority %5$f for %6$f time")
                % this->script.c_str()
                % type
                % min
                % max
                % priority
                % timetofinish));
    }
}

const int AIUNKNOWN = 0;
const int AIMIN = 1;
const int AIMAX = 2;
const int AISCRIPT = 3;
const int AINOT = 4;
const int TIMEIT = 5;
const int OBEDIENCE = 6;
const int TIMETOINTERRUPT = 7;
const int PRIORITY = 8;
const XMLSupport::EnumMap::Pair AIattribute_names[] = {
        EnumMap::Pair("UNKNOWN", AIUNKNOWN),
        EnumMap::Pair("min", AIMIN),
        EnumMap::Pair("max", AIMAX),
        EnumMap::Pair("not", AINOT),
        EnumMap::Pair("Script", AISCRIPT),
        EnumMap::Pair("time", TIMEIT),
        EnumMap::Pair("obedience", OBEDIENCE),
        EnumMap::Pair("timetointerrupt", TIMETOINTERRUPT),
        EnumMap::Pair("priority", PRIORITY)
};
const XMLSupport::EnumMap attr_map(AIattribute_names, 9);

void GeneralAIEventBegin(void *userData, const XML_Char *name, const XML_Char **atts) {
    AttributeList attributes(atts);
    string aiscriptname("");
    float min = -FLT_MAX;
    float max = FLT_MAX;
    ElemAttrMap *eam = ((ElemAttrMap *) userData);
    float timetofinish = eam->maxtime;
    float timetointerrupt = 0;
    int elem = eam->element_map.lookup(name);
    AttributeList::const_iterator iter;
    float priority = 4;
    eam->level++;
    if (elem == 0) {
        eam->result.push_back(std::list<AIEvresult>());
        eam->result.push_back(std::list<AIEvresult>());
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch (attr_map.lookup((*iter).name)) {
                case TIMEIT:
                    eam->maxtime = (short) XMLSupport::parse_float((*iter).value);
                    break;
                case OBEDIENCE:
                    eam->obedience = (float) (XMLSupport::parse_float((*iter).value));
            }
        }
    } else {
        assert(eam->level != 1 && eam->result.size() >= 2);         //might not have a back on result();
        if (eam->result.back().size() != eam->result[eam->result.size() - 2].size()) {
            eam->result.push_back(eam->result.back());
        }
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch (attr_map.lookup((*iter).name)) {
                case AINOT:
                    elem = -elem;                 //since elem can't be 0 (see above) this will save the "not" info
                    break;
                case AIMIN:
                    min = XMLSupport::parse_float((*iter).value);
                    break;
                case AIMAX:
                    max = XMLSupport::parse_float((*iter).value);
                    break;
                case AISCRIPT:
                    aiscriptname = (*iter).value;
                    break;
                case TIMEIT:
                    timetofinish = XMLSupport::parse_float((*iter).value);
                    break;
                case TIMETOINTERRUPT:
                    timetointerrupt = XMLSupport::parse_float((*iter).value);
                    break;
                case PRIORITY:
                    priority = XMLSupport::parse_float((*iter).value);
                    break;
                default:
                    break;
            }
        }
        AIEvresult newelem(elem, min, max, timetofinish, timetointerrupt, priority, aiscriptname);
        eam->result.back().push_back(newelem);
        eam->result[eam->result.size() - 2].push_back(newelem);
    }
}

void GeneralAIEventEnd(void *userData, const XML_Char *name) {
    ElemAttrMap *eam = ((ElemAttrMap *) userData);
    eam->level--;
    if (eam->result.back().size() == 0) {
        eam->result.pop_back();
        assert(eam->level == 0);
    } else {
        eam->result.back().pop_back();
    }
}

void LoadAI(const char *filename, ElemAttrMap &result, const string &faction) {
    //returns obedience
    using namespace VSFileSystem;
    static float cfg_obedience = XMLSupport::parse_float(vs_config->getVariable("AI",
            "Targetting",
            "obedience",
            ".99"));
    result.obedience = cfg_obedience;
    result.maxtime = 10;
    VSFile f;
    VSError err;
    err = f.OpenReadOnly(filename, AiFile);
    if (err > Ok) {
        VS_LOG(warning, (boost::format("ai file %1% not found") % filename));
        string full_filename = filename;
        full_filename = full_filename.substr(0, strlen(filename) - 4);
        string::size_type where = full_filename.find_last_of(".");
        string type = ".agg.xml";
        if (where != string::npos) {
            type = full_filename.substr(where);
            full_filename = full_filename.substr(0, where) + ".agg.xml";
            err = f.OpenReadOnly(full_filename, AiFile);
        }
        if (err > Ok) {
            VS_LOG(warning, (boost::format("ai file %1% again not found") % full_filename));
            full_filename = "default";
            full_filename += type;
            err = f.OpenReadOnly(full_filename, AiFile);
        }
        if (err > Ok) {
            VS_LOG(warning, (boost::format("ai file again %1% again not found") % full_filename));
            err = f.OpenReadOnly("default.agg.xml", AiFile);
            if (err > Ok) {
                VS_LOG_AND_FLUSH(error, "ai file again default.agg.xml again not found");
                return;                 //Who knows what will happen now? Crash?    // TODO: VSExit()?
            }
        }
    }
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &result);
    XML_SetElementHandler(parser, &GeneralAIEventBegin, &GeneralAIEventEnd);
    XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
    f.Close();
    XML_ParserFree(parser);
    if (result.level != 0) {
        VS_LOG(error,
                (boost::format("Error loading AI script %1% for faction %2%. Final count not zero.") % filename
                        % faction.c_str()));
    }
    result.level = 0;
}
}

