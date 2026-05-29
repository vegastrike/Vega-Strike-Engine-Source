##
# bootstrap.ps1
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2026 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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


# You can customize this directory location if desired, but it should be
# something very short. Otherwise, you will run into problems.
param(
    [String]$VCPKG_PARENT_DIR = "C:\Projects"
)

New-Item "$VCPKG_PARENT_DIR" -ItemType Directory -Force
Push-Location "$VCPKG_PARENT_DIR"

[Environment]::SetEnvironmentVariable('VCPKG_ROOT', "$VCPKG_PARENT_DIR\vcpkg", 'User')
$env:VCPKG_ROOT = "$VCPKG_PARENT_DIR\vcpkg"

[Environment]::SetEnvironmentVariable('VCPKG_ASSET_SOURCES', "$VCPKG_PARENT_DIR\vcpkgAssets", 'User')
$env:VCPKG_ASSET_SOURCES="$VCPKG_PARENT_DIR\vcpkgAssets"
mkdir -p $env:VCPKG_ASSET_SOURCES
[Environment]::SetEnvironmentVariable('X_VCPKG_ASSET_SOURCES', "clear;x-azurl,file:///$VCPKG_PARENT_DIR\vcpkgAssets,,readwrite", 'User')
$env:X_VCPKG_ASSET_SOURCES="clear;x-azurl,file:///$VCPKG_PARENT_DIR\vcpkgAssets,,readwrite"

[Environment]::SetEnvironmentVariable('VCPKG_BINARY_SOURCES', "clear;files,$VCPKG_PARENT_DIR\vcpkgBin,readwrite", 'User')
$env:VCPKG_BINARY_SOURCES = "clear;files,$VCPKG_PARENT_DIR\vcpkgBin,readwrite"
mkdir -p $VCPKG_PARENT_DIR\vcpkgBin

[Environment]::SetEnvironmentVariable('VCPKG_BUILD_TREES', "$VCPKG_PARENT_DIR\vcpkgBuild", 'User')
$env:VCPKG_BUILD_TREES="$VCPKG_PARENT_DIR\vcpkgBuild"
mkdir -p $env:VCPKG_BUILD_TREES
[Environment]::SetEnvironmentVariable('VCPKG_INSTALL_OPTIONS', "--x-buildtrees-root=$VCPKG_PARENT_DIR\vcpkgBuild", 'User')
$env:VCPKG_INSTALL_OPTIONS="--x-buildtrees-root=$VCPKG_PARENT_DIR\vcpkgBuild"

git clone https://github.com/Microsoft/vcpkg.git "$env:VCPKG_ROOT"
& "$env:VCPKG_ROOT\bootstrap-vcpkg.bat" -disableMetrics

Set-Variable -Name CMAKE_WITH_VERSION -Value (Get-ChildItem "$env:VCPKG_ROOT\downloads\tools" -Filter "cmake-*-windows" -Directory | Select-Object -Last 1).Name

$path = [Environment]::GetEnvironmentVariable('PATH', 'User')
$newPath = $path + ";$env:VCPKG_ROOT\downloads\tools\$CMAKE_WITH_VERSION\$CMAKE_WITH_VERSION-i386\bin"
[Environment]::SetEnvironmentVariable('PATH', $newPath, 'User')
$env:PATH = $newPath

$triplet = 'x64-win10'
[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_TRIPLET', $triplet, 'User')
$env:VCPKG_DEFAULT_TRIPLET = $triplet
[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_HOST_TRIPLET', $triplet, 'User')
$env:VCPKG_DEFAULT_HOST_TRIPLET = $triplet

Pop-Location
