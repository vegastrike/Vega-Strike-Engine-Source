IF (NOT BEOS)
# vssetup is a standalone Dear ImGui + SDL3 + OpenGL3 app. It does not use the
# engine's config.h defines or hidden-visibility flags, so strip them here.
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Version string from git (tag + commits, e.g. "v0.5.1" or "v0.5.1-12-gabc1234").
execute_process(
    COMMAND git describe --tags --dirty --always
    WORKING_DIRECTORY ${vsUTCS_SOURCE_DIR}
    OUTPUT_VARIABLE VSSETUP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if (NOT VSSETUP_VERSION)
    set(VSSETUP_VERSION "unknown")
endif()
add_definitions(-DVSSETUP_VERSION="${VSSETUP_VERSION}")

add_executable(vssetup src/c/setup.cpp src/c/modern_ui.cpp src/c/config_model.cpp)
set_target_properties(vssetup PROPERTIES OUTPUT_NAME config-vs-05)   # settings app binary

# Vendored Dear ImGui (libraries/imgui) with the SDL3 + OpenGL3 backends.
find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
add_library(vegastrike_imgui STATIC
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/imgui.cpp
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/imgui_draw.cpp
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/imgui_tables.cpp
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/imgui_widgets.cpp
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/backends/imgui_impl_sdl3.cpp
    ${vsUTCS_SOURCE_DIR}/libraries/imgui/backends/imgui_impl_opengl3.cpp )
set_property(TARGET vegastrike_imgui PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(vegastrike_imgui PUBLIC SDL3::SDL3 OpenGL::GL)

target_link_libraries(vssetup vegastrike_imgui m)
include_directories(${vsUTCS_SOURCE_DIR}/libraries/imgui)
ENDIF (NOT BEOS)
