import sys
import xml
import xml.dom
import xml.dom.minidom
import math
stardatafile=None
arg=0
remakePlanets=0
while (arg<len(sys.argv)):
	match=0
	if sys.argv[arg].find("-csv")==0:
		stardatafile=sys.argv[arg][4:]
		match=1
	if sys.argv[arg].find("-replanet")==0:
		remakePlanets=1
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
def getValND(node,val):
	for i in node.getElementsByTagName('var'):
		if (i.getAttribute('name')==val):
			return i.getAttribute('value')
	return None
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

def CombineTables(Prob,Mult,fac,newkey,ret):
	tot=0	
	ret[(fac,newkey)]=Prob[fac].copy();
	for i in ret[(fac,newkey)]:
		if (i in Mult):
			ret[(fac,newkey)][i]=ret[(fac,newkey)][i]*Prob[fac][i];
		tot+=ret[(fac,newkey)][i]
	for i in Prob[fac]:
		ret[(fac,newkey)][i]/=tot;
	return ret;
	

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

starplanetmult={(70,0):{'Ice':8,
					 'Aera_Ice':8,
					 'Rocky':8,
					 'Uninhabitable_Dwarf_Gas_Giant':4},
		(70,1):{'Ice':8,
					 'Aera_Ice':4,
					 'Rocky':4,
					 'Uninhabitable_Medium_Gas_Giant':4},
		(70,2):{'Ice':8,
					 'Aera_Ice':4,
					 'Rocky':4,
					 'Oceanic':2,
					 'Uninhabitable_Medium_Gas_Giant':2,
					 'Uninhabitable_Dwarf_Gas_Giant':2,
					 'Uninhabitable_Dwarf_Gas_Giant':4},
		(60,0):{'Ice':3,
					 'Oceanic':2,
					 'Aera_Ice':3,
					 'Bio_Simple':2,
					 'Bio_Simple_Methane':2,
					 'Rocky':3,
					 'Overgrown':2,
					 'Overgrown_Methane':2},
		(60,1):{'Ice':3,
					 'Oceanic':2,
					 'Aera_Ice':3,
					 'Bio_Simple':2,
					 'Bio_Simple_Methane':2,
					 'Rocky':3,
					 'Uninhabitable_Dwarf_Gas_Giant':2,
					 'Uninhabitable_Medium_Gas_Giant':4},
		(60,2):{'Ice':3,
					 'Oceanic':2,
					 'Aera_Ice':3,
					 'Bio_Simple':2,
					 'Bio_Simple_Methane':2,
					 'Rocky':3,
					 'Uninhabitable_Dwarf_Gas_Giant':4,
					 'Uninhabitable_Medium_Gas_Giant':4,
					 'Uninhabitable_Gas_Giant':2},
		(50,0):{'Trantor_Class':2,
					 'Bio_Diverse':4,
					 'University':2,
					 'Tropical':2,
					 'Oceanic':4,
					 'Oceanic_Ammonia':2,
					 'Rlaan_Trantor':2,
					 'Bio_Diverse_Methane':2,
					 'Rocky':2,
					 'Overgrown':4,
					 'Overgrown_Methane':2},
		(50,1):{'Trantor_Class':2,
					 'Bio_Diverse':2,
					 'Bio_Simple':4,
					 'University':2,
					 'Tropical':2,
					 'Oceanic':2,
					 'Oceanic_Ammonia':2,
					 'Rlaan_Trantor':2,
					 'Bio_Diverse_Methane':4,
					 'Rocky':2,
					 'Overgrown':4,
					 'Overgrown_Methane':4,
					 'Uninhabitable_Gas_Giant':2},
		(50,2):{'Bio_Simple':4,
					 'University':2,
					 'Tropical':2,
					 'Oceanic':2,
					 'Oceanic_Ammonia':2,
					 'Rlaan_Trantor':2,
					 'Rocky':4,
					 'Bio_Diverse_Methane':4,
					 'Overgrown':4,
					 'Overgrown_Methane':4,
					 'Uninhabitable_Gas_Giant':2},
		(40,0):{},#default
		(40,1):{'Arid':8,
					 'Arid_Methane':8,
					 'Volcanic':8,
					 'Bio_Simple_Methane':6,
					 'Bio_Simple':8,
					 'Uninhabitable_Gas_Giant':2,
					 'Uninhabitable_Medium_Gas_Giant':4,
					 'Uninhabitable_Dwarf_Gas_Giant':2},
		(40,2):{'Arid':10,
					 'Arid_Methane':8,
					 'Volcanic':8,
					 'Bio_Simple_Methane':2,
					 'Bio_Simple':2,
					 'Uninhabitable_Gas_Giant':4,
					 'Uninhabitable_Medium_Gas_Giant':4,
					 'Uninhabitable_Dwarf_Gas_Giant':2},
		(30,0):{'Arid':4,
					 'Arid_Methane':4,
					 'Volcanic':4,
					 'Molten':1./32,
					 'Uninhabitable_Gas_Giant':4,
					 'Uninhabitable_Dwarf_Gas_Giant':2},
		(30,1):{'Arid':4,
					 'Arid_Methane':4,
					 'Volcanic':4,
					 'Molten':4,
					 'Uninhabitable_Gas_Giant':8,
					 'Uninhabitable_Medium_Gas_Giant':2},
		(30,2):{'Arid':4,
					 'Arid_Methane':4,
					 'Volcanic':4,
					 'Molten':4,
					 'Uninhabitable_Gas_Giant':8},
		(20,0):{'Arid':8,
					 'Arid_Methane':8,
					 'Volcanic':4,
					 'Molten':8,
					 'Uninhabitable_Dwarf_Gas_Giant':4},
		(20,1):{'Arid':8,
					 'Arid_Methane':8,
					 'Volcanic':4,
					 'Molten':8,
					 'Uninhabitable_Dwarf_Gas_Giant':4,
					 'Uninhabitable_Medium_Gas_Giant':2},
		(20,2):{'Arid':8,
					 'Arid_Methane':8,
					 'Volcanic':8,
					 'Molten':8,
					 'Uninhabitable_Medium_Gas_Giant':4,
					 'Uninhabitable_Dwarf_Gas_Giant':4,
					 'Uninhabitable_Gas_Giant':8},
		(10,0):{'Arid':8,
					 'Arid_Methane':12,
					 'Volcanic':12,
					 'Molten':12,
					 'Uninhabitable_Medium_Gas_Giant':4,
					 'Uninhabitable_Dwarf_Gas_Giant':4},
		(10,1):{'Arid':16,
					 'Arid_Methane':16,
					 'Volcanic':12,
					 'Molten':32,
					 'Uninhabitable_Gas_Giant':12},
		(10,2):{'Arid':16,
					 'Arid_Methane':12,
					 'Volcanic':12,
					 'Molten':32,
					 'Uninhabitable_Gas_Giant':12}
}

giant_moon={'Rocky':1./2,
		'Oceanic':1./4,
		'Oceanic_Ammonia':1./8,
		'Overgrown':1./8}
normal_moon={'Rocky':3./4,
		'Oceanic':1./16,
		'Oceanic_Ammonia':1./16,
		'Volcanic':1./8}
cold_moon={'Rocky':1./4,
		'Ice':1./2,
		'Frozen_Ammonia':1./4}
hot_moon={'Molten':1./2,
		'Volcanic':1./4,
		'Arid':1./4}

	
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
					 'Overgrown_Methane':1./16,
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
					 'Overgrown_Methane':1./16,
					 'Uninhabitable_Gas_Giant':1./16,
					 'Uninhabitable_Medium_Gas_Giant':1./16,
					 'Uninhabitable_Dwarf_Gas_Giant':1./16},
			#FIXME use these probabilities
			'Uncultivable' : {'Rocky':1./8,
							  'Molten':1./8,
							  'Ice':1./16,
							  'Arid':1./32,
							  'Arid_Methane':1./32,
							  'Frozen_Ammonia':1./8,
							  'Volcanic':1./8,
							  'Uninhabitable_Gas_Giant':1./8,
							  'Uninhabitable_Medium_Gas_Giant':1./8,
							  'Uninhabitable_Dwarf_Gas_Giant':1./8},
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
					 'Overgrown_Methane':1./32,
					 'Uninhabitable_Gas_Giant':1./16,
					 'Uninhabitable_Medium_Gas_Giant':1./32,
					 'Uninhabitable_Dwarf_Gas_Giant':1./16}
			}
					 
moonprob={'Trantor_Class':(1./2,1./8,giant_moon),
		  'Arid':(1./8,1./16,hot_moon),
		  'Arid_Methane':(1./8,1./16,normal_moon),
		  'Bio_Diverse':(1./2,1./8,normal_moon),
		  'University':(1./2.,1./8,normal_moon),
		  'Ice':(1./8,1./16,cold_moon),
		  'Tropical':(1./2,1./8,normal_moon),
		  'Oceanic':(1./8,1./16,normal_moon),
		  'Oceanic_Ammonia':(1./8,1./16,normal_moon),
		  'Aera_Trantor':(1./2,1./8,giant_moon),
		  'Rlaan_Trantor':(1./2,1./8,giant_moon),
		  'Aera_Ice':(1./8,1./16,cold_moon),
		  'Bio_Simple':(1./4,1./12,normal_moon),
		  'Frozen_Ammonia':(1./8,1./16,cold_moon),
		  'Volcanic':(1./8,1./16,normal_moon),
		  'Bio_Diverse_Methane':(1./2.,1./8,normal_moon),
		  'Bio_Simple_Methane':(1./4.,1./16,normal_moon),
		  'Rocky':(1./8,1./16,normal_moon),
		  'Molten':(1./8,1./16,hot_moon),
		  'Overgrown':(1./8,1./16,normal_moon),
		  'Overgrown_Methane':(1./8,1./16,normal_moon),
		  'Uninhabitable_Gas_Giant':(.96975,1./4,giant_moon),
		  'Uninhabitable_Medium_Gas_Giant':(.75,1./4,giant_moon),
		  'Uninhabitable_Dwarf_Gas_Giant':(.5,1./8,giant_moon),
		  None:(1./8,1./16,normal_moon)}

ordering={'Trantor_Class':4,
		  'Arid':2,
		  'Arid_Methane':4,
		  'Bio_Diverse':4,
		  'University':4,
		  'Ice':6,
		  'Tropical':3,
		  'Oceanic':4,
		  'Oceanic_Ammonia':5,
		  'Aera_Trantor':3,
		  'Rlaan_Trantor':4,
		  'Aera_Ice':5,
		  'Bio_Simple':3,
		  'Frozen_Ammonia':7,
		  'Volcanic':2,
		  'Bio_Diverse_Methane':3,
		  'Bio_Simple_Methane':4,
		  'Rocky':3,
		  'Molten':1,
		  'Overgrown':3,
		  'Overgrown_Methane':4,
		  'Uninhabitable_Gas_Giant':5,
		  'Uninhabitable_Medium_Gas_Giant':5,
		  'Uninhabitable_Dwarf_Gas_Giant':4,
		  None:4}
combined_planet_prob={}

def planets_compare(x,y):
	return ordering[x]-ordering[y];
import random
rak=random.Random(31337)
def getBody(plist):
	rfloat=rak.random()
	for i in plist:
		if (rfloat<plist[i]):
			return i
		rfloat-=plist[i]
	return plist.items()[len(plist)-1][0]

def getPlanet(fac,radpair=None):
	global combined_planet_prob
	if not fac in planetprob:
		fac=None
	if not (fac,radpair) in combined_planet_prob:
		if (radpair in starplanetmult):
			combined_planet_prob=CombineTables(planetprob,starplanetmult[radpair],fac,radpair,combined_planet_prob)
		else:
			radpair=None		
	if (radpair==None):
		plist=planetprob[fac]
	else:
		plist = combined_planet_prob[(fac,radpair)]
		#print str( (fac,radpair) )+" plist: " +str(plist)
		#tot=0
		#for i in plist:
		#	tot+=plist[i];
		#print "total: "+str(tot);
	return getBody(plist)
def abbreviate(l,planets):
	ret=[]
	for longname in l:
		found=0
		for p in planets:
			if p.getAttribute('name')==longname:
				abbrev=getValND(p,'initial')
				if (abbrev):
					found=1
					ret.append(abbrev)
					break
		if (not found):
			print 'Error no abbreviation! for '+longname	
	return ret
			
def getPlanets(fac,planets, sun_radius):
	import starCodes
	pair=starCodes.sizeToNum(sun_radius)
	numplan=rak.randint(0,5)
	if (numplan>=4):
		numplan = rak.randint(1,10)
	plist=[]
	moons=[]
	moonindex=[]
	j=0
	for i in range(numplan):
		plan=getPlanet(fac,pair)
		plist.append(plan)
		mi=[]
		if (plan in moonprob):
			iter=0
			while (rak.random()<moonprob[plan][iter]):
				iter=1
				moons.append(getBody(moonprob[plan][2]));
				mi.append(j);
				j+=1;
		moonindex.append(mi)
	plist.sort(planets_compare)
	moons.sort(planets_compare)
	plist=abbreviate(plist,planets)
	moons=abbreviate(moons,planets)
	ret=[]
	for i in range(len(plist)):
		ret.append(plist[i])
		for ind in moonindex[i]:
			mon = moons[ind]
			mon = '*'+mon
			ret.append(mon)
	return ret
def getPlanetsString(fac,planets, sun_radius):
	ret=""
	for p in getPlanets(fac,planets,sun_radius):
		if (len(ret)):
			ret+=' '
		ret+=p
	return ret
if (remakePlanets):
	for s in systems:
		removeVal(s,'planets')
		newchild = xml.dom.minidom.Element('var')
		newchild.setAttribute('name','planets')
		newchild.setAttribute('value',getPlanetsString(getVal(s,'faction'),planets,float(getVal(s,"sun_radius"))))
		s.insertBefore(newchild,s.firstChild)
	
fil = open (sys.argv[2],"w")
fil.write(g.toxml())
fil.close()
