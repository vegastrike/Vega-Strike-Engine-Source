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
			print tmp
			tmp=elimiQuote(tmp,'"')
		ret=ret+tmp
		even = 1-even
		s=s[where+len(delim):]
		where=findQuot(s,delim)
	ret=ret+s
	return ret
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
	s+='\n'
	return s
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
	print "lust" +str(keylist)
	for l in llist[1:]:
		key=l[keynum]
		mhash={}
		for i in range (len(l)):
			try:
				mhash[keylist[i]]=l[i]
			except:
				print str(l)+":: "+str(l[i])+" not in keys "
				
		ret[key]=mhash
	return ret