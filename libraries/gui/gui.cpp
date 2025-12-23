/*
 * gui.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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

    roboto_18_font = io.Fonts->AddFontFromFileTTF(
        "fonts/RobotoTTF/Roboto-Regular.ttf", // path to .ttf
        18.0f                       // pixel size
    );

    gui_initialized = true;
}

void CleanupGui() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    gui_initialized = false;
}

