/*
 * markup.cpp
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
#include "vega_draw/markup.h"

namespace vega_draw {
namespace {

// Decode one hex digit ('0'-'9', 'a'-'f', 'A'-'F').
bool hex_nibble(char c, std::uint8_t &out) {
    if (c >= '0' && c <= '9') {
        out = static_cast<std::uint8_t>(c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        out = static_cast<std::uint8_t>(c - 'a' + 10);
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        out = static_cast<std::uint8_t>(c - 'A' + 10);
        return true;
    }
    return false;
}

// Parse "RRGGBB" or "RRGGBBAA" (already separated from the "color=" prefix).
bool parse_hex_color(const std::string &hex, Color &out) {
    if (hex.size() != 6 && hex.size() != 8) {
        return false;
    }
    std::uint8_t nibbles[8];
    for (std::size_t i = 0; i < hex.size(); ++i) {
        if (!hex_nibble(hex[i], nibbles[i])) {
            return false;
        }
    }
    out.r = static_cast<std::uint8_t>(nibbles[0] * 16 + nibbles[1]);
    out.g = static_cast<std::uint8_t>(nibbles[2] * 16 + nibbles[3]);
    out.b = static_cast<std::uint8_t>(nibbles[4] * 16 + nibbles[5]);
    out.a = (hex.size() == 8) ? static_cast<std::uint8_t>(nibbles[6] * 16 + nibbles[7]) : 255;
    out.set = true;
    return true;
}

} // namespace

TextLines ParseMarkup(const std::string &source) {
    TextLines lines;
    Line line;
    std::string pending;            // literal text accumulated for the current run
    std::vector<Color> color_stack; // nested colour spans; empty = no explicit colour
    int bold_depth = 0;

    auto current_color = [&]() -> Color {
        if (color_stack.empty()) {
            return Color{}; // set == false
        }
        return color_stack.back();
    };

    auto flush_run = [&]() {
        if (pending.empty()) {
            return;
        }
        Run run;
        run.text.swap(pending);
        run.style.weight = (bold_depth > 0) ? kWeightBold : kWeightNormal;
        run.style.color = current_color();
        line.runs.push_back(std::move(run));
    };

    auto hard_break = [&]() {
        flush_run();
        line.manual_break = true;
        lines.push_back(std::move(line));
        line = Line{};
    };

    const std::size_t n = source.size();
    std::size_t i = 0;
    while (i < n) {
        const char c = source[i];
        if (c == '<') {
            const std::size_t close = source.find('>', i + 1);
            if (close != std::string::npos) {
                const std::string tag = source.substr(i + 1, close - i - 1);
                bool handled = true;
                if (tag == "br") {
                    hard_break();
                } else if (tag == "b") {
                    flush_run();
                    ++bold_depth;
                } else if (tag == "/b") {
                    flush_run();
                    if (bold_depth > 0) {
                        --bold_depth;
                    }
                } else if (tag.compare(0, 6, "color=") == 0) {
                    Color col;
                    if (parse_hex_color(tag.substr(6), col)) {
                        flush_run();
                        color_stack.push_back(col);
                    } else {
                        handled = false;
                    }
                } else if (tag == "/color") {
                    flush_run();
                    if (!color_stack.empty()) {
                        color_stack.pop_back();
                    }
                } else {
                    handled = false;
                }
                if (handled) {
                    i = close + 1;
                    continue;
                }
            }
            // Not a recognised tag: treat the '<' as literal text.
            pending += c;
            ++i;
        } else if (c == '&') {
            if (source.compare(i, 4, "&lt;") == 0) {
                pending += '<';
                i += 4;
            } else if (source.compare(i, 4, "&gt;") == 0) {
                pending += '>';
                i += 4;
            } else if (source.compare(i, 5, "&amp;") == 0) {
                pending += '&';
                i += 5;
            } else {
                pending += c;
                ++i;
            }
        } else {
            pending += c;
            ++i;
        }
    }

    flush_run();
    lines.push_back(std::move(line));
    return lines;
}

} // namespace vega_draw
