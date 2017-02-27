import quest
import Vector
import VS
import unit
import vsrandom
class quest_disappear (quest.quest):
    def __init__ (self):
        self.sysfile = VS.getSystemFile()
        self.drone=VS.Unit()
        self.significants=1
        self.timer = VS.GetGameTime();
        VS.cacheAnimation ("explosion_wave.ani");
        self.playlist=VS.musicAddList("galacticbattle.m3u")
    def launchNewDrone (self):
        playa=VS.getPlayer()
        if (not playa.isNull()):
            if (self.drone.isNull()):
                vec = playa.Position()
                vec = Vector.Add(vec,(000,3000,0))
                self.drone=VS.launch("IO42","beholder","unknown","unit","default",1,1,vec,'')
                self.drone.SetTarget(playa)
                pos=self.drone.Position()
                size=10*self.drone.rSize()
                VS.playAnimation("explosion_wave.ani",pos,size)
                if (self.playlist != -1):
                    VS.musicPlayList(self.playlist)
                    VS.musicLoopList(2)
#                VS.musicPlaySong('../music/galacticbattle.mp3')

    #not currently used
    def setDroneNear (self,playa):
        vec = playa.Position()
        vec = Vector.Add (vec,(vsrandom.uniform(-1000,1000),
                               vsrandom.uniform(-1000,1000),
                               vsrandom.uniform(-1000,1000)))
        if (not self.drone.isNull()):
            self.drone.SetCurPosition(vec)
            self.drone.SetTarget(playa)
    def SignificantsNear(self,sig):
        self.significants=0
        if (VS.getSystemFile()==self.sysfile):
            self.significants=1
            if (vsrandom.randrange(0,10)==0):
                self.launchNewDrone()
    def unitRipe (self,playa,un):
        if (un!=self.drone):
            if (un.isPlayerStarship()==-1):
                if (not un.isSignificant()):
                    if (not un.isSun()):
                        if (un.getFlightgroupLeader()!=playa):
                            if (playa.InRange(un,0,1)):
                                return 1
        return 0
    def killUnit (self,un):
        pos=un.Position()
        size=4*un.rSize()
        if (size<1000):
            size=1000
        print "kill"
        print un.getName()
        un.Kill()
        VS.playAnimation("explosion_wave.ani",pos,size)
        VS.playSound("cloak.wav",pos,(1,0,0))
    def DestroyUnit (self):
        playa = VS.getPlayer()
        if (not playa.isNull()):
            if (vsrandom.randrange(0,2)==0):
                un=playa.GetTarget()
                if (not un.isNull()):
                    if (self.unitRipe(playa,un)):
                        self.killUnit(un)
                        return
            i=VS.getUnitList ()
            un = i.current()
            while (not un.isNull()):
                if (self.unitRipe(playa,un)):
                    self.killUnit(un)
                    return
                i.advance()
                un=i.current()
    def Execute (self):
        if (VS.getSystemFile()==self.sysfile):
            mytime = VS.GetGameTime();
            if (mytime-self.timer>10):
                self.timer=mytime
                if (vsrandom.randrange(0,5)==0):
                    self.DestroyUnit()
        return 1

class quest_disappear_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_disappear")
    def create (self):
        return quest_disappear()
