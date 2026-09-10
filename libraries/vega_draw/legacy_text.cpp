/*
 * legacy_text.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file.
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
#include "vega_draw/legacy_text.h"

#include <cstdlib>
#include <string>
#include <vector>

namespace vega_draw {
namespace {

bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int hex_digit_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return c - 'A' + 10;
}

// Legacy colour components are 0..1 floats; convert to a byte. The old
// ImGuiText path went through ImGui's colour conversion, which rounds, so we
// round here rather than truncate.
std::uint8_t to_byte(float value) {
    if (value <= 0.0f) {
        return 0;
    }
    if (value >= 1.0f) {
        return 255;
    }
    return static_cast<std::uint8_t>(value * 255.0f + 0.5f);
}

// Read one legacy numeric token: digits with an optional decimal separator
// ('.' or ',', since the original parser tolerated a comma for localisation).
// On success advances pos past the token and returns true; with no digits pos
// is left unchanged and it returns false.
bool read_number(const std::string &source, std::size_t &pos, float &out) {
    const std::size_t start = pos;
    std::string token;
    while (pos < source.size()) {
        const char c = source[pos];
        if ((c >= '0' && c <= '9') || c == '.') {
            token += c;
            ++pos;
        } else if (c == ',') {
            token += '.';
            ++pos;
        } else {
            break;
        }
    }
    if (pos == start) {
        return false;
    }
    out = static_cast<float>(std::strtod(token.c_str(), nullptr));
    return true;
}

bool consume(const std::string &source, std::size_t &pos, char expected) {
    if (pos < source.size() && source[pos] == expected) {
        ++pos;
        return true;
    }
    return false;
}

// True when the character after '#' begins a legacy format code.
bool is_format_code(char c) {
    return c == 'n' || c == 'l' || c == 'b' || c == 'c' || c == '-' || c == '!';
}

} // namespace

TextLines ParseLegacyVegaText(const std::string &source, LegacyTextDialect dialect) {
    TextLines lines;
    Line line;
    std::string pending;

    std::vector<Color> color_stack; // empty == no explicit colour (the default)
    std::vector<float> weight_stack;
    weight_stack.push_back(kWeightNormal);
    float permanent_line_spacing = 0.0f;

    const bool is_text_plane = (dialect == LegacyTextDialect::TextPlane);

    auto current_color = [&]() -> Color {
        if (color_stack.empty()) {
            return Color{};
        }
        return color_stack.back();
    };

    auto flush_run = [&]() {
        if (pending.empty()) {
            return;
        }
        Run run;
        run.text.swap(pending);
        run.style.weight = weight_stack.back();
        run.style.color = current_color();
        line.runs.push_back(std::move(run));
    };

    auto end_line = [&](bool manual, float spacing) {
        flush_run();
        line.manual_break = manual;
        line.line_spacing = spacing;
        lines.push_back(std::move(line));
        line = Line{};
        if (is_text_plane) {
            // The legacy TextPlane parser reset the colour at every line break.
            color_stack.clear();
        }
    };

    const std::size_t n = source.size();
    std::size_t i = 0;
    while (i < n) {
        const char c = source[i];

        if (c == '#') {
            // "##" is a literal '#'.
            if (i + 1 < n && source[i + 1] == '#') {
                pending += '#';
                i += 2;
                continue;
            }

            // "#RRGGBB" hex colour, checked before the format codes so a six-hex
            // sequence such as "#b0b0b0" or "#cccccc" is a colour and not a
            // stroke/colour code.
            if (i + 7 <= n) {
                bool hex = true;
                for (std::size_t k = 1; k <= 6; ++k) {
                    if (!is_hex_digit(source[i + k])) {
                        hex = false;
                        break;
                    }
                }
                if (hex) {
                    flush_run();
                    const int r = hex_digit_value(source[i + 1]) * 16 + hex_digit_value(source[i + 2]);
                    const int g = hex_digit_value(source[i + 3]) * 16 + hex_digit_value(source[i + 4]);
                    const int b = hex_digit_value(source[i + 5]) * 16 + hex_digit_value(source[i + 6]);
                    color_stack.clear();
                    if (!(r == 0 && g == 0 && b == 0)) {
                        // "#000000" resets to the default colour, not literal black.
                        Color color;
                        color.r = static_cast<std::uint8_t>(r);
                        color.g = static_cast<std::uint8_t>(g);
                        color.b = static_cast<std::uint8_t>(b);
                        color.a = 255;
                        color.set = true;
                        color_stack.push_back(color);
                    }
                    i += 7;
                    continue;
                }
            }

            if (i + 1 >= n || !is_format_code(source[i + 1])) {
                pending += '#';
                ++i;
                continue;
            }

            const char code = source[i + 1];
            std::size_t pos = i + 2;

            if (code == 'n') {
                float spacing = permanent_line_spacing;
                float value = 0.0f;
                if (read_number(source, pos, value)) {
                    spacing = value;
                }
                consume(source, pos, '#');
                end_line(true, spacing);
                i = pos;
            } else if (code == 'l') {
                float value = 0.0f;
                if (read_number(source, pos, value)) {
                    permanent_line_spacing = value;
                }
                consume(source, pos, '#');
                i = pos;
            } else if (code == 'b') {
                float value = 0.0f;
                const bool has_value = read_number(source, pos, value);
                consume(source, pos, '#');
                flush_run();
                weight_stack.push_back(has_value ? value : kWeightBold);
                i = pos;
            } else if (code == 'c') {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;
                float a = 1.0f;
                std::size_t color_pos = pos;
                const bool ok = read_number(source, color_pos, r) && consume(source, color_pos, ':')
                        && read_number(source, color_pos, g) && consume(source, color_pos, ':')
                        && read_number(source, color_pos, b);
                if (ok && color_pos < n && source[color_pos] == ':') {
                    ++color_pos;
                    read_number(source, color_pos, a);
                }
                if (ok) {
                    consume(source, color_pos, '#');
                    Color color;
                    color.r = to_byte(r);
                    color.g = to_byte(g);
                    color.b = to_byte(b);
                    color.a = to_byte(a);
                    color.set = true;
                    flush_run();
                    color_stack.push_back(color);
                    i = color_pos;
                } else {
                    // Malformed colour code: keep the '#' as literal text and
                    // let the rest of the sequence be parsed as ordinary text.
                    pending += '#';
                    ++i;
                }
            } else if (code == '-') {
                flush_run();
                if (pos < n) {
                    if (source[pos] == 'b' && weight_stack.size() > 1) {
                        weight_stack.pop_back();
                    } else if (source[pos] == 'c' && !color_stack.empty()) {
                        color_stack.pop_back();
                    }
                    ++pos;
                }
                i = pos;
            } else { // '!'
                flush_run();
                if (pos < n) {
                    if (source[pos] == 'b') {
                        weight_stack.resize(1);
                    } else if (source[pos] == 'c') {
                        color_stack.clear();
                    }
                    ++pos;
                }
                i = pos;
            }
            continue;
        }

        if (c == '\n') {
            end_line(true, permanent_line_spacing);
            ++i;
            continue;
        }

        if (!is_text_plane && c == '\\') {
            // The legacy parser treated a backslash (and the character after it)
            // as a line break -- this is how "\n" inside content becomes a break.
            end_line(true, permanent_line_spacing);
            i += (i + 1 < n) ? 2 : 1;
            continue;
        }

        if (is_text_plane && c == '_') {
            pending += ' ';
            ++i;
            continue;
        }

        pending += c;
        ++i;
    }

    flush_run();
    if (!line.runs.empty() || lines.empty()) {
        line.manual_break = false;
        line.line_spacing = permanent_line_spacing;
        lines.push_back(std::move(line));
    }
    return lines;
}

} // namespace vega_draw
