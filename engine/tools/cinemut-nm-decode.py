#!/usr/bin/python
##
# cinemut-nm-decode.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
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
    print("Usage: ./cinemut-nm-decode.py <inputfile> <outputfile>")
    sys.exit(0)

try:
    import numpy
    def encode(im, len=len):
        im.load()
        r,g,b,a = im.split()
        atan = numpy.arctan
        tan = numpy.tan
        e = 2.0
        
        u  = numpy.array(r.getdata(), dtype=numpy.float32)
        u += numpy.array(g.getdata(), dtype=numpy.float32)
        u += numpy.array(b.getdata(), dtype=numpy.float32)
        v  = numpy.array(a.getdata(), dtype=numpy.float32)
        
        u *= 1.0/(3*255)
        u -= 0.5
        
        v *= 1.0/255
        v -= 0.5
        
        z  = 1.0/numpy.sqrt(0.25*0.25 + u*u + v*v)
        u *= z
        v *= z
        z *= 0.25
        
        u *= 0.5
        v *= 0.5
        u += 0.5
        v += 0.5
        
        u *= 255
        v *= 255
        z *= 255
        
        r.putdata(u)
        g.putdata(v)
        b.putdata(z)
        a.putdata(numpy.zeros(len(u))+255)
        im = Image.merge("RGBA", (r,g,b,a))
        return im
except ImportError:
    print("Warning: numpy not found. Install it to greatly improve performance.", file=sys.stderr)
    def encode(im, int=int):
        data = im.load()
        sqrt = math.sqrt
        e = 2.0
        for x in range(im.size[0]):
            for y in range(im.size[1]):
                loc = x,y
                r,g,b,a = data[loc]
                u = (r+g+b) / (3*255.0) - 0.5
                v = a / 255.0 - 0.5
                z = 1.0/sqrt(0.25*0.25+u*u+v*v)
                u *= z
                v *= z
                z *= 0.25
                r = (u*0.5+0.5)*255
                g = (v*0.5+0.5)*255
                b = z*255
                a = 255
                data[loc] = int(r),int(g),int(b),int(a)
        return im

im = Image.open(sys.argv[1])

if im.mode != "RGBA":
    im = im.convert("RGBA")

im = encode(im)

im.save(sys.argv[2])
