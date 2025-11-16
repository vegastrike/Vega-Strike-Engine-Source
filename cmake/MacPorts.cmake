SET(CMAKE_PREFIX_PATH "/opt/local")
SET(Boost_INCLUDE_DIR "/opt/local/libexec/boost/1.87/include" CACHE STRING "Boost install directory")
SET(Boost_DIR "/opt/local/libexec/boost/1.87/lib/cmake/Boost-1.87.0" CACHE STRING "Boost root directory")
SET(CMAKE_FIND_FRAMEWORK "LAST")
SET(CMAKE_FIND_APPBUNDLE "LAST")

SET(INSTALL_GTEST ON CACHE BOOL "Should we download and install GTest and GMock using FetchContent_MakeAvailable?" FORCE)
SET(USE_GTEST ON CACHE BOOL "Should we build and run the unit tests using GTest?" FORCE)
SET(INSTALL_SDL3 OFF CACHE BOOL "Should we download and install SDL3 using FetchContent_MakeAvailable?" FORCE)
SET(INSTALL_SDL3_IMAGE OFF CACHE BOOL "Should we download and install SDL3_image using FetchContent_MakeAvailable?" FORCE)

# Is this correct?
SET(MacPortsLibArchivePrefix CACHE "/opt/local/libarchive/")

SET(LibArchive_INCLUDE_DIR "${MacPortsLibArchivePrefix}/include")
SET(LibArchive_LIBRARY "${MacPortsLibArchivePrefix}/lib")
