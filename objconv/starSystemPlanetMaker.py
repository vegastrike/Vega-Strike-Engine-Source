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
planetprob={'rlaan':{'Trantor_Class':1./1024,
					 'Arid':1./128,
					 'Arid_Methane':1./32,
					 'Bio_Diverse':1./128,
					 'University':1./1024.,
					 'Ice':1./32,
					 'Tropical':1./128,
					 'Oceanic':1./32,
					 'Oceanic_Ammonia':1./32,
					 'Aera_Trantor':1./1024,
					 'Rlaan_Trantor':1./32,
					 'Aera_Ice':1./1024,
					 'Bio_Simple':1./128,
					 'Frozen_Ammonia':1./32,
					 'Volcanic':1./32,
					 'Bio_Diverse_Methane':1./8.,
					 'Bio_Simple_Methane':1./4.,
					 'Rocky':1./32,
					 'Molten':1./32,
					 'Overgrown':1./32,
					 'OvergrownMethane':1./16,
					 'Uninhabitable_Gas_Giant':1./16,
					 'Uninhabitable_Medium_Gas_Giant':1./16,
					 'Uninhabitable_Dwarf_Gas_Giant':1./16},
			'aera': {'Trantor_Class':1./1024,
					 'Arid':1./32,
					 'Arid_Methane':1./128,
					 'Bio_Diverse':1./16,
					 'University':1./1024.,
					 'Ice':1./1024,
					 'Tropical':1./128,
					 'Oceanic':1./32,
					 'Oceanic_Ammonia':1./32,
					 'Aera_Trantor':1./32,
					 'Rlaan_Trantor':1./1024,
					 'Aera_Ice':1./32,
					 'Bio_Simple':1./8,
					 'Frozen_Ammonia':1./32,
					 'Volcanic':1./32,
					 'Bio_Diverse_Methane':1./128.,
					 'Bio_Simple_Methane':1./128.,
					 'Rocky':1./32,
					 'Molten':1./32,
					 'Overgrown':1./32+1./8,
					 'OvergrownMethane':1./16,
					 'Uninhabitable_Gas_Giant':1./16,
					 'Uninhabitable_Medium_Gas_Giant':1./16,
					 'Uninhabitable_Dwarf_Gas_Giant':1./16},
			'moon': {'Rocky':1./4,
					 'Molten':1./8,
					 'Ice':1./16,
					 'Frozen_Ammonia':1./16,
					 'Volcanic':1./8,
					 'Oceanic':1./8,
					 'Oceanic_Ammonia':1./8,
					 'Overgrown':1./8},
			None:   {'Trantor_Class':1./16,
					 'Arid':1./16,
					 'Arid_Methane':1./128,
					 'Bio_Diverse':1./4,
					 'University':1./16.,
					 'Ice':1./16,
					 'Tropical':1./32,
					 'Oceanic':1./32,
					 'Oceanic_Ammonia':1./64,
					 'Aera_Trantor':1./1024,
					 'Rlaan_Trantor':1./1024,
					 'Aera_Ice':1./128,
					 'Bio_Simple':1./32,
					 'Frozen_Ammonia':1./64,
					 'Volcanic':1./32,
					 'Bio_Diverse_Methane':1./128.,
					 'Bio_Simple_Methane':1./128.,
					 'Rocky':1./32,
					 'Molten':1./32,
					 'Overgrown':1./16,
					 'OvergrownMethane':1./32,
					 'Uninhabitable_Gas_Giant':1./16,
					 'Uninhabitable_Medium_Gas_Giant':1./32,
					 'Uninhabitable_Dwarf_Gas_Giant':1./16}
			}
					 
					 
		  
		  
		  
			
fil = open (sys.argv[2],"w")
fil.write(g.toxml())
fil.close()
