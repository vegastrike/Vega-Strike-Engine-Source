#!/usr/bin/env bash
#====================================
# @file   : bootstrap-on-linux.sh
# @brief  : installs dependencies for building Vega Strike on Linux
# @usage  : sudo script/bootstrap-on-linux.sh 1 (to update all installed
#           packages on the system in the process)
#     or  : sudo script/bootstrap-on-linux.sh 0 (to skip updating, and
#           just install the new packages that are needed)
# @param  : just one parameter, either a 1 or a 0, to indicate whether or not to
#           UPDATE_ALL_SYSTEM_PACKAGES
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

echo "------------------------------------------"
echo "--- bootstrap-on-linux.sh | 2025-11-29 ---"
echo "------------------------------------------"

UPDATE_ALL_SYSTEM_PACKAGES="$1"

if [ -f /etc/os-release ]
then
    OS_RELEASE_LOCATION="/etc/os-release"
elif [ -f /usr/lib/os-release ]
then
    OS_RELEASE_LOCATION="/usr/lib/os-release"
else
    echo "os-release file not found; unable to continue"
    exit 1
fi
LINUX_ID=$(grep ^ID= $OS_RELEASE_LOCATION | sed 's/^ID=//' | tr -d '"\n')
echo "LINUX_ID = ${LINUX_ID}"
LINUX_CODENAME=$(grep ^VERSION_CODENAME= $OS_RELEASE_LOCATION | sed 's/^VERSION_CODENAME=//' | tr -d '"\n')
echo "LINUX_CODENAME = ${LINUX_CODENAME}"
LINUX_VERSION_ID=$(grep ^VERSION_ID= $OS_RELEASE_LOCATION | sed 's/^VERSION_ID=//' | tr -d '"\n')
echo "LINUX_VERSION_ID = ${LINUX_VERSION_ID}"

function bootstrapOnDebian()
{
    apt-get update

    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        apt-get -qy upgrade
    fi

    case "$LINUX_CODENAME" in
        "trixie")
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg62-turbo-dev \
                            libexpat1-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libsdl3-dev \
                            libsdl3-image-dev \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            opentelemetry-cpp-dev \
                            libprotobuf-dev \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "bookworm")
            echo "Bookworm does NOT support SDL3"
            apt-get -qy remove \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev
            apt-get -qy autoremove
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg62-turbo-dev \
                            libexpat1-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python1.81-dev \
                            libboost-log1.81-dev \
                            libboost-regex1.81-dev \
                            libboost-json1.81-dev \
                            libboost-program-options1.81-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "bullseye"|"buster"|"stretch")
            echo "Sorry, Debian ${LINUX_CODENAME} is no longer supported"
            exit 2
            ;;
        *)
            echo "Sorry, this version of Debian is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnUbuntu()
{
    apt-get update

    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        apt-get -qy upgrade
    fi

    case "$LINUX_CODENAME" in
        "questing")
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
                            libexpat1-dev \
                            libgdk-pixbuf-2.0-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libsdl3-dev \
                            libsdl3-image-dev \
                            libopengl0 \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "plucky")
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
                            libexpat1-dev \
                            libgdk-pixbuf-2.0-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libsdl3-dev \
                            libsdl3-image-dev \
                            libopengl0 \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "noble")
            echo "Noble does NOT support SDL3"
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
                            libexpat1-dev \
                            libgdk-pixbuf-2.0-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libopengl0 \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "jammy"|"hirsute"|"impish"|"focal"|"bionic"|"xenial")
            echo "Sorry, Ubuntu ${LINUX_CODENAME} is no longer supported"
            exit 2
            ;;
        *)
            echo "Sorry, this version of Ubuntu is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnPopOS ()
{
    apt-get update

    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        apt-get -qy upgrade
    fi

    case "$LINUX_CODENAME" in
        "jammy")
            echo "Sorry, Pop! OS jammy is no longer supported"
            exit 2
            ;;
        "noble")
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
                            libexpat1-dev \
                            libgdk-pixbuf-2.0-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libsdl3-dev \
                            libsdl3-image-dev \
                            libopengl0 \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        *)
            echo "Sorry, this version of Pop! OS is not currently supported"
            exit 2
            ;;
    esac
}

function bootstrapOnLinuxMint ()
{
    apt-get update

    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        apt-get -qy upgrade
    fi

    echo "Linux Mint base Distros do NOT support SDL3"
    case "$LINUX_CODENAME" in
        "wilma"|"zara")
            apt-get -qy install \
                            git \
                            cmake \
                            python3-dev \
                            build-essential \
                            automake \
                            autoconf \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
                            libexpat1-dev \
                            libgdk-pixbuf-2.0-dev \
                            libgtk-3-dev \
                            libopenal-dev \
                            libogg-dev \
                            libvorbis-dev \
                            libglvnd-dev \
                            libgl1-mesa-dev \
                            libsdl2-dev \
                            libsdl2-image-dev \
                            libopengl0 \
                            libpostproc-dev \
                            freeglut3-dev \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            libxmu-dev \
                            clang \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            gnome-desktop-testing \
                            libasound2-dev \
                            libpulse-dev \
                            libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
                            libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
                            libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
                            libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev \
                            libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev
            ;;
        "virginia"|"victoria"|"vera"|"vanessa"|"ulyana")
            echo "Sorry, Linux Mint ${LINUX_CODENAME} is no longer supported"
            exit 2
            ;;
        *)
            echo "This version of Linux Mint is not directly supported. You may be able to use the corresponding Ubuntu installation package"
            exit 2
            ;;
    esac
}

function bootstrapOnOpenSuseLeap ()
{
    case "${LINUX_VERSION_ID}" in
        "15.1"|"15.2"|"15.3"|"15.4"|"15.5")
            echo "Sorry, openSUSE Leap ${LINUX_VERSION_ID} is no longer supported"
            exit 2
            ;;
        "15.6")
            zypper --non-interactive refresh
            zypper --non-interactive install -y \
                                    libboost_log1_75_0-devel \
                                    libboost_python-py3-1_75_0-devel \
                                    libboost_system1_75_0-devel \
                                    libboost_filesystem1_75_0-devel \
                                    libboost_thread1_75_0-devel \
                                    libboost_regex1_75_0-devel \
                                    libboost_chrono1_75_0-devel \
                                    libboost_atomic1_75_0-devel \
                                    libboost_json1_75_0-devel \
                                    libboost_container1_75_0-devel \
                                    libboost_program_options1_75_0-devel \
                                    cmake \
                                    gcc-c++ \
                                    freeglut-devel \
                                    libopenal0 \
                                    openal-soft-devel \
                                    libSDL2-devel \
                                    libSDL2_image-devel \
                                    libvorbis-devel \
                                    libglvnd-devel \
                                    libjpeg-turbo \
                                    libjpeg62-devel \
                                    libpng16-devel \
                                    libarchive-devel \
                                    expat \
                                    libexpat-devel \
                                    libgtk-3-0 \
                                    gtk3-devel \
                                    python3-devel \
                                    git \
                                    rpm-build \
                                    clang \
                                    libunwind-devel libusb-1_0-devel Mesa-libGL-devel libxkbcommon-devel libdrm-devel \
                                    libgbm-devel pipewire-devel libpulse-devel sndio-devel Mesa-libEGL-devel alsa-devel xwayland-devel \
                                    wayland-devel wayland-protocols-devel
            ;;
        "16.0")
            zypper --non-interactive refresh
            zypper --non-interactive install -y \
                                    libboost_log1_86_0-devel \
                                    libboost_python-py3-1_86_0-devel \
                                    libboost_system1_86_0-devel \
                                    libboost_filesystem1_86_0-devel \
                                    libboost_thread1_86_0-devel \
                                    libboost_regex1_86_0-devel \
                                    libboost_chrono1_86_0-devel \
                                    libboost_atomic1_86_0-devel \
                                    libboost_json1_86_0-devel \
                                    libboost_container1_86_0-devel \
                                    libboost_program_options1_86_0-devel \
                                    cmake \
                                    gcc-c++ \
                                    freeglut-devel \
                                    libopenal0 \
                                    openal-soft-devel \
                                    libSDL2-devel \
                                    libSDL2_image-devel \
                                    libvorbis-devel \
                                    libglvnd-devel \
                                    libjpeg-turbo \
                                    libjpeg62-devel \
                                    libpng16-devel \
                                    libarchive-devel \
                                    expat \
                                    libexpat-devel \
                                    libgtk-3-0 \
                                    gtk3-devel \
                                    python3-devel \
                                    git \
                                    rpm-build \
                                    clang \
                                    libunwind-devel libusb-1_0-devel Mesa-libGL-devel libxkbcommon-devel libdrm-devel \
                                    libgbm-devel pipewire-devel libpulse-devel sndio-devel Mesa-libEGL-devel alsa-devel xwayland-devel \
                                    wayland-devel wayland-protocols-devel
            ;;
        *)
            echo "Sorry, this version of openSUSE Leap is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnFedora ()
{
    case "${LINUX_VERSION_ID}" in
        30|31|32|33|34|35|36|37|38|39)
            echo "Sorry, Fedora ${LINUX_VERSION_ID} is no longer supported"
            exit 2
            ;;
        40)
            dnf install -y \
                                git \
                                cmake \
                                boost-devel \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        41)
            dnf install -y \
                                git \
                                cmake \
                                boost-devel \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        42)
            dnf install -y \
                                git \
                                cmake \
                                boost-devel \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL3-devel \
                                SDL3_image-devel \
                                sdl2-compat-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        43)
            dnf install -y \
                                git \
                                cmake \
                                boost-devel \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL3-devel \
                                SDL3_image-devel \
                                sdl2-compat-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        *)
            echo "Sorry, this version of Fedora is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnRedHat ()
{
    case "${LINUX_VERSION_ID}" in
        "9.5")
            dnf -y install dnf-plugins-core
            dnf config-manager --set-enabled crb
            dnf config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        "9.6")
            dnf -y install dnf-plugins-core
            dnf config-manager --set-enabled crb
            dnf config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        "10.0"|"10.1")
            dnf -y upgrade --refresh
            dnf -y install 'dnf-command(config-manager)'
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL3-devel \
                                SDL2-compat-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        *)
            echo "Sorry, this version of Red Hat is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnRockyLinux ()
{
    case "${LINUX_VERSION_ID}" in
        "9.5")
            dnf -y install dnf-plugins-core
            dnf config-manager --set-enabled crb
            dnf config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        "9.6")
            dnf -y install dnf-plugins-core
            dnf config-manager --set-enabled crb
            dnf config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL2-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        "10.0"|"10.1")
            declare -a pkgs_to_uninstall=('SDL2_image-devel' 'SDL2_image' 'SDL2-devel' 'SDL2')
            for pkg in "${pkgs_to_uninstall[@]}"
            do
                if dnf list installed | grep -qF "$pkg"; then
                    dnf -y remove "$pkg"
                else
                    echo "Package '$pkg' is not installed."
                fi
            done

            dnf -y upgrade --refresh
            dnf -y install 'dnf-command(config-manager)'
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
                                gcc-c++ \
                                openal-soft-devel \
                                SDL3-devel \
                                sdl2-compat-devel \
                                SDL2_image-devel \
                                libvorbis-devel \
                                libglvnd-devel \
                                libjpeg-turbo-devel \
                                libpng-devel \
                                expat-devel \
                                gtk3-devel \
                                python3-devel \
                                libarchive-devel \
                                rpm-build \
                                make \
                                clang \
                                alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
                                libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
                                libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
                                mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
                                mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
                                libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
                                pipewire-jack-audio-connection-kit-devel \
                                liburing-devel
            ;;
        *)
            echo "Sorry, this version of Rocky Linux is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnManjaro ()
{
        pacman -Sy --noconfirm archlinux-keyring manjaro-keyring

        if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
        then
            pacman -Syyu --refresh --noconfirm
        fi

        pacman -Sy --noconfirm cmake \
                         boost \
                         clang \
                         gcc \
                         gcc-libs \
                         sdl3 \
                         sdl2-compat \
                         sdl2_image \
                         expat \
                         gtk3 \
                         libglvnd \
                         mesa \
                         python \
                         autoconf \
                         automake \
                         freeglut \
                         git \
                         libjpeg-turbo \
                         libpng \
                         libvorbis \
                         libxmu \
                         openal \
                         libarchive \
                         yay \
                         make \
                         alsa-lib hidapi ibus jack libdecor libgl libpulse libusb libx11 libxcursor libxext libxinerama \
                         libxkbcommon libxrandr libxrender libxss libxtst mesa ninja pipewire sndio vulkan-driver vulkan-headers \
                         wayland wayland-protocols

        # NOTE: `yay` requires SystemD which isn't available in Docker
        # AUR related packages that are not in pacman by default
        # yay -S --noconfirm sdl3_image
}

function bootstrapOnFuntoo ()
{
    ego sync
    dispatch-conf
    # enable `autounmask-write` so that USE flags
    # change in the image appropriately
    USE="-libffi -userland_GNU gles2 X" emerge --autounmask-write \
              cmake \
              boost \
              libsdl2 \
              libsdl2-image \
              libsdl3 \
              libsdl3image \
              expat \
              gtk3 \
              libglvnd \
              mesa \
              python \
              autoconf \
              automake \
              freeglut \
              git \
              libjpeg-turbo \
              libpng \
              libvorbis \
              libXmu \
              openal \
              libarchive \
              make \
              x11-libs/gtk+
}

function bootstrapOnArch ()
{
    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        pacman -Syyu --refresh --noconfirm
    fi

    # NOTE: Arch requires GCC 12 right now
    # also installing latest GCC.
    pacman -Sy --noconfirm \
              base-devel \
              cmake \
              boost \
              llvm \
              clang \
              gcc \
              gcc12 \
              sdl3 \
              sdl3_image \
              sdl2-compat \
              sdl2_image \
              expat \
              gtk3 \
              libglvnd \
              mesa \
              python \
              freeglut \
              git \
              libjpeg-turbo \
              libpng \
              libvorbis \
              libxmu \
              openal \
              libarchive \
              make \
              alsa-lib hidapi ibus jack libdecor libgl libpulse libusb libx11 libxcursor libxext libxinerama \
              libxkbcommon libxrandr libxrender libxss libxtst mesa ninja pipewire sndio vulkan-driver vulkan-headers \
              wayland wayland-protocols
}

function bootstrapOnEndeavourOS ()
{
    if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
    then
        pacman -Syyu --refresh --noconfirm
    fi

    # NOTE: Arch requires GCC 12 right now
    # also installing latest GCC.
    pacman -Sy --noconfirm \
              base-devel \
              icu \
              cmake \
              boost \
              llvm \
              clang \
              gcc \
              sdl2 \
              sdl2_image \
              sdl3 \
              expat \
              gtk3 \
              libglvnd \
              mesa \
              python \
              freeglut \
              git \
              libjpeg-turbo \
              libpng \
              libvorbis \
              libxmu \
              openal \
              libarchive \
              make \
              alsa-lib hidapi ibus jack libdecor libgl libpulse libusb libx11 libxcursor libxext libxinerama \
              libxkbcommon libxrandr libxrender libxss libxtst mesa ninja pipewire sndio vulkan-driver vulkan-headers \
              wayland wayland-protocols
}

case "${LINUX_ID}" in
    "debian")
        bootstrapOnDebian
        ;;
    "ubuntu")
        bootstrapOnUbuntu
        ;;
    "pop")
        bootstrapOnPopOS
        ;;
    "linuxmint")
        bootstrapOnLinuxMint
        ;;
    "opensuse-leap")
        bootstrapOnOpenSuseLeap
        ;;
    "fedora")
        bootstrapOnFedora
        ;;
    "rhel")
        bootstrapOnRedHat
        ;;
    "redhat")
        bootstrapOnRedHat
        ;;
    "rocky")
        bootstrapOnRockyLinux
        ;;
    "manjaro")
        bootstrapOnManjaro
        ;;
    "funtoo")
        bootstrapOnFuntoo
        ;;
    "arch")
        bootstrapOnArch
        ;;
    "endeavouros")
        bootstrapOnEndeavourOS
        ;;
    *)
        echo "Sorry, unrecognized/unsupported Linux distribution"
        exit 2
        ;;
esac

mkdir -p /usr/src/Vega-Strike-Engine-Source

echo "Bootstrapping finished!"
