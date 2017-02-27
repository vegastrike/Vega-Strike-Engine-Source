from go_to_adjacent_systems import *
from go_somewhere_significant import *
import vsrandom
import launch
import faction_ships
import VS
import Briefing
import universe
import unit
import Director
import quest
class go_none:
    def Execute(self):
        return 1
    def HaveArrived(self):
        return 1
    def SignificantUnit(self):
        return VS.getUnit(0);
    def __init__(self):
        pass

isambushrunning={}

class directions_mission (Director.Mission):
    def dir_privateSetupPlayer(self,cp):
        print "setting up mission"
        print self.jumps
        self.arrived=0
        self.wasnull=0
        self.you=VS.getPlayerX(cp)
        self.adjsys=go_to_adjacent_systems(self.you,0,self.jumps)
        self.adjsys.Print("You should start in the system named %s","Then jump to %s","Lastly, jump to %s, your final destination","cargo mission",1)
    def setupPlayer(self,cp):
        self.dir_privateSetupPlayer(cp)
    def __init__ (self,savevar,jumps=(),destination=''):
        Director.Mission.__init__(self);
        print 'Directions: Starting'
        global isambushrunning
        self.var=savevar
        self.savedCargo=self.getCargo(VS.getPlayer())
#        print self.savedCargo
        if (self.var,self.savedCargo) in isambushrunning:
            #VS.terminateMission(0)
            print 'Directions: Stopping: directions already running! (before mission restore)'
        isambushrunning[(self.var,self.savedCargo)]=True
        self.jumps=jumps
        self.cp=VS.getCurrentPlayer()
        self.you=VS.Unit()
        self.base=VS.Unit()
        self.arrived=0
        self.destination=destination
        self.mplay="all"
        self.dir_privateSetupPlayer(self.cp)
        self.mplay=universe.getMessagePlayer(self.you)
        self.obj=0
        name = self.you.getName ()

    def takeCargoAndTerminate (self,you, remove=1):
        global isambushrunning
        if ((self.var,self.savedCargo) in isambushrunning):
            del isambushrunning[(self.var,self.savedCargo)]
        VS.terminateMission(1)
        return
        
    def findUnit(self, name):
        i = VS.getUnitList()
        while i.notDone():
            testun=i.current()
            i.advance()
            if testun.getName().lower()==name.lower() or testun.getFullname().lower()==name.lower():
                return testun
        i = VS.getUnitList()
        while i.notDone():
            testun=i.current()
            i.advance()
            if testun.isDockableUnit():
                return testun
        return VS.getUnit(0)
    def getCargo(self,un):
        lis=[]
        for i in range(un.numCargo()):
            if (un.GetCargoIndex(i).GetMissionFlag()):
                lis.append(un.GetCargoIndex(i).GetContent())
        return tuple(lis)
    def checkCargo(self,un):
        import quest
        return not (quest.checkSaveValue(self.cp,self.var,0) or quest.checkSaveValue(self.cp,self.var,1) or quest.checkSaveValue(self.cp,self.var,-1) or self.getCargo(un)!=self.savedCargo)
    def Execute (self):
        if (VS.getPlayerX(self.cp).isNull()):
            self.wasnull=1
            return
        if (self.arrived and self.base.isNull()):
            return
        if (self.wasnull):
            print "INEQUALITY"
            if (not self.checkCargo(VS.getPlayerX(self.cp))):
                self.takeCargoAndTerminate(self.you,1)
                return
            else:
                self.setupPlayer(self.cp)
        if (not self.you):
            return
        if (not self.adjsys.Execute()):
            return
        if (self.destination=='' and len(self.jumps)==0):
            return #obviously wrapper for ambush
        if (self.arrived):
            dis=self.you.getSignificantDistance(self.base)
            if (dis<200):
                VS.setCompleteness(self.obj,1)
            if (dis<50 or self.base.isDocked(self.you) or self.you.isDocked(self.base)):
                self.takeCargoAndTerminate(self.you,1)
                return
        else:
            self.arrived=1
            self.adjsys=go_none()
            self.base=self.findUnit(self.destination)
            self.obj=VS.addObjective("Deliver cargo to %s." % self.destination);
            VS.setOwner(self.obj,self.you)
            VS.setCompleteness(self.obj,0)






