#!/usr/bin/python
import sys
def sameending (s):
	lastpartwhere=s.rfind("/");
	if (lastpartwhere==-1):
		lastpartwhere=s.rfind("\\");
	if (lastpartwhere!=-1):
		name=s[lastpartwhere+1:]
		s=s[0:lastpartwhere]
		where=s.rfind("/");
		if (where==-1):
			where=s.rfind("\\");
		if (where!=-1):
			dir=s[where+1:]
		else:
			return 0
		if (name.find(dir)==0):
			if (len(name)==len(dir)):
				return 1
			if(name[len(dir)]=='.'):
				ss=name[len(dir)+1:]
				if (ss!="bfxm" and ss!="png" and ss!="old" and ss!="xmesh" and ss!="spr" and ss!="jpg" and ss!="py" and ss!="xmesh"):
					return 1
	return 0;
for arg in sys.argv[1:]:
	str=""
	if sameending(arg):	
		print arg
