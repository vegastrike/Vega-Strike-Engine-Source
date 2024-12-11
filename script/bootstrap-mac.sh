#!/usr/bin/env bash
set -e

DETECT_MAC_OS_VERSION=$(sw_vers -productVersion | cut -f 1,2 -d '.')
echo "Detected Mac OS X Version: ${DETECT_MAC_OS_VERSION}"

MAC_OS_NAME="UNKOWN"
case "${DETECT_MAC_OS_VERSION}" in
    "10.13")
        MAC_OS_NAME="High_Sierra"
        ;;
    "10.14")
        MAC_OS_NAME="Mojave"
        ;;
    "10.15")
        MAC_OS_NAME="Catalina"
        ;;
    "11.*")
        MAC_OS_NAME="Big_Sur"
        ;;
    "12.*")
        MAC_OS_NAME="Monterey"
        ;;
    "13.*")
        MAC_OS_NAME="Ventura"
        ;;
    "14.*")
        MAC_OS_NAME="Sonoma"
        ;;
    "15.*")
        MAC_OS_NAME="Sequoia"
        ;;
    *)
        echo "Unknown Mac Release: ${DETECT_MAC_OS_VERSION}"
        exit 1
        ;;
esac

echo "Detected Mac OS X ${DETECT_MAC_OS_VERSION} - ${MAC_OS_NAME}"
# While we may be able to assume some other dependencies there does seem to be
# some changes between versions of MacOS on which are provided by default; so
# aside from the two above we should just go ahead an install everything ourselves.

# Install the stuff we know needs to get installed all the time
brew install \
    gcc \
    cmake \
    python3 \
    boost-python3 \
    jpeg \
    libpng \
    gtk+3 \
    gtkglext \
    sdl2

# The following Apple-provided libraries are deprecated:
# * OpenGL as of macOS 10.14
# * GLUT as of macOS 10.9
brew install mesa mesa-glu freeglut
ln -s /usr/local/include/GL /usr/local/include/OpenGL
ln -s /usr/local/include/GL /usr/local/include/GLUT
# ln -s /usr/local/lib/libGL.dylib /usr/local/lib/libOpenGL.dylib
# find /usr/local/lib/ -iname '*gl*.dylib'

# MacOS 13+ needs Vorbis support
brew install vorbis-tools

# The Apple-provided OpenAL is deprecated as of macOS 10.15
brew install openal-soft freealut
