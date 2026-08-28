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
#include "vsdraw/imguitext.h"
#include "src/vs_logging.h"
#include "configuration/configuration.h"
#include "imgui_internal.h"

#include <cmath>  // std::round
#include <cctype>  // std::isxdigit

// Whether a packed ImU32 color is fully transparent (alpha == 0).
static bool isTransparent(ImU32 color) {
    return ((color >> IM_COL32_A_SHIFT) & 0xFF) == 0;
}

// Normalized glyph height, driven by configuration font_point (relocated from the
// removed TextPlane implementation so other code can keep linking against it).
float getFontHeight() {
    return configuration().graphics.font_point_flt / configuration().graphics.resolution_y;
}

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
    float pMinX = Coordinates::normToPixelX(m_rect.left(), resW());
    float pMinY = Coordinates::normToPixelY(m_rect.top(), resH()) - 5; // Top of rect
    float pMaxX = Coordinates::normToPixelX(m_rect.right(), resW());
    float pMaxY = Coordinates::normToPixelY(m_rect.bottom(), resH()) + 10; // Bottom of rect

    // Clipping coords to avoid overrunning text
    ImVec4 clipRect(pMinX, pMinY, pMaxX, pMaxY);

    float pixelX = Coordinates::normToPixelX(m_rect.left(), resW());
    // position text in middle of rect for single line text
    float pixelY = m_multiLine ? Coordinates::normToPixelY(m_rect.top(), resH()) : Coordinates::normToPixelY((m_rect.top() + m_rect.bottom()) *0.5f, resH());
    float pixelWidth = Coordinates::normToPixelW(m_rect.size.width, resW());

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
        if (currentY > Coordinates::normToPixelY(m_rect.origin.y, resH())) {
            break;
        }
        const auto& line = m_layout[i];
        float currentX = pixelX;
        if (m_justification == CENTER_JUSTIFY && (pixelWidth - line.width) > 0) { 
            currentX += (pixelWidth - line.width) * 0.5f;
        }
        
        for (const auto& frag : line) {
            // Font::size() IS the pixel glyph height (font_point-relative). Round to a
            // whole pixel so GetFontBaked bakes at exactly this size and RenderText's
            // rescale is 1.0 (crisp, sharp). No resolution-relative scaling here.
            float pixelFontSize = std::round(frag.font.size());
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

// A run of text sharing one color, used by the TextPlane-compatible Draw() path
// (which measures with raw ImGui font metrics, not ImGuiText's scaled layout).
struct TextPlaneRun {
    std::string text;
    ImU32 color;
};

// Parse a #cR:G:B[:A]# color argument (each float in 0..1) into a packed ImU32.
static ImU32 parseColorU32(const std::string &spec) {
    float r = 1, g = 1, b = 1, a = 1;
    std::vector<float> comps;
    std::string cur;
    for (char c : spec) {
        if (c == ':') {
            comps.push_back(static_cast<float>(atof(cur.c_str())));
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) comps.push_back(static_cast<float>(atof(cur.c_str())));
    if (comps.size() > 0) r = comps[0];
    if (comps.size() > 1) g = comps[1];
    if (comps.size() > 2) b = comps[2];
    if (comps.size() > 3) a = comps[3];
    return IM_COL32(int(r * 255), int(g * 255), int(b * 255), int(a * 255));
}

// Draw one complete line of color runs at the given pen position and advance the pen
// down one line with the raw glyph height.  Returns the height used (for line spacing).
static float drawLine(const std::vector<TextPlaneRun> &runs, ImVec2 &pen, ImDrawList *draw_list,
        ImU32 background_color, bool drawBg, float textScale) {
    // Render this line's text at textScale * the current font size (the base-room
    // streaming word-by-word text sets textScale=2.0; everything else is 1.0).
    // Measure with the same scaled size so the pen advance and line height stay
    // consistent with what is actually drawn.
    ImFont *font = ImGui::GetFont();
    const float font_size = ImGui::GetFontSize();
    const float scale = textScale;
    const float draw_size = font_size * scale;

    auto measure = [&](const std::string &s) -> ImVec2 {
        if (font && font->IsLoaded())
            return font->CalcTextSizeA(draw_size, FLT_MAX, -1.0f, s.c_str());
        ImVec2 r = ImGui::CalcTextSize(s.c_str());
        return ImVec2(r.x * scale, r.y * scale);
    };

    float lineHeight = measure("hello world").y;
    for (const auto &run : runs) {
        ImVec2 sz = measure(run.text);
        if (sz.y > lineHeight) lineHeight = sz.y;
        // Draw the background and the text at the word's own left edge (word_pen), NOT
        // offset behind the previous word (the old code drew bg at pen.x - pad.x, which
        // overlapped the trailing pixels of the previous word and clipped it).
        ImVec2 word_pen = pen;
        if (drawBg) {
            // Very dark, semi-transparent background rect behind the word.
            draw_list->AddRectFilled(word_pen,
                    ImVec2(word_pen.x + sz.x, word_pen.y + sz.y), background_color, 0.0f);
        }
        draw_list->AddText(nullptr, draw_size, word_pen, run.color, run.text.c_str(), nullptr, 0.0f, nullptr);
        pen.x += sz.x;
    }
    pen.y += lineHeight;
    return lineHeight;
}

// Forward declaration: parse 2 hex chars into a 0-255 byte (defined below).
static int ParseHexByte(const char* hex);

// TextPlane-compatible drawing path.  Replicates the legacy TextPlane::Draw semantics
// (top-left anchor, optional one-line-up for !start_lower, per-run background rectangle
// gated by automatte).  It keeps TextPlane's raw-ImGui-font rendering behaviour: glyphs
// are drawn at the default ImGui font size and lines advance/wrap by the *unscaled*
// CalcTextSize metrics (ImGuiText::getTextWidth applies a scaleFactor meant only for the
// native GUI draw() path; using it here would inflate line spacing).  Format codes
// consumed: #cR:G:B[:A]# pushes a color, #-c pops back to the default color.  Newlines
// are honoured; a long line wraps at the rect width.  '_' is kept literal (the old
// TextPlane turned it into a space, which broke long '_'-separated names).
int ImGuiText::Draw(const std::string &newText, int offset, bool start_lower,
        bool force_highquality, bool automatte) {
    if (ImGui::GetCurrentContext() == nullptr || ImGui::GetCurrentWindowRead() == nullptr) {
        return 1;
    }
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    if ( !draw_list ) return 1;

    // Top-left anchor (TextPlane passed its pos through CalculateAbsoluteXY, which is
    // identical to normToPixelX/Y for a -1..1 normalized coordinate). The letterbox
    // offset (m_offX/Y) is added because this draws on the screen-absolute background
    // draw list, so base text must land inside the base's letterboxed window.
    ImVec2 position;
    position.x = Coordinates::normToPixelX(m_rect.origin.x, resW()) + m_offX;
    position.y = Coordinates::normToPixelY(m_rect.origin.y, resH()) + m_offY;

    // Move one line up if !start_lower (as TextPlane did).
    if (!start_lower) {
        position.y -= ImGui::CalcTextSize("hello world").y;
    }

    // Word-wrap width in normalized units: a fraction of the full screen width
    // (1.0 == full width). Callers specify normalized coords; the library converts
    // to pixels for measurement. No fudge factor: text must not exceed the box, so
    // it cannot spill into a neighbouring element.
    const float wrapWidth = m_wrapWidth > 0.0f ? m_wrapWidth : m_rect.size.width;
    const bool doWrap = (wrapWidth > 0.0f);
    const float leftX = position.x;

    // Measure text at the size it is actually drawn (draw_size = font * textScale) so
    // the wrap decision matches the rendered width. Glyph widths come from ImGui in
    // pixels; convert to normalized units (fraction of screen width) so the comparison
    // stays resolution-independent. The screen width is only the pixel basis.
    ImFont *font = ImGui::GetFont();
    const float draw_size = ImGui::GetFontSize() * m_textScale;
    const float displayW = ImGui::GetIO().DisplaySize.x;
    const float wrapWidthPx = wrapWidth * displayW;   // for ImGui's pixel-based wrap
    auto measure = [&](const std::string &s) -> float {
        float px;
        if (font && font->IsLoaded()) {
            px = font->CalcTextSizeA(draw_size, FLT_MAX, -1.0f, s.c_str()).x;
        } else {
            px = ImGui::CalcTextSize(s.c_str()).x * m_textScale;
        }
        return (displayW > 0.0f) ? px / displayW : 0.0f;
    };

    // Split the text into lines of color runs, wrapping at wrapWidth. Wrapping is
    // word-granular: a line is broken at a space when it would exceed wrapWidth. A
    // word wider than the box is split so it cannot spill horizontally. Lines are
    // collected first so `offset` can skip leading lines.
    std::vector<std::vector<TextPlaneRun>> lines;
    ImU32 currentColor = m_colorU32;
    std::string word;
    float lineWidth = 0.0f;
    auto finishLine = [&]() {
        if (!word.empty()) {
            lines.back().push_back({word, currentColor});
            word.clear();
        }
        lines.emplace_back();
        lineWidth = 0.0f;
    };
    lines.emplace_back();

    // Push the current word (already rendered color) onto the line, wrapping at
    // wrapWidth. A word wider than the box is broken into chunks that fit, using
    // ImGui's wrap algorithm to find the break points.
    auto pushWord = [&](bool hardBreak) {
        if (word.empty()) return;
        const float ww = measure(word);
        const bool tooWide = doWrap && ww > wrapWidth;
        if (doWrap && !tooWide && !hardBreak && lineWidth + ww > wrapWidth) {
            // Wraps mid-line, keeping the word intact on the new line.
            lines.emplace_back();
            lineWidth = 0.0f;
        }
        if (!tooWide) {
            lines.back().push_back({word, currentColor});
            lineWidth += ww;
        } else {
            // Oversized word: split into chunks, each on a fresh line, so nothing
            // spills past the box. ImGui's wrap finds the break points.
            const char *start = word.c_str();
            const char *end = start + word.size();
            while (start < end) {
                if (lineWidth > 0.0f) {   // finish the current line first
                    lines.emplace_back();
                    lineWidth = 0.0f;
                }
                const char *brk = (font && font->IsLoaded())
                        ? font->CalcWordWrapPosition(draw_size, start, end, wrapWidthPx)
                        : end;
                if (brk <= start) {
                    brk = start + 1;   // guard: a single glyph wider than the box
                }
                std::string chunk(start, brk);
                const float cw = measure(chunk);
                lines.back().push_back({chunk, currentColor});
                lineWidth = cw;
                start = brk;
            }
        }
        word.clear();
    };

    const size_t n = newText.size();

    // Readahead: detect a '#' that begins a color token but is truncated by the
    // word-by-word reveal (the token is not fully present in the revealed prefix).
    // A partial token must NOT be rendered as literal text -- Draw() re-parses the
    // whole prefix every frame, so once the reveal exposes the full token it will
    // be recognized as a color command and rendered correctly. The reveal only ever
    // truncates at the tail, so nothing valid follows a partial token (break is safe).
    auto isIncompleteColorToken = [&](size_t idx) -> bool {
        if (newText[idx] != '#') {
            return false;
        }
        if (idx + 1 >= n) {
            return true;   // lone trailing '#': the first char of a not-yet-revealed token
        }
        const char next = newText[idx + 1];
        if (next == 'c') {
            // '#cR:G:B#' form: incomplete until the closing '#' is revealed.
            return newText.find('#', idx + 2) == std::string::npos;
        }
        if (next == '-') {
            // '#-c' reset: incomplete only while the reveal is at a trailing '#-'.
            return idx + 2 >= n;
        }
        if (std::isxdigit(static_cast<unsigned char>(next))) {
            // Legacy '#RRGGBB': incomplete if the hex run hits the end before 6 digits.
            size_t k = idx + 1;
            while (k < n && std::isxdigit(static_cast<unsigned char>(newText[k]))) {
                ++k;
            }
            return (k == n) && (k - (idx + 1)) < 6;
        }
        return false;
    };

    for (size_t i = 0; i < n; ++i) {
        char c = newText[i];
        if (c == '\n') {
            pushWord(true);
            finishLine();
            // Per-line color: each new line starts at the default color, so a
            // #cR:G:B# tag colors only its own line and the content below it
            // reverts to white (e.g. the credits headers vs. the body text).
            currentColor = m_colorU32;
        } else if (c == ' ' && doWrap) {
            pushWord(false);
            lines.back().push_back({" ", currentColor});
            lineWidth += measure(" ");
        } else if (c == '#' && isIncompleteColorToken(i)) {
            // The reveal has cut through a color token: don't render the partial
            // token as literal text. Break -- nothing valid follows it, and the
            // whole token is re-parsed correctly once the reveal completes it.
            break;
        } else if (c == '#' && i + 2 < n && newText[i + 1] == '-' && newText[i + 2] == 'c') {
            pushWord(false);
            currentColor = m_colorU32;
            i += 2;
        } else if (c == '#' && i + 6 < n
                && std::isxdigit(static_cast<unsigned char>(newText[i + 1]))
                && std::isxdigit(static_cast<unsigned char>(newText[i + 2]))
                && std::isxdigit(static_cast<unsigned char>(newText[i + 3]))
                && std::isxdigit(static_cast<unsigned char>(newText[i + 4]))
                && std::isxdigit(static_cast<unsigned char>(newText[i + 5]))
                && std::isxdigit(static_cast<unsigned char>(newText[i + 6]))) {
            // Legacy '#RRGGBB' (6 hex digits, no prefix) sets the color; '#000000'
            // (black) is a RESET to the default color -- matching the removed
            // TextPlane::ParseText semantics. Checked before the '#c' float form so
            // hex colors beginning with 'b' or 'c' (e.g. #cccccc) aren't misparsed.
            pushWord(false);
            const int r = ParseHexByte(&newText[i + 1]);
            const int g = ParseHexByte(&newText[i + 3]);
            const int b = ParseHexByte(&newText[i + 5]);
            if (r == 0 && g == 0 && b == 0) {
                currentColor = m_colorU32;
            } else {
                currentColor = IM_COL32(r, g, b, 255);
            }
            i += 6;
        } else if (c == '#' && i + 1 < n && newText[i + 1] == 'c') {
            const size_t end = newText.find('#', i + 2);
            if (end != std::string::npos) {
                pushWord(false);
                currentColor = parseColorU32(newText.substr(i + 2, end - (i + 2)));
                i = end;
            } else {
                word += c;
            }
        } else {
            word += c;
        }
    }
    pushWord(false);

    // Very dark, semi-transparent word background. Drawn under each word (at the
    // word's own left edge) so it never overlaps the previous word's glyphs.
    const ImU32 dark_bg = IM_COL32(0, 0, 0, 180);   // nearly opaque black (~70% alpha)
    const bool drawBg = (!isTransparent(m_backgroundColor) && !automatte);

    // Draw the lines, skipping `offset` leading lines.
    for (size_t li = 0; li < lines.size(); ++li) {
        if (static_cast<int>(li) < offset) continue;
        drawLine(lines[li], position, draw_list, dark_bg, drawBg, m_textScale);
        position.x = leftX;
    }
    return 1;
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
        m_layout = parseText(m_text, Coordinates::normToPixelW(m_rect.size.width, resW())); // Parser runs only when text changes
        m_layoutVersion++; // Text has changed, outside logic uses this information to update GUI state
    }
    m_layout.needsProcessing = false;
}

int ImGuiText::visibleLineCountStartingWith(int lineNumber, float vertInterval) {
    parseTextIfNeeded();
    int result = 0;
    float currentHeight = Coordinates::normToPixelH(vertInterval, resH())*0.95; // Have a little safety margin, otherwise the last line looks sometimes cut

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
    // fontSize is a real pixel size (Font::size()), matching the draw path. Measure
    // with CalcTextSizeA at that same pixel size so layout matches rendering exactly
    // (no resolution-relative scaleFactor fabrication). The dynamic font atlas bakes
    // the requested size on demand, so this is accurate.
    ImFont *font = ImGui::GetFont();
    if (font && font->IsLoaded()) {
        return font->CalcTextSizeA(fontSize, FLT_MAX, -1.0f, text.c_str());
    }
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
        // Legacy '#RRGGBB' (6 hex digits, no prefix) is checked BEFORE the format
        // switch, so a hex color that begins with 'b' or 'c' (e.g. #cccccc, #b0b0b0)
        // is not captured by the reserved stroke/color codes. A '#000000' (black)
        // tag resets to the default color (bottom of the stack), not literal black.
        if (curPos + 6 <= endPos
                && std::isxdigit(static_cast<unsigned char>(input[curPos]))
                && std::isxdigit(static_cast<unsigned char>(input[curPos + 1]))
                && std::isxdigit(static_cast<unsigned char>(input[curPos + 2]))
                && std::isxdigit(static_cast<unsigned char>(input[curPos + 3]))
                && std::isxdigit(static_cast<unsigned char>(input[curPos + 4]))
                && std::isxdigit(static_cast<unsigned char>(input[curPos + 5]))) {
            const int r = ParseHexByte(&input[curPos]);
            const int g = ParseHexByte(&input[curPos + 2]);
            const int b = ParseHexByte(&input[curPos + 4]);
            if (r == 0 && g == 0 && b == 0) {
                while (m_colorStack.size() > 1) {
                    m_colorStack.pop_back();
                }
            } else {
                m_colorStack.push_back(IM_COL32(r, g, b, 255));
            }
            curPos += 6;
        } else {
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
    m_colorStack.push_back(m_colorU32);
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