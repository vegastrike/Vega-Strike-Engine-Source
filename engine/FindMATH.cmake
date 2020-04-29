# - Try to find Math
# Once done this will define
#
#  MATH_FOUND - system has Math lib
#  MATH_INCLUDE_DIRS - the Math include directory
#  MATH_LIBRARIES - Link these to use Math lib

if (NOT BEOS)
if (MATH_LIBRARIES AND MATH_INCLUDE_DIRS)
  # in cache already
  set(MATH_FOUND TRUE)
else (MATH_LIBRARIES AND MATH_INCLUDE_DIRS)

  find_path(MATH_INCLUDE_DIRS
    NAMES
      math.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

 find_library(MATH_LIBRARIES
    NAMES
      m
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )


  if (MATH_INCLUDE_DIRS AND MATH_LIBRARIES)
     set(MATH_FOUND TRUE)
  endif (MATH_INCLUDE_DIRS AND MATH_LIBRARIES)

  if (MATH_FOUND)
    if (NOT MATH_FIND_QUIETLY)
      message(STATUS "Found Math: ${MATH_LIBRARIES}")
    endif (NOT MATH_FIND_QUIETLY)
  else (MATH_FOUND)
    if (MATH_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Math lib")
    endif (MATH_FIND_REQUIRED)
  endif (MATH_FOUND)

  # show the MATH_INCLUDE_DIRS and MATH_LIBRARIES variables only in the advanced view
  mark_as_advanced(MATH_INCLUDE_DIRS MATH_LIBRARIES)

endif (MATH_LIBRARIES AND MATH_INCLUDE_DIRS)
endif (NOT BEOS)
