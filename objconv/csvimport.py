#!/usr/bin/python

def CollapseStruct(s):
	l=""
	for i in range(0,len(s),2):
		if i!=0:
			l+=";"
		l+=s[i];
	return l
def removeBraces(s):
	r=""
	s=s.split("{");
	for k in s:
		r+=k
	r=r.split("}")
	s=""
	for k in r:
		s+=k
	return s
def CollapseList(lis):
	s=""
	r=[]
	i=0
	l=[]
	for i in lis:
		op=0
		clo=0
		guide=0
		if (i.find("{")!=-1):
			op=1
		if (i.find("}")!=-1):
			clo=1
		if (i.find("=")!=-1):
			guide=1
		if (op+clo+guide>1):
			if (clo):
				l.append("}")
			if (op):
				l.append("{")
			if(guide):
				l.append(removeBraces(i))
		else:
			l.append(i)
	for i in range(0,len(l),2):
		if l[i]=='{':
			r.append([])
		else:
			r[len(r)-1].append(l[i])
	found=0
	for i in r[len(r)-1]:
		if i!="":
			found=1
	if (0==found):
		r=r[0:len(r)-1]
	for i in r:
		s+='{'
		for j in range(len(i)):
			if (j!=0):
				s+=";"
			s+=i[j]
		s+='}'
	return s;


def ProcessUnit(f):
	l=[]
	line=f.readline()
	while (line!=""):
		x=csv.semiColonSeparatedList(line,',')
		if (len(x)>=2):
			if (len(x)==2):
				l.append(x[1])
			elif (x[1].find('{')!=-1):
				l.append(CollapseList(x[1:]))
			else:
				l.append(CollapseStruct(x[2:]))
		else:
			l.append('')
		line=f.readline()
		
	return l
import sys
import csv
units=sys.argv[1]
unit=sys.argv[2]
f=open (units);
o=open(units+".tmp","w");
u=open(unit+".csv");
line=f.readline()
while (line!=""):
	if (csv.earlyStrCmp(line,unit)):
		o.write(csv.writeList(ProcessUnit(u)));
	else:
		o.write(line);
	line=f.readline();

o.close();
u.close()
f.close()
import os
os.rename (units+".tmp",units);
