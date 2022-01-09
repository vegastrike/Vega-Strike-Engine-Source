##
# csv.py
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

def findQuot(s,chr='"', offset=0):
	if (offset>=len(s)):
		return -1
	quot=s[offset:].find(chr)	
	if (quot!=-1):
		return quot+offset
	return -1
def elimiQuote(s, delim='"""'):
	ret="";
	where=findQuot(s,delim)
	even = 1
	if (where==-1 and delim!='"'):
		s=elimiQuote(s,'"')
	while(where!=-1):
		tmp=s[0:where]
		if (even==1 and delim!='"'):
			print(tmp)
			tmp=elimiQuote(tmp,'"')
		ret=ret+tmp
		even = 1-even
		s=s[where+len(delim):]
		where=findQuot(s,delim)
	ret=ret+s
	return ret
def earlyStrCmp(haystack,needle):
	ln = len(needle)
	lh = len(haystack)
	if lh>=ln+7:	
		return (haystack[0:ln]==needle and haystack[ln]==',') or (haystack[0]=='"' and ((haystack[1:ln+1]==needle and haystack[ln+2]==',') or (haystack[3:ln+3]==needle and haystack[ln+6]==',')))
	return 0

def writeList(lis,delim=','):
	s=""
	for i in range(len(lis)):
		l=lis[i]
		if (l.find('"')!=-1):
			s+='"""'+l+'"""';
		elif (l.find(delim)!=-1):
			s+='"'+l+'"'
		else:
			s+=l
		if (i!=len(lis)-1):
			s+=delim
		else:
			if (l==''):
				s+='""'
	s+='\n'
	return s
def commaSeparatedList(s,delim=','):
	return semiColonSeparatedList(s,delim)
def semiColonSeparatedList(s, delim=','):
	s = s.strip().rstrip()
	trip=1
	quot3 = findQuot(s,'"""')
	quot1=findQuot(s,'"')
	if (quot3==-1 or (quot1!=-1 and quot1<quot3)):
		trip=0
		quot=quot1
	else:
		quot=quot3		
	sem = s.find(delim);	
	l=[]
	while sem!=-1:
		equot=0
		while (quot<sem and quot!=-1):
			if (trip):
				equot=findQuot(s,'"""',quot+1)
			else:
				equot=findQuot(s,'"',quot+1)	
			quot3=findQuot(s,'"""',equot+1)
			quot1 = findQuot(s,'"',equot+1)
			trip=1
			quot=quot3
			if (quot3==-1 or (quot1!=-1 and quot1<quot3)):
				quot=quot1			
				trip=0
		sem = s[equot:].find(delim);
		if (sem!=-1):
			sem=sem+equot;
		else:
			break;
		l.append(elimiQuote(s[0:sem]))
		s=s[sem+len(delim):]
		sem = s.find(delim);
		quot3=findQuot(s,'"""')
		quot1 = findQuot(s,'"')
		quot=quot3;
		trip=1
		if (quot3==-1 or (quot1!=-1 and quot1<quot3)):
			quot=quot1			
			trip=0

	if (len(s)):
		l.append(elimiQuote(s))
	return l


def makeTable(llist, keynum=0):
	keylist = llist[0];
	ret={}
	print("lust" +str(keylist))
	for l in llist[1:]:
		key=l[keynum]
		mhash={}
		for i in range (len(l)):
			try:
				mhash[keylist[i]]=l[i]
			except:
				print(str(l)+":: "+str(l[i])+" not in keys ")
				
		ret[key]=mhash
	return ret
