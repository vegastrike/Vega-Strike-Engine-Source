##
# asteroidgen.py
#
# Copyright (c) 2001-2002 Daniel Horn
# Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
# Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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
vertices=((0.469,0.469,0.469),
	(0.290,0.000,0.759),
	(-0.759,-0.290,0.000),
	(0.759,0.290,0.000),
	(-0.469,0.469,-0.469),
	(0.000,-0.759,-0.290),
	(-0.759,0.290,0.000),
	(0.469,-0.469,0.469),
	(-0.469,0.469,0.469),
	(-0.469,-0.469,0.469),
	(0.469,-0.469,-0.469),
	(0.290,0.000,-0.759),
	(-0.469,-0.469,-0.469),
	(0.000,-0.759,0.290),
	(0.000,0.759,-0.290),
	(-0.290,0.000,0.759),
	(0.759,-0.290,0.000),
	(-0.290,0.000,-0.759),
	(0.469,0.469,-0.469),
	(0.000,0.759,0.290))
faces=((9, 13, 7, 1, 15),
	(6, 4, 14, 19, 8),
	(12, 5, 13, 9, 2),
	(6, 2, 12, 17, 4),
	(16, 10, 11, 18),
	(19, 8, 15, 1, 0),
	(16, 7, 1, 0, 3),
	(5, 12, 17, 11, 10),
	(18, 14, 4, 17, 11),
	(16, 10, 5, 13, 7),
	(2, 6, 8, 15, 9),
	(19, 0, 3, 18, 14))
import sys
import random
def writeVec(myvec):
	return str(myvec[0])+";"+str(myvec[1])+";"+str(myvec[2]);
def writeRoid(x,y,z,name):
	whicha =random.randrange(0,3)
	whichb=random.randrange(0,3)
	if (whichb==whicha):
		whichb=(whicha+1)%3
	rvec=[0,0,0]
	orvec=[0,0,0]
	a=random.randrange(0,2)
	b=random.randrange(0,2)
	if (a==0):
		a=-1
	if (b==0):
		b=-1
	rvec[whicha]=a
	orvec[whichb]=b
	sys.stdout.write("{"+name+";"+writeVec((x,y,z))+";"+writeVec(rvec)+";"+writeVec(orvec)+";180.0}")
scale=2000
rad=900
num=40
centroid=True
sys.stdout.write("AFieldBasePriv,objects/asteroids,AFieldBasePriv,FIXME,FIXME,,,af-hud.spr,1,,,,,,,,,,,36000,36000,,5,400000000,,,,,,,,,,,,,,,,,,,,,,FALSE,,,,,0,,0,0,0,,,,0,0,0,0,0,0,0,,,FALSE,TRUE,14999,5,180,10,,,,,,,,0,0,0,1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1,1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1,1,1,1,1,1,1,1,1,0,0,,,,")
for roid in vertices:
	(i,j,k)=roid
	i*=scale;
	j*=scale;
	k*=scale;
	writeRoid(i,j,k,"asteroidsPriv");
if centroid:
	for face in faces:
		place=[0,0,0]
		for nume in face:
			place[0]+=vertices[nume][0]/len(face)
			place[1]+=vertices[nume][1]/len(face)
			place[2]+=vertices[nume][2]/len(face)
		writeRoid(place[0]*scale,scale*place[1],scale*place[2],"asteroidsPriv")
def vecLen(x):
	sum=0
	for i in x:
		sum+=i*i
	return math.sqrt(sum)
i=0
sys.stdout.write(",,,,,,,,,0,0,16,0,1")
import math
print("")
sys.stdout.write('"asteroidsPriv","objects/asteroids","Asteroids","FIXME","FIXME","INERT","AFieldBasePriv","af-hud.spr",1,,,,," ",,,,,,20000,20000,,,999999999,500,500,500,500,500,500,500,500,0,0,0,0,,,,,0,,,,,FALSE,,,,,0,,500,500,500,10,10,10,20000,20000,20000,20000,20000,20000,20000,,,FALSE,FALSE,4999,5,180,10,,,,,,,,0,0,0,"1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1","1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1;1",1,1,1,1,1,1,1,1,0,0,,,,')
for i in range(num):
	vec=(rad,rad,rad)
	while vecLen(vec)>rad:
		vec=(random.uniform(-rad,rad),random.uniform(-rad,rad),random.uniform(-rad,rad))		

	writeRoid(vec[0],vec[1],vec[2],"asteroid"+str(random.randrange(0,8)))
sys.stdout.write(",,,,,,,,,0,0,16,0,1,0,,1")
print("")
