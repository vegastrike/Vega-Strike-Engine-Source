/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "root_generic/xml_serializer.h"
#include "cmd/images.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "root_generic/xml_support.h"

///Assumes that the tag is  <Mount type=\"  and that it will finish with " ></Mount>
using namespace XMLSupport;
using namespace VSFileSystem;

std::string intStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(*input.w.i);
}

std::string uintStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(*input.w.ui);
}

std::string floatStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(*input.w.f);
}

std::string fabsFloatStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((float) fabs(*input.w.f));
}

std::string absIntStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((int) abs(*input.w.i));
}

std::string scaledFloatStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((float) ((*input.w.f) / XMLSupport::parse_float(input.str)));
}

std::string angleStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((float((*input.w.f) * 180 / 3.1415926536)));
}

std::string doubleStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((float) (*input.w.d));
}

std::string boolStarHandler(const XMLType &input, void *mythis) {
    if (*input.w.b) {
        return "1";
    }
    return "0";
}

std::string charStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(*input.w.c);
}

std::string ucharStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(*input.w.uc);
}

std::string negationCharStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(-(*input.w.c));
}

std::string negationIntStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(-(*input.w.i));
}

std::string negationFloatStarHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(-(*input.w.f));
}

std::string stringStarHandler(const XMLType &input, void *mythis) {
    if (!input.w.p) {
        return string("");
    }
    return *((string *) (input.w.p));
}

std::string stringHandler(const XMLType &input, void *mythis) {
    return input.str;
}

std::string intHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(input.w.hardint);
}

std::string floatHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(input.w.hardfloat);
}

std::string lessNeg1Handler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(((*input.w.c) < -1) ? 1 : 0);
}

std::string cloakHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring(((*input.w.i) == -1) ? 1 : 0);       //short fix
}

std::string intToFloatHandler(const XMLType &input, void *mythis) {
    return XMLSupport::tostring((float) (((float) (*input.w.i)) / ((float) (2147483647))));
}

void XMLElement::Write(VSFileSystem::VSFile &f, void *mythis) {
    f.Fprintf(" %s=\"%s\"", elem.c_str(), ((*handler)(value, mythis)).c_str());
}

static void Tab(VSFileSystem::VSFile &f) {
    f.Fprintf("\t");
}

static void Tab(VSFileSystem::VSFile &f, int level) {
    for (int i = 0; i < level; i++) {
        Tab(f);
    }
}

void XMLnode::Write(VSFileSystem::VSFile &f, void *mythis, int level) {
    Tab(f, level);
    f.Fprintf("<%s", val.c_str());
    for (auto & element : elements) {
        element.Write(f, mythis);
    }
    if (subnodes.empty()) {
        f.Fprintf("/>\n");
    } else {
        f.Fprintf(">\n");
        for (auto & subnode : subnodes) {
            subnode.Write(f, mythis, level + 1);
        }
        Tab(f, level);
        f.Fprintf("</%s>\n", val.c_str());
    }
}

void XMLSerializer::Write(const char *modificationname) {
    if (modificationname) {
        if (strlen(modificationname) != 0) {
            savedir = modificationname;
        }
    }
    VSFileSystem::CreateDirectoryHome(VSFileSystem::savedunitpath + "/" + savedir);
    VSFile f;
    VSError err = f.OpenCreateWrite(savedir + "/" + this->filename, UnitFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("!!! ERROR : Writing saved unit file : %1%") % f.GetFullPath().c_str()));
        return;
    }
    for (auto & subnode : topnode.subnodes) {
        subnode.Write(f, mythis, 0);
    }
    f.Close();
}

static string TabString(int level) {
    string ret{};
    for (int i = 0; i < level; ++i) {
        ret += '\t';
    }
    return ret;
}

string XMLSerializer::WriteString() {
    string ret{};
    for (auto & subnode : topnode.subnodes) {
        ret += subnode.WriteString(mythis, 0);
    }
    return ret;
}

string XMLnode::WriteString(void *mythis, int level) {
    const int MAXBUFFER = 16384;
    string ret;
    char buffer[MAXBUFFER];
    memset(buffer, 0, MAXBUFFER);
    ret = TabString(level);
    ret = ret + "<" + val;
    ret += string(buffer);
    for (auto & element : elements) {
        ret += element.WriteString(mythis);
    }
    if (subnodes.empty()) {
        ret += "/>\n";
    } else {
        ret += ">\n";
        for (auto & subnode : subnodes) {
            ret += subnode.WriteString(mythis, level + 1);
        }
        ret += TabString(level);
        ret = ret + "</" + val + ">\n";
    }
    return ret;
}

string XMLElement::WriteString(void *mythis) {
    string ret(" " + elem + "=\"" + ((*handler)(value, mythis)) + "\"");
    return ret;
}

XMLSerializer::XMLSerializer(const char *filename, const char *modificationname, void *mythis) : savedir(
        modificationname), mythis(mythis) {
    curnode = &topnode;
    //In network mode we don't care about saving filename, we want always to save with modification
    //name since we only work with savegames
    this->filename = string(filename);
}

void XMLSerializer::AddTag(const std::string &tag) {
    curnode->subnodes.emplace_back(tag, curnode);
    curnode = &curnode->subnodes.back();
}

void XMLSerializer::AddElement(const std::string &element, XMLHandler *handler, const XMLType &input) {
    curnode->elements.emplace_back(element, input, handler);
}

void XMLSerializer::EndTag(const std::string endname) {
    if (curnode) {
        if (endname == curnode->val) {
            curnode = curnode->up;
        }
    }
}

