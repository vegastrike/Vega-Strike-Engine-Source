#include "savegame.h"
#include "fg_util.h"
#include "faction_generic.h"
#include <algorithm>
namespace fg_util {
  void itoa (unsigned int dat, char * output) {
    if (dat==0) {
      *output++='0';
      *output='\0';
    }else {
      char *s=output;
      while (dat){
        *s++='0'+dat%10;
        dat/=10;
      }
      *s='\0';
      std::reverse(output,s);
    }
  }
std::string MakeFactionKey (int faction) {
  char output[16];
  output[0]='F';
  output[1]='F';
  output[2]=':';
  itoa(faction,output+3);
  return std::string(output);
}
  bool IsFGKey(const std::string&fgcandidate) {
    if (fgcandidate.length()>3&&fgcandidate[0]=='F'&&fgcandidate[1]=='G'&&fgcandidate[2]==':') return true;
    return false;
  }

static std::string gFG="FG:";
std::string MakeFGKey (const std::string &fgname,int faction){
  char tmp[16];
  tmp[0]='|';
  itoa(faction,tmp+1);
  return gFG+fgname+tmp;
}
static std::string gSS="SS:";
std::string MakeStarSystemFGKey (const std::string &starsystem) {
  return gSS+starsystem;
}
unsigned int ShipListOffset () {
  return 3;
}
unsigned int PerShipDataSize () {
  return 3;
}
/*
basenamelist={}
flightgroupnamelist={}
genericalphabet=['Alpha','Beta','Gamma','Delta','Epsilon','Zeta','Phi','Omega']
def ReadBaseNameList(faction):
    bnl=[]
    debug.debug('reading base names '+str(faction))
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
    for i in range(len(bnl)):
        bnl[i]=bnl[i].rstrip()#.decode('utf8','ignore')
    import vsrandom
    vsrandom.shuffle(bnl)
    return bnl
def GetRandomFGNames (numflightgroups, faction):
    global flightgroupnamelist
    if (not (faction in flightgroupnamelist)):
        flightgroupnamelist[faction]=ReadBaseNameList(faction)
    additional=[]
    if (numflightgroups>len(flightgroupnamelist[faction])):
        for i in range (numflightgroups-len(flightgroupnamelist)):
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
        for i in range (n):
            retval+=[str(n)]
            n+=1
    return retval
origfgoffset=0
def TweakFGNames (origfgnames):
    global origfgoffset
    tweaker=str(origfgoffset)
    tweaktuple = ('Squadron','Prime','Arc','Alpha','Aleph','Beta','Quadratis','Zeta','X','Plus','Blade','Delta','Dash','Xprime','Gamma','Hydris','Dual','Tri','Quad','Penta','Hex','Octo','Deca','Octate')
    if (origfgoffset<len(tweaktuple)):
        tweaker = tweaktuple[origfgoffset]
    rez=[]
    for i in origfgnames:
        rez.append (i+'_'+tweaker)
    return rez
            
def WriteStringList(cp,key,tup):
    siz = Director.getSaveStringLength (cp,key)
    s_size=siz;
    lentup= len(tup)
    if (lentup<siz):
        siz=lentup
    for i in range(siz):
        Director.putSaveString(cp,key,i,tup[i])
    for i in range (s_size,lentup):
        Director.pushSaveString(cp,key,tup[i])
    for i in range (lentup,s_size):
        Director.eraseSaveString(cp,key,lentup)
def ReadStringList (cp,key):
    siz = Director.getSaveStringLength (cp,key)
    tup =[]
    for i in range (siz):
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
    for i in range (1,len(tup)):
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

def _AddShipToKnownFG(key,tn):
    leg = Director.getSaveStringLength (ccp,key)
    #try:
    numtotships =int(Director.getSaveString(ccp,key,0))
    numtotships+=int(tn[1])
    Director.putSaveString(ccp,key,0,str(numtotships))
    #except:
    #       print 'error adding ship to flightgroup'
    for i in range (ShipListOffset()+1,leg,PerShipDataSize()):
        if (Director.getSaveString(ccp,key,i-1)==str(tn[0])):
            numships=0
            numactiveships=0
            try:
                numships+= int(tn[1])
                numactiveships+= int(tn[1])
                numships+= int (Director.getSaveString(ccp,key,i))
                numactiveships+= int (Director.getSaveString(ccp,key,i+1))
            except:
                pass
            Director.putSaveString(ccp,key,i,str(numships))
            Director.putSaveString(ccp,key,i+1,str(numactiveships))
            return
    Director.pushSaveString(ccp,key,str(tn[0]))
    Director.pushSaveString(ccp,key,str(tn[1]))
    Director.pushSaveString(ccp,key,str(tn[1]))#add to number active ships

def _AddFGToSystem (fgname,faction,starsystem):
    key = MakeStarSystemFGKey (starsystem)
    leg = Director.getSaveStringLength (ccp,key)
    index = VS.GetFactionIndex (faction)
    if (leg>index):
        st=Director.getSaveString (ccp,key,index)
        if (len(st)>0):
            st+='|'
        try:
            test=st+fgname
        except:
            fgname=fgname.decode('utf8','ignore')
        Director.putSaveString(ccp,key,index,st+fgname)
    else:
        for i in range (leg,index):
            Director.pushSaveString(ccp,key,'')
        Director.pushSaveString(ccp,key,fgname)


def _RemoveFGFromSystem (fgname,faction,starsystem):
    key = MakeStarSystemFGKey( starsystem)
    leg = Director.getSaveStringLength(ccp,key)
    index = VS.GetFactionIndex(faction)
    if (leg>index):
        tup = Director.getSaveString (ccp,key,index).split('|')
        try:
            del tup[tup.index(fgname)]
            Director.putSaveString(ccp,key,index,ListToPipe(tup))                   
        except:
            debug.error('fg '+fgname+' not found in '+starsystem)
    else:
        debug.debug('no ships of faction '+faction+' in '+starsystem)

def _AddFGToFactionList(fgname,faction):
    key = MakeFactionKey(faction)
    Director.pushSaveString (ccp,key,fgname)
            
def _RemoveFGFromFactionList (fgname,faction):
    key = MakeFactionKey(faction)
    lun=Director.getSaveStringLength(ccp,key)
    for i in range (lun):
        if (Director.getSaveString(ccp,key,i)==fgname):
            Director.eraseSaveString(ccp,key,i)
            return 1
    return 0
*/
  bool CheckFG(std::vector<StringPool::Reference> &data) {
    bool retval=false;
    unsigned int leg = data.size();
    unsigned int totalships=0;
    unsigned int inc=PerShipDataSize();
    for (unsigned int i=ShipListOffset()+1;i+1<leg;i+=inc) {
      StringPool::Reference *numlanded=&data[i+1];
      StringPool::Reference*numtotal=&data[i];
      if (*numlanded!=*numtotal) {
        retval=true;
        *numlanded=*numtotal;
      }
    }
    return retval;
  }
bool CheckFG (SaveGame *sg,const std::string& fgname,unsigned int faction) {
  std::string key = MakeFGKey (fgname,faction);
  //sg=_Universe->AccessCockpit()->savegame
  return CheckFG(sg->getMissionStringData(key));
}
void PurgeZeroShips (SaveGame*sg,unsigned int faction) {
  std::string key=MakeFactionKey(faction);
  unsigned int len=sg->getMissionStringDataLength(key);
  unsigned int i=0;
  while (i<len) {
    CheckFG (sg,sg->getMissionStringData(key)[i]/*flightgroup*/,faction);
    i+=1;
  }
}
void PurgeZeroShips(SaveGame*sg) {
  for (unsigned int i=0;i<factions.size();++i) {
    fg_util::PurgeZeroShips(sg,i);
  }
}
             /*
def NumShipsInFG (fgname,faction):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len==0):
        return 0
    else:
        try:
            return int(Director.getSaveString(ccp,key,0))
        except:
            #print 'fatal: flightgroup without size'
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
            #print 'nonfatal: flightgroup without size'
            return 0

def SetDamageInFGPool (fgname,faction,num):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len>2):
        Director.putSaveString(ccp,key,2,str(num))                                 


def DeleteFG(fgname,faction):
    key = MakeFGKey (fgname,faction)
    len = Director.getSaveStringLength (ccp,key)
    if (len>=ShipListOffset()):
        starsystem=Director.getSaveString(ccp,key,1)
        _RemoveFGFromSystem(fgname,faction,starsystem)
        _RemoveFGFromFactionList(fgname,faction)
        WriteStringList (ccp,MakeFGKey(fgname,faction),[] )
def DeleteAllFG (faction):
    for fgname in ReadStringList (ccp,MakeFactionKey (faction)):
        DeleteFG (fgname,faction)
def FGSystem (fgname,faction):
    key = MakeFGKey(fgname,faction)
    len = Director.getSaveStringLength(ccp,key)
    if (len>1):
        return Director.getSaveString(ccp,key,1)
    else:
        #print fgname+' for '+faction+' already died, in no system'
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
        WriteStringList(ccp,key,_MakeFGString( starsystem,typenumbertuple) )
        _AddFGToSystem (fgname,faction,starsystem)
        _AddFGToFactionList (fgname,faction)
        debug.info('adding new fg '+fgname+" of "+str(typenumbertuple)+" to "+starsystem)
    else:
        debug.info('adding old fg '+fgname+" of "+str(typenumbertuple)+" to "+FGSystem(fgname,faction))
        for tn in typenumbertuple:
            _AddShipToKnownFG(key,tn)

def RemoveShipFromFG (fgname,faction,type,numkill=1,landed=0):
    key = MakeFGKey (fgname,faction)
    leg = Director.getSaveStringLength (ccp,key)
    for i in range (ShipListOffset()+1,leg,PerShipDataSize()):
        if (Director.getSaveString(ccp,key,i-1)==str(type)):
            numships=0
            numlandedships=0
            try:
                numships = int (Director.getSaveString (ccp,key,i))
                numlandedships=int (Director.getSaveString (ccp,key,i+1))
            except:
                debug.error("unable to get savestring "+i+" from FG "+fgname +" "+faction+ " "+type)
            if (numships>numkill):
                numships-=numkill
                if (numships<numlandedships):
                    if (landed==0):
                       debug.debug('trying to remove launched ship '+type+' but all are landed')
                       landed=1
                       return 0#failur
                Director.putSaveString (ccp,key,i,str(numships))
                if (landed and numlandedships>0):
                    Director.putSaveString(ccp,key,i+1,str(numlandedships-numkill))
            else:
                numkill=numships
                numships=0
                for j in range (i-1,i+PerShipDataSize()-1):
                    Director.eraseSaveString(ccp,key,i-1)
            if (numships>=0):
                try:
                    totalnumships = int(Director.getSaveString(ccp,key,0))
                    totalnumships -=numkill
                    if (totalnumships>=0):
                        Director.putSaveString(ccp,key,0,str(totalnumships))
                        if(totalnumships==0):
                            DeleteFG(fgname,faction)
                    else:
                        debug.error('error...removing too many ships')
                except:
                    debug.error('error, flight record '+fgname+' corrupt')
            return numkill
    debug.debug('cannot find ship to delete in '+faction+' fg ' + fgname)
    return 0
def BaseFGInSystemName (system):
    return 'Base_'+system
def AllFGsInSystem(faction,system):
    key = MakeStarSystemFGKey (system)
    leg = Director.getSaveStringLength (ccp,key)
#       if 1:#(not (Director.dontdoprint)):
#               print faction
    facnum = VS.GetFactionIndex (faction)
    #print "faction name "+faction+" facnum "+str(facnum)
    ret=[]
    if (leg>facnum):
        st=Director.getSaveString(ccp,key,facnum)
        if (len(st)>0):
            ret = st.split('|')
    return ret

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
    key = MakeStarSystemFGKey (system)
    leg = Director.getSaveStringLength (ccp,key)
    facnum = VS.GetFactionIndex (faction)
    st=''
    if (leg>facnum):
        st=Director.getSaveString(ccp,key,facnum)
    if (st):
        return st.count('|')+1
    return 0

def CountFactionShipsInSystem(faction,system):
    count=0
    st=''
    for fgs in FGsInSystem (faction,system):
        st+=fgs+' '
        ships=ReadStringList (ccp,MakeFGKey (fgs,faction))
        for num in range(ShipListOffset()+2,len(ships),PerShipDataSize()):
            try:
                count+= int(ships[num])
            except:
                debug.debug('number ships '+ships[num] + ' not read')
    print 'OFFICIALCOUNT '+st
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
    for num in range (ShipListOffset(),len(ships),PerShipDataSize()):
        rez.append((ships[num],int(ships[num+offset])))
    return rez
def CapshipInFG(fg,fac):
    key = MakeFGKey(fg,fac)
    for num in range(ShipListOffset(),Director.getSaveStringLength(ccp,key),PerShipDataSize()):
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
    debug.debug('Lauching '+str(faction)+' base '+str(type)+' by sig '+sig.getName()+' ('+sig.getFullname()+')')
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
    debug.debug('launching base '+ type)
    seedrandom.seed(seedrandom.seedstring(seedrandom.interleave(['type',
                                                                                                                             'faction',
                                                                                                                             'system'])))
    if (len(sig_units)):
        for i in range (num):
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
        for i in range(num):
            launchBaseStuck(type,faction)
def zeros (le):
    return [0 for i in range(le)]
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
   #print "A: "+str(num+val)+" or "+str(num-val)
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
    print "Launching units for "+sys
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

    for factionnum in range(0,faction_ships.getMaxFactions()-1):
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
            typenumbers=ShipsInFG(flightgroup,faction)
            #print "Really Far Apart around "+"  "+str(X)+"  "+str(Y)+"  "+str(Z)+" and 10000"
            debug.debug(" launching "+str(typenumbers) +" for "+faction+" at "+str((X,Y,Z)))
            launch_recycle.launch_types_around(flightgroup,faction,typenumbers,'default',1,(X,Y,Z),0,'','',1,10000)
      else:
         for flightgroup in fglist:
            #jp = sig.isJumppoint()
            #if sig.isPlanet() or not isForeign:
            sig = sig_units[vsrandom.randrange(0,len(sig_units))]
            typenumbers=ShipsInFG(flightgroup,faction)
            debug.debug(" XXlaunching "+str(typenumbers) +" for "+faction)

            print "LALALAUNCHING "+str(typenumbers)+" t "+sig.getName()+":"+sig.getFullname()
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
        debug.error('bad flightgroup record '+ships)
    launchnum = DefaultNumShips()
    if (launchnum>count):
        launchnum=count
    ret = []
    for num in range(ShipListOffset(),len(ships),PerShipDataSize()):
        curnum=int(ships[num+2])
        cnum = _prob_round(curnum*float(launchnum)/count)
        if (cnum>0):
            ret+=[(ships[num],cnum)]
    return ret

def LaunchLandShip(fgname,faction,typ,numlaunched=1):
    key = MakeFGKey (fgname,faction)
    ships=ReadStringList (ccp,key)
    debug.debug('LaunchLandShip: '+str((fgname, faction, typ, numlaunched)))
    for num in range (ShipListOffset(),len(ships),PerShipDataSize()):
        if (typ == ships[num]):
            try:
                debug.debug("attempting launch for ship"+str(typ)+', begin '+
                            str(ships[num+1])+', act '+str(ships[num+2]))
                ntobegin=int(ships[num+1])
                nactive=int(ships[num+2])
                nactive-=numlaunched
		# Happens regularly -Patrick
		# In the first system, nactive seems to always be 0 for all ships.
		# In other systems, this isn't always true.
		# This doesn't really seem to matter, though.
                if (nactive<0):
                    nactive=0
                    debug.debug('error more ships launched than in FG '+fgname)
                if (nactive>ntobegin):
                    nactive=ntobegin
                    debug.debug('error ships '+typ+'landed that never launched')
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
             */
}
