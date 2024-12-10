#include <string>
#include <iostream>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>

#include "graphics_config.h"
#include "resource/json_utils.h"


// TODO: delete this. Graphics2Config should be generated automatically from config.json
Graphics2Config::Graphics2Config(const std::string config) {
    boost::json::value json_value = boost::json::parse(config);
    boost::json::object root = json_value.get_object();

    if (root.if_contains("graphics")) {
        boost::json::object graphics = root.at("graphics").get_object();
        ConditionalJsonGet(graphics, screen, "screen");
        ConditionalJsonGet(graphics, resolution_x, "resolution_x");
        ConditionalJsonGet(graphics, resolution_y, "resolution_y");
    }
}




