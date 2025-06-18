##
# CMakeLists.txt
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file. Specifically:
# safemode, Anth0rx, pyramid, Nachum Barcohen, Rune Morling
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

# One version header generator to rule them all
CONFIGURE_FILE(src/version.h.in ${Vega_Strike_BINARY_DIR}/src/version.h)
CONFIGURE_FILE(src/version.h.in ${Vega_Strike_BINARY_DIR}/setup/src/include/version.h)

SET(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
INCLUDE(InstallRequiredSystemLibraries)

# The source files used to be listed here...

# Option to turn off compiling vegastrike bin
OPTION(DISABLE_CLIENT "Disable building the vegastrike bin" OFF )

# Should we prefer the Mesa OpenGL implementation, or GLVND?
IF (OpenGL_GL_PREFERENCE STREQUAL "LEGACY")
    SET (VEGA_STRIKE_GLU_DEPENDENCY "libglu1-mesa")
ELSEIF (OpenGL_GL_PREFERENCE STREQUAL "GLVND")
    SET (VEGA_STRIKE_GLU_DEPENDENCY "libglvnd0")
ELSE ()
    UNSET (VEGA_STRIKE_GLU_DEPENDENCY)
ENDIF ()
MESSAGE("** OpenGL_GL_PREFERENCE: ${OpenGL_GL_PREFERENCE}")

SET(DATADIR "" CACHE STRING "Default vegastrike data directory")

# Hide our temporary variable and initialize it to empty
SET(CPU_OPTS  )
SET(DEFINES "-DNV_CUBE_MAP -DBOOST_PYTHON_NO_PY_SIGNATURES -DBOOST_ALL_DYN_LINK ")
MARK_AS_ADVANCED( CPU_OPTS )
MARK_AS_ADVANCED( DEFINES )
SET(CPU_SMP "1" CACHE STRING "Number of CPU's in system")

IF (CPU_SMP GREATER 1)
    SET(CPU_OPTS " ${CPU_OPTS} -ftree-parallelize-loops=${CPU_SMP} ")
ENDIF (CPU_SMP GREATER 1)

IF (DATADIR)
    SET(DEFINES "${DEFINES} -DDATA_DIR='\"${DATADIR}\"'")
ENDIF (DATADIR)

IF (NOT DISABLE_CLIENT) ##########
    SET(VSE_TST_LIBS ${TST_LIBS})
    SET(VSE_TST_INCLUDES ${TST_INCLUDES})

    #Find OpenAL
    FIND_PACKAGE(OpenAL REQUIRED)
    IF (OPENAL_FOUND)
        MESSAGE(STATUS "Found OpenAL")
        LIST(APPEND VSE_TST_INCLUDES ${OPENAL_INCLUDE_DIR})
        LIST(APPEND VSE_TST_LIBS ${OPENAL_LIBRARY})
        SET(HAVE_AL 1)
    ELSE (OPENAL_FOUND)
        MESSAGE(FATAL_ERROR "Failed to find required library OpenAL for sound functionality")
    ENDIF (OPENAL_FOUND)

    IF (NOT BEOS)
        #Find SDL
        FIND_PACKAGE(SDL2 REQUIRED)
        IF (SDL2_FOUND)
            LIST(APPEND VSE_TST_INCLUDES ${SDL2_INCLUDE_DIRS})
            LIST(APPEND VSE_TST_LIBS ${SDL2_LIBRARIES})
            MESSAGE(STATUS "SDL2 Found")
            SET(HAVE_SDL 1)
            SET(SDL_WINDOWING 1)
        ELSE (SDL2_FOUND)
            MESSAGE(FATAL_ERROR "Failed to find required library SDL2")
        ENDIF (SDL2_FOUND)
    ENDIF (NOT BEOS)

    #find Vorbis
    FIND_PACKAGE(Vorbis REQUIRED)
    IF (Vorbis_FOUND)
        LIST(APPEND VSE_TST_INCLUDES ${Vorbis_INCLUDE_DIRS})
        LIST(APPEND VSE_TST_LIBS ${Vorbis_LIBRARIES})
        SET(HAVE_OGG 1)
    ELSE (Vorbis_FOUND)
        MESSAGE(WARNING "Can't find Vorbis libs")
    ENDIF (Vorbis_FOUND)

    #Find JPEG
    FIND_PACKAGE(JPEG REQUIRED)
    IF (JPEG_FOUND)
        LIST(APPEND VSE_TST_INCLUDES ${JPEG_INCLUDE_DIR})
        LIST(APPEND VSE_TST_LIBS ${JPEG_LIBRARIES})
    ELSE (JPEG_FOUND)
        MESSAGE(WARNING "How are we gonna open jpegs?")
    ENDIF (JPEG_FOUND)

    #Find PNG
    FIND_PACKAGE(PNG REQUIRED)
    IF (PNG_FOUND)
        LIST(APPEND VSE_TST_INCLUDES ${PNG_INCLUDE_DIRS})
        LIST(APPEND VSE_TST_LIBS ${PNG_LIBRARIES})
    ELSE (PNG_FOUND)
        MESSAGE(WARNING "Can't find PNG lib")
    ENDIF (PNG_FOUND)

ENDIF(NOT DISABLE_CLIENT) ##########

IF (UNIX)
    LIST(APPEND VSE_TST_INCLUDES ${CMAKE_PREFIX_PATH}/harfbuzz)
ENDIF ()

IF (NOT BEOS)
    FIND_LIBRARY(UTIL_LIB util)
ELSEIF (WIN32)
    # Don't need it?
ELSE (NOT BEOS)
    FIND_LIBRARY(UTIL_LIB network)
ENDIF (NOT BEOS)

IF (NOT WIN32)
    LIST(APPEND VSE_TST_LIBS ${UTIL_LIB})
ENDIF (NOT WIN32)

# Still need to add CEGUI and OGRE find packages

# Multiplayer networking option
OPTION(USE_NET_THREAD_POSIX "Enable Posix net threads" OFF)
IF (NOT USE_NET_THREAD_POSIX)
    SET(USE_NET_THREAD_NONE 1)
ENDIF (NOT USE_NET_THREAD_POSIX)


#config.h generation
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
CHECK_INCLUDE_FILE(wchar.h HAVE_WCHAR_H)
CHECK_INCLUDE_FILE(float.h HAVE_FLOAT_H)
CHECK_INCLUDE_FILE(wctype.h HAVE_WCTYPE_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(zlib.h HAVE_ZLIB_H)
CHECK_INCLUDE_FILE(math.h HAVE_MATH_H)
CHECK_INCLUDE_FILE_CXX(tr1/unordered_map HAVE_TR1_UNORDERED_MAP)

CHECK_FUNCTION_EXISTS(gettimeofday HAVE_GETTIMEOFDAY)
IF (HAVE_MATH_H)
    CHECK_FUNCTION_EXISTS(isnan HAVE_ISNAN)
    IF (NOT HAVE_ISNAN)
        CHECK_FUNCTION_EXISTS(_isnan HAVE__ISNAN)
    ENDIF (NOT HAVE_ISNAN)
    CHECK_FUNCTION_EXISTS(finite HAVE_FINITE)
    IF (NOT HAVE_FINITE)
        CHECK_FUNCTION_EXISTS(_finite HAVE__finite)
    ENDIF (NOT HAVE_FINITE)
ENDIF (HAVE_MATH_H)

CHECK_INCLUDE_FILE(sys/stat.h HAVE_STAT_H)
IF (HAVE_STAT_H)
    CHECK_FUNCTION_EXISTS(lstat HAVE_LSTAT)
ENDIF (HAVE_STAT_H)

CHECK_SYMBOL_EXISTS(GLX_ARB_get_proc_address GL/glx.h HAVE_GLXGETPROCADDRESSARB)

IF (UNIX)
    IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(HOSTOS "LINUX")
        SET(_GNU_SOURCE 1)
    ELSE (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        SET(HOSTOS "BSD")
    ENDIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
ELSEIF (WIN32)
    SET(HOSTOS "WIN32")
ELSEIF (BEOS)
    SET(HOSTOS "HAIKU")
ELSE (UNIX)
    SET(HOSTOS "APPLE")
ENDIF (UNIX)

CONFIGURE_FILE(cmake-config.h.in config.h)

#end config.h generation

ADD_COMPILE_OPTIONS("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-include>"
                    "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${Vega_Strike_BINARY_DIR}/engine/config.h>")
ADD_LINK_OPTIONS("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Bsymbolic>")

# Now the source files are listed starting here!
SET(LIBVS_LOGGING
    src/vs_logging.cpp
)

SET(LIBCONFIG
    src/configuration/configuration.cpp
    src/configuration/game_config.cpp
    src/configuration/graphics_config.cpp
)

SET(LIBDAMAGE
    src/damage/damageable_layer.cpp
    src/damage/damageable_object.cpp
)

SET(LIBRESOURCE
    src/resource/resource.cpp
    src/resource/store.cpp
    src/resource/product.cpp
    src/resource/cargo.cpp
    src/resource/manifest.cpp
    src/resource/json_utils.cpp
    src/resource/random_utils.cpp
    )

SET(LIBPYTHON
    src/python/config/python_utils.cpp
)

SET(LIBCOMPONENT
    src/components/component.cpp
    src/components/components_manager.cpp

    src/components/dummy_component.cpp
    src/components/component_utils.cpp
    src/components/energy_consumer.cpp
    src/components/energy_container.cpp
    src/components/reactor.cpp

    src/components/armor.cpp
    src/components/hull.cpp
    src/components/shield.cpp

    src/components/afterburner.cpp
    src/components/afterburner_upgrade.cpp
    src/components/computer.cpp
    src/components/cloak.cpp
    src/components/drive.cpp
    src/components/drive_upgrade.cpp
    src/components/ftl_drive.cpp
    src/components/jump_drive.cpp
    src/components/radar.cpp
    src/components/ecm.cpp
    src/components/repair_bot.cpp
    src/components/ship_functions.cpp
    )

SET(LIBGUI_SOURCES
    src/gui/button.cpp
    src/gui/control.cpp
    src/gui/eventmanager.cpp
    src/gui/eventresponder.cpp
    src/gui/font.cpp
    src/gui/glut_support.cpp
    src/gui/groupcontrol.cpp
    src/gui/guidefs.cpp
    src/gui/guitexture.cpp
    src/gui/modaldialog.cpp
    src/gui/newbutton.cpp
    src/gui/painttext.cpp
    src/gui/picker.cpp
    src/gui/scroller.cpp
    src/gui/simplepicker.cpp
    src/gui/slider.cpp
    src/gui/staticdisplay.cpp
    src/gui/text_area.cpp
    src/gui/textinputdisplay.cpp
    src/gui/window.cpp
    src/gui/windowcontroller.cpp
)

SET(LIBPYTHON_SOURCES
    src/python/init.cpp
    src/python/python_compile.cpp
    src/python/unit_exports.cpp
    src/python/unit_exports1.cpp
    src/python/unit_exports2.cpp
    src/python/unit_exports3.cpp
    src/python/unit_method_defs.cpp
    src/python/unit_wrapper.cpp
    src/python/universe_util_export.cpp

    src/python/infra/get_string.cpp
)

SET(LIBSCRIPT_SOURCES
    src/cmd/script/director_generic.cpp
    src/cmd/script/mission_script.cpp
    src/cmd/script/mission.cpp
    src/cmd/script/msgcenter.cpp
    src/cmd/script/pythonmission.cpp
    src/cmd/script/script_call_olist.cpp
    src/cmd/script/script_call_omap.cpp
    src/cmd/script/script_call_order.cpp
    src/cmd/script/script_call_string.cpp
    src/cmd/script/script_call_unit_generic.cpp
    src/cmd/script/script_callbacks.cpp
    src/cmd/script/script_expression.cpp
    src/cmd/script/script_generic.cpp
    src/cmd/script/script_statement.cpp
    src/cmd/script/script_util.cpp
    src/cmd/script/script_variables.cpp
)

SET(LIBCMDSCRIPTSERVER_SOURCES
    src/cmd/script/flightgroup_server.cpp
    src/cmd/script/script_call_unit_server.cpp
    src/cmd/script/c_alike/c_alike.tab.cpp
    src/cmd/script/c_alike/lex.yy.cpp
)

SET(LIBAI_SOURCES
    src/cmd/ai/aggressive.cpp
    src/cmd/ai/autodocking.cpp
    src/cmd/ai/comm_ai.cpp
    src/cmd/ai/communication_xml.cpp
    src/cmd/ai/communication.cpp
    src/cmd/ai/docking.cpp
    src/cmd/ai/event_xml.cpp
    src/cmd/ai/fire.cpp
    src/cmd/ai/fireall.cpp
    src/cmd/ai/flybywire.cpp
    src/cmd/ai/hard_coded_scripts.cpp
    src/cmd/ai/ikarus.cpp
    src/cmd/ai/missionscript.cpp
    src/cmd/ai/navigation.cpp
    src/cmd/ai/order_comm.cpp
    src/cmd/ai/order.cpp
    src/cmd/ai/script.cpp
    src/cmd/ai/tactics.cpp
    src/cmd/ai/turretai.cpp
    src/cmd/ai/warpto.cpp
    src/cmd/ai/flykeyboard_generic.cpp
)

SET(LIBCMDSERVER_SOURCES
    src/cmd/atmosphere_server.cpp
    src/cmd/base_util_server.cpp
    src/cmd/beam_server.cpp
    src/cmd/bolt_server.cpp
    src/cmd/unit_factory_server.cpp
    src/cmd/unit_functions_server.cpp
    src/cmd/unit_util_server.cpp
    src/cmd/unit_server.cpp
    src/cmd/pilot_server.cpp
    src/cmd/images_server.cpp
    src/cmd/faction_xml_server.cpp
)

SET(LIBGFXCLIENT_SOURCES
    src/gfx/ani_texture.cpp
    src/gfx/animation.cpp
    src/gfx/aux_logo.cpp
    src/gfx/aux_palette.cpp
    src/gfx/aux_texture.cpp
    src/gfx/background.cpp
    src/gfx/camera.cpp
    src/gfx/cockpit_xml.cpp
    src/gfx/cockpit.cpp
    src/gfx/cockpit_gfx.cpp
    src/gfx/cockpit_gfx_utils.cpp
    src/gfx/coord_select.cpp
    src/gfx/env_map_gent.cpp
    src/gfx/gauge.cpp
    src/gfx/halo_system.cpp
    src/gfx/halo.cpp
    src/gfx/hud.cpp
    src/gfx/jpeg_memory.cpp
    src/gfx/loc_select.cpp
    src/gfx/masks.cpp
    src/gfx/mesh_bin.cpp
    src/gfx/mesh_fx.cpp
    src/gfx/mesh_gfx.cpp
    src/gfx/nav/criteria_xml.cpp
    src/gfx/nav/criteria.cpp
    src/gfx/nav/drawgalaxy.cpp
    src/gfx/nav/drawlist.cpp
    src/gfx/nav/drawsystem.cpp
    src/gfx/nav/navcomputer.cpp
    src/gfx/nav/navgetxmldata.cpp
    src/gfx/nav/navpath.cpp
    src/gfx/nav/navscreen.cpp
    src/gfx/nav/navscreenoccupied.cpp
    src/gfx/radar/bubble_display.cpp
    src/gfx/radar/bubble_display.h
    src/gfx/radar/dual_display.cpp
    src/gfx/radar/dual_display.h
    src/gfx/radar/null_display.h
    src/gfx/radar/plane_display.cpp
    src/gfx/radar/plane_display.h
    src/gfx/radar/radar.cpp
    src/gfx/radar/radar.h
    src/gfx/radar/sensor.cpp
    src/gfx/radar/sensor.h
    src/gfx/radar/sphere_display.cpp
    src/gfx/radar/sphere_display.h
    src/gfx/radar/track.cpp
    src/gfx/radar/track.h
    src/gfx/radar/viewarea.cpp
    src/gfx/radar/viewarea.h
    src/gfx/particle.cpp
    src/gfx/pipelined_texture.cpp
    src/gfx/quadsquare_cull.cpp
    src/gfx/quadsquare_render.cpp
    src/gfx/quadsquare_update.cpp
    src/gfx/quadsquare.cpp
    src/gfx/quadtree_xml.cpp
    src/gfx/quadtree.cpp
    src/gfx/ring.cpp
    src/gfx/occlusion.cpp
    src/gfx/screenshot.cpp
    src/gfx/soundcontainer.cpp
    src/gfx/soundcontainer_aldrv.cpp
    src/gfx/sphere.cpp
    src/gfx/sprite.cpp
    src/gfx/star.cpp
    src/gfx/stream_texture.cpp
    src/gfx/technique.cpp
    src/gfx/pass.cpp
    src/gfx/tex_transform.cpp
    src/gfx/vdu.cpp
    src/gfx/vid_file.cpp
    src/ffmpeg_init.cpp
    src/gfx/vsbox.cpp
    src/gfx/vsimage.cpp
    src/gfx/warptrail.cpp
)

SET(LIBGFXSERVER_SOURCES
    src/gfx/sphere_server.cpp
    src/gfx/vsbox_server.cpp
    src/gfx/mesh_server.cpp
    src/gfx/background_server.cpp
    src/gfx/mesh_bin_server.cpp
)

SET(LIBROOTCLIENT_SOURCES
    src/cg_global.cpp
    src/command.cpp
    src/config_xml.cpp
    src/debug_vs.cpp
    src/faction_util.cpp
    src/force_feedback.cpp
    src/gamemenu.cpp
    src/gfxlib_struct.cpp
    src/in_joystick.cpp
    src/in_kb.cpp
    src/sdl_key_converter.cpp
    src/in_main.cpp
    src/in_mouse.cpp
    src/in_sdl.cpp
    src/main_loop.cpp
    src/physics.cpp
    src/rendertext.cpp
    src/ship_commands.cpp
    src/star_system_jump.cpp
    src/star_system.cpp
    src/universe_util.cpp
    src/universe.cpp
)

SET(LIBROOTSERVER_SOURCES
    src/force_feedback_server.cpp
    src/faction_util_server.cpp
    src/gfxlib_struct_server.cpp
    src/libaudioserver.cpp
    src/libserver.cpp
    src/universe_util_server.cpp
)

SET(LIBAUDIO_SOURCES
    src/audio/CodecRegistry.cpp
    src/audio/Listener.cpp
    src/audio/RenderableListener.cpp
    src/audio/RenderableSource.cpp
    src/audio/Renderer.cpp
    src/audio/Scene.cpp
    src/audio/SceneManager.cpp
    src/audio/SimpleScene.cpp
    src/audio/SimpleSound.cpp
    src/audio/SimpleSource.cpp
    src/audio/SoundBuffer.cpp
    src/audio/Sound.cpp
    src/audio/Source.cpp
    src/audio/SourceTemplate.cpp
    src/audio/Stream.cpp
    src/audio/test.cpp
    src/audio/utils.cpp
    src/audio/codecs/Codec.cpp
    src/audio/codecs/FFCodec.cpp
    src/audio/codecs/FFStream.cpp
    src/audio/codecs/OggCodec.cpp
    src/audio/codecs/OggData.cpp
    src/audio/codecs/OggStream.cpp
    src/audio/renderers/OpenAL/OpenALHelpers.cpp
    src/audio/renderers/OpenAL/OpenALRenderableListener.cpp
    src/audio/renderers/OpenAL/OpenALRenderableSource.cpp
    src/audio/renderers/OpenAL/OpenALRenderableStreamingSource.cpp
    src/audio/renderers/OpenAL/OpenALRenderer.cpp
    src/audio/renderers/OpenAL/OpenALSimpleSound.cpp
    src/audio/renderers/OpenAL/OpenALStreamingSound.cpp
)


ADD_LIBRARY(vegastrike_python SHARED
    ${LIBPYTHON}
)
# Enable PIE for this library in a cross-platform-compatible way
set_property(TARGET vegastrike_python PROPERTY POSITION_INDEPENDENT_CODE TRUE)
SET_PROPERTY(TARGET vegastrike_python PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike_python PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike_python PROPERTY CXX_EXTENSIONS ON)
TARGET_INCLUDE_DIRECTORIES(vegastrike_python SYSTEM PRIVATE ${VSE_TST_INCLUDES})


TARGET_LINK_LIBRARIES(
    vegastrike_python
    ${Boost_LIBRARIES}
    ${Python3_LIBRARIES}
)

# Name must be witout the lib prefix
SET_TARGET_PROPERTIES(vegastrike_python PROPERTIES PREFIX "")

ADD_LIBRARY(vegastrike-engine_com OBJECT
    ${VEGA_GL_H_PATH}
    ${VEGA_GLU_H_PATH}
    ${VEGA_GLEXT_H_PATH}
    ${VEGA_GLUT_H_PATH}
    ${LIBPYTHON}
    ${LIBVS_LOGGING}
    ${LIBCONFIG}
    ${LIBDAMAGE}
    ${LIBRESOURCE}
    ${LIBCOMPONENT}
    ${LIBAI_SOURCES}
    ${LIBNET_SOURCES}
    ${LIBPYTHON_SOURCES}
    ${LIBSCRIPT_SOURCES}
)

#TARGET_COMPILE_FEATURES(vegastrike-engine_com PUBLIC cxx_std_11)
SET_PROPERTY(TARGET vegastrike-engine_com PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET vegastrike-engine_com PROPERTY CXX_STANDARD_REQUIRED TRUE)
SET_PROPERTY(TARGET vegastrike-engine_com PROPERTY CXX_EXTENSIONS ON)
set_property(TARGET vegastrike-engine_com PROPERTY POSITION_INDEPENDENT_CODE TRUE)

TARGET_COMPILE_DEFINITIONS(vegastrike-engine_com PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
IF (WIN32)
    TARGET_COMPILE_DEFINITIONS(vegastrike-engine_com PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-engine_com PUBLIC _WIN32_WINNT=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-engine_com PUBLIC WINVER=0x0A00)
    TARGET_COMPILE_DEFINITIONS(vegastrike-engine_com PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
ENDIF()

TARGET_INCLUDE_DIRECTORIES(vegastrike-engine_com SYSTEM PRIVATE ${VSE_TST_INCLUDES})
TARGET_INCLUDE_DIRECTORIES(vegastrike-engine_com PRIVATE
        # VS engine headers
        ${Vega_Strike_SOURCE_DIR}
        ${Vega_Strike_SOURCE_DIR}/engine
        ${Vega_Strike_SOURCE_DIR}/engine/src
        # Library Headers
        ${Vega_Strike_SOURCE_DIR}/libraries
        # CMake Artifacts
        ${Vega_Strike_BINARY_DIR}
        ${Vega_Strike_BINARY_DIR}/src
        ${Vega_Strike_BINARY_DIR}/engine
        ${Vega_Strike_BINARY_DIR}/engine/src
)

IF (NEED_LINKING_AGAINST_LIBM)
    TARGET_LINK_LIBRARIES(vegastrike-engine_com PRIVATE m)
ENDIF()
TARGET_LINK_LIBRARIES(vegastrike-engine_com PRIVATE
        OpenGL::GL
        OpenGL::GLU
        GLUT::GLUT
        ${VSE_TST_LIBS}
        ${Boost_LIBRARIES}
        ${Python3_LIBRARIES}
)

SET(VEGASTRIKE_SOURCES
    ${VEGA_GL_H_PATH}
    ${VEGA_GLU_H_PATH}
    ${VEGA_GLEXT_H_PATH}
    ${VEGA_GLUT_H_PATH}
    ${LIBGUI_SOURCES}
    ${LIBNETCLIENT_SOURCES}
    ${LIBROOTCLIENT_SOURCES}
    ${LIBGFXCLIENT_SOURCES}
    ${LIBAUDIO_SOURCES}
    src/aldrv/al_globals.cpp
    src/aldrv/al_init.cpp
    src/aldrv/al_listen.cpp
    src/aldrv/al_sound.cpp
    src/cmd/ai/firekeyboard.cpp
    src/cmd/ai/flyjoystick.cpp
    src/cmd/ai/flykeyboard.cpp
    src/cmd/asteroid.cpp
    src/cmd/atmosphere.cpp
    src/cmd/base_init.cpp
    src/cmd/base_interface.cpp
    src/cmd/base_util.cpp
    src/cmd/base_write_python.cpp
    src/cmd/base_write_xml.cpp
    src/cmd/base_xml.cpp
    src/cmd/basecomputer.cpp
    src/cmd/beam.cpp
    src/cmd/bolt.cpp
    src/cmd/briefing.cpp
    src/cmd/building.cpp
    src/cmd/click_list.cpp
    src/cmd/cont_terrain.cpp
    src/cmd/controls_factory.cpp
    src/cmd/missile.cpp
    src/cmd/music.cpp
    src/cmd/nebula.cpp
    src/cmd/planet.cpp
    src/cmd/ship.cpp
    src/cmd/script/c_alike/c_alike.tab.cpp
    src/cmd/script/c_alike/lex.yy.cpp
    src/cmd/script/director.cpp
    src/cmd/script/flightgroup.cpp
    src/cmd/script/script_call_briefing.cpp
    src/cmd/script/script_call_unit.cpp
    src/cmd/terrain.cpp
    src/cmd/unit_functions.cpp
    src/cmd/unit_util.cpp
    src/cmd/faction_xml.cpp
    src/cmd/faction_factory.cpp
    src/cmd/pilot.cpp
    src/cmd/images.cpp
    src/cmd/turret.cpp
    src/gldrv/gl_program.cpp
    src/gldrv/gl_clip.cpp
    src/gldrv/gl_fog.cpp
    src/gldrv/gl_globals.cpp
    src/gldrv/gl_init.cpp
    src/gldrv/gl_light_pick.cpp
    src/gldrv/gl_light_state.cpp
    src/gldrv/gl_light.cpp
    src/gldrv/gl_material.cpp
    src/gldrv/gl_matrix.cpp
    src/gldrv/gl_misc.cpp
    src/gldrv/gl_quad_list.cpp
    src/gldrv/gl_sphere_list.cpp
    src/gldrv/gl_state.cpp
    src/gldrv/sdds.cpp
    src/gldrv/gl_texture.cpp
    src/gldrv/gl_vertex_list.cpp
    src/gldrv/winsys.cpp
    src/main.cpp
    src/python/briefing_wrapper.cpp
)

IF (NOT DISABLE_CLIENT)
    ADD_EXECUTABLE(vegastrike-engine WIN32 MACOSX_BUNDLE ${VEGASTRIKE_SOURCES})

    #    TARGET_COMPILE_FEATURES(vegastrike-engine PUBLIC cxx_std_14)
    SET_PROPERTY(TARGET vegastrike-engine PROPERTY CXX_STANDARD 14)
    SET_PROPERTY(TARGET vegastrike-engine PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET vegastrike-engine PROPERTY CXX_EXTENSIONS ON)

    TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
    IF (WIN32)
        TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC _WIN32_WINNT=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC WINVER=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
    ENDIF()

    OPTION (LOG_TIME_TAKEN_DETAILS "Whether to log timing details in the main loop" OFF)
    IF (LOG_TIME_TAKEN_DETAILS)
        TARGET_COMPILE_DEFINITIONS(vegastrike-engine PUBLIC LOG_TIME_TAKEN_DETAILS)
    ENDIF ()

    TARGET_INCLUDE_DIRECTORIES(vegastrike-engine SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vegastrike-engine PRIVATE
            # VS engine headers
            ${Vega_Strike_SOURCE_DIR}
            ${Vega_Strike_SOURCE_DIR}/engine
            ${Vega_Strike_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Strike_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Strike_BINARY_DIR}
            ${Vega_Strike_BINARY_DIR}/src
            ${Vega_Strike_BINARY_DIR}/engine
            ${Vega_Strike_BINARY_DIR}/engine/src
    )

    IF (NEED_LINKING_AGAINST_LIBM)
        TARGET_LINK_LIBRARIES(vegastrike-engine m)
    ENDIF()

    TARGET_LINK_LIBRARIES(vegastrike-engine
            OpenGL::GL
            OpenGL::GLU
            GLUT::GLUT
            ${VSE_TST_LIBS}
            ${Boost_LIBRARIES}
            ${Python3_LIBRARIES}
            $<TARGET_OBJECTS:vegastrike-engine_com>
            vegastrike_gfx_generic
            vegastrike_root_generic
            vegastrike_cmd
            vegastrike-OPcollide
    )
    SET_TARGET_PROPERTIES(vegastrike-engine PROPERTIES LINK_FLAGS "${TST_LFLAGS}")
    SET_TARGET_PROPERTIES(vegastrike-engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
ENDIF (NOT DISABLE_CLIENT)

# Vssetup Sub build file
ADD_SUBDIRECTORY(setup)

# Add other utilities here
ADD_SUBDIRECTORY(objconv)

## show debug output
#GET_DIRECTORY_PROPERTY(TEMP_DIRECTORY INCLUDE_DIRECTORIES)
#MESSAGE(STATUS "Default build type is RelWithDebInfo, no cpu opts enabled. ")
#MESSAGE(STATUS "Building with BUILD_OPT: ${BUILD_OPT}")
#MESSAGE(STATUS "Building with CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
#MESSAGE("Linking with : ${VSE_TST_LIBS}")
#MESSAGE("including : ${TEMP_DIRECTORY}")
## end debug output

MARK_AS_ADVANCED(FORCE
    Boost_DIR
    AVCODEC_FOUND
    AVFORMAT_FOUND
    AVUTIL_FOUND
    AVCODEC_LIBRARY
    AVCODEC_INCLUDE_DIR
    AVUTIL_INCLUDE_DIR
    AVFORMAT_LIBRARY
    AVUTIL_LIBRARY
    SWSCALE_INCLUDE_DIR
    SWSCALE_LIBRARY
    AVFORMAT_INCLUDE_DIR
    HAVE_AVFORMAT_H
    HAVE_AVCODEC_H
    HAVE_AVIO_H
    HAVE_FFMPEG_SWSCALE_H
    CMAKE_BACKWARDS_COMPATIBILITY
    FFMPEG_INCLUDE_DIR
    FFMPEG_FOUND
    FFMPEG_LIBRARIES
    FFMPEG_DEFINITIONS
    GTK3_ATK_INCLUDE_DIR
    GTK3_ATK_LIBRARY
    GTK3_CAIRO_INCLUDE_DIR
    GTK3_CAIRO_LIBRARY
    GTK3_GDK_INCLUDE_DIR
    GTK3_GDK_LIBRARY
    GTK3_GDK_PIXBUF_LIBRARY
    GTK3_GDK_PIXBUF_INCLUDE_DIR
    GTK3_GLIBCONFIG_INCLUDE_DIR
    GTK3_GLIB_INCLUDE_DIR
    GTK3_GLIB_LIBRARY
    GTK3_GMODULE_LIBRARY
    GTK3_GOBJECT_LIBRARY
    GTK3_PANGO_INCLUDE_DIR
    GTK3_PANGO_LIBRARY
    GTK3_GTHREAD_LIBRARY
    GTK3_GTKGL_INCLUDE_DIR
    GTK3_GTKGL_LIBRARY
    GTK3_GTK_INCLUDE_DIR
    GTK3_GTK_LIBRARY
    LIBRARY_OUTPUT_PATH
    OPENAL_INCLUDE_DIR
    OPENAL_LIBRARY
    SDLMAIN_LIBRARY
    SDL_INCLUDE_DIR
    SDL_LIBRARY
    UTIL_LIB
    ogg_INCLUDE_DIR
    ogg_LIBRARY
    vorbis_INCLUDE_DIR
    vorbis_LIBRARY
    vorbisfile_LIBRARY
)
INSTALL(TARGETS vegastrike-engine DESTINATION bin)

# CMake doesn't support doing symlinks directly; however,
# it's easy enough to mimic such behavior via CMake by installing
# the executable a second time using a different name.
IF (WIN32)
    INSTALL(PROGRAMS "${Vega_Strike_BINARY_DIR}/vegastrike-engine" RENAME vegastrike.exe DESTINATION bin)
ELSE (WIN32)
    INSTALL(PROGRAMS "${Vega_Strike_BINARY_DIR}/vegastrike-engine" RENAME vegastrike DESTINATION bin)
ENDIF (WIN32)


ADD_SUBDIRECTORY("${PROJECT_SOURCE_DIR}/doc" "./doc")

SET(CPACK_VERBATIM_VARIABLES YES)

# As per https://braintrekking.wordpress.com/2013/04/27/dll-hell-how-to-include-microsoft-redistributable-runtime-libraries-in-your-cmakecpack-project/
INSTALL(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT Libraries)

SET(CPACK_PACKAGE_NAME "Vega-Strike")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "3D OpenGL spaceflight simulator")
SET(CPACK_PACKAGE_VENDOR "Vega Strike")
SET(CPACK_PACKAGE_DESCRIPTION "Vega Strike is a Space Flight Simulator that allows a player to explore, trade, and fight in the vast openness of space. You start in an old beat up cargo ship, with endless possibilities in front of you and just enough cash to scrape together a life. Yet danger lurks in the space beyond.")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VEGASTRIKE_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VEGASTRIKE_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VEGASTRIKE_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "VegaStrike-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_CONTACT "maintainers@vega-strike.org")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-Source")
SET(CPACK_SOURCE_GENERATOR "TBZ2" "TXZ")

# See https://gitlab.kitware.com/cmake/community/-/wikis/doc/cpack/PackageGenerators for generator lists
IF (CMAKE_SYSTEM_NAME STREQUAL Windows)
    MESSAGE(STATUS "Configuring Packaging for Windows")
    # NSIS - See https://cmake.org/cmake/help/v3.3/module/CPackNSIS.html
    # NSI bug requires at least 1 set of 4 forwards slashes??
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}\\\\icon4.ico")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\VegaStrike.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "Vega Strike")
    SET(CPACK_NSIS_MANIFEST_DPI_AWARE true)
    SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL YES)
    SET(CPACK_NSIS_HELP_LINK "https://vega-strike.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "https://vega-strike.org/about")
    SET(CPACK_NSIS_CONTACT "maintainers@vega-strike.org")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_NSIS_MENU_LINKS "")

    # Windows installer format(s) to output
    SET(CPACK_GENERATOR "NSIS64")
    SET(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/packages")
    SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}_Windows_${CMAKE_SYSTEM_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    MESSAGE(STATUS "Configuring Packaging for macOS")
    # Mac OS X
    # There's a few options for MacOSX; not sure what we want to use
    # DragNDrop -> DMG
    #  See https://cmake.org/cmake/help/v3.3/module/CPackDMG.html
    # Bundle -> Compressed Disk Image
    # PackageMaker - see https://cmake.org/cmake/help/v3.3/module/CPackPackageMaker.html
    SET(CPACK_GENERATOR "DragNDrop")
    SET(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/packages")
    SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}_macOS_${CMAKE_SYSTEM_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL Linux)
    MESSAGE(STATUS "Configuring Packaging for Linux")
    SET(CPACK_GENERATOR "TBZ2" "TGZ" "TXZ")
    # Linux

    SET(VEGA_STRIKE_PYTHON_VERSION_STR "py3")

    # Moved up above the boost_python detection logic - stephengtuggy 2021-09-18
    # # Detect available Linux Distros that can be built for
    # FIND_PACKAGE(LinuxDistro REQUIRED)

    # "DEB"
    IF(VS_CAN_BUILD_DEB)
        MESSAGE(STATUS "Configuring Debian Packaging")
        # See https://cmake.org/cmake/help/v3.3/module/CPackDeb.html
        SET(CPACK_DEBIAN_PACKAGE_NAME "Vega-Strike")
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "maintainers@vega-strike.org")
        SET(CPACK_DEBIAN_PACKAGE_VERSION "${VEGASTRIKE_PKG_VERSION_STR}")
        # Debian Version: /etc/debian_version - <name>/<junk>
        IF (NOT LSB_EXISTS)
            # this still needs to be true so LSB variables are not used below
            # even if we don't detect the /etc/debian_version file
            SET(USE_DEBIAN_VERSION TRUE)
            IF (EXISTS "/etc/debian_version")
                FILE(READ "/etc/debian_version" DEBIAN_VERSION_DATA)
                STRING(REGEX MATCH "^(.+)\/.*$" DEBIAN_VERSION_DATA DEBIAN_RELEASE_VERSION)
            ELSE (EXISTS "/etc/debian_version")
                MESSAGE("Detected dpkg-release but Debian Distro is unknown")
                SET(DEBIAN_RELEASE_VERSION "Unknown")
            ENDIF (EXISTS "/etc/debian_version")
        ELSE (NOT LSB_EXISTS)
            SET(USE_DEBIAN_VERSION FALSE)
            SET(DEBIAN_RELEASE_VERSION "Debian Derivative Release Version ${LSB_LINUX_DISTRIBUTION_CODENAME}")
        ENDIF (NOT LSB_EXISTS)

        # dependency.list is generated by `script/packages`. It requires the binaries to already be built to work.
        # if using `script/package` to build the packages, then it will automatically re-run `cmake` to update the data
        # for the dependencies.
        #
        # If dependency.list is not available, then a hard-coded set of dependencies is provided below.
        # Thanks to stephengtuggy for providing the static list for known distributions.
        #
        # Prefer the auto-generated list by default as it'll be more accurate
        # Fall back to the static lists when it's not available.
        MESSAGE("Looking for ${CMAKE_BINARY_DIR}/dependency.list")
        IF (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")
            MESSAGE("Found Dependency file at ${CMAKE_BINARY_DIR}/dependency.list")
            # Load the dependencies from the file into the CPack variable:
            FILE(STRINGS "${CMAKE_BINARY_DIR}/dependency.list" AUTOMATED_DEBIAN_DEPENDENCY_LIST)
            FOREACH(DLD_DEPENDENCY IN LISTS AUTOMATED_DEBIAN_DEPENDENCY_LIST)
                MESSAGE("** Found Dependency ${DLD_DEPENDENCY}")
                IF (DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${DLD_DEPENDENCY}")
                ELSE(DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
                    IF (NOT "${DLD_DEPENDENCY}" STREQUAL "")
                        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DLD_DEPENDENCY}")
                    ENDIF (NOT "${DLD_DEPENDENCY}" STREQUAL "")
                ENDIF(DEFINED CPACK_DEBIAN_PACKAGE_DEPENDS)
            ENDFOREACH(DLD_DEPENDENCY)

        ELSE (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")
            MESSAGE("Still need to generate dependency list")
            IF (USE_DEBIAN_VERSION)
                # Debian Dependency Chain
                IF (DEBIAN_RELEASE_VERSION STREQUAL "bookworm")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.11")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.81.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "bullseye")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "buster")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "stretch")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.5")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.62.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSE ()
                    MESSAGE(WARNING "!! Unsupported Debian version.")
                ENDIF ()

            ELSE (USE_DEBIAN_VERSION)
                # Ubuntu Dependency Chain
                IF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bionic")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.65.1")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, ${VEGA_STRIKE_GLU_DEPENDENCY}, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER} libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "focal")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.8")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER} libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bookworm") # Apparently some versions of Debian have lsb_release after all
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.11")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.81.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "bullseye")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl-gfx1.2-5, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER} libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "buster")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.7")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER} libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (DEBIAN_RELEASE_VERSION STREQUAL "stretch")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.5")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.62.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "groovy")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.71.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "hirsute")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.9")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "ulyana")  # Linux Mint 20 codename "ulyana"
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.8")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.67.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "vanessa")  # Linux Mint 20 codename "vanessa"
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.10")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "wilma")  # Linux Mint 22 codename "wilma"
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.12-stdlib")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx-1.0-0, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "impish")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.10")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "jammy")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.10")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.74.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg62-turbo, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx2.0.14, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-json${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSEIF (LSB_LINUX_DISTRIBUTION_CODENAME STREQUAL "noble")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.12-stdlib")
                    IF (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "1.83.0")
                    ELSE (Boost_DERIVED_VERSION STREQUAL "")
                        SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                    ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libjpeg-turbo8, libpng16-16, freeglut3, libgtk-3-0, libvorbis0a, libopenal1, libsdl2-gfx-1.0-0, xdg-utils, libgl1, libopengl0, ${VEGA_STRIKE_GLU_DEPENDENCY}, libboost-atomic${BOOST_VER}, libboost-chrono${BOOST_VER}, libboost-date-time${BOOST_VER}, libboost-filesystem${BOOST_VER}, libboost-log${BOOST_VER}, libboost-python${BOOST_VER}, libboost-regex${BOOST_VER}, libboost-system${BOOST_VER}, libboost-thread${BOOST_VER}, libboost-program-options${BOOST_VER}")
                ELSE ()
                    MESSAGE(WARNING "!! Unsupported Debian derivative.")
                ENDIF ()

            ENDIF (USE_DEBIAN_VERSION)
        ENDIF (EXISTS "${CMAKE_BINARY_DIR}/dependency.list")

        IF (USE_DEBIAN_VERSION)
            # Add the platform (Ubuntu, Debian, etc) to the filename, including the version (stretch, jessie, etc)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-debian-${DEBIAN_RELEASE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")

        ELSE (USE_DEBIAN_VERSION)
            # Add the platform (Ubuntu, Debian, etc) to the filename, including the version (stretch, jessie, etc)
            SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-${LSB_LINUX_DISTRIBUTION}-${LSB_LINUX_DISTRIBUTION_CODENAME}_${CMAKE_SYSTEM_PROCESSOR}")
        ENDIF (USE_DEBIAN_VERSION)
        SET(CPACK_DEBIAN_PACKAGE_SECTION "Amusements/Games")
        SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.vega-strike.org")
        SET(CPACK_GENERATOR "DEB")
    ENDIF (VS_CAN_BUILD_DEB)

    #  "RPM"
    IF (VS_CAN_BUILD_RPM)
        MESSAGE(STATUS "Configuring RPM Packaging")
        # See https://cmake.org/cmake/help/v3.3/module/CPackRPM.html
        SET(CPACK_RPM_PACKAGE_LICENSE "GPLv3") # See ../LICENSE
        SET(CPACK_RPM_PACKAGE_URL "https://www.vega-strike.org")
        SET(CPACK_RPM_PACKAGE_NAME "Vega-Strike")

        # Figure out the filename
        IF (LINUX_CODENAME)
            SET(VEGA_STRIKE_LINUX_VERSION_STR "${LINUX_CODENAME}")
        ELSE (LINUX_CODENAME)
            SET(VEGA_STRIKE_LINUX_VERSION_STR "${LINUX_VERSION_ID}")
        ENDIF (LINUX_CODENAME)

        SET(CPACK_PACKAGE_FILE_NAME "${CPACK_RPM_PACKAGE_NAME}_v${VEGASTRIKE_PKG_VERSION_STR}-${OpenGL_GL_PREFERENCE}-${LINUX_ID}-${VEGA_STRIKE_LINUX_VERSION_STR}_${CMAKE_SYSTEM_PROCESSOR}")

        # TODO: Finish porting the script/package script over for RPM-based distros
        # Detect whether SuSe or RH/CentOS/Fedora as deps may change
        IF (LINUX_ID STREQUAL opensuse-leap)
            IF (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
                SET(CPACK_RPM_PACKAGE_REQUIRES "libboost_python-py3-1_66_0, libpython3_6m1_0")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng16-16, libglut3, libgtk-3-0, libvorbis0, libopenal0, libSDL-1_2-0, libglvnd, expat, libboost_log1_66_0, libboost_system1_66_0, libboost_filesystem1_66_0, libboost_thread1_66_0, libboost_regex1_66_0, libboost_chrono1_66_0, libboost_atomic1_66_0, libboost_program_options1_66_0")
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 15.4)
                IF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "1.75.0")
                ELSE (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                SET(CPACK_RPM_PACKAGE_REQUIRES "libboost_python-py3-${BOOST_VER}, libpython3_10-1_0")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng16-16, libglut3, libgtk-3-0, libvorbis0, libopenal0, libSDL-1_2-0, libglvnd, expat, libboost_log${BOOST_VER}, libboost_system${BOOST_VER}, libboost_filesystem${BOOST_VER}, libboost_thread${BOOST_VER}, libboost_regex${BOOST_VER}, libboost_chrono${BOOST_VER}, libboost_atomic${BOOST_VER}, libboost_json${BOOST_VER}, libboost_program_options${BOOST_VER}")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 15.5 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 15.6)
                IF (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "1.75.0")
                ELSE (Boost_DERIVED_VERSION STREQUAL "")
                    SET(BOOST_VER "${Boost_DERIVED_VERSION}")
                ENDIF (Boost_DERIVED_VERSION STREQUAL "")
                SET(CPACK_RPM_PACKAGE_REQUIRES "libboost_python-py3-${BOOST_VER}, python311-base")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng16-16, libglut3, libgtk-3-0, libvorbis0, libopenal0, libSDL-1_2-0, libglvnd, expat, libboost_log${BOOST_VER}, libboost_system${BOOST_VER}, libboost_filesystem${BOOST_VER}, libboost_thread${BOOST_VER}, libboost_regex${BOOST_VER}, libboost_chrono${BOOST_VER}, libboost_atomic${BOOST_VER}, libboost_program_options${BOOST_VER}")
            ELSE (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
                MESSAGE (SEND_WARNING "!! Unsupported version of openSUSE")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF (LINUX_VERSION_ID VERSION_EQUAL 15.2 OR LINUX_VERSION_ID VERSION_EQUAL 15.3)
        ELSEIF (LINUX_ID STREQUAL fedora AND LINUX_VERSION_ID VERSION_GREATER_EQUAL 30 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 41)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 32 OR LINUX_VERSION_ID VERSION_EQUAL 33)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python38, boost-python3")       # python39
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 31)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python38, boost-python3")
            ELSE (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python37, boost-python3")
            ENDIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 34)
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
        ELSEIF (LINUX_ID STREQUAL centos)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.7)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python39, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.8 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.9)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3.11, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 9.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 9.3)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSE ()
                MESSAGE(WARNING "We do not currently support this version of CentOS.")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF ()
        ELSEIF (LINUX_ID STREQUAL rhel OR LINUX_ID STREQUAL redhat)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.7)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python39, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.8 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.9)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3.11, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 9.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 9.3)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSE ()
                MESSAGE(WARNING "This version of Red Hat is not currently supported.")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF ()
        ELSEIF (LINUX_ID STREQUAL rocky)
            IF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.7)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python39, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 8.8 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 8.9)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3.11, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_GREATER_EQUAL 9.0 AND LINUX_VERSION_ID VERSION_LESS_EQUAL 9.3)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSEIF (LINUX_VERSION_ID VERSION_EQUAL 9.5)
                SET(CPACK_RPM_PACKAGE_REQUIRES "python3, boost-python3")
                SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg-turbo, libpng, freeglut, gtk3, libvorbis, openal-soft, SDL, libglvnd, expat, boost-log, boost-system, boost-filesystem, boost-thread, boost-regex, boost-chrono, boost-atomic, boost-program-options")
            ELSE ()
                MESSAGE(WARNING "This version of Rocky Linux is not currently supported.")
                SET(CPACK_RPM_PACKAGE_REQUIRES "")
            ENDIF ()
        ELSE (LINUX_ID STREQUAL opensuse-leap)
            MESSAGE(WARNING "!! Unrecognized RPM-based Linux distribution. RPM Dependencies may be wrong for this platform.")
            SET(CPACK_RPM_PACKAGE_REQUIRES "python3")
            SET(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, libjpeg, libpng, freeglut, gtk3, libvorbis, openal, SDL_mixer")
        ENDIF (LINUX_ID STREQUAL opensuse-leap)
        SET(CPACK_GENERATOR "RPM")
    ENDIF (VS_CAN_BUILD_RPM)
ELSE ()
    MESSAGE(STATUS "Configuring Packaging for Unknown Platform - \"${CMAKE_SYSTEM_NAME}\"")
    # Unknown Platform --> Just do the various compressed tarballs
    SET(CPACK_GENERATOR "TBZ2" "TGZ" "TXZ")
ENDIF ()

INCLUDE(CPack)

IF (USE_GTEST)
    SET(TEST_NAME ${PROJECT_NAME}_tests)
    ADD_EXECUTABLE(
        ${TEST_NAME}
        src/cmd/tests/csv_tests.cpp
        src/cmd/tests/json_tests.cpp
        src/configuration/tests/configuration_tests.cpp
        src/damage/tests/layer_tests.cpp
        src/damage/tests/object_tests.cpp
        src/resource/tests/buy_sell.cpp
        src/resource/tests/resource_test.cpp
        src/resource/tests/manifest_tests.cpp
        src/resource/tests/random_tests.cpp
        src/configuration/tests/python_tests.cpp
        src/exit_unit_tests.cpp
        src/components/tests/energy_container_tests.cpp
        src/components/tests/balancing_tests.cpp
        src/components/tests/drive_tests.cpp
        src/components/tests/afterburner_tests.cpp
        src/components/tests/jump_drive_tests.cpp
        src/components/tests/utils_tests.cpp
        src/components/tests/operational_tests.cpp
    )
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(${TEST_NAME} PRIVATE
            # VS engine headers
            ${Vega_Strike_SOURCE_DIR}
            ${Vega_Strike_SOURCE_DIR}/engine
            ${Vega_Strike_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Strike_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Strike_BINARY_DIR}
            ${Vega_Strike_BINARY_DIR}/src
            ${Vega_Strike_BINARY_DIR}/engine
            ${Vega_Strike_BINARY_DIR}/engine/src
    )
    SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${Vega_Strike_BINARY_DIR}/)

    #TARGET_COMPILE_FEATURES(${TEST_NAME} PUBLIC cxx_std_11)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD 14)
    SET_PROPERTY(TARGET ${TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    IF (NEED_LINKING_AGAINST_LIBM)
        TARGET_LINK_LIBRARIES(${TEST_NAME} m)
    ENDIF()

    ADD_LIBRARY(vegastrike-testing OBJECT
        ${LIBPYTHON}
        ${LIBVS_LOGGING}
        ${LIBCONFIG}
        ${LIBDAMAGE}
        ${LIBRESOURCE}
        ${LIBCOMPONENT}
    )
    TARGET_INCLUDE_DIRECTORIES(vegastrike-testing SYSTEM PRIVATE ${VSE_TST_INCLUDES})
    TARGET_INCLUDE_DIRECTORIES(vegastrike-testing PRIVATE
            # VS engine headers
            ${Vega_Strike_SOURCE_DIR}
            ${Vega_Strike_SOURCE_DIR}/engine
            ${Vega_Strike_SOURCE_DIR}/engine/src
            # Library Headers
            ${Vega_Strike_SOURCE_DIR}/libraries
            # CMake Artifacts
            ${Vega_Strike_BINARY_DIR}
            ${Vega_Strike_BINARY_DIR}/src
            ${Vega_Strike_BINARY_DIR}/engine
            ${Vega_Strike_BINARY_DIR}/engine/src
    )
    TARGET_COMPILE_DEFINITIONS(vegastrike-testing PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
    SET_PROPERTY(TARGET vegastrike-testing PROPERTY CXX_STANDARD 14)
    SET_PROPERTY(TARGET vegastrike-testing PROPERTY CXX_STANDARD_REQUIRED TRUE)
    SET_PROPERTY(TARGET vegastrike-testing PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    IF (WIN32)
        TARGET_COMPILE_DEFINITIONS(vegastrike-testing PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-testing PUBLIC _WIN32_WINNT=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-testing PUBLIC WINVER=0x0A00)
        TARGET_COMPILE_DEFINITIONS(vegastrike-testing PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
    ENDIF()

    TARGET_LINK_LIBRARIES(
            ${TEST_NAME}
            ${OPENAL_LIBRARY}
            ${Vorbis_LIBRARIES}
            ${JPEG_LIBRARIES}
            ${PNG_LIBRARIES}
            ${Boost_LIBRARIES}
            ${Python3_LIBRARIES}
            gtest_main
            $<TARGET_OBJECTS:vegastrike-testing>
            vegastrike_cmd
            Boost::log
            Boost::log_setup
            Boost::json
    )
    TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC "BOOST_ALL_DYN_LINK" "$<$<CONFIG:Debug>:BOOST_DEBUG_PYTHON>")
    IF (WIN32)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC BOOST_USE_WINAPI_VERSION=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC _WIN32_WINNT=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC WINVER=0x0A00)
        TARGET_COMPILE_DEFINITIONS(${TEST_NAME} PUBLIC "$<$<CONFIG:Debug>:Py_DEBUG>")
    ENDIF()

    FILE(
        COPY "src/configuration/tests/vegastrike.config"
        DESTINATION ${CMAKE_BINARY_DIR}/test_assets
    )

    FILE(
        COPY "src/cmd/tests/units.json"
        DESTINATION ${CMAKE_BINARY_DIR}/test_assets
    )

    FILE(
        COPY "src/configuration/tests/python_tests.py"
        DESTINATION ${CMAKE_BINARY_DIR}/test_assets
    )

    INCLUDE(GoogleTest)
    gtest_discover_tests(${TEST_NAME} PROPERTIES DISCOVERY_TIMEOUT 30)
ENDIF (USE_GTEST)
