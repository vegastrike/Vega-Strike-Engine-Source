##import ai_flyto_jumppoint
##current_system=""
##last_system=""
##old_system=""
##system_map={}
##outstr=""

import VS
import vsrandom
import faction_ships
import Director
import launch
import debug

def catInCatList (cat,catlist):
    for i in catlist:
        loc =cat.find (i)
        if (loc==0):
            return 1
    return 0

def adjustUnitCargo(un,cat,pr,qr):
    numcargo = un.numCargo()
    carglist =[]
    for i in range(numcargo):
        carg = un.GetCargoIndex(i)
        if (len(cat)==0 or catInCatList(carg.GetCategory(),cat)):
            carg.SetPrice (pr*carg.GetPrice())
            carg.SetQuantity (int(qr*carg.GetQuantity()))
        carglist += [carg]
    for carg in carglist:
        un.removeCargo (carg.GetCategory(),carg.GetQuantity(),1)
    for carg in carglist:
        un.addCargo(carg)
    carglist=0

#universe.systemCargoDemand (("Natural_Products","starships",),.0001,1000)
def systemCargoDemand (category,priceratio,quantratio,ships=1,planets=1):
    i = VS.getUnitList()
    un = i.current()
    while (not un.isNull()):
        if (un.isPlayerStarship()==-1):
            isplanet = un.isPlanet()
            if ( (isplanet and planets) or ((not isplanet) and ships)):
                adjustUnitCargo(un,category,priceratio,quantratio)
        i.advance()
        un=i.current()

def setFirstSaveData(player,key,val):
    mylen = Director.getSaveDataLength(player,key)
    if (mylen>0):
        Director.putSaveData(player,key,0,val)
    else:
        Director.pushSaveData(player,key,val)

def getAdjacentSystems (currentsystem, sysaway, jumps=(),preferredfaction=''):
    """returns a tuple in the format ("[lastsystem]",(system1,system2,system3,...))"""
    if preferredfaction=='':
        preferredfaction=VS.GetGalaxyProperty(currentsystem,"faction")
    max=VS.GetNumAdjacentSystems(currentsystem)
    if ((sysaway<=0) or (max<=0)):
#      _io.message (1,"game","all","Your final destination is %s" % (currentsystem))
        return (currentsystem,jumps)
    else:
        syslist=[]
        numadj=VS.GetNumAdjacentSystems(currentsystem)
        for i in range(numadj):
            cursys=VS.GetAdjacentSystem(currentsystem,i)
            if preferredfaction!=None:
                if VS.GetGalaxyProperty(cursys,"faction")!=preferredfaction:
                    continue
            if ((cursys in jumps) or (cursys == VS.getSystemFile())):
                continue
            syslist.append(cursys)
        if not len(syslist):
            return getAdjacentSystems(currentsystem,0,jumps)
        nextsystem=syslist[vsrandom.randrange(0,len(syslist))]
#      _io.message (1,"game","all","Jump from %s to %s." % (currentsystem,nextsystem))
        return getAdjacentSystems(nextsystem,sysaway-1,jumps+(nextsystem,))

#def getAdjacentSystems (currentsystem, num_systems_away):
#    return nearsys (currentsystem,num_systems_away,())

def getAdjacentSystemList (tothissystem):
    list=[]
    max=VS.GetNumAdjacentSystems(tothissystem);
    for i in range(max):
        list.append(VS.GetAdjacentSystem(tothissystem,i))
    return list

def getRandomJumppoint():
    jp_list=getJumppointList()
    size=len(jp_list)
    if (size>0):
        return jp_list[vsrandom.randrange(0,size)]
    else:
        return VS.Unit()

def getJumppointList():
    jp_list=[]
    i = VS.getUnitList()
    i.advanceNJumppoint(0)
    while i.notDone():
        jp_list.append(i.current())
        i.advanceJumppoint()
    return jp_list

def getMessagePlayer(un):
    num=un.isPlayerStarship()
    if (num<0):
        return "all"
    else:
        return "p"+str(num)

def punish (you,faction,difficulty):
    VS.AdjustRelation(you.getFactionName(),faction,difficulty*-.01,1)
    if (difficulty>=2):
        VS.IOmessage (0,"mission",getMessagePlayer(you),"#ff0000Your idiocy will be punished.")
        VS.IOmessage (0,"mission",getMessagePlayer(you),"#ff0000You had better run for what little life you have left.")
        for i in range(difficulty):
            un=faction_ships.getRandomFighter(faction)
            newunit=launch.launch_wave_around_unit("shadow", faction, un, "default", 1, 200.0,400.0,you)
            newunit.setFgDirective("B")
            newunit.SetTarget(you)

def _tmpint(str,default):
    try:
        return int(str)
    except:
        return default

def significantUnits():
    ret=[]
    iter= VS.getUnitList()
    iter.advanceNSignificant(0)
    while (iter.notDone()):
	un = iter.current()
	debug.debug('Found sig unit: '+un.getName()+' ('+un.getFullname()+')')
	if not un.isSignificant():
		debug.error('Unit '+un.getName()+' ('+un.getFullname()+') is not significant!')
        ret.append(iter.current())
        iter.advanceSignificant()
    return ret

def GetNumSignificantsForSystem (cursys):
    numjmp=VS.GetNumAdjacentSystems(cursys)
    return _tmpint(VS.GetGalaxyProperty(cursys,"num_planets"),3)+_tmpint(VS.GetGalaxyProperty(cursys,"num_moons"),4)+_tmpint(VS.GetGalaxyProperty(cursys,"num_gas_giants"),2)+_tmpint(VS.GetGalaxyProperty(cursys,"num_starbases"),1)+numjmp
#use go_somewhere_significant instead:
##def __init__(): #(?)
##    outstr=_string.new()
##    current_system=_std.GetSystemName()
##    last_system=_std.GetSystemName()
##    old_system=_std.GetSystemName()
##    system_map=_omap.new()
##    _omap.set(system_map,current_system,current_system)
##
##def Execute():
##    jumped=false
##    current_system=_std.GetSystemName()
##    if(current_system!=last_system):
##      // we have jumped
##      _io.sprintf(outstr,"jumped from %s to %s",last_system,current_system)
##      _io.message(0,"game","all",outstr)
##      old_system=last_system
##      last_system=_std.GetSystemName()
##      jumped=true
##    return jumped

def greet(greetingText,enemy=None,you=None):
    for i in range(len(greetingText)):
        color="#ff0000"
        text=greetingText[i]
        if type(greetingText[i])==tuple:
            if not len(greetingText[i]):
                continue
            if len(greetingText[i])>1 and greetingText[i][1]:
                color="#0000ff"
            if len(greetingText[i])>2 and greetingText[i][2] and you:
                VS.playSound(greetingText[i][2],(0.,0.,0.),(0.,0.,0.))
                if enemy:
                    you.communicateTo(enemy,-1)
                #else: doesn't actually do anything
                #    you.communicateTo(VS.Unit(),-1)
            text=greetingText[i][0]
        if (enemy):
            fromname=enemy.getFlightgroupName()+", "+enemy.getName()+"#000000"
        else:
            fromname="[Unidentified]"
        if (you):
            toname = getMessagePlayer(you)
        else:
            toname="all"
        VS.IOmessage (8+i*4,color+fromname,toname,"#ff0000"+text+"#000000")

def getDockedBase():
    iter = VS.getUnitList()
    while iter.notDone():
        if VS.getPlayer().isDocked(iter.current()) or iter.current().isDocked(VS.getPlayer()):
            return iter.current()
        iter.advance()
    return iter.current()

def getDockedBaseName():
    un = getDockedBase()
    if (un):
        return (un.getName(),un.getFullname())
    return ('','')

def ReachableSystems(startingsys):
    rv_list=[]
    closed={}
    opened=[startingsys]
    while len(opened):
	openmore=[]
	for sys in opened:
	    if not sys in closed:
		closed[sys]=1
		rv_list.append(sys)
		openmore += getAdjacentSystemList(sys)
	opened=openmore
    return rv_list    

def AllSystems():
    sys=VS.getSystemFile()
    if (VS.GetGalaxyProperty("Sol/Sol","jumps")!="" and VS.GetGalaxyProperty("Sol/Sol","faction")!=""):
        sys="Sol/Sol"
    return ReachableSystems(sys)

def addTechLevel(level, addToBase=True):
	try:
		import faction_ships
		upgrades=faction_ships.earnable_upgrades[level]
	except:
		debug.warn("No tech level named "+str(level))
		return
	bas=getDockedBase()
	if (not bas):
		import unit
		import vsrandom
		debug.debug("getting significant for upgrade addage")
		bas = unit.getSignificant(vsrandom.randrange(1,20),1,0);
	for upgrade in upgrades:
		if (len(upgrade)<5):
			debug.debug("Upgrade list not big enough to add to tech")
			print upgrade
			continue
		import Director
		import VS
		cp = VS.getCurrentPlayer()
		siz = Director.getSaveStringLength(cp,"master_part_list_content")
		doIt=True
		for index in range (siz):
			if (Director.getSaveString(cp,"master_part_list_content",index)==upgrade[0]):
				doIt=False
		if (doIt):
			debug.debug("added UPGRADE AS FOLLOWS to tech level ")
			print upgrade
			Director.pushSaveString(cp,"master_part_list_content",str(upgrade[0]))
			Director.pushSaveString(cp,"master_part_list_category",str(upgrade[1]))
			Director.pushSaveString(cp,"master_part_list_price",str(upgrade[2]))
			Director.pushSaveString(cp,"master_part_list_mass",str(upgrade[3]))
			Director.pushSaveString(cp,"master_part_list_volume",str(upgrade[4]))
			if (len(upgrade)>5):
				Director.pushSaveString(cp,"master_part_list_description",str(upgrade[5]))				
			else:
				Director.pushSaveString(cp,"master_part_list_description","No description")				
			if (bas and addToBase):
				debug.debug(" adding "+str(upgrade[0]) +" to base "+bas.getName())
				cargo=VS.Cargo(str(upgrade[0]),str(upgrade[1]),float(upgrade[2]),1,float(upgrade[3]),float(upgrade[4]))
				bas.forceAddCargo(cargo)
