##
# build.ps1
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


param(
    [String]$PresetName = "VS2022Win64-pie-enabled-RelWithDebInfo",
    [String]$BuildType = "RelWithDebInfo", # You can also specify "Debug" or "Release"
    [Boolean]$IsRelease = $false,
    [String]$GitTag = "not-applicable", # Git Tag, default empty string for PR builds
    [String]$GitSha = "not-applicable"  # Git Short SHA Reference, default empty string for PR builds
)

# Hack around PowerShell not allowing empty string parameters
if ($GitTag -ieq "not-applicable") {
    $GitTag = ""
}
if ($GitSha -ieq "not-applicable" ) {
    $GitSha = ""
}


[String]$baseDir = (Get-Location -PSProvider "FileSystem").Path
[String]$binaryDir = "$baseDir\build\$PresetName"
Push-Location $baseDir
cmake --preset $PresetName
cmake --build --preset "build-$PresetName" -v
Pop-Location

New-Item bin -ItemType Directory -Force
$aPossibleBinaryDirs = @("$binaryDir","$binaryDir\$BuildType", "$binaryDir\$BuildType\objconv", "$binaryDir\objconv\$BuildType", "$binaryDir\$BuildType\setup", "$binaryDir\setup\$BuildType")
$aPossibleBinaryDirs | ForEach-Object {
    if (Test-Path $_) {
        Copy-Item -Force -Verbose $_\*.* .\bin
    }
}

if ($IsRelease) {
    Compress-Archive .\bin\* "VegaStrike_${GitTag}_${GitSha}.zip"
}
