import unit
import universe
import VS
import faction_ships
import vsrandom
import launch
import Briefing
import launch_recycle
def moveUnitTo(un,place,radius):
    where=place.LocalPosition()
    print "moving "+un.getName() +" to "+place.getName()
    prsize=place.rSize();
    prp = VS.getPlanetRadiusPercent()
    if (place.isPlanet()):
        prsize*=1+prp
    where=(where[0]+un.rSize()+prsize+radius,where[1],where[2]);
    un.SetPosition(VS.SafeEntrancePoint(where,un.rSize()))
    return un
class go_somewhere_significant:
#  frameoffset=0 #see note at bottom
#  begsigdis=1.0 #same note
    def HaveArrived (self):
        return self.arrivedarea

    def SignificantUnit(self):
        return self.significantun
    
    def getSignificantFullName(self):
        return unit.getUnitFullName(self.significantun,False)
    
    def __init__ (self,you, landable_only, distance_away_to_trigger,base_only=0,capshipfaction="", dyn_fg="", showObjective=1, forcestarship=0):
        self.obj=0
        self.showObjective=showObjective
        self.orbitee=""
        self.capship=0
        self.you = you
        self.arrivedarea=0
        self.distfrombase=distance_away_to_trigger
        significant=VS.Unit()
        aroundthe=""
        self.sysfil=VS.getSystemFile()
        if (landable_only or base_only):
            randint=vsrandom.randrange(0,128)
            lim=1
            if landable_only and not base_only:
                lim=10
            for i in xrange(lim):
                significant = unit.getSignificant (randint,landable_only,base_only)
                
                if significant.isPlanet():
                    break
                else:
                    randint+=1
            if (landable_only and not unit.isLandable(significant)) or forcestarship:
                self.capship=1
                if (capshipfaction==""):
                    capshipfaction="merchant"
            if (self.capship and capshipfaction!=""):
                if (significant.isNull()):
                    significant=you
                self.orbitee="%s" % (significant.getName())
                self.capship=1
                print "orbitee %s " % self.orbitee
                if (dyn_fg==""):
                    newship=faction_ships.getRandomCapitol(capshipfaction)
                    found=False
                    near=2000.0
                    far=5000.0
                    try:
                        near*=faction_ships.launch_distance_factor
                        far*=faction_ships.launch_distance_factor
                    except:
                        pass

                    i=VS.getUnitList()
                    while i.notDone():
                        testun = i.current()
                        i.advance()
                        if testun.getFactionName()==capshipfaction and faction_ships.isCapital(testun.getName()):
                           significant=moveUnitTo(testun,significant,near)
                           found=True
                           break
                    if (not found):
                        significant=launch.launch_wave_around_unit("Base",capshipfaction,newship,"sitting_duck",1,near,far,significant,"")
                else:
                    near=5000.0
                    try:
                        near*=faction_ships.launch_distance_factor
                    except:
                        pass
                    found=False
                    aroundthe=" near "+unit.getUnitFullName(significant,True);
                    i = VS.getUnitList()
                    while i.notDone():
                        testun = i.current()
                        i.advance()
                        if testun.getFactionName()==capshipfaction and faction_ships.isCapital(testun.getName()):
                           significant=moveUnitTo(testun,significant,near)
                           found=True
                           break
                    if (not found):
                        significant=launch_recycle.launch_dockable_around_unit(dyn_fg,capshipfaction,"sitting_duck",near,significant,4*near,'','Base')
                    significant.setFullname(dyn_fg)
        else:
            significant = universe.getRandomJumppoint ()
        if (significant.isNull()):
            print "ERROR: no significants found in starsystem %s" % (self.sysfil)
            self.significantun=VS.getPlayer()
        else:
            self.significantun=significant
            self.significantun.setMissionRelevant()
            #qualifier="the "
            #if (significant.isPlanet() and significant.isDockableUnit()):
            #    qualifier=""
            if (self.showObjective):
                self.obj=VS.addObjective("Visit %s %s" % (self.getSignificantFullName (),aroundthe))
                VS.setOwner(self.obj,VS.getPlayer())
#      self.begsigdis=self.you.getSignificantDistance(self.significantun) #see note below

    def Print(self,visitstr,fro,dockstr="\0%s",time=0):
        if (self.capship):
            visitstr+=(dockstr % (self.orbitee))
        thename=unit.getUnitFullName(self.significantun,True)
        VS.IOmessage(time,fro,universe.getMessagePlayer(self.you),visitstr % (thename) )
    def DestinationSystem(self):
        return self.significantun.getUnitSystemFile();
    def JumpPoints (self):
        return self.DestinationSystem()
    def Execute(self):
        if (self.significantun.isNull() or self.you.isNull() or VS.getSystemFile()!=self.DestinationSystem()):
            return 0
#    self.frameoffset+=1 #see note below...
        sigdis=self.you.getSignificantDistance(self.significantun)
        if (sigdis<=self.distfrombase):
            self.arrivedarea=1
            if (self.showObjective):
                VS.setCompleteness(self.obj,1.0)
#    if ((not self.arrivedarea) and (self.frameoffset%25)):
#      VS.setCompleteness(self.obj,(1-(float(sigdis)/float(self.begsigdis)))) #doesn't work too well... for now, it will be 0 until you dock
        return self.HaveArrived()
    def initbriefing (self):
        self.mytime = VS.GetGameTime();
        faction=self.you.getFactionName();
        name=self.you.getName()
        self.brief_you=Briefing.addShip(name,faction,(40.0,0.0,80.0))
        faction=self.significantun.getFactionName()
        name = self.significantun.getName()
        self.brief_sig=Briefing.addShip(name,faction,(-40,0.0,8000.0))
        Briefing.enqueueOrder (self.brief_you,(-30,0.0,7900.0),5.0)
    def loopbriefing (self):
        if (VS.GetGameTime()-self.mytime>5):
            return self.brief_you
        return -1
    def endbriefing(self):
        del self.mytime
        del self.brief_you
        del self.brief_sig
