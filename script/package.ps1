##
# package.ps1
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
    [String]$Generator = "VS2019Win64", # Other options include "ninja" and "VS2022Win64"
    [Boolean]$EnablePIE = $false,
    [String]$BuildType = "Release", # You can also specify "Debug"
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

[String]$cmakePresetName = ""
if ($Generator -ieq "Ninja") {
    $cmakePresetName += "windows-ninja"
} elseif ($Generator -ieq "VS2019Win64") {
    $cmakePresetName += "VS2019Win64"
} elseif ($Generator -ieq "VS2022Win64") {
    $cmakePresetName += "VS2022Win64"
} else {
    Write-Error "Invalid value for Generator: $Generator"
    exit 1
}
$cmakePresetName += "-"
if ($EnablePIE) {
    $cmakePresetName += "pie-enabled"
} else {
    $cmakePresetName += "pie-disabled"
}
$cmakePresetName += "-"
if ($BuildType -ieq "Debug") {
    $cmakePresetName += "debug"
} elseif ($BuildType -ieq "Release") {
    $cmakePresetName += "release"
} elseif ($BuildType -ieq "RelWithDebInfo") {
    $cmakePresetName += "RelWithDebInfo"
} else {
    Write-Error "Unrecognized value for BuildType: $BuildType"
    exit 1
}

[String]$baseDir = (Get-Location -PSProvider "FileSystem").Path
[String]$binaryDir = "$baseDir\build\$cmakePresetName"

Push-Location $baseDir
cpack -V --preset "package-$cmakePresetName"
Pop-Location
