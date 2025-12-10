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
echo "--- bootstrap-on-linux.sh | 2025-12-08 ---"
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
        "trixie"|"bookworm")
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
                            libxcb-xv0-dev
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
        "questing"|"plucky"|"noble")
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
                            libxcb-xv0-dev
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
                            libxcb-xv0-dev
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
                                    libX11-devel
            ;;
        "16.0")
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
                                    python313-Jinja2
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
        30|31|32|33|34|35|36|37|38|39|40|41)
            echo "Sorry, Fedora ${LINUX_VERSION_ID} is no longer supported"
            exit 2
            ;;
        42|43)
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
                                python3-jinja2
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
        "9.6")
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
                                python3-jinja2
            ;;
        "10.0"|"10.1")
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
                                python3-jinja2
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
        "9.6")
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
                                python3-jinja2
            ;;
        "10.0"|"10.1")
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
                                python3-jinja2
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
                         tar
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
              make
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
              tar
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
              tar
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

if [ -z "$VCPKG_ROOT" ]
then
    export VCPKG_ROOT="$(pwd)/../vcpkg"
fi

git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
export PATH="$VCPKG_ROOT:$PATH"

pushd "$VCPKG_ROOT"
./bootstrap-vcpkg.sh
popd

echo "Bootstrapping finished!"
