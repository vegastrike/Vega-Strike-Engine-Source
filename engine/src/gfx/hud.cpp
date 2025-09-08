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

#include "gldrv/gl_globals.h"

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

int TextPlane::Draw(const string &newText, int offset, bool startlower, bool force_highquality, bool automatte) {
    int retval = 1;
    bool drawbg = (bgcol.a != 0);
    static unsigned int *display_lists = CreateLists();
    //some stuff to draw the text stuff
    string::const_iterator text_it = newText.begin();
    const bool use_bit = force_highquality || configuration().graphics.high_quality_font;
    const float font_point = configuration().graphics.font_point_flt;
    const bool font_antialias = configuration().graphics.font_antialias;
    void *fnt = getFont();
    static float std_wid = glutStrokeWidth(GLUT_STROKE_ROMAN, 'W');
    myFontMetrics.i = font_point * std_wid / (119.05 + 33.33);
    if (use_bit) {
        myFontMetrics.i = glutBitmapWidth(fnt, 'W');
    }
    myFontMetrics.j = font_point;
    myFontMetrics.i /= .5 * configuration().graphics.resolution_x;
    myFontMetrics.j /= .5 * configuration().graphics.resolution_y;
    float tmp, row, col;
    float origcol;
    GetPos(row, col);
    GetPos(row, origcol);
    float rowheight = use_bit ? getFontHeight() : myFontMetrics.j;
    myFontMetrics.j = rowheight;
    if (startlower) {
        row -= rowheight;
    }
    GFXPushBlendMode();
    glLineWidth(1);
    if (!use_bit && font_antialias) {
        GFXBlendMode(SRCALPHA, INVSRCALPHA);
        if (configuration().graphics.smooth_lines) {
            glEnable(GL_LINE_SMOOTH);
        }
    } else {
        GFXBlendMode(SRCALPHA, INVSRCALPHA);
        if (configuration().graphics.smooth_lines) {
            glDisable(GL_LINE_SMOOTH);
        }
    }
    GFXColorf(this->col);
    GFXDisable(DEPTHTEST);
    GFXDisable(CULLFACE);
    GFXDisable(LIGHTING);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    glPushMatrix();
    glLoadIdentity();
    if (!automatte && drawbg) {
        GFXColorf(this->bgcol);
        DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight);
    }
    GFXColorf(this->col);
    int entercount = 0;
    for (; entercount < offset && text_it != newText.end(); text_it++) {
        if (*text_it == '\n') {
            entercount++;
        }
    }
    glTranslatef(col, row, 0);
    glRasterPos2f(0, 0);
    float scalex = 1;
    float scaley = 1;
    int potentialincrease = 0;
    if (!use_bit) {
        int numplayers = 1;
        if (_Universe) {          //_Universe can be NULL during bootstrap.
            numplayers = (_Universe->numPlayers() > 3 ? _Universe->numPlayers() / 2
                    : _Universe->numPlayers());
        }
        scalex = numplayers * myFontMetrics.i / std_wid;
        scaley = myFontMetrics.j / (119.05 + 33.33);
    }
    glScalef(scalex, scaley, 1);
    bool firstThroughLoop = true;
    GFXColor currentCol(this->col);
    while (text_it != newText.end() && (firstThroughLoop || row > myDims.j - rowheight * .25)) {
        unsigned char myc = *text_it;
        if (myc == '_') {
            myc = ' ';
        }
        float shadowlen = 0;
        if (myc == '\t') {
            shadowlen = glutBitmapWidth(fnt, ' ') * 5. / (.5 * configuration().graphics.resolution_x);
        } else {
            if (use_bit) {
                shadowlen = glutBitmapWidth(fnt, myc) / (float) (.5
                        * configuration().graphics.resolution_x);                    //need to use myc -- could have transformed '_' to ' '
            } else {
                shadowlen = myFontMetrics.i * glutStrokeWidth(GLUT_STROKE_ROMAN, myc) / std_wid;
            }
        }
        if (*text_it == '#') {
            if (newText.end() - text_it > 6) {
                float r, g, b;
                r = TwoCharToFloat(*(text_it + 1), *(text_it + 2));
                g = TwoCharToFloat(*(text_it + 3), *(text_it + 4));
                b = TwoCharToFloat(*(text_it + 5), *(text_it + 6));
                if (r == 0 && g == 0 && b == 0) {
                    currentCol = this->col;
                } else {
                    currentCol = GFXColor(r, g, b, this->col.a);
                }
                GFXColorf(currentCol);
                const bool setRasterPos = configuration().graphics.set_raster_text_color;
                if (use_bit && setRasterPos) {
                    glRasterPos2f(col - origcol, 0);
                }
                text_it = text_it + 6;
            } else {
                break;
            }
            text_it++;
            continue;
        } else if (*text_it >= 32) {
            //always true
            if (automatte) {
                GFXColorf(this->bgcol);
                DrawSquare(col - origcol,
                        col - origcol + shadowlen / scalex,
                        -rowheight * .25 / scaley,
                        rowheight * .75 / scaley);
                GFXColorf(currentCol);
            }
            //glutStrokeCharacter (GLUT_STROKE_ROMAN,*text_it);
            retval += potentialincrease;
            potentialincrease = 0;
            int lists = display_lists[myc + (isInside() ? 128 : 0)];
            if (lists) {
                GFXCallList(lists);
            } else {
                if (use_bit) {
                    glutBitmapCharacter(fnt, myc);
                } else {
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, myc);
                }
            }
        }
        if (*text_it == '\t') {
            if (automatte) {
                GFXColorf(this->bgcol);
                DrawSquare(col - origcol,
                        col - origcol + shadowlen * 5 / (.5 * configuration().graphics.resolution_x),
                        -rowheight * .25 / scaley,
                        rowheight * .75 / scaley);
                GFXColorf(currentCol);
            }
            col += shadowlen;
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
            glutBitmapCharacter(fnt, ' ');
        } else {
            col += shadowlen;
        }
        if (doNewLine(text_it, newText.end(), col, myDims.i, myFontMetrics.i, row - rowheight <= myDims.j)) {
            GetPos(tmp, col);
            firstThroughLoop = false;
            row -= rowheight;
            glPopMatrix();
            glPushMatrix();
            glLoadIdentity();
            if (!automatte && drawbg) {
                GFXColorf(this->bgcol);
                DrawSquare(col, this->myDims.i, row - rowheight * .25, row + rowheight * .75);
            }
            if (*text_it == '\n') {
                currentCol = this->col;
            }
            GFXColorf(currentCol);
            glTranslatef(col, row, 0);
            glScalef(scalex, scaley, 1);
            glRasterPos2f(0, 0);
            potentialincrease++;
        }
        text_it++;
    }
    if (configuration().graphics.smooth_lines) {
        glDisable(GL_LINE_SMOOTH);
    }
    glPopMatrix();

    GFXPopBlendMode();
    GFXColorf(this->col);
    return retval;
}

