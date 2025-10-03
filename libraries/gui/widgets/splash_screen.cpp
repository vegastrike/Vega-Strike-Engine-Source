/*
 * splash_screen.cpp
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
#include <iostream>
#include <SDL2/SDL.h>
#include <boost/filesystem.hpp>



#include "collections.h"
#include "gui.h"

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdlrenderer2.h"

void (*displayTexturePtr)(int) = nullptr;

static const ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse;   // makes it transparent

static ColorCollection colors;

void RenderSplashScreen(const std::string message, int width, int height) {
    if(!gui_initialized) {
        return;
    }

    // This is used to generate a random name for the ImGui window,
    // in order to prevent collisions.
    const std::string* message_ptr = &message;
    const std::string window_name = std::to_string((unsigned long long)message_ptr);

    // SDL_Event e;
    // while (SDL_PollEvent(&e)) {
    //     ImGui_ImplSDL2_ProcessEvent(&e);
    // }
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
      
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    //ImGui::SetNextWindowBgAlpha(0.6f); // slightly transparent
    ImGui::SetNextWindowSize(ImVec2(width,height), ImGuiCond_Always);

    ImGui::Begin(window_name.c_str(), nullptr, window_flags);

    //ImGui::PushFont(fonts[2]);

    ImGui::Text("%s", message.c_str());
    if(displayTexturePtr) {
        displayTexturePtr(0);
    }
    
    //ImGui::PopFont();
    ImGui::End();
        
    // Rendering
    ImGui::Render();
    // SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    // SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    // SDL_RenderClear(renderer);
    //renderBackgroundImage(renderer, current_window, background_texture);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(current_window);       
}

