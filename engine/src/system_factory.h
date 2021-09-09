/**
* system_factory.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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

#ifndef SYSTEM_FACTORY_H
#define SYSTEM_FACTORY_H

#include "gfx/vec.h"
#include "gfxlib_struct.h"

#include <string>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>

using std::string;
using std::map;
using std::vector;


class Star_XML;
class Planet;



class SystemFactory
{
    struct Object {
        string type;
        map<string, string> attributes;
        vector<Object> objects;
    };

    Object root;

    struct Color {
        double red;
        double green;
        double blue;
        double alpha;
    };

    struct Light
    {
        GFXColor ambient = GFXColor( 0, 0, 0, 1 );
        GFXColor diffuse = GFXColor( 0, 0, 0, 1 );
        GFXColor specular = GFXColor( 0, 0, 0, 1 );
    };

    vector<Light> lights;

public:
    // Fields
    string name;
    string fullname;
    string background;
    int near_stars;
    int stars;
    int star_spread;
    int scale_system;

    // Constructor
    SystemFactory(string const &relative_filename, string& system_file, Star_XML *xml);

    void recursiveParse(boost::property_tree::ptree tree, Object& object);
    void recursiveProcess(Star_XML *xml, Object& object, Planet* owner, int level = 0);

    void processLight(Object& object) ;
    void processSystem(Star_XML *xml, Object& object);
    void processRing(Star_XML *xml, Object& object, Planet* owner);
    Planet* processPlanet(Star_XML *xml, Object& object, Planet* owner);
    void processSpaceElevator(Object& object, Planet* owner);
    void processFog(Star_XML *xml, Object& object, Planet* owner);
    void processEnhancement(string element, Star_XML *xml, Object& object, Planet* owner);
    void processAsteroid(Star_XML *xml, Object& object, Planet* owner);

    string getStringAttribute(Object object, string key, string default_value = "");
    bool getBoolAttribute(Object object, string key, bool default_value = true);
    char getCharAttribute(Object object, string key, char default_value);
    int getIntAttribute(Object object, string key, int default_value = 1,
                        int multiplier = 1, int default_multiplier = 1);
    float getFloatAttribute(Object object, string key, float default_value = 1.0f,
                            float multiplier = 1.0f, float default_multiplier = 1.0f);
    double getDoubleAttribute(Object object, string key, double default_value = 1.0,
                            double multiplier = 1.0, double default_multiplier = 1.0);

    void initializeQVector(Object object, string key_prefix, QVector& vector,
                           double multiplier = 1.0);
    void initializeMaterial(Object object, GFXMaterial& material);
    void initializeAlpha(Object object, BLENDFUNC blend_source,
                         BLENDFUNC blend_destination);
    GFXColor initializeColor(Object object);
};

#endif // SYSTEM_FACTORY_H
