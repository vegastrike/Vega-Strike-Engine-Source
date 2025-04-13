/*
 * galaxy_xml.cpp
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

/*
 *  Daniel Horn
 */

#include <expat.h>
#include "xml_support.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "vs_globals.h"

#include "galaxy_xml.h"
#include "galaxy_gen.h"
#ifdef WRITEGALAXYCOORDS
#include "gfx/nav/navscreen.h"
#endif
#include <float.h>
using namespace XMLSupport;
using namespace VSFileSystem;

namespace GalaxyXML {
enum GalaxyNames {
    UNKNOWN,
    GALAXY,
    SYSTEMS,
    SECTOR,
    SYSTEM,
    VAR,
    NAME,
    VALUE,
    PLANETS,
    PLANET
};
const EnumMap::Pair element_names[8] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("Galaxy", GALAXY),
        EnumMap::Pair("Systems", SYSTEMS),
        EnumMap::Pair("Sector", SECTOR),
        EnumMap::Pair("System", SYSTEM),
        EnumMap::Pair("Planets", PLANETS),
        EnumMap::Pair("Planet", PLANET),
        EnumMap::Pair("Var", VAR)
};
const EnumMap::Pair attribute_names[3] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("name", NAME),
        EnumMap::Pair("value", VALUE)
};

const EnumMap element_map(element_names, 8);
const EnumMap attribute_map(attribute_names, 3);
class XML {
public:
    SGalaxy *g;
    std::vector<std::string> stak;
};

void beginElement(void *userdata, const XML_Char *nam, const XML_Char **atts) {
    AttributeList::const_iterator iter;
    XML *xml = (XML *) userdata;
    string tname(nam);
    AttributeList attributes(atts);
    GalaxyNames elem = (GalaxyNames) element_map.lookup(tname);
    GalaxyNames attr;
    string name;
    string value;
    switch (elem) {
        case GALAXY:
            break;
        case SYSTEMS:
            break;
        case PLANETS:
            xml->stak.push_back("<planets>");
            xml->g->addSection(xml->stak);
            break;
        case SECTOR:
        case SYSTEM:

        case PLANET:
            for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
                attr = (GalaxyNames) attribute_map.lookup((*iter).name);
                switch (attr) {
                    case NAME:
                        name = (*iter).value;
                        break;
                    default:
                        break;
                }
            }
            xml->stak.push_back(name);
            xml->g->addSection(xml->stak);

            break;
        case VAR:
            for (iter = attributes.begin(); iter != attributes.end(); ++iter) {
                attr = (GalaxyNames) attribute_map.lookup((*iter).name);
                switch (attr) {
                    case NAME:
                        name = (*iter).value;
                        break;
                    case VALUE:
                        value = (*iter).value;
                        break;
                    default:
                        break;
                }
            }
            xml->g->setVariable(xml->stak, name, value);
            break;
        default:
            break;
    }
}

void endElement(void *userdata, const XML_Char *nam) {
    XML *xml = (XML *) userdata;
    string name(nam);
    GalaxyNames elem = (GalaxyNames) element_map.lookup(name);
    switch (elem) {
        case GALAXY:
        case SYSTEMS:
            break;
        case VAR:
            break;
        case SECTOR:
        case SYSTEM:
        case PLANETS:
        case PLANET:
            xml->stak.pop_back();
            break;
        default:
            break;
    }
}
}

using namespace GalaxyXML;

SGalaxy::~SGalaxy() {
    if (subheirarchy != nullptr) {
        delete subheirarchy;
        subheirarchy = nullptr;
    }
}

SGalaxy &SGalaxy::operator=(const SGalaxy &g) {
    if (g.subheirarchy) {
        SubHeirarchy *temp = new SubHeirarchy(*g.subheirarchy);
        if (subheirarchy != nullptr) {
            delete subheirarchy;
        }
        subheirarchy = temp;
    } else if (subheirarchy != nullptr) {
        delete subheirarchy;
        subheirarchy = nullptr;
    }
    data = g.data;
    return *this;
}

SGalaxy::SGalaxy(const SGalaxy &g) : data(g.data) {
    if (g.subheirarchy) {
        subheirarchy = new SubHeirarchy(*g.subheirarchy);
    } else {
        subheirarchy = nullptr;
    }
}

void SGalaxy::processSystem(const string &sys, const QVector &coords) {
    string sector = getStarSystemSector(sys);
    string sys2 = getStarSystemName(sys);
    char coord[65536];
    sprintf(coord, "%lf %lf %lf", coords.i, coords.j, coords.k);
    string ret = getVariable(sector, sys2, "");
    if (ret.length() == 0) {
        setVariable(sector, sys2, "xyz", coord);
    }
}

void SGalaxy::processGalaxy(const string &sys) {
#ifdef WRITEGALAXYCOORDS
    NavigationSystem::SystemIterator si( sys, 256000 );
    while ( !si.done() ) {
        string sys = *si;
        processSystem( sys, si.Position() );
        ++si;
    }
#endif
}

void dotabs(VSFileSystem::VSFile &f, unsigned int tabs) {
    for (unsigned int i = 0; i < tabs; ++i) {
        f.Fprintf("\t");
    }
}

void SGalaxy::writeSector(VSFileSystem::VSFile &f, int tabs, const string &sectorType, SGalaxy *planet_types) const {
    StringMap::const_iterator dat;
    for (dat = data.begin(); dat != data.end(); ++dat) {
        if ((*dat).first != "jumps") {
            dotabs(f, tabs);
            f.Fprintf("<var name=\"%s\" value=\"%s\"/>\n", (*dat).first.c_str(), (*dat).second.c_str());
        }
    }
    dat = data.find(std::string("jumps"));
    if (dat != data.end()) {
        dotabs(f, tabs);
        f.Fprintf("<var name=\"jumps\" value=\"%s\"/>\n", (*dat).second.c_str());
    }
    if (subheirarchy) {
        for (SubHeirarchy::const_iterator it = subheirarchy->begin(); it != subheirarchy->end(); ++it) {
            if (&(*it).second != planet_types) {
                dotabs(f, tabs);
                f.Fprintf("<%s name=\"%s\">\n", sectorType.c_str(), (*it).first.c_str());
                (*it).second.writeSector(f, tabs + 1, sectorType, planet_types);
                dotabs(f, tabs);
                f.Fprintf("</%s>\n", sectorType.c_str());
            }
        }
    }
}

void SGalaxy::writeGalaxy(VSFile &f) const {
    f.Fprintf("<galaxy>\n<systems>\n");
    writeSector(f, 1, "sector", NULL);
    f.Fprintf("</systems>\n");
    f.Fprintf("</galaxy>\n");
}

void Galaxy::writeGalaxy(VSFile &f) const {
    f.Fprintf("<galaxy>\n<systems>\n");
    writeSector(f, 1, "sector", planet_types);
    f.Fprintf("</systems>\n");
    if (planet_types) {
        f.Fprintf("<planets>\n");
        planet_types->writeSector(f, 1, "planet", NULL);
        f.Fprintf("</planets>\n");
    }
    f.Fprintf("</galaxy>\n");
}

SGalaxy::SGalaxy(const char *configfile) {
    using namespace VSFileSystem;
    subheirarchy = NULL;
    VSFile f;
    VSError err = f.OpenReadOnly(configfile, UniverseFile);
    if (err <= Ok) {
        GalaxyXML::XML x;
        x.g = this;

        XML_Parser parser = XML_ParserCreate(NULL);
        XML_SetUserData(parser, &x);
        XML_SetElementHandler(parser, &GalaxyXML::beginElement, &GalaxyXML::endElement);
        XML_Parse(parser, (f.ReadFull()).c_str(), f.Size(), 1);
        f.Close();

        XML_ParserFree(parser);
    }
}

SubHeirarchy &SGalaxy::getHeirarchy() {
    if (!subheirarchy) {
        subheirarchy = new SubHeirarchy;
    }
    return *subheirarchy;
}
/* *********************************************************** */

/* *********************************************************** */
const string &SGalaxy::getVariable(const std::vector<string> &section, const string &name,
        const string &default_value) const {
    const SGalaxy *g = this;
    for (unsigned int i = 0; i < section.size(); ++i) {
        if (g->subheirarchy) {
            SubHeirarchy::const_iterator sub = subheirarchy->find(section[i]);
            if (sub != subheirarchy->end()) {
                g = &(*sub).second;
            } else {
                return default_value;
            }
        } else {
            return default_value;
        }
    }
    StringMap::const_iterator dat = data.find(name);
    if (dat != data.end()) {
        return (*dat).second;
    }
    return default_value;
}

SGalaxy *Galaxy::getPlanetTypes() {
    return planet_types;
}

void SGalaxy::addSection(const std::vector<string> &section) {
    SubHeirarchy *temp = &getHeirarchy();
    for (unsigned int i = 0; i < section.size(); ++i) {
        temp = &((*temp)[section[i]].getHeirarchy());
    }
}

void SGalaxy::setVariable(const std::vector<string> &section, const string &name, const string &value) {
    SGalaxy *g = this;
    for (unsigned int i = 0; i < section.size(); ++i) {
        g = &g->getHeirarchy()[section[i]];
    }
    g->data[name] = value;
}

/* *********************************************************** */

bool SGalaxy::setVariable(const string &section, const string &name, const string &value) {
    getHeirarchy()[section].data[name] = value;
    return true;
}

bool SGalaxy::setVariable(const string &section, const string &subsection, const string &name, const string &value) {
    getHeirarchy()[section].getHeirarchy()[subsection].data[name] = value;
    return true;
}

const string &SGalaxy::getRandSystem(const string &sect, const string &def) const {
    if (subheirarchy) {
        const SGalaxy &sector = (*subheirarchy)[sect];
        if (sector.subheirarchy) {
            unsigned int size = sector.subheirarchy->size();
            if (size > 0) {
                int which = rand() % size;
                SubHeirarchy::const_iterator i =
                        sector.subheirarchy->begin();
                while (which > 0) {
                    --which, ++i;
                }
                return (*i).first;
            }
        }
    }
    return def;
}

const string &SGalaxy::getVariable(const string &section,
        const string &subsection,
        const string &name,
        const string &defaultvalue) const {
#ifdef WRITEGALAXYCOORDS
    static bool blah = false;
    if (!blah) {
        processGalaxy( "Sol/Sol" );
        writeGalaxy( "/tmp/outputgalaxy" );
        blah = true;
    }
#endif
    SubHeirarchy *s = subheirarchy;
    SubHeirarchy::const_iterator i;
    if (s) {
        i = s->find(section);
        if (i != s->end()) {
            s = (*i).second.subheirarchy;
            if (s) {
                i = s->find(subsection);
                if (i != s->end()) {
                    const SGalaxy *g = &(*i).second;
                    StringMap::const_iterator j = g->data.find(name);
                    if (j != g->data.end()) {
                        return (*j).second;
                    }
                }
            }
        }
    }
    return defaultvalue;
}

const string &SGalaxy::getVariable(const string &section, const string &name, const string &defaultvalue) const {
    SubHeirarchy *s = subheirarchy;
    SubHeirarchy::const_iterator i;
    if (s) {
        i = s->find(section);
        if (i != s->end()) {
            const SGalaxy *g = &(*i).second;
            StringMap::const_iterator j = g->data.find(name);
            if (j != g->data.end()) {
                return (*j).second;
            }
        }
    }
    return defaultvalue;
}

bool Galaxy::setPlanetVariable(const string &name, const string &value) {
    if (!planet_types) {
        return false;
    }
    planet_types->data[name] = value;
    return true;
}

bool Galaxy::setPlanetVariable(const string &section, const string &name, const string &value) {
    if (!planet_types) {
        return false;
    }
    planet_types->getHeirarchy()[section].data[name] = value;
    return true;
}

const string &Galaxy::getPlanetVariable(const string &section, const string &name, const string &defaultvalue) const {
    SGalaxy *planet_types = &((*subheirarchy->find("<planets>")).second);
    if (planet_types) {
        SubHeirarchy::const_iterator i;
        i = planet_types->subheirarchy->find(section);
        if (i == planet_types->subheirarchy->end()) {
            return getPlanetVariable(name, defaultvalue);
        } else {
            const SGalaxy *g = &(*i).second;
            StringMap::const_iterator j = g->data.find(name);
            if (j == g->data.end()) {
                return getPlanetVariable(name, defaultvalue);
            } else {
                return (*j).second;
            }
        }
    }
    return defaultvalue;
}

const string &Galaxy::getPlanetVariable(const string &name, const string &defaultvalue) const {
    if (planet_types) {
        StringMap::const_iterator j = planet_types->data.find(name);
        if (j != planet_types->data.end()) {
            return (*j).second;
        }
    }
    return defaultvalue;
}

SGalaxy *Galaxy::getInitialPlanetTypes() {
    if (subheirarchy) {
        SubHeirarchy::iterator iter = subheirarchy->find("<planets>");
        if (iter == subheirarchy->end()) {
            return NULL;
        } else {
            return &(*iter).second;
        }
    }
    return NULL;
}

void Galaxy::setupPlanetTypeMaps() {
    if (planet_types) {
        SubHeirarchy::iterator i = planet_types->getHeirarchy().begin();
        for (; i != planet_types->getHeirarchy().end(); ++i) {
            string name = (*i).first;

            string val;

            {
                static const string _unit("unit");
                const string &unit = (*i).second[_unit];

                if (!unit.empty()) {
                    val = unit;
                } else {
                    static const string _texture("texture");
                    const string &tex = (*i).second[_texture];
                    if (!tex.empty()) {
                        val = tex;

                        // Filter out irrelevant texture bits
                        // We only want the base name of the diffuse map
                        // Texture formats are: <path>/<diffse>.<extension>|<path>/<specular>.<extension>|...
                        string::size_type pipe = val.find_first_of('|');
                        if (pipe != string::npos) {
                            val = val.substr(0, pipe);
                        }
                        string::size_type slash = val.find_last_of('/');
                        if (slash != string::npos) {
                            val = val.substr(slash + 1);
                        }
                        string::size_type dot = val.find_last_of('.');
                        if (dot != string::npos) {
                            val = val.substr(0, dot);
                        }

                        static const string numtag = "#num#";
                        static const string::size_type numtaglen = numtag.length();
                        string::size_type tagpos;
                        while (string::npos != (tagpos = val.find(numtag))) {
                            val.erase(tagpos, numtaglen);
                        }
                    }
                }
            }

            if (texture2name.find(val) != texture2name.end()) {
                VS_LOG(warning,
                        (boost::format("name conflict %1% has texture %2% and %3% has texture %4%")
                                % name.c_str()
                                % val.c_str()
                                % texture2name[val].c_str()
                                % val.c_str()));
            } else {
                texture2name[val] = name;
            }
            val = (*i).second["initial"];
            if (initial2name.find(val) != initial2name.end()) {
                VS_LOG(warning,
                        (boost::format("name conflict %1% has initial %2% and %3% has initial %4%")
                                % name.c_str()
                                % val.c_str()
                                % initial2name[val].c_str()
                                % val.c_str()));
            } else {
                initial2name[val] = name;
            }
        }
    }
    if (initial2name.empty() || texture2name.empty()) {
        VS_LOG(warning, "Warning, galaxy contains no overarching planet info");
    }
}

Galaxy::Galaxy(const SGalaxy &g) : SGalaxy(g) {
    planet_types = getInitialPlanetTypes();
    setupPlanetTypeMaps();
}

Galaxy::Galaxy(const char *configfile) : SGalaxy(configfile) {
    planet_types = getInitialPlanetTypes();
    setupPlanetTypeMaps();
}

