/*
 * Vega Strike
 * Copyright (C) 2002 David Jeffery
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define BITMASK(bits) ((1 << bits) - 1)
#define SIGN_MASK    0x80000000
#define EXP_MASK     0x7f800000
#define SIGNIF_MASK  0x007fffff
#define EXP_BITS     8
#define SIGNIF_BITS  23

// encode a float into a smaller form
unsigned int float_to_uint32(float value, char exponent, char significand, char denorm);
// decode a shrunk float back to its natural form
float uint32_to_float(unsigned int value, char exponent, char significand, char denorm);

