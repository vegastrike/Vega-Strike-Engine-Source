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
#include <SDL2/SDL.h>

#include "collections.h"

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdlrenderer2.h"


bool gui_initialized = false;
SDL_Window* current_window = nullptr;
ImFont* roboto_18_font;

void InitGui() {
    current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext gl_context = SDL_GL_GetCurrentContext();

    assert(current_window);

    ImGui::CreateContext();
    
    ImGui_ImplSDL2_InitForOpenGL(current_window, gl_context);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();
    // The game manages its own cursor (changeCursor/hideCursor/showCursor: arrow in
    // bases, crosshair in glide mouse, hidden otherwise). By default the ImGui SDL2
    // backend forces the OS cursor on/off every frame based on ImGui::GetMouseCursor(),
    // which overrides hideCursor and re-shows the arrow in flight. Stop ImGui from
    // changing the OS cursor so the game's cursor model wins.
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.Fonts->Clear();
    ImFontConfig cfg;
    cfg.SizePixels = 18.0f;
    io.FontDefault = io.Fonts->AddFontDefault(&cfg);

    gui_initialized = true;
}

// A pending font-size change, applied synchronously at the NEXT frame start (see
// ImGui_ApplyPendingFontSize) rather than during the settings Save, so the font
// texture isn't destroyed/rebuilt mid-frame (which segfaults the GL HUD).
static float s_pending_font_size = 0.0f;

// Request that the ImGui font atlas be rebuilt at the given pixel size on the
// next frame. Used by the settings screen font-point control.
void RequestImGuiFontSize(float fontSize) {
    if (fontSize <= 0.0f) return;
    s_pending_font_size = fontSize;
}

// Apply a pending font-size change by rebuilding the font atlas. Call at the
// start of each ImGui frame, before ImGui::NewFrame(), so the new atlas is in
// place for the whole frame and no glyphs reference a destroyed texture.
void ImGui_ApplyPendingFontSize() {
    if (s_pending_font_size <= 0.0f) {
        return;
    }
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    io.Fonts->Clear();
    ImFontConfig cfg;
    cfg.SizePixels = s_pending_font_size;
    io.FontDefault = io.Fonts->AddFontDefault(&cfg);
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    // The glyph size is set by ImFontConfig.SizePixels above (0.10.x ImGui has
    // no FontSizeBase member), so text renders at the new size via the atlas.
    s_pending_font_size = 0.0f;
}

void CleanupGui() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    gui_initialized = false;
}

