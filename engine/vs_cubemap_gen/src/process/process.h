/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
#ifndef VEGA_STRIKE_ENGINE_CUBE_MAP_PROCESS_H
#define VEGA_STRIKE_ENGINE_CUBE_MAP_PROCESS_H
/*
 * The purpose of this is to control the general processing: reading the input
 * textures into floating point memory textures, allocating memory textures for
 * the output and all its mipmaps, calling filter for each mipmap, writing png's,
 * calling nvcompress, and reassembling all the many dds into a single dds.
 * But it's also called "process" because we might want to implement multithreading
 * or multiple processes here, to run on separate cores, in multi-core machines.
*/

class process {
public
};

#endif //VEGA_STRIKE_ENGINE_CUBE_MAP_PROCESS_H
