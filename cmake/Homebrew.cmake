EXECUTE_PROCESS(COMMAND brew --prefix
                OUTPUT_VARIABLE HomebrewPrefixOut
                ERROR_VARIABLE HomebrewPrefixErr
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY)
SET(CMAKE_PREFIX_PATH "${HomebrewPrefixOut}:${CMAKE_PREFIX_PATH}")
SET(CMAKE_FIND_FRAMEWORK "LAST")
SET(CMAKE_FIND_APPBUNDLE "LAST")
SET(PythonInstalledVia "Homebrew" CACHE STRING "How Python was installed, such as via Homebrew or MacPorts")

EXECUTE_PROCESS(COMMAND brew --prefix libarchive
                OUTPUT_VARIABLE LIBARCHIVE_PREFIX
                ERROR_VARIABLE LIBARCHIVE_PREFIX_ERR
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY)
SET(PKG_CONFIG_PATH "${LIBARCHIVE_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}")
SET(LibArchive_INCLUDE_DIR:PATH "${LIBARCHIVE_PREFIX}/include")
SET(LibArchive_LIBRARY "${LIBARCHIVE_PREFIX}/lib")
SET(LibArchive_LIBRARIES:FILEPATH "${LibArchive_LIBRARY}/libarchive.dylib")

FIND_LIBRARY(LibArchive REQUIRED)
