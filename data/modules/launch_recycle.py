import vsrandom
import unit
import launch
import VS
import faction_ships
import dj_lib
import debug

def NextPos (un, pos, FarApart=1):
    rad=un.rSize ()
    whichcoord = vsrandom.randrange(0,3)
    x = pos[whichcoord]
    pos = list(pos)
    x=x+3.0*rad*FarApart
    debug.debug("next coord is "+str(3.0*rad*FarApart)+" awa")
    pos[whichcoord]=x
    return tuple(pos)
  
def move_to (un, where):
    un.SetPosition(where)
    unit.moveOutOfPlayerPath(un)
    un.SetTarget (VS.Unit())
    return NextPos (un,where)
  
def whereTo (radius, launch_around):
    if (type(launch_around)==type( (1,2,3))):
        pos=launch_around
        rsize = faction_ships.max_radius
    else:
        pos = launch_around.Position ()    
        rsize = ((launch_around.rSize())*5.0)
    if (rsize > faction_ships.max_radius):
	    rsize=faction_ships.max_radius
    rsize+=radius;
    import fg_util
    dir = fg_util.randDirection()
    return (pos[0]+rsize*dir[0],
            pos[1]+rsize*dir[1],
            pos[2]+rsize*dir[2])
  
def unOrTupleDistance(un,unortuple,significantp):
    if (type(unortuple)==type((1,2,3))):
        import Vector
        return Vector.Mag(Vector.Sub(un.Position(),unortuple))-un.rSize()
    else:
        if (significantp):
            return un.getSignificantDistance(unortuple)
        else:
            return un.getDistance(unortuple)
# THIS FUNCTION NO LONGER WORKS...because it's kinda silly to drag units around the system forcing them in front of a player
def look_for (fg, faction, numships,myunit,  pos, gcd,newship=[None]):
    return (numships,pos)
    unl = unit.getUnitSequenceBackwards()
    for un in unl:
        if (numships<=0):
            break
        if (unOrTupleDistance(un,myunit,1)>gcd ):
            fac = un.getFactionName ()
            fgname = un.getFlightgroupName ()
            name = un.getName ()
            if ((fg==fgname) and (fac==faction)):
                if (vsrandom.random()<0.75):
                    pos=move_to (un,pos)
                    numships-=1
                    newship[0]=un
                    debug.debug("TTYmoving %s to current area" % (name))
    return (numships,pos)
  
def LaunchNext (fg, fac, type, ai, pos, logo,newshp=[None],fgappend='',FarApart=1):
    debug.debug("Launch nexting "+str(type))
    combofg=fg+fgappend
    if (fgappend=='Base'):
        combofg=fgappend
    newship = launch.launch (combofg,fac,type,ai,1,1,pos,logo,1,1)
    import dynamic_universe
    dynamic_universe.TrackLaunchedShip(fg,fac,type,newship)
    rad=newship.rSize ()
#VS.playAnimation ("warp.ani",pos,(3.0*rad))
    newshp[0]=newship
    return NextPos (newship,pos,FarApart)

def launch_dockable_around_unit (fg,faction,ai,radius,myunit,garbage_collection_distance,logo='',fgappend='',skipdj=0):
    import fg_util
    for i in fg_util.LandedShipsInFG(fg,faction):
        if (i[0]=='Mule.stock' or i[0]=='Mule' or faction_ships.isCapital(i[0])):
            un=launch_types_around (fg,faction,[i],ai,radius,myunit,garbage_collection_distance,logo,fgappend,skipdj)
            if (un.isDockableUnit()):
                return un
    if (fgappend=='Base'):
        fg=fgappend
    else:
        fg=fg+fgappend
    return launch.launch_wave_around_unit(fg,faction,faction_ships.getRandomCapitol(faction),ai,1,radius,radius*1.5,myunit,logo,1,skipdj)

def launch_types_around ( fg, faction, typenumbers, ai, radius, myunit, garbage_collection_distance,logo,fgappend='',skipdj=0,FarApart=1):
    pos = whereTo(radius, myunit)
    nr_ships=0
    for t in typenumbers:
        nr_ships+=t[1]
    debug.debug("before"+str(nr_ships))
    retcontainer=[None]
    if (fgappend=='' and nr_ships>1):
        (nr_ships,pos) = look_for (fg,faction,nr_ships-1,myunit,pos,garbage_collection_distance,retcontainer)
        nr_ships+=1
    debug.debug("after "+str(nr_ships)+ str(retcontainer))
    count=0
    ret=retcontainer[0]
    found=0
    for tn in typenumbers:
        num = tn[1]
        debug.debug("Python launched "+str(faction)+" "+str(tn[0])+" FG "+str(fg)+" with "+str(num)+" ships")
        if num>nr_ships:
            num=nr_ships
        for i in range(num):
            newship=[None]
            debug.debug(pos)
            pos = LaunchNext (fg,faction,tn[0], ai, pos,logo,newship,fgappend,FarApart)
            if (i==0 and found==0):
                ret=newship[0]
                found=1
        nr_ships-=num
        if (nr_ships==0):
            if (not skipdj):
                dj_lib.PlayMusik(0,dj_lib.HOSTILE_NEWLAUNCH_DISTANCE)
            return ret
    if (not skipdj):
        dj_lib.PlayMusik(0,dj_lib.HOSTILE_NEWLAUNCH_DISTANCE)
    return ret
    
  
def launch_wave_around ( fg, faction, ai, nr_ships, capship, radius, myunit, garbage_collection_distance,logo,skipdj=0):
    pos = whereTo(radius, myunit)
    debug.debug("before"+str(nr_ships))
    (nr_ships,pos) = look_for (fg,faction,nr_ships,myunit,pos,garbage_collection_distance)
    debug.debug("after "+str(nr_ships))
    while (nr_ships>0):
        type=""
        if (capship):
            type = faction_ships.getRandomCapitol(faction)
        else:
            type = faction_ships.getRandomFighter(faction)
        pos = LaunchNext (fg,faction,type, ai, pos,logo)
        nr_ships-=1
    if (not skipdj):
        dj_lib.PlayMusik(0,dj_lib.HOSTILE_NEWLAUNCH_DISTANCE)
     
