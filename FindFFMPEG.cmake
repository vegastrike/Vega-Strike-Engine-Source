# - Try to find FFmpeg
# Once done this will define
#
#  FFMPEG_FOUND - system has FFmpeg
#  FFMPEG_INCLUDE_DIRS - the FFmpeg include directory
#  FFMPEG_LIBRARIES - Link these to use FFmpeg
#  FFMPEG_DEFINITIONS - Compiler switches required for using FFmpeg
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
  # in cache already
  set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
	SET(FFMPEG_LIBRARIES)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  
  pkg_check_modules(ffmpeg1 REQUIRED libavcodec)
  SET(_FFMPEGIncDir ${ffmpeg1_INCLUDE_DIRS})
  SET(_FFMPEGLinkDir ${ffmpeg1_LIBRARY_DIRS})
  SET(_FFMPEGLinkFlags ${ffmpeg1_LDFLAGS})
  SET(_FFMPEGCflags ${ffmpeg1_CFLAGS})
  
  pkg_check_modules(swscale1 libswscale)
  IF(swscale1_FOUND)
  	SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${swscale1_LIBRARIES})
  ENDIF(swscale1_FOUND)
  
  set(FFMPEG_DEFINITIONS ${_FFMPEGCflags})

  find_path(FFMPEG_INCLUDE_DIR
    NAMES
      avcodec.h
    PATHS
      ${_FFMPEGIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      ffmpeg
  )

 find_library(AVUTIL_LIBRARY
    NAMES
      avutil
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  find_library(AVCODEC_LIBRARY
    NAMES
      avcodec
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  find_library(AVFORMAT_LIBRARY
    NAMES
      avformat
    PATHS
      ${_FFMPEGLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(FFMPEG_INCLUDE_DIRS
    ${FFMPEG_INCLUDE_DIR}
  )


  if (AVUTIL_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVUTIL_LIBRARY}
    )
  endif (AVUTIL_LIBRARY)

  if (AVCODEC_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVCODEC_LIBRARY}
    )
  endif (AVCODEC_LIBRARY)

  if (AVFORMAT_LIBRARY)
    set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBRARIES}
      ${AVFORMAT_LIBRARY}
    )
  endif (AVFORMAT_LIBRARY)

  if (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
     set(FFMPEG_FOUND TRUE)
  endif (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)

  if (FFMPEG_FOUND)
    if (NOT FFMPEG_FIND_QUIETLY)
      message(STATUS "Found FFmpeg: ${FFMPEG_LIBRARIES}")
    endif (NOT FFMPEG_FIND_QUIETLY)
  else (FFMPEG_FOUND)
    if (FFMPEG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FFmpeg")
    endif (FFMPEG_FIND_REQUIRED)
  endif (FFMPEG_FOUND)

  # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
  mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)

