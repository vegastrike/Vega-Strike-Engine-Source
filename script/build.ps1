# build.ps1

# Copyright (C) 2021-2023 Stephen G. Tuggy and other Vega Strike contributors

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


param(
    [String]$BuildType = "Release"
)

cmake -B build -S .\engine\ -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=$BuildType
cmake --build .\build\ --config $BuildType -v

New-Item bin -ItemType Directory -Force
xcopy /y .\build\$BuildType\*.* .\bin\
xcopy /y .\build\objconv\$BuildType\*.* .\bin\
# Not building vegasettings for the moment
# xcopy /y .\build\setup\$BuildType\*.* .\bin\
