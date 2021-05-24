@REM Copyright (C) 2021 Stephen G. Tuggy

@REM https://github.com/vegastrike/Vega-Strike-Engine-Source

@REM This file is part of Vega Strike.

@REM Vega Strike is free software: you can redistribute it and/or modify
@REM it under the terms of the GNU General Public License as published by
@REM the Free Software Foundation, either version 3 of the License, or
@REM (at your option) any later version.

@REM Vega Strike is distributed in the hope that it will be useful,
@REM but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM GNU General Public License for more details.

@REM You should have received a copy of the GNU General Public License
@REM along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.

cmake -B build -S .\engine\ -DCMAKE_TOOLCHAIN_FILE=C:\Projects\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DUSE_PYTHON_3=ON
cmake --build .\build\ --config Release
