/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Evert Vorster,
 * Roy Falk, Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef _STARSYSGEN_H_
#define _STARSYSGEN_H_
#include <vector>
#include <string>
using std::string;
using std::vector;

/// All the properties from the galaxy in a system.
struct SystemInfo {
    string sector;
    string name;
    string filename;
    float sunradius;
    float compactness;
    int numstars;
    bool nebulae;
    bool asteroids;
    int numun1;
    int numun2;
    string faction;
    string names;
    string stars;
    string planetlist;
    string smallun;
    string nebulaelist;
    string asteroidslist;
    string ringlist;
    string backgrounds;
    vector<string> jumps;
    int seed;
    bool force;
};

///appends .system
std::string getStarSystemFileName(const std::string &input);
///finds the name after all / characters and capitalizes the first letter
std::string getStarSystemName(const std::string &in);
///finds the name before the first /  this is the sector name
std::string getStarSystemSector(const std::string &in);
string getUniversePath();
void readnames(vector<string> &entity, const char *filename);
void generateStarSystem(SystemInfo &si);
#endif

