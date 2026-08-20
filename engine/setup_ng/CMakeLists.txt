##
# CMakeLists.txt — vs-settings-ng (vs-05 setup app ported; JSON backend later)
#
# Layer 1: the vs-05 setup app copied as-is, building an XML-baseline
# vssetup. It is a standalone Dear ImGui + SDL3 + OpenGL3 app (no engine
# internals). Temporary name/binary; becomes `vegasettings` on engine merge.
#
# It does not use the engine's config.h defines or hidden-visibility flags,
# so strip them here (mirrors vs-05's setup/CMakeLists.txt).
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "-include config.h" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Version string from git (tag + commits).
execute_process(
    COMMAND git describe --tags --dirty --always
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE VSSETUP_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if (NOT VSSETUP_VERSION)
    set(VSSETUP_VERSION "unknown")
endif()
add_definitions(-DVSSETUP_VERSION="${VSSETUP_VERSION}")

add_executable(vssetup src/c/setup.cpp src/c/modern_ui.cpp src/c/config_model.cpp)
set_target_properties(vssetup PROPERTIES OUTPUT_NAME vs-settings-ng)

# The engine already builds imgui (vegastrike_imgui target, via libraries/imgui)
# with the SDL3 + OpenGL3 backends; link against it rather than redefining it.
find_package(SDL3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
target_link_libraries(vssetup vegastrike_imgui SDL3::SDL3 OpenGL::GL m)
target_include_directories(vssetup PRIVATE
    ${PROJECT_SOURCE_DIR}/libraries/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/c
)

# Install the config utility alongside the engine binaries (temporary name).
INSTALL(TARGETS vssetup RUNTIME DESTINATION bin)
