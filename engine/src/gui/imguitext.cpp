/*
 * imguitext.cpp
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
#include "imguitext.h"
#include <sstream>
#include "src/vs_logging.h"
#include "configuration/configuration.h"

// Macros

#define NORM_TO_PIXEL_X(val) (((val) + 1.0f) * 0.5f * ImGui::GetIO().DisplaySize.x)
#define NORM_TO_PIXEL_Y(val) ((1.0f - (val)) * 0.5f * ImGui::GetIO().DisplaySize.y)
#define NORM_TO_PIXEL_W(val) ((val) * 0.5f * ImGui::GetIO().DisplaySize.x)
const double BASE_LINE_FONT_SIZE = 0.05d;

void FormattedLayout::endLine(Line& line) {
    if (!line.empty()) {
        this->push_back(line);
    }
    line.clear();
    line.width = 0.0f;
}

// UI functions

// Draws (a line of) text to the UI
void ImGuiText::draw() {
    // Calculate the Pixel Rect for Clipping
    float pMinX = NORM_TO_PIXEL_X(m_rect.origin.x);
    float pMinY = NORM_TO_PIXEL_Y(m_rect.origin.y + m_rect.size.height) - 3; // Top of rect
    float pMaxX = NORM_TO_PIXEL_X(m_rect.origin.x + m_rect.size.width);
    float pMaxY = NORM_TO_PIXEL_Y(m_rect.origin.y) + 7; // Bottom of rect

    // Clipping coords to avoid overrunning text
    ImVec4 clipRect(pMinX, pMinY, pMaxX, pMaxY);

    float pixelX = NORM_TO_PIXEL_X(m_rect.origin.x);
    // center in cell
    float pixelCenterY = NORM_TO_PIXEL_Y(m_rect.origin.y + (m_rect.size.height * 0.5f));
    float pixelWidth = NORM_TO_PIXEL_W(m_rect.size.width);

    ImVec2 textSize = ImGui::CalcTextSize(m_text.c_str());
    // Move half the text size up
    float pixelY = pixelCenterY - (textSize.y * 0.5f);
    ImVec2 pos(pixelX, pixelY);

    if (m_justification == CENTER_JUSTIFY) { 
        pos.x += (pixelWidth - textSize.x) * 0.5f;
    }

    ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a));
    // const int fontSize = m_font.size()/BASE_LINE_FONT_SIZE*configuration().graphics.font_point_dbl;
    
    // Poor man's bold effect, redraw with 2px offset
    // FIXME Replace with proper bold font
    if(m_font.strokeWeight() == BOLD_STROKE) {
        ImGui::GetBackgroundDrawList()->AddText(nullptr, 0.0f, ImVec2(pos.x, pos.y + 2.0f), color, m_text.c_str(), nullptr, 0.0f, &clipRect);
    }

    // Main text
    ImGui::GetBackgroundDrawList()->AddText(nullptr, 0.0f, pos, color, m_text.c_str(), nullptr, 0.0f, &clipRect);
}

void ImGuiText::drawFormattedMultilineText(int firstLineToDraw) {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return;
    const float lineHeight = ImGui::CalcTextSize("Hg").y; // this should give us the the full height of a text line
    // Calculate the Pixel Rect for Clipping
    float pMinX = NORM_TO_PIXEL_X(m_rect.origin.x);
    float pMinY = NORM_TO_PIXEL_Y(m_rect.origin.y + m_rect.size.height); // Top of rect
    float pMaxX = NORM_TO_PIXEL_X(m_rect.origin.x + m_rect.size.width);
    float pMaxY = NORM_TO_PIXEL_Y(m_rect.origin.y); // Bottom of rect

    // Clipping coords to avoid overrunning text
    ImVec4 clipRect(pMinX, pMinY, pMaxX, pMaxY);

    float pixelX = NORM_TO_PIXEL_X(m_rect.origin.x);
    float pixelY = NORM_TO_PIXEL_Y(m_rect.origin.y + m_rect.size.height);
    float pixelWidth = NORM_TO_PIXEL_W(m_rect.size.width);

    ImVec2 textSize = ImGui::CalcTextSize(m_text.c_str());
    ImVec2 pos(pixelX, pixelY);

    if (m_justification == CENTER_JUSTIFY && (pixelWidth - textSize.x) > 0) { 
        pos.x += (pixelWidth - textSize.x) * 0.5f;
    }

    // ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a));
    
    float currentY = pixelY;
    for (size_t i = 0; i < m_layout.size(); ++i) {
        // Skip lines that are above the scroll position
        if (i < (size_t)firstLineToDraw) {
            continue;
        }

        // Stop if the line is going to be drawn below the bottom of the clipping rect
        if (currentY > NORM_TO_PIXEL_Y(m_rect.origin.y)) {
            break;
        }
        const auto& line = m_layout[i];
        float currentX = pixelX;

        // Add Justification logic here?
        
        for (const auto& frag : line) {
            // const int fontSize = frag.font.size()/BASE_LINE_FONT_SIZE*configuration().graphics.font_point_dbl;

            // Draw Bold "shadow"
            if (frag.isBold || m_font.strokeWeight() == BOLD_STROKE) {
                ImGui::GetBackgroundDrawList()->AddText(nullptr, 0.0f, 
                    ImVec2(currentX, currentY + 2.0f), frag.color, frag.text.c_str(), nullptr, 0.0f, &clipRect);
            }

            // Draw Main Text
            ImGui::GetBackgroundDrawList()->AddText(nullptr, 0.0f, 
                ImVec2(currentX, currentY), frag.color, frag.text.c_str(), nullptr, 0.0f, &clipRect);
            
            currentX += frag.width; // Move pen right
        }
        currentY += lineHeight; // Move pen down
    }
}

int ImGuiText::visibleLineCountStartingWith(int lineNumber, float vertInterval) const {
    int result = 0;
    float currentHeight = vertInterval;
    float lineHeight = ImGui::CalcTextSize("Hg").y; // Use the same height as in drawLines()

    // 2. Iterate through m_layout starting from lineNumber
    for (size_t i = lineNumber; i < m_layout.size(); ++i) {
        // If the remaining space is less than our line height, we are done
        if (currentHeight - lineHeight < 0.0f) {
            break;
        }
        
        currentHeight -= lineHeight;
        result++;
    }
    return result;
}

// Text processing

// Set the text and parse
void ImGuiText::setText(const std::string& text) {
    // only parse upon change
    if( text != m_text) {
        VS_LOG(debug, (boost::format("Raw text set for GUI control: %1%") % text));
        m_layout = parseText(text, NORM_TO_PIXEL_W(m_rect.size.width)); // Parser runs only when text changes
        m_layoutVersion++; // Text has changed, outside logic uses this information to update GUI state
    }
    m_text = text;
}

//Get a floating-point argument for a PaintText format command.
//This will not accept exponential format, just num-plus-decimal.
//The argument is optional, but must be ended with a format char.
//Examples:  #b2.35#, #b#.  #b is not allowed.
void ImGuiText::parseFormatFloat(const std::string &str, //String.
        const size_t startPos, //First character to examine.
        const size_t endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        float *resultValue, //OUT: Parsed value.  If no value, not changed.
        size_t *resultPos, //OUT: One past last format char.
        const char optionalTerminator //Another terminator besides DT_FORMAT_CHAR
) {
    *formatSuccess = false;
    std::string num;
    size_t curPos;
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
void ImGuiText::parseFormatColor(const std::string &str, //String.
        const size_t startPos, //First character to examine.
        const size_t endPos, //One past last char to consider.
        bool *formatSuccess, //OUT: True = It worked.
        GFXColor &color, //OUT: Parsed value.
        size_t *resultPos //OUT: One past last format char.
) {
    *formatSuccess = false;
    size_t curPos = startPos;
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
void ImGuiText::parseFormat(std::string input, size_t startPos, //Location of beginning of string to examine.
        size_t *resultPos, //OUT: Ptr to string past the format string.
        bool *endLine //OUT: True = Done with current line.
) {
    const size_t endPos = input.size();
    //Default return value.
    *endLine = false;
    bool formatSuccess = false;
    size_t curPos = startPos;
    if (curPos < endPos) {
        //Make sure we have some chars to process.
        switch (input[curPos]) {
            case DT_FORMAT_NEWLINE_CHAR:
                //End of line.
            {
                float value = BOGUS_LINE_SPACING;            //Bogus value.
                parseFormatFloat(input, curPos + 1, endPos, &formatSuccess, &value, &curPos);
                if (formatSuccess) {
                    *endLine = true;                 //End of this line.
                    if (value != BOGUS_LINE_SPACING) {
                        // m_layout.currentLineSpacing = value;
                    }
                }
                break;
            }
            case DT_FORMAT_LINE_SPACING_CHAR:
                //New permanent line spacing.
            {
                float value = BOGUS_LINE_SPACING;            //Bogus value.
                parseFormatFloat(input, curPos + 1, endPos, &formatSuccess, &value, &curPos);
                if (formatSuccess && value != BOGUS_LINE_SPACING) {
                    // m_layout.permanentLineSpacing = value;
                }
                break;
            }
            case DT_FORMAT_STROKE_CHAR:
                //"Bold" -- change stroke width of font.
            {
                float strokeWeight = BOLD_STROKE;
                parseFormatFloat(input, curPos + 1, endPos, &formatSuccess, &strokeWeight, &curPos);
                if (formatSuccess) {
                    Font f(m_fontStack.back());             //Make a new font.
                    f.setStrokeWeight(strokeWeight);
                    m_fontStack.push_back(f);
                }
                break;
            }
            case DT_FORMAT_COLOR_CHAR:
                //Change the text color.
            {
                GFXColor color;
                parseFormatColor(input, curPos + 1, endPos, &formatSuccess, color, &curPos);
                if (formatSuccess) {
                    m_colorStack.push_back(ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a)));
                }
                break;
            }
            case DT_FORMAT_POP_CHAR:
                //Pop a color/font.
                curPos++;
                if (curPos == endPos) {
                    *endLine = true;
                } else if (input[curPos] == DT_FORMAT_STROKE_CHAR) {
                    if (m_fontStack.size() > 1) {
                        m_fontStack.pop_back();
                    } else if (input[curPos] == DT_FORMAT_COLOR_CHAR) {
                        if (m_colorStack.size() > 1) {
                            m_colorStack.pop_back();
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
                } else if (input[curPos] == DT_FORMAT_STROKE_CHAR) {
                    while (m_fontStack.size() > 1) {
                        m_fontStack.pop_back();
                    }
                } else if (input[curPos] == DT_FORMAT_COLOR_CHAR) {
                    while (m_colorStack.size() > 1) {
                        m_colorStack.pop_back();
                    }
                }
                curPos++;
                break;
        }
    }
    *resultPos = curPos;
}

FormattedLayout ImGuiText::parseText(const std::string& input, const float widthInPixels) {
    FormattedLayout layout;
    Line currentLine;
    
    // ImU32 currentColor = ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a));
    
    // Helper to add fragment
    auto addFragment = [&](const std::string& text) {
        // if (text.empty()) return;
        const float len = ImGui::CalcTextSize(text.c_str()).x;
        currentLine.push_back({text, m_fontStack.back(), m_colorStack.back(), m_fontStack.back().strokeWeight() == BOLD_STROKE, len});
        currentLine.width += len;
    };

    // Initialise stacks for top level color and font weight
    m_colorStack.push_back(ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a)));
    m_fontStack.push_back(m_font);
    size_t curPos = 0;
    size_t lastWordBreakPos = 0;
    size_t endPos = input.size();
    size_t fragmentStartPos = 0;
    while (curPos < endPos) {
        // consume all non-control chars until 
        const char currentChar = input[curPos];
        std::string currentCharStr(1, input[curPos]);

        // first let's check if it is a format char
        if(currentChar == DT_FORMAT_CHAR) {
            // save previous text as fragment, avoid empty fragements
            if(curPos > fragmentStartPos) {
                addFragment(input.substr(fragmentStartPos, curPos - fragmentStartPos));
            }
            // Process format
            curPos++; // Move past '#'
            if (curPos >= endPos) break;

            if (input[curPos] == DT_FORMAT_CHAR) {
                // Handle double hash "##" -> treat as a single "#"
                std::string charStr(1, input[curPos]);
                currentLine.push_back({charStr, m_fontStack.back(), m_colorStack.back(), m_fontStack.back().strokeWeight() == BOLD_STROKE, ImGui::CalcTextSize(charStr.c_str()).x});
                currentLine.width += ImGui::CalcTextSize(charStr.c_str()).x;
                curPos++;
            } else {
                bool forceEndLine = false;
                
                // keep consuming all subsequent control sequences
                size_t oldPos = 0;
                // consume all subsequent control sequences, position will remain the same when no more found
                while(oldPos < curPos){
                    oldPos = curPos;
                    parseFormat(input, curPos, &curPos, &forceEndLine);
                    if (forceEndLine) {
                        layout.endLine(currentLine);
                        break;
                    }
                }
            }
            // consider control chars are word breaks
            lastWordBreakPos = curPos;
            fragmentStartPos = curPos;
        } else {
            // this consumes all chars but control chars and breakes when the line is full
            if(currentChar == ' ') {
                // we'll skip the space
                lastWordBreakPos = curPos + 1;
            }

            // special news handling
            if(currentChar == '\\') {
                addFragment(input.substr(fragmentStartPos, curPos - fragmentStartPos));
                fragmentStartPos = curPos + 2;
                layout.endLine(currentLine);
                // we skipt the \\, +1 here and +1 at the end of the loop
                curPos++;
            } else
            // line too long?
            if(currentLine.width + ImGui::CalcTextSize(input.substr(fragmentStartPos, curPos - fragmentStartPos).c_str()).x > widthInPixels) {
                // break at last word break
                addFragment(input.substr(fragmentStartPos, lastWordBreakPos - fragmentStartPos));
                fragmentStartPos = lastWordBreakPos;
                layout.endLine(currentLine);
            } 

            curPos++;
        }

        // when we hit the end, add what we got
        if(curPos >= endPos) {
            addFragment(input.substr(fragmentStartPos));
        }

    }
    if (!currentLine.empty()) layout.push_back(currentLine);
    m_colorStack.clear();
    m_fontStack.clear();
    return layout;
}

// Draws a red rectangle for (degugging purposes)
// void ImGuiText::drawDebugRect(const Rect &r, const GFXColor &color) {
//     ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    
//     // Calculate top-left and bottom-right for ImGui
//     ImVec2 pMin(NORM_TO_PIXEL_X(r.origin.x), NORM_TO_PIXEL_Y(r.origin.y + r.size.height));
//     ImVec2 pMax(NORM_TO_PIXEL_X(r.origin.x + r.size.width), NORM_TO_PIXEL_Y(r.origin.y));

//     // Convert color
//     ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));

//     // Draw the rectangle
//     ImGui::GetBackgroundDrawList()->AddRect(pMin, pMax, col, 0.0f, 0, 2.0f);
// }