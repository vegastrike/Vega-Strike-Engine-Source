##
# starSystemPlanetMaker.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
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
import xml
import xml.dom
import xml.dom.minidom
import math
import planetMakerHelper
stardatafile=None
arg=0
remakePlanets=0
while (arg<len(sys.argv)):
	match=0
	if sys.argv[arg].find("-csv")==0:
		stardatafile=sys.argv[arg][4:]
		match=1
	if sys.argv[arg].find("-replanet")==0:
		remakePlanets=1
		match=1
	if (match==1):
		del sys.argv[arg]
		arg-=1
	arg+=1
def getParentVal(node,val):
	i = node.parentNode
	while(i):
		j = i.firstChild
		while (j):
			try:
				if (j.tagName!='var'):
					return None
				if (j.getAttribute('name')==val):
					return j.getAttribute('value')
			except:
				pass
			j = j.nextSibling		
		i = node.parentNode
	return getParentVal(node,val)
def toPair (s):
	if (not s):
		return None
	k=s.split(' ')
	return (float(k[0]),float(k[1]),float(k[2]))
def getVal(node,val):
	for i in node.getElementsByTagName('var'):
		if (i.getAttribute('name')==val):
			return i.getAttribute('value')
	return getParentVal(node,val)
def getValND(node,val):
	for i in node.getElementsByTagName('var'):
		if (i.getAttribute('name')==val):
			return i.getAttribute('value')
	return None
def removeVal(node,val):
	for i in node.getElementsByTagName('var'):
		if (i.getAttribute('name')==val):
			node.removeChild(i)
fil = open(sys.argv[1],"r")
g = xml.dom.minidom.parseString(fil.read());
fil.close()
planets= g.getElementsByTagName('planet')
systems = g.getElementsByTagName('system')
stardata=None
starcoords=[]


if (stardatafile):
	fsd = open(stardatafile)
	stardatalines=fsd.readlines()
	stardatalines=stardatalines[1:]
	for i in range(len(stardatalines)):
		stardatalines[i]=stardatalines[i].strip().split(',')
		if (len(stardatalines[i])<4):
			starcoords.append((1./0.000001,1./0.00000001,1./0.000000000001))
			print('error '+str(stardatalines[i]))			
			continue
		rad = float(stardatalines[i][1])
		asc = float(stardatalines[i][2])
		dec = float(stardatalines[i][3])
		radcos = rad*math.cos(dec);
		coord = (radcos*math.sin(asc),radcos*math.cos(asc),rad*math.sin(dec))
		starcoords.append(coord)
	stardata=stardatalines
if (stardata):
	for s in systems:
		removeVal(s,'designation')
		coord = toPair(getVal(s,'xyz'))
		if (not coord):
			continue	
		for k in range(len(starcoords)):
			c=starcoords[k]
			x=c[0]-coord[0]
			y=c[1]-coord[1]
			z=c[2]-coord[2]
			if (x*x+y*y+z*z<.000001):
				if (not len(stardata[k][0])):
					break;
				print(s.getAttribute('name') +' same as '+stardata[k][0]);
				newchild = xml.dom.minidom.Element('var')
				newchild.setAttribute('name','designation')
				newchild.setAttribute('value',stardata[k][0])
				s.insertBefore(newchild,s.firstChild)
				break;

if (remakePlanets):
	for s in systems:
		removeVal(s,'planets')
		newchild = xml.dom.minidom.Element('var')
		newchild.setAttribute('name','planets')
		newchild.setAttribute('value',planetMakerHelper.getPlanetsString(getVal(s,'faction'),planets,float(getVal(s,"sun_radius")),s.getAttribute('name')))
		s.insertBefore(newchild,s.firstChild)
fil = open (sys.argv[2],"w")
fil.write(g.toxml())
fil.close()
