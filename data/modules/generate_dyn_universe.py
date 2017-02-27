# -*- coding: utf-8 -*-
import VS
import Director
import vsrandom
import fg_util
import dj_lib # Needs to be enabled when loading.
import campaigns
from universe import getAdjacentSystemList,AllSystems
import debug
import ShowProgress

cp=fg_util.ccp
maxshipsinfg=20
fgnames=[] #list of lists of flightgroup names for each faction
fgoffset=[] #list of tweak offsets
origfgnames=[]
import faction_ships
def XProductionRate(fac,type):
    if fac in type:
        return type[fac]
    return type["default"]
def floatToRand(maximum):
   if maximum==0:
     return 0
   tmp=vsrandom.randrange(0,int(maximum)+1)
   if (vsrandom.uniform(0,1)<maximum-int(maximum)):
     return tmp+1
   return tmp
def GenerateCivilianFgShips (faction,factionnr,docapships):
    lst=[]
    capitals=faction_ships.capitals[factionnr]
    fighters=faction_ships.fighters[factionnr]   
    numfighters=1
    try:
      numfighters=faction_ships.fightersPerFG["default"]
      numfighters=faction_ships.fightersPerFG[faction]
    except:
      pass
    numcapitals=0
    if docapships:
      try:
        numcapitals=faction_ships.fightersPerFG["default"]
        numcapitals=faction_ships.PerFG[faction]
      except:
        pass
    fighters_per=len(fighters)
    ht={}
    for i in fighters:
       numtogen=floatToRand(numfighters/float(len(fighters)))
       if numtogen:
         if i in ht:
           ht[i]+=numtogen
         else:
           ht[i]=numtogen
    for i in capitals:
       numtogen=floatToRand(numcapitals/float(len(capitals)))
       if numtogen>0:
         if i in ht:
           ht[i]+=numtogen
         else:
           ht[i]=numtogen
    for hv in ht:
        lst.append([hv,ht[hv]])
    return lst
def GenerateFgShips (shipinfg,factionnr,friendly):
    lst=[]
    capship=()
    fac = faction_ships.intToFaction(factionnr)
    fpr=XProductionRate(fac,faction_ships.fighterProductionRate)
    cpr=XProductionRate(fac,faction_ships.capitalProductionRate)
    if cpr>0 and (friendly==2 or (friendly==1 and vsrandom.random()<cpr/fpr)):
        capship=((faction_ships.getRandomCapitolInt(factionnr),1),)
        debug.debug("Generating capital "+str(capship))
    return ((faction_ships.getRandomFighterInt(factionnr),shipinfg),)+capship

def GenerateAllShips ():
    global fgnames,origfgnames,fgoffset
    for fnr in xrange(faction_ships.getMaxFactions()):
        fgnames.append(fg_util.GetRandomFGNames(-1,faction_ships.factions[fnr]))
        fgoffset.append(0)
        origfgnames.append(list(fgnames[-1]))


doNotAddBasesTo={"enigma_sector/heavens_gate":1,"sol_sector/celeste":1,"enigma_sector/enigma":1,"enigma_sector/niven":1,"Gemini":1,"Crucible/Cephid_17":1}
def AddBasesToSystem (faction,sys):
    if (sys in doNotAddBasesTo):
        return
    slash = sys.find("/")
    if (slash!= -1):
        if (sys[0:slash] in doNotAddBasesTo):
            return
    if faction in faction_ships.factions:
        fsfac= list(faction_ships.factions).index(faction)
        numbases=0
#               numplanets=VS.GetGalaxyProperty(sys,"num_planets");
        numjumppoints=VS.GetNumAdjacentSystems(sys);
        if (numjumppoints<4):
            if (vsrandom.random()>=.25):
                numbases=1
        elif (vsrandom.random()>=.005):
            if (numjumppoints<7):
                numbases=vsrandom.randrange(1,int(numjumppoints/2)+1)
            elif numjumppoints==7:
                numbases=vsrandom.randrange(1,6)
            else:
                numbases=vsrandom.randrange(1,numjumppoints+1)
        if numbases==0:
            return
        shiplist=[]
        nums=[]
        for i in xrange(numbases):
            whichbase = faction_ships.bases[fsfac][vsrandom.randrange(0,len(faction_ships.bases[fsfac]))]
            if whichbase in shiplist:
                nums[shiplist.index(whichbase)]+=1
            else:
                shiplist.append(whichbase)
                nums.append(1)
        tn =[]
        for i in xrange(len(shiplist)):
            tn+=[ (shiplist[i],nums[i])]
        fg_util.AddShipsToFG(fg_util.BaseFGInSystemName (sys),faction,tn,sys)

numericalfaction=0

def GetNewFGName(faction):
    factionnr=faction_ships.factionToInt(faction)
    global numericalfaction,fgnames,origfgnames,fgoffset
    if(factionnr>=len(fgnames)):
        debug.error("Faction "+faction+" unable to create fgname")

        numericalfaction+=1
        return "Alpha_"+str(numericalfaction)
    if (not len(fgnames[factionnr])):
        debug.debug("Loading FG names for %s" % faction)
        fg_util.origoffset=fgoffset[factionnr]
        fgnames[factionnr]=fg_util.TweakFGNames(origfgnames[factionnr])
        fgoffset[factionnr]+=1
    k=vsrandom.randrange(0,len(fgnames[factionnr])); #pop returns item inside array
    fgname=fgnames[factionnr][k]
    del fgnames[factionnr][k]
    return fgname
    
_generatedsys = 0
_last_progress_percent = -1
def AddSysDict (cursys):
    global _generatedsys, _last_progress_percent
    #pick random fighter from insysenemies with .3 probability OR pick one from the friendlies list.
#       debug.debug('Addsysdict')
    sysfaction=VS.GetGalaxyFaction(cursys)

    numflightgroups = 1+vsrandom.randrange(fg_util.MinNumFlightgroupsInSystem(cursys)-1,fg_util.MaxNumFlightgroupsInSystem(cursys))
    
    #debug.debug("Initializing system %s with %d flightgroups... " % (cursys,numflightgroups))
    progress_percent = (float(_generatedsys) / getSystemCount())
    if progress_percent - _last_progress_percent > 0.01:
        ShowProgress.setProgressBar("loading",progress_percent)
        ShowProgress.setProgressMessage("loading","Generating dynamic universe (%.2f%%)" % (100*progress_percent))
        _last_progress_percent = progress_percent
    _generatedsys += 1

    global fgnames, fglists
    i=0
    AddBasesToSystem(sysfaction, cursys)
    adjsystems=getAdjacentSystemList(cursys)
    for i in xrange(len(faction_ships.factions)):
       thisfac=faction_ships.factions[i]
       thisfactionnr=faction_ships.factionToInt(thisfac)
       rel=VS.GetRelation(sysfaction,thisfac)
       iscit=VS.isCitizen(thisfac)
       if iscit and thisfac.find(sysfaction)!=-1:
          #debug.debug("generating spc civilian for "+cursys+" faction "+thisfac)
          #do somethign special for this military faction
          if thisfac==sysfaction:
              pass
          elif iscit:
              for neighbor in adjsystems:
                 if (VS.GetRelation(thisfac,VS.GetGalaxyFaction(neighbor))>-.05):
                   #fgname=cursys+"->"+neighbor
                   fgname="(CivReg) "+GetNewFGName(thisfac)
                   typenumbertuple=GenerateCivilianFgShips(thisfac,thisfactionnr,True)
                   fg_util.AddShipsToFG (fgname,thisfac,typenumbertuple,cursys)
              fgname="Insystem_"+cursys;
              typenumbertuple=GenerateCivilianFgShips(thisfac,thisfactionnr,False)
       if iscit:
          #debug.debug("generating gen civilian for "+cursys+" faction "+thisfac)
          if VS.GetRelation(thisfac,sysfaction)>-.05:#brave citizens of the new order... 
             fgname=GetNewFGName(thisfac)
             typenumbertuple=GenerateCivilianFgShips(thisfac,thisfactionnr,True)
             fg_util.AddShipsToFG (fgname,thisfac,typenumbertuple,cursys)
    friendlychance=.7
    if cursys in faction_ships.fortress_systems:
        friendlychance=faction_ships.fortress_systems[cursys]
        #debug.debug('enemy chance for '+cursys +'= '+str(friendlychance))
    for i in xrange (numflightgroups): #number of fgs in a system.
        faction=sysfaction
        friendly=0
        if not sysfaction or sysfaction=='unknown' or vsrandom.random()>friendlychance:
            faction=faction_ships.get_rabble_of_no_citizen(sysfaction)#why even have citizens on the list then
        else:            
            faction=faction_ships.get_friend_of_no_citizen(sysfaction)#likewise--- maybe this should be a faction_ships fix
            if (faction==sysfaction):
                friendly=1
            if (sysfaction in faction_ships.production_centers):
                if (cursys in faction_ships.production_centers[sysfaction]):
                    friendly=2
            #if (friendly):
            #    debug.debug(faction+" "+sysfaction+" "+cursys)
        factionnr=faction_ships.factionToInt(faction)
        global maxshipsinfg
        typenumbertuple=GenerateFgShips(vsrandom.randrange(maxshipsinfg)+1,factionnr,friendly)
        fgname=GetNewFGName(faction)
        fg_util.AddShipsToFG (fgname,faction,typenumbertuple,cursys)
    return i

def ForEachSys (functio):
    debug.debug("Getting reachable systems...")
    systems = AllSystems()
    debug.debug("done")
    for sys in systems:
	functio(sys)
    return len(systems)
def MakeUniverse():
    fg_util.DeleteAllFGFromAllSystems()
    ForEachSys(AddSysDict)

systemcount={None:0}
def getSystemCount(fac=None):
    global systemcount
    return systemcount[fac]
def CountSystems(sys):
    fac =VS.GetGalaxyFaction(sys)
    if fac in systemcount:
        systemcount[fac]+=1
    else:
        systemcount[fac]=1
    systemcount[None] += 1
def TakeoverSystem(fac,sys):
    systemcount[VS.GetGalaxyFaction(sys)]-=1
    VS.SetGalaxyFaction(sys,fac)
    systemcount[fac]+=1
    AddBasesToSystem(fac,sys)

hasUniverse=False

genUniverse=-1
def ReloadUniverse():
  global genUniverse, hasUniverse
  if cp>=0:
    ShowProgress.activateProgressScreen("loading",force=True)
    debug.debug('Purging...')
    for i in fg_util.AllFactions():
        fg_util.PurgeZeroShips(i)
        systemcount[i]=0
    debug.debug('StartSystemCount')
    ForEachSys(CountSystems)
    debug.debug(systemcount)
    debug.debug('EndSystemCount')
    genUniverse=0
    needNewUniverse=0
    curfaclist = fg_util.AllFactions()
    reflist = fg_util.ReadStringList(cp,"FactionRefList")
    
    if (reflist !=curfaclist):
        needNewUniverse = 1
        debug.debug('reflist is '+str(reflist))
        debug.debug('curfaclist is '+str(curfaclist))

    if (fg_util.HasLegacyFGFormat()):
        needNewUniverse = 1
        debug.warn('save using legacy FG format... resetting universe to reformat')
        fg_util.DeleteLegacyFGLeftovers()

    if needNewUniverse:
        fg_util.WriteStringList(cp,"FactionRefList",curfaclist)
        debug.debug('generating ships... ... ...')
        GenerateAllShips() ###Insert number of flight groups and max ships per fg
        debug.debug('placing ships... ... ...')
        genUniverse=MakeUniverse()
        #now every system has distributed ships in the save data!
    else:
        GenerateAllShips()
        debug.debug("Second Load")
        for i in xrange(len(fgnames)):
            fg_util.origfgoffset=fgoffset[i]
            fgnames[i]=fg_util.TweakFGNames(origfgnames[i])
            fgoffset[i]+=1
    campaigns.loadAll(cp)
    hasUniverse=True;
    #TODO: add ships to current system (for both modes)  uru?
    ShowProgress.deactivateProgressScreen('loading')
  else:
    debug.error('fatal error: no cockpit')

def KeepUniverseGenerated():
    if VS.networked():
        dj_lib.enable()
        #debug.debug('Not generating dyn universe: Networked game')
        return False
    
    sys = VS.getSystemFile()
    if not VS.GetNumAdjacentSystems(sys):
        #debug.debug('Not generating dyn universe: System has no jumps or is not in Universe XML.')
        return False
    
    dj_lib.enable()
    #curfaclist = fg_util.AllFactions()
    #reflist = fg_util.ReadStringList(cp,"FactionRefList")
    #if (reflist == curfaclist):
    #    debug.debug('Generating dyn universe!');
    if not hasUniverse:
        ReloadUniverse()
        return True
    return False
