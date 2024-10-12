#include <string>
#include <boost/filesystem.hpp>

#include "graphics_config.h"
#include "json.h"

// Exposes the struct to python
// TODO: move this somewhere as the same library should have multiple
// definitions
// Can we spread a boost python module over several files?
/*BOOST_PYTHON_MODULE(vegastrike_python) {
    boost::python::class_<Graphics2Config>("GraphicsConfig", boost::python::init<>())
    .def_readwrite("screen", &Graphics2Config::screen)
    .def_readwrite("resolution_x", &Graphics2Config::resolution_x)
    .def_readwrite("resolution_y", &Graphics2Config::resolution_y);
}*/

// a temporary helper function until we move to Boost/JSON
// TODO: remove
static const int GetValue(
    const std::string key,
    const int default_value,
    const json::jobject object) {
    if(!object.has_key(key)) {
        return default_value;
    }

    const std::string attribute = object.get(key);
    const int value = std::stoi(attribute);
    return value;
}

Graphics2Config::Graphics2Config(const std::string config) {
    json::jobject json_root = json::jobject::parse(config);

    if(!json_root.has_key("graphics")) {
        return;
    }

    const std::string graphics_json = json_root.get("graphics");
    json::jobject json_graphics = json::jobject::parse(config);

    screen = GetValue("screen", 0, json_graphics);
    resolution_x = GetValue("resolution_x", 2560, json_graphics);
    resolution_y = GetValue("resolution_y", 1600, json_graphics);
}




