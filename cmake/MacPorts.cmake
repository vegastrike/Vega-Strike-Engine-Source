SET(CMAKE_PREFIX_PATH "/opt/local")
SET(Boost_INCLUDE_DIR "/opt/local/libexec/boost/1.87/include" CACHE STRING "Boost install directory")
SET(Boost_DIR "/opt/local/libexec/boost/1.87/lib/cmake/Boost-1.87.0" CACHE STRING "Boost root directory")
SET(CMAKE_FIND_FRAMEWORK "LAST")
SET(CMAKE_FIND_APPBUNDLE "LAST")

SET(INSTALL_GTEST CACHE "OFF")
SET(USE_GTEST CACHE "ON")
SET(INSTALL_SDL3 CACHE "OFF")
SET(INSTALL_SDL3_IMAGE CACHE "OFF")

# Is this correct?
SET(MacPortsLibArchivePrefix CACHE "/opt/local/libarchive/")

SET(LibArchive_INCLUDE_DIR "${MacPortsLibArchivePrefix}/include")
SET(LibArchive_LIBRARY "${MacPortsLibArchivePrefix}/lib")
