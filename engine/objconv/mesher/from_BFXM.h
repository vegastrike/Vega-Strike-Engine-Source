/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
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

// NO HEADER GUARD

#include <string>
#include <vector>
using std::string;
using std::vector;
#include "endianness.h"
#include <stdlib.h>
#include <stdio.h>

void BFXMToXmeshOrOBJ(FILE *Inputfile,
        FILE *Outputfile,
        FILE *outputObj,
        FILE *mtl,
        std::string meshname,
        char hackaroundhack); //Translate BFXM file Inputfile to text file Outputfile
void BFXMtoBoxDims(FILE *Inputfile, const char *name); //computes box dimensions given a bfxm
