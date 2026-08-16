/*
 * gui.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "gui.h"
#include "fonts.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"


bool gui_initialized = false;
SDL_Window* current_window = nullptr;
GuiFonts gui_Fonts;
const float defaultNormTextSize = 0.03F; // 1.5% of the screen height

void InitGui(SDL_Window *window, const SDL_GLContext *context, const std::string& fontFilePath, const float fontSize2) {
    current_window = window;
    SDL_GLContext gl_context = *context;
    ImFont* default_font = nullptr;

    assert(current_window);

    ImGui::CreateContext();
    
    ImGui_ImplSDL3_InitForOpenGL(current_window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig cfg;
    cfg.OversampleH = 3; // 3x horizontal oversampling for smooth edges
    cfg.OversampleV = 2; // 2x vertical oversampling
    cfg.PixelSnapH = true; // Align glyphs to whole pixels to prevent blurriness
    // cfg.GlyphOffset.y = -1.5F;

    // Calculate pixel font size from normalized text size and screen height
    // The math replicates Coordinates::normToPixelFontSize(Font.getSize())
    int pixelHeight = 0;
    SDL_GetWindowSizeInPixels(window, nullptr, &pixelHeight);
    const float fontSize = defaultNormTextSize * 0.5F * static_cast<float>(pixelHeight);

    if (!fontFilePath.empty()) {
        default_font = io.Fonts->AddFontFromFileTTF(fontFilePath.c_str(), fontSize, &cfg);
        gui_Fonts.Regular = default_font;
    }

    // Fallback if the TTF file path is invalid or missing, this shoud not normally happen as we ship a default font
    if (default_font == nullptr) {
        cfg.SizePixels = fontSize;
        default_font = io.Fonts->AddFontDefault();
        gui_Fonts.Regular  = default_font;
        gui_Fonts.Bold     = default_font;
        gui_Fonts.SemiBold = default_font;
        gui_Fonts.Italic   = default_font;
    } else {
        // We found the regular font, now load the weight/style variants safely
        std::string boldPath     = boost::algorithm::replace_first_copy(fontFilePath, "-Regular", "-Bold");
        std::string semiBoldPath = boost::algorithm::replace_first_copy(fontFilePath, "-Regular", "-SemiBold");
        std::string italicPath   = boost::algorithm::replace_first_copy(fontFilePath, "-Regular", "-Italic");

        auto loadVariantOrFallback = [&](const std::string& path, const char* variantName) -> ImFont* {
            if (boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)) {
                ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize, &cfg);
                if (font != nullptr) {
                    return font;
                }
            }
            // VS_LOG(warn, (boost::format("Font variant '%1%' not found at path: %2%! Falling back to Regular font.") 
            //              % variantName % path));
            return gui_Fonts.Regular;
        };

        gui_Fonts.Bold     = loadVariantOrFallback(boldPath, "Bold");
        gui_Fonts.SemiBold = loadVariantOrFallback(semiBoldPath, "SemiBold");
        gui_Fonts.Italic   = loadVariantOrFallback(italicPath, "Italic");
    }
    
    io.FontDefault = default_font;

    gui_initialized = true;
}

void CleanupGui() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    gui_initialized = false;
}

