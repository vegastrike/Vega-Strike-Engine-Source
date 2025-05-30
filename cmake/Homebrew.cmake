EXECUTE_PROCESS(COMMAND brew --prefix
                OUTPUT_VARIABLE HomebrewPrefixOut
                ERROR_VARIABLE HomebrewPrefixErr)
SET(CMAKE_PREFIX_PATH $HomebrewPrefixOut)
SET(CMAKE_FIND_FRAMEWORK "LAST")
SET(PythonInstalledVia "Homebrew" CACHE STRING "How Python was installed, such as via Homebrew or MacPorts")

EXECUTE_PROCESS(COMMAND homebrew --prefix libarchive
                OUTPUT_VARIABLE HOMEBREW_LIBARCHIVE_PREFIX
                ERROR_VARIABLE HOMEBREW_LIBARCHIVE_PREFIX_ERR)
SET(PKG_CONFIG_PATH "${HOMEBREW_LIBARCHIVE_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}")

SET(LibArchive_INCLUDE_DIR "${HOMEBREW_LIBARCHIVE_PREFIX}/include")
SET(LibArchive_LIB_DIR "${HOMEBREW_LIBARCHIVE_PREFIX}/lib")
