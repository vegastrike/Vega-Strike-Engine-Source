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

const std::string &getStringFont(bool &changed, bool force_inside = false, bool whatinside = false) {
    bool inside = isInside();
    if (force_inside) {
        inside = whatinside;
    }
    static bool lastinside = inside;
    if (lastinside != inside) {
        changed = true;
        lastinside = inside;
    } else {
        changed = false;
    }
    return inside ? configuration().graphics.bases.font : configuration().graphics.font;
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

void *getFont(bool forceinside = false, bool whichinside = false) {
    bool changed = false;
    std::string whichfont = getStringFont(changed, forceinside, whichinside);
    static void *retval = nullptr;
    if (changed) {
        retval = nullptr;
    }
    if (retval == nullptr) {
        if (whichfont == "helvetica10") {
            retval = GLUT_BITMAP_HELVETICA_10;
        } else if (whichfont == "helvetica18") {
            retval = GLUT_BITMAP_HELVETICA_18;
        } else if (whichfont == "times24") {
            retval = GLUT_BITMAP_TIMES_ROMAN_24;
        } else if (whichfont == "times10") {
            retval = GLUT_BITMAP_TIMES_ROMAN_10;
        } else if (whichfont == "fixed13") {
            retval = GLUT_BITMAP_8_BY_13;
        } else if (whichfont == "fixed15") {
            retval = GLUT_BITMAP_9_BY_15;
        } else {
            retval = GLUT_BITMAP_HELVETICA_12;
        }
    }
    return retval;
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
    col = c;
    this->bgcol = bgcol;
    myDims.i = 2;
    myDims.j = -2;
    myFontMetrics.Set(.06, .08, 0);
    SetPos(0, 0);
}

TextPlane::~TextPlane() = default;

int TextPlane::Draw(int offset) {
    return Draw(myText, offset, true, false, true);
}

static unsigned int *CreateLists() {
    static unsigned int lists[256] = {0};
    void *fnt0 = getFont(true, false);
    void *fnt1 = getFont(true, true);
    const bool use_bit = configuration().graphics.high_quality_font;
    const bool use_display_lists = configuration().graphics.text_display_lists;
    if (use_display_lists) {
        for (unsigned int i = 32; i < 256; i++) {
            if ((i < 128) || (i >= 128 + 32)) {
                lists[i] = GFXCreateList();
                if (use_bit) {
                    glutBitmapCharacter(i < 128 ? fnt0 : fnt1, i % 128);
                } else {
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, i % 128);
                }
                if (!GFXEndList()) {
                    lists[i] = 0;
                }
            }
        }
    }
    return lists;
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

void DrawSquare(float left, float right, float top, float bot) {
    float verts[8 * 3] = {
            left, top, 0,
            left, bot, 0,
            right, bot, 0,
            right, top, 0,
            right, top, 0,
            right, bot, 0,
            left, bot, 0,
            left, top, 0
    };
    GFXDraw(GFXQUAD, verts, 8);
}

float charWidth(char c, float myFontMetrics) {
    const bool use_bit = configuration().graphics.high_quality_font;
    void *fnt = use_bit ? getFont() : GLUT_STROKE_ROMAN;
    float charwid = use_bit ? glutBitmapWidth(fnt, c) : glutStrokeWidth(fnt, c);
    float dubyawid = use_bit ? glutBitmapWidth(fnt, 'W') : glutStrokeWidth(fnt, 'W');
    return charwid * myFontMetrics / dubyawid;
}

bool doNewLine(string::const_iterator begin,
        string::const_iterator end,
        float cur_pos,
        float end_pos,
        float metrics,
        bool last_row) {
    if (*begin == '\n') {
        return true;
    }
    if (*begin == ' ' && !last_row) {
        cur_pos += charWidth(*begin, metrics);
        for (++begin; begin != end && cur_pos <= end_pos && !isspace(*begin); ++begin) {
            cur_pos += charWidth(*begin, metrics);
        }
        return cur_pos > end_pos;
    }
    return cur_pos + ((begin + 1 != end) ? charWidth(*begin, metrics) : 0) >= end_pos;
}


static const ImU32 default_color = IM_COL32(255,255,255,255);

// Text in a specific color
struct TextSegment {
    std::string text;
    ImU32 color;
};

struct TextLine {
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


static std::vector<TextLine> ParseText(const std::string& text) {
    std::vector<TextLine> lines;
    std::vector<TextSegment> segments;

    ImU32 current_color = default_color;
    std::string buffer;

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
    }

    FlushBuffer(segments, buffer, current_color);
    TextLine line;
    line.segments = segments;
    lines.emplace_back(line);
    return lines;
}

int TextPlane::Draw(const string &newText, int offset, bool startlower, bool force_highquality, bool automatte) {
    std::pair<int,int> pair = CalculateAbsoluteXY(myDims.k, myFontMetrics.k);
    ImVec2 position(pair.first, pair.second);

    std::vector<TextLine> lines = ParseText(newText);
    ImVec2 text_size;

    for (TextLine& line : lines) {
        for(TextSegment& segment : line.segments) {
            ImGui::GetForegroundDrawList()->AddText(position, segment.color, segment.text.c_str());
            text_size = ImGui::CalcTextSize(segment.text.c_str()); 
            position.x += text_size.x;
        }

        position.y += text_size.y;
        position.x = pair.first;
    }

    return 1;
}

