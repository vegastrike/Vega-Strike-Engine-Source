#!/usr/bin/env bash
#====================================
# @file   : brew-install-for-cmake.sh
# @brief  : installs a package with Homebrew, keg-only, then registers that package's
#           include, lib, bin, etc. directories for use by CMake
# @usage  : script/brew-install-for-cmake.sh <package>
# @param  : first, the brew formula name, optionally including a version suffix
#           second, a truthy value indicating whether or not to leave it linked
#====================================
# Copyright (C) 2023 Stephen G. Tuggy and other Vega Strike contributors
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

set -e

PACKAGE_SPEC="$1"
LEAVE_LINKED="$2"

HOMEBREW_NO_AUTO_UPDATE=1 HOMEBREW_NO_INSTALL_UPGRADE=1 brew install "$PACKAGE_SPEC"
if [ ! "$LEAVE_LINKED" ] ; then
  HOMEBREW_NO_AUTO_UPDATE=1 HOMEBREW_NO_INSTALL_UPGRADE=1 brew unlink "$PACKAGE_SPEC"
fi
PACKAGE_INSTALLED_PREFIX=$(HOMEBREW_NO_AUTO_UPDATE=1 HOMEBREW_NO_INSTALL_UPGRADE=1 brew --prefix "$PACKAGE_SPEC")
PACKAGE_INSTALLED_BIN="$PACKAGE_INSTALLED_PREFIX/bin"
#PACKAGE_INSTALLED_INCLUDE="$PACKAGE_INSTALLED_PREFIX/include"
#PACKAGE_INSTALLED_LIB="$PACKAGE_INSTALLED_PREFIX/lib"
. export PATH="$PACKAGE_INSTALLED_BIN:$PATH"
. export CMAKE_PREFIX_PATH="$PACKAGE_INSTALLED_PREFIX:$CMAKE_PREFIX_PATH"
