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


ADD_LIBRARY(vegastrike_gui STATIC
    clickable_text.cpp
    credits.cpp
    selection_group.cpp
    toggleable_text.cpp
    load_game.cpp
    main_menu.cpp

    layout/layout.cpp

    widgets/label.cpp
    widgets/widget.cpp
)


ADD_EXECUTABLE(test_gui 
               test_gui.cpp) 

SET_PROPERTY(TARGET vegastrike_gui PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_gui PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_gui PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_gui PROPERTY POSITION_INDEPENDENT_CODE TRUE)

# SDL
FIND_PACKAGE(SDL2 REQUIRED)
IF (SDL2_FOUND)
    LIST(APPEND VSE_TST_INCLUDES ${SDL2_INCLUDE_DIRS})
    LIST(APPEND VSE_TST_LIBS ${SDL2_LIBRARIES})
    MESSAGE(STATUS "Found required library SDL2")
ELSE (SDL2_FOUND)
    MESSAGE(FATAL_ERROR "Failed to find required library SDL2")
ENDIF (SDL2_FOUND)

TARGET_INCLUDE_DIRECTORIES(vegastrike_gui 
    PUBLIC 
        ${Vega_Strike_SOURCE_DIR}/libraries/gui
        ${Vega_Strike_SOURCE_DIR}/libraries/gui/layout
        ${Vega_Strike_SOURCE_DIR}/libraries/gui/widgets
    SYSTEM PRIVATE 
        ${Vega_Strike_SOURCE_DIR}/libraries/imgui
        ${SDL2_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES(vegastrike_gui
    ${SDL2_LIBRARIES}
    vegastrike_imgui
    Boost::log
    Boost::log_setup
    Boost::json
)

TARGET_LINK_LIBRARIES(test_gui
    ${SDL2_LIBRARIES}
    vegastrike_imgui
    vegastrike_gui
    Boost::log
    Boost::log_setup
    Boost::json
)
    
