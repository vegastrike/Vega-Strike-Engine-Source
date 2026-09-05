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
#include "gui/imguitext.h"
#include "src/vs_logging.h"
#include "configuration/configuration.h"
#include "imgui_internal.h"

void FormattedLayout::endLine(Line& line) {
    // if (!line.empty()) {
        line.lineSpacing = currentLineSpacing;
        this->push_back(line);
    // }
    line.clear();
    line.width = 0.0f;
    currentLineSpacing = line.lineSpacing = 0.0f;
}

// UI functions

// Draws (one or more lines of) text to the UI
void ImGuiText::draw(int firstLineToDraw) {
    parseTextIfNeeded();
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return; // Safe exit: No active window context
    }
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if ( !draw_list ) return;
    // const float lineHeight = ImGui::CalcTextSize("Hg").y; // this should give us the the full height of a text line
    // Calculate the Pixel Rect for Clipping
    float pMinX = Coordinates::normToPixelX(m_rect.left());
    float pMinY = Coordinates::normToPixelY(m_rect.top()) - 5; // Top of rect
    float pMaxX = Coordinates::normToPixelX(m_rect.right());
    float pMaxY = Coordinates::normToPixelY(m_rect.bottom()) + 10; // Bottom of rect

    // Clipping coords to avoid overrunning text
    ImVec4 clipRect(pMinX, pMinY, pMaxX, pMaxY);

    float pixelX = Coordinates::normToPixelX(m_rect.left());
    // position text in middle of rect for single line text
    float pixelY = m_multiLine ? Coordinates::normToPixelY(m_rect.top()) : Coordinates::normToPixelY((m_rect.top() + m_rect.bottom()) *0.5f);
    float pixelWidth = Coordinates::normToPixelW(m_rect.size.width);

    ImVec2 textSize = getTextWidth(m_text.c_str(), m_font.size());
    // position single-line text half a line down so it is perfectly centered
    if(!m_multiLine) {
        pixelY -= (textSize.y * 0.5f);
    }
    // ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a));
    
    float currentY = pixelY;
    for (size_t i = 0; i < m_layout.size(); ++i) {
        // Skip lines that are above the scroll position
        if (i < (size_t)firstLineToDraw) {
            continue;
        }

        // Stop if the line is going to be drawn below the bottom of the clipping rect
        if (currentY > Coordinates::normToPixelY(m_rect.origin.y)) {
            break;
        }
        const auto& line = m_layout[i];
        float currentX = pixelX;
        if (m_justification == CENTER_JUSTIFY && (pixelWidth - line.width) > 0) { 
            currentX += (pixelWidth - line.width) * 0.5f;
        }
        
        for (const auto& frag : line) {
            // Glyph height comes from the user-set Text Height (font_point); no
            // resolution-derived font-height calculation here.
            float pixelFontSize = configuration().graphics.font_point_flt;
            // Draw Bold "shadow"
            if (frag.isBold || m_font.strokeWeight() == BOLD_STROKE) {
                draw_list->AddText(nullptr, pixelFontSize, 
                    ImVec2(currentX, currentY + 2.0f), frag.color, frag.text.c_str(), nullptr, 0.0f, &clipRect);
            }

            // Draw Main Text
            draw_list->AddText(nullptr, pixelFontSize, 
                ImVec2(currentX, currentY), frag.color, frag.text.c_str(), nullptr, 0.0f, &clipRect);
            
            currentX += frag.width; // Move pen right
        }
        currentY += line.lineHeight + (line.lineSpacing * line.lineHeight); // Move pen down
    }
}

// Helper to convert 2 hex chars to an integer (0-255)
static int ParseHexByte(const char* hex) {
    int val = 0;
    for (int i = 0; i < 2; ++i) {
        char c = hex[i];
        val <<= 4;
        if (c >= '0' && c <= '9') val |= (c - '0');
        else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
    }
    return val;
}

void DrawVegaStrikeRichText(const std::string& text, ImVec4 defaultColor) {
    const char* s = text.c_str();
    const char* end = s + text.length();
    int colorPushes = 0;
    bool firstChunk = true;

    ImGui::PushStyleColor(ImGuiCol_Text, defaultColor);
    colorPushes++;

    while (s < end) {
        const char* hash = strchr(s, '#');
        
        if (hash) {
            // 1. Draw any text that comes BEFORE the '#'
            if (hash > s) {
                if (!firstChunk) ImGui::SameLine(0.0f, 0.0f);
                ImGui::TextUnformatted(s, hash);
                firstChunk = false;
            }

            // 2. Do we have a complete 7-character color code?
            if (hash + 7 <= end) {
                int r = ParseHexByte(hash + 1);
                int g = ParseHexByte(hash + 3);
                int b = ParseHexByte(hash + 5);

                if (r == 0 && g == 0 && b == 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, defaultColor);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(r, g, b, 255));
                }
                colorPushes++;

                // Skip the parsed color code
                s = hash + 7; 
            } 
            // 3. Incomplete code: Typewriter effect check
            else {
                bool isPartialHex = true;
                for (const char* c = hash + 1; c < end; ++c) {
                    if (!std::isxdigit(static_cast<unsigned char>(*c))) {
                        isPartialHex = false;
                        break;
                    }
                }

                if (isPartialHex) {
                    // It's a partial color code. Stop drawing and hide it 
                    // until the remaining characters type out in future frames.
                    break; 
                } else {
                    // It's just a normal '#' followed by non-hex text (e.g., "Ship #4")
                    if (!firstChunk) ImGui::SameLine(0.0f, 0.0f);
                    ImGui::TextUnformatted(hash, end);
                    break;
                }
            }
        } else {
            // No more color codes, draw the remainder of the string
            if (!firstChunk) ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(s, end);
            break;
        }
    }

    ImGui::PopStyleColor(colorPushes);
}

// Text processing

// Set the text and parse
void ImGuiText::setText(const std::string& text) {
    if(text != m_text) {
        m_layout.needsProcessing = true;
    }
    m_text = text; 
}

// this is a little complicated and only needed because text and multiline can be set separately, 
// but both is needed in order to decide how to process the text
void ImGuiText::parseTextIfNeeded() {
    // only process the text upon change
    if(m_layout.needsProcessing) {
        VS_LOG(debug, (boost::format("Raw text set for GUI control: %1%") % m_text));
        m_layout = parseText(m_text, Coordinates::normToPixelW(m_rect.size.width)); // Parser runs only when text changes
        m_layoutVersion++; // Text has changed, outside logic uses this information to update GUI state
    }
    m_layout.needsProcessing = false;
}

int ImGuiText::visibleLineCountStartingWith(int lineNumber, float vertInterval) {
    parseTextIfNeeded();
    int result = 0;
    float currentHeight = Coordinates::normToPixelH(vertInterval)*0.95; // Have a little safety margin, otherwise the last line looks sometimes cut

    // 2. Iterate through m_layout starting from lineNumber
    for (size_t i = lineNumber; i < m_layout.size(); ++i) {
        const auto& line = m_layout[i];
        // If the remaining space is less than our line height, we are done
        if (currentHeight - line.lineHeight < 0.0f) {
            break;
        }
        
        currentHeight -= line.lineHeight;
        result++;
    }
    return result;
}

ImVec2 ImGuiText::getTextWidth(const std::string text, const float fontSize) {
    // fontSize is a normalized font size (fraction of screen height). Measure the
    // text at the SAME pixel size the draw() path renders its glyphs at
    // (Coordinates::normToPixelFontSize), so layout (wrap width, line height,
    // centering, scroll counts) agrees with what is actually drawn. This replaces
    // the old hand-rolled font_point/resolution-relative scaleFactor, which was a
    // second, incompatible size convention that drifted from rendering.
    // Measure at the same user-set glyph height (font_point) that draw() renders at.
    const float pixelFontSize = configuration().graphics.font_point_flt;
    ImFont* font = ImGui::GetFont();
    if (font && font->IsLoaded()) {
        return font->CalcTextSizeA(pixelFontSize, FLT_MAX, -1.0f, text.c_str());
    }
    // No font bound yet (measuring outside a render window): fall back to the
    // default-font measure rather than the per-size one.
    return ImGui::CalcTextSize(text.c_str());
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
            //Found a bad character. Return carriage. Stop.
            curPos -= 2;
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
                        m_layout.currentLineSpacing = value;
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
                    m_layout.permanentLineSpacing = value;
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
                    }
                } else if (input[curPos] == DT_FORMAT_COLOR_CHAR) {
                    if (m_colorStack.size() > 1) {
                        m_colorStack.pop_back();
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
        ImVec2 dimensions = getTextWidth(text,  m_fontStack.back().size());
        currentLine.push_back({text, m_fontStack.back(), m_colorStack.back(), m_fontStack.back().strokeWeight() == BOLD_STROKE, dimensions.x});
        currentLine.width += dimensions.x;
        // update lineheight if fragment is larger
        currentLine.lineHeight = dimensions.y > currentLine.lineHeight ? dimensions.y : currentLine.lineHeight;
    };

    // Initialise stacks for top level color and font weight
    m_colorStack.push_back(ImGui::ColorConvertFloat4ToU32(ImVec4(m_color.r, m_color.g, m_color.b, m_color.a)));
    m_fontStack.push_back(m_font);
    size_t curPos = 0;
    size_t lastWordBreakPos = 0;
    size_t endPos = input.size();
    size_t fragmentStartPos = 0;
    while (curPos < endPos) {
        // consume all non-control chars until control char is hit

        // first let's check if it is a format char
        if(input[curPos] == DT_FORMAT_CHAR) {
            // save previous text as fragment, avoid empty fragements
            if(curPos > fragmentStartPos) {
                addFragment(input.substr(fragmentStartPos, curPos - fragmentStartPos));
            }
            // Process format
            curPos++; // Move past '#'
            if (curPos >= endPos) break;

            if (input[curPos] == DT_FORMAT_CHAR) {
                // Handle double hash "##" -> treat as a single "#"
                // std::string charStr(1, input[curPos]);
                // currentLine.push_back({charStr, m_fontStack.back(), m_colorStack.back(), m_fontStack.back().strokeWeight() == BOLD_STROKE, ImGui::CalcTextSize(charStr.c_str()).x});
                // currentLine.width += ImGui::CalcTextSize(charStr.c_str()).x;
                fragmentStartPos = curPos;
                continue;
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
            if(input[curPos] == ' ') {
                // we'll skip the space
                lastWordBreakPos = curPos + 1;
            }

            // special news handling
            switch (input[curPos]) {
                case '\\':
                    addFragment(input.substr(fragmentStartPos, curPos - fragmentStartPos));
                    fragmentStartPos = curPos + 2;
                    layout.endLine(currentLine);
                    // we skipt the \\, +1 here and +1 at the end of the loop
                    curPos++;
                    break;
                case '\n':
                    // create a new line and continue
                    addFragment(input.substr(fragmentStartPos, curPos - fragmentStartPos));
                    fragmentStartPos = curPos + 1;
                    layout.endLine(currentLine);
                    break;
                default:
                    // line too long?
                    std::string currentFragmentText = input.substr(fragmentStartPos, curPos - fragmentStartPos);
                    // for single-line text, add ellipsis to the end of the line
                    if(!m_multiLine) {
                        currentFragmentText += ELLIPSIS_STRING;
                    }
                    if(currentLine.width + getTextWidth(currentFragmentText,  m_fontStack.back().size()).x > widthInPixels) {
                        // break at last word break
                        addFragment(input.substr(fragmentStartPos, lastWordBreakPos - fragmentStartPos) + (m_multiLine ? "" : ELLIPSIS_STRING));
                        fragmentStartPos = lastWordBreakPos;
                        layout.endLine(currentLine);
                    }
                    break; 
            };
            if(!m_multiLine && layout.size() > 0) {
                break;
            }

            curPos++;
        }

        // when we hit the end, add what we got
        if(curPos >= endPos && fragmentStartPos < endPos) {
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
//     ImVec2 pMin(Coordinates::normToPixelX(r.origin.x), Coordinates::normToPixelY(r.origin.y + r.size.height));
//     ImVec2 pMax(Coordinates::normToPixelX(r.origin.x + r.size.width), Coordinates::normToPixelY(r.origin.y));

//     // Convert color
//     ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));

//     // Draw the rectangle
//     ImGui::GetBackgroundDrawList()->AddRect(pMin, pMax, col, 0.0f, 0, 2.0f);
// }