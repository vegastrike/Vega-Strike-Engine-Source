/*
 * painttext.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Mike Byron specifically
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

#include "vegastrike.h"

#include "painttext.h"

#include "vs_globals.h"
#include "config_xml.h"
#include "gldrv/gl_globals.h"
const size_t PaintText::END_LINE = 1000000;           //Draw to the end.
extern bool useStroke();
//This function allows a number of formatting characters.  Here are the rules:
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
//#-{bc}:  "Pop" a font/color/ off the appropriate stack.  Restores previous value.
//See font and color examples above.
//#!{bc}:  "Reset" font/color to the original version.  Same parameters as "#-".

//Old format characters that need to be converted.
static const char OLD_FORMAT_NEWLINE = '\\';

//Formatting characters for PaintText() strings.
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
static const string::size_type ELLIPSIS_FRAGMENT = 64; //@
static const string ELLIPSIS_STRING = "...";

//This object compares vertical distances to see whether lines fit in rectangles.
//These are floating-point calculations, and have some error involved.
//Below is a small factor of the "next" line height we use when comparing to see whether
//that line will fit in a vertical interval.
static const float LINE_HEIGHT_EPSILON = .99;       //1% of the line height.

//Have a reserve for the lines array so we don't have a lot of copying.
static const int LINES_RESERVE = 50;

//The outside boundaries to use for drawing.
void PaintText::setRect(const Rect &r) {
    if (m_rect != r) {
        m_rect = r;
        m_needLayout = true;
    }
}

//The text to draw.
void PaintText::setText(const std::string &text) {
    const string *finalText = &text;
    string conversionResult;
    if (text.find(OLD_FORMAT_NEWLINE) != string::npos) {
        //We have some old format characters we need to convert.
        for (string::const_iterator i = text.begin(); i != text.end(); i++) {
            if (*i == OLD_FORMAT_NEWLINE) {
                conversionResult.append("#n#");
            } else {
                conversionResult += (*i);
            }
        }
        finalText = &conversionResult;
    }
    //OK, now we see whether anything has changed.
    if (m_text != *finalText) {
        m_text = *finalText;
        m_needLayout = true;
    }
}

//The initial color of the text.
void PaintText::setColor(const GFXColor &c) {
    if (!equalColors(m_color, c)) {
        m_color = c;
        m_needLayout = true;
    }
}

//The initial Font for text.
void PaintText::setFont(const Font &f) {
    if (m_font != f) {
        m_font = f;
        m_needLayout = true;
    }
}

//Text justification.
void PaintText::setJustification(Justification j) {
    if (m_justification != j) {
        m_justification = j;
        m_needLayout = true;
    }
}

//What to do when text width exceeds boundary rectangle.
void PaintText::setWidthExceeded(WidthExceeded w) {
    if (m_widthExceeded != w) {
        m_widthExceeded = w;
        m_needLayout = true;
    }
}

//How many lines are in the current layout.
int PaintText::lineCount(void) const {
    calcLayoutIfNeeded();
    return m_lines.size();
}

//How many lines would be painted in a vertical interval.
int PaintText::visibleLineCountStartingWith(int lineNumber, float vertInterval) const {
    calcLayoutIfNeeded();
    int result = 0;
    float currentHeight = vertInterval;
    for (vector<TextLine>::size_type i = lineNumber; i < m_lines.size(); ++i) {
        const float lineHeight = m_lines[i].height;
        if (currentHeight - lineHeight * LINE_HEIGHT_EPSILON < 0.0) {
            //Did all the lines.
            break;
        }
        currentHeight -= lineHeight;
        result++;
    }
    return result;
}

//Layout version.  This is used to tell whether the layout has changed.
int PaintText::layoutVersion(void) const {
    calcLayoutIfNeeded();
    return m_layoutVersion;
}

//Check whether we need to recalc the layout, and do it in const object.
void PaintText::calcLayoutIfNeeded(void) const {
    if (m_needLayout) {
        //calcLayout is a cache.  Doesn't change the "real" state of the object.
        PaintText *s = const_cast< PaintText * > (this);
        s->calcLayout();
    }
}

//Draw a fragment of text.  Assumes graphics origin and scaling are correct.
static float drawChars(const string &str,
        int start,
        int end,
        const Font &font,
        const GFXColor &color,
        float inRasterPos) {
    //Make sure the graphics state is right.
    GFXColorf(color);
    if (useStroke()) {
        glLineWidth(font.strokeWidth());
    } else {
        static bool setRasterPos =
                XMLSupport::parse_bool(vs_config->getVariable("graphics", "set_raster_text_color", "true"));
        if (setRasterPos) {
            glRasterPos2f(inRasterPos / (g_game.x_resolution / 2), 0);
        }
    }
    //Draw all the characters.
    for (int charPos = start; charPos <= end; charPos++) {
        inRasterPos += font.drawChar(str[charPos]);
    }
    return inRasterPos;
}

//Draw specified lines of text.
void PaintText::drawLines(size_t start, size_t count) const {
    //Make sure we hav a display list.
    calcLayoutIfNeeded();
    //Make sure we have something to do.
    if (m_lines.empty()) {
        return;
    }
    //Initialize the graphics state.
    GFXToggleTexture(false, 0);
    if (gl_options.smooth_lines) {
        glEnable(GL_LINE_SMOOTH);
    }
    GFXPushBlendMode();
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    glPushMatrix();
    //Keep track of line position.
    float lineTop = m_rect.top();
    //Figure ending line index.
    const size_t end = guiMin(start + count, m_lines.size());
    //Loop through the display list lines.
    for (size_t i = start; i < end; i++) {
        const TextLine &line = m_lines[i];
        //Make sure we can paint this line in the vertical space we have left.
        if (lineTop - line.height * LINE_HEIGHT_EPSILON < m_rect.origin.y) {
            //Not enough space to draw this line.
            break;
        }
        //Position at the start of the line.
        glLoadIdentity();
        glTranslatef(m_rect.origin.x + line.x, lineTop - line.baseLine, 0.0);
        if (line.fragments.size()) {
            GFXColorf(line.fragments[0].color);
        }
        if (!useStroke()) {
            glRasterPos2f(0, 0);
        } else {
            glScaled(m_horizontalScaling, m_verticalScaling, 1.0);
        }
        float rasterpos = 0;
        //Draw each fragment.
        for (vector<TextFragment>::const_iterator frag = line.fragments.begin(); frag != line.fragments.end(); frag++) {
            if (frag->start == ELLIPSIS_FRAGMENT) {
                //We have a special-case for the ellipsis at the end of a line.
                drawChars(ELLIPSIS_STRING, 0, 2, frag->font, frag->color, rasterpos);
            } else {
                rasterpos = drawChars(m_text, frag->start, frag->end, frag->font, frag->color, rasterpos);
            }
        }
        //Top of next line.
        lineTop -= line.height;
    }
    glRasterPos2f(0, 0);
    //Undo graphics state
    GFXPopBlendMode();
    if (gl_options.smooth_lines) {
        glDisable(GL_LINE_SMOOTH);
    }
    glPopMatrix();
    GFXToggleTexture(true, 0);
}

//Get a floating-point argument for a PaintText format command.
//This will not accept exponential format, just num-plus-decimal.
//The argument is optional, but must be ended with a format char.
//Examples:  #b2.35#, #b#.  #b is not allowed.
static void parseFormatFloat(const std::string &str, //String.
        const string::size_type startPos, //First character to examine.
        const string::size_type endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        float *resultValue, //OUT: Parsed value.  If no value, not changed.
        string::size_type *resultPos, //OUT: One past last format char.
        const char optionalTerminator = '\0' //Another terminator besides DT_FORMAT_CHAR
) {
    *formatSuccess = false;
    std::string num;
    string::size_type curPos;
    for (curPos = startPos; curPos < endPos; curPos++) {
        const char c = str[curPos];
        if (c == DT_FORMAT_CHAR || c == optionalTerminator) {
            //Found the trailing end of the format string.  Done.
            *formatSuccess = true;
            break;
        }
        //We only take digits and period, so we only parse simple floating numbers.
        //We'll take comma for simple localization purposes.
        if (isdigit(c) || c == '.' || c == ',') {
            num += c;
        } else {
            //Found a bad character.  Stop.
            break;
        }
    }
    *resultPos = curPos + 1;           //Skip over these chars no matter what.
    if (formatSuccess && num.size() > 0) {
        //Convert string to float.
        //Can't figure out std::locale, so we'll use easy, dumb conversion.
        *resultValue = atof(num.c_str());
    }
}

//Get a color argument for a PaintText format command.
//Format is R:B:G:A, where A is optional.  The numbers should be between 0 and 1.
//This will not accept exponential format, just num-plus-decimal.
//Examples:  #c1:.5:0.3:1.0#, #c.5:.5:.5#
static void parseFormatColor(const string &str, //String.
        const string::size_type startPos, //First character to examine.
        const string::size_type endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        GFXColor &color, //OUT: Parsed value.
        string::size_type *resultPos //OUT: One past last format char.
) {
    *formatSuccess = false;
    string::size_type curPos = startPos;
    parseFormatFloat(str, curPos, endPos, formatSuccess, &color.r, &curPos, DT_FORMAT_COLOR_SEP);
    if (!formatSuccess || str[curPos - 1] == DT_FORMAT_CHAR) {
        return;
    }
    parseFormatFloat(str, curPos, endPos, formatSuccess, &color.g, &curPos, DT_FORMAT_COLOR_SEP);
    if (!formatSuccess || str[curPos - 1] == DT_FORMAT_CHAR) {
        return;
    }
    parseFormatFloat(str, curPos, endPos, formatSuccess, &color.b, &curPos, DT_FORMAT_COLOR_SEP);
    if (!formatSuccess) {
        return;
    }
    if (str[curPos - 1] != DT_FORMAT_CHAR) {
        //Not done -- still have alpha to do.
        parseFormatFloat(str, curPos, endPos, formatSuccess, &color.a, &curPos);
    } else {
        //Default alpha value is opaque.
        color.a = 1.0;
    }
    *resultPos = curPos;
}

//Parse a format string in a PaintText string.
//The first character should be the one *after* the initial format char.
void PaintText::parseFormat(string::size_type startPos, //Location of beginning of string to examine.
        string::size_type *resultPos, //OUT: Ptr to string past the format string.
        bool *endLine //OUT: True = Done with current line.
) {
    const string::size_type endPos = m_text.size();
    //Default return value.
    *endLine = false;
    bool formatSuccess = false;
    string::size_type curPos = startPos;
    if (curPos < endPos) {
        //Make sure we have some chars to process.
        switch (m_text[curPos]) {
            case DT_FORMAT_NEWLINE_CHAR:
                //End of line.
            {
                float value = BOGUS_LINE_SPACING;            //Bogus value.
                parseFormatFloat(m_text, curPos + 1, endPos, &formatSuccess, &value, &curPos);
                if (formatSuccess) {
                    *endLine = true;                 //End of this line.
                    if (value != BOGUS_LINE_SPACING) {
                        m_layout.currentLineSpacing = value;
                    }
                }
                break;
            }
            case DT_FORMAT_LINE_SPACING_CHAR:
                //New permanent line spacing.
            {
                float value = BOGUS_LINE_SPACING;            //Bogus value.
                parseFormatFloat(m_text, curPos + 1, endPos, &formatSuccess, &value, &curPos);
                if (formatSuccess && value != BOGUS_LINE_SPACING) {
                    m_layout.permanentLineSpacing = value;
                }
                break;
            }
            case DT_FORMAT_STROKE_CHAR:
                //"Bold" -- change stroke width of font.
            {
                float strokeWeight = BOLD_STROKE;
                parseFormatFloat(m_text, curPos + 1, endPos, &formatSuccess, &strokeWeight, &curPos);
                if (formatSuccess) {
                    Font f(m_layout.fontStack.back());             //Make a new font.
                    f.setStrokeWeight(strokeWeight);
                    m_layout.fontStack.push_back(f);
                }
                break;
            }
            case DT_FORMAT_COLOR_CHAR:
                //Change the text color.
            {
                GFXColor color;
                parseFormatColor(m_text, curPos + 1, endPos, &formatSuccess, color, &curPos);
                if (formatSuccess) {
                    m_layout.colorStack.push_back(color);
                }
                break;
            }
            case DT_FORMAT_POP_CHAR:
                //Pop a color/font.
                curPos++;
                if (curPos == endPos) {
                    *endLine = true;
                } else if (m_text[curPos] == DT_FORMAT_STROKE_CHAR) {
                    if (m_layout.fontStack.size() > 1) {
                        m_layout.fontStack.pop_back();
                    } else if (m_text[curPos] == DT_FORMAT_COLOR_CHAR) {
                        if (m_layout.colorStack.size() > 1) {
                            m_layout.colorStack.pop_back();
                        }
                    }
                }
                curPos++;
                break;
            case DT_FORMAT_RESET_CHAR:
                //Reset colors/fonts back to original.  Pops all off stack except bottom.
                curPos++;
                if (curPos == endPos) {
                    *endLine = true;
                } else if (m_text[curPos] == DT_FORMAT_STROKE_CHAR) {
                    while (m_layout.fontStack.size() > 1) {
                        m_layout.fontStack.pop_back();
                    }
                } else if (m_text[curPos] == DT_FORMAT_COLOR_CHAR) {
                    while (m_layout.colorStack.size() > 1) {
                        m_layout.colorStack.pop_back();
                    }
                }
                curPos++;
                break;
        }
    }
    *resultPos = curPos;
}

//Create a fragment for the next substring of characters that fits in the specified width.
//The fragment is added to the specified TextLine.
//Formatting commands should have been filtered out already.
void PaintText::addFragment(TextLine &line, //Line descriptor.
        const string::size_type endPos, //One past last char to consider.
        string::size_type &startPos, //IN/OUT: location of string.
        double &width //IN/OUT: Reference width of string.
) {
    string::size_type curPos = startPos;
    const Font &font = m_layout.fontStack.back();
    //Loop through the characters until we run out of room.
    while (curPos < endPos) {
        double charWidth = font.charWidth(m_text[curPos]);
        if (width - charWidth < 0.0) {
            //The current character goes past the specified width.
            break;
        }
        width -= charWidth;
        curPos++;
    }
    //Create the fragment.
    if (curPos > startPos) {
        TextFragment frag;
        frag.start = startPos;
        frag.end = curPos - 1;         //Last char in frag, not one past.
        frag.font = font;
        frag.color = m_layout.colorStack.back();
        line.fragments.push_back(frag);
    }
    startPos = curPos;
}

//Return whether a character qualifies as a word break.
static bool isWordBreak(char c) {
    return c == ' ';
}

//Parse one line of text, create fragments, end line when overflow width.
void PaintText::parseFragmentsWithCharBreak(TextLine &line, //Line descriptor.
        string::size_type startPos, //Location of beginning of string to examine.
        string::size_type endPos, //Location of one past the last character to examine.
        float maxWidth, //Can't go beyond this width.
        bool ellipsis, //True = if line doesn't fit, append ellipsis.
        string::size_type *resultPos //OUT: Ptr to string past the format string.
) {
    string::size_type curPos = startPos;          //Beginning of current part of the string we are working on.
    double curWidth = maxWidth;     //The width left to work with.
    bool forceEndLine = false;    //True = end-of-line through format.  False = char width.
    while (curPos < endPos) {
        //Is there a format char left in this string?
        const string::size_type formatPos = m_text.find(DT_FORMAT_CHAR, curPos);
        if (formatPos == std::string::npos || formatPos >= endPos) {
            //No format char.
            addFragment(line, endPos, curPos, curWidth);
            break;
        }
        //Create fragment for characters before the format char.
        addFragment(line, formatPos, curPos, curWidth);
        if (curPos < formatPos) {
            //Format is past the max width.  We're done with this line.
            break;
        }
        //Interpret the format command.
        assert(m_text[curPos] == DT_FORMAT_CHAR);
        curPos++;         //Look at the command char.
        if (curPos >= endPos) {
            //No command char.  String ends with single "#".  Ignore it.
            curPos = endPos - 1;
            break;
        }
        if (m_text[curPos] == DT_FORMAT_CHAR) {
            //Double format char.  Equals one format char.
            const string::size_type oldPos = curPos;
            addFragment(line, curPos + 1, curPos, curWidth);
            if (curPos == oldPos) {
                //No room in the line for the one char.  Leave it for next line.
                curPos--;                 //Put chars back in string.
                break;                 //End of this line.
            }
        } else {
            parseFormat(curPos, &curPos, &forceEndLine);
            if (forceEndLine) {
                break;
            }
        }
    }
    if (!forceEndLine && curPos < endPos) {
        if (ellipsis) {
            //We need to append an ellipsis. We didn't use the whole string, and we didn't end
            //the line because of a format command.
            //We use the font at the end of the line.  This is a hack, but it seems like a
            //reasonable compromise.  If we use the font from the beginning of the line, suppose the
            //line starts out with bold, followed by a non-bold explanation?
            //We use the color of the last fragment before the ellipsis.

            const Font &font = m_layout.fontStack.back();
            const double ellipsisWidth = font.stringWidth(ELLIPSIS_STRING);
            //If ellipsis doesn't fit in the original width, just ship the truncated string.
            if (ellipsisWidth < maxWidth) {
                while (line.fragments.size() > 0) {
                    TextFragment &frag = line.fragments.back();
                    //Remove enough space in the last fragment to be able to append the ellipsis.
                    string::size_type i;
                    for (i = frag.end; i >= frag.start; i--) {
                        curWidth -= frag.font.charWidth(m_text[i]);
                        if (curWidth + ellipsisWidth <= maxWidth) {
                            //If we back up this far, the ellipsis will fit in the max width.
                            frag.end = i;
                            break;
                        }
                    }
                    if (i < frag.start) {
                        //Used up the whole fragment and still didn't find enough space.
                        line.fragments.pop_back();
                    } else {
                        //Create an ellipsis fragment and append it to the line.
                        TextFragment newFrag = frag;
                        newFrag.start = ELLIPSIS_FRAGMENT;
                        line.fragments.push_back(newFrag);
                        curWidth += ellipsisWidth;
                        break;
                    }
                }
            }
        } else {
            //Get rid of word break chars at the end of this line.
            //First, make sure to skip over any white space.
            while (curPos < endPos && isWordBreak(m_text[curPos])) {
                curPos++;
            }
            //Now, get rid of any word break chars at the tail end of our fragment list.
            while (line.fragments.size() > 0) {
                TextFragment &frag = line.fragments.back();
                string::size_type i;
                for (i = frag.end; i >= frag.start; i--) {
                    if (!isWordBreak(m_text[i])) {
                        break;
                    }
                    curWidth -= frag.font.charWidth(m_text[i]);
                }
                if (i >= frag.start) {
                    //Found something besides a word break in this fragment.
                    break;
                }
                //Used up the whole fragment.  Start on the next one.
                line.fragments.pop_back();
            }
        }
    }
    //Set the width of this line.
    line.width = maxWidth - curWidth;
    //And make sure we know how far we got in the string.
    *resultPos = curPos;
}

//Parse one line of text, create fragments, end line on word break when width overflows.
void PaintText::parseFragmentsWithWordBreak(TextLine &line, //Line descriptor.
        string::size_type startPos, //Location of beginning of string to examine.
        float maxWidth, //Can't go beyond this width.
        string::size_type *resultPos //OUT: Ptr to string past the format string.
) {
    string::size_type curPos = startPos;          //Beginning of current part of the string we are working on.
    double curWidth = maxWidth;     //The width left to work with.
    const string::size_type endPos = m_text.size();     //One past the end of the string.
    bool forceEndLine = false;   //True = end-of-line through format.  False = char width.
    LayoutState origLayout = m_layout;     //The original layout state before we start the line.
    string::size_type wordBreakPos = endPos;      //Previous word break location in text.
    //In this loop we just measure the width.  We find the end of the current line in m_text,
    //then call parseFragmentsWithCharBreak once we know how far to go.
    while (curPos < endPos) {
        //Is there a format char left in this string?
        const string::size_type formatPos = m_text.find(DT_FORMAT_CHAR, curPos);
        string::size_type endFragPos = formatPos;
        if (formatPos == std::string::npos || formatPos >= endPos) {
            //No format char.
            endFragPos = endPos;
        }
        //Loop through the characters until we run out of room.
        const Font &font = m_layout.fontStack.back();
        while (curPos < endFragPos) {
            double charWidth = font.charWidth(m_text[curPos]);
            if (isWordBreak(m_text[curPos])) {
                wordBreakPos = curPos;
            }
            if (curWidth - charWidth < 0.0) {
                //The current character goes past the specified width.
                break;
            }
            curWidth -= charWidth;
            curPos++;
        }
        if (curPos == endPos) {
            //The rest of the text is not as wide as the max.  We are done with this pass.
            wordBreakPos = endPos;
            break;
        } else if (curPos < endFragPos) {
            //We found a last character. Go back to the last word break.
            break;
        }
        assert(curPos == formatPos);         //Other other case: we ran into a format command.
        //Interpret the format command.
        assert(m_text[curPos] == DT_FORMAT_CHAR);
        curPos++;         //Look at the command char.
        if (curPos >= endPos) {
            //No command char.  String ends with single "#".  Ignore it.
            curPos = endPos - 1;
            break;
        }
        if (m_text[curPos] == DT_FORMAT_CHAR) {
            //Double format char.  Equals one format char.
            curWidth -= font.charWidth(DT_FORMAT_CHAR);
            if (curWidth < 0.0) {
                //No room in the line for the one char.  Leave it for next line.
                curPos--;                 //Put chars back in string.
                break;                 //End of this line.
            }
        } else {
            parseFormat(curPos, &curPos, &forceEndLine);
            if (forceEndLine) {
                wordBreakPos = endPos;
                break;
            }
        }
    }
    //Now we need to generate the fragments.
    //NOTE:  If the text contains a word that is too long for a line, we get a special case
    //where we never find a word break.  That ends up calling parseFragmentsWithCharBreak with
    //the end of the string as the limit, so the word is broken on a char boundary, which is
    //exactly what we want.
    string::size_type endLinePos = wordBreakPos + 1;
    //Include all extra word break characters.
    while (endLinePos + 1 < endPos && isWordBreak(m_text[endLinePos])) {
        endLinePos++;
    }
    m_layout = origLayout;                 //Undo any format changes.
    parseFragmentsWithCharBreak(line, startPos, endLinePos, maxWidth, false, resultPos);
}

//The x-origin of a line.  Horizontal starting position.
float PaintText::lineInset(const TextLine &line) {
    float result = 0.0;         //Assume we will RIGHT_JUSTIFY.
    if (m_justification == CENTER_JUSTIFY) {
        result = (m_rect.size.width - line.width * m_horizontalScaling) / 2.0;
    } else if (m_justification == LEFT_JUSTIFY) {
        result = m_rect.size.width - line.width * m_horizontalScaling;
    }
    return result;
}

//Use the current attributes to create a display list for the text.
//This does the real work, and doesn't check whether it needs to be done.
void PaintText::calcLayout(void) {
    //Clear the old layout.
    m_lines.clear();
    //Make sure the version number changes.
    m_layoutVersion++;
    //Make sure we don't call this again unless we need to.
    m_needLayout = false;
    if (m_text.empty()) {
        return;
    }
    //Scaling factors.
    m_verticalScaling = m_font.verticalScaling();
    m_horizontalScaling = m_font.horizontalScaling();
    //Max line width in character reference space.
    static float
            font_width_hack = XMLSupport::parse_float(vs_config->getVariable("graphics", "font_width_hack", "0.925"));
    const float maxLineWidth = m_rect.size.width * font_width_hack / m_horizontalScaling;
    //The temporary global state for the layout operation.
    //Make sure this gets initialized at the beginning of an operation.
    m_layout = LayoutState(1.0, 1.0);
    //Keep track of switches in fonts/colors.
    m_layout.fontStack.push_back(m_font);
    m_layout.colorStack.push_back(m_color);
    //Create the current line.
    m_lines.reserve(LINES_RESERVE);
    m_lines.resize(1);
    TextLine *currentLine = &m_lines.back();
    if (m_widthExceeded != MULTI_LINE) {
        //SINGLE LINE.
        currentLine->height = m_rect.size.height;
        currentLine->baseLine = (currentLine->height - m_font.size()) / 2.0
                + m_verticalScaling * REFERENCE_FONT_ASCENDER;

        string::size_type ignorePos = 0;
        bool ellipsis = (m_widthExceeded == ELLIPSIS);
        parseFragmentsWithCharBreak(*currentLine, 0, m_text.size(), maxLineWidth, ellipsis, &ignorePos);

        //Need line width before we can set this.
        currentLine->x = lineInset(*currentLine);
        //If we got no fragments, get rid of the line.
        if (currentLine->fragments.empty()) {
            m_lines.pop_back();
        }
    } else {
        //MULTIPLE LINES.
        int nextLinePos = 0;         //The char loc in m_text of the beginning of the next line.
        while (true) {
            //Figure vertical measurements before we parse the line.
            currentLine->height = m_layout.fontStack.back().size() * m_layout.currentLineSpacing;
            currentLine->baseLine = currentLine->height - m_verticalScaling * REFERENCE_BASELINE_POS;

            //Get the first line of chars, including the length.
            string::size_type endNextLinePos = 0;
            m_layout.currentLineSpacing =
                    BOGUS_LINE_SPACING;             //Line spacing for this line only. Bogus value.
            parseFragmentsWithWordBreak(*currentLine, nextLinePos, maxLineWidth, &endNextLinePos);
            if (m_layout.currentLineSpacing == BOGUS_LINE_SPACING) {
                //We found no format command for current line spacing.  Use permanent spacing.
                //Need to set this *after* we get the line spacing for this line.
                m_layout.currentLineSpacing = m_layout.permanentLineSpacing;
            }
            //Horizontal starting position.
            currentLine->x = lineInset(*currentLine);
            //See if we're done.
            if (endNextLinePos >= m_text.size()) {
                //EXIT FROM LOOP
                break;
            }
            //Start of next line.
            nextLinePos = endNextLinePos;
            m_lines.resize(m_lines.size() + 1);             //Add a new TextLine.
            currentLine = &m_lines.back();
        }
    }
}

//Pass in RGB values, get out a color command string for those values.
std::string colorsToCommandString(float r, float g, float b, float a) {
    char buf[256];
    if (a >= 1.0) {
        //Three-color string.
        sprintf(buf, "#c%.3g:%.3g:%.3g#", r, g, b);
    } else {
        //Four-color string.
        sprintf(buf, "#c%.3g:%.3g:%.3g:%.3g#", r, g, b, a);
    }
    return buf;
}

//CONSTRUCTION
PaintText::PaintText(void) :
        m_rect(),
        m_text(),
        m_color(GUI_OPAQUE_BLACK()),
        m_font(),
        m_justification(RIGHT_JUSTIFY),
        m_widthExceeded(ELLIPSIS),
        m_needLayout(true),
        m_layoutVersion(0),
        m_verticalScaling(0.7),
        m_horizontalScaling(0.7) {
}

PaintText::PaintText(const Rect &r, const std::string &t, const Font &f, const GFXColor &c, Justification j,
        WidthExceeded w) :
        m_rect(r),
        m_text(),             //Don't set text here.
        m_color(c),
        m_font(f),
        m_justification(j),
        m_widthExceeded(w),
        m_needLayout(true),
        m_layoutVersion(0),
        m_verticalScaling(0.7),
        m_horizontalScaling(0.7) {
    setText(t);     //Do conversion if necessary.
}
