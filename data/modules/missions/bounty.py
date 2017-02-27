# -*- coding: utf-8 -*-
import universe
from go_to_adjacent_systems import go_to_adjacent_systems
from go_somewhere_significant import go_somewhere_significant
import vsrandom
import Vector
import launch
import faction_ships
import Director
import Briefing
import unit
import VS
import quest

class bounty (Director.Mission):
  def SetVar (self,val):
    if (self.var_to_set!=''):
      quest.removeQuest (self.you.isPlayerStarship(),self.var_to_set,val)

  def __init__ (self,minnumsystemsaway, maxnumsystemsaway, creds, run_away, shipdifficulty, tempfaction,jumps=(),var_to_set='',dynfg='',dyntype="",displayLocation=1,greetingText=['It appears we have something in common, privateer.','My name may be on your list, but now your name is on mine.'],dockable_unit=False):
    Director.Mission.__init__ (self)
    self.firsttime=VS.GetGameTime()
    self.newship=dyntype
    self.dynfg=dynfg        
    self.mplay="all"
    self.var_to_set = var_to_set
    self.istarget=0
    self.obj=0
    self.curiter=0
    self.arrived=0
    self.faction = tempfaction      
    self.difficulty = shipdifficulty
    self.runaway=run_away
    self.greetingText=greetingText
    self.cred=creds
    mysys=VS.getSystemFile()
    sysfile = VS.getSystemFile()
    self.you=VS.getPlayer()
    self.enemy=VS.Unit()
    self.adjsys=go_to_adjacent_systems (self.you,vsrandom.randrange(minnumsystemsaway,maxnumsystemsaway+1),jumps)
    self.dockable_unit=dockable_unit
    self.mplay=universe.getMessagePlayer(self.you)
    self.displayLocation=displayLocation
    if (self.you):
      VS.IOmessage (0,"bounty mission",self.mplay,"[Computer] Bounty Mission Objective: (%.2f Credits)" % self.cred)
      self.adjsys.Print("From %s system","Procede to %s","Search for target at %s, your final destination","bounty mission",1)
      VS.IOmessage (1,"bounty mission",self.mplay,"Target is a %s unit." % (self.faction))
    else:
      print "aboritng bounty constructor..."
      VS.terminateMission (0)

  def AdjLocation(self):
    print "ADJUSTING LOC"
    self.enemy.SetPosition(Vector.Add(self.enemy.LocalPosition(),Vector.Scale(self.enemy.GetVelocity(),-40))) #eta 20 sec

  def Win (self,un,terminate):
    self.SetVar(1)
    VS.IOmessage (0,"bounty mission",self.mplay,"[Computer] #00ff00Bounty Mission Accomplished!")
    un.addCredits(self.cred)
    if (terminate):
      print "you win bounty mission!"
      VS.terminateMission(1)
        
  def Lose (self,terminate):
    VS.IOmessage(0,"bounty mission",self.mplay,"[Computer] #ff0000Bounty Mission Failed.")
    self.SetVar(-1)
    if (terminate):
      print "lose bounty mission"
      VS.terminateMission(0)

  def LaunchedEnemies(self,significant):
    pass

  def Execute (self):
    isSig=0
    if (self.you.isNull()):
      self.Lose (1)
      return
    if (self.arrived==2):
      if (not self.runaway):
        if (not self.istarget):
          if (self.enemy):
            curun=VS.getUnit(self.curiter)
            self.curiter+=1
            if (curun==self.enemy):
              self.enemy.SetTarget(self.you)
            elif (curun.isNull()):
              self.curiter=0
      else:
        if (VS.GetGameTime()>self.firsttime+2.5 and self.enemy):
            self.firsttime+=1000000
            self.AdjLocation()
      if (self.enemy.isNull()):
        self.Win(self.you,1)
        return
    elif (self.arrived==1):
      significant=self.adjsys.SignificantUnit()
      if (significant.isNull ()):
        print "sig null"
        VS.terminateMission(0)
        return
      else:
        if (self.you.getSignificantDistance(significant)<self.adjsys.distfrombase):
          if (self.newship==""):
            self.newship=faction_ships.getRandomFighter(self.faction)
          #self.enemy=launch.launch_wave_around_unit("Shadow",self.faction,self.newship,"default",1+self.difficulty,3000.0,4000.0,significant)
          L = launch.Launch()
          L.fg="Shadow"
          L.dynfg=self.dynfg
          L.type = self.newship
          L.faction = self.faction
          L.ai = "default"
          L.num=1+self.difficulty
          L.minradius=3000.0
          L.maxradius = 4000.0
          try:
            L.minradius*=faction_ships.launch_distance_factor
            L.maxradius*=faction_ships.launch_distance_factor
          except:
            pass
          self.enemy=L.launch(significant)
          self.enemy.SetTarget(self.you)
          self.enemy.setFgDirective('B.')
          self.you.SetTarget(self.enemy)
          universe.greet(self.greetingText,self.enemy,self.you)
          str_faction = self.faction.split('_')[0]
          quest.playSoundCockpit('sounds/missions/' + str_faction + '_it-appears-we-have.ogg')
          self.obj=VS.addObjective("Destroy %s." % (unit.getUnitFullName(self.enemy)))
          if (self.enemy):
            if (self.runaway):
              self.enemy.SetTarget(significant) #CHANGE TO SetTarget ==>NOT setTarget<==
              self.enemy.ActivateJumpDrive(0)
              self.firsttime=VS.GetGameTime()
              #self.enemy.SetPosAndCumPos(Vector.Add(self.you.Position(),Vector.Vector(0,0,self.you.rSize()*1.2)))
            self.LaunchedEnemies(significant)
            self.arrived=2
          else:
            print "enemy null"
            VS.terminateMission(0)
            return
    else:
      if (self.adjsys.Execute()):
        self.arrived=1
        if (self.newship=="" and self.dynfg==''):
          self.newship=faction_ships.getRandomFighter(self.faction)
        self.adjsys=go_somewhere_significant(self.you,self.dockable_unit,10000.0,0,'','',self.displayLocation)
        if not self.displayLocation:
          VS.addObjective("Search/Destroy "+self.faction.capitalize()+" mark");
        localdestination=unit.getUnitFullName(self.adjsys.SignificantUnit(),True)
        tmpfg=self.dynfg
        if len(tmpfg)==0:
          tmpfg="shadow"
        VS.IOmessage (3,"bounty mission",self.mplay,"Hunt the %s unit in the %s flightgroup in this system." % (self.newship,tmpfg))
        if (self.runaway):        #ADD OTHER JUMPING IF STATEMENT CODE HERE
          VS.IOmessage (4,"bounty mission",self.mplay,"Target is fleeing to the jump point!")
          VS.IOmessage (5,"bounty mission",self.mplay,"Target Destination appears to be %s" % (localdestination))
        elif (self.displayLocation):
          VS.IOmessage (4,"bounty mission",self.mplay,"Scanners detect bounty target!")
          VS.IOmessage (5,"bounty mission",self.mplay,"Coordinates appear near %s" % (localdestination))
        else:            
          print "Location "+str(self.displayLocation)
          VS.IOmessage (4,"bounty mission",self.mplay,"[Computer] Mission description indicates bounty target may be in this system.")

  def initbriefing(self):
    print "ending briefing"                

  def loopbriefing(self):
    print "loop briefing"
    Briefing.terminate();

  def endbriefing(self):
    print "ending briefing"           

def initrandom (minns, maxns, credsmin, credsmax, run_away, minshipdifficulty, maxshipdifficulty,jumps=(),var_to_set=''):
  you=VS.getPlayer()
  tempfaction='aera'
  if (you):
    name = you.getFactionName ()
    factionname=vsrandom.randrange(0,faction_ships.getMaxFactions())
    tempfaction=faction_ships.intToFaction(factionname)
    i=0
    while ((name==tempfaction or name=="unknown") and i<10):
      factionname=vsrandom.randrange(0,faction_ships.getMaxFactions())
      tempfaction=faction_ships.intToFaction(factionname)
      i+=1
    sd = vsrandom.random()*(maxshipdifficulty-minshipdifficulty)+minshipdifficulty
    return bounty (minns,maxns,(1.0+(sd*0.5))*(vsrandom.random ()*(credsmax-credsmin)+credsmin),run_away,sd,tempfaction,jumps,var_to_set)
  else:
    print "aborting bounty initrandom"
    VS.terminateMission(0)
