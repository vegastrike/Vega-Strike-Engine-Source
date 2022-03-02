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

#ifndef STAR_XML_H
#define STAR_XML_H

#include "xml_support.h"

using namespace XMLSupport;

class Terrain;
class ContinuousTerrain;
class Planet;
struct AtmosphericFogMesh;

struct Star_XML {
    std::string name;
    Terrain *parentterrain;
    ContinuousTerrain *ct;
    int unitlevel;
    std::vector<GFXLight> lights;
    std::vector<Planet *> moons;
    std::string backgroundname;
    GFXColor backgroundColor;
    bool backgroundDegamma;
    Vector systemcentroid;
    Vector cursun;
    float timeofyear;
    float reflectivity;
    int numnearstars;
    int numstars;
    bool fade;
    float starsp;
    float scale;
    std::vector<AtmosphericFogMesh> fog;
    std::vector<bool> conditionStack;
    int fogopticalillusion = 0;
};

namespace StarXML {

enum Names {
    UNKNOWN,
    XFILE,
    X,
    Y,
    Z,
    RI,
    RJ,
    RK,
    SI,
    SJ,
    SK,
    QI,
    QJ,
    QK,
    NAME,
    DIRECTION,
    RADIUS,
    GRAVITY,
    YEAR,
    DAY,
    PPOSITION,
    SYSTEM,
    PLANET,
    UNIT,
    EMRED,
    EMGREEN,
    EMBLUE,
    EMALPHA,
    SRED,
    SGREEN,
    SBLUE,
    SALPHA,
    SPOWER,
    BACKGROUND,
    BACKGROUND_COLOR,
    BACKGROUND_DEGAMMA,
    STARS,
    STARSPREAD,
    NEARSTARS,
    FADESTARS,
    REFLECTIVITY,
    ALPHA,
    DESTINATION,
    JUMP,
    FACTION,
    LIGHT,
    COLL,
    ATTEN,
    DIFF,
    SPEC,
    AMB,
    TERRAIN,
    CONTTERRAIN,
    MASS,
    BUILDING,
    VEHICLE,
    ATMOSPHERE,
    NEBULA,
    NEBFILE,
    ASTEROID,
    SCALEX,
    NUMWRAPS,
    DIFFICULTY,
    REFLECTNOLIGHT,
    ENHANCEMENT,
    SCALEATMOS,
    SCALESYSTEM,
    CITYLIGHTS,
    INSIDEOUT,
    INNERRADIUS,
    OUTERRADIUS,
    NUMSLICES,
    RING,
    SPACEELEVATOR,
    WRAPX,
    WRAPY,
    FOG,
    FOGELEMENT,
    FOCUS,
    CONCAVITY,
    MINALPHA,
    MAXALPHA,
    DIRED,
    DIGREEN,
    DIBLUE,
    DIALPHA,
    TAILMODESTART,
    TAILMODEEND,
    OPTICALILLUSION,
    SERIAL,
    VARNAME,
    VARVALUE,
    CONDITION,
    EXPRESSION,
    TECHNIQUE,
    OVERRIDE
};

const EnumMap::Pair element_names[] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("Planet", PLANET),
        EnumMap::Pair("System", SYSTEM),
        EnumMap::Pair("Unit", UNIT),
        EnumMap::Pair("Enhancement", ENHANCEMENT),
        EnumMap::Pair("Jump", JUMP),
        EnumMap::Pair("Light", LIGHT),
        EnumMap::Pair("Attenuated", ATTEN),
        EnumMap::Pair("Diffuse", DIFF),
        EnumMap::Pair("Specular", SPEC),
        EnumMap::Pair("Ambient", AMB),
        EnumMap::Pair("Terrain", TERRAIN),
        EnumMap::Pair("ContinuousTerrain", CONTTERRAIN),
        EnumMap::Pair("Building", BUILDING),
        EnumMap::Pair("Vehicle", VEHICLE),
        EnumMap::Pair("Atmosphere", ATMOSPHERE),
        EnumMap::Pair("Nebula", NEBULA),
        EnumMap::Pair("Asteroid", ASTEROID),
        EnumMap::Pair("RING", RING),
        EnumMap::Pair("citylights", CITYLIGHTS),
        EnumMap::Pair("SpaceElevator", SPACEELEVATOR),
        EnumMap::Pair("Fog", FOG),
        EnumMap::Pair("FogElement", FOGELEMENT),
        EnumMap::Pair("Condition", CONDITION),
};
const EnumMap::Pair attribute_names[] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("background", BACKGROUND),
        EnumMap::Pair("backgroundColor", BACKGROUND_COLOR),
        EnumMap::Pair("backgroundDegamma", BACKGROUND_DEGAMMA),
        EnumMap::Pair("stars", STARS),
        EnumMap::Pair("nearstars", NEARSTARS),
        EnumMap::Pair("fadestars", FADESTARS),
        EnumMap::Pair("starspread", STARSPREAD),
        EnumMap::Pair("reflectivity", REFLECTIVITY),
        EnumMap::Pair("file", XFILE),
        EnumMap::Pair("alpha", ALPHA),
        EnumMap::Pair("destination", DESTINATION),
        EnumMap::Pair("direction", DIRECTION),
        EnumMap::Pair("x", X),
        EnumMap::Pair("y", Y),
        EnumMap::Pair("z", Z),
        EnumMap::Pair("ri", RI),
        EnumMap::Pair("rj", RJ),
        EnumMap::Pair("rk", RK),
        EnumMap::Pair("si", SI),
        EnumMap::Pair("sj", SJ),
        EnumMap::Pair("sk", SK),
        EnumMap::Pair("qi", QI),
        EnumMap::Pair("qj", QJ),
        EnumMap::Pair("qk", QK),
        EnumMap::Pair("name", NAME),
        EnumMap::Pair("radius", RADIUS),
        EnumMap::Pair("gravity", GRAVITY),
        EnumMap::Pair("year", YEAR),
        EnumMap::Pair("day", DAY),
        EnumMap::Pair("position", PPOSITION),
        EnumMap::Pair("MaxAlpha", MAXALPHA),
        EnumMap::Pair("MinAlpha", MINALPHA),
        EnumMap::Pair("DRed", DIRED),
        EnumMap::Pair("DGreen", DIGREEN),
        EnumMap::Pair("DBlue", DIBLUE),
        EnumMap::Pair("DAlpha", DIALPHA),
        EnumMap::Pair("Red", EMRED),
        EnumMap::Pair("Green", EMGREEN),
        EnumMap::Pair("Blue", EMBLUE),
        EnumMap::Pair("Alfa", EMALPHA),
        EnumMap::Pair("SRed", SRED),
        EnumMap::Pair("SGreen", SGREEN),
        EnumMap::Pair("SBlue", SBLUE),
        EnumMap::Pair("SAlfa", SALPHA),
        EnumMap::Pair("SPower", SPOWER),
        EnumMap::Pair("ReflectNoLight", REFLECTNOLIGHT),
        EnumMap::Pair("faction", FACTION),
        EnumMap::Pair("Light", LIGHT),
        EnumMap::Pair("Mass", MASS),
        EnumMap::Pair("ScaleX", SCALEX),
        EnumMap::Pair("NumWraps", NUMWRAPS),
        EnumMap::Pair("NumSlices", NUMSLICES),
        EnumMap::Pair("Difficulty", DIFFICULTY),
        EnumMap::Pair("ScaleAtmosphereHeight", SCALEATMOS),
        EnumMap::Pair("ScaleSystem", SCALESYSTEM),
        EnumMap::Pair("InsideOut", INSIDEOUT),
        EnumMap::Pair("InnerRadius", INNERRADIUS),
        EnumMap::Pair("OuterRadius", OUTERRADIUS),
        EnumMap::Pair("WrapX", WRAPX),
        EnumMap::Pair("WrapY", WRAPY),
        EnumMap::Pair("Focus", FOCUS),
        EnumMap::Pair("Concavity", CONCAVITY),
        EnumMap::Pair("TailModeStart", TAILMODESTART),
        EnumMap::Pair("TailModeEnd", TAILMODEEND),
        EnumMap::Pair("OpticalIllusion", OPTICALILLUSION),
        EnumMap::Pair("serial", SERIAL),
        EnumMap::Pair("VarName", VARNAME),
        EnumMap::Pair("VarValue", VARVALUE),
        EnumMap::Pair("Condition", CONDITION),
        EnumMap::Pair("expression", EXPRESSION),
        EnumMap::Pair("technique", TECHNIQUE),
        EnumMap::Pair("unit", UNIT),
        EnumMap::Pair("override", OVERRIDE)
};

//By Klauss - more flexible this way
const EnumMap element_map(element_names, sizeof(element_names) / sizeof(element_names[0]));
//By Klauss - more flexible this way
const EnumMap attribute_map(attribute_names, sizeof(attribute_names) / sizeof(attribute_names[0]));

}

#endif // STAR_XML_H
