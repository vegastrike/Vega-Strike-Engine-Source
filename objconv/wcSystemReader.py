#!/usr/bin/python
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

def numToSize(type,size):
#code lifted from readstarsystem.cpp;
# modify there
		rad=16000;
		lifeprob= .25;
		if (type<30):
			rad=type*4000./32;
			lifeprob=.01;
		elif (type < 40):
			rad = 6000;
			lifeprob=.02;
			if (size==0):
				rad = 4200;
			elif (size==2):
				rad = 7500;
		elif (type < 50):
			lifeprob=.05;
			rad = 14250;
			if (size==0):
				rad= 13600;
				lifeprob=.08;
			elif (size==2):
				rad= 14750;
		elif (type < 60):
			
			lifeprob = .125;
			rad = 25000;
			if (size==0):
				rad= 16600;
				lifeprob = .25;
				#if (xyz.x*xyz.x+xyz.y*xyz.y+xyz.z*xyz.z>500*500):
				#	lifeprob=1;
			elif (size==2):
				rad= 36500;
				lifeprob = .0625;
		elif (type < 70):
			rad = 50000;
			lifeprob = .02;
			if (size==0):
				lifeprob = .125;
				rad = 37000;
			elif (size==2):
				rad = 75000;			
		elif (type < 80):
			rad = 85000;
			lifeprob = .005;
			if (size==0):
				rad =10000;
				lifeprob = .125;
			elif (size==2):
				rad = 150000;		
		return rad
def RemoveStr(st, typ):
	where=st.find(typ)
	while (where!=-1):
		st = st[0:where]+st[where+1:]
		where=st.find(typ)
	return st
def Prettify (st):	
	return RemoveStr(RemoveStr(RemoveStr(RemoveStr(RemoveStr(st,'"'),"'")," "),"/"),"\\");
def TypToChar(ch):
	ch.capitalize()
	if (ch=='O'):
		 return 10
	if (ch=='B'):
		return 20
	if (ch=='A'):
		return 30
	if (ch=='F'):
		return 40
	if (ch=='G'):
		return 50
	if (ch=='K'):
		return 60
	if (ch=='M'):
		return 70
	return 70

def codeToSize(code):
	codes=code.split(" ");
	sub=1
	if (codes[1].find('V')==0):
		sub=0
	elif (codes[1].find('III')==-1):
		sub=2
	
	return numToSize(TypToChar(codes[0][0])+int(codes[0][1:]),sub)

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
	print "error faction "+inf+" unknown"
	return "border_worlds"


jumps={}
arg=sys.argv[1]
linknam=sys.argv[2]
if 1:
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
		h["sun_radius"]=str(codeToSize(sys["StarColorType"]))
		h["xyz"]=sys["XCoordinates"]+" "+sys["YCoordinates"]+" "+sys["ZCoordinates"];
		h["quadrant"]=Prettify(sys["QuadrantName"])
		h["faction"]=InfluenceToFaction(sys["Influence"])
		jamp=""
		if (i in jumps):
			for k in jumps[i]:
				if not k in tab:
					print k+" missing from system list"
					continue
				if (jamp!=""):
					jamp+=" "
				jamp+=Prettify(tab[k]["SectorName"])+"/"+Prettify(tab[k]["SystemName"])
			h["jumps"]=jamp
		else:
			print "no jumps for "+i+" "+name
		if not sec in secs:
			secs[sec]={}
		secs[sec][name]=h
	ret=writeXML(secs)
	f=open("output.xml","w")
	f.write(ret)
	f.close()