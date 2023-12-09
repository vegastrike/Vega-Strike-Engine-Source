#!/bin/bash
#
# bootstrap-on-macos.sh
#
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

export HOMEBREW_NO_INSTALL_UPGRADE=1
export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1

brew bundle install --file=./Brewfile --no-upgrade

declare -a keg_only_packages=("python@3.12" "boost" "boost-python3")

for i in "${keg_only_packages[@]}"
do
    PACKAGE_INSTALLED_PREFIX=$(brew --prefix "$i")
    PACKAGE_INSTALLED_BIN="$PACKAGE_INSTALLED_PREFIX/bin"
    export PATH="$PACKAGE_INSTALLED_BIN:$PATH"
    export CMAKE_PREFIX_PATH="$PACKAGE_INSTALLED_PREFIX:$CMAKE_PREFIX_PATH"
done

ln -s /usr/local/include/GL /usr/local/include/OpenGL
ln -s /usr/local/include/GL /usr/local/include/GLUT
export OPENALDIR=$(brew --prefix openal-soft)
