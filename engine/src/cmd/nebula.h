/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Roy Falk,
 * David Wales, Stephen G. Tuggy, and other Vega Strike contributors.
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
#ifndef VEGA_STRIKE_ENGINE_CMD_NEBULA_H
#define VEGA_STRIKE_ENGINE_CMD_NEBULA_H

#include "unit_generic.h"

class Nebula : public Unit {
public:

    // Constructors
    Nebula(const char *unitfile, bool SubU, int faction, Flightgroup *fg = NULL, int fg_snumber = 0);

    enum Vega_UnitType isUnit() const {
        return Vega_UnitType::nebula;
    }

protected:
    // Method
    static void beginElement(void *Userdata, const XML_Char *name, const XML_Char **atts);
    Vector color;
    float Density;
    float fognear;
    float fogfar;
    int index;
    float explosiontime;
    enum FOGMODE fogmode;     //0=OFF (I won't use this), 1=EXP, 2=EXP2, 3=LINEAR
    bool fogme;
    void LoadXML(const char *filename);
    void beginElem(const std::string &, const AttributeList &);
    float lastfadein;
    float fadeinvalue;
public:

    float getFade() {
        return fadeinvalue;
    }

    void PutInsideCam(int);

public:
    void InitNebula(const char *unitfile, bool SubU, int faction, Flightgroup *fg, int fg_snumber);

public:
    virtual void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL);

    void SetFogState();

protected:
    /// default constructor forbidden
    Nebula();
    /// copy constructor forbidden
    Nebula(const Nebula &);
    /// assignment operator forbidden
    Nebula &operator=(const Nebula &);
};

namespace NebulaXML {
FOGMODE parse_fogmode(string val);

enum Names {
    UNKNOWN,
    NEBULA,
    RED,
    GREEN,
    BLUE,
    COLOR,
    MODE,
    NEBNEAR,
    NEBFAR,
    DENSITY,
    LIMITS,
    INDEX,
    EXPLOSIONTIME,
    FOGTHIS
};
const unsigned short int MAXENAMES = 4;
const unsigned short int MAXANAMES = 11;

const XMLSupport::EnumMap::Pair element_names[MAXENAMES] = {
        XMLSupport::EnumMap::Pair("UNKNOWN", UNKNOWN),
        XMLSupport::EnumMap::Pair("Nebula", NEBULA),
        XMLSupport::EnumMap::Pair("Color", COLOR),
        XMLSupport::EnumMap::Pair("Limits", LIMITS),
};
const XMLSupport::EnumMap::Pair attribute_names[MAXANAMES] = {
        XMLSupport::EnumMap::Pair("UNKNOWN", UNKNOWN),
        XMLSupport::EnumMap::Pair("Red", RED),
        XMLSupport::EnumMap::Pair("Green", GREEN),
        XMLSupport::EnumMap::Pair("Blue", BLUE),
        XMLSupport::EnumMap::Pair("Near", NEBNEAR),
        XMLSupport::EnumMap::Pair("Far", NEBFAR),
        XMLSupport::EnumMap::Pair("Density", DENSITY),
        XMLSupport::EnumMap::Pair("Mode", MODE),
        XMLSupport::EnumMap::Pair("Index", INDEX),
        XMLSupport::EnumMap::Pair("ExplosionTime", EXPLOSIONTIME),
        XMLSupport::EnumMap::Pair("FogThis", FOGTHIS)
};

const XMLSupport::EnumMap element_map(element_names, MAXENAMES);
const XMLSupport::EnumMap attribute_map(attribute_names, MAXANAMES);
}

#endif //VEGA_STRIKE_ENGINE_CMD_NEBULA_H
