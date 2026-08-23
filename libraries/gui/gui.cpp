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


bool gui_initialized = false;
SDL_Window* current_window = nullptr;
ImFont* roboto_18_font;

// Font size requested by a settings change, applied on the next frame. Stored as a
// float so we can defer the atlas rebuild to a safe point (start of a frame, before
// any text is laid out), avoiding mid-frame texture destruction.
static float s_pending_font_size = 0.0f;

void InitGui(SDL_Window *window, const SDL_GLContext *context, const float fontSize) {
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
    // Bake the glyphs at higher detail than the requested render size so the base
    // computer (which renders each control at font_point * scale, scale up to ~1.5)
    // scales DOWN from a high-detail rasterization rather than UP from a single
    // low-res bake.  Scaling a crisp high-detail bake is sharp; scaling up a 1x bake
    // (the default) is blurry.  RasterizerDensity is the legacy-backend (locked atlas)
    // knob for this -- it does not alter font metrics, only the rasterization detail.
    cfg.SizePixels = fontSize;
    cfg.RasterizerDensity = 2.0f;  // bake at 2x detail; covers base control scales up to ~2x
    io.FontDefault = io.Fonts->AddFontDefault(&cfg);

    gui_initialized = true;
}

void RequestImGuiFontSize(float fontSize) {
    s_pending_font_size = fontSize;
}

// Apply a pending font-size change by rebuilding the ImGui font atlas. Call at the
// start of each frame, before ImGui::NewFrame(), so the new atlas is in place for
// the whole frame and no glyphs reference a destroyed texture.
void ImGui_ApplyPendingFontSize() {
    if (s_pending_font_size <= 0.0f) {
        return;
    }
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    io.Fonts->Clear();
    ImFontConfig cfg;
    cfg.SizePixels = s_pending_font_size;
    cfg.RasterizerDensity = 2.0f;  // keep the 2x-detail bake on rebuild (see InitGui)
    io.FontDefault = io.Fonts->AddFontDefault(&cfg);
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    // FontSizeBase (the render size, distinct from the atlas rasterization size) is
    // only derived from the font's LegacySize on the first frame and then cached, so
    // a rebuild alone would change the glyph rasterization but NOT the on-screen size
    // (blurry text at the old size). Set it explicitly so the render size follows.
    ImGui::GetStyle().FontSizeBase = s_pending_font_size;
    s_pending_font_size = 0.0f;
}

void CleanupGui() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    gui_initialized = false;
}

