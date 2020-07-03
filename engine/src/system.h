#ifndef SYSTEM_H
#define SYSTEM_H

#include "gfx/vec.h"
#include "gfxlib_struct.h"
#include <string>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>

using std::string;
using std::vector;
using std::map;


class System
{
    struct Object {
        string type;
        map<string, string> attributes;
        vector<Object> objects;
    };

    Object root;

    struct Background {
        string name;
        int num_stars;
        int num_near_stars;
        GFXColor backgroundColor;
        bool   backgroundDegamma;
        float  star_spread;
    };

    struct Color {
        double red;
        double green;
        double blue;
        double alpha;
    };

    struct SystemStruct {
        string name;
        string background;
        Color background_color;
        bool background_degamma;
        float scale;
        float reflectivity;
    } system_struct;




    /*class Light
    {
        Color ambient;
        Color diffuse;
        Color specular;
    };

    class Asteroid
    {
        string name;
        string file;
        QVector r;
        QVector s;
        QVector location;
    };

    class Planet
    {
        string name;
        string file;
        string technique;
        string unit;
        string alpha;
        string destination;
        string faction;
        double year;
        double day;

        QVector r;
        QVector s;
        QVector location;

        double radius;
        Color color;
        Color diffuse;
        Color specular;
        bool reflectNoLight;
        double light;

    };

    class Unit
    {
        string name;
        string file;

        QVector r;
        QVector s;
        QVector location;

        double year;
        double day;
    };*/

public:
    // Fields
    string name;
    string background;
    int near_stars;
    int stars;
    int star_spread;
    int scale_system;

    // Constructor
    System(string const &system_file);

    void recursiveProcess(boost::property_tree::ptree tree, Object object);

    void processSystem(boost::property_tree::ptree tree);

};

#endif // SYSTEM_H
