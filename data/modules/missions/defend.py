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
class defend (Director.Mission):
    def __init__ (self,factionname,numsystemsaway, enemyquantity, distance_from_base, escape_distance, creds, defendthis, defend_base,protectivefactionname='',jumps=(),var_to_set='',dynamic_flightgroup='',dynamic_type='', dynamic_defend_fg='',waves=0, greetingText=['We will defeat your assets in this battle, privateer...','Have no doubt!']):
        Director.Mission.__init__(self)
        self.dedicatedattack=vsrandom.randrange(0,2)
        self.arrived=0
        self.waves=waves;
        self.greetingText=greetingText
        self.protectivefaction = protectivefactionname
        self.var_to_set=var_to_set
        self.quantity=0
        self.mplay="all"
        self.defendbase = defend_base   
        self.dynatkfg = dynamic_flightgroup     
        self.dynatktype = dynamic_type
        self.dyndeffg = dynamic_defend_fg
        self.attackers = []
        self.objective= 0
        self.targetiter = 0
        self.ship_check_count=0
        self.defend = defendthis
        self.defend_base = defend_base
        self.faction = factionname
        self.escdist = escape_distance
        minsigdist=unit.minimumSigDistApart()
        if (minsigdist*.5<self.escdist):
            self.escdist = minsigdist
        self.cred=creds
        self.respawn=0
        self.quantity=enemyquantity
        self.savedquantity=enemyquantity
        self.distance_from_base=distance_from_base
        self.defendee=VS.Unit()
        self.difficulty=1
        self.you=VS.getPlayer()
        name = self.you.getName ()
        self.mplay=universe.getMessagePlayer(self.you)
        self.adjsys = go_to_adjacent_systems(self.you,numsystemsaway,jumps)  
        self.adjsys.Print("You are in the %s system,","Proceed swiftly to %s.","Your arrival point is %s.","defend",1)
        VS.IOmessage (2,"defend",self.mplay,"And there eliminate any %s starships."  % self.faction)
    def SetVarValue (self,value):
        if (self.var_to_set!=''):
            quest.removeQuest (self.you.isPlayerStarship(),self.var_to_set,value)
    def SuccessMission (self):
        self.you.addCredits (self.cred)
        VS.AdjustRelation(self.you.getFactionName(),self.faction,.03,1)
        self.SetVarValue(1)
        VS.IOmessage(0,"defend",self.mplay,"[Computer] Defend mission accomplished")
        if (self.cred>0):
            VS.IOmessage(0,"defend",self.mplay,"[Computer] Bank account has been credited as agreed.")
        VS.terminateMission(1)
    def FailMission (self):
        self.you.addCredits (-self.cred)
        VS.AdjustRelation(self.you.getFactionName(),self.faction,-.02,1)                
        self.SetVarValue(-1)
        VS.IOmessage (0,"defend",self.mplay,"[Computer] Detected failure to protect mission asset.")
        VS.IOmessage (0,"defend",self.mplay,"[Computer] Mission failed!")
        VS.IOmessage (1,"defend",self.mplay,"[Computer] Bank has been informed of failure to assist asset. They have removed a number of your credits as a penalty to help pay target insurance.")
        VS.terminateMission(0)
    def NoEnemiesInArea (self,jp):
        if (self.adjsys.DestinationSystem()!=VS.getSystemFile()):
            return 0
        if (self.ship_check_count>=len(self.attackers)):
            VS.setCompleteness(self.objective,1.0)
            return 1

        un= self.attackers[self.ship_check_count]
        self.ship_check_count+=1
        if (un.isNull() or (un.GetHullPercent()<.7 and self.defendee.getDistance(un)>7000)):
            return 0
        else:
            VS.setObjective(self.objective,"Destroy the %s"%unit.getUnitFullName(un))
            self.ship_check_count=0
        return 0
        
    def GenerateEnemies (self,jp,you):
        VS.IOmessage (0,"escort mission",self.mplay,"You must protect %s." % unit.getUnitFullName(jp,True))
        count=0
        jp.setMissionRelevant()
        VS.addObjective ("Protect %s from the %s" % (unit.getUnitFullName(jp),self.faction.capitalize().replace("_"," ")))
        self.objective = VS.addObjective ("Destroy All %s Hostiles" % self.faction)
        VS.setCompleteness(self.objective,0.0)
        print "quantity "+str(self.quantity)
        while (count<self.quantity):
            L = launch.Launch()
            L.fg="Shadow";L.dynfg=self.dynatkfg;
            if (self.dynatktype==''):
                L.type=faction_ships.getRandomFighter(self.faction)
            else:
                L.type=self.dynatktype
            L.ai="default";L.num=1;L.minradius=2000.0;L.maxradius=4500.0
            try:
                L.minradius*=faction_ships.launch_distance_factor
                L.maxradius*=faction_ships.launch_distance_factor
            except:
                pass
            L.faction=self.faction
            launched=L.launch(you)
            if (count==0):              
                self.you.SetTarget(launched)            
            if (self.defend):
                launched.SetTarget (jp)
            else:
                launched.SetTarget (you)
            if (self.dedicatedattack):
                launched.setFgDirective('B')
            self.attackers += [ launched ]
            count+=1
        if (self.respawn==0 and len(self.attackers)>0):
            self.respawn=1
            import universe
            universe.greet(self.greetingText,self.attackers[0],you);
        else:
            VS.IOmessage (0,"escort mission",self.mplay,"Eliminate all %s ships here" % self.faction)

        self.quantity=0
    def Execute (self):
        if (self.you.isNull() or (self.arrived and self.defendee.isNull())):
            VS.IOmessage (0,"defend",self.mplay,"#ff0000You were unable to arrive in time to help. Mission failed.")
            self.SetVarValue(-1)
            VS.terminateMission(0)
            return   
        if (not self.adjsys.Execute()):
            return
        if (not self.arrived):
            self.arrived=1
            tempfaction=''
            if (self.defend_base):
                tempfaction=self.protectivefaction
                if (tempfaction==''):
                    tempfaction = faction_ships.get_enemy_of(self.faction)
            self.adjsys=go_somewhere_significant (self.you,self.defend_base or self.defend,self.distance_from_base,self.defend,tempfaction,self.dyndeffg,1,not self.defend_base)
            self.adjsys.Print ("You must visit the %s","defend","near the %s", 0)
            self.defendee=self.adjsys.SignificantUnit()
        else:
            if (self.defendee.isNull ()):
                if (self.defend):
                    self.FailMission(you)
                else:
                    self.SuccessMission()
                    return
            else:
                if (self.quantity>0):
                    self.GenerateEnemies (self.defendee,self.you)
                if (self.ship_check_count==0 and self.dedicatedattack):
                    if (self.targetiter>=len(self.attackers)):
                        self.targetiter=0
                    else:
                        un =  self.attackers[self.targetiter]
                        if (not un.isNull()):
                            if (self.defend):#            if (not un.isNull()
                                if (un.getName() in faction_ships.isBomber):
                                    if (self.quantity >= faction_ships.isBomber[un.getName()]):
                                        un.SetTarget (self.defendee)
                                    else:
                                        un.setFgDirective('b')
                                else:
                                    un.setFgDirective('b')
                            else:
                                un.SetTarget (self.you)
                        self.targetiter=self.targetiter+1
                if (self.NoEnemiesInArea (self.defendee)):
                    if (self.waves>0):
                        self.quantity=self.savedquantity
                        self.waves-=1
                    else:
                        self.SuccessMission()
    def initbriefing(self):
        print "ending briefing"                
    def loopbriefing(self):
        print "loop briefing"
        Briefing.terminate();
    def endbriefing(self):
        print "ending briefing"        
                
def initrandom(factionname,numsysaway,minenquant,maxenquant,credperen,defendit,defend_base,p_faction='',jumps=(),var_to_set=''):
    enq=minenquant
    enq=vsrandom.uniform(minenquant,maxenquant) 
    return defend(factionname,numsysaway,enq,8000.0,100000.0,enq*credperen,defendit,defend_base,p_faction,jumps,var_to_set)
