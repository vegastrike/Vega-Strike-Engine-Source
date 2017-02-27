import vsrandom
import VS
import unit
import launch
from Vector import Add
from wc1 import findOriginAndMove
class wc1_mis1:
    def __init__(self):
        pass
    def Start(self,carrier):
        self.inflight=0
        self.jump = findOriginAndMove(carrier,(0,0,-20000));
        self.carrier=carrier
        self.origin = carrier.Position();
        self.wingman= launch.launch (VS.getPlayer().getFlightgroupName(),"confed","nova","default",1,1,Add((1000,200,0),self.origin))
        self.transport= launch.launch ("Transport","confed","cargoship","default",1,1,Add((-1000,200,0),self.origin))
        self.transport.ActivateJumpDrive(0)
        self.transport.SetTarget(self.jump)

        self.launched=[0,0]
        self.nav=[launch.launch("nav1","neutral","eject","sitting_duck",1,1,Add(self.origin,(20000,000,10000))),
                  self.jump]
        self.roids = [VS.launch("Asteroids","AFieldSparse","neutral","asteroid","default",1,1,Add(self.origin,(0000,00000,10000)),"")]
    def moveTrans (self,targ):
        print "moving trans"
        self.transport.SetPosAndCumPos(Add(targ.Position(),(0,0,-1000)))

    def LaunchNav (self,i,playa):
        print "launching %d" % i
        if (i==0):
            launch.launch_wave_around_unit("BadGuys","aera","lekra","default",3,100,1000,playa)
            self.moveTrans(self.nav[0])
        elif (i==1):
            if (self.launched[0]):
                self.moveTrans(self.nav[1])
                launch.launch_wave_around_unit("BadGuys","aera","kyta","default",2,100,1000,playa)
    def EndMission (self):
        print "endmission"
        self.wingman.Kill()
        for n in self.nav:
            n.Kill()
        for n in self.roids:
            n.Kill()
        VS.terminateMission(1)
        if (self.transport):
            if (not unit.inSystem(self.transport)):
                print "success"
                return ("vega_sector/vega",0)
            #success (change debrief maybe?)
        return ("vega_sector/hellskitchen",0)
    def Execute(self):
        playa= VS.getPlayer()
        if (playa):
            for i in range (len(self.nav)):
                if (self.nav[i]):
                    if (not self.launched[i]):
                        if (playa.getDistance (self.nav[i])<1000):
                            self.LaunchNav(i,playa)
                            self.launched[i]=1
            if (playa.isDocked(self.carrier) or self.carrier.isDocked(playa)):
                if (self.inflight):
                    return self.EndMission()
            else:
                self.inflight=1
        return
