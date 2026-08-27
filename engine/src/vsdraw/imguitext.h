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
#include <cctype>
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

// Display helper: turn underscores into spaces and capitalize the first letter
// of each word. Used for base/link labels authored with underscores (e.g.
// "Upgrade_Ship" -> "Upgrade Ship"). Lives here because it is a text-display
// concern of the shared drawing library.
inline std::string beautify(const std::string &input) {
    std::string result;
    bool wordStart = true;
    for (auto i = input.begin(); i != input.end(); ++i) {
        if (*i == '_') {
            result += ' ';
            wordStart = true;
        } else if (wordStart) {
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(*i)));
            wordStart = false;
        } else {
            result += *i;
        }
    }
    return result;
}

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
    // TextPlane-compatible constructor: text color + background color.
    ImGuiText(const GFXColor &col = GFXColor(1, 1, 1, 1),
            const GFXColor &bgcol = GFXColor(0, 0, 0, 0))
            : m_color(col), m_colorU32(static_cast<ImU32>(col)),
              m_backgroundColor(static_cast<ImU32>(bgcol)) {}
    void setRect(const Rect &r) { m_rect = r; }
    void setText(const std::string &text);
    std::string text( void ) { return m_text; }
    std::string getText() { return m_text; }
    void setFont(const Font &f) { m_font = f; }
    Font font(void) { return m_font; }
    void setJustification(Justification j) { m_justification = j; }
    Justification justification(void) { return m_justification; };
    void setColor(const GFXColor &c) { m_color = c; m_colorU32 = static_cast<ImU32>(c); }
    GFXColor color() { return m_color; };
    // Text render scale (1.0 = normal). The base-room word-by-word streaming text
    // (othtext) sets this to 2.0 so it renders larger; other text stays at 1.0.
    void setTextScale(float s) { m_textScale = s; }
    float textScale() const { return m_textScale; }
    // Set the text color from a packed ImU32 (TextPlane-compatible interface).
    void setColorU32(ImU32 c) { m_colorU32 = c; m_color = GFXColor(c); }
    // Background rectangle behind the text (TextPlane `background_color`).
    void setBackgroundColor(ImU32 c) { m_backgroundColor = c; }
    ImU32 backgroundColor() const { return m_backgroundColor; }
    // Whether to suppress the background rectangle (TextPlane `automatte`).
    void setAutomatte(bool a) { m_automatte = a; }
    bool automatte() const { return m_automatte; }
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
    // TextPlane-compatible convenience: set position (normalized top-left origin),
    // size (normalized width/height), and char size (glyph width/height).
    void setPos(float x, float y) { m_rect.origin.x = x; m_rect.origin.y = y; }
    void setSize(float w, float h) { m_rect.size.width = w; m_rect.size.height = h; }
    void setCharSize(float w, float h) { m_charW = w; m_charH = h; }
    void getCharSize(float &x, float &y) { x = m_charW; y = m_charH; }
    // TextPlane-compatible reads.  Note the argument order matches TextPlane:
    // getPos(y, x) and getSize(x, y), reading back the stored rect.
    void getPos(float &y, float &x) { y = m_rect.origin.y; x = m_rect.origin.x; }
    void getSize(float &x, float &y) { x = m_rect.size.width; y = m_rect.size.height; }
    // TextPlane-compatible drawing entry point. Replicates TextPlane::Draw semantics
    // (top-left anchor, optional one-line-lower, force_highquality, automatte background)
    // using the unified ImGuiText parser. Returns line count.
    int Draw(const std::string &newText,
            int offset = 0,
            bool start_lower = false,
            bool force_highquality = false,
            bool automatte = false);
    int Draw(int offset = 0) { return Draw(m_text, offset, false, false, m_automatte); }
    ImVec2 getTextWidth(const std::string text, const float fontSize);
    // The target resolution to lay out against. Bases (Python-driven) set this to
    // the persisted base resolution (graphics.bases.max_width/height); base computer
    // and HUD (C++-driven) leave the default, which uses the screen/ImGui DisplaySize.
    // 0/0 = use the screen DisplaySize.
    void setResolution(float w, float h) { m_resW = w; m_resH = h; }
private:
    // Effective layout resolution: the caller-set base resolution if provided,
    // otherwise the screen / ImGui DisplaySize (0 = use screen).
    float resW() const { return m_resW > 0.0f ? m_resW : ImGui::GetIO().DisplaySize.x; }
    float resH() const { return m_resH > 0.0f ? m_resH : ImGui::GetIO().DisplaySize.y; }
    Rect m_rect;
    Font m_font;
    std::string m_text;
    Justification m_justification = LEFT_JUSTIFY;
    bool m_multiLine = false;
    GFXColor m_color;
    ImU32 m_colorU32 = IM_COL32(255, 255, 255, 255);
    float m_textScale = 1.0f;    // Text render scale (1.0 = normal).
    ImU32 m_backgroundColor = IM_COL32(0, 0, 0, 0);
    bool m_automatte = false;
    float m_charW = 0.06f;
    float m_charH = 0.08f;
    float m_resW = 0.0f;   // target layout resolution (0 = use screen DisplaySize)
    float m_resH = 0.0f;
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