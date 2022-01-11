#!/usr/bin/python

##
# cinemut-nm-encode.py
#
# Copyright (C) 2001-2002 Daniel Horn
# Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
#

from __future__ import print_function
import sys
from PIL import Image
import math
try:
    range = xrange
except:
    pass

if len(sys.argv) <= 2:
    print("Usage: ./cinemut-nm-encode.py <inputfile> <outputfile>")
    sys.exit(0)

try:
    import numpy
    def encode(im):
        im.load()
        r,g,b,a = im.split()
        atan = numpy.arctan
        tan = numpy.tan
        e = 2.0
        u = numpy.array(r.getdata(), dtype=numpy.float32)
        v = numpy.array(g.getdata(), dtype=numpy.float32)
        z = numpy.array(b.getdata(), dtype=numpy.float32)
        u *= (2.0/255)
        v *= (2.0/255)
        z *= (1.0/255)
        u -= 1
        v -= 1
        u /= z
        v /= z
        u  = atan(u, u)
        v  = atan(v, v)
        u *= 0.5
        v *= 0.5
        u  = tan(u, u)
        v  = tan(v, v)
        u *= 0.5
        v *= 0.5
        u += 0.5
        v += 0.5
        u *= 255
        v *= 255
        g.putdata(u)
        u -= e
        r.putdata(u)
        u += 2*e
        b.putdata(u)
        a.putdata(v)
        im = Image.merge("RGBA", (r,g,b,a))
        return im
except ImportError:
    print("Warning: numpy not found. Install it to greatly improve performance.", file=sys.stderr)
    def encode(im, int=int):
        data = im.load()
        atan = math.atan
        tan = math.tan
        e = 2.0
        for x in range(im.size[0]):
            for y in range(im.size[1]):
                loc = x,y
                u,v,z,a = data[loc]
                if z:
                    z = z/255.0
                    u = 2*(u/255.0)-1
                    v = 2*(v/255.0)-1
                    u = tan(0.5*atan(u/z))
                    v = tan(0.5*atan(v/z))
                    u = (0.5*u+0.5)*255
                    v = (0.5*v+0.5)*255
                else:
                    u=v=z=0
                data[loc] = int(u-e),int(u),int(u+e),int(v)
        return im

im = Image.open(sys.argv[1])

if im.mode != "RGBA":
    im = im.convert("RGBA")

im = encode(im)

im.save(sys.argv[2])
