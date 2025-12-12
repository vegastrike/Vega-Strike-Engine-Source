#!/usr/bin/env bash
#====================================
# @file   : bootstrap-mac.sh
# @brief  : installs dependencies for building Vega Strike on macOS X and later
#====================================
# Copyright (C) 2020-2025 Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer,
# and other Vega Strike contributors
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.

set -e

echo "-------------------------------------"
echo "--- bootstrap-mac.sh | 2025-12-11 ---"
echo "-------------------------------------"

DETECT_MAC_OS_VERSION=$(sw_vers -productVersion | cut -f 1,2 -d '.')
echo "Detected Mac OS X Version: ${DETECT_MAC_OS_VERSION}"

MAC_OS_NAME="UNKNOWN"
case "${DETECT_MAC_OS_VERSION}" in
    "10.13")
        MAC_OS_NAME="High_Sierra"
        echo "Unsupported Mac Version"
        exit 2
        ;;
    "10.14")
        MAC_OS_NAME="Mojave"
        echo "Unsupported Mac Version"
        exit 2
        ;;
    "10.15")
        MAC_OS_NAME="Catalina"
        echo "Unsupported Mac Version"
        exit 2
        ;;
    "11.*")
        MAC_OS_NAME="Big_Sur"
        echo "Unsupported Mac Version"
        exit 2
        ;;
    "12.*")
        MAC_OS_NAME="Monterey"
        echo "Unsupported Mac Version"
        exit 2
        ;;
    "13.0"|"13.1"|"13.2"|"13.3"|"13.4"|"13.5"|"13.6"|"13.7")
        MAC_OS_NAME="Ventura"
        ;;
    "14.0"|"14.1"|"14.2"|"14.3"|"14.4"|"14.5"|"14.6"|"14.7")
        MAC_OS_NAME="Sonoma"
        ;;
    "15.0"|"15.1"|"15.2"|"15.3"|"15.4"|"15.5"|"15.6"|"15.7")
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

## Ensure we're using the latest formula
#brew update

# Install the stuff we know needs to get installed all the time
brew install \
    gcc \
    python3 \
    boost-python3 \
    jpeg \
    libpng \
    gtk+3 \
    gtkglext \
    sdl3 \
    sdl3_image \
    ninja \
    abseil \
    nlohmann-json \
    protobuf-c \
    re2 \
    grpc \
    autoconf \
    autoconf-archive \
    automake \
    libtool

# Reinstall some key items because it might be problematic otherwise
brew reinstall \
    libarchive

# Only install cmake if it isn't installed yet
brew ls --versions cmake || brew install cmake

# The following Apple-provided libraries are deprecated:
# * OpenGL as of macOS 10.14
# * GLUT as of macOS 10.9
brew install mesa-glu freeglut

# Keep these lines! Otherwise you will likely get PFNGL... not found errors.
if [ ! -d "$(brew --prefix)/include/OpenGL" ]
then
  ln -s "$(brew --prefix)/include/GL" "$(brew --prefix)/include/OpenGL"
fi
if [ ! -d "$(brew --prefix)/include/GLUT" ]
then
  ln -s "$(brew --prefix)/include/GL" "$(brew --prefix)/include/GLUT"
fi

# MacOS 13+ needs Vorbis support
brew install vorbis-tools

# The Apple-provided OpenAL is deprecated as of macOS 10.15
brew install openal-soft freealut

if [ -z "$VCPKG_ROOT" ]
then
    export VCPKG_ROOT="$(pwd)/../vcpkg"
fi

git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
export PATH="$VCPKG_ROOT:$PATH"

pushd "$VCPKG_ROOT"
./bootstrap-vcpkg.sh
popd
