import sys
import xml
import xml.dom
import xml.dom.minidom
import math
stardatafile=None
arg=0
while (arg<len(sys.argv)):
	match=0
	if sys.argv[arg].find("-csv")==0:
		stardatafile=sys.argv[arg][4:]
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
			print 'error '+str(stardatalines[i])			
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
				print s.getAttribute('name') +' same as '+stardata[k][0];
				newchild = xml.dom.minidom.Element('var')
				newchild.setAttribute('name','designation')
				newchild.setAttribute('value',stardata[k][0])
				s.insertBefore(newchild,s.firstChild)
				break;

#################fixme Insert code to do probability distribution and add planets##################

			
fil = open (sys.argv[2],"w")
fil.write(g.toxml())
fil.close()
