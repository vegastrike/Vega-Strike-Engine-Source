/**
 * controls_factory.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2023 Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer and other Vega Strike
 * contributors
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

#include "json.h"
#include "drawable.h"
#include "vs_globals.h"
#include "configxml.h"

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

    std::vector<std::string> controls_json = json::parsing::parse_array(json_text.c_str());
     
    // Iterate over root
    for (const std::string &control_text : controls_json) {
        json::jobject control_json = json::jobject::parse(control_text);
        std::map<std::string, std::string> control_attributes;

        for (const std::string &key : keys) {
            // For some reason, parser adds quotes
            if(control_json.has_key(key)) {
                const std::string attribute = control_json.get(key);
                const std::string stripped_attribute = attribute.substr(1, attribute.size() - 2);
                control_attributes[key] = stripped_attribute;
            } 
        }

        controls_map[control_attributes["name"]] = control_attributes;
    }

    return controls_map;
}

static std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

static std::vector<double> splitAndConvert (const std::string &s, char delim) {
    std::vector<double> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (std::stod(item));
    }

    return result;
}

GFXColor getColor(const std::string& colorString) {
    std::vector<double> colorTuple = splitAndConvert(colorString, ',');
    if(colorTuple.size() == 4) {
        return GFXColor(colorTuple[0], colorTuple[1], colorTuple[2], colorTuple[3]);
    } else {
        return GFXColor(colorTuple[0], colorTuple[1], colorTuple[2]);
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
            sd->setTextMargins(Size(size[0], size[1]));
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
            p->setTextMargins(Size(size[0], size[1]));
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
        Font font(font_array[0], font_array[1]);
        c->setFont(font);
    }

    // Rect
    if(attributes.count("rect")) {
        std::vector<double> rect = splitAndConvert(attributes["rect"], ',');
        c->setRect(Rect(rect[0], rect[1], rect[2], rect[3]));
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
