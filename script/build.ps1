# Copyright (C) 2021 Stephen G. Tuggy

# https://github.com/vegastrike/Vega-Strike-Engine-Source

# This file is part of Vega Strike.

# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.

cmake -B build -S .\engine\ -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build .\build\ --config Release -v
New-Item bin -ItemType Directory -Force
xcopy /y .\build\Release\*.* .\bin\
xcopy /y .\build\objconv\Release\*.* .\bin\
# Not building vegasettings for the moment
# xcopy /y .\build\setup\Release\*.* .\bin\
