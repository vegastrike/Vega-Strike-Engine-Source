/**
* process.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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

#ifndef __PROCESS_H__
#define __PROCESS_H__
/*
 * The purpose of this is to control the general processing: reading the input
 * textures into floating point memory textures, allocating memory textures for
 * the output and all its mipmaps, calling filter for each mipmap, writing png's,
 * calling nvcompress, and reassembling all the many dds into a single dds.
 * But it's also called "process" because we might want to implement multithreading
 * or multiple processes here, to run on separate cores, in multi-core machines.
*/

class process
{
public
};



#endif


