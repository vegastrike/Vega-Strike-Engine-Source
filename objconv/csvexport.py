#!/usr/bin/python
import sys
import csv
args=sys.argv[2:]
file=sys.argv[1]
key=[]
guide=[]
f=open(file)
showunits=0
def interleave (l1,l2,add1,add2):
	ret=[]
	for i in range(len(l1)):
		ret.append(add1+l1[i]+add2);
		ret.append(l2[i]);
	return ret
def earlyStrCmp(haystack,needle):
	ln = len(needle)
	lh = len(haystack)
	if lh>=ln+7:	
		return (haystack[0:ln]==needle and haystack[ln]==',') or (haystack[0]=='"' and ((haystack[1:ln+1]==needle and haystack[ln+2]==',') or (haystack[3:ln+3]==needle and haystack[ln+6]==',')))
	return 0
def makeName(nam,guide,useguide):
	if (useguide):
		return nam+'('+guide+')'
	return nam
def ProcessStruct (guide,struc):
	if (len(struc)==0):
		for i in range(guide.count(";")):
			struc+=";"
	l=struc.split(';')
	g=guide.split(';')	
	return interleave(g,l," ",'=');
def ProcessList(guide,row):
	og=guide.find('{');
	cg=guide.find('}');
	if (og==-1 or cg== -1):
		print "error in "+str(row)+" "+str(guide)
		return ""
	guide=guide[og+1:cg]
	ret=[]
	while(1):
		_or=row.find('{');
		_cr=row.find('}');
		if (_or==-1 or _cr==-1):
			break;
		ret+=['{']
		ret+=ProcessStruct(guide,row[_or+1:_cr])		
		row=row[_cr+1:]
		ret+=['}']

	ret+=['{']
	ret+=ProcessStruct(guide,"")
	ret+=['}']
	return ret


key=csv.semiColonSeparatedList(f.readline().strip(),',')
guide=csv.semiColonSeparatedList(f.readline().strip(),',')
for i in args:
	if (i=="-units"):
		showunits=1
arg=args[-1]
line= f.readline().strip();
while len(line):
	if (earlyStrCmp(line,arg)):
		row = csv.semiColonSeparatedList(line,',')
		length=len(row)
		if (length!=len(key) or length!=len(guide)):
			print "Mismatch in row length "+str(length)+ " for "+arg+" with key len = "+str(len(key))+" and guide "+ str(len(guide))
			break
		ff=open(arg+".csv","w");
		for i in range(length):
			if guide[i].find('{')!=-1:
				ff.write(csv.writeList([key[i]]+ProcessList(guide[i],row[i])))
			elif guide[i].find(';')!=-1:
				ff.write(csv.writeList([key[i]]+ProcessStruct(guide[i],row[i])))
			else:
				ff.write(csv.writeList([makeName(key[i],guide[i],showunits),
							row[i]]));
		ff.close();
		break
	line=f.readline().strip()	
f.close()
