#WARNING THIS FILE HAS A MIRROR FILE IN C++ FOR SIMILAR ACCESS...
#SO THAT C++ CAN LEARN ABOUT THE DYNAMIC UNIVERSE SHIPS
#
import Director
import VS
import vsrandom
import Vector
import faction_ships
from universe import AllSystems

import ShowProgress
import debug

ccp=VS.getCurrentPlayer()
_tweaktuple = ('Squadron','Prime','Arc','Alpha','Aleph','Beta','Quadratis','Zeta','X','Plus','Blade','Delta','Dash','Xprime','Gamma','Hydris','Dual','Tri','Quad','Penta','Hex','Octo','Deca','Octate')

#Aera Plants
#Rlaan Rocks
#Hum0n Cities
#Uln adVerbs
#klkk Adjectives
def MaxNumFlightgroupsInSystem (syst):
    if syst in faction_ships.max_flightgroups:
        return faction_ships.max_flightgroups[syst]
    else:
        return 3

def MinNumFlightgroupsInSystem (syst):
    if syst in faction_ships.min_flightgroups:
        return faction_ships.min_flightgroups[syst]
    else:
        return 1

def MaxNumBasesInSystem():
    return 10

def MinNumBasesInSystem():
    return 0

def MakeFactionKey (faction):
    return 'FF:'+str(VS.GetFactionIndex(faction))

def MakeFGKey (fgname,faction):
    return 'FG:'+str(fgname)+'|'+str(VS.GetFactionIndex(faction))

def MakeStarSystemFGKey (starsystem):
    return 'SS:'+str(starsystem)

def MakeStarSystemFactionFGKey(starsystem,faction):
    skey = MakeStarSystemFGKey (starsystem)
    index = VS.GetFactionIndex (faction)
    key = "%s[%d]" % (skey,index)
    return key

def ShipListOffset ():
    return 3

def PerShipDataSize ():
    return 3

def AllFactions ():
    facs =[]
    for i in xrange (VS.GetNumFactions()):
        facs.append(VS.GetFactionName(i))
    return facs

basenamelist={}
flightgroupnamelist={}
genericalphabet=['Alpha','Beta','Gamma','Delta','Epsilon','Zeta','Phi','Omega']

def ReadBaseNameList(faction):
    bnl=[]
    debug.debug('reading base names %s', faction)
    filename = 'universe/fgnames/'+faction+'.txt'
    try:
        f = open (filename,'r')
        bnl = f.readlines()
        f.close()
    except:
        try:
            f = open ('../'+filename,'r')
            bnl = f.readlines()
            f.close()
        except:
            try:
                f = open ('../universe/names.txt','r')
                bnl = f.readlines()
                f.close()
            except:
                try:
                    f = open ('universe/names.txt','r')
                    bnl = f.readlines()
                    f.close()
                except:
                    global genericalphabet
                    bnl=genericalphabet
    for i in xrange(len(bnl)):
        bnl[i]=bnl[i].rstrip()#.decode('utf8','ignore')
    import vsrandom
    vsrandom.shuffle(bnl)
    return bnl

def GetRandomFGNames (numflightgroups, faction):
    global flightgroupnamelist
    if (not (faction in flightgroupnamelist)):
        flightgroupnamelist[faction]=ReadBaseNameList(faction)
    if numflightgroups < 0:
        numflightgroups = len(flightgroupnamelist[faction])
    additional=[]
    if (numflightgroups>len(flightgroupnamelist[faction])):
        for i in xrange (numflightgroups-len(flightgroupnamelist)):
            additional.append(str(i))
    if (len(additional)+len(flightgroupnamelist[faction])==0):
        flightgroupnamelist[faction]=ReadBaseNameList(faction)
    return additional+flightgroupnamelist[faction]
basecounter=0

def GetRandomBaseName (n,faction):
    global basecounter
    retval=[]
    global basenamelist
    try:
        import seedrandom
        if (not (faction in basenamelist)):
            basenamelist[faction]=ReadBaseNameList(faction+'_base')
        retval = basenamelist[faction][basecounter:basecounter+n]
        while len(retval)<n:
            basecounter=n-len(retval)
            retval = retval + basenamelist[faction][:basecounter]
        basecounter = basecounter % len(basenamelist[faction])
    except:
        debug.error('uhoh base lsit wrong')
        retval=[]
        for i in xrange (n):
            retval+=[str(n)]
            n+=1
    return retval

origfgoffset=0

def TweakFGNames (origfgnames):
    global origfgoffset
    tweaker=str(origfgoffset)
    if (origfgoffset<len(_tweaktuple)):
        tweaker = _tweaktuple[origfgoffset]
    rez=[]
    for i in origfgnames:
        rez.append (i+'_'+tweaker)
    return rez
    
def maxTweakFGOffset():
    return len(_tweaktuple)
            
def WriteStringList(cp,key,tup):
    siz = Director.getSaveStringLength (cp,key)
    s_size=siz;
    lentup= len(tup)
    if (lentup<siz):
        siz=lentup
    for i in xrange(siz):
        Director.putSaveString(cp,key,i,tup[i])
    for i in xrange (s_size,lentup):
        Director.pushSaveString(cp,key,tup[i])
    while s_size > lentup:
        Director.eraseSaveString(cp,key,s_size-1)
	s_size -= 1

def ReadStringList (cp,key):
    siz = Director.getSaveStringLength (cp,key)
    tup =[]
    for i in xrange (siz):
        tup.append(Director.getSaveString(cp,key,i))
    return tup

def AllFlightgroups (faction):
    key = MakeFactionKey (faction)
    return ReadStringList(ccp,key)

def NumAllFlightgroups (faction):
    key = MakeFactionKey(faction)
    return Director.getSaveStringLength(ccp,key)

def RandomFlightgroup (faction):
    key = MakeFactionKey(faction)
    i = Director.getSaveStringLength(ccp,MakeFactionKey(faction))
    if (i==0):
        return ''
    import vsrandom
    return Director.getSaveString(ccp,key,vsrandom.randrange(0,i))

def ListToPipe (tup):
    fina=''
    if (len(tup)):
        fina=tup[0]
    for i in xrange (1,len(tup)):
        fina+='|'+tup[i]
    return fina

def _MakeFGString (starsystem,typenumlist):
    totalships = 0
    ret = []
    damage=0
    strlist=[]
    for tt in typenumlist:
        totalships+=int(tt[1])
        strlist+=[str(tt[0]),str(tt[1]),str(tt[1])]
    return [str(totalships),str(starsystem),str(damage)]+strlist
    
def _AddShipsToKnownFG(key,tnlist):
    leg = Director.getSaveStringLength (ccp,key)
    
    numtotships = int(Director.getSaveString(ccp,key,0))
    fgentry = {}
    for i in xrange(ShipListOffset(),leg,PerShipDataSize()):
	fgentry[Director.getSaveString(ccp,key,i)] = \
	    ( int(Director.getSaveString(ccp,key,i+1)) , \
	      int(Director.getSaveString(ccp,key,i+2)) , \
	      int(i), \
	      0  )

    for tn in tnlist:
	numtotships += int(tn[1])
	if tn[0] in fgentry:
	    fgentry[tn[0]] = ( \
		fgentry[tn[0]][0]+int(tn[1]), \
		fgentry[tn[0]][1]+int(tn[1]), \
		fgentry[tn[0]][2], \
		1 )
	else:
	    baseidx=\
	    Director.pushSaveString(ccp,key,str(tn[0]))
	    Director.pushSaveString(ccp,key,str(tn[1]))
	    Director.pushSaveString(ccp,key,str(tn[1]))
	    fgentry[tn[0]] = (int(tn[1]),int(tn[1]),int(baseidx),0)
	    
    Director.putSaveString(ccp,key,0,str(numtotships))
    for fg in fgentry:
	#only if dirty
	if fgentry[fg][3]:
	    Director.putSaveString(ccp,key,fgentry[fg][2]+1,str(fgentry[fg][0]))
	    Director.putSaveString(ccp,key,fgentry[fg][2]+2,str(fgentry[fg][1]))
    
def _AddFGToSystem (fgname,faction,starsystem):
    key = MakeStarSystemFactionFGKey(starsystem,faction)
    Director.pushSaveString(ccp,key,fgname)

def _RemoveFGFromSystem (fgname,faction,starsystem):
    key = MakeStarSystemFactionFGKey(starsystem,faction)
    leg = Director.getSaveStringLength(ccp,key)
    for index in xrange(leg):
	if Director.getSaveString(ccp,key,index) == fgname:
	    Director.eraseSaveString(ccp,key,index)
	    break

def _RemoveAllFGFromSystem (faction,starsystem):
    key = MakeStarSystemFactionFGKey(starsystem,faction)
    WriteStringList(ccp,key,[])

def _AddFGToFactionList(fgname,faction):
    key = MakeFactionKey(faction)
    Director.pushSaveString (ccp,key,fgname)
            
def _RemoveFGFromFactionList (fgname,faction):
    key = MakeFactionKey(faction)
    leg=Director.getSaveStringLength(ccp,key)
    for index in xrange(leg):
	if Director.getSaveString(ccp,key,index) == fgname:
	    Director.eraseSaveString(ccp,key,index)
	    return 1
    
def _RemoveAllFGFromFactionList(faction):
    key = MakeFactionKey(faction)
    WriteStringList(ccp,key,[])

def CheckFG (fgname,faction):
    key = MakeFGKey (fgname,faction)
    leg = Director.getSaveStringLength (ccp,key)
    totalships=0
    try:
	# Check ship counts
        for i in xrange (ShipListOffset()+1,leg,PerShipDataSize()):
            shipshere=Director.getSaveString(ccp,key,i)
            totalships+=int(shipshere)
            temp=Director.getSaveString(ccp,key,i+1)
            if (temp!=shipshere):
                debug.debug('correcting flightgroup %r to have right landed ships', fgname)
                Director.putSaveString(ccp,key,i+1,shipshere)#set num actives to zero
        #DEBUG ONLY if (totalships!=int(Director.getSaveString(ccp,key,0))):
        #    debug.debug('mismatch on flightgroup '+fgname+' faction '+faction)
        #    return 0
    except:
        debug.debug('nonint reading on flightgroup %r faction %r', fgname, faction)
        return 0
    return 1

def PurgeZeroShips (faction):
    return #perf problem...but we need it for persistent bases...so now it's in C++, not python---means this whole file is duplicated
    key=MakeFactionKey(faction)
    len=Director.getSaveStringLength (ccp,key)
    i=0
    debug.debug("purging 0 ships start")
    while i<len:
        curfg=Director.getSaveString(ccp,key,i)
        CheckFG (curfg,faction)
        #numships=NumShipsInFG(curfg,faction)
        #commented to allow 0 ship flightgroups for future reinforcement
        #if (numships==0):
        #    DeleteFG(curfg,faction)
        #    i-=1
        #    len-=1
        i+=1
    debug.debug("purging 0 ships end")        

def NumShipsInFG (fgname,faction):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len==0):
        return 0
    else:
        try:
            return int(Director.getSaveString(ccp,key,0))
        except:
            debug.debug("fatal: flightgroup without size")
            return 0

def GetDamageInFGPool (fgname,faction):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len<3):
        return 0
    else:
        try:
            return int(Director.getSaveString(ccp,key,2))
        except:
            debug.debug("nonfatal: flightgroup without size")
            return 0

def SetDamageInFGPool (fgname,faction,num):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len>2):
        Director.putSaveString(ccp,key,2,str(num))                                 

def DeleteFG(fgname,faction,deferaux=0):
    key = MakeFGKey (fgname,faction)
    leg = Director.getSaveStringLength (ccp,key)
    if (leg>=ShipListOffset()):
        starsystem=Director.getSaveString(ccp,key,1)
	if not deferaux:
    	    _RemoveFGFromSystem(fgname,faction,starsystem)
            _RemoveFGFromFactionList(fgname,faction)
        WriteStringList (ccp,key,[])
	
def AllFG(faction):
    return ReadStringList (ccp,MakeFactionKey (faction))
    
def SystemsWithFactionFGs(faction):
    sysSpan = {}
    rv = []
    for fgname in AllFG(faction):
        sysSpan[FGSystem(fgname,faction)] = 1
    for starsystem in sysSpan:
        rv.append(starsystem)
    return rv

def DeleteAllFG (faction):
    # Watch the ordering of operations! (won't work in most other permutations)
    debug.debug("DeleteAllFG from %s", faction)
    sysspan = SystemsWithFactionFGs(faction)
    for fgname in AllFG(faction):
	DeleteFG(fgname,faction,1)
    for starsystem in sysspan:
        _RemoveAllFGFromSystem(faction,starsystem)
    _RemoveAllFGFromFactionList(faction)

def DeleteAllFGFromAllSystems():
    for faction in AllFactions():
        DeleteAllFG(faction)

def DeleteLegacyFGs(sys):
    key = MakeStarSystemFGKey(sys)
    factionfglists = ReadStringList(ccp,key)
    for i in xrange(len(factionfglists)):
        faction = VS.GetFactionName(i)
        fglist = factionfglists[i].split('|')
        for fg in fglist:
            DeleteFG(fg,faction)


def DeleteLegacyFGLeftovers():
    # Legacy cleanup
    allsys = AllSystems()
    numsys = len(allsys)
    count = 0
    oldpct = 0
    for sys in allsys:
        count += 1
        pct = count*100/len(allsys)
        if pct != oldpct:
            ShowProgress.setProgressBar("loading",pct/100.)
            ShowProgress.setProgressMessage("loading","Resetting old universe (%d%%)" % (pct))
            oldpct = pct
        DeleteLegacyFGs(sys)
	
def HasLegacyFGFormat():
    for starsystem in AllSystems():
        key = MakeStarSystemFGKey(starsystem)
	if Director.getSaveStringLength(ccp,key)>0:
	    return 1
    return 0
    
def FGSystem (fgname,faction):
    key = MakeFGKey(fgname,faction)
    len = Director.getSaveStringLength(ccp,key)
    if (len>1):
        return Director.getSaveString(ccp,key,1)
    else:
        debug.debug('%r for %r already died, in no system', fgname, faction)
        return 'nil'
def TransferFG (fgname,faction,tosys):
    key = MakeFGKey(fgname,faction)
    len = Director.getSaveStringLength(ccp,key)
    if (len>1):
        starsystem=Director.getSaveString(ccp,key,1)
        _RemoveFGFromSystem(fgname,faction,starsystem)
        _AddFGToSystem(fgname,faction,tosys)
        Director.putSaveString(ccp,key,1,tosys)

def AddShipsToFG (fgname,faction,typenumbertuple,starsystem):
    key = MakeFGKey(fgname,faction) 
    len = Director.getSaveStringLength (ccp,key)
    if (len<ShipListOffset()):
        debug.debug('adding new fg %r of %s to %s', fgname, typenumbertuple, starsystem)
        WriteStringList(ccp,key,_MakeFGString( starsystem,typenumbertuple) )
        _AddFGToSystem (fgname,faction,starsystem)
        _AddFGToFactionList (fgname,faction)
        debug.info('adding new fg %r of %s to %s', fgname, typenumbertuple, starsystem)
    else:
        debug.info('adding old fg %r of %s to %s', fgname, typenumbertuple, FGSystem(fgname,faction))
        _AddShipsToKnownFG(key,typenumbertuple)

def RemoveShipFromFG (fgname,faction,type,numkill=1,landed=0):
    key = MakeFGKey (fgname,faction)
    leg = Director.getSaveStringLength (ccp,key)
    debug.debug("Scanning %d units...", leg)
    for i in xrange (ShipListOffset()+1,leg,PerShipDataSize()):
        if (Director.getSaveString(ccp,key,i-1)==str(type)):
            debug.debug("Removing unit %s", type)
            numships=0
            numlandedships=0
            try:
                numships = int (Director.getSaveString (ccp,key,i))
                numlandedships=int (Director.getSaveString (ccp,key,i+1))
            except:
                debug.error("unable to get savestring %s from FG %s %s %s", i, fgname, faction, type)
            if (numships>numkill):
                numships-=numkill
                if (numships<numlandedships):
                    if (landed==0):
                       debug.debug('trying to remove launched ship %s but all are landed', type)
                       landed=1
                       return 0#failur
                Director.putSaveString (ccp,key,i,str(numships))
                if (landed and numlandedships>0):
                    Director.putSaveString(ccp,key,i+1,str(numlandedships-numkill))
            else:
                numkill=numships
                numships=0
                for j in xrange (i-1,i+PerShipDataSize()-1):
                    Director.eraseSaveString(ccp,key,i-1)
            if (numships>=0):
                try:
                    totalnumships = int(Director.getSaveString(ccp,key,0))
                    totalnumships -=numkill
                    if (totalnumships>=0):
                        Director.putSaveString(ccp,key,0,str(totalnumships))
                        if(totalnumships==0):
			    debug.debug("Removing %s FG %r", faction, fgname)
                            DeleteFG(fgname,faction)
                    else:
                        debug.error('error...removing too many ships')
                except:
                    debug.error('error, flight record %r corrupt', fgname)
            return numkill
    debug.debug('cannot find ship to delete in %s fg %r', faction, fgname)
    return 0

def BaseFGInSystemName (system):
    return 'Base_'+system

def AllFGsInSystem(faction,system):
    key = MakeStarSystemFactionFGKey(system,faction)
    return ReadStringList(ccp,key)

def FGsInSystem(faction,system):
    ret = AllFGsInSystem(faction,system)
    basefg = BaseFGInSystemName(system)
    if (basefg in ret):
        del ret[ret.index(basefg)]
    return ret

def BaseFGInSystem(faction,system):
    ret = AllFGsInSystem(faction,system)
    basefg = BaseFGInSystemName(system)
    if (basefg in ret):
        return 1
    return 0

def BaseFG(faction,system):
    if (BaseFGInSystem(faction,system)):
        return LandedShipsInFG (BaseFGInSystemName(system),faction)             
    return []

def NumFactionFGsInSystem(faction,system):
    key = MakeStarSystemFactionFGKey(system,faction)
    return Director.getSaveStringLength(ccp,key)

def CountFactionShipsInSystem(faction,system):
    count=0
    st=''
    for fgs in FGsInSystem (faction,system):
        st+=fgs+' '
        ships=ReadStringList (ccp,MakeFGKey (fgs,faction))
        for num in xrange(ShipListOffset()+2,len(ships),PerShipDataSize()):
            try:
                count+= int(ships[num])
            except:
                debug.error('number ships '+ships[num] + ' not read')
    debug.debug('OFFICIALCOUNT %s is %s', st, count)
    return count

def _prob_round(curnum):
    import vsrandom
    ret = int(curnum)
    diff = curnum-int(curnum)
    if (diff>0):
        if (vsrandom.uniform (0,1)<diff):
            ret+=1
    else:
        if (vsrandom.uniform (0,1)<-diff):
            ret-=1
    return ret

def LandedShipsInFG(fgname,faction):
    return ShipsInFG(fgname,faction,2)
def ShipsInFG(fgname,faction,offset=1):
    ships = ReadStringList (ccp,MakeFGKey(fgname,faction))
    rez=[]
    for num in xrange (ShipListOffset(),len(ships),PerShipDataSize()):
        rez.append((ships[num],int(ships[num+offset])))
    return rez
def CapshipInFG(fg,fac):
    key = MakeFGKey(fg,fac)
    for num in xrange(ShipListOffset(),Director.getSaveStringLength(ccp,key),PerShipDataSize()):
        import faction_ships
        shipinquestion=Director.getSaveString(ccp,key,num)
        if (faction_ships.isCapital(shipinquestion)):
            return shipinquestion
    return None

def RandomShipIn (fg,fac):
    key = MakeFGKey(fg,fac)
    len = Director.getSaveStringLength(ccp,key)-ShipListOffset()
    len = int(len/PerShipDataSize())
    if (len>0):
        return Director.getSaveString(ccp,key,ShipListOffset()+PerShipDataSize()*vsrandom.randrange(0,len))
    return ''

def minIndex (vals,indices):
    mini = None
    minv = None
    for i in indices:
        if (mini == None) or (minv > vals[i]):
            mini = i
            minv = vals[i]
    return mini or 0

def launchBaseOrbit(type,faction,loc,orbitradius,orbitspeed,unit):
    #orbitradius*=2
    import Vector
    import dynamic_universe
    R = Vector.Vector(vsrandom.uniform(1.25*orbitradius,orbitradius),
                                      vsrandom.uniform(1.25*orbitradius,orbitradius),
                                      vsrandom.uniform(1.25*orbitradius,orbitradius))
    RMag = Vector.Mag(R)
    T = Vector.Vector(vsrandom.uniform(.5*orbitradius,orbitradius),
                                      vsrandom.uniform(.75*orbitradius,.85*orbitradius),
                                      vsrandom.uniform(.5*orbitradius,orbitradius))
    S = Vector.Cross (T,R)
    
    S = Vector.Scale(S,
                                     vsrandom.uniform (1.5*orbitradius,orbitradius)
                                     /Vector.Mag(S))
    SMag = Vector.Mag(S)    
    bas=VS.launch("Base",type,faction,"unit","default",1,1,Vector.Add(loc,R),'')
    nam=GetRandomBaseName (1,faction);
    R = Vector.Scale (R,(RMag+2.0*bas.rSize())/RMag)
    S = Vector.Scale (S,(SMag+2.0*bas.rSize())/SMag)        
    bas.orbit (unit,orbitspeed,R,S,(0.0,0.0,0.0))
    #bas.SetPosition(Vector.Add(loc,R))
    dynamic_universe.TrackLaunchedShip (BaseFGInSystemName(VS.getSystemFile()),
                                                                            faction,
                                                                            type,
                                                                            bas)

def launchSingleBase (type,faction,sig):
    radpct = VS.getPlanetRadiusPercent()
    sigrsize=sig.rSize()
    radpct = sigrsize*(1+radpct)
    if radpct<200000000.+sigrsize:
        radpct=200000000.+sigrsize
    speed = vsrandom.uniform (0,50)
    debug.debug('Lauching %s base %s by sig %s (%s)', faction, type, sig.getName(), sig.getFullname())
    launchBaseOrbit (type,faction,sig.Position(),radpct,speed,sig)

def launchBaseStuck (type,faction):
    un=VS.getPlayer()
    maxspeed=100.1
    if (un):
        maxspeed=un.maxAfterburnerSpeed()+30.1
    un.setNull();
    launchBaseOrbit (type,faction,un.Position(),maxspeed*180,0,un)

def launchBase (type,num,faction,system,sig_units,numfighters):
    import seedrandom
    debug.debug('launching base %s', type)
    seedrandom.seed(seedrandom.seedstring(seedrandom.interleave(['type',
                                                                'faction',
                                                                'system'])))
    if (len(sig_units)):
        for i in xrange (num):
            one=seedrandom.rand()
            two=seedrandom.rand()
            three=seedrandom.rand()
            indices = [one%len(sig_units),
                               two%len(sig_units),
                               three%len(sig_units)];
            which = minIndex(numfighters,indices)
            if (sig_units[which].isJumppoint()):
                numfighters[which]+=20
            else:
                numfighters[which]+=1
            launchSingleBase (type,faction,sig_units[which])
    else:
        for i in xrange(num):
            launchBaseStuck(type,faction)

def zeros (le):
    return [0 for i in xrange(le)]

def launchBases(sys):
    import universe
    print 'Launching bases for '+sys
    fac = VS.GetGalaxyFaction(sys)
    fgs = BaseFG (fac,sys)
    sig_units = universe.significantUnits()
    shipcount=zeros(len(sig_units)) 
    for fg in fgs:
        launchBase(fg[0],fg[1],fac,sys,sig_units,shipcount)

def randDirection():
   leng=2
   while leng>1 or leng<.00001:
      X = vsrandom.uniform(-1,1);
      Y = vsrandom.uniform(-1,1);
      Z = vsrandom.uniform(-1,1);
      leng=X*X+Y*Y+Z*Z
   import VS
   leng=VS.sqrt(leng)
   return (X/leng,Y/leng,Z/leng)

def incr_by_abs(num,val):
   debug.debug("A: %s or %s", num+val, num-val)
   if (num>0):
      return num+val
   return num-val

def FGsLaunchedInCurrentSystemAsSet():
    rvset = {}
    iter = VS.getUnitList()
    while iter.notDone():
        un = iter.next()
        rvset[un.getFlightgroupName()] = 1
    return rvset

def FGsLaunchedInCurrentSystem():
    rvset = FGsLaunchedInCurrentSystemAsSet()
    rv = []
    for fg in rvset:
        rv.append(fg)
    return rv

def filterLaunchedFGs(fglist):
    launched = FGsLaunchedInCurrentSystemAsSet()
    rv = []
    for fg in fglist:
        if not fg in launched:
            rv.append(fg)
    return rv

def launchUnits(sys):
    debug.info("Launching units for %s", sys)
    
    import faction_ships
    import launch_recycle
    import universe
    sig_units=universe.significantUnits()
    ownerfac= VS.GetGalaxyFaction(sys)
    jumpcount=0
    planetcount=0
#    asteroidcount=0
    basecount=0
    farlen=0
    for sig in sig_units:
      if sig.isJumppoint():
         jumpcount+=1
      elif sig.isPlanet():
         planetcount+=1
#      elif sig.isAsteroid():
#         asteroidcount+=1
      else:
         basecount+=1
      tmplen=Vector.Mag(sig.Position())
      if tmplen>farlen:
          farlen=tmplen

    for factionnum in xrange(0,faction_ships.getMaxFactions()-1):
      faction=faction_ships.intToFaction(factionnum)
      fglist=filterLaunchedFGs(FGsInSystem(faction,sys))
      isHostile=VS.GetRelation(ownerfac,faction)<0
      isForeign=faction.find(ownerfac)==-1
      
      if isForeign:
        if basecount+jumpcount:
           frac=len(fglist)/float(basecount+jumpcount)
        else:
           frac=0.0
      else:
        if basecount+planetcount+jumpcount:
           frac=len(fglist)/float(planetcount+basecount+jumpcount)
        else:
           frac=0.0
      if isHostile:
         for flightgroup in fglist:
            X=incr_by_abs(vsrandom.uniform(-1.0,1.0),1)*farlen
            Y=incr_by_abs(vsrandom.uniform(-1.0,1.0),1)*farlen
            Z=incr_by_abs(vsrandom.uniform(-1.0,1.0),1)*farlen
            XYZ = (X,Y,Z)
            typenumbers=ShipsInFG(flightgroup,faction)
            debug.debug("Really Far Apart around   %s and 10000",XYZ)
            debug.debug(" launching %s for %s at %s", typenumbers, faction, XYZ)
            launch_recycle.launch_types_around(flightgroup,faction,typenumbers,'default',1,XYZ,0,'','',1,10000)
      else:
         for flightgroup in fglist:
            #jp = sig.isJumppoint()
            #if sig.isPlanet() or not isForeign:
            sig = sig_units[vsrandom.randrange(0,len(sig_units))]
            typenumbers=ShipsInFG(flightgroup,faction)
            debug.debug(" launching %s for %s", typenumbers, faction)

            launch_recycle.launch_types_around(flightgroup,faction,typenumbers,'default',sig.rSize()*vsrandom.randrange(10,100),sig,0,'','',1,10000)



def DefaultNumShips():
    import vsrandom
    diff=VS.GetDifficulty()
    if (diff>.9):
       return vsrandom.randrange(1,5)
    if (diff>.5):
       return vsrandom.randrange(1,4)
    if (diff>.2):
       return vsrandom.randrange(1,3)
    if (vsrandom.randrange(0,4)==0):
       return 2
    return 1

def GetShipsInFG(fgname,faction):
    ships = ReadStringList (ccp,MakeFGKey(fgname,faction))
    if (len(ships)<=ShipListOffset()):
        return []
    try:
        count=int(ships[0])
    except:
        debug.error('bad flightgroup record %s', ships)
    launchnum = DefaultNumShips()
    if (launchnum>count):
        launchnum=count
	
    nent = (len(ships) - ShipListOffset()) / PerShipDataSize()
    retn = [0] * nent
    for i in xrange(_prob_round(launchnum*(0.7+vsrandom.random()+0.3))):
	which = vsrandom.randrange(count)
	for j in xrange(nent):
	    pos = j*PerShipDataSize()+ShipListOffset()
	    which -= int(ships[pos+2])
	    if which <= 0:
		retn[j] += 1
		break
    ret = []
    for i in xrange(nent):
	if retn[i]:
	    pos = i*PerShipDataSize()+ShipListOffset()
	    ret.append((ships[pos],retn[i]))
    return ret

def LaunchLandShip(fgname,faction,typ,numlaunched=1):
    key = MakeFGKey (fgname,faction)
    ships=ReadStringList (ccp,key)
    debug.debug('LaunchLandShip: fg:%s fac:%s typ:%s, num:%s', fgname, faction, typ, numlaunched)
    for num in xrange (ShipListOffset(),len(ships),PerShipDataSize()):
        if (typ == ships[num]):
            try:
                ntobegin=int(ships[num+1])
                nactive=int(ships[num+2])
                
                debug.debug("attempting launch for ship %s, begin %s, act %s)",
                    typ, ntobegin, nactive)
                
                nactive-=numlaunched
                # Happens regularly -Patrick
                # In the first system, nactive seems to always be 0 for all ships.
                # In other systems, this isn't always true.
                # This doesn't really seem to matter, though.
                # Klauss: turned debug.error into debug.debug if it happens so often
                #   to clean up non-debug logs
                if (nactive<0):
                    nactive=0
                    debug.debug('error more ships launched than in FG %s', fgname)
                if (nactive>ntobegin):
                    nactive=ntobegin
                    debug.debug('error ships %s landed that never launched', typ)
                Director.putSaveString(ccp,key,num+2,str(nactive))
            except:
                debug.error('error in FG data (str->int)')

def LaunchShip (fgname,faction,typ,num=1):
    LaunchLandShip (fgname,faction,typ,num)

def LandShip (fgname,faction,typ,num=1):
    LaunchLandShip(fgname,faction,typ,-num)

def AllShips (faction,offset=1):
    ret=[]
    for i in AllFlightgroups (faction):
        ret+=ShipsInFG (i,faction,offset)
    return ret

def CheckAllShips(faction):
    for i in AllFlightgroups(faction):
        sys = FGSystem(i,faction)
        fgsin=AllFGsInSystem(faction,sys)
        if (not i in fgsin):
            debug.error('error '+str(fgsin) + i+' not in system '+ sys)

def SortedAllShips (faction,offset=1):
    ret={}
    for i in AllFlightgroups (faction):
        for j in ShipsInFG(i,faction,offset):
            if j[0] in ret:
                ret[j[0]]+=j[1]
            else:
                ret[j[0]]=j[1]
    return ret

def getFgLeaderType(fgname,faction):
#       print "wah " +str(ShipsInFG(fgname,faction))
    l = ShipsInFG(fgname,faction)
    if (len(l)):
        if (len(l[0])):
            return l[0][0];
    import faction_ships
    return faction_ships.getRandomFighter(faction)
