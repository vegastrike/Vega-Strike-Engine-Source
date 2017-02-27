import vsrandom
import VS
import unit
import launch
from wc1 import findOriginAndMove
from Vector import Add
class wc1_mis0:
    def __init__(self):
        pass
    def Start(self,carrier):
        self.inflight=0
        self.origin = findOriginAndMove(carrier).Position();
        self.carrier=carrier
        self.wingman= launch.launch (VS.getPlayer().getFlightgroupName(),"confed","nova","default",1,1,Add((1000,200,0),self.origin))
        self.nav=[]
        self.visited=[0,0,0]
        self.launched=[0,0,0]
        self.nav+=[launch.launch("nav1","neutral","navpoint","sitting_duck",1,1,Add(self.origin,(100000,0,50000)))]
        self.nav+=[launch.launch("nav2","neutral","navpoint","sitting_duck",1,1,Add(self.origin,(0000,0,100000)))]
        self.roids = [VS.launch("Asteroids","AFieldSparse","neutral","asteroid","default",1,1,Add(self.origin,(0000,0,100000)),""),
                      VS.launch("Asteroids","AFieldThin","neutral","asteroid","default",1,1,Add(self.origin,(-10000,0,50000)),"")]
        self.nav+=[launch.launch("nav3","neutral","navpoint","sitting_duck",1,1,Add(self.origin,(-100000,0,48000)))]
    def LaunchNav (self,i,playa):
        if (i==0):
            launch.launch_wave_around_unit("BadGuys","aera","lekra","default",3,100,1000,playa)
        elif (i==2):
            launch.launch_wave_around_unit("BadGuys","aera","kyta","default",2,100,1000,playa)
    def EndMission (self):
        self.wingman.Kill()
        for n in self.nav:
            n.Kill()
        for n in self.roids:
            n.Kill()
        if (self.visited[0] and self.visited[1] and self.visited[2]):
            #success (change debrief maybe?)
            return ("vega_sector/vega",0)
        else:
            #failed
            return ("vega_sector/vega",0)
    def Execute(self):
        playa= VS.getPlayer()
        if (playa):
            for i in range (len(self.nav)):
                if (self.nav[i]):
                    if (not self.launched[i]):
                        if (playa.getDistance (self.nav[i])<1000):
                            self.LaunchNav(i,playa)
                            self.launched[i]=1
                    else:
                        if (playa.getDistance (self.nav[i])<100):
                            self.visited[i]=1
            if (playa.isDocked(self.carrier) or self.carrier.isDocked(playa)):
                if (self.inflight):
                    return self.EndMission()
            else:
                self.inflight=1
        return
