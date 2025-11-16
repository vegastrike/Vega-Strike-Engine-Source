EXECUTE_PROCESS(COMMAND brew --prefix
                OUTPUT_VARIABLE HomebrewPrefixOut
                ERROR_VARIABLE HomebrewPrefixErr
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY)
SET(CMAKE_PREFIX_PATH "${HomebrewPrefixOut}:${CMAKE_PREFIX_PATH}")
SET(CMAKE_FIND_FRAMEWORK "LAST")
SET(CMAKE_FIND_APPBUNDLE "LAST")

SET(INSTALL_GTEST ON CACHE BOOL "Should we download and install GTest and GMock using FetchContent_MakeAvailable?" FORCE)
SET(USE_GTEST ON CACHE BOOL "Should we build and run the unit tests using GTest?" FORCE)
SET(INSTALL_SDL3 OFF CACHE BOOL "Should we download and install SDL3 using FetchContent_MakeAvailable?" FORCE)
SET(INSTALL_SDL3_IMAGE OFF CACHE BOOL "Should we download and install SDL3_image using FetchContent_MakeAvailable?" FORCE)

EXECUTE_PROCESS(COMMAND brew --prefix libarchive
                OUTPUT_VARIABLE LIBARCHIVE_PREFIX
                ERROR_VARIABLE LIBARCHIVE_PREFIX_ERR
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY)
SET(PKG_CONFIG_PATH "${LIBARCHIVE_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}")
MESSAGE(DEBUG "LIBARCHIVE_PREFIX: ${LIBARCHIVE_PREFIX}")
MESSAGE(DEBUG "PKG_CONFIG_PATH: ${PKG_CONFIG_PATH}")
SET(LibArchive_ROOT "${LIBARCHIVE_PREFIX}")
MESSAGE(DEBUG "LibArchive_ROOT: ${LibArchive_ROOT}")
