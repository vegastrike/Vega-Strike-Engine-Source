/*
 * multi_color_text.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "imgui.h"

static std::map<std::string, ImU32> color_map;
static const ImU32 default_color = IM_COL32(255, 255, 255, 255);

ImU32 GetColor(const std::string& color_string) {
    if(!color_map.count(color_string)) {
        unsigned int color_int;
        try {
            color_int = std::stoul(color_string, nullptr, 16);
        } catch (const std::invalid_argument& e) {
            return default_color;
        }

        unsigned char red = static_cast<unsigned char>(color_int & 0xFF);
        unsigned char green = static_cast<unsigned char>((color_int >> 8) & 0xFF);
        unsigned char blue = static_cast<unsigned char>((color_int >> 16) & 0xFF);
        unsigned char alpha = 255;

        ImU32 color = IM_COL32(red, green, blue, alpha);
        color_map[color_string] = color;
    }

    return color_map[color_string];
}

// Note: this function does not support printing the character #
// as it is used to denote a color.
// Error checking, as in the original code is limited.
void DrawText(const std::string& text, int x, int y) {
    if(x<0 || x>4000) assert(0);
    if(y<0 || y>2200) assert(0);
    //std::cout << (boost::format("%1% x=%2% y=%3%\n") % text % x % y).str();
    const int original_x = x;
    ImU32 color = default_color;
    std::vector<std::string> text_sections;
    boost::split(text_sections, text, boost::is_any_of("#"));
    bool first = true; // To deal with the edge case where there are no color notations
    for(std::string& text_section : text_sections) {
        if(text_section.empty()) {
            first = false;
            continue;
        }

        if(text_section.size() > 6 && !first) {
            std::string color_string = text_section.substr(0,6);
            text_section = text_section.substr(6);

            color = GetColor(color_string);
        }

        first = false;

        std::vector<std::string> lines;
        boost::split(lines, text_section, boost::is_any_of("\n"));
        for(std::string& line : lines) {
            ImVec2 line_size = ImGui::CalcTextSize(line.c_str(), nullptr, true);
            std::cout << "y=" << line_size.y << std::endl;
            ImGui::GetForegroundDrawList()->AddText(ImVec2(x,y),color,line.c_str());

            if(lines.size() > 1) {
                x = original_x;
                y += line_size.y;
            } else {
                x += line_size.x;
            }
        }
    }
}