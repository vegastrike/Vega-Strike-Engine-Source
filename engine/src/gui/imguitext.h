/*
 * imguitext.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Danny Gehl
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
#pragma once
#include "control.h"
#include <vector>
#include <string>
#include "guidefs.h"
#include "imgui/imgui.h"

//Formatting characters for ImGuiText() strings.
static const char DT_FORMAT_CHAR = '#';

static const char DT_FORMAT_NEWLINE_CHAR = 'n';    //Break line.  Optional line spacing.
static const char DT_FORMAT_LINE_SPACING_CHAR = 'l'; //Set new line spacing.
static const char DT_FORMAT_STROKE_CHAR = 'b';    //Push new stroke width.  Optional width.
static const char DT_FORMAT_COLOR_CHAR = 'c';    //Push new color.
static const char DT_FORMAT_POP_CHAR = '-';    //Pop color/stroke.
static const char DT_FORMAT_RESET_CHAR = '!';    //Reset to original color/stroke.

static const char DT_FORMAT_COLOR_SEP = ':';    //Separator in color specification.
static const float BOGUS_LINE_SPACING = -100.0; //"NULL" value for line spacing.

//The ellipsis at the end of a line is represented as a special text fragment.  We mark
//the fragment with this constant as the start position.
static const size_t ELLIPSIS_FRAGMENT = 64; //@
static const std::string ELLIPSIS_STRING = "...";

struct TextFragment {
    std::string text;
    Font font;
    ImU32 color;
    bool isBold;
    float width;
};

struct Line : public std::vector<TextFragment> {
    float width = 0.0f;
    float lineHeight = 0.0f;
    float lineSpacing = 0.0f;
};
struct FormattedLayout : public std::vector<Line> {
    void endLine(Line& currentLine);
    float currentLineSpacing = 0.0f;           //Line spacing for the current line.
    float permanentLineSpacing = 0.0f;         //New permanent line spacing.
    bool needsProcessing = true;
};

class ImGuiText {
public:
    void setRect(const Rect &r) { m_rect = r; }
    void setText(const std::string &text);
    std::string text( void ) { return m_text; }
    void setFont(const Font &f) { m_font = f; }
    Font font(void) { return m_font; }
    void setJustification(Justification j) { m_justification = j; }
    Justification justification(void) { return m_justification; };
    void setColor(const GFXColor &c) { m_color = c; }
    GFXColor color() { return m_color; };
    int lineCount(void) const { return m_layout.size(); };
    int layoutVersion(void) const { return m_layoutVersion; };
    int visibleLineCountStartingWith(int lineNumber, float vertInterval);
    //Whether the text is multi-line.
    bool multiLine(void) {
        return m_multiLine;
    }
    void setMultiLine(bool multi) {
        m_multiLine = multi;
    }    
    // Draws the text
    void draw(int firstLineToDraw=0);
    ImVec2 getTextWidth(const std::string& text, const Font& font);
private:
    Rect m_rect;
    Font m_font;
    std::string m_text;
    Justification m_justification = LEFT_JUSTIFY;
    bool m_multiLine = false;
    GFXColor m_color;
    FormattedLayout m_layout;
    int m_layoutVersion = 0;

    vector<ImU32> m_colorStack;        //Color stack.
    vector<Font> m_fontStack;             //Font stack.


    FormattedLayout parseText(
        const std::string& input, // the text to parse
        const float widthInPixels // the width in pixels after which lines need to break
    );
    void parseFormatFloat(const std::string &str, //String.
        const size_t startPos, //First character to examine.
        const size_t endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        float *resultValue, //OUT: Parsed value.  If no value, not changed.
        size_t *resultPos, //OUT: One past last format char.
        const char optionalTerminator = '\0' //Another terminator besides DT_FORMAT_CHAR
    );
    void parseFormatColor(const std::string &str, //String.
        const size_t startPos, //First character to examine.
        const size_t endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        GFXColor &color, //OUT: Parsed value.
        size_t *resultPos //OUT: One past last format char.
    );
    void parseFormat(std::string input, // the text to parse
        size_t startPos, //Location of beginning of string to examine.
        size_t *resultPos, //OUT: Ptr to string past the format string.
        bool *endLine //OUT: True = Done with current line.
    );
    void parseTextIfNeeded();
};