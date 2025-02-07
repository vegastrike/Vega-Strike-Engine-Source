/*
 * painttext.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Mike Byron specifically
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
#ifndef VEGA_STRIKE_ENGINE_GUI_PAINTTEXT_H
#define VEGA_STRIKE_ENGINE_GUI_PAINTTEXT_H

#include <string>
#include "guidefs.h"
#include "font.h"

//See cpp file for detailed descriptions of classes, functions, etc.

//This class is used to draw text in a rectangle.  It is not a Control, just a
//repository of text attributes and behavior.  The line spacing is the same as the
//Font size.  Multi-line text starts at the top-left of the rectangle, and continues
//for as many lines a necessary.  Single-line text is centered vertically, and an
//ellipsis is added at the end of the visible text if the text must be clipped to
//fit in the rectangle.
//This object can be used to calculate, say, the bounding box of some text without
//needing to actually draw it.
//This class supports a simple language embedded in the text that can be used to
//change some attributes fo sections of text.

//This class allows a number of formatting characters.  Here are the rules:
//-- The formatting char is "#".
//-- Format commands are indicated by a single character, which is case-sensitive.
//-- The results of errors in the formatting commands is not defined.
//-- Commands can take no parameter, a required param, or an optional param.
//-- Commands that take parameters always end with another "#".
//-- Some kinds of format state are pushed onto a stack.  You can pop the stack back
//to the previous value, or reset to the original value.
//##: One "#".  "This is door ##3."
//#n[line-space]#:  Break a line here.  Optional floating-point parameter which is the
//line spacing between the current line and the next line only.  (This doesn't
//affect line spacing after the current line.)
//0.0, for instance, would results in overwriting the same line, 2.0 is double-space.
//"First line.#n#Second line."  "Double space#n2#line#n#line 2".
//#l<line-spacing>#:  Set the amount of space between lines.  Parameter is not optional.
//This command does not break the line, it just sets the line spacing for the
//following lines.  There isn't a stack associated with this setting.
//#b[stroke-weight]#: "Bold".  "Push" the font stroke weight.  Parameter has same semantics
//as Font object -- floating-point.  Default stroke weight is BOLD_STROKE.
//"#b#Price:#-b $50,000"  "I must #b4.5#emphasize#!b this."
//#c<color-spec>#: "Push" a new text color (doesn't change the background color).
//Specify R, then B, G, A as floating-point values between 0 and 1.0 (same spec as
//GFXColor).  The value are separated by ":", and A is optional.  Default for A
//is 1.0.
//"This is #c1:0:0#red#-c."
//#-{bc}:  "Pop" a font/color off the appropriate stack.  Restores previous value.
//See font and color examples above.
//#!{bc}:  "Reset" font/color to the original version.  Same parameters as "#-".

//Helper function.  Pass in RGB values, get out a color command string for those values.
std::string colorsToCommandString(float r, float g, float b, float a = 1.0);

//Class.
class PaintText {
public:
//The outside boundaries to use for drawing.
    virtual void setRect(const Rect &r);

    virtual Rect rect(void) const {
        return m_rect;
    }

//The text to draw.
    virtual void setText(const std::string &text);

    virtual const std::string &text(void) const {
        return m_text;
    }

//The initial color of the text.
    virtual void setColor(const GFXColor &c);

    virtual GFXColor color(void) const {
        return m_color;
    }

//The initial Font for text.
    virtual void setFont(const Font &f);

    virtual Font font(void) const {
        return m_font;
    }

//Text justification.
    virtual void setJustification(Justification j);

    virtual Justification justification(void) const {
        return m_justification;
    }

//What to do when text width exceeds boundary rectangle.
    enum WidthExceeded {
        FULL_WIDTH,             //Do nothing.  Just do one line of text.
        ELLIPSIS,               //Put an ellipsis at the end of the text.
        MULTI_LINE              //Go to the next line.
    };

//What to do when text width exceeds boundary rectangle.
    virtual void setWidthExceeded(WidthExceeded w);

    virtual WidthExceeded widthExceeded(void) const {
        return m_widthExceeded;
    }

//How many lines are in the current layout.
    virtual int lineCount(void) const;

//How many lines would be painted in a vertical interval.
    virtual int visibleLineCountStartingWith(int lineNumber, float vertInterval) const;

//Layout version.  This is used to tell whether the layout has changed.
    virtual int layoutVersion(void) const;

//Draw the text.
    static const size_t END_LINE;
    virtual void drawLines(size_t start, size_t count = END_LINE) const;

    void draw(void) const {
        drawLines(0);
    }

//CONSTRUCTION
public:
    PaintText(void);
    PaintText(const Rect &rect,
            const std::string &text,
            const Font &font,
            const GFXColor &color,
            Justification just = RIGHT_JUSTIFY,
            WidthExceeded w = ELLIPSIS);

    virtual ~PaintText(void) {
    }

protected:
//INTERNAL IMPLEMENTATION

//Use the current attributes to create a display list for the text.
//This does the real work, and doesn't check whether it needs to be done.
    void calcLayout(void);

//Check whether we need to recalc the layout, and do it in const object.
    void calcLayoutIfNeeded(void) const;

//Description of a "fragment" of text to be displayed.
//This is a section of text with the same attributes.
    struct TextFragment {
        size_t start;        //Index (in m_text) of first character in the fragment.
        size_t end;          //Index of last character in fragment (NOT one past last char).
        Font font;      //Font.
        GFXColor color;     //Text color.
    };

//Layout of one line.
    struct TextLine {
        float height;                   //Height of this line.
        float width;                    //Width of all the chars in this line (including ellipsis).
        float x;                        //x-origin of first character, relative to boundary rect.
        float baseLine;                 //y-origin of first character, relative to top of line.
        vector<TextFragment> fragments; //Bits of text that make up this line.
    };

//The state used to calculate the layout.
    struct LayoutState {
        vector<GFXColor> colorStack;        //Color stack.
        vector<Font> fontStack;        //Font stack.
        float currentLineSpacing;           //Line spacing for the current line.
        float permanentLineSpacing;         //New permanent line spacing.

        //CONSTRUCTION.
        LayoutState(float lineSpacing = 0.0, float permSpacing = 0.0) :
                currentLineSpacing(lineSpacing), permanentLineSpacing(permSpacing) {
        }
    };

//The x-origin of a line.  Horizontal starting position.
    float lineInset(const TextLine &line);

//Create a fragment for the next substring of characters that fits in the specified width.
//The fragment is added to the specified TextLine.
//Formatting commands should have been filtered out already.
    void addFragment(TextLine &line, //Line descriptor.
            const std::string::size_type endPos, //One past last char to consider.
            std::string::size_type &startPos, //IN/OUT: location of string.
            double &width //IN/OUT: Reference width of string.
    );

//Parse a format string in a PaintText string.
//The first character should be the one *after* the initial format char.
    void parseFormat(std::string::size_type startPos,         //Location of beginning of string to examine.
            std::string::size_type *resultPos,       //OUT: Ptr to string past the format string.
            bool *endLine         //OUT: True = Done with current line.
    );

//Parse one line of text, create fragments, end line when overflow width.
    void parseFragmentsWithCharBreak(TextLine &line, //Line descriptor.
            std::string::size_type startPos, //Location of beginning of string to examine.
            std::string::size_type endPos, //Location of one past the last character to examine.
            float maxWidth, //Can't go beyond this width.
            bool ellipsis, //True = if line doesn't fit, append ellipsis.
            std::string::size_type *resultPos //OUT: Ptr to string past the format string.
    );

//Parse one line of text, create fragments, end line on word break when width overflows.
    void parseFragmentsWithWordBreak(TextLine &line, //Line descriptor.
            std::string::size_type startPos, //Location of beginning of string to examine.
            float maxWidth, //Can't go beyond this width.
            std::string::size_type *resultPos //OUT: Ptr to string past the format string.
    );

//VARIABLES
protected:
    Rect m_rect;                //Boundary rectangle.
    std::string m_text;         //Text to draw.
    GFXColor m_color;           //Initial color.
    Font m_font;                //Initial font.
    Justification m_justification; //Right, center, left.
    WidthExceeded m_widthExceeded; //What to do when text width exceeds boundary rectangle.

    bool m_needLayout; //True = something changed.  Need to recalc the layout.
    int m_layoutVersion;        //Incremented every time we re-layout things.
    vector<TextLine> m_lines;  //List of lines.
    double m_verticalScaling; //Vertical factor from char reference space to identity space.
    double m_horizontalScaling; //Horizontal factor from char reference space to identity space.
    LayoutState m_layout; //Shared state for layout operation.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_PAINTTEXT_H
