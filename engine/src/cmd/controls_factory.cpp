/*
 * controls_factory.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include <cstdio>
#include <sstream>
#include <string>
#include <map>


#include <clocale>
#include <boost/json.hpp>

#include "src/vega_cast_utils.h"
#include "cmd/drawable.h"

#include "gui/staticdisplay.h"
#include "gui/newbutton.h"
#include "gui/scroller.h"
#include "gui/control.h"
#include "gui/simplepicker.h"
#include "gui/textinputdisplay.h"

// All supported UI control property keys
enum class ControlProp {
    Unknown,
    // Text / Content
    Id, Name, Type, Parent, Scroller, Text, Label, Command, Texture,
    // Layout / Geometry
    Rect, Justification, TextMargins, Multiline, Font,
    // Colors
    Color, TextColor, DownTextColor, DownColor, HighlightColor,
    HighlightTextColor, BorderColor, EndBorderColor, ButtonColor,
    OutlineColor, SelectionColor,
    // Numeric Parameters
    CycleTime, ShadowWidth
};

// Fast string-to-enum mapper
static ControlProp parseProp(const std::string& key) {
    static const std::unordered_map<std::string, ControlProp> propMap = {
        {"name",               ControlProp::Name},
        {"type",               ControlProp::Type},
        {"parent",             ControlProp::Parent},
        {"scroller",           ControlProp::Scroller},
        {"text",               ControlProp::Text},
        {"label",              ControlProp::Label},
        {"command",            ControlProp::Command},
        {"texture",            ControlProp::Texture},
        {"rect",               ControlProp::Rect},
        {"justification",      ControlProp::Justification},
        {"textMargins",        ControlProp::TextMargins},
        {"multiline",          ControlProp::Multiline},
        {"font",               ControlProp::Font},
        {"color",              ControlProp::Color},
        {"textColor",          ControlProp::TextColor},
        {"downTextColor",      ControlProp::DownTextColor},
        {"downColor",          ControlProp::DownColor},
        {"highlightColor",     ControlProp::HighlightColor},
        {"highlightTextColor", ControlProp::HighlightTextColor},
        {"borderColor",        ControlProp::BorderColor},
        {"endBorderColor",     ControlProp::EndBorderColor},
        {"buttonColor",        ControlProp::ButtonColor},
        {"outlineColor",       ControlProp::OutlineColor},
        {"selectionColor",     ControlProp::SelectionColor},
        {"id",                 ControlProp::Id},
        {"cycleTime",          ControlProp::CycleTime},
        {"shadowWidth",        ControlProp::ShadowWidth}
    };

    auto it = propMap.find(key);
    return (it != propMap.end()) ? it->second : ControlProp::Unknown;
}

std::map<std::string, std::map<std::string, std::string>> parseControlsJSON(VSFileSystem::VSFile &file) {
    const std::string json_text = file.ReadFull();

    std::map<std::string, std::map<std::string, std::string>> controls_map;

    boost::system::error_code ec;
    boost::json::value json_value = boost::json::parse(json_text, ec);
    if (ec) {
        VS_LOG(error, (boost::format("parseControlsJSON: Error parsing JSON - %1%") % ec.message()));
        return controls_map;
    }

    if (!json_value.is_array()) {
        VS_LOG(error, "parseControlsJSON: Root JSON element is not an array");
        return controls_map;
    }

    const boost::json::array& root_array = json_value.get_array();

    for (const boost::json::value& control_value : root_array) {
        if (!control_value.is_object()) {
            continue;
        }

        const boost::json::object& control = control_value.get_object();
        std::map<std::string, std::string> control_attributes;

        // Iterate dynamically over all keys in the object withou filtering!
        for (boost::json::object::const_iterator it = control.begin(); it != control.end(); ++it) {
            std::string key(it->key());
            const boost::json::value& val = it->value();

            if (val.is_bool()) {
                control_attributes[key] = val.get_bool() ? "true" : "false";
            } else if (val.is_string()) {
                control_attributes[key] = boost::json::value_to<std::string>(val);
            } else if (val.is_int64()) {
                control_attributes[key] = std::to_string(val.get_int64());
            } else if (val.is_double()) {
                control_attributes[key] = std::to_string(val.get_double());
            } else if (val.is_array()) {
                // If rect or color are JSON arrays like [-0.1, 0.2, 0.5], format them back to strings
                std::string array_str;
                for (size_t i = 0; i < val.get_array().size(); ++i) {
                    if (i > 0) {
                        array_str += ", ";
                    }
                    const auto& elem = val.get_array()[i];
                    if (elem.is_double()) {
                        array_str += std::to_string(elem.get_double());
                    } else if (elem.is_int64()) {
                        array_str += std::to_string(elem.get_int64());
                    } else if (elem.is_string()) {
                        array_str += elem.get_string().c_str();
                    }
                }
                control_attributes[key] = array_str;
            }
        }

        // Only register if a valid 'name' attribute exists
        auto itName = control_attributes.find("name");
        if (itName != control_attributes.end() && !itName->second.empty()) {
            controls_map[itName->second] = control_attributes;
        } else {
            VS_LOG(error, "parseControlsJSON: Control entry missing 'name' key, skipping");
        }
    }

    return controls_map;
}

// TODO: CLion informs me that delim is always 44 (','). As such, we could probably eliminate this parameter at some point.
static std::vector<double> splitAndConvert (const std::string &s, char delim) {
    std::vector<double> result;
    std::istringstream overall_ss (s);
    overall_ss.imbue(our_numeric_locale);
    std::string item;

    while (std::getline(overall_ss, item, delim)) {
        result.push_back(locale_aware_stod(item));
    }

    return result;
}

GFXColor getColor(const std::string& colorString) {
    std::vector<double> colorTuple = splitAndConvert(colorString, ',');
    if (colorTuple.size() == 4) {
        return GFXColor(colorTuple[0], colorTuple[1], colorTuple[2], colorTuple[3]);
    } else if (colorTuple.size() == 3) {
        return GFXColor(colorTuple[0], colorTuple[1], colorTuple[2]);
    }
    
    VS_LOG(error, "getColor(): Invalid color string: " + colorString);
    return GFXColor(); // Return default fallback color
}

Control* getControl(const std::map<std::string, std::string>& attributes, std::vector<unsigned int>* base_keyboard_queue) {
    auto itType = attributes.find("type");
    if (itType == attributes.end()) {
        VS_LOG(error, "getControl(): Missing 'type' attribute");
        return nullptr;
    }

    const std::string& type = itType->second;
    Control* c = nullptr;

    // --- Type-Specific Construction & Configuration ---
    if (type == "staticDisplay") {
        auto* sd = new StaticDisplay;
        c = sd;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::Text:
                    sd->setText(value);
                    break;
                case ControlProp::Justification:
                    if (value == "Left" || value == "0") {
                        sd->setJustification(LEFT_JUSTIFY);
                    } else if (value == "Right" || value == "1") {
                        sd->setJustification(RIGHT_JUSTIFY);
                    } else if (value == "Center" || value == "2") {
                        sd->setJustification(CENTER_JUSTIFY);
                    }
                    break;
                case ControlProp::TextMargins: {
                    auto size = splitAndConvert(value, ',');
                    if (size.size() >= 2) {
                        sd->setTextMargins(Size(size[0], size[1]));
                    }
                    break;
                }
                case ControlProp::Multiline:
                    sd->setMultiLine(value == "true");
                    break;
                case ControlProp::OutlineColor:
                    sd->setOutlineColor(getColor(value));
                    break;
                default:
                    break;
            }
        }
    } else if (type == "button") {
        auto* b = new NewButton;
        c = b;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::Label:          b->setLabel(value); break;
                case ControlProp::Command:        b->setCommand(value); break;
                case ControlProp::DownTextColor:  b->setDownTextColor(getColor(value)); break;
                case ControlProp::DownColor:      b->setDownColor(getColor(value)); break;
                case ControlProp::HighlightColor: b->setHighlightColor(getColor(value)); break;
                case ControlProp::BorderColor:    b->setBorderColor(getColor(value)); break;
                case ControlProp::EndBorderColor: b->setEndBorderColor(getColor(value)); break;
                case ControlProp::CycleTime:      b->setVariableBorderCycleTime(locale_aware_stof(value)); break;
                case ControlProp::ShadowWidth:    b->setShadowWidth(locale_aware_stof(value)); break;
                default:
                    break;
            }
        }
    } else if (type == "scroller") {
        auto* s = new Scroller;
        c = s;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::ButtonColor:  s->setButtonColor(getColor(value)); break;
                case ControlProp::OutlineColor: s->setOutlineColor(getColor(value)); break;
                default:
                    break;
            }
        }
    } else if (type == "picker") {
        auto* p = new SimplePicker;
        c = p;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::OutlineColor:       p->setOutlineColor(getColor(value)); break;
                case ControlProp::SelectionColor:     p->setSelectionColor(getColor(value)); break;
                case ControlProp::HighlightColor:     p->setHighlightColor(getColor(value)); break;
                case ControlProp::HighlightTextColor: p->setHighlightTextColor(getColor(value)); break;
                case ControlProp::TextMargins: {
                    auto size = splitAndConvert(value, ',');
                    if (size.size() >= 2) {
                        p->setTextMargins(Size(size[0], size[1]));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    } else if (type == "staticImageDisplay") {
        auto* sid = new StaticImageDisplay;
        c = sid;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::Texture: sid->setTexture(value); break;
                default:                   break;
            }
        }
    } else if (type == "textInputDisplay") {
        // TextInputDisplay requires keyboard queue and filter string
        std::string filter = "\x1b\r*?\\/|:<>\"^"; // Default filter mask
        
        auto itFilter = attributes.find("invalidChars");
        if (itFilter != attributes.end()) {
            filter = itFilter->second;
        }

        auto* tid = new TextInputDisplay(base_keyboard_queue, filter.c_str());
        c = tid;

        for (const auto& pair : attributes) {
            const std::string& key = pair.first;
            const std::string& value = pair.second;
            switch (parseProp(key)) {
                case ControlProp::Text:
                    tid->setText(value);
                    break;
                case ControlProp::Justification:
                    if (value == "Left" || value == "0") {
                        tid->setJustification(LEFT_JUSTIFY);
                    } else if (value == "Right" || value == "1") {
                        tid->setJustification(RIGHT_JUSTIFY);
                    } else if (value == "Center" || value == "2") {
                        tid->setJustification(CENTER_JUSTIFY);
                    }
                    break;
                case ControlProp::TextMargins: {
                    auto size = splitAndConvert(value, ',');
                    if (size.size() >= 2) {
                        tid->setTextMargins(Size(size[0], size[1]));
                    }
                    break;
                }
                case ControlProp::Multiline:
                    tid->setMultiLine(value == "true");
                    break;
                case ControlProp::OutlineColor:
                    tid->setOutlineColor(getColor(value));
                    break;
                default:
                    break;
            }
        }
    } else if (type == "groupControl") {
        // GroupControls act primarily as structural containers
        auto* gc = new GroupControl;
        c = gc;
        // Add any GroupControl-specific property parsing here if needed in the future
    } else {
        VS_LOG(error, (boost::format("%1%: Unrecognized control type '%2%'") % __FUNCTION__ % type));
        return nullptr;
    }

    // --- Common Base Properties ---
    for (const auto& pair : attributes) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;
        switch (parseProp(key)) {
            case ControlProp::Id:
                c->setId(value);
                break;
            case ControlProp::Color:
                c->setColor(getColor(value));
                break;
            case ControlProp::TextColor:
                c->setTextColor(getColor(value));
                break;
            case ControlProp::Rect: {
                auto rect = splitAndConvert(value, ',');
                if (rect.size() >= 4) {
                    c->setRect(Rect(rect[0], rect[1], rect[2], rect[3]));
                } else {
                    VS_LOG(error, (boost::format("getControl(): 'rect' requires 4 float values, got %1%") % rect.size()));
                }
                break;
            }
            case ControlProp::Font: {
                auto font_array = splitAndConvert(value, ',');
                if (font_array.size() == 1) {
                    c->setFont(Font(font_array[0]));
                } else if (font_array.size() >= 2) {
                    c->setFont(Font(font_array[0], font_array[1]));
                } else {
                    VS_LOG(error, "getControl(): 'font' requires at least 1 value");
                }
                break;
            }
            default:
                break;
        }
    }

    return c;
}

bool getControls(
        const std::string& filename, // the file name of the JSON data 
        Window* window, // the window the group controls will be added to
        std::vector<unsigned int>* base_keyboard_queue
    ) {

    // Load file via engine API
    VSFileSystem::VSFile jsonFile;
    VSFileSystem::VSError err = jsonFile.OpenReadOnly(filename);
    if (err > VSFileSystem::Ok) {
        VS_LOG(error, (boost::format("%1%: '%2%' not found") % __FUNCTION__ % filename));
        return false;
    }

    // Parse JSON file into nested maps: map<ControlName, map<PropertyKey, PropertyValue>>
    std::map<std::string, std::map<std::string, std::string>> parsedControls = parseControlsJSON(jsonFile);
    if (parsedControls.empty()) {
        VS_LOG(error, (boost::format("%1%: Failed to parse or empty file '%2%'") % __FUNCTION__ % filename));
        return false;
    }

    std::map<std::string, Control*> controlMap;
    std::vector<std::pair<Control*, std::string>> parentLinks;
    std::vector<std::pair<Control*, std::string>> scrollerLinks;

    // --- Pass 1: Instantiate all controls ---
    for (const auto& pair : parsedControls) {
        const std::string& controlName = pair.first;
        const auto& attributes = pair.second;
        Control* c = getControl(attributes, base_keyboard_queue);
        if (!c) {
            continue;
        }

        controlMap[controlName] = c;

        auto itParent = attributes.find("parent");
        // Cache relationships requiring resolution in Pass 2
        if (itParent != attributes.end()) {
            VS_LOG(debug, (boost::format("Control '%1%' found parent '%2%'") % controlName % itParent->second));
            // Child control: Pass 2 will add it to its GroupControl
            parentLinks.emplace_back(c, itParent->second);
        } else {
            // controls without parent are top-level and get added to the window
           window->addControl(c);
        }

        auto itScroller = attributes.find("scroller");
        if (itScroller != attributes.end()) {
            scrollerLinks.emplace_back(c, itScroller->second);
        }
    }

    // --- Pass 2: Wire Hierarchy & Bindings ---
    for (const auto& pair : parentLinks) {
        Control* child = pair.first;
        const std::string& parentName = pair.second;
        auto it = controlMap.find(parentName);
        if (it != controlMap.end()) {
            if (auto* group = dynamic_cast<GroupControl*>(it->second)) {
                group->addChild(child);
            }
        } else {
            VS_LOG(error, (boost::format("getControls(): Parent '%1%' not found for control link") % parentName));
        }
    }

    // --- Pass 3: Add the scrollers ---
    for (const auto& pair : scrollerLinks) {
        Control* targetControl = pair.first;
        const std::string& scrollerName = pair.second;
        auto it = controlMap.find(scrollerName);
        if (it != controlMap.end()) {
            if (auto* scroller = vega_dynamic_cast_ptr<Scroller>(it->second, true)) {
                if (auto* picker = vega_dynamic_cast_ptr<SimplePicker>(targetControl, true)) {
                    picker->setScroller(scroller);
                } else if (auto* sd = dynamic_cast<StaticDisplay*>(targetControl)) {
                    sd->setScroller(scroller);
                }
            }
        } else {
            VS_LOG(error, (boost::format("getControls(): Scroller '%1%' not found for control binding") % scrollerName));
        }
    }

    return true;
}
