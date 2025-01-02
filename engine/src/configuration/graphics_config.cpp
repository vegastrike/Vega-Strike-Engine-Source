#include <string>
#include <iostream>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>

#include "graphics_config.h"
#include "resource/json_utils.h"


// TODO: delete this. Graphics2Config should be generated automatically from config.json
Graphics2Config::Graphics2Config(boost::json::object object) {
    if (object.if_contains("graphics")) {
        boost::json::object graphics = object.at("graphics").get_object();
        ConditionalJsonGet(graphics, screen, "screen");
        ConditionalJsonGet(graphics, resolution_x, "resolution_x");
        ConditionalJsonGet(graphics, resolution_y, "resolution_y");
    }
}




