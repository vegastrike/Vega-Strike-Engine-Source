def numToSize(type,size):
#code lifted from readstarsystem.cpp;
# modify there
		rad=16000;
		lifeprob= .25;
		if (type<30):
			rad=(type+size)*4000./32;
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

def codeToNum(code):
	codes=code.split(" ");
	sub=1
	if (codes[1].find('V')==0):
		sub=0
	elif (codes[1].find('III')==-1):
		sub=2
	return (TypToChar(codes[0][0])+int(codes[0][1:]),sub);
def codeToSize(code):
	(num,sub)=codeToNum(code);
	return numToSize(num,sub)
def sizeToNum(size):
	for i in range (200):
		for j in range (3):
			if (numToSize(i,j)==size):
				return (i,j)
	print "Size "+str(size)+" failed"
	return (50,1)