#!/usr/bin/python
import sys

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
	while(where!=-1):
		tmp=s[0:where]
		if (even==1 and delim!='"'):
			print tmp
			tmp=elimiQuote(tmp,'"')
		ret=ret+tmp
		even = 1-even
		s=s[where+len(delim):]
		where=findQuot(s,delim)
	ret=ret+s
	return ret
def semiColonSeparatedList(s):
	s = s.strip().rstrip()
	trip=1
	quot3 = findQuot(s,'"""')
	quot1=findQuot(s,'"')
	if (quot3==-1 or (quot1!=-1 and quot1<quot3)):
		trip=0
		quot=quot1
	else:
		quot=quot3		
	sem = s.find(";");	
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
		sem = s[equot:].find(";");
		if (sem!=-1):
			sem=sem+equot;
		else:
			break;
		l.append(elimiQuote(s[0:sem]))
		s=s[sem+1:]
		sem = s.find(";");
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
	


for arg in sys.argv[1:]:
	f = open (arg)
	lis = f.readlines();
	olist=[]
	lis.sort()
	for l in lis:
		if (len(olist)!=0):
			if (l==olist[len(olist)-1]):
				continue
		olist=olist+[l];
	f.close()
#	f = open (arg,"w")
#	f.writelines(olist);
#	f.close();
#now to read this sucker
	print semiColonSeparatedList(olist[0]);
