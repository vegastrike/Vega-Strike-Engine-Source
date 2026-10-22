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
echo "File extensions:" "${file_extensions[@]}"

declare -a dirs=('engine' 'libraries/cmd' 'libraries/gfx_generic' 'libraries/gui' 'libraries/posh' \
    'libraries/root_generic' 'libraries/vegadisk')
echo "Directories:" "${dirs[@]}"

use_clang_format_21=false
use_clang_format_20=false
use_clang_format_generic=false

set +e
clang_format_21=$(which clang-format-21)
clang_format_20=$(which clang-format-21)
clang_format_generic=$(which clang-format)
if test -n "$clang_format_21"; then
  use_clang_format_21=true
  echo "Using clang-format-21"
elif test -n "$clang_format_20"; then
  use_clang_format_20=true
  echo "Using clang-format-20"
elif test -n "$clang_format_generic"; then
  use_clang_format_generic=true
  echo "Warning: Couldn't find compatible version of clang-format; using generic version"
else
  echo "Fatal: Couldn't find any version of clang-format"
  exit 1
fi
set -e

function clangFormat()
{
  if $use_clang_format_21; then
    find "$1" -name "*.$2" -type f -print0 | xargs -0 clang-format-21 -i
  elif $use_clang_format_20; then
    find "$1" -name "*.$2" -type f -print0 | xargs -0 clang-format-20 -i
  elif $use_clang_format_generic; then
    find "$1" -name "*.$2" -type f -print0 | xargs -0 clang-format -i
  else
    echo "Fatal: Couldn't find any version of clang-format"
    exit 1
  fi
}

for subdir in "${dirs[@]}"; do
  echo "Processing ${subdir} subdirectory"
  for file_extension in "${file_extensions[@]}"; do
    echo "Processing .${file_extension} files in ${subdir} subdirectory"
    if test -n "$(find "${subdir}" -name "*.${file_extension}" -type f -print -quit)"; then
      clangFormat "${subdir}" "${file_extension}"
      git add "${subdir}"
      if test -n "$(git status -z)"; then
        git commit -m "Apply clang-format -i (in-place) to all .${file_extension} files under ${subdir}"
      fi
    fi
  done
done
