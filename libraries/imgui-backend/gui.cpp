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

#include <cassert>
#include <string>
#include <SDL3/SDL.h>

#include "collections.h"

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

// TrueType font (Roboto-Medium) embedded as a compressed base85 array so the engine
// needs no font file on disk at runtime (no data-dir/path dependency). This is how
// Dear ImGui itself embeds its default ProggyClean.ttf. Loaded via the dynamic font
// atlas, which bakes each requested glyph size on demand (RendererHasTextures).
#include "roboto_font.h"


bool gui_initialized = false;
SDL_Window* current_window = nullptr;

// Font size requested by a settings change, applied on the next frame. Stored as a
// float so we can defer the atlas rebuild to a safe point (start of a frame, before
// any text is laid out), avoiding mid-frame texture destruction.
static float s_pending_font_size = 0.0f;

// Font file requested by a settings change, applied on the next frame. Empty means
// "use the embedded Roboto". When s_font_file_pending is set, the atlas is rebuilt
// with the new .ttf.
static std::string s_pending_font_file;
static bool s_font_file_pending = false;

void InitGui(SDL_Window *window, const SDL_GLContext *context, const float fontSize, const char *fontFile) {
    current_window = window;
    SDL_GLContext gl_context = *context;

    assert(current_window);

    ImGui::CreateContext();
    
    ImGui_ImplSDL3_InitForOpenGL(current_window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig cfg;
    cfg.SizePixels = fontSize;
    // Load the configured .ttf font file if provided (a path into the data fonts/
    // directory); otherwise fall back to the embedded Roboto TrueType. With the
    // synced OpenGL3 backend (RendererHasTextures) the atlas is dynamic: each glyph
    // size is baked crisp on demand, so no RasterizerDensity workaround is needed.
    io.FontDefault = (fontFile != nullptr && fontFile[0] != '\0')
            ? io.Fonts->AddFontFromFileTTF(fontFile, fontSize, &cfg) : nullptr;
    if (io.FontDefault == nullptr) {
        io.FontDefault = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
            RobotoMediumFont_compressed_data_base85, fontSize, &cfg);
    }

    gui_initialized = true;
}

void RequestImGuiFontSize(float fontSize) {
    s_pending_font_size = fontSize;
}

void RequestImGuiFont(const char *fontFile) {
    s_pending_font_file = (fontFile != nullptr) ? fontFile : "";
    s_font_file_pending = true;
}

// Apply a pending font-size / font change. A font-size change only needs FontSizeBase
// (the dynamic atlas bakes glyphs on demand); a font FACE change requires a real
// atlas rebuild (clear fonts, add the new .ttf or Roboto, rebuild the texture).
// Call at the start of each frame, before ImGui::NewFrame().
void ImGui_ApplyPendingFontSize() {
    if (s_pending_font_size <= 0.0f && !s_font_file_pending) {
        return;
    }
    if (s_pending_font_size > 0.0f) {
        ImGui::GetStyle().FontSizeBase = s_pending_font_size;
        s_pending_font_size = 0.0f;
    }
    if (s_font_file_pending) {
        ImGuiIO &io = ImGui::GetIO();
        io.Fonts->Clear();
        ImFontConfig cfg;
        cfg.SizePixels = ImGui::GetStyle().FontSizeBase;
        ImFont *f = (!s_pending_font_file.empty())
                ? io.Fonts->AddFontFromFileTTF(s_pending_font_file.c_str(), cfg.SizePixels, &cfg)
                : nullptr;
        if (f == nullptr) {
            f = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
                RobotoMediumFont_compressed_data_base85, cfg.SizePixels, &cfg);
        }
        io.FontDefault = f;
        io.Fonts->Build();   // the dynamic-atlas backend updates the texture on NewFrame
        s_font_file_pending = false;
        s_pending_font_file.clear();
    }
}

void CleanupGui() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    gui_initialized = false;
}

