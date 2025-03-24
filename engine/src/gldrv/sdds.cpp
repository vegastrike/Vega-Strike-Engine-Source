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

#include <stdlib.h>
#include "gldrv/sdds.h"
#include "root_generic/vs_globals.h"

#ifndef GETL16
#define GETL16(buf) ( ( (unsigned short) (buf)[0] )|( (unsigned short) (buf)[1]<<8 ) )
#endif
#ifndef GETL64
#define GETL64(buf)                 \
    ( ( (unsigned int) (buf)[0] )     \
     |( (unsigned long long) (buf)[1] \
       <<8 )                          \
     |( (unsigned long long) (buf)[2] \
       <<16 )                         \
     |( (unsigned long long) (buf)[3] \
       <<24 )                         \
     |( (unsigned long long) (buf)[4] \
       <<32 )                         \
     |( (unsigned long long) (buf)[5] \
       <<40 )|( (unsigned long long) (buf)[6]<<48 )|( (unsigned long long) (buf)[7]<<56 ) )
#endif

/*	Software decompression for DDS files, helper functions */

void decode_color_block(unsigned char *RESTRICT dst,
        unsigned char *RESTRICT src,
        int w,
        int h,
        int rowbytes,
        TEXTUREFORMAT format) {
    int i, x, y;
    unsigned int indexes, idx;
    unsigned char *d;
    unsigned char colors[4][3];
    unsigned short c0, c1;
    c0 = GETL16(&src[0]);
    c1 = GETL16(&src[2]);
    colors[0][0] = ((c0 >> 11) & 0x1f) << 3;
    colors[0][1] = ((c0 >> 5) & 0x3f) << 2;
    colors[0][2] = ((c0) & 0x1f) << 3;
    colors[1][0] = ((c1 >> 11) & 0x1f) << 3;
    colors[1][1] = ((c1 >> 5) & 0x3f) << 2;
    colors[1][2] = ((c1) & 0x1f) << 3;
    if ((c0 > c1) || (format == DXT5)) {
        for (i = 0; i < 3; ++i) {
            colors[2][i] = (2 * colors[0][i] + colors[1][i] + 1) / 3;
            colors[3][i] = (2 * colors[1][i] + colors[0][i] + 1) / 3;
        }
    } else {
        for (i = 0; i < 3; ++i) {
            colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
            colors[3][i] = 255;
        }
    }
    src += 4;
    if (format == DXT1 || format == DXT1RGBA) {
        for (y = 0; y < h; ++y) {
            d = dst + (y * rowbytes);
            indexes = src[y];
            for (x = 0; x < w; ++x) {
                idx = indexes & 0x03;
                d[0] = colors[idx][0];
                d[1] = colors[idx][1];
                d[2] = colors[idx][2];
                d[3] = ((c0 <= c1) && idx == 3) ? 0 : 255;
                indexes >>= 2;
                d += 4;
            }
        }
    } else {
        for (y = 0; y < h; ++y) {
            d = dst + (y * rowbytes);
            indexes = src[y];
            for (x = 0; x < w; ++x) {
                idx = indexes & 0x03;
                d[0] = colors[idx][0];
                d[1] = colors[idx][1];
                d[2] = colors[idx][2];
                indexes >>= 2;
                d += 4;
            }

        }
    }
}

void decode_dxt3_alpha(unsigned char *RESTRICT dst, unsigned char *RESTRICT src, int w, int h, int rowbytes) {
    int x, y;
    unsigned char *d;
    unsigned int bits;
    for (y = 0; y < h; ++y) {
        d = dst + (y * rowbytes);
        bits = GETL16(&src[2 * y]);
        bits = GETL16(&src[2 * y]);
        for (x = 0; x < w; ++x) {
            d[0] = (bits & 0x0f) * 17;
            bits >>= 4;
            d += 4;
        }
    }
}

void decode_dxt5_alpha(unsigned char *RESTRICT dst, unsigned char *RESTRICT src, int w, int h, int bpp, int rowbytes) {
    int x, y, code;
    unsigned char *d, a0 = src[0], a1 = src[1];
    unsigned long long bits = GETL64(src) >> 16;
    for (y = 0; y < h; ++y) {
        d = dst + (y * rowbytes);
        for (x = 0; x < w; ++x) {
            code = ((unsigned int) bits) & 0x07;
            if (code < 2) {
                d[0] = src[code];
            } else if (a0 > a1) {
                d[0] = ((8 - code) * a0 + (code - 1) * a1) / 7;
            } else if (code >= 6) {
                d[0] = (code == 6) ? 0 : 255;
            } else {
                d[0] = ((6 - code) * a0 + (code - 1) * a1) / 5;
            }
            bits >>= 3;
            d += bpp;
        }
        if (w < 4) {
            bits >>= (3 * (4 - w));
        }
    }
}

void ddsDecompress(unsigned char *&RESTRICT buffer,
        unsigned char *&RESTRICT data,
        TEXTUREFORMAT internformat,
        int height,
        int width) {
    unsigned char *pos_out = NULL, *pos_in = NULL;
    int bpp = 4;
    unsigned int sx, sy;

    sx = (width < 4) ? width : 4;
    sy = (height < 4) ? width : 4;
    data = (unsigned char *) malloc(height * width * bpp);
    pos_out = data;
    pos_in = buffer;
    for (int y = 0; y < height; y += 4) {
        for (int x = 0; x < width; x += 4) {
            pos_out = data + (y * width + x) * bpp;
            if (internformat == DXT3) {
                decode_dxt3_alpha(pos_out + 3, pos_in, sx, sy, width * bpp);
                pos_in += 8;
            } else if (internformat == DXT5) {
                decode_dxt5_alpha(pos_out + 3, pos_in, sx, sy, bpp, width * bpp);
                pos_in += 8;
            }
            decode_color_block(pos_out, pos_in, sx, sy, width * bpp, internformat);
            pos_in += 8;
        }
    }
}

/*  END of software decompression for DDS helper functions */

