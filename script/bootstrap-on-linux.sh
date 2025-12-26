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
echo "--- bootstrap-on-linux.sh | 2025-12-26 ---"
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
                            build-essential \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            libx11-dev \
                            libxext-dev \
                            libxfixes-dev \
                            libxi-dev \
                            libxmu-dev \
                            libxrandr-dev \
                            libxss-dev \
                            libxtst-dev \
                            libwayland-dev \
                            libxkbcommon-dev \
                            wayland-protocols \
                            libibus-1.0-dev \
                            python3-jinja2 \
                            libx11-xcb-dev \
                            libxcb-dpms0-dev \
                            libxxf86vm-dev \
                            libxcb-xv0-dev \
                            python3-dev \
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
                            clang \
                            opentelemetry-cpp-dev \
                            libprotobuf-dev \
                            libaudio-dev \
                            libfribidi-dev \
                            libglu1-mesa-dev \
                            mesa-common-dev \
                            libltdl-dev
            ;;
        "bookworm")
            echo "Bookworm does NOT support SDL3"
            apt-get -qy remove \
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-program-options-dev \
                            libboost-regex-dev
            apt-get -qy autoremove
            apt-get -qy install \
                            git \
                            cmake \
                            build-essential \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            libx11-dev \
                            libxext-dev \
                            libxfixes-dev \
                            libxi-dev \
                            libxmu-dev \
                            libxrandr-dev \
                            libxss-dev \
                            libxtst-dev \
                            libwayland-dev \
                            libxkbcommon-dev \
                            wayland-protocols \
                            libibus-1.0-dev \
                            python3-jinja2 \
                            libx11-xcb-dev \
                            libxcb-dpms0-dev \
                            libxxf86vm-dev \
                            libxcb-xv0-dev \
                            python3-dev \
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
                            clang \
                            libprotobuf-dev \
                            libaudio-dev \
                            libfribidi-dev \
                            libglu1-mesa-dev \
                            mesa-common-dev \
                            libltdl-dev
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
        "questing"|"plucky")
            apt-get -qy install \
                            git \
                            cmake \
                            build-essential \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            libx11-dev \
                            libxext-dev \
                            libxfixes-dev \
                            libxi-dev \
                            libxmu-dev \
                            libxrandr-dev \
                            libxss-dev \
                            libxtst-dev \
                            libwayland-dev \
                            libxkbcommon-dev \
                            wayland-protocols \
                            libibus-1.0-dev \
                            python3-jinja2 \
                            libx11-xcb-dev \
                            libxcb-dpms0-dev \
                            libxxf86vm-dev \
                            libxcb-xv0-dev \
                            python3-dev \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
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
                            clang \
                            opentelemetry-cpp-dev \
                            libprotobuf-dev \
                            libaudio-dev \
                            libfribidi-dev \
                            libglu1-mesa-dev \
                            mesa-common-dev \
                            libltdl-dev
            ;;
        "noble")
            echo "Noble does NOT support SDL3"
            apt-get -qy install \
                            git \
                            cmake \
                            build-essential \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            libx11-dev \
                            libxext-dev \
                            libxfixes-dev \
                            libxi-dev \
                            libxmu-dev \
                            libxrandr-dev \
                            libxss-dev \
                            libxtst-dev \
                            libwayland-dev \
                            libxkbcommon-dev \
                            wayland-protocols \
                            libibus-1.0-dev \
                            python3-jinja2 \
                            libx11-xcb-dev \
                            libxcb-dpms0-dev \
                            libxxf86vm-dev \
                            libxcb-xv0-dev \
                            python3-dev \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
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
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            clang \
                            libprotobuf-dev \
                            libaudio-dev \
                            libfribidi-dev \
                            libglu1-mesa-dev \
                            mesa-common-dev \
                            libltdl-dev
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
                            build-essential \
                            lsb-release \
                            make \
                            pkg-config \
                            ninja-build \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            libx11-dev \
                            libxext-dev \
                            libxfixes-dev \
                            libxi-dev \
                            libxmu-dev \
                            libxrandr-dev \
                            libxss-dev \
                            libxtst-dev \
                            libwayland-dev \
                            libxkbcommon-dev \
                            wayland-protocols \
                            libibus-1.0-dev \
                            python3-jinja2 \
                            libx11-xcb-dev \
                            libxcb-dpms0-dev \
                            libxxf86vm-dev \
                            libxcb-xv0-dev \
                            python3-dev \
                            libarchive-dev \
                            libpng16-16 \
                            libpng-dev \
                            libpng-tools \
                            libjpeg-turbo8-dev \
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
                            libboost-python-dev \
                            libboost-log-dev \
                            libboost-regex-dev \
                            libboost-json-dev \
                            libboost-program-options-dev \
                            clang \
                            libprotobuf-dev \
                            libaudio-dev \
                            libfribidi-dev \
                            libglu1-mesa-dev \
                            mesa-common-dev \
                            libltdl-dev
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
                                    cmake \
                                    gcc-c++ \
                                    git \
                                    rpm-build \
                                    autoconf \
                                    autoconf-archive \
                                    automake \
                                    libtool \
                                    curl \
                                    zip \
                                    unzip \
                                    tar \
                                    libX11-devel \
                                    xextproto-devel \
                                    libXfixes-devel \
                                    libXi-devel \
                                    libXmu-devel \
                                    libXrandr-devel \
                                    libXss-devel \
                                    libXtst-devel \
                                    wayland-devel \
                                    libxkbcommon-devel \
                                    wayland-protocols-devel \
                                    ibus-devel \
                                    python3-Jinja2 \
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
                                    clang \
                                    glu-devel \
                                    Mesa-devel \
                                    libtool \
                                    libltdl7
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
                                    git \
                                    rpm-build \
                                    autoconf \
                                    autoconf-archive \
                                    automake \
                                    libtool \
                                    curl \
                                    zip \
                                    unzip \
                                    tar \
                                    libX11-devel \
                                    xorgproto-devel \
                                    libXfixes-devel \
                                    libXi-devel \
                                    libXmu-devel \
                                    libXrandr-devel \
                                    libXss-devel \
                                    libXtst-devel \
                                    wayland-devel \
                                    libxkbcommon-devel \
                                    wayland-protocols-devel \
                                    ibus-devel \
                                    python313-Jinja2 \
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
                                    clang \
                                    glu-devel \
                                    Mesa-devel \
                                    libtool \
                                    libltdl7
            ;;
        *)
            echo "Sorry, this version of openSUSE Leap is unsupported"
            exit 2
            ;;
    esac
}

function bootstrapOnFedora ()
{
    export fedoraVersion=${LINUX_VERSION_ID}
    export fedoraMaxSupportedVersion=43
    export fedoraMinSupportedVersion=42
    if [ ${fedoraVersion} -gt ${fedoraMaxSupportedVersion} ]
    then
        echo "Fedora Version ${fedoraVersion} is not yet supported. Pull requests welcome"
    elif [ ${fedoraVersion} -ge ${fedoraMinSupportedVersion} ]
    then
        declare -a pkgs_to_uninstall=('SDL2-devel' 'SDL2')
        for pkg in "${pkgs_to_uninstall[@]}"
        do
            if dnf list installed | grep -qF "$pkg"; then
                dnf -y remove "$pkg"
            else
                echo "Package '$pkg' is not installed."
            fi
        done

        if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
        then
            dnf -y upgrade --refresh
        fi
        dnf install -y \
                            git \
                            cmake \
                            gcc-c++ \
                            rpm-build \
                            make \
                            autoconf \
                            autoconf-archive \
                            automake \
                            libtool \
                            curl \
                            zip \
                            unzip \
                            tar \
                            kernel-headers \
                            perl \
                            libX11-devel \
                            xorg-x11-proto-devel \
                            libXfixes-devel \
                            libXi-devel \
                            libXmu-devel \
                            libXrandr-devel \
                            libXtst-devel \
                            wayland-devel \
                            libxkbcommon-devel \
                            wayland-protocols-devel \
                            ibus-devel \
                            python3-jinja2 \
                            boost-devel \
                            freeglut-devel \
                            openal-soft-devel \
                            libvorbis-devel \
                            libglvnd-devel \
                            libjpeg-turbo-devel \
                            libpng-devel \
                            expat-devel \
                            gtk3-devel \
                            python3-devel \
                            libarchive-devel \
                            rpm-build \
                            clang \
                            fribidi-devel \
                            mesa-libGLU-devel \
                            SDL3-devel \
                            SDL3_image-devel \
                            sdl2-compat-devel \
                            SDL2_image-devel \
                            libtool-ltdl-devel
    else
        echo "Sorry, Fedora ${LINUX_VERSION_ID} is no longer supported"
        exit 2
    fi
}

function bootstrapOnRedHat ()
{
    case "${LINUX_VERSION_ID}" in
        "9.6"|"9.7")
            if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
            then
                dnf -y upgrade --refresh
            fi
            dnf -y install dnf-plugins-core
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                gcc-c++ \
                                rpm-build \
                                make \
                                autoconf \
                                autoconf-archive \
                                automake \
                                libtool \
                                curl-minimal \
                                zip \
                                unzip \
                                tar \
                                kernel-headers \
                                perl \
                                libX11-devel \
                                xorg-x11-proto-devel \
                                libXfixes-devel \
                                libXi-devel \
                                libXmu-devel \
                                libXrandr-devel \
                                libXtst-devel \
                                wayland-devel \
                                libxkbcommon-devel \
                                wayland-protocols-devel \
                                ibus-devel \
                                python3-jinja2 \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
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
                                clang \
                                fribidi-devel \
                                mesa-libGLU-devel \
                                libtool-ltdl-devel
            ;;
        "10.0"|"10.1")
            declare -a pkgs_to_uninstall=('SDL2-devel' 'SDL2')
            for pkg in "${pkgs_to_uninstall[@]}"
            do
                if dnf list installed | grep -qF "$pkg"; then
                    dnf -y remove "$pkg"
                else
                    echo "Package '$pkg' is not installed."
                fi
            done

            if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
            then
                dnf -y upgrade --refresh
            fi
            dnf -y install 'dnf-command(config-manager)'
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                gcc-c++ \
                                rpm-build \
                                make \
                                autoconf \
                                autoconf-archive \
                                automake \
                                libtool \
                                curl \
                                zip \
                                unzip \
                                tar \
                                kernel-headers \
                                perl \
                                libX11-devel \
                                xorg-x11-proto-devel \
                                libXfixes-devel \
                                libXi-devel \
                                libXmu-devel \
                                libXrandr-devel \
                                libXtst-devel \
                                wayland-devel \
                                libxkbcommon-devel \
                                wayland-protocols-devel \
                                ibus-devel \
                                python3-jinja2 \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
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
                                clang \
                                fribidi-devel \
                                mesa-libGLU-devel \
                                libtool-ltdl-devel
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
        "9.6"|"9.7")
            if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
            then
                dnf -y upgrade --refresh
            fi
            dnf -y install dnf-plugins-core
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install epel-release
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                gcc-c++ \
                                rpm-build \
                                make \
                                autoconf \
                                autoconf-archive \
                                automake \
                                libtool \
                                curl-minimal \
                                zip \
                                unzip \
                                tar \
                                kernel-headers \
                                perl \
                                libX11-devel \
                                xorg-x11-proto-devel \
                                libXfixes-devel \
                                libXi-devel \
                                libXmu-devel \
                                libXrandr-devel \
                                libXtst-devel \
                                wayland-devel \
                                libxkbcommon-devel \
                                wayland-protocols-devel \
                                ibus-devel \
                                python3-jinja2 \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
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
                                clang \
                                fribidi-devel \
                                mesa-libGLU-devel \
                                libtool-ltdl-devel
            ;;
        "10.0"|"10.1")
            declare -a pkgs_to_uninstall=('SDL2-devel' 'SDL2')
            for pkg in "${pkgs_to_uninstall[@]}"
            do
                if dnf list installed | grep -qF "$pkg"; then
                    dnf -y remove "$pkg"
                else
                    echo "Package '$pkg' is not installed."
                fi
            done

            if [ "${UPDATE_ALL_SYSTEM_PACKAGES}" -eq 1 ]
            then
                dnf -y upgrade --refresh
            fi
            dnf -y install 'dnf-command(config-manager)'
            dnf -y config-manager --set-enabled crb
            dnf -y config-manager --set-enabled devel
            dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
            dnf -y update
            dnf -y install \
                                git \
                                cmake \
                                gcc-c++ \
                                rpm-build \
                                make \
                                autoconf \
                                autoconf-archive \
                                automake \
                                libtool \
                                curl \
                                zip \
                                unzip \
                                tar \
                                kernel-headers \
                                perl \
                                libX11-devel \
                                xorg-x11-proto-devel \
                                libXfixes-devel \
                                libXi-devel \
                                libXmu-devel \
                                libXrandr-devel \
                                libXtst-devel \
                                wayland-devel \
                                libxkbcommon-devel \
                                wayland-protocols-devel \
                                ibus-devel \
                                python3-jinja2 \
                                boost-devel \
                                boost-python3-devel \
                                boost-json \
                                freeglut-devel \
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
                                clang \
                                fribidi-devel \
                                mesa-libGLU-devel \
                                libtool-ltdl-devel
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

        pacman -Sy --noconfirm base-devel --needed
        pacman -Sy --noconfirm cmake \
                         gcc \
                         gcc-libs \
                         git \
                         make \
                         autoconf \
                         autoconf-archive \
                         automake \
                         libtool \
                         curl \
                         zip \
                         unzip \
                         tar \
                         boost \
                         sdl3 \
                         sdl2-compat \
                         sdl2_image \
                         expat \
                         gtk3 \
                         libglvnd \
                         mesa \
                         python \
                         freeglut \
                         libjpeg-turbo \
                         libpng \
                         libvorbis \
                         libxmu \
                         openal \
                         libarchive \
                         yay \
                         hidapi \
                         libgl \
                         libusb \
                         ninja \
                         wayland \
                         wayland-protocols \
                         glu

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
    USE="-libffi -userland_GNU" emerge --autounmask-write \
              cmake \
              boost \
              python \
              git \
              make \
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
              gcc \
              gcc12 \
              git \
              make \
              autoconf \
              autoconf-archive \
              automake \
              libtool \
              curl \
              zip \
              unzip \
              tar \
              boost \
              llvm \
              clang \
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
              libjpeg-turbo \
              libpng \
              libvorbis \
              libxmu \
              openal \
              libarchive \
              hidapi \
              libgl \
              libusb \
              ninja \
              wayland \
              wayland-protocols \
              glu
}

function bootstrapOnEndeavourOS ()
{
    pacman -Sy --noconfirm archlinux-keyring

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
              gcc \
              git \
              make \
              autoconf \
              autoconf-archive \
              automake \
              libtool \
              curl \
              zip \
              unzip \
              tar \
              boost \
              llvm \
              clang \
              sdl2_image \
              sdl3 \
              expat \
              gtk3 \
              libglvnd \
              mesa \
              python \
              freeglut \
              libjpeg-turbo \
              libpng \
              libvorbis \
              libxmu \
              openal \
              libarchive \
              hidapi \
              libgl \
              libusb \
              ninja \
              wayland \
              wayland-protocols \
              glu
}

case "${LINUX_ID}" in
    "debian")
        bootstrapOnDebian
        ;;
    "ubuntu")
        bootstrapOnUbuntu
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

mkdir -p /usr/local/src/Vega-Strike-Engine-Source

echo "Bootstrapping finished!"
