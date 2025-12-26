/*
 * hud.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically:
 * pyramid3d, ace123, dan_w, jacks, klaussfreire
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

#include <vector>
#include <string>
#include <utility>

#include <ctype.h>
#include "src/gfxlib.h"
#include "cmd/unit_generic.h"
#include "gfx/hud.h"
#include "src/file_main.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "cmd/base.h"
//#include "glut.h"
#include "src/universe.h"
#include "gldrv/mouse_cursor.h"

#include "gldrv/gl_globals.h"

#include "libraries/gui/gui.h"
#include "imgui.h"


static bool isInside() {
    if (BaseInterface::CurrentBase) {
        return true;
    }
    return false;
}



const std::string &getStringFontForHeight(bool &changed) {
    const bool inside = isInside();
    static bool lastinside = inside;
    if (lastinside != inside) {
        changed = true;
        lastinside = inside;
    } else {
        changed = false;
    }
    return inside ? configuration().graphics.bases.font : configuration().graphics.font;
}



float getFontHeight() {
    bool changed = false;
    std::string whichfont = getStringFontForHeight(changed);
    static float point = 0;
    if (changed) {
        point = 0;
    }
    if (point == 0) {
        if (whichfont == "helvetica10") {
            point = 22;
        } else if (whichfont == "helvetica18") {
            point = 40;
        } else if (whichfont == "times24") {
            point = 50;
        } else if (whichfont == "times10") {
            point = 22;
        } else if (whichfont == "fixed13") {
            point = 30;
        } else if (whichfont == "fixed15") {
            point = 34;
        } else {
            point = 26;
        }
    }
    return point / configuration().graphics.resolution_y;
}

TextPlane::TextPlane(const GFXColor &c, const GFXColor &bgcol) {
    color = static_cast<ImU32>(c);
    background_color = static_cast<ImU32>(bgcol);
    myDims.i = 2;
    myDims.j = -2;
    myFontMetrics.Set(.06, .08, 0);
    SetPos(0, 0);
}

TextPlane::~TextPlane() = default;

int TextPlane::Draw(int offset) {
    return Draw(myText, offset, true, false, true);
}


static unsigned char HexToChar(char a) {
    if (a >= '0' && a <= '9') {
        return a - '0';
    } else if (a >= 'a' && a <= 'f') {
        return 10 + a - 'a';
    } else if (a >= 'A' && a <= 'F') {
        return 10 + a - 'A';
    }
    return 0;
}

static unsigned char TwoCharToByte(char a, char b) {
    return 16 * HexToChar(a) + HexToChar(b);
}

static float TwoCharToFloat(char a, char b) {
    return TwoCharToByte(a, b) / 255.;
}


// Text in a specific color
struct TextSegment {
    std::string text;
    ImU32 color;
};

struct TextLine {
    float width;
    std::vector<TextSegment> segments;
};

static ImU32 MakeColorU32(float r, float g, float b, float a) {
    return IM_COL32(
        int(r * 255.0f),
        int(g * 255.0f),
        int(b * 255.0f),
        int(a * 255.0f));
}

static void FlushBuffer(std::vector<TextSegment>& segments, std::string& buffer, ImU32 currentColor) {
    if (buffer.empty()) {
        return;
    }

    TextSegment segment{buffer, currentColor};
    segments.emplace_back(segment);
    buffer.clear();
}


static std::vector<TextLine> ParseText(const std::string& text, ImU32 default_color) {
    std::vector<TextLine> lines;
    std::vector<TextSegment> segments;

    ImU32 current_color = default_color;
    std::string buffer;
    std::string current_line;

    // Can't use for-each because we need i to move forward and back
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];

        // '_' is rendered as space
        if (c == '_')
            c = ' ';

        // Color tag: #RRGGBB
        if (c == '#' && i + 6 < text.size()) {
            FlushBuffer(segments, buffer, current_color);

            float r = TwoCharToFloat(text[i + 1], text[i + 2]);
            float g = TwoCharToFloat(text[i + 3], text[i + 4]);
            float b = TwoCharToFloat(text[i + 5], text[i + 6]);

            if (r == 0 && g == 0 && b == 0)
                current_color = default_color;
            else
                current_color = MakeColorU32(r, g, b, 1.0);

            i += 6;
            continue;
        }

        // Newline
        if(c=='\n') {
            FlushBuffer(segments, buffer, current_color);
            current_color = default_color;

            TextLine line;
            line.width = ImGui::CalcTextSize(current_line.c_str()).x;
            line.segments = segments;
            lines.emplace_back(line);
            segments.clear();

            continue;
        }

        // Tabs are kept as-is (rendering decides width)
        // if (c == '\t') {
        //     flush();
        //     result.emplace_back("\t", currentColor);
        //     continue;
        // }

        // Normal character (includes tabs but not including newline)
        buffer.push_back(c);
        current_line.push_back(c);
    }

    FlushBuffer(segments, buffer, current_color);
    TextLine line;
    line.segments = segments;
    lines.emplace_back(line);
    return lines;
}

// TODO: move to gui library/utilities
bool isTransparent(ImU32 color) {
    return ((color >> IM_COL32_A_SHIFT) & 0xFF) == 0;
}

// Draw background
/*if (!automatte && drawbg) {
    GFXColorf(this->bgcol);
    DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight);
}*/

/*if (!automatte && drawbg) {
    GFXColorf(this->bgcol);
    DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight * .75);
}*/

void drawBackground(ImDrawList* draw_list, ImU32 background_color, 
                    ImVec2 position, ImVec2 size, bool automatte) {
    const ImVec2 pad(4.0f, 2.0f); // TODO: make this variable
}

void drawBackgroundAroundText(ImDrawList* draw_list, ImU32 background_color, 
                              ImVec2 position, std::string text, bool automatte) {
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str());                        
    drawBackground(draw_list, background_color, position, text_size, automatte);
}



int TextPlane::Draw(const string &newText, int offset, bool start_lower, bool force_highquality, bool automatte) {
    std::pair<int,int> pair = CalculateAbsoluteXY(myDims.k, myFontMetrics.k);
    ImVec2 position(pair.first, pair.second);

    std::vector<TextLine> lines = ParseText(newText, color);
    ImVec2 text_size;
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    const ImVec2 pad(4.0f, 2.0f); // TODO: make this variable
    
    // Move one line up if not start_lower 
    if(!start_lower) {
        // Calculate dummy line, otherwise, text might move several lines down
        text_size = ImGui::CalcTextSize("hello world");
        position.y -= text_size.y;
    }

    for (TextLine& line : lines) {
        for(TextSegment& segment : line.segments) {
            text_size = ImGui::CalcTextSize(segment.text.c_str());

            // Background rectangle
            if(!isTransparent(background_color) && !automatte) {
                // Need these because we pass a reference
                // Need explicit construction because ImVec2 did not overload arithmetic operators
                // TODO: add this to imgui
                
                const ImVec2 start_position(position.x - pad.x, position.y - pad.y);
                const ImVec2 end_position(position.x + text_size.x + pad.x, position.y +text_size.y + pad.y);
                draw_list->AddRectFilled(
                    start_position,
                    end_position,
                    background_color,
                    0.0f // No rounded borders
                );

            }

            draw_list->AddText(position, segment.color, segment.text.c_str());
             
            position.x += text_size.x;
        }

        if(line.segments.empty()) {
           text_size = ImGui::CalcTextSize("hello world"); 
        }

        position.y += text_size.y;
        position.x = pair.first;
    }

    return 1;
}

