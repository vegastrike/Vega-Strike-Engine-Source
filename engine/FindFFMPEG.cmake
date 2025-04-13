##
# FindFFMPEG.cmake
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
#

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
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(FindPkgConfig)
  
  pkg_check_modules(ffmpeg1 libavcodec)
  
  if (ffmpeg1_FOUND)
    SET(_FFMPEGIncDir ${ffmpeg1_INCLUDE_DIRS})
    SET(_FFMPEGLinkDir ${ffmpeg1_LIBRARY_DIRS})
    SET(_FFMPEGLinkFlags ${ffmpeg1_LDFLAGS})
    SET(_FFMPEGCflags ${ffmpeg1_CFLAGS})
    
    SET(FFMPEG_LIBRARIES)
    SET(FFMPEG_DEFINITIONS ${_FFMPEGCflags} -D__STDC_CONSTANT_MACROS)

    find_path(FFMPEG_INCLUDE_DIR
      NAMES
        avcodec.h
      PATHS
        ${_FFMPEGIncDir}
        /usr/include
        /usr/include/libavcodec
        /usr/local/include
        /opt/local/include
        /sw/include
      PATH_SUFFIXES
        ffmpeg
        libavcodec
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

    set(FFMPEG_INCLUDE_DIRS
      ${FFMPEG_INCLUDE_DIRS}
      ${FFMPEG_INCLUDE_DIR}
    )
    
    if (AVCODEC_LIBRARY)
      set(FFMPEG_LIBRARIES
        ${FFMPEG_LIBRARIES}
        ${AVCODEC_LIBRARY}
      )
    endif (AVCODEC_LIBRARY)

    pkg_check_modules(avformat1 libavformat)
    if (avformat1_FOUND) 
      SET(_AVFORMATIncDir ${avformat1_INCLUDE_DIRS})
      SET(_AVFORMATLinkDir ${avformat1_LIBRARY_DIRS})
      SET(_AVFORMATLinkFlags ${avformat1_LDFLAGS})
      SET(_AVFORMATCflags ${avformat1_CFLAGS})

      set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_AVFORMATCflags})
      
      find_path(AVFORMAT_INCLUDE_DIR
        NAMES
          avformat.h
          avio.h
        PATHS
          ${_AVFORMATIncDir}
          ${_FFMPEGIncDir}
          /usr/include
          /usr/local/include
          /opt/local/include
          /sw/include
        PATH_SUFFIXES
          ffmpeg
          libavformat
      )

      find_library(AVFORMAT_LIBRARY
        NAMES
          avformat
        PATHS
          ${_AVFORMATLinkDir}
          ${_FFMPEGLinkDir}
          /usr/lib
          /usr/local/lib
          /opt/local/lib
          /sw/lib
      )

      set(FFMPEG_INCLUDE_DIRS
        ${FFMPEG_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIR}
      )
      
      if (AVFORMAT_LIBRARY)
        set(FFMPEG_LIBRARIES
          ${FFMPEG_LIBRARIES}
          ${AVFORMAT_LIBRARY}
        )
      endif (AVFORMAT_LIBRARY)

      pkg_check_modules(avutil1 libavutil)
      if (avutil1_FOUND)
        SET(_AVUTILIncDir ${avutil1_INCLUDE_DIRS})
        SET(_AVUTILLinkDir ${avutil1_LIBRARY_DIRS})
        SET(_AVUTILLinkFlags ${avutil1_LDFLAGS})
        SET(_AVUTILCflags ${avutil1_CFLAGS})

        set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_AVUTILCflags})
        
        find_library(AVUTIL_LIBRARY
          NAMES
            avutil
          PATHS
            ${_AVUTILLinkDir}
            ${_FFMPEGLinkDir}
            /usr/lib
            /usr/local/lib
            /opt/local/lib
            /sw/lib
        )

        if (AVUTIL_LIBRARY)
          set(FFMPEG_LIBRARIES
            ${FFMPEG_LIBRARIES}
            ${AVUTIL_LIBRARY}
          )
        endif (AVUTIL_LIBRARY)

        pkg_check_modules(swscale1 libswscale)
        SET(_SWSCALEIncDir ${swscale1_INCLUDE_DIRS})
        SET(_SWSCALELinkDir ${swscale1_LIBRARY_DIRS})
        SET(_SWSCALELinkFlags ${swscale1_LDFLAGS})
        SET(_SWSCALECflags ${swscale1_CFLAGS})
        
        set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_SWSCALECflags})
        
        IF(swscale1_FOUND)
          SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${swscale1_LIBRARIES})

          find_path(SWSCALE_INCLUDE_DIR
            NAMES
              swscale.h
            PATHS
              ${_SWSCALEIncDir}
              /usr/include
              /usr/local/include
              /opt/local/include
              /sw/include
            PATH_SUFFIXES
              ffmpeg
              libswscale
          )
          
          set(FFMPEG_INCLUDE_DIRS
            ${FFMPEG_INCLUDE_DIRS}
            ${SWSCALE_INCLUDE_DIR}
          )
        
          SET(HAVE_SWSCALE_H 1)
        ENDIF(swscale1_FOUND)
      ENDIF(avutil1_FOUND)
    ENDIF (avformat1_FOUND)
  ENDIF(ffmpeg1_FOUND)

  if (FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
     set(FFMPEG_FOUND TRUE)
     set(HAVE_AVFORMAT_H 1)
     set(HAVE_AVCODEC_H 1)
     set(HAVE_AVIO_H 1)
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
