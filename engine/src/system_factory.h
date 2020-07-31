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
