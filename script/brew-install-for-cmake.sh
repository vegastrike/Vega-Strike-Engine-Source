#!/usr/bin/env bash
##
# brew-install-for-cmake.sh
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Creator: Daniel Horn
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

set -e

PACKAGE_SPEC="$1"
LEAVE_LINKED="$2"

#export HOMEBREW_NO_AUTO_UPDATE=1
#export HOMEBREW_NO_INSTALL_UPGRADE=1
#export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1

if ! brew list "$PACKAGE_SPEC" &>/dev/null; then
  if "$LEAVE_LINKED"; then
    brew install --force --overwrite "$PACKAGE_SPEC"
  else
    brew install "$PACKAGE_SPEC"
    brew unlink "$PACKAGE_SPEC"
  fi
fi
PACKAGE_INSTALLED_PREFIX=$(brew --prefix "$PACKAGE_SPEC")
PACKAGE_INSTALLED_BIN="$PACKAGE_INSTALLED_PREFIX/bin"
#PACKAGE_INSTALLED_INCLUDE="$PACKAGE_INSTALLED_PREFIX/include"
#PACKAGE_INSTALLED_LIB="$PACKAGE_INSTALLED_PREFIX/lib"
export PATH="$PACKAGE_INSTALLED_BIN:$PATH"
export CMAKE_PREFIX_PATH="$PACKAGE_INSTALLED_PREFIX:$CMAKE_PREFIX_PATH"
