import VS
import debug
import Vector

#statistics - for profiling (disabled in release version)
try:
    profiling_level = int(VS.vsConfig("general","profiling","0"))
except:
    profiling_level = 0

def isLandable (un):
    if (un):
        return un.isDockableUnit()

def isBase (un):
    unit_fgid = un.getFlightgroupName()
    retval = unit_fgid=="Base"
    return retval and not un.isSun()

def findPlayerNum (un):
    for i in range (VS.getNumPlayers()):
        if (VS.getPlayerX(i)==VS.getPlayer()):
            return i
    return 0

def isAsteroid (un):
    unit_fgid = un.getFlightgroupName()
    retval = unit_fgid=="Asteroid"
    return retval

def moveOutOfPlayerPath(un):
    def reposition(un,playa,min_distance,vel):
        import Vector
        import vsrandom
        ex=(vsrandom.uniform(-1,1),vsrandom.uniform(-1,1),vsrandom.uniform(-1,1))
        if vel[0]==0 and vel[1]==0 and vel[2]==0:
           vel=(1,0,0)
        dir=Vector.Scale(Vector.ScaledCross(ex,vel),min_distance)
        print "offsetting you a few meters to the "+str(dir)
        un.SetPosition(Vector.Add(playa.Position(),dir))
    import faction_ships
    playa=VS.getPlayer()
    min_distance=100
    min_forward_distance=1300
    try:
       min_distance=faction_ships.min_distance
       min_forward_distance=faction_ships.min_forward_distance
    except:
       print "badness no faction_ships.min_distance"
    dis=un.getDistance(playa)
    import Vector
    vel=playa.GetVelocity()
    vel=Vector.SafeNorm(vel)
    if (dis<min_distance):
       reposition(un,playa,min_distance,vel)
    dir=Vector.SafeNorm(Vector.Sub(playa.Position(),un.Position()))
    if (dis<min_forward_distance and Vector.Dot(dir,vel)>.8):
       reposition(un,playa,min_distance,vel)

def getUnitFullName(un,inenglish=False):
    thename=(un.getFullname())#.replace("_"," ")
    snumber=str(un.getFgSubnumber())
    fg=un.getFlightgroupName()#.replace("_"," ")
    if un.isPlanet():
	thename=un.getName()
    elif fg=='Base':
        if un.getFullname().capitalize()==un.getName().capitalize():
             thename=thename+" "+snumber        
    elif fg!='Shadow' and fg!='':
        if inenglish:
           thename=un.getFactionName().replace("_"," ")+" "+thename+ " "+snumber +' in the '+fg+' flightgroup'
        else:
           thename=thename+":"+fg+' <'+snumber+'>'
    else:
        thename=thename+":"+fg+' <'+snumber+'>'
    return thename

def getSignificant (whichsignificant, landable_only, capship_only):
    import vsrandom
    if not (landable_only or capship_only):
        rez = getPlanetList(True) #Gets all significants
    else:
        rez = []
        i = VS.getUnitList()
        while i.notDone():
            un = i.current()
            if (capship_only and isBase(un)) or (not capship_only and isLandable(un)):
                rez.append(un)
            i.advance()
    if (len(rez)==0):
        if (capship_only):
            return getSignificant(whichsignificant,landable_only,0)
        elif(landable_only):
            return getSignificant (whichsignificant,0,0)
        else:
            debug.warn("no significants in system "+VS.getSystemFile())
            return VS.getPlayer()
    return rez[vsrandom.randrange(0,len(rez))]

  #this one terminates if fewer than so many planets exist with null

def inSystem (unit):
    i=VS.getUnitList ()
    while (i.notDone()):
        if (i.current()==unit):
            return 1
        i.advance()
    return 0

def getPlanet (whichsignificant, sig):
    i = VS.getUnitList()
    if sig:
        i.advanceNSignificant(whichsignificant)
    else:
        i.advanceNPlanet(whichshignificant)   
    if i.isDone():
        return VS.Unit()
    else:
        return i.current()

def getPlanetList (sig):
    res = []
    i = VS.getUnitList()
    if i.notDone():
        if sig:
            i.advanceNSignificant(0)
        else:
            i.advanceNPlanet(0)
    while i.notDone():
        res.append(i.current())
        if sig:
            i.advanceSignificant()
        else:
            i.advancePlanet()   
    return res

def getJumpPoint(whichsignificant):
    i = VS.getUnitList()
    i.advanceNJumppoint(whichsignificant)
    if i.isDone():
        un = VS.Unit()
        un.setNull()
        return un
    else:
        return i.current()

def obsolete_getNearestEnemy(my_unit,range):
    i = VS.getUnitList()
    min_dist=9999999.0
    min_enemy=VS.Unit()
    while(i.notDone()):
        un=i.current()
        unit_pos=un.Position()
        dist=my_unit.getMinDis(unit_pos)
        relation=my_unit.getRelation(unit)
        if(relation<0.0):
            if((my_unit==unit) and (dist<range) and (dist<min_dist)):
                min_dist=dist
                min_enemy=unit
        i.advance()
    if(min_enemy):
        other_fgid=min_enemy.getFgID()
    return min_enemy

def obsolete_getThreatOrEnemyInRange(un,range):
    threat=un.getThreat()
    if(threat.isNull()):
        threat=obsolete_getNearestEnemy(un,range)
    return threat

def setPreciseTargetShip (which_fgid, target_unit):
    if (target_unit):
        i = VS.getUnitList()
        while i.notDone():
            un = i.current()
            unit_fgid=un.getFgID()
            if(unit_fgid[:len(which_fgid)]==which_fgid):
                un.SetTarget(target_unit)
            i.advance()

def getMinDistFrom(sig1,siglist=None):
    siglist=siglist or getPlanetList(0)
    mindist=100000000000000000000000000000000000000000000.0
    for sig2 in siglist:
        tempdist = sig1.getSignificantDistance(sig2)
        if (tempdist<mindist and tempdist>0.0):
            mindist=tempdist
    return mindist

def minimumSigDistApart():
    siglist=getPlanetList(0)
    i=0
    mindist=100000000000000000000000000000000000000000000.0
    ave=0.0
    for sig1 in siglist:
        tempdist = getMinDistFrom (sig1,siglist)
        if (ave<0.9):
            mindist = tempdist
        else:
            mindist += tempdist
        ave+=1.0
    if (ave!=0.0):
        mindist = mindist/ave
    return mindist

def getUnitByName (name):
    return VS.getUnitByName(name) or VS.getUnit(0)

def getUnitByFgIDFromNumber(fgid, ship_nr):
    i = VS.getUnitList()
    i.advanceN(ship_nr)
    found_unit = VS.Unit()
    while i.notDone() and not found_unit:
        un = i.current()
        unit_fgid=un.getFgID()
        if(unit_fgid==fgid):
            found_unit=un
        i.advance()
    return found_unit

def getUnitByFgID(fgid):
    return getUnitByFgIDFromNumber(fgid,0)

def setTargetShip(which_fgid,target_fgid):
    target_unit=getUnitByFgID(target_fgid)
    setPreciseTargetShip(which_fgid,target_unit)

def removeFg(which_fgid):
    i = VS.getUnitList()
    klist = []
    while i.notDone():
        un = i.current()
        unit_fgid=un.getFgID()
        if(unit_fgid[:len(which_fgid)]==which_fgid):
            klist.append(un)
        i.advance()
    for un in klist:
        un.Kill()

# A collection of functions useful for dealing with flightgroup tuples.
# Added as used.
# For more information on the types of veriables used,
# see the VS python doc in the manual.

# Cloaks or uncloaks (1 or 0) a flightgroup tuple (tup).
def TfgCloak(state,tup):
    num = len(tup)
    for i in range(num):
        tup[i].Cloak(state)
        num = num + 1

# Tells us if a tup is null
def TfgisNull(tup):
    num = 0
    for i in tup:
        if (i):
            num = num + 1
    if num == 0:
        return 1
    else:
        return 0

# Returns an integer value of the number of ships in the tup.  Not sure if it takes notice of null state.
def TfgHeadCount(tup):
    num = 0
    for i in tup:
        if (i):
            num = num + 1
    return num

# Sets a whole tupled flightgroup on a target.
def setTfgDirective(tup,tgt,dir):
    num = len(tup)
    for i in range(num):
        tup[i].SetTarget(tgt)
        tup[i].setFgDirective(dir)

# Jumps a whole fg tuple using the JumpTo command.
def TfgJumpTo(tup,system):
    num = len(tup)
    for i in range(num):
        tup[i].JumpTo(system)

def getUnitSequenceBackwards():
    rez1 = []
    rez2 = []
    i = VS.getUnitList()
    while i.notDone():
       rez1.append(i.current())
       i.advance()
    i = len(rez1)-1
    while (i>=0):
       rez2.append(rez1[i])
       i-=1
    return rez2

# the ship will approach the station until 500 meters and then dock
def approachAndDock (ship,station):
    # if ship is more than 500m away it will set
    distance = ship.getDistance(station) #+station.rSize()
    if (distance>=500):
        # orientate the nose of the ship towards the station
        vec = Vector.Sub(station.Position(),ship.Position())
        ship.SetOrientation((1,0,0),vec)
        # set velocity proportional to distance from player
        vec = max(10,Vector.Scale(Vector.SafeNorm(vec),distance/10))
        ship.SetVelocity(vec)
    # if ship has approached the station until 500m then stop it
    if (distance<500):
        ship.SetVelocity((0,0,0))
        # this is also needed to stop rotation of the ship
        ship.SetAngularVelocity((0,0,0))
        # request clearance and dock seems to have no effect
        station.RequestClearance(ship)
        ship.Dock(station)
        # emulate docking by making the ship disappear
        ship.Kill()
        ship.setNull()
    return 1

# orientate the nose of the ship towards the station
def faceTaget (ship,target):
    vec = Vector.Sub(target.Position(),ship.Position())
    ship.SetOrientation((1,0,0),vec)

# returns the facing angle between unit 1 and unit 2
# when unit 1 is facing unit 2 the return value is 0
# when unit 1 is completely turned away from unit 2 the return value is pi (~3.14)
def facingAngleToUnit(unit1,unit2):
    vec = Vector.Sub(unit2.Position(),unit1.Position())
    dot = Vector.Dot(Vector.SafeNorm(unit1.GetOrientation()[2]),Vector.SafeNorm(vec))
    angle = VS.acos(dot)
    return angle

# signed velocity is negative when the thrust is reverse
# otherwise velocity is positive
def getSignedVelocity(unit):
    velocity = Vector.Dot(Vector.SafeNorm(unit.GetOrientation()[2]),unit.GetVelocity())
    return velocity

# change the direction and thrust forward at the same time
def changeDirectionAndThrust(unit, angular, thrust):
    import Vector
    import vsrandom
    unit.SetAngularVelocity(angular)
    unit.LongitudinalThrust(thrust)
    return 1

# returns the shield percent for a ship
# ships with dual shields will return 0.5 with full shields
def getShieldPercent(unit):
    shield = (unit.FShieldData() + unit.RShieldData() + unit.LShieldData() + unit.BShieldData()) / 4
    return shield
