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
    [String]$Generator = "VS2019Win64", # Other options include "ninja" and "VS2022Win64"
    [Boolean]$EnablePIE = $false,
    [String]$BuildType = "Release" # You can also specify "Debug"
)

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
} else {
    Write-Error "Unrecognized value for BuildType: $BuildType"
    exit 1
}

[String]$baseDir = (Get-Location -PSProvider "FileSystem").Path
[String]$binaryDir = "$baseDir\build\$cmakePresetName"
Push-Location $baseDir\engine
cmake --preset $cmakePresetName
cmake --build --preset "build-$cmakePresetName" -v
Pop-Location

New-Item bin -ItemType Directory -Force
$aPossibleBinaryDirs = @("$binaryDir","$binaryDir\$BuildType", "$binaryDir\$BuildType\objconv", "$binaryDir\objconv\$BuildType", "$binaryDir\$BuildType\setup", "$binaryDir\setup\$BuildType")
$aPossibleBinaryDirs | ForEach-Object {
    if (Test-Path $_) {
        Copy-Item -Force -Verbose $_\*.* .\bin
    }
}
