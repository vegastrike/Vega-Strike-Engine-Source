import quest
import Vector
import VS
import unit
import vsrandom
import save_util
import faction_ships
import universe
import launch
class quest_isowing (quest.quest):
    def recoverShipTypes (self):
        return save_util.loadStringList (self.playernum,"%s_wingships"%self.faction);
    def genShipTypes (self,num):
        namelist= []
        for i in range (num):
            namelist+= [faction_ships.getRandomFighter (self.faction)]
        return namelist
    def putShipTypes (self,names):
        save_util.saveStringList(self.playernum,"%s_wingships"%self.faction,names);
    def Respawn (self):
        for n in self.winglist:
            if (n):
                n.Kill()
        self.names = self.recoverShipTypes ()
        self.SpawnShips ()
    def SpawnShips (self):
        print "spawning"
        print self.names
        playa = VS.getPlayer()
        if (playa):
            for n in self.names:
                if (len (n)>0):
                    self.winglist+=[self.SpawnShip (playa,n)]
    def SpawnShip (self,playa,name):
        un=launch.launch_wave_around_unit(playa.getFlightgroupName(),
                                          playa.getFactionName(),
                                          name,
                                          "default",
                                          1,
                                          playa.rSize()+100,
                                          playa.rSize()+1000,
                                          playa,
                                          "")
        un.setFactionName(self.faction)
        return un
    def makeAdversary (self,faction,num):
        self.adv=[]
        playa = VS.getPlayer()
        if (playa):
            for i in range (num):
                self.adv+=[launch.launch_wave_around_unit("Shadow",
                                                          faction,
                                                          faction_ships.getRandomFighter(faction),
                                                          "default",
                                                          1,
                                                          playa.rSize()+500,
                                                          playa.rSize()+2000,
                                                          playa,
                                                          "")]
    def ReName (self):
        if (self.cname >= len (self.winglist)):
            self.cname=0
        else:
            if (not self.winglist[self.cname]):
                if (self.names[self.cname]!=''):
                    self.names[self.cname]=''
                    self.putShipTypes(self.names)
            self.cname+=1
    def ReTarget (self):
        if (self.tname >=len (self.adv)):
            self.tname=0
        else:
            ship = self.winglist[self.tname % len (self.winglist) ]
            if (self.adv[self.tname]):
                if (ship):
                    self.adv[self.tname].SetTarget (ship)
                self.tname +=1
            else:
                del self.adv[self.tname]
    def __init__ (self,faction,num,enfaction,ennum):
        self.faction = faction
        self.num = num
        self.winglist=[]
        self.adv=[]
        self.cname=0
        self.tname=0
        self.respawn=0
        self.won=0
        self.norep=0
        self.begin=0
        self.enfaction=enfaction
        self.ennum = ennum
    def Intro(self):
        VS.IOmessage (0,"game","all","[%s Enforcers] Under edict 502419 We hereby command"%self.enfaction)
        VS.IOmessage (0,"game","all","[%s Enforcers] All %s terrorists to power down and prepare to be boarded."%(self.enfaction,self.faction))
        VS.IOmessage (3,"game","all","[%s Enforcers] You are in violation of anti-terrorist laws."%self.enfaction)
        VS.IOmessage (4,"game","all","[%s Enforcers] And are conspiring against the legitimate Star Confederacy."%self.enfaction)
        VS.IOmessage (5,"game","all","[%s Wing] We have nothing to do with your alleged terrorists"%self.faction)
        VS.IOmessage (5,"game","all","[%s Wing] and are only exercising our freedom of assembly."%self.faction)
        VS.IOmessage (8,"game","all","[%s Wing] This is a %s wing requesting assistance from any nearby craft! We are being arrested unfairly!"% (self.faction,self.faction))
        VS.IOmessage (8,"game","all","[%s Wing] Should anyone honor our request for help we would be severely in debt!"% self.faction)

    def Begin(self):
        self.begin=1
        self.respawn=0
        if (self.isPersistent()):
            self.won=1
            self.names = self.recoverShipTypes ()
            VS.IOmessage (0,"game","all","Establishing link communication.")
            VS.IOmessage (0,"game","all","[%s Wing] We're back at your service." % self.faction)
        else:
            VS.SetAutoStatus (0,-1)
            self.names = self.genShipTypes (self.num)
            self.makeAdversary(self.enfaction,self.ennum)
            self.Intro()
                #vsrandom.randrange (1,self.num+1))
        self.SpawnShips ()
        self.putShipTypes(self.names)

    def doWin(self):
        self.won=1
        if (self.NothingHere()):
            return
        VS.SetAutoStatus (0,0)
        self.makeQuestPersistent()
        VS.IOmessage (0,"game","all","Thank you for saving us!")
        VS.IOmessage (0,"game","all","We owe our lives to you...")
        VS.IOmessage (0,"game","all","So therefore we will go with you and follow your orders where they may take us")
        VS.IOmessage (0,"game","all","Together we can succeed!")
        VS.IOmessage (0,"game","news",'TERRORIST FLIGHTGROUP ESCAPES   Recently a long sought for %s wing confronted and destroyed %s special forces. "After learning of a recent assembly by these millitants, we had to move at once; it is sad that these militants preemtively struck our forces before retreating like cowards!" said SubCommander Ret in a recent interview with GNN, "We need to warn our citizens of the presence of roaming %s starships...Any sighting of these starships must be reported to %s authorities immediately."  Will such slips in confed security cause further problems in the frontier, or will they finally meet their end after a deadly manhunt? The future remains to be seen.  The SubCommanders parting words were: "But See to it I will, I can promise you that!"'%(self.faction,self.enfaction,self.faction,self.enfaction))
    def doLose(self):
        VS.SetAutoStatus (0,0)
        self.removeQuest()
        VS.IOmessage (0,"game","all","Terrorists eliminated! Looks like we have another success to report for %s."%self.enfaction)
        VS.IOmessage (0,"game","news",'TERRORIST CELL DESTROYED   Recently a %s wing was eliminated by %s special forces. "After learning of a recent assembly by these millitants, we had to move at once!" said SubCommander Ret in a recent interview with GNN, "It is good to see such justice served on our frontiers!" Hopefully this will reduce the recent rash of violence in the frontier sectors of our society!'%(self.faction,self.enfaction))
        if (self.won==0):
            VS.SetAutoStatus (0,0)

    def NothingHere(self):
        for i in self.winglist:
            if (i):
                return 0
        return 1

    def Execute (self):
        playa=VS.getPlayer()
        if (playa.isNull()):
            if (self.won==1):
                self.respawn=1
            return 1
        if (self.begin==0):
            self.Begin()
        if (self.respawn):
            self.respawn=0
            self.Respawn()
        if (len (self.winglist)==0):
            VS.SetAutoStatus (0,0)
            return 0
        if (len (self.adv)==0):
            if (self.won==0):
                self.doWin()
        else:
            self.ReTarget()
        self.norep+=1
        if (self.norep>100):
            self.ReName()
            self.norep=0
            if (self.NothingHere()):
                self.doLose()
                return 0
        return 1

class quest_isowing_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_isowing")
    def create (self):
        return quest_isowing("ISO",2,"confed",10)
