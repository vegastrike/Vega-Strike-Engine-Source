/*
 * controls_factory.cpp
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

#include "controls_factory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <clocale>

#include <boost/json.hpp>

#include "cmd/drawable.h"
#include "root_generic/vs_globals.h"
#include "root_generic/configxml.h"
#include "resource/json_utils.h"

#include "gui/staticdisplay.h"
#include "gui/newbutton.h"
#include "gui/scroller.h"
#include "gui/control.h"
#include "gui/simplepicker.h"

const std::string keys[] = {"name", "type", "rect",	"text", "textColor",
                            "defaultTextColor", "color", "font", "id",
                            "label", "command", "justification","thumbColor",
                            "buttonColor", "outlineColor", "selectionColor",
                            "highlightColor", "highlightTextColor",
                            "textMargins", "textures", "multiline" };

std::map<std::string, std::map<std::string, std::string>> parseControlsJSON(VSFileSystem::VSFile &file) {
    const std::string json_text = file.ReadFull();

    std::map<std::string, std::map<std::string, std::string>> controls_map;

    boost::json::value json_value = boost::json::parse(json_text);
    boost::json::array root_array = json_value.get_array();

    for(boost::json::value& control_value : root_array) {
        boost::json::object control = control_value.get_object();
        std::map<std::string, std::string> control_attributes;

        for (const std::string &key : keys) {
            if(!control.if_contains(key)) {
                continue;
            }

            // Worked in singleson because it saw everything as string.
            if(key == "multiline") {
                bool multiline = JsonGetWithDefault(control, key, false);
                if(multiline) {
                    control_attributes["multiline"] = "true";
                } else {
                    control_attributes["multiline"] = "false";
                }
            } else {
                const std::string text = boost::json::value_to<std::string>(control.at(key));
                control_attributes[key] = text;
            }
        }

        controls_map[control_attributes["name"]] = control_attributes;
    }

    return controls_map;
}



static std::vector<double> splitAndConvert (const std::string &s, char delim) {
    std::vector<double> result;
    std::stringstream ss (s);
    std::string item;
    
    // pmx-20251103 str::stod() fails on the decimal point on french locale (decimal separator is ',')
    const char* loc = std::setlocale(LC_NUMERIC, "en_US.UTF-8");
    
    while (std::getline (ss, item, delim)) {
        double d=std::stod(item);
        result.push_back (d);
    }
    
    // Restore locale
    std::setlocale(LC_NUMERIC, loc);
    return result;
}

GFXColor getColor(const std::string& colorString) {
    std::vector<double> colorTuple = splitAndConvert(colorString, ',');
    if(colorTuple.size() == 4) {
        return GFXColor(colorTuple.at(0), colorTuple.at(1), colorTuple.at(2), colorTuple.at(3));
    } else {
        return GFXColor(colorTuple.at(0), colorTuple.at(1), colorTuple.at(2));
    }

}

Control* getControl(std::map<std::string, std::string> attributes) {
    const std::string type = attributes["type"];

    Control* c;

    if(type == "staticDisplay") {
        StaticDisplay* sd = new StaticDisplay;
        c = sd;

        // Text
        if(attributes.count("text")) {
            sd->setText(attributes["text"]);
        }

        // Justification
        if(attributes.count("justification")) {
            if(attributes["justification"] == "Left") {
                sd->setJustification(CENTER_JUSTIFY);
            } else if(attributes["justification"] == "Right") {
                sd->setJustification(CENTER_JUSTIFY);
            } else if(attributes["justification"] == "Center") {
                sd->setJustification(CENTER_JUSTIFY);
            }
        }

        // Text Margin
        if(attributes.count("textMargins")) {
            std::vector<double> size = splitAndConvert(attributes["textMargins"], ',');
            sd->setTextMargins(Size(size.at(0), size.at(1)));
        }

        if(attributes.count("multiline")) {
            sd->setMultiLine(true);
        }
    } else if(type == "button") {
        NewButton* b = new NewButton;
        c = b;

        // Label
        if(attributes.count("label")) {
            b->setLabel(attributes["label"]);
        }

        // Command
        if(attributes.count("command")) {
            b->setCommand(attributes["command"]);
        }

        // Down Text Color
        if(attributes.count("downTextColor")) {
            std::string colorString = attributes["downTextColor"];
            GFXColor color = getColor(colorString);
            b->setDownTextColor(color);
        }

        // Down Color
        if(attributes.count("downColor")) {
            std::string colorString = attributes["downColor"];
            GFXColor color = getColor(colorString);
            b->setDownColor(color);
        }

        // Highlight Color
        if(attributes.count("highlightColor")) {
            std::string colorString = attributes["highlightColor"];
            GFXColor color = getColor(colorString);
            b->setHighlightColor(color);
        }

        // Border Color
        if(attributes.count("borderColor")) {
            std::string colorString = attributes["borderColor"];
            GFXColor color = getColor(colorString);
            b->setBorderColor(color);
        }

        // End Border Color
        if(attributes.count("endBorderColor")) {
            std::string colorString = attributes["endBorderColor"];
            GFXColor color = getColor(colorString);
            b->setEndBorderColor(color);
        }

        // Variable Border Cycle Time
        if(attributes.count("cycleTime")) {
            std::string cycleTimeString = attributes["cycleTime"];
            b->setVariableBorderCycleTime(std::stod(cycleTimeString));
        }

        // Shadow Width
        if(attributes.count("shadowWidth")) {
            std::string shadowWidth = attributes["shadowWidth"];
            b->setShadowWidth(std::stod(shadowWidth));
        }
    } else if(type == "scroller") {
        Scroller* s = new Scroller;
        c = s;

        // Button Color
        if(attributes.count("buttonColor")) {
            std::string colorString = attributes["buttonColor"];
            GFXColor color = getColor(colorString);
            s->setButtonColor(color);
        }

        // Outline Color
        if(attributes.count("outlineColor")) {
            std::string colorString = attributes["outlineColor"];
            GFXColor color = getColor(colorString);
            s->setOutlineColor(color);
        }
    } else if(type == "picker") {
        SimplePicker* p = new SimplePicker;
        c = p;

        // Outline Color
        if(attributes.count("outlineColor")) {
            std::string colorString = attributes["outlineColor"];
            GFXColor color = getColor(colorString);
            p->setOutlineColor(color);
        }

        // Selection Color
        if(attributes.count("selectionColor")) {
            std::string colorString = attributes["selectionColor"];
            GFXColor color = getColor(colorString);
            p->setOutlineColor(color);
        }

        // Highlight Color
        if(attributes.count("highlightColor")) {
            std::string colorString = attributes["highlightColor"];
            GFXColor color = getColor(colorString);
            p->setHighlightColor(color);
        }

        // Highlight Text Color
        if(attributes.count("highlightTextColor")) {
            std::string colorString = attributes["highlightTextColor"];
            GFXColor color = getColor(colorString);
            p->setHighlightTextColor(color);
        }

        // Text Margin
        if(attributes.count("textMargins")) {
            std::vector<double> size = splitAndConvert(attributes["textMargins"], ',');
            p->setTextMargins(Size(size.at(0), size.at(1)));
        }
    } else if(type == "staticImageDisplay") {
        StaticImageDisplay* sid = new StaticImageDisplay;
        c = sid;

        if(attributes.count("texture")) {
            sid->setTexture(attributes["texture"]);
        }
    }

    // Font
    if(attributes.count("font")) {
        std::vector<double> font_array = splitAndConvert(attributes["font"], ',');
        if (font_array.size() < 2) {
            VS_LOG(error, "controls_factory getControl(): font_array doesn't have enough elements");
        } else {
            Font font(font_array.at(0), font_array.at(1));
            c->setFont(font);
        }
    }

    // Rect
    if(attributes.count("rect")) {
        std::vector<double> rect = splitAndConvert(attributes["rect"], ',');
        c->setRect(Rect(rect.at(0), rect.at(1), rect.at(2), rect.at(3)));
    }

    // Color
    if(attributes.count("color")) {
        std::string colorString = attributes["color"];
        GFXColor color = getColor(colorString);
        c->setColor(color);
    }

    // Text Color
    if(attributes.count("textColor")) {
        std::string colorString = attributes["textColor"];
        GFXColor color = getColor(colorString);
        c->setTextColor(color);
    }

    // ID
    if(attributes.count("id")) {
        c->setId(attributes["id"]);
    }

    return c;
}
