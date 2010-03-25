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


if (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
    # in cache already
    set(FFMPEG_FOUND TRUE)
else (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    include(FindPkgConfig)
    
    # If pkg-config knows about it, lets use what it knows first
    pkg_check_modules(ffmpeg1 REQUIRED libavcodec)
    if(ffmpeg1_FOUND)
        set(_FFMPEGIncDir ${ffmpeg1_INCLUDE_DIRS})
        set(_FFMPEGLinkDir ${ffmpeg1_LIBRARY_DIRS})
        set(_FFMPEGLinkFlags ${ffmpeg1_LDFLAGS})
        set(_FFMPEGCflags ${ffmpeg1_CFLAGS})
        set(FFMPEG_DEFINITIONS ${_FFMPEGCflags})  
    endif(ffmpeg1_FOUND)
    find_path(AVCODEC_INCLUDE_DIR
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
                        libavcodec)
    find_library(AVCODEC_LIBRARY
                    NAMES
                        avcodec
                    PATHS
                        ${_FFMPEGLinkDir}
                        /usr/lib
                        /usr/local/lib
                        /opt/local/lib
                        /sw/lib)
    if(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY)
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${AVCODEC_LIBRARY})
        set(AVCODEC_FOUND TRUE CACHE BOOL "")
    endif(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY)
    
    pkg_check_modules(avformat1 REQUIRED libavformat)
    if(avformat1_FOUND)
        set(_AVFORMATIncDir ${avformat1_INCLUDE_DIRS})
        set(_AVFORMATLinkDir ${avformat1_LIBRARY_DIRS})
        set(_AVFORMATLinkFlags ${avformat1_LDFLAGS})
        set(_AVFORMATCflags ${avformat1_CFLAGS})
        set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_AVFORMATCflags})
    endif(avformat1_FOUND)
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
                        libavformat)
    find_library(AVFORMAT_LIBRARY
                    NAMES
                        avformat
                    PATHS
                        ${_AVFORMATLinkDir}
                        ${_FFMPEGLinkDir}
                        /usr/lib
                        /usr/local/lib
                        /opt/local/lib
                        /sw/lib)
    if(AVFORMAT_INCLUDE_DIR AND AVFORMAT_LIBRARY)
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${AVFORMAT_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${AVFORMAT_LIBRARY})
        set(AVFORMAT_FOUND TRUE CACHE BOOL "")
    endif(AVFORMAT_INCLUDE_DIR AND AVFORMAT_LIBRARY)

    pkg_check_modules(avutil1 REQUIRED libavutil)
    if(avutil1_FOUND)
        set(_AVUTILIncDir ${avutil1_INCLUDE_DIRS})
        set(_AVUTILLinkDir ${avutil1_LIBRARY_DIRS})
        set(_AVUTILLinkFlags ${avutil1_LDFLAGS})
        set(_AVUTILCflags ${avutil1_CFLAGS})
        set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_AVUTILCflags})
    endif(avutil1_FOUND)
    find_path(AVUTIL_INCLUDE_DIR
                    NAMES
                        avutil.h
                    PATHS
                        ${_AVUTILIncDir}
                        ${_FFMPEGIncDir}
                        /usr/include
                        /usr/local/include
                        /opt/local/include
                        /sw/include
                    PATH_SUFFIXES
                        ffmpeg
                        libavutil)
    find_library(AVUTIL_LIBRARY
                    NAMES
                        avutil
                    PATHS
                        ${_AVUTILLinkDir}
                        ${_FFMPEGLinkDir}
                        /usr/lib
                        /usr/local/lib
                        /opt/local/lib
                        /sw/lib)                        
    if(AVUTIL_INCLUDE_DIR AND AVUTIL_LIBRARY)
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${AVUTIL_LIBRARY})
        set(AVUTIL_FOUND TRUE CACHE BOOL "")
    endif(AVUTIL_INCLUDE_DIR AND AVUTIL_LIBRARY)

    pkg_check_modules(swscale1 libswscale)
    if(swscale1_FOUND)
        set(_SWSCALEIncDir ${swscale1_INCLUDE_DIRS})
        set(_SWSCALELinkDir ${swscale1_LIBRARY_DIRS})
        set(_SWSCALELinkFlags ${swscale1_LDFLAGS})
        set(_SWSCALECflags ${swscale1_CFLAGS})
        set(FFMPEG_DEFINITIONS ${FFMPEG_DEFINITIONS} ${_SWSCALECflags})
    endif(swscale1_FOUND)
    find_path(SWSCALE_INCLUDE_DIR
                    NAMES
                        swscale.h
                    PATHS
                        ${_SWSCALEIncDir}
                        ${_FFMPEGIncDir}
                        /usr/include
                        /usr/local/include
                        /opt/local/include
                        /sw/include
                    PATH_SUFFIXES
                        ffmpeg
                        libswscale)
    find_library(SWSCALE_LIBRARY
                    NAMES
                        swscale
                    PATHS
                        ${_SWSCALELinkDir}
                        ${_FFMPEGLinkDir}
                        /usr/lib
                        /usr/local/lib
                        /opt/local/lib
                        /sw/lib)
    if(SWSCALE_INCLUDE_DIR AND SWSCALE_LIBRARY)
        set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${SWSCALE_LIBRARY})
        set(SWSCALE_FOUND TRUE)
    endif(SWSCALE_INCLUDE_DIR AND SWSCALE_LIBRARY)
    
    # Only report finding ffmpeg if we have all 3 of our required libraries and dev files
    if (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
        set(FFMPEG_FOUND TRUE)
        #The following individual header defines are redundent, either we have ffmpeg or we dont. 
        #I'm keeping them in here since i dont know if klauss uses them or just the HAVE_FFMPEG define
        set(HAVE_AVFORMAT_H 1)
        set(HAVE_AVCODEC_H 1)
        set(HAVE_AVIO_H 1)
        # Add in swscale support if we have it,  not required (afaik)
        if(SWSCALE_FOUND)
            set(HAVE_FFMPEG_SWSCALE_H 1)
        endif(SWSCALE_FOUND)
        set(FFMPEG_DEFINITIONS "${FFMPEG_DEFINITIONS}" CACHE STRING "")
        set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INCLUDE_DIRS}" CACHE STRING "")
        set(FFMPEG_LIBRARIES "${FFMPEG_LIBRARIES}" CACHE STRING "")
    else(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
        set(FFMPEG_DEFINITIONS "" CACHE STRING "")
        set(FFMPEG_INCLUDE_DIRS "" CACHE STRING "")
        set(FFMPEG_LIBRARIES "" CACHE STRING "")
    endif (AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)

    if (FFMPEG_FOUND)
        message(STATUS "Found FFmpeg: ${FFMPEG_LIBRARIES}")
    else (FFMPEG_FOUND)
        if (FFMPEG_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find FFmpeg")
        endif (FFMPEG_FIND_REQUIRED)
    endif (FFMPEG_FOUND)
    
    # show the FFMPEG_INCLUDE_DIRS and FFMPEG_LIBRARIES variables only in the advanced view
    mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)

endif(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND)
