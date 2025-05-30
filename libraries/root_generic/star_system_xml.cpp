/*
 * star_system_xml.cpp
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


#include <expat.h>
#include "root_generic/xml_support.h"
#include "cmd/planet.h"
#include "root_generic/vs_globals.h"
#include "vegadisk/vsfilesystem.h"
#include "root_generic/configxml.h"
#include "src/vegastrike.h"
#include <assert.h>                              /// needed for assert() calls.
#include "gfx_generic/mesh.h"
#include "cmd/building.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/fire.h"
#include "cmd/nebula.h"
#include "cmd/enhancement.h"
#include "cmd/script/flightgroup.h"
#include "src/universe_util.h"
#include "cmd/atmosphere.h"
#include "src/star_xml.h"
#include "cmd/planetary_orbit.h"
#include "root_generic/atmospheric_fog_mesh.h"

#include "root_generic/options.h"

#include "root_generic/system_factory.h"

#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

using namespace XMLSupport;
using namespace VSFileSystem;
extern const vector<string> &ParseDestinations(const string &value);
extern void bootstrap_draw(const string &message, Animation *SplashScreen = NULL);
extern void disableTerrainDraw(ContinuousTerrain *ct);

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

extern Flightgroup *getStaticBaseFlightgroup(int faction);
extern Flightgroup *getStaticNebulaFlightgroup(int faction);
extern Flightgroup *getStaticAsteroidFlightgroup(int faction);

template<typename T>
static bool EvalCondition(const char *op, const T &left, const T &right) {
    switch (op[0]) {
        case '>':
            switch (op[1]) {
                case 0:
                    return left > right;

                case '=':
                    return left >= right;

                default:
                    return false;
            }
        case '=':
            switch (op[1]) {
                case 0:
                case '=':
                    return left == right;

                default:
                    return false;
            }
        case '<':
            switch (op[1]) {
                case 0:
                    return left < right;

                case '=':
                    return left <= right;

                default:
                    return false;
            }
        case '!':
            switch (op[1]) {
                case '=':
                    return left != right;

                default:
                    return false;
            }
        default:
            return false;
    }
}

// Commented out 2020-08, issue #239
//static bool ConfigCondition( const string &cond )
//{
//    if ( cond.empty() )
//        return true;
//    char varname[64];
//    char op[3];
//    char varval[64];
//    bool ok = 3 == sscanf( cond.c_str(), "%63[-a-zA-Z_0-9] %2[<>=!] %63[-0-9.Ee]",
//                           varname,
//                           op,
//                           varval );
//    if (!ok)
//        return false;
//    //finalize character strings, for security
//    varname[sizeof (varname)/sizeof (*varname)-1] = 0;
//    op[sizeof (op)/sizeof (*op)-1] = 0;
//    varval[sizeof (varval)/sizeof (*varval)-1]    = 0;
//    //try to parse varval - if not parseable as float, assume it's a string
//    char *endptr = 0;
//    bool  rv;
//    if (endptr == varval) {
//        string sval = vs_config->getVariable( "graphics", varname, "0.0" );
//        rv = EvalCondition< string > ( op, sval, varval );
//    } else {
//        float fval = XMLSupport::parse_floatf( vs_config->getVariable( "graphics", varname, "0.0" ) );
//        float fref = XMLSupport::parse_floatf( varval );
//        rv = EvalCondition< float > ( op, fval, fref );
//    }
//    return rv;
//}

bool ConfigAllows(string var, float val) {
    bool invert = false;
    if (var.length() == 0) {
        return true;
    }
    if (var[0] == '-') {
        var = var.substr(1);
        invert = true;
    }
    float x = XMLSupport::parse_floatf(vs_config->getVariable("graphics", var, "0.0"));
    if (var.length() == 0) {
        return true;
    }
    return invert ? -x >= val : x >= val;
}

Vector ComputeRotVel(float rotvel, const QVector &r, const QVector &s) {
    if ((r.i || r.j || r.k) && (s.i || s.j || s.k)) {
        QVector retval = r.Cross(s);
        retval.Normalize();
        retval = retval * rotvel;
        return retval.Cast();
    } else {
        return Vector(0, rotvel, 0);
    }
}

void GetLights(const vector<GFXLight> &origlights, vector<GFXLightLocal> &curlights, const char *str, float lightSize) {
    int tint;
    char isloc;
    char *tmp = strdup(str);
    GFXLightLocal lloc;
    char *st = tmp;
    int numel;
    while ((numel = sscanf(st, "%d%c", &tint, &isloc)) > 0) {
        assert(tint < (int) origlights.size());
        lloc.ligh = origlights[tint];
        lloc.islocal = (numel > 1 && isloc == 'l');

        lloc.ligh.setSize(lightSize);

        curlights.push_back(lloc);
        while (isspace(*st)) {
            ++st;
        }
        while (isalnum(*st)) {
            ++st;
        }
    }
    free(tmp);
}

extern Unit *getTopLevelOwner();
extern BLENDFUNC parse_alpha(const char *);

void SetSubunitRotation(Unit *un, float difficulty) {
    Unit *unit;
    for (un_iter iter = un->getSubUnits(); (unit = *iter); ++iter) {
        float x = 2 * difficulty * ((float) rand()) / RAND_MAX - difficulty;
        float y = 2 * difficulty * ((float) rand()) / RAND_MAX - difficulty;
        float z = 2 * difficulty * ((float) rand()) / RAND_MAX - difficulty;
        unit->SetAngularVelocity(Vector(x, y, z));
        SetSubunitRotation(unit, difficulty);
    }
}

void parse_dual_alpha(const char *alpha, BLENDFUNC &blendSrc, BLENDFUNC &blendDst) {
    blendSrc = ONE;
    blendDst = ZERO;
    if (alpha == NULL) {
    } else if (alpha[0] == '\0') {
    } else {
        char *s = strdup(alpha);
        char *d = strdup(alpha);
        blendSrc = SRCALPHA;
        blendDst = INVSRCALPHA;
        if (2 == sscanf(alpha, "%s %s", s, d)) {
            if (strcmp(s, "true") != 0) {
                blendSrc = parse_alpha(s);
                blendDst = parse_alpha(d);
            }
        }
        free(s);
        free(d);
    }
}

int GetNumNearStarsScale() {
    return game_options()->num_near_stars;
}

int GetNumStarsScale() {
    return game_options()->num_far_stars;
}

float GetStarSpreadScale() {
    return game_options()->star_spreading * game_options()->game_speed;
}

float ScaleOrbitDist(bool autogenerated) {
    if (autogenerated) {
        return game_options()->autogen_compactness * game_options()->runtime_compactness;
    }
    return game_options()->runtime_compactness;
}

extern float ScaleJumpRadius(float radius);

void StarSystem::LoadXML(const string filename, const Vector &centroid, const float timeofyear) {
    using namespace StarXML;
    bool autogenerated = false;
    this->filename = filename;
    string file = VSFileSystem::GetCorrectStarSysPath(filename, autogenerated);
    if (file.empty()) {
        file = filename;
    }

    if (game_options()->game_speed_affects_autogen_systems) {
        autogenerated = false;
    }

    xml = new Star_XML;
    xml->scale = 1;
    xml->fade = autogenerated;
    xml->scale *= game_options()->star_system_scale;
    xml->parentterrain = NULL;
    xml->ct = NULL;
    xml->systemcentroid = centroid;
    xml->timeofyear = timeofyear;
    xml->starsp = GetStarSpreadScale();
    xml->numnearstars = GetNumNearStarsScale();
    xml->numstars = GetNumStarsScale();
    xml->backgroundname = string("cube");
    xml->backgroundColor = GFXColor(1.0, 1.0, 1.0, 1.0);
    xml->backgroundDegamma = false;
    xml->reflectivity = game_options()->reflectivity;
    xml->unitlevel = 0;

    VSFile other_file;
    string full_path = other_file.GetSystemDirectoryPath(file);
    SystemFactory sys = SystemFactory(file, full_path, xml);

    for (auto &unit : xml->moons) {
        if (unit->getUnitType() == Vega_UnitType::planet) {
            Unit *un = nullptr;
            // This code here is completely unclear to me and should be refactored
            // TODO: remove the whole PlanetIterator thing and all custom iterators
            for (Planet::PlanetIterator iter((Planet *) unit); (un = *iter); iter.advance()) {
                AddUnit(un);
            }
        } else {
            AddUnit(unit);
        }
    }

    createBackground(xml);
    delete xml;
}
