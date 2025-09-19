##
# CMakeLists.txt
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
#

MESSAGE(STATUS "PROCESSING DIRECTORY: ${CMAKE_CURRENT_SOURCE_DIR}")

#Find SDL_image
FIND_PACKAGE(SDL2_IMAGE REQUIRED)
IF (SDL2_IMAGE_FOUND)
    MESSAGE(STATUS "SDL2_image Found")
ELSE (SDL2_IMAGE_FOUND)
    MESSAGE(FATAL_ERROR "Failed to find required library SDL2_image")
ENDIF (SDL2_IMAGE_FOUND)

ADD_LIBRARY(vegastrike_imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    
    backends/imgui_impl_sdl2.cpp
    backends/imgui_impl_sdlrenderer2.cpp
)

 TARGET_LINK_LIBRARIES(vegastrike_imgui PRIVATE SDL2_image::SDL2_image)

SET_PROPERTY(TARGET vegastrike_imgui PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_imgui PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_imgui PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_imgui PROPERTY POSITION_INDEPENDENT_CODE TRUE)


message("libs: ${TST_INCLUDES} ${Vega_Strike_SOURCE_DIR}")

TARGET_INCLUDE_DIRECTORIES(vegastrike_imgui 
    PUBLIC ${Vega_Strike_SOURCE_DIR}/libraries/imgui
    PUBLIC ${Vega_Strike_SOURCE_DIR}/libraries/imgui/backends
    SYSTEM PRIVATE ${TST_INCLUDES})

IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike_imgui PRIVATE m)
ENDIF()



