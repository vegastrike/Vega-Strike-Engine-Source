##
# CMakeLists.txt
#
# Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
# Benjamen R. Meyer, and other Vega Strike contributors
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

IF (CMAKE_SYSTEM_NAME STREQUAL BEOS OR CMAKE_SYSTEM_NAME STREQUAL Windows)
	MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
	MESSAGE(STATUS "Configuring vegasettings for Linux")

	SET(VEGASETTINGS_SOURCES
		src/c/setup.cpp
		src/include/central.cpp
		src/include/display_gtk.cpp
		src/include/display_dialog.cpp
		src/include/file.cpp
		src/include/general.cpp
		../src/common/common.cpp
		)

	ADD_EXECUTABLE(vegasettings ${VEGASETTINGS_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(vegasettings SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vegasettings PRIVATE
            # VS engine headers
            ${Vega_Root_SOURCE_DIR}
            ${Vega_Root_SOURCE_DIR}/engine
            ${Vega_Root_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Root_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Root_BINARY_DIR}
            ${Vega_Root_BINARY_DIR}/src
            ${Vega_Root_BINARY_DIR}/engine
            ${Vega_Root_BINARY_DIR}/engine/src
    )
	SET_TARGET_PROPERTIES(vegasettings PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_PROPERTY(TARGET vegasettings PROPERTY POSITION_INDEPENDENT_CODE TRUE)
	TARGET_COMPILE_DEFINITIONS(vegasettings PUBLIC "BOOST_ALL_DYN_LINK")

	#find GTK3
	FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
	IF(GTK3_FOUND)
		SET(GTK_LIBS ${GTK3_LIBRARIES})
		SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
	ELSE(GTK3_FOUND)
		MESSAGE("Did not find GTK3")
	ENDIF(GTK3_FOUND)

	ADD_DEFINITIONS(${GTK_CFLAGS})

	FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

	INCLUDE_DIRECTORIES(
        ${Vega_Root_SOURCE_DIR}/setup/src/include
        ${Vega_Root_SOURCE_DIR}/src/common
        ${Vega_Root_BINARY_DIR}
        ${GTK3_INCLUDE_DIRS}
		${Boost_INCLUDE_DIRS}
    )

	TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS} ${Boost_LIBRARIES})

	INSTALL(TARGETS vegasettings DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vegasettings)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Darwin)
	MESSAGE(STATUS "Configuring vegasettings for macOS")

	SET(VEGASETTINGS_SOURCES
			src/c/setup.cpp
			src/include/central.cpp
			src/include/display_gtk.cpp
			src/include/display_dialog.cpp
			src/include/file.cpp
			src/include/general.cpp
			../src/common/common.cpp
	)

	ADD_EXECUTABLE(vegasettings MACOSX_BUNDLE ${VEGASETTINGS_SOURCES})
    TARGET_INCLUDE_DIRECTORIES(vegasettings SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vegasettings PRIVATE
            # VS engine headers
            ${Vega_Root_SOURCE_DIR}
            ${Vega_Root_SOURCE_DIR}/engine
            ${Vega_Root_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Root_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Root_BINARY_DIR}
            ${Vega_Root_BINARY_DIR}/src
            ${Vega_Root_BINARY_DIR}/engine
            ${Vega_Root_BINARY_DIR}/engine/src
    )
	SET_TARGET_PROPERTIES(vegasettings PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    SET_PROPERTY(TARGET vegasettings PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    TARGET_COMPILE_DEFINITIONS(vegasettings PUBLIC "BOOST_ALL_DYN_LINK")

	#find GTK3
	FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
	IF(GTK3_FOUND)
		SET(GTK_LIBS ${GTK3_LIBRARIES})
		SET(GTK_CFLAGS ${GTK3_DEFINITIONS} -DGTK)
	ELSE(GTK3_FOUND)
		MESSAGE("Did not find GTK3")
	ENDIF(GTK3_FOUND)

	ADD_DEFINITIONS(${GTK_CFLAGS})

	FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

	INCLUDE_DIRECTORIES(
			${Vega_Strike_SOURCE_DIR}/setup/src/include
			${Vega_Strike_SOURCE_DIR}/src/common
			${Vega_Root_BINARY_DIR}
			${Vega_Root_BINARY_DIR}/setup/src/include
			${GTK3_INCLUDE_DIRS}
			${Boost_INCLUDE_DIRS}
			${GTK3_GLIBCONFIG_INCLUDE_DIR}
			${CMAKE_PREFIX_PATH}/lib/glib-2.0/include
	)

	TARGET_LINK_LIBRARIES(vegasettings ${GTK_LIBS} ${Boost_LIBRARIES})

	INSTALL(TARGETS vegasettings DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT vegasettings BUNDLE DESTINATION "/Applications/VegaStrike")
ELSE ()
	MESSAGE ("SKIPPING vegasettings - not supported on this platform at this time")
ENDIF ()
