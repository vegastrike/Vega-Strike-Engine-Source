# - Try to find PNG
# Once done this will define
#
#  PNG_FOUND - system has PNG
#  PNG_INCLUDE_DIRS - the PNG include directory
#  PNG_LIBRARIES - Link these to use PNG


if (PNG_LIBRARIES AND PNG_INCLUDE_DIRS)
  # in cache already
  set(PNG_FOUND TRUE)
else (PNG_LIBRARIES AND PNG_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  
  pkg_check_modules(LIBPNG REQUIRED libpng)
  SET(_LIBPNGIncDir ${LIBPNG_INCLUDE_DIRS})
  SET(_LIBPNGLinkDir ${LIBPNG_LIBRARY_DIRS})
  SET(_LIBPNGLinkFlags ${LIBPNG_LDFLAGS})
  SET(_LIBPNGCflags ${LIBPNG_CFLAGS})
  

  set(LIBPNG_DEFINITIONS ${LIBPNG_CFLAGS})

  find_path(PNG_INCLUDE_DIRS
    NAMES
      png.h
    PATHS
      ${LIBPNG_INCLUDE_DIRS}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      png
  )

 find_library(PNG_LIBRARIES
    NAMES
      png
    PATHS
      ${PNG_LIBRARY_DIRS}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )


  if (PNG_INCLUDE_DIRS AND PNG_LIBRARIES)
     set(PNG_FOUND TRUE)
  endif (PNG_INCLUDE_DIRS AND PNG_LIBRARIES)

  if (PNG_FOUND)
    if (NOT PNG_FIND_QUIETLY)
      message(STATUS "Found PNG: ${PNG_LIBRARIES}")
    endif (NOT PNG_FIND_QUIETLY)
  else (PNG_FOUND)
    if (PNG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find PNG")
    endif (PNG_FIND_REQUIRED)
  endif (PNG_FOUND)

  # show the PNG_INCLUDE_DIRS and PNG_LIBRARIES variables only in the advanced view
  mark_as_advanced(PNG_INCLUDE_DIRS PNG_LIBRARIES)

endif (PNG_LIBRARIES AND PNG_INCLUDE_DIRS)

