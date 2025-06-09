#!/usr/bin/python
##
# wcSystemReader.py
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
import csv

def writeXML(secs):
	s="<galaxy>\n<systems>\n";
	for sec in secs:
		s+="\t<sector name=\""+sec+"\">\n"
		for sys in secs[sec]:
			s+="\t\t<system name=\""+sys+"\">\n"
			sys=secs[sec][sys]
			for prop in sys:
				s+="\t\t\t<var name=\""+prop+"\" value=\""+sys[prop]+"\"/>\n";
			s+="\t\t</system>\n"
		s+="\t</sector>\n";
	s+="</systems>\n</galaxy>\n";
	return s

def RemoveStr(st, typ):
	where=st.find(typ)
	while (where!=-1):
		st = st[0:where]+st[where+1:]
		where=st.find(typ)
	return st
def Prettify (st):
	return RemoveStr(RemoveStr(RemoveStr(RemoveStr(RemoveStr(st,'"'),"'")," "),"/"),"\\");

def InfluenceToFaction(inf):
	if inf.find("Terran")!=-1 and inf.find("Confed")!=-1:
		return "confed"
	if (inf.find("Unexplored")!=-1):
		return "unknown"
	if (inf.capitalize().find("Kilrathi")!=-1):
		return "kilrathi"
	if (inf.find("Landreich")!=-1):
		return "landreich"
	if (inf.find("Border")!=-1):
		return "border_worlds"
	if  (inf.find("kkan")!=-1):
		return "firekkan"
	print("error faction "+inf+" unknown")
	return "border_worlds"


if len(sys.argv)>1:
	jumps={}
	arg=sys.argv[1]
	linknam=sys.argv[2]
	f = open (arg)
	lis = f.readlines();
	olist=[]
#	lis.sort()
	for l in lis:
		if (len(olist)!=0):
			if (l==olist[len(olist)-1]):
				continue
		olist=olist+[l];
	f.close()
#	f = open (arg,"w")
#	f.writelines(olist);
#	f.close();
	linkfil=open(linknam)
	link=linkfil.readlines();
	for l in link:
		m=csv.semiColonSeparatedList(l,";")
		if not m[0] in jumps:
			jumps[m[0]]=[]
		if not m[2] in jumps:
			jumps[m[2]]=[]
		if (not m[2] in jumps[m[0]]):
			jumps[m[0]].append(m[2])
		if (not m[0] in jumps[m[2]]):
			jumps[m[2]].append(m[0])
#now to read this sucker
	for i in range(len(olist)):
		olist[i]=csv.semiColonSeparatedList(olist[i],';')
	tab = csv.makeTable(olist );
	secs={}
	for i in tab:
		sys=tab[i]
		h={}
#		print i + str(sys)
		sec=Prettify(sys["SectorName"])
		name = Prettify(sys["SystemName"])
		import starCodes
		h["sun_radius"]=str(starCodes.codeToSize(sys["StarColorType"]))
		h["xyz"]=sys["XCoordinates"]+" "+sys["YCoordinates"]+" "+sys["ZCoordinates"];
		h["quadrant"]=Prettify(sys["QuadrantName"])
		h["faction"]=InfluenceToFaction(sys["Influence"])
		jamp=""
		if (i in jumps):
			for k in jumps[i]:
				if not k in tab:
					print(k+" missing from system list")
					continue
				if (jamp!=""):
					jamp+=" "
				jamp+=Prettify(tab[k]["SectorName"])+"/"+Prettify(tab[k]["SystemName"])
			h["jumps"]=jamp
		else:
			print("no jumps for "+i+" "+name)
		if not sec in secs:
			secs[sec]={}
		secs[sec][name]=h
	ret=writeXML(secs)
	f=open("output.xml","w")
	f.write(ret)
	f.close()
