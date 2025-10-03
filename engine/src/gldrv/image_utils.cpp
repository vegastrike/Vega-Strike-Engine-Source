/*
 * image_utils.cpp
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

#include <iostream>
#include <SDL2/SDL_image.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "image_utils.h"

#include "imgui/imgui.h"
#include "vs_logging.h"

// Globals
std::vector<ImageData> splash_images;

ImageData::ImageData(): texture_id(0), width(0), height(0) {}
ImageData::ImageData(GLuint texture_id, int width, int height): 
    texture_id(texture_id), width(width), height(height) {}

// Helper: Load PNG/JPEG via SDL_image and upload to GL texture
ImageData LoadTexture(const std::string& filename)
{
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        VS_LOG(error, (boost::format("Failed to load %1%: %2%\n") % filename % IMG_GetError()).str());
        return ImageData();
    }

    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(surface);

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formatted->w, formatted->h,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, formatted->pixels);

    ImageData image_data(texture_id, formatted->w, formatted->h);
    SDL_FreeSurface(formatted);
    return image_data;
}

void LoadSpashTextures() {
    const std::string backgrounds_folder = "animations/splash_screen";
    for (auto const& entry : boost::filesystem::directory_iterator("animations/splash_screen")) {
        std::cout << entry.path().relative_path() << std::endl;
        const std::string relative_path = entry.path().relative_path().generic_string();
        ImageData image_data = LoadTexture(relative_path);
        if(image_data.texture_id) {
            splash_images.push_back(image_data);
        }
    }
}

void DisplayTexture(int index) {
    ImageData image_data = splash_images[index];
    GLuint texture_id = image_data.texture_id;
    int width = image_data.width;
    int height = image_data.height;
    ImGui::Image((ImTextureID)(intptr_t)texture_id, ImVec2(width, height));
}

void DeleteTextures(std::vector<unsigned int> textures) {
    for(auto& texture : textures) {
        glDeleteTextures(1, &texture);
    }
    textures.clear();
}
