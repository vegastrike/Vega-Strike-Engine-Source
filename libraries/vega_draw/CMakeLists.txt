##
# CMakeLists.txt
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2026 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen M. Meyer, Stephen G. Tuggy
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

# vega_draw — the unified 2D drawing/text library (design-drawing-coordinate-model).
# Sources are added module by module (text primitive -> text box -> picker).
ADD_LIBRARY(vegastrike_vega_draw STATIC
    markup.cpp
    markup.h
    geometry.h
    text_layout.cpp
    text_layout.h
    text_box.cpp
    text_box.h
    picker.cpp
    picker.h
    scroller.cpp
    scroller.h
    widget.h
    container.cpp
    button.cpp
    button.h
    panel.cpp
    panel.h
    event_manager.cpp
    event_manager.h
    units.h
    imgui_backend.cpp
    imgui_backend.h
)

SET_PROPERTY(TARGET vegastrike_vega_draw PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_vega_draw PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_vega_draw PROPERTY CXX_EXTENSIONS ON)
SET_PROPERTY(TARGET vegastrike_vega_draw PROPERTY POSITION_INDEPENDENT_CODE TRUE)

TARGET_INCLUDE_DIRECTORIES(vegastrike_vega_draw
    PUBLIC
        ${Vega_Strike_SOURCE_DIR}/libraries
)

# The ImGui adapter lives in this library; consumers of it need ImGui too.
TARGET_LINK_LIBRARIES(vegastrike_vega_draw PUBLIC vegastrike_imgui)

IF (USE_GTEST)
    SET(TEST_NAME ${PROJECT_NAME}_vega_draw_tests)
    ADD_EXECUTABLE(
        ${TEST_NAME}
        tests/geometry_tests.cpp
        tests/markup_tests.cpp
        tests/text_layout_tests.cpp
        tests/text_box_tests.cpp
        tests/picker_tests.cpp
        tests/scroller_tests.cpp
        tests/units_tests.cpp
        tests/widget_tests.cpp
        tests/button_tests.cpp
        tests/event_manager_tests.cpp
    )
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} SYSTEM PRIVATE ${TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} PRIVATE
        ${Vega_Strike_SOURCE_DIR}/libraries
    )
    SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${Vega_Strike_BINARY_DIR}/)

    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 14)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY POSITION_INDEPENDENT_CODE TRUE)

    TARGET_LINK_LIBRARIES(
        ${TEST_NAME}
        gtest_main
        vegastrike_vega_draw
    )

    INCLUDE(GoogleTest)
    gtest_discover_tests(${TEST_NAME} PROPERTIES DISCOVERY_TIMEOUT 30)
ENDIF (USE_GTEST)
