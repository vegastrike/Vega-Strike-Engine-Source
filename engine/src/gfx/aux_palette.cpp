/*
 * aux_palette.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2003-2019 dan_w and other Vega Strike contributors
 * Copyright (C) 2020 pyramid3d
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include <math.h>
#define UNDEFINED -99999

double Maximum(double r, double g, double b)
{
    if (r > g && r > b) {
        return r;
    }
    if (b > r && b > g) {
        return b;
    }
    return g;
}

double Minimum(double r, double g, double b)
{
    if (r < g && r < b) {
        return r;
    }
    if (b < r && b < g) {
        return b;
    }
    return g;
}

void RGB_To_HSV(double r, double g, double b, double *h, double *s, double *v)
{
/*
 * given: rgb, each in [0,1]
 * desired: h in [0,360), s and v in [0,1] except if s = 0 then h = undefined
 */
    double max = Maximum(r, g, b);
    double min = Minimum(r, g, b);
    *v = max;                           /*This is the value v. */
    /* Next calculate saturation, s. Saturation is 0 if red, green and blue are all 0 */
    *s = (max != 0.0) ? ((max - min) / max) : 0.0;
    if (*s == 0.0) {
        *h = UNDEFINED;
    } else {
        /*Chromatic case: Saturation is not 0*/
        double delta = max - min;           /*Determine Hue*/
        if (r == max) {
            *h = (g - b) / delta;                  /* resulting color is between yellow and magenta */
        } else if (g == max) {
            *h = 2.0 + (b - r) / delta;                    /* resulting color is between cyan and yellow */
        } else if (b == max) {
            *h = 4.0 + (r - g) / delta;
        }                   /* resulting color is between magenta and cyan */
        *h *= 60.0;                             /*convert hue to degrees*/
        if (*h < 0.0)
            *h += 360.0;                        /*make sure its not negative*/
    }
}

void HSV_To_RGB(double *r, double *g, double *b, double h, double s, double v)
{
    if (s == 0.0) {
        /*
         * the color is on the black and white center line.
         *******************************achromatic color: there is no hue.
         */
        if (h == UNDEFINED) {
            *r = v;             /* this is the achromatic case. */
            *g = v;
            *b = v;
        } else {
            *r = v;             /* this is the achromatic case. */
            *g = v;
            *b = v;
        }
    } else {
        double f, p, q, t;         /*chromatic color: s != 0, so there is a hue*/
        int i;
        if (h == 360.0) {          /*360 degrees is equivalent to 0 degrees. */
            h = 0.0;
        }
        h /= 60.0;         /* h is now in [0,6).*/
        i = (int) (h);           /*Floor returns the greatest integer <= h*/
        f = h - i;               /*f is the fractional part of h.*/
        p = v * (1.0 - s);
        q = v * (1.0 - (s * f));
        t = v * (1.0 - (s * (1.0 - f)));
        switch (i) {
            case 0:
                *r = v;
                *g = t;
                *b = p;
                break;
            case 1:
                *r = q;
                *g = v;
                *b = p;
                break;
            case 2:
                *r = p;
                *g = v;
                *b = t;
                break;
            case 3:
                *r = p;
                *g = q;
                *b = v;
                break;
            case 4:
                *r = t;
                *g = p;
                *b = v;
                break;
            case 5:
                *r = v;
                *g = p;
                *b = q;
                break;
        }
    }     /*Chromatic case */
}

//desired: h in [0,360), s and v in [0,1] except if s = 0 then h = undefined
void ShiftPalette(unsigned char Palette[769], double DH, double DS, double DV, double DsH, double DsS, double DsV)
{
    double r, g, b, h, s, v;
    int i;
    for (i = 192; i < 224; i++) {
        r = ((double) .003921568627) * Palette[i * 3];
        g = ((double) .003921568627) * Palette[i * 3 + 1];
        b = ((double) .003921568627) * Palette[i * 3 + 2];
        RGB_To_HSV(r, g, b, &h, &s, &v);
        h += DH;
        s += DS;
        v += DV;
        if (h >= 360) {
            h -= 360;
        }
        if (h < 0) {
            h += 360;
        }
        if (s > 1) {
            s -= 1;
        }
        if (s < 0) {
            s += 1;
        }
        if (s == 0) {
            s += .01;
        }
        if (v > 1) {
            v -= 1;
        }
        if (v < 0) {
            v += 1;
        }
        HSV_To_RGB(&r, &g, &b, h, s, v);
        Palette[i * 3] = (unsigned char) (r * 255);
        Palette[i * 3 + 1] = (unsigned char) (g * 255);
        Palette[i * 3 + 2] = (unsigned char) (b * 255);
    }
    for (i = 224; i < 256; i++) {
        r = ((double) .003921568627) * Palette[i * 3];
        g = ((double) .003921568627) * Palette[i * 3 + 1];
        b = ((double) .003921568627) * Palette[i * 3 + 2];
        RGB_To_HSV(r, g, b, &h, &s, &v);
        h += DsH;
        s += DsS;
        v += DsV;
        if (h >= 360) {
            h -= 360;
        }
        if (h < 0) {
            h += 360;
        }
        if (s > 1) {
            s -= 1;
        }
        if (s < 0) {
            s += 1;
        }
        if (s == 0) {
            s += .01;
        }
        if (v > 1) {
            v -= 1;
        }
        if (v < 0) {
            v += 1;
        }
        HSV_To_RGB(&r, &g, &b, h, s, v);
        Palette[i * 3] = (unsigned char) (r * 255);
        Palette[i * 3 + 1] = (unsigned char) (g * 255);
        Palette[i * 3 + 2] = (unsigned char) (b * 255);
    }
}

