/*
 * text_layout.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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

namespace {

// Byte length of the UTF-8 sequence starting with `lead` (1 on an invalid lead).
std::size_t utf8_sequence_length(unsigned char lead) {
    if (lead < 0x80) {
        return 1;
    }
    if ((lead >> 5) == 0x6) {
        return 2;
    }
    if ((lead >> 4) == 0xE) {
        return 3;
    }
    if ((lead >> 3) == 0x1E) {
        return 4;
    }
    return 1;
}

} // namespace

TextLayout LayoutText(const TextLines &parsed, const TextStyle &style, const TextMeasurer &measurer) {
    TextLayout layout;
    // Line advance = the font's line height plus any extra spacing; used both for
    // drawing (via line.y) and for fitting/visible-line counts.
    const float line_height = measurer.Measure("", style.font_px).height * (1.0f + style.line_spacing);
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

void TruncateLineWithEllipsis(LaidOutLine &line,
                              float max_width,
                              float font_px,
                              const TextMeasurer &measurer,
                              const std::string &ellipsis) {
    if (line.width <= max_width) {
        return;
    }
    const float ellipsis_width = measurer.Measure(ellipsis, font_px).width;

    std::vector<LaidOutRun> kept_runs;
    float width = 0.0f;
    bool truncated = false;
    Style last_style;
    bool have_style = false;

    for (std::size_t r = 0; r < line.runs.size() && !truncated; ++r) {
        const LaidOutRun &run = line.runs[r];
        last_style = run.style;
        have_style = true;
        const float run_x = width;
        std::string kept;
        std::size_t i = 0;
        while (i < run.text.size()) {
            const unsigned char lead = static_cast<unsigned char>(run.text[i]);
            std::size_t len = utf8_sequence_length(lead);
            if (i + len > run.text.size()) {
                len = 1;
            }
            const std::string ch = run.text.substr(i, len);
            const float ch_width = measurer.Measure(ch, font_px).width;
            if (width + ch_width + ellipsis_width > max_width) {
                truncated = true;
                break;
            }
            kept += ch;
            width += ch_width;
            i += len;
        }
        if (!kept.empty()) {
            LaidOutRun placed;
            placed.text = kept;
            placed.style = run.style;
            placed.x = run_x;
            placed.width = width - run_x;
            kept_runs.push_back(std::move(placed));
        }
    }

    if (truncated && have_style) {
        LaidOutRun dotdotdot;
        dotdotdot.text = ellipsis;
        dotdotdot.style = last_style;
        dotdotdot.x = width;
        dotdotdot.width = ellipsis_width;
        kept_runs.push_back(std::move(dotdotdot));
        width += ellipsis_width;
    }

    line.runs.swap(kept_runs);
    line.width = width;
}

} // namespace vega_draw
