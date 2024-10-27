# - Try to find Ogre
# Once done this will define
#
#  HAVE_OGRE - system has Ogre
#  OGRE_LIBRARIES - Linker flags
#  OGRE_DEFINITIONS - Compiler switches required for using Ogre
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)
  # in cache already
  set(HAVE_OGRE TRUE)
else (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  
  pkg_check_modules(OGRE OGRE>=1.2.0)
  
  if (OGRE_FOUND)
      SET(OGRE_DEFINITIONS ${OGRE_CFLAGS})
      SET(OGRE_LIBRARIES ${OGRE_LIBRARIES})
      SET(OGRE_INCLUDE_DIRS ${OGRE_INCLUDE_DIRS})
      message(STATUS "Found Ogre: ${OGRE_VERSION}")
  else (OGRE_FOUND)
      SET(HAVE_OGRE FALSE)
      message(STATUS "Ogre not found")
  endif (OGRE_FOUND)
  
  # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
  mark_as_advanced(OGRE_LIBRARIES OGRE_DEFINITIONS OGRE_INCLUDE_DIRS)

endif (OGRE_LIBRARIES AND OGRE_DEFINITIONS AND OGRE_INCLUDE_DIRS)

