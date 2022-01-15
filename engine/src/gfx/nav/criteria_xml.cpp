/*
 * criteria_xml.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2003 Mike Byron
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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


#include "vegastrike.h"
#if defined (_WIN32) && !defined (__CYGWIN__) && !defined (__MINGW32__)
//For WIN32 debugging.
#include <crtdbg.h>
#endif

#include <assert.h>
#include <set>
#include <string>
#include <expat.h>

#include "criteria.h"
#include "xml_support.h"
#include "vsfilesystem.h"
#include "vs_logging.h"

using XMLSupport::AttributeList;
using namespace VSFileSystem;   // FIXME -- Shouldn't include entire namespace
using std::set;
using std::string;

void CriteriaContains::beginElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    AttributeList attributes(atts);
    if (string(name) == "Planet") {
        for (AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); iter++) {
            if ((*iter).name == "file") {
                static_cast< set<string> * > (userData)->insert(string((*iter).value));
            }
        }
    }
}

void CriteriaContains::endElement(void *userData, const XML_Char *name)
{
}

std::set<std::string> CriteriaContains::getPlanetTypesFromXML(const char *filename) const
{
    set<string> textures;

    VSFile f;
    VSError err;
    err = f.OpenReadOnly(string(string(filename) + string(".system")).c_str(), SystemFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("CriteriaContains: file not found %1%") % filename));
        return textures;
    }
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &textures);
    XML_SetElementHandler(parser, &CriteriaContains::beginElement, &CriteriaContains::endElement);
    XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
    f.Close();
    XML_ParserFree(parser);

    return textures;
}

