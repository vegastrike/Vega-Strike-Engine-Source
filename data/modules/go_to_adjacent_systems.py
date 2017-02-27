import VS
import universe
import unit
import Briefing
import vsrandom
import Director
def formatSystemName(ship):
    where=ship.rfind("/")
    if (where!=-1):
        ship=ship[where+1:]
    return ship.capitalize()

class go_to_adjacent_systems:  
    def InSystem(self):
        return self.arrivedsys
      
    def DestinationSystem (self):
        return self.destination
      
    def JumpPoints (self):
        return self.jumps
      
    def ChangeObjective(self,newind):
        if (self.jumps[-1]!=self.jumps[newind]):
            VS.setObjective(self.obj,"Jump to %s enroute to %s." % (formatSystemName(self.jumps[newind]),formatSystemName(self.jumps[-1])))
        else:
            VS.setObjective(self.obj,"Jump to final system: %s." % (formatSystemName(self.jumps[-1])))
    
    def __init__ (self,you, numsystemsaway,jumps=(),preffaction=''):
        self.arrivedsys=0
        self.jumps=()
        self.destination=""
        self.obj=0
        self.com=0
        self.you = you
        sys=VS.getSystemFile()
        if len(jumps)>0:
            sys=jumps[-1] 
        (self.destination,self.jumps)=universe.getAdjacentSystems(sys,numsystemsaway,jumps,preffaction)
        for i in self.jumps:
            key = "visited_"+i
            leng=Director.getSaveDataLength(VS.getCurrentPlayer(),key)
            if leng==0:
                Director.pushSaveData(VS.getCurrentPlayer(),key,float(2))
        if (len(self.jumps)>0):
            self.obj=VS.addObjective("")
            self.com=(1.0/float(len(self.jumps)))
            VS.setCompleteness(self.obj,0)
            if (self.you):
                VS.setOwner(self.obj,self.you)
            self.ChangeObjective(0)
        else:
            self.arrivedsys=1
        
    def Print(self,beginstr,midstr,endstr,fro,wait=0):
        msgply=universe.getMessagePlayer(self.you)
        if (len(self.jumps)>0):
            VS.IOmessage(wait,fro,msgply,beginstr % (formatSystemName(VS.getSystemFile())))
            for i in range(len(self.jumps)-1):
                VS.IOmessage(wait,fro,msgply,midstr % (formatSystemName(self.jumps[i])))
            VS.IOmessage(wait,fro,msgply,endstr % (formatSystemName(self.jumps[len(self.jumps)-1])))
    def HaveArrived(self):
        return self.arrivedsys       
    def Execute (self):
        cursys=VS.getSystemFile()
        if (cursys in self.jumps):
            newjumps=list(self.jumps) #only lists can do 'index' but tuples strangely can do 'in'... at least it only happens when you jump...
            curind=newjumps.index(cursys)
            if ((curind+1)<len(self.jumps)):
                self.ChangeObjective(curind+1)
            newjumps.pop(curind)
            self.jumps=tuple(newjumps) #make it another tuple...
            if (cursys==self.destination):
                self.arrivedsys=1
                VS.setCompleteness(self.obj,1.0)
            else:
                VS.setCompleteness(self.obj,self.com*curind)
        return self.arrivedsys
    
    def initbriefing(self):
        self.jump_ani=0
        self.rnd_y=0.0
        self.added_warp=1
        self.brief_stage=0
        self.begintime= VS.GetGameTime()-6.0
        print "starting briefing"
        if (self.you.isNull()):
            Briefing.terminate()
            return
        self.faction=self.you.getFactionName()
        name=self.you.getName()
        self.brief_you=Briefing.addShip(name,self.faction,(0.0,0.0,80.0))
        VS.IOmessage (0,"go_to_adjacent_system","briefing","You must go to the %s system. In order to get there, you must follow this route that we have planned out for you." % self.DestinationSystem())
            
    def loopbriefing(self):
        size=len(self.JumpPoints())
        time = VS.GetGameTime()
        Briefing.setCamPosition((1.6*(time-self.begintime)*self.brief_stage,0.0,0.0))
        if (((time-self.begintime)>=5.0) and self.added_warp):
            self.jump_ani=Briefing.addShip("brief_warp",self.faction,(20.0*(self.brief_stage),self.rnd_y,79.5+self.rnd_y))
            self.added_warp=0
        if (((time-self.begintime)>=6.0)):
            if (self.jump_ani!=0):
                Briefing.removeShip(self.jump_ani)
                self.jump_ani=0
        if ((size==self.brief_stage) and ((time-self.begintime)>=6.0)):
            self.brief_stage=size+1
            self.added_warp=0
            self.time=0.0
        elif ((self.brief_stage>size) and ((time-self.begintime)>=11.0)):
            return self.brief_you
        elif (((time-self.begintime)>=6.0) and (self.brief_stage<size)):
            self.added_warp=1
            self.rnd_y=(vsrandom.random()*40.0-20.0)
            Briefing.addShip("brief_jump",self.faction,(20.0*(self.brief_stage+1),self.rnd_y,79.6+self.rnd_y))
            Briefing.enqueueOrder (self.brief_you,(20.0*(self.brief_stage+1) ,self.rnd_y,80.0+self.rnd_y) , 5.0)
            self.begintime=time
            myname=self.JumpPoints() [self.brief_stage]
            VS.IOmessage (0,"cargo mission","briefing","You must go to the '%s' jump point" % (myname))
            self.brief_stage+=1
        return -1
    def endbriefing(self):
        print "endinging briefing"
        del self.jump_ani
        del self.rnd_y
        del self.added_warp
        del self.brief_stage
        del self.begintime
        del self.faction
        del self.brief_you
