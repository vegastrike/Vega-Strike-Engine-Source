# bootstrap.ps1

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

# You can customize this directory location if desired, but it should be
# something very short. Otherwise, you will run into problems.

param(
    [String]$VCPKG_PARENT_DIR = "C:\Projects"
)
Set-Variable -Name CMAKE_VERSION -Value "3.25.1"

New-Item "$VCKPG_PARENT_DIR" -ItemType Directory -Force
Push-Location "$VCKPG_PARENT_DIR"
git clone https://github.com/stephengtuggy/vcpkg-local.git
.\vcpkg-local\bootstrap-vcpkg.bat -disableMetrics

[Environment]::SetEnvironmentVariable('VCPKG_ROOT', "$VCKPG_PARENT_DIR\vcpkg-local", 'User')

$path = [Environment]::GetEnvironmentVariable('PATH', 'User')
$newPath = $path + ";$VCKPG_PARENT_DIR\vcpkg-local\downloads\tools\cmake-$CMAKE_VERSION-windows\cmake-$CMAKE_VERSION-windows-i386\bin"
[Environment]::SetEnvironmentVariable('PATH', $newPath, 'User')

[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_TRIPLET', 'x64-windows', 'User')
[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_HOST_TRIPLET', 'x64-windows', 'User')
[Environment]::SetEnvironmentVariable('PYTHONHOME', "$VCKPG_PARENT_DIR\vcpkg-local\packages\python3_x64-windows\tools\python3", 'User')

Pop-Location
