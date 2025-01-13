##
# FindGTK3.cmake
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Creator: Daniel Horn
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
# - FindGTK3.cmake
# This module can find the GTK3 widget libraries and several of its other
# optional components like gtkmm, glade, and glademm.
#
# NOTE: If you intend to use version checking, CMake 2.6.2 or later is
# required.
#
# Specify one or more of the following components
# as you call this find module. See example below.
#
#   gtk
#   gtkmm
#   glade
#   glademm
#
# The following variables will be defined for your use
#
#   GTK3_FOUND - Were all of your specified components found?
#   GTK3_INCLUDE_DIRS - All include directories
#   GTK3_LIBRARIES - All libraries
#
#   GTK3_VERSION - The version of GTK3 found (x.y.z)
#   GTK3_MAJOR_VERSION - The major version of GTK3
#   GTK3_MINOR_VERSION - The minor version of GTK3
#   GTK3_PATCH_VERSION - The patch version of GTK3
#
# Optional variables you can define prior to calling this module:
#
#   GTK3_DEBUG - Enables verbose debugging of the module
#   GTK3_SKIP_MARK_AS_ADVANCED - Disable marking cache variables as advanced
#   GTK3_ADDITIONAL_SUFFIXES - Allows defining additional directories to
#                              search for include files
#
#=================
# Example Usage:
#
#   Call FIND_PACKAGE() once, here are some examples to pick from:
#
#   Require GTK 3.0 or later
#       FIND_PACKAGE(GTK3 3.0 REQUIRED gtk)
#
#   IF(GTK3_FOUND)
#      INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
#      ADD_EXECUTABLE(mygui mygui.cc)
#      TARGET_LINK_LIBRARIES(mygui ${GTK3_LIBRARIES})
#   ENDIF()
#

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2008-2009 Philip Lowman <philip@yhbt.com>
# Copyright 2014-2018 Ettercap Development Team <info@ettercap-project.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


# Version 0.1 (5/13/2011)
#   * First cut at a GTK3 version (Heavily derived from
#     FindGTK2.cmake)

# Version 0.2 (3/02/2018)
#   * Run git diff against this file to see all changes
#=============================================================
# _GTK3_GET_VERSION
# Internal function to parse the version number in gtkversion.h
#   _OUT_major = Major version number
#   _OUT_minor = Minor version number
#   _OUT_micro = Micro version number
#   _gtkversion_hdr = Header file to parse
#=============================================================
FUNCTION(_GTK3_GET_VERSION _OUT_major _OUT_minor _OUT_micro _gtkversion_hdr)
    FILE(READ ${_gtkversion_hdr} _contents)
    IF(_contents)
        STRING(REGEX REPLACE
            ".*#define GTK_MAJOR_VERSION[ \t]+\\(([0-9]+)\\).*"
            "\\1" ${_OUT_major} "${_contents}"
            )

        STRING(REGEX REPLACE
            ".*#define GTK_MINOR_VERSION[ \t]+\\(([0-9]+)\\).*"
            "\\1" ${_OUT_minor} "${_contents}"
            )

        STRING(REGEX REPLACE
            ".*#define GTK_MICRO_VERSION[ \t]+\\(([0-9]+)\\).*"
            "\\1" ${_OUT_micro} "${_contents}"
            )
        IF(NOT ${_OUT_major} MATCHES "[0-9]+")
            MESSAGE(FATAL_ERROR "Version parsing failed for GTK3_MAJOR_VERSION!")
        ENDIF()
        IF(NOT ${_OUT_minor} MATCHES "[0-9]+")
            MESSAGE(FATAL_ERROR "Version parsing failed for GTK3_MINOR_VERSION!")
        ENDIF()
        IF(NOT ${_OUT_micro} MATCHES "[0-9]+")
            MESSAGE(FATAL_ERROR "Version parsing failed for GTK3_MICRO_VERSION!")
        ENDIF()

        SET(${_OUT_major} ${${_OUT_major}} PARENT_SCOPE)
        SET(${_OUT_minor} ${${_OUT_minor}} PARENT_SCOPE)
        SET(${_OUT_micro} ${${_OUT_micro}} PARENT_SCOPE)
    ELSE()
        MESSAGE(FATAL_ERROR "Include file ${_gtkversion_hdr} does not exist")
    ENDIF()
ENDFUNCTION()

#=============================================================
# _GTK3_FIND_INCLUDE_DIR
# Internal function to find the GTK include directories
#   _var = variable to set
#   _hdr = header file to look for
#=============================================================
FUNCTION(_GTK3_FIND_INCLUDE_DIR _var _hdr)

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
            "_gtk3_find_include_dir( ${_var} ${_hdr} )")
    ENDIF()

    SET(_relatives
        # If these ever change, things will break.
        ${GTK3_ADDITIONAL_SUFFIXES}
        glibmm-2.0
        glib-2.0
        atk-1.0
        atkmm-1.0
        cairo
        cairomm-1.0
        gdk-pixbuf-2.0
        gdkmm-2.4
        giomm-2.4
        gtk-3.0
        gtkmm-2.4
        libglade-2.0
        libglademm-2.4
        harfbuzz
        pango-1.0
        pangomm-1.4
        sigc++-2.2
        gtk-unix-print-2.0)

    SET(_suffixes)
    FOREACH(_d ${_relatives})
        LIST(APPEND _suffixes ${_d})
        LIST(APPEND _suffixes ${_d}/include) # for /usr/lib/gtk-2.0/include
    ENDFOREACH()

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
            "include suffixes = ${_suffixes}")
    ENDIF()

    FIND_PATH(${_var} ${_hdr}
        PATHS
        # On Windows, glibconfig.h is located
        # under $PREFIX/lib/glib-2.0/include.
        C:/GTK/lib/glib-2.0/include
        C:/msys64/$ENV{MSYSTEM}/lib/glib-2.0
        # end
        /usr/local/lib64
        /usr/local/lib
        # fix for Ubuntu == 11.04 (Natty Narwhal)
        /usr/lib/i386-linux-gnu/
        /usr/lib/x86_64-linux-gnu/
        # end
        # fix for Ubuntu >= 11.10 (Oneiric Ocelot)
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        # end
        /usr/lib64
        /usr
        /opt/gnome
        /usr/openwin
        /sw
        /opt/local
        /opt/local/include
        /opt/local/lib/glib-2.0/include
        $ENV{GTKMM_BASEPATH}
        [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
        PATH_SUFFIXES
        ${_suffixes} include lib
        )

    IF(${_var})
        SET(GTK3_INCLUDE_DIRS ${GTK3_INCLUDE_DIRS} ${${_var}} PARENT_SCOPE)
        IF(NOT GTK3_SKIP_MARK_AS_ADVANCED)
            MARK_AS_ADVANCED(${_var})
        ENDIF()
    ENDIF()

ENDFUNCTION()

#=============================================================
# _GTK3_FIND_LIBRARY
# Internal function to find libraries packaged with GTK3
#   _var = library variable to create
#=============================================================
FUNCTION(_GTK3_FIND_LIBRARY _var _lib _expand_vc _append_version)

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
            "_GTK3_FIND_LIBRARY( ${_var} ${_lib} ${_expand_vc} ${_append_version} )")
    ENDIF()

    # Not GTK versions per se but the versions encoded into Windows
    # IMPORT LIBRARIES (GtkMM 2.14.1 has a gtkmm-vc80-2_4.lib for example)
    # aLSO THE msvc LIBRARIES USE _ FOR . (this is handled below)
    # ********* SOMEONE WITH WINDOWS NEEDS TO CHECK THIS BIT FOR V3 *********
    # ********* the plain 3 is needed to get Debian Sid to find the libraries
    SET(_versions 3.0 3 2.20 2.18 2.16 2.14 2.12
        2.10  2.8  2.6  2.4  2.2 2.0
        1.20 1.18 1.16 1.14 1.12
        1.10  1.8  1.6  1.4  1.2 1.0)

    SET(_library)
    SET(_library_d)

    SET(_library ${_lib})

    IF(_expand_vc AND MSVC)
        # Add vc80/vc90/vc100 midfixes
        IF(MSVC80)
            SET(_library   ${_library}-vc80)
        ELSEIF(MSVC90)
            SET(_library   ${_library}-vc90)
        ELSEIF(MSVC10)
            SET(_library ${_library}-vc100)
        ENDIF()
        SET(_library_d ${_library}-d)
    ENDIF()

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
            "After midfix addition = ${_library} and ${_library_d}")
    ENDIF()

    SET(_lib_list)
    SET(_libd_list)
    IF(_append_version)
        FOREACH(_ver ${_versions})
            LIST(APPEND _lib_list  "${_library}-${_ver}")
            LIST(APPEND _libd_list "${_library_d}-${_ver}")
        ENDFOREACH()
    ELSE()
        SET(_lib_list ${_library})
        SET(_libd_list ${_library_d})
    ENDIF()

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
            "library list = ${_lib_list} and library debug list = ${_libd_list}")
    ENDIF()

    # For some silly reason the MSVC libraries use _ instead of .
    # in the version fields
    IF(_expand_vc AND MSVC)
        SET(_no_dots_lib_list)
        SET(_no_dots_libd_list)
        FOREACH(_l ${_lib_list})
            STRING(REPLACE "." "_" _no_dots_library ${_l})
            LIST(APPEND _no_dots_lib_list ${_no_dots_library})
        ENDFOREACH()
        # And for debug
        SET(_no_dots_libsd_list)
        FOREACH(_l ${_libd_list})
            STRING(REPLACE "." "_" _no_dots_libraryd ${_l})
            LIST(APPEND _no_dots_libd_list ${_no_dots_libraryd})
        ENDFOREACH()
        # Copy list back to original names
        SET(_lib_list ${_no_dots_lib_list})
        SET(_libd_list ${_no_dots_libd_list})
    ENDIF()

    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
            "While searching for ${_var}, our proposed library list is ${_lib_list}")
    ENDIF()

    FIND_LIBRARY(${_var}
        NAMES ${_lib_list}
        PATHS
        /opt/gnome
        /usr/openwin
        /sw
        $ENV{GTKMM_BASEPATH}
        [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
        PATH_SUFFIXES lib lib64
        )

    IF(_expand_vc AND MSVC)
        IF(GTK3_DEBUG)
            MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}]     "
                "While searching for ${_var}_DEBUG our proposed library list is ${_libd_list}")
        ENDIF()

        FIND_LIBRARY(${_var}_DEBUG
            NAMES ${_libd_list}
            PATHS
            $ENV{GTKMM_BASEPATH}
            [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
            PATH_SUFFIXES lib
            )

        IF(${_var} AND ${_var}_DEBUG)
            IF(NOT GTK3_SKIP_MARK_AS_ADVANCED)
                MARK_AS_ADVANCED(${_var}_DEBUG)
            ENDIF()
            SET(GTK3_LIBRARIES
                ${GTK3_LIBRARIES}
                optimized ${${_var}}
                debug ${${_var}_DEBUG}
                )
            SET(GTK3_LIBRARIES ${GTK3_LIBRARIES} PARENT_SCOPE)
        ENDIF()
    ELSE()
        IF(NOT GTK3_SKIP_MARK_AS_ADVANCED)
            MARK_AS_ADVANCED(${_var})
        ENDIF()
        SET(GTK3_LIBRARIES ${GTK3_LIBRARIES} ${${_var}})
        SET(GTK3_LIBRARIES ${GTK3_LIBRARIES} PARENT_SCOPE)
        # Set debug to release
        SET(${_var}_DEBUG ${${_var}})
        SET(${_var}_DEBUG ${${_var}} PARENT_SCOPE)
    ENDIF()
ENDFUNCTION()

#=============================================================

#
# MAIN()
#

SET(GTK3_FOUND)
SET(GTK3_INCLUDE_DIRS)
SET(GTK3_LIBRARIES)

IF(NOT GTK3_FIND_COMPONENTS)
    # Assume they only want GTK
    SET(GTK3_FIND_COMPONENTS gtk)
ENDIF()

#
# If not specified, enforce version number
#
IF(GTK3_FIND_VERSION)
    IF(NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
    ENDIF()
    SET(GTK3_FAILED_VERSION_CHECK true)
    IF(GTK3_DEBUG)
        MESSAGE(STATUS "[FindGTK3.cmake:${CMAKE_CURRENT_LIST_LINE}] "
            "Searching for version ${GTK3_FIND_VERSION}")
    ENDIF()
    _GTK3_FIND_INCLUDE_DIR(GTK3_GTK_INCLUDE_DIR gtk/gtk.h)
    IF(GTK3_GTK_INCLUDE_DIR)
        _GTK3_GET_VERSION(GTK3_MAJOR_VERSION
        GTK3_MINOR_VERSION
        GTK3_PATCH_VERSION
        ${GTK3_GTK_INCLUDE_DIR}/gtk/gtkversion.h)
    SET(GTK3_VERSION
        ${GTK3_MAJOR_VERSION}.${GTK3_MINOR_VERSION}.${GTK3_PATCH_VERSION})
    IF(GTK3_FIND_VERSION_EXACT)
        IF(GTK3_VERSION VERSION_EQUAL GTK3_FIND_VERSION)
            SET(GTK3_FAILED_VERSION_CHECK false)
        ENDIF()
    ELSE()
        IF(GTK3_VERSION VERSION_EQUAL   GTK3_FIND_VERSION OR
                GTK3_VERSION VERSION_GREATER GTK3_FIND_VERSION)
            SET(GTK3_FAILED_VERSION_CHECK false)
        ENDIF()
    ENDIF()
ELSE()
    # If we can't find the GTK include dir, we can't do version checking
    IF(GTK3_FIND_REQUIRED AND NOT GTK3_FIND_QUIETLY)
        MESSAGE(FATAL_ERROR "Could not find GTK3 include directory")
    ENDIF()
    RETURN()
ENDIF()

IF(GTK3_FAILED_VERSION_CHECK)
    IF(GTK3_FIND_REQUIRED AND NOT GTK3_FIND_QUIETLY)
        IF(GTK3_FIND_VERSION_EXACT)
            MESSAGE(FATAL_ERROR "GTK3 version check failed.
            Version ${GTK3_VERSION} was found, \
            version ${GTK3_FIND_VERSION} is needed exactly."
            )
    ELSE()
        MESSAGE(FATAL_ERROR "GTK3 version check failed.
        Version ${GTK3_VERSION} was found, \
        at least version ${GTK3_FIND_VERSION} is required"
        )
ENDIF()
    ENDIF()

    # If the version check fails, exit out of the module here
    RETURN()
ENDIF()
ENDIF()

#
# Find all components
#

FIND_PACKAGE(Freetype QUIET)
LIST(APPEND GTK3_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
LIST(APPEND GTK3_LIBRARIES ${FREETYPE_LIBRARIES})

FOREACH(_GTK3_component ${GTK3_FIND_COMPONENTS})
    IF(_GTK3_component STREQUAL "gtk")
        _GTK3_FIND_INCLUDE_DIR(GTK3_GLIB_INCLUDE_DIR glib.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GLIBCONFIG_INCLUDE_DIR glibconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GLIB_LIBRARY glib false true)

        _GTK3_FIND_LIBRARY(GTK3_GTHREAD_LIBRARY gthread false true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GOBJECT_INCLUDE_DIR gobject/gobject.h)
        _GTK3_FIND_LIBRARY(GTK3_GOBJECT_LIBRARY gobject false true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GIO_INCLUDE_DIR gio/gio.h)
        _GTK3_FIND_LIBRARY(GTK3_GIO_LIBRARY gio false true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GDK_PIXBUF_INCLUDE_DIR gdk-pixbuf/gdk-pixbuf.h)
        _GTK3_FIND_LIBRARY(GTK3_GDK_PIXBUF_LIBRARY gdk_pixbuf false true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GDK_INCLUDE_DIR gdk/gdk.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GDKCONFIG_INCLUDE_DIR gdk/gdkconfig.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GTK_INCLUDE_DIR gtk/gtk.h)

        # ********* At least on Debian the gdk & gtk libraries
        # ********* don't have the -x11 suffix.
        IF(UNIX)
            _GTK3_FIND_LIBRARY(GTK3_GDK_LIBRARY gdk false true)
            _GTK3_FIND_LIBRARY(GTK3_GTK_LIBRARY gtk false true)
        ELSE()
            # ********* There are various gtk3 builds/packages/bundles
            # ********* available on Windows. Some of them follow the
            # ********* CLASSIC NAMING SCHEME (libs with -win32 suffix)
            # ********* and others prefer the original names.
            # ********* Because we want to support as many packages
            # ********* as possible, we search for both naming styles.
            # ********* Starting with the original names.
            # *********
            # ********* tESTED WITH BOTH VCPKG (gtk+-3.22.19)
            # ********* AND mSYS2 (gtk+-3.22.28)
            _GTK3_FIND_LIBRARY(GTK3_GDK_LIBRARY gdk false true)
            _GTK3_FIND_LIBRARY(GTK3_GTK_LIBRARY gtk false true)
            _GTK3_FIND_LIBRARY(GTK3_GDK_LIBRARY gdk-win32 false true)
            _GTK3_FIND_LIBRARY(GTK3_GTK_LIBRARY gtk-win32 false true)
        ENDIF()

        _GTK3_FIND_INCLUDE_DIR(GTK3_CAIRO_INCLUDE_DIR cairo.h)
        _GTK3_FIND_LIBRARY(GTK3_CAIRO_LIBRARY cairo false false)

        _GTK3_FIND_INCLUDE_DIR(GTK3_FONTCONFIG_INCLUDE_DIR fontconfig/fontconfig.h)

        _GTK3_FIND_INCLUDE_DIR(GTK3_PANGO_INCLUDE_DIR pango/pango.h)
        _GTK3_FIND_LIBRARY(GTK3_PANGO_LIBRARY pango false true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_HARFBUZZ_INCLUDE_DIR hb.h)

        _GTK3_FIND_INCLUDE_DIR(GTK3_ATK_INCLUDE_DIR atk/atk.h)
        _GTK3_FIND_LIBRARY(GTK3_ATK_LIBRARY atk false true)

    ELSEIF(_GTK3_component STREQUAL "gtkmm")

        _GTK3_FIND_INCLUDE_DIR(GTK3_GLIBMM_INCLUDE_DIR glibmm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GLIBMMCONFIG_INCLUDE_DIR glibmmconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GLIBMM_LIBRARY glibmm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GDKMM_INCLUDE_DIR gdkmm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GDKMMCONFIG_INCLUDE_DIR gdkmmconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GDKMM_LIBRARY gdkmm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GTKMM_INCLUDE_DIR gtkmm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GTKMMCONFIG_INCLUDE_DIR gtkmmconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GTKMM_LIBRARY gtkmm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_CAIROMM_INCLUDE_DIR cairomm/cairomm.h)
        _GTK3_FIND_LIBRARY(GTK3_CAIROMM_LIBRARY cairomm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_PANGOMM_INCLUDE_DIR pangomm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_PANGOMMCONFIG_INCLUDE_DIR pangommconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_PANGOMM_LIBRARY pangomm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_SIGC++_INCLUDE_DIR sigc++/sigc++.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_SIGC++CONFIG_INCLUDE_DIR sigc++config.h)
        _GTK3_FIND_LIBRARY(GTK3_SIGC++_LIBRARY sigc true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_GIOMM_INCLUDE_DIR giomm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GIOMMCONFIG_INCLUDE_DIR giommconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GIOMM_LIBRARY giomm true true)

        _GTK3_FIND_INCLUDE_DIR(GTK3_ATKMM_INCLUDE_DIR atkmm.h)
        _GTK3_FIND_LIBRARY(GTK3_ATKMM_LIBRARY atkmm true true)

    ELSEIF(_GTK3_component STREQUAL "glade")

        _GTK3_FIND_INCLUDE_DIR(GTK3_GLADE_INCLUDE_DIR glade/glade.h)
        _GTK3_FIND_LIBRARY(GTK3_GLADE_LIBRARY glade false true)

    ELSEIF(_GTK3_component STREQUAL "glademm")

        _GTK3_FIND_INCLUDE_DIR(GTK3_GLADEMM_INCLUDE_DIR libglademm.h)
        _GTK3_FIND_INCLUDE_DIR(GTK3_GLADEMMCONFIG_INCLUDE_DIR libglademmconfig.h)
        _GTK3_FIND_LIBRARY(GTK3_GLADEMM_LIBRARY glademm true true)

    ELSE()
        MESSAGE(FATAL_ERROR "Unknown GTK3 component ${_component}")
    ENDIF()
ENDFOREACH()

#
# Solve for the GTK3 version if we haven't already
#
IF(NOT GTK3_FIND_VERSION AND GTK3_GTK_INCLUDE_DIR)
    _GTK3_GET_VERSION(GTK3_MAJOR_VERSION
    GTK3_MINOR_VERSION
    GTK3_PATCH_VERSION
    ${GTK3_GTK_INCLUDE_DIR}/gtk/gtkversion.h)
SET(GTK3_VERSION
    ${GTK3_MAJOR_VERSION}.${GTK3_MINOR_VERSION}.${GTK3_PATCH_VERSION}
    )

ENDIF()

#
# Try to enforce components
#

SET(_GTK3_did_we_find_everything true)  # This gets set to GTK3_FOUND

INCLUDE(FindPackageHandleStandardArgs)
#INCLUDE(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

FOREACH(_GTK3_component ${GTK3_FIND_COMPONENTS})
    STRING(TOUPPER ${_GTK3_component} _COMPONENT_UPPER)

    IF(_GTK3_component STREQUAL "gtk")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK3_${_COMPONENT_UPPER}
            "Some or all of the gtk libraries were not found."
            GTK3_GTK_LIBRARY
            GTK3_GTK_INCLUDE_DIR

            GTK3_GLIB_INCLUDE_DIR
            GTK3_GLIBCONFIG_INCLUDE_DIR
            GTK3_GLIB_LIBRARY

            GTK3_GTHREAD_LIBRARY

            GTK3_GDK_INCLUDE_DIR
            GTK3_GDKCONFIG_INCLUDE_DIR
            GTK3_GDK_LIBRARY
            )
    ELSEIF(_GTK3_component STREQUAL "gtkmm")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK3_${_COMPONENT_UPPER}
            "Some or all of the gtkmm libraries were not found."
            GTK3_GTKMM_LIBRARY
            GTK3_GTKMM_INCLUDE_DIR
            GTK3_GTKMMCONFIG_INCLUDE_DIR

            GTK3_GLIBMM_INCLUDE_DIR
            GTK3_GLIBMMCONFIG_INCLUDE_DIR
            GTK3_GLIBMM_LIBRARY

            GTK3_GDKMM_INCLUDE_DIR
            GTK3_GDKMMCONFIG_INCLUDE_DIR
            GTK3_GDKMM_LIBRARY
            )
    ELSEIF(_GTK3_component STREQUAL "glade")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK3_${_COMPONENT_UPPER}
            "The glade library was not found."
            GTK3_GLADE_LIBRARY
            GTK3_GLADE_INCLUDE_DIR
            )
    ELSEIF(_GTK3_component STREQUAL "glademm")
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTK3_${_COMPONENT_UPPER}
            "The glademm library was not found."
            GTK3_GLADEMM_LIBRARY
            GTK3_GLADEMM_INCLUDE_DIR
            GTK3_GLADEMMCONFIG_INCLUDE_DIR
            )
    ENDIF()

    IF(NOT GTK3_${_COMPONENT_UPPER}_FOUND)
        SET(_GTK3_did_we_find_everything false)
    ENDIF()
ENDFOREACH()

IF(_GTK3_did_we_find_everything AND NOT GTK3_VERSION_CHECK_FAILED)
    SET(GTK3_FOUND true)
ELSE()
    # Unset our variables.
    SET(GTK3_FOUND false)
    SET(GTK3_VERSION)
    SET(GTK3_VERSION_MAJOR)
    SET(GTK3_VERSION_MINOR)
    SET(GTK3_VERSION_PATCH)
    SET(GTK3_INCLUDE_DIRS)
    SET(GTK3_LIBRARIES)
ENDIF()

IF(GTK3_INCLUDE_DIRS)
    LIST(REMOVE_DUPLICATES GTK3_INCLUDE_DIRS)
ENDIF()
