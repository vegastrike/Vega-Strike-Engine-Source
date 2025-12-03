#!/usr/bin/env bash
##
# docker-entrypoint.sh
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


set -e

echo "docker-entrypoint.sh | 2025-11-29 | Flags passed in: $FLAGS"

# shellcheck disable=SC2086
if [ $IS_RELEASE -eq 1 ]
then
    script/build -DCMAKE_BUILD_TYPE=RelWithDebInfo "$FLAGS"
    script/package "$FLAGS"
else
    if [ "$CC" == "clang" ]
    then
        script/build -DCMAKE_BUILD_TYPE=RelWithDebInfo "$FLAGS"
    else
        script/build -DCMAKE_BUILD_TYPE=Debug "$FLAGS"
    fi
fi

pushd build
GTEST_OUTPUT=xml:test-results ctest -V
popd
