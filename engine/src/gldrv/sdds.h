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
#ifndef VEGA_STRIKE_ENGINE_GLDRV_SDDS_H
#define VEGA_STRIKE_ENGINE_GLDRV_SDDS_H
#include "gfxlib_struct.h"

/*
 *       input is the compressed dxt file, already read in by vsimage.
 *       output is an empty pointer created in the calling function.
 *       format is the bit format of the compressed texture (rgba)
 *       height and width are self explanatory
 *
 *       when function returns, output will contain the uncompressed first mipmap of the dxt image.
 *       the calling function will have to replace the input pointer with the output pointer and set
 *       the texture format to rgba.
 */

void ddsDecompress(unsigned char *&input, unsigned char *&output, TEXTUREFORMAT format, int height, int width);

#endif //VEGA_STRIKE_ENGINE_GLDRV_SDDS_H
