/*
 * text_layout.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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
#include "vega_draw/text_layout.h"

#include <cstddef>

namespace vega_draw {
namespace {

bool is_space_char(char c) {
    return c == ' ' || c == '\t';
}

// A word or a run of spaces, carrying the style of the run it came from.
struct Token {
    std::string text;
    Style style;
    bool is_space = false;
    float width = 0.0f;
};

// Flatten a parsed line's runs into alternating word/space tokens (for wrapping).
std::vector<Token> Flatten(const Line &line, float font_px, const TextMeasurer &measurer) {
    std::vector<Token> tokens;
    for (std::size_t r = 0; r < line.runs.size(); ++r) {
        const Run &run = line.runs[r];
        const std::string &text = run.text;
        std::size_t i = 0;
        while (i < text.size()) {
            const bool space = is_space_char(text[i]);
            std::size_t j = i;
            while (j < text.size() && is_space_char(text[j]) == space) {
                ++j;
            }
            Token token;
            token.text = text.substr(i, j - i);
            token.style = run.style;
            token.is_space = space;
            token.width = measurer.Measure(token.text, font_px).width;
            tokens.push_back(std::move(token));
            i = j;
        }
    }
    return tokens;
}

void append_tokens(LaidOutLine &line, const std::vector<Token> &tokens, std::size_t begin, std::size_t end) {
    float x = 0.0f;
    for (std::size_t k = begin; k < end; ++k) {
        const Token &token = tokens[k];
        if (token.text.empty()) {
            continue;
        }
        LaidOutRun run;
        run.text = token.text;
        run.style = token.style;
        run.x = x;
        run.width = token.width;
        line.runs.push_back(std::move(run));
        x += token.width;
    }
    line.width = x;
}

// Build one line directly from the parsed runs (no wrapping; preserves runs 1:1).
LaidOutLine layout_unwrapped(const Line &line, float font_px, const TextMeasurer &measurer) {
    LaidOutLine out;
    float x = 0.0f;
    for (std::size_t i = 0; i < line.runs.size(); ++i) {
        const Run &run = line.runs[i];
        if (run.text.empty()) {
            continue;
        }
        LaidOutRun placed;
        placed.text = run.text;
        placed.style = run.style;
        placed.x = x;
        placed.width = measurer.Measure(run.text, font_px).width;
        x += placed.width;
        out.runs.push_back(std::move(placed));
    }
    out.width = x;
    return out;
}

float justification_factor(Justification j) {
    switch (j) {
        case Justification::Center:
            return 0.5f;
        case Justification::Right:
            return 1.0f;
        case Justification::Left:
        default:
            return 0.0f;
    }
}

} // namespace

TextLayout LayoutText(const TextLines &parsed, const TextStyle &style, const TextMeasurer &measurer) {
    TextLayout layout;
    const float line_height = measurer.Measure("", style.font_px).height;
    const bool do_wrap = style.wrap && style.wrap_width_px > 0.0f;
    const float factor = justification_factor(style.justification);

    float y = 0.0f;
    for (std::size_t li = 0; li < parsed.size(); ++li) {
        std::vector<LaidOutLine> physical;
        if (!do_wrap) {
            physical.push_back(layout_unwrapped(parsed[li], style.font_px, measurer));
        } else {
            const std::vector<Token> tokens = Flatten(parsed[li], style.font_px, measurer);
            std::size_t begin = 0;
            while (begin < tokens.size()) {
                while (begin < tokens.size() && tokens[begin].is_space) {
                    ++begin; // never start a wrapped line with spaces
                }
                if (begin >= tokens.size()) {
                    break;
                }
                std::size_t end = begin;
                std::size_t last_content = begin;
                float width = 0.0f;
                while (end < tokens.size()) {
                    const float w = tokens[end].width;
                    if (width + w > style.wrap_width_px && end > begin) {
                        break;
                    }
                    width += w;
                    if (!tokens[end].is_space) {
                        last_content = end + 1;
                    }
                    ++end;
                }
                const std::size_t emit_end = (last_content > begin) ? last_content : end;
                LaidOutLine line;
                append_tokens(line, tokens, begin, emit_end);
                physical.push_back(std::move(line));
                begin = end;
            }
            if (physical.empty()) {
                physical.push_back(LaidOutLine{}); // an empty parsed line still occupies a line
            }
        }

        for (std::size_t p = 0; p < physical.size(); ++p) {
            LaidOutLine &line = physical[p];
            line.height = line_height;
            line.y = y;
            if (factor > 0.0f && style.region_width_px > line.width) {
                const float shift = (style.region_width_px - line.width) * factor;
                for (std::size_t k = 0; k < line.runs.size(); ++k) {
                    line.runs[k].x += shift;
                }
            }
            if (line.width > layout.width) {
                layout.width = line.width;
            }
            y += line.height;
            layout.lines.push_back(std::move(line));
        }
    }

    layout.height = y;
    return layout;
}

} // namespace vega_draw
