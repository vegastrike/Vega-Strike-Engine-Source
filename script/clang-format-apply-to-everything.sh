#!/usr/bin/env bash
##
# clang-format-apply-to-everything.sh
#
# Copyright (C) 2026 Stephen G. Tuggy, David Wales, Benjamen R. Meyer,
# Roy Falk, Evert Vorster, and other Vega Strike contributors
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

set -euo pipefail

declare -a file_extensions=('h' 'c' 'cpp')
declare -a dirs=('engine' 'libraries/cmd' 'libraries/collide2' 'libraries/gfx_generic' 'libraries/gui' 'libraries/posh' \
    'libraries/root_generic' 'libraries/vegadisk')
for subdir in "${dirs[@]}"; do
  for file_extension in "${file_extensions[@]}"; do
    echo "Processing .${file_extension} files in ${subdir} subdirectory"
    if test -n "$(find "${subdir}" -name "*.${file_extension}" -type f -print -exit)"; then
      find "${subdir}" -name "*.${file_extension}" -type f -print0 | xargs -0 clang-format -i
      git add "${subdir}"
      git commit -m "Apply clang-format -i (in-place) to all .${file_extension} files under ${subdir}"
    fi
  done
done
