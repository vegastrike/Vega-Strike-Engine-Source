#---------------------------------------------------------------------------------
# Vega Strike script for the tutorial quest
# Copyright (C) 2008 Vega Strike team
# Contact: hellcatv@users.sourceforge.net
# Internet: http://vegastrike.sourceforge.net/
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# Author: pyramid
# Version: 2008-12-22
#
#---------------------------------------------------------------------------------

# This script is a start to a turorial mission (quest/adventure)
# Currently, there is a small navigation tutorial available
# You can decline the offer after Oswald says hello by simply not stopping your ship
# It is possible to break off the tutorial. The last completed stage is saved
# in your savegame and resumes from there.

# things that can be improved
# (a) include other tutorial parts: targetting, weapons, ...
# (b) change the mission name that appears on the hud - not possible currently
# (c) make tutorial repeatable after some time
# (d) there should be a negative relation adjustment when oswald gets destroyed
# (e) fix mission name and delete mission targets after each stage

# import used libraries
import quest
import Director
import VS
import Vector
import universe
import unit
import vsrandom
import launch
import news

# predefine stages
SAVE_KEY = "quest_tutorial"
STAGE_DOCKED = 0
STAGE_AWAY = 1
STAGE_ORBIT = 2
STAGE_ACCEPT = 4
COMPLETE_TUTORIAL1 = 5
COMPLETE_TUTORIAL2 = 6
COMPLETE_TUTORIAL3 = 7
COMPLETE_TUTORIAL4 = 8
COMPLETE_TUTORIAL5 = 10
STAGE_DECLINE = 98

# the class that will be executed
class quest_tutorial (quest.quest):
  # initialize quest variables
  def __init__ (self):
    # initialize variables
    self.stage = STAGE_DOCKED        # tutorial stage
    self.practice = 0                #sequence within a practice function
    self.system = VS.getSystemName() # starting system
    self.player = VS.getPlayer()
    self.playerhull = VS.getPlayer().GetHull()
    self.drone = VS.Unit()
    self.droneshield = 1              # shield before fight
    self.fight = 0                    # how many fight rounds
    self.dockeddistance = 0
    self.timer = VS.GetGameTime()     # controls the stage timing
    self.talktime = VS.GetGameTime()  #controls individual talk time within animation
    self.anitime = VS.GetGameTime()   # controls the animation time
    self.stayputtime = 0
    self.msgColor = "#FFFF99"
    self.objectives = []              # list of objectives
    self.objective = 0                # current objective
    self.startobjectname = ""
    self.animations = [["com_tutorial_oswald.ani",2]] # the actors and ani duration
    self.speech = []                  # has a list of current sound files for speech
    self.newsplayed = 0

  def putSaveValue(self,value, key=SAVE_KEY):
    Director.eraseSaveData(self.player.isPlayerStarship(),key,0)
    Director.pushSaveData(self.player.isPlayerStarship(),key,value)
    return 1

  def getSaveValue(self,key=SAVE_KEY):
    if Director.getSaveDataLength(self.player.isPlayerStarship(),key) > 0:
      #print "----getSaveValue---", int(Director.getSaveData(self.player.isPlayerStarship(),key,0))
      return int(Director.getSaveData(self.player.isPlayerStarship(),key,0))
    return 0

  def playSoundCockpit(self,soundfile):
    try:
      VS.playSoundCockpit(soundfile)
    except AttributeError:
      VS.playSound(soundfile, self.player.Position(), (0,0,0))

  # checks if the player has undocked from Atlantis. If so sets next stage.
  # Has been replaced by the more generic function playerIsUndocked
  def hasUndockedFromAtlantis(self):
    # get the planet object 
    self.startobject = unit.getUnitByName('Atlantis')
    # target the departing planet to see the distance
    self.player.SetTarget(self.startobject)
    # verify if player is still docked at the planet
    if (self.startobject.isDocked(self.player)):
        self.dockeddistance = self.startobject.getDistance(self.player)
    # if the player is not docked and at least 5km away then set next stage number
    if (not self.startobject.isDocked(self.player) and self.startobject.getDistance(self.player)>(self.dockeddistance+5000)):
        self.stage = STAGE_AWAY
    return 0

  # checks if the player has undocked. if so sets next stage
  def playerIsUndocked(self):
    # dockedobject and dockeddistance must be global, i.e prefixed with self
    # otherwise the script will advance to next stage just after undocking
    # get the planet object 
    self.dockedobject = universe.getDockedBase()
    name = self.dockedobject.getName()
    # verify if player is still docked and set the reference distance
    if (not name==""):
      self.dockeddistance = self.dockedobject.getDistance(self.player)
      self.startobjectname = name
    # when starting from Atlantis, target the departing planet to see the distance
    if (name=="Atlantis"):
      self.player.SetTarget(self.dockedobject)
    # if the player was never docked or is not docked and at least 1km away then set next stage number
    if (name=="" or ((not self.startobjectname=="") and (not self.startobjectname=="Atlantis") and self.dockedobject.getDistance(self.player)>(self.dockeddistance+1000))):
      self.stage = STAGE_AWAY
      self.timer = VS.GetGameTime()+5
    return 0

  def LightMinuteToMeter(self,lightminute):
    meter = 17987547500 * lightminute
    return meter

  # launches a unit aka the drone
  def launchNewDrone (self):
    if (not self.player.isNull()):
      # get the player's position
      vec = self.player.Position()
      # set drone position 1000m away from the player
      vec = Vector.Add(vec,(1000,0,0))
      # launch the tutorial drone.
      #VS.launch(name,type,faction,unittype,ai,nr,nrwaves,pos,squadlogo):
      self.drone = VS.launch("Oswald","Robin.tutorial","klkk_citizen","unit","default",1,1,vec,'')
      # upgrade drone
      self.drone.upgrade("quadshield15",0,0,1,0)
      self.drone.upgrade("armor06",0,0,1,0)
      # when launching give the player some text and ask him to decide if he wants to participate
      VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Hello traveler.")
      VS.IOmessage (3,"Oswald","Privateer",self.msgColor+"My name is Oswald and I am offering flight assistance.")
      VS.IOmessage (8,"Oswald","Privateer",self.msgColor+"Would you like to refresh your space faring skills?")
      VS.IOmessage (12,"Oswald","Privateer",self.msgColor+"To participate in my tutorial mission, cut your engines with the #9999FFBACKSPACE"+self.msgColor+" key, let me approach you, and stay put until I contact you again.")
      #VS.playSound("com_tutorial_oswald/oswald_greet.ogg", self.player.Position(), (0,0,0))
      #VS.musicPlaySong("com_tutorial_oswald/oswald_greet.ogg")
      # set comm animation parameters
      # list [start,duration,anilist_entry]
      self.sequence = [[0,1,0],[3,3,0],[8,2,0],[12,7,0]]
      # set speech files
      self.speech = ["com_tutorial_oswald/oswald_greet_01.ogg", "com_tutorial_oswald/oswald_greet_02.ogg", "com_tutorial_oswald/oswald_greet_03.ogg", "com_tutorial_oswald/oswald_greet_04.ogg"]
      #set talktime start
      self.talktime = VS.GetGameTime()
      # on launching the drone, set its position near the player
      vec = self.player.Position()
      vec = Vector.Add (vec,(vsrandom.uniform(-2000,2000),
                              vsrandom.uniform(-2000,2000),
                              vsrandom.uniform(-2000,2000)))
      self.drone.SetPosition(vec)
      # get rid of all orders, so that strange maneouvers don't happen
      self.drone.PrimeOrders()
      # display the drone on HUD
      self.player.SetTarget(self.drone)
      # remember the initial shield strength
      self.droneshield = unit.getShieldPercent(self.drone)
      self.fight = 0
      self.stage += 1
      #self.stage = COMPLETE_TUTORIAL3 # debug
      # duration of this part until end of animation
      self.timer = VS.GetGameTime() + 20
    return 0

  # check if drone has been attacked
  # if so branch off to attack lesson
  def checkDroneHealth (self):
    if (unit.getShieldPercent(self.drone)<self.droneshield*0.90):
      self.drone.PrimeOrders()
      self.drone.SetTarget(self.player)
      self.drone.LoadAIScript("fighter.ace")
      self.drone.setFgDirective("a.")
      self.savestage = self.stage
      self.practice = 0
      #if (self.fight<2):
      self.fight += 1
      self.stage = 96
    return 0

  # checks distance to drone
  # if player is too far, the tutorial breaks off
  def checkDistance (self):
    if (self.drone.getDistance(self.player)>20000):
      self.practice = 0
      self.stage = 97
    return 0

  # keeps the drone near the player
  # the drone doesn't quite orbit
  # it will approach the player until 1000 meters and then stop
  def orbitMe (self):
    #self.player.SetTarget(self.drone)
    # if the drone is more than 1000m away it will start instructions
    if (self.drone.getDistance(self.player)>=1100):
      # orientate the nose of the drone towards the player ship
      vec = Vector.Sub(self.player.Position(),self.drone.Position())
      self.drone.SetOrientation((1,0,0),vec)
      # set velocity proportional to distance from player
      vec = Vector.Scale(Vector.SafeNorm(vec),self.drone.getDistance(self.player)/10)
      self.drone.SetVelocity(vec)
      #self.stayputtime = VS.GetGameTime()
    # if drone has approached player until 1000m then stop it
    if (self.drone.getDistance(self.player)<1100):
      self.drone.SetVelocity((0,0,0))
      # this is also needed to stop rotation of the drone
      self.drone.SetAngularVelocity((0,0,0))
    return 0

  # check if player stays put close to the drone to accept tutorial
  def acceptTutorial (self):
    velocity = Vector.Mag(self.player.GetVelocity())
    # if the offer has been placed, and player is put for 10s and drone is near
    if (VS.GetGameTime()>self.timer and self.drone.getDistance(self.player)<1100 and velocity<=10):
      self.player.SetTarget(self.drone)
      self.timer = VS.GetGameTime()
      self.complete = self.getSaveValue()
      if (self.complete>0):
        self.stage = self.complete
      else:
        self.stage = STAGE_ACCEPT
    if (VS.GetGameTime()>self.timer and velocity>=10):
      # allow some time for slowing down and approach if ships were moving
      if (self.stayputtime>60):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Have a nice journey and come back for a space faring refresher anytime here in Cephid 17.")
        self.speech = ["com_tutorial_oswald/oswald_bye.ogg"]
        self.playSoundCockpit(self.speech[0])
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.stage = STAGE_DECLINE
        self.timer = VS.GetGameTime()
      else:
        self.stayputtime += 10
        self.timer = VS.GetGameTime() + 10
    return 0

  # play the first part of the tutorial
  def tutorialComm (self):
    if (self.practice==0):
      VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Glad I can be of help.")
      VS.IOmessage (5,"Oswald","Privateer",self.msgColor+"In the first place let's have a look at your heads up display (HUD).")
      VS.IOmessage (10,"Oswald","Privateer",self.msgColor+"Please do not move your ship in order to better focus on my instructions.")
      VS.IOmessage (15,"Oswald","Privateer",self.msgColor+"In the upper left corner you can see the communication messages.")
      VS.IOmessage (20,"Oswald","Privateer",self.msgColor+"Each communication message shows the sender, the game time of the sending, and the message itself, like this one.")
      VS.IOmessage (25,"Oswald","Privateer",self.msgColor+"To scroll the messages back and forth use the #9999FFPage Up"+self.msgColor+" and #9999FFPage Down"+self.msgColor+" keys. Try it out now.")
      VS.IOmessage (35,"Oswald","Privateer",self.msgColor+"Good.")
      VS.IOmessage (37,"Oswald","Privateer",self.msgColor+"Now you can send me a message by pressing the #9999FFF1"+self.msgColor+" key.")
      # set comm animation parameters
      self.sequence = [[0,1,0],[5,4,0],[10,4,0],[15,4,0],[20,4,0],[25,5,0],[35,1,0],[37,4,0]]
      # set speech files
      self.speech = ["com_tutorial_oswald/oswald_comm_01.ogg", "com_tutorial_oswald/oswald_comm_02.ogg", "com_tutorial_oswald/oswald_comm_03.ogg", "com_tutorial_oswald/oswald_comm_04.ogg", "com_tutorial_oswald/oswald_comm_05.ogg", "com_tutorial_oswald/oswald_comm_06.ogg", "com_tutorial_oswald/oswald_comm_07.ogg", "com_tutorial_oswald/oswald_comm_08.ogg"]
      self.talktime = VS.GetGameTime()
      self.timer = VS.GetGameTime()+45
      self.practice = 1
    if (self.practice==1 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
      for index in range (len(self.sequence)):
        if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
          self.player.commAnimation(self.animations[self.sequence[index][2]][0])
          # play each soundfile once
          if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
            self.playSoundCockpit(self.speech[index])
          self.anitime = VS.GetGameTime()+2
    if (self.practice==1 and VS.GetGameTime()>=self.timer):
      self.practice = 2
    if (self.practice==2 and VS.GetGameTime()>self.timer):
      VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Usually, messages assigned to keys #9999FFF1"+self.msgColor+" and #9999FFF2"+self.msgColor+" are friendly messages which slightly increase you relation with a faction, while the other keys #9999FFF3"+self.msgColor+" and #9999FFF4"+self.msgColor+" decrease your relationship.")
      VS.IOmessage (14,"Oswald","Privateer",self.msgColor+"Sometimes it can be very useful to send multiple friendly messages to improve your relation with a hostile faction or taunt an enemy into attacking you instead of a ship you are protecting.")
      VS.IOmessage (26,"Oswald","Privateer",self.msgColor+"That's about it on the messages display.")
      # set comm animation parameters
      self.sequence = [[0,12,0],[14,12,0],[26,2,0]]
      # set speech files
      self.speech = ["com_tutorial_oswald/oswald_comm_09.ogg", "com_tutorial_oswald/oswald_comm_10.ogg", "com_tutorial_oswald/oswald_comm_11.ogg"]
      self.talktime = VS.GetGameTime()
      self.timer = VS.GetGameTime()+30
      self.practice = 3
    if (self.practice==3 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
      for index in range (len(self.sequence)):
        if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
          self.player.commAnimation(self.animations[self.sequence[index][2]][0])
          # play each soundfile once
          if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
            self.playSoundCockpit(self.speech[index])
          self.anitime = VS.GetGameTime()+2
    if (self.practice==3 and VS.GetGameTime()>=self.timer):
      self.practice = 4
    if (self.practice==4 and VS.GetGameTime()>self.timer):
      # make sure to reset the counter for the next practice loops
      self.practice = 0
      self.timer = VS.GetGameTime()+0
      self.stage = COMPLETE_TUTORIAL1
      self.putSaveValue(self.stage)
    return 0

  def tutorialNav (self):
    if (self.practice==0):
      VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Now, let's review the navigation information on your HUD. We'll do theory first, then some practice.")
      VS.IOmessage (9,"Oswald","Privateer",self.msgColor+"In the lower left corner you will find your ship's shield status in blue, and armor status in orange.")
      VS.IOmessage (16,"Oswald","Privateer",self.msgColor+"We will come to the text indicators later.")
      VS.IOmessage (20,"Oswald","Privateer",self.msgColor+"In the middle of the bottom part you have your dashboard with the front radar on the left side and the rear radar on the right side.")
      VS.IOmessage (29,"Oswald","Privateer",self.msgColor+"The active target will display as a small cross on your radar. Other targets will be dots with their colors representing your relation to them.")
      VS.IOmessage (38,"Oswald","Privateer",self.msgColor+"Green is friendly, red is hostile, and yellow is neutral. An attacking ship is blue, a targetting ship is light blue, and a locking ship is violet. Neutral and significant objects like planets, stations, wormholes, or suns are white.")
      VS.IOmessage (53,"Oswald","Privateer",self.msgColor+"The top center part of the dashboard has four round indicators which begin flashing when the following events occur:")
      VS.IOmessage (59,"Oswald","Privateer",self.msgColor+" (L) means a hostile has missile lock on you")
      VS.IOmessage (63,"Oswald","Privateer",self.msgColor+" (J) means you are in range of a jump point and your jump drive is ready")
      VS.IOmessage (67,"Oswald","Privateer",self.msgColor+" (S) means your SPEC drive, needed for faster-than-light (FTL) travel, is activated")
      VS.IOmessage (73,"Oswald","Privateer",self.msgColor+" (E) means your electronic counter measures (ECM) are active")
      VS.IOmessage (77,"Oswald","Privateer",self.msgColor+"Below the round indicators are three colored bars")
      VS.IOmessage (81,"Oswald","Privateer",self.msgColor+" (CAPACITOR) shows your weapons capacitor charge")
      VS.IOmessage (86,"Oswald","Privateer",self.msgColor+" (DRIVES) shows your SPEC and jump drives energy charge")
      VS.IOmessage (91,"Oswald","Privateer",self.msgColor+" (FUEL) shows you capacity for in-system travel and overdrive propulsion")
      VS.IOmessage (97,"Oswald","Privateer",self.msgColor+"The numbers below the bars are your current speed to the left and your set speed to the right.")
      VS.IOmessage (105,"Oswald","Privateer",self.msgColor+"Below that is the effective SPEC velocity to the left and the flight computer (FCMP) mode to the right.")
      # set comm animation parameters                                                #8                                            #13
      self.sequence = [[0,8,0],[9,6,0],[16,2,0],[20,8,0],[29,8,0],[38,14,0],[53,6,0],[59,3,0],[63,4,0],[67,6,0],[73,4,0],[77,4,0],[81,4,0],[86,4,0],[91,5,0],[97,6,0],[105,6,0]]
      self.speech = ["com_tutorial_oswald/oswald_nav_01.ogg", "com_tutorial_oswald/oswald_nav_02.ogg", "com_tutorial_oswald/oswald_nav_03.ogg", "com_tutorial_oswald/oswald_nav_04.ogg", "com_tutorial_oswald/oswald_nav_05.ogg", "com_tutorial_oswald/oswald_nav_06.ogg", "com_tutorial_oswald/oswald_nav_07.ogg", "com_tutorial_oswald/oswald_nav_08.ogg", "com_tutorial_oswald/oswald_nav_09.ogg", "com_tutorial_oswald/oswald_nav_10.ogg", "com_tutorial_oswald/oswald_nav_11.ogg", "com_tutorial_oswald/oswald_nav_12.ogg", "com_tutorial_oswald/oswald_nav_13.ogg", "com_tutorial_oswald/oswald_nav_14.ogg", "com_tutorial_oswald/oswald_nav_15.ogg", "com_tutorial_oswald/oswald_nav_16.ogg", "com_tutorial_oswald/oswald_nav_17.ogg"]
      self.talktime = VS.GetGameTime()
      self.timer = VS.GetGameTime()+112
      self.practice = 1
    if (self.practice==1 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
      for index in range (len(self.sequence)):
        if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
          self.player.commAnimation(self.animations[self.sequence[index][2]][0])
          # play each soundfile once
          if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
            self.playSoundCockpit(self.speech[index])
          self.anitime = VS.GetGameTime()+2
    if (self.practice==1 and VS.GetGameTime()>=self.timer):
      self.practice = 2
    if (self.practice==2 and VS.GetGameTime()>self.timer):
      # make sure to reset the counter for the next practice loops
      self.practice = 0
      self.timer = VS.GetGameTime()+0
      self.stage = COMPLETE_TUTORIAL2
      self.putSaveValue(self.stage)
    return 0

  def practiceNav (self):
      # practice intro
      if (self.practice==0):
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"So much for theory. Let's do some practice now.")
          VS.IOmessage (4,"Oswald","Privateer",self.msgColor+"First we'll do some basic navigation and targetting.")
          self.sequence = [[0,3,0],[4,3,0]]
          self.speech = ["com_tutorial_oswald/oswald_target_01.ogg", "com_tutorial_oswald/oswald_target_02.ogg"]
          self.talktime = VS.GetGameTime()
          self.timer = VS.GetGameTime()+10
          self.practice = 1
      if (self.practice==1 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==1 and VS.GetGameTime()>=self.timer):
          self.practice = 3
      if (self.practice==3):
        # explain basic targetting if drone is not already target
        if (unit.getUnitFullName(self.player.GetTarget()) != unit.getUnitFullName(self.drone)):
          nam = unit.getUnitFullName(self.drone)
          self.objective = VS.addObjective("Target %s" % nam)
          self.objectives+=[int(self.objective)]
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"In the lower right corner you can see the target video display unit (VDU) where you can see your current target.")
          VS.IOmessage (8,"Oswald","Privateer",self.msgColor+"Target me by repeatedly toggling the #9999FFT"+self.msgColor+" key until you see my ship on the right VDU.")
          VS.IOmessage (16,"Oswald","Privateer",self.msgColor+"Should you pass me, you may reverse the target selection sequence by pressing the #9999FFShift+T"+self.msgColor+" keys.")
          # set comm animation parameters
          self.sequence = [[0,7,0],[8,7,0],[16,6,0]]
          self.speech = ["com_tutorial_oswald/oswald_target_03.ogg", "com_tutorial_oswald/oswald_target_04.ogg", "com_tutorial_oswald/oswald_target_05.ogg"]
          self.talktime = VS.GetGameTime()
          self.timer = VS.GetGameTime()+23
          self.practice = 4
        else:
          self.practice = 5
      if (self.practice==4 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==4 and VS.GetGameTime()>=self.timer):
          self.practice = 5
      # make the drone the players target
      if (self.practice==5 and unit.getUnitFullName(self.player.GetTarget())==unit.getUnitFullName(self.drone)):
        nam = unit.getUnitFullName(self.drone)
        self.objective = VS.addObjective("Face %s" % nam)
        self.objectives+=[int(self.objective)]
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"OK. Now, orient your ship so that your targetting reticule (cross) points directly at me.")
        VS.IOmessage (7,"Oswald","Privateer",self.msgColor+"To get my ship into your visual range just turn in the direction of the white target arrow at the edge of your HUD.")
        VS.IOmessage (14,"Oswald","Privateer",self.msgColor+"When my ship is in your visual range you will notice that it is framed by a target box.")
        VS.IOmessage (20,"Oswald","Privateer",self.msgColor+"Align your targetting reticule with my ship.")
        # set comm animation parameters
        self.sequence = [[0,6,0],[7,6,0],[14,5,0],[20,3,0]]
        self.speech = ["com_tutorial_oswald/oswald_target_06.ogg", "com_tutorial_oswald/oswald_target_07.ogg", "com_tutorial_oswald/oswald_target_08.ogg", "com_tutorial_oswald/oswald_target_09.ogg"]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+24
        self.practice = 6
      if (self.practice==6 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==6 and VS.GetGameTime()>=self.timer):
        self.practice = 7
      if (self.practice==7):
        # check if the player is facing the drone
        angle = unit.facingAngleToUnit(self.player,self.drone)
        #print "facing: " + str(angle)
        if (angle<=0.05):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Well done.")
          VS.IOmessage (2,"Oswald","Privateer",self.msgColor+"Now, turn your ship away from my ship and accelerate to full speed using the #9999FF\\"+self.msgColor+" key.")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.objective = VS.addObjective("Set max velocity")
          self.objectives+=[int(self.objective)]
          self.sequence = [[0,1,0],[2,6,0]]
          self.speech = ["com_tutorial_oswald/oswald_nav_21.ogg", "com_tutorial_oswald/oswald_nav_22.ogg"]
          self.talktime = VS.GetGameTime()
          self.timer = VS.GetGameTime()+9
          self.practice = 8
      if (self.practice==8 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==8 and VS.GetGameTime()>=self.timer):
        self.practice = 9
      if (self.practice==9):
        # check if the player is facing away
        angle = unit.facingAngleToUnit(self.player,self.drone)
        velocity = Vector.Mag(self.player.GetVelocity())
        #check if angle to drone is at least 22 degrees (0.20 radians)
        if (angle>=0.20 and velocity>=295):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"And now set your velocity reference to zero by pressing the #9999FFBACKSPACE"+self.msgColor+" key and come to a complete stop.")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.playSoundCockpit("com_tutorial_oswald/oswald_nav_23.ogg")
          self.objective = VS.addObjective("Set full stop")
          self.objectives+=[int(self.objective)]
          self.timer = VS.GetGameTime()+9
          self.practice = 10
      if (self.practice==10):
        # check if the player is stopped
        velocity = Vector.Mag(self.player.GetVelocity())
        if (velocity<=2):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"You can also increment your velocity gradually with the #9999FF+"+self.msgColor+" key. Accelerate to 100 m/s now.")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.playSoundCockpit("com_tutorial_oswald/oswald_nav_24.ogg")
          self.objective = VS.addObjective("Set velocity reference to 100m/s")
          self.objectives+=[int(self.objective)]
          self.timer = VS.GetGameTime()+9
          self.practice = 11
      if (self.practice==11):
        # check if the player has velocity >100
        velocity = Vector.Mag(self.player.GetVelocity())
        if (velocity>=98 and velocity<=110):
          VS.setCompleteness(self.objectives[3],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"In the same way you can reduce your velocity gradually with the #9999FF-"+self.msgColor+" key. Deccelerate to 50 m/s.")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.playSoundCockpit("com_tutorial_oswald/oswald_nav_25.ogg")
          self.objective = VS.addObjective("Set velocity reference to 50m/s")
          self.objectives+=[int(self.objective)]
          self.timer = VS.GetGameTime()+9
          self.practice = 12
      if (self.practice==12):
        # check if the player has velocity <50
        velocity = Vector.Mag(self.player.GetVelocity())
        if (velocity<=55 and velocity>=40):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Great. If you further deccelerate your velocity with the #9999FF-"+self.msgColor+" key you can actually reverse your thrust. Deccelerate now to -20 m/s.")
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.playSoundCockpit("com_tutorial_oswald/oswald_nav_26.ogg")
          self.objective = VS.addObjective("Set velocity reference to -20m/s")
          self.objectives+=[int(self.objective)]
          self.timer = VS.GetGameTime()+12
          self.practice = 13
      if (self.practice==13):
        # check if the player has velocity <=-20m/s
        velocity = unit.getSignedVelocity(self.player)
        if (velocity<=-18):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Excellent.")
          VS.IOmessage (2,"Oswald","Privateer",self.msgColor+"You have learned how to target, orient your ship, accellerate, decellerate, and bring your ship to a stop.")
          VS.IOmessage (10,"Oswald","Privateer",self.msgColor+"I'm sure this will come in handy during your future endeavors.")
          # set comm animation parameters
          self.sequence = [[0,1,0],[2,7,0],[10,4,0]]
          self.speech = ["com_tutorial_oswald/oswald_nav_27.ogg", "com_tutorial_oswald/oswald_nav_28.ogg", "com_tutorial_oswald/oswald_nav_29.ogg"]
          self.talktime = VS.GetGameTime()
          self.timer = VS.GetGameTime()+15
          self.practice = 14
      if (self.practice==14 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==14 and VS.GetGameTime()>=self.timer):
        self.practice = 15
      if (self.practice==15):
        # make sure to reset the counter for the next practice loops
        self.practice = 0
        self.timer = VS.GetGameTime()+10
        self.stage = COMPLETE_TUTORIAL3
        self.putSaveValue(self.stage)
        #print "NAV - save: " + str(self.getSaveValue())
      return 0

  def practiceSpec (self):
      # practice intro
      if (self.practice==0):
        #self.jump = universe.getRandomJumppoint()
        self.jump = unit.getUnitByName('JumpToOldziey')
        nam = unit.getUnitFullName(self.jump)
        self.objective = VS.addObjective("Target %s" % nam)
        self.objectives+=[int(self.objective)]
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Let's practice some faster than light (FTL) travel now.")
        VS.IOmessage (5,"Oswald","Privateer",self.msgColor+"Target "+unit.getUnitFullName(self.jump)+" navigation point using the #9999FFN"+self.msgColor+" or the #9999FFShift+N"+self.msgColor+" keys.")
        self.sequence = [[0,4,0],[5,6,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_01.ogg", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+12
        self.practice = 1
      if (self.practice==1 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==1 and VS.GetGameTime()>=self.timer):
        self.practice = 2
      if (self.practice==2 and self.player.GetTarget()==self.jump):
        VS.setCompleteness(self.objectives[self.objective],1.0)
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Set your velocity to maximum with the #9999FF\\"+self.msgColor+" key and activate the SPEC auto pilot with the #9999FFA"+self.msgColor+" key to get there. Hang on as this might take a while if you are close to massive objects.")
        VS.IOmessage (10,"Oswald","Privateer",self.msgColor+"You will notice that your SPEC drive indicator (S) is flashing, which indicates that SPEC is active.")
        VS.IOmessage (20,"Oswald","Privateer",self.msgColor+"During FTL travel your shields become inactive, as you can see below on your ship VDU.")
        VS.IOmessage (26,"Oswald","Privateer",self.msgColor+"You will also notice that your steering has no effect on your vessel since the auto pilot has taken over the controls.")
        VS.IOmessage (32,"Oswald","Privateer",self.msgColor+"You can always interrupt and resume the auto pilot by toggling the #9999FFA"+self.msgColor+" key. You may try that, if you wish.")
        VS.IOmessage (40,"Oswald","Privateer",self.msgColor+"In the lower left corner, just above your ship staus you will notice two indicators.")
        VS.IOmessage (45,"Oswald","Privateer",self.msgColor+"SPEC shows you if your SPEC drive is enabled.")
        VS.IOmessage (47,"Oswald","Privateer",self.msgColor+"AUTO tells you if auto pilot is engaged.")
        name = unit.getUnitFullName(self.jump)
        self.objective = VS.addObjective("Approach %s" % name)
        self.objectives+=[int(self.objective)]
        # set comm animation parameters
        self.sequence = [[0,12,0],[12,6,0],[20,6,0],[26,6,0],[32,7,0],[40,4,0],[45,2,0],[47,4,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_02.ogg", "", "com_tutorial_oswald/oswald_spec_03.ogg", "", "", "com_tutorial_oswald/oswald_spec_04.ogg", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+52
        self.practice = 3
      if (self.practice==3 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==3 and VS.GetGameTime()>=self.timer):
        self.practice = 4
      if (self.practice==4 and self.player.getDistance(self.jump)<=10000):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Almost there.")
        VS.IOmessage (2,"Oswald","Privateer",self.msgColor+"The auto pilot only gives back control some time after the SPEC auto pilot light stopped flashing.")
        VS.IOmessage (8,"Oswald","Privateer",self.msgColor+"Notice also how your shields start recharing when leaving FTL travel mode.")
        # set comm animation parameters
        self.sequence = [[0,1,0],[2,5,0],[8,4,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_05.ogg", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+14
        self.practice = 5
      if (self.practice==5 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==5 and VS.GetGameTime()>=self.timer):
        self.practice = 6
      if (self.practice==6 and self.player.getDistance(self.jump)<=3000):
        VS.setCompleteness(self.objectives[self.objective],1.0)
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Here we are.")
        VS.IOmessage (1,"Oswald","Privateer",self.msgColor+"You may try out manual FTL travel at this point.")
        VS.IOmessage (5,"Oswald","Privateer",self.msgColor+"Target the planet Atlantis using your significant objects targetting keys #9999FFN"+self.msgColor+" and #9999FFShift+N"+self.msgColor+".")
        # set comm animation parameters
        self.sequence = [[0,1,0],[1,4,0],[5,7,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_06.ogg", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+12
        self.practice += 1
      if (self.practice==7 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==7 and VS.GetGameTime()>=self.timer):
        self.practice = 8
      if (self.practice==8 and VS.GetGameTime()>self.timer):
        self.destination = unit.getUnitByName('Atlantis')
        self.distance = self.player.getDistance(self.destination)
        name = unit.getUnitFullName(self.destination)
        self.objective = VS.addObjective("Target %s" % name)
        self.objectives+=[int(self.objective)]
        self.timer = VS.GetGameTime()
        self.practice += 1
      if (self.practice==9 and self.player.GetTarget()==self.destination):
        VS.setCompleteness(self.objectives[self.objective],1.0)
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Roger that. Turn towards the planet, set your velocity to maximum with the #9999FF\\"+self.msgColor+" key, and enable the manual SPEC with the #9999FFShift+A"+self.msgColor+" key to approach the planet.")
        VS.IOmessage (12,"Oswald","Privateer",self.msgColor+"Make sure that the planet is fairly well centered in your targetting reticule.")
        VS.IOmessage (17,"Oswald","Privateer",self.msgColor+"Notice how your speed starts increasing gradually after leaving the jump point range.")
        self.objective = VS.addObjective("Enable manual SPEC")
        self.objectives+=[int(self.objective)]
        # set comm animation parameters
        self.sequence = [[0,12,0],[12,5,0],[17,5,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_07.ogg", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+22
        self.practice += 1
      if (self.practice==10 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==10 and VS.GetGameTime()>=self.timer):
          self.practice = 11
      if (self.practice==11 and self.player.GetTarget()==self.destination):
          #disabled for now, since max velocity does not return the spec values
          #velocity = Vector.Mag(self.player.GetVelocity())
          #print "velocity=" + str(self.player.GetVelocity())
          #print "magnitude=" + str(velocity)
          #if (velocity>=5000):
          if (self.player.getDistance(self.destination)<=(self.distance*0.97)):
            VS.setCompleteness(self.objectives[self.objective],1.0)
            VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"If your are getting too much off course, stop the SPEC drive toggling the #9999FFShift+A"+self.msgColor+" key, recenter your target, and then re-enable the manual SPEC drive again with the same keys.")
            VS.IOmessage (13,"Oswald","Privateer",self.msgColor+"When you have approched Atlantis to 10 000km please disble the SPEC drive toggling the #9999FFShift+A"+self.msgColor+" key again and then stop your ship.")
            name = unit.getUnitFullName(self.destination)
            self.objective = VS.addObjective("Approach %s" % name)
            self.objectives+=[int(self.objective)]
            # set comm animation parameters
            self.sequence = [[0,12,0],[13,9,0]]
            self.speech = ["com_tutorial_oswald/oswald_spec_08.ogg", ""]
            self.talktime = VS.GetGameTime()
            self.timer = VS.GetGameTime()+22
            self.practice += 1
          #else:
          #    self.practice += 2
      if (self.practice==12 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==12 and VS.GetGameTime()>=self.timer):
          self.practice = 13
      if (self.practice==13 and self.player.getDistance(self.destination)<=10000000):
        VS.setCompleteness(self.objectives[self.objective],1.0)
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"All right.")
        VS.IOmessage (2,"Oswald","Privateer",self.msgColor+"You have learned how to conveniently travel within the system.")
        self.speech = ["com_tutorial_oswald/oswald_spec_09.ogg", ""]
        self.playSoundCockpit(self.speech[0])
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.timer = VS.GetGameTime()+7
        self.practice += 1
      if (self.practice==14):
        velocity = Vector.Mag(self.player.GetVelocity())
        if (velocity>=10 and self.player.getDistance(self.destination)<=2000000):
          VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Bring your ship to full stop before crashing into the planet.")
          self.speech = ["com_tutorial_oswald/oswald_spec_10.ogg", ""]
          self.player.commAnimation("com_tutorial_oswald.ani")
          self.playSoundCockpit(self.speech[0])
          self.objective = VS.addObjective("Stop your ship")
          self.objectives+=[int(self.objective)]
          self.timer = VS.GetGameTime()+10
          self.practice += 1
        else:
          self.practice += 2
      if (self.practice==15):
        velocity = Vector.Mag(self.player.GetVelocity())
        if (velocity<=10):
          VS.setCompleteness(self.objectives[self.objective],1.0)
          self.practice += 1
      if (self.practice==16):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Now dock to the planet, go to the mission computer, and save your game.")
        VS.IOmessage (5,"Oswald","Privateer",self.msgColor+"Then get yourself a Jump Drive and an Overdrive and come back for more tutoring if you wish.")
        VS.IOmessage (10,"Oswald","Privateer",self.msgColor+"To dock, turn towards the planet and press the docking clearance request key #9999FFD"+self.msgColor+". A green docking frame will appear.")
        VS.IOmessage (17,"Oswald","Privateer",self.msgColor+"You may still enable the SPEC drive until you close up on the planet and your velocity matches the set velocity.")
        VS.IOmessage (24,"Oswald","Privateer",self.msgColor+"When you are within range, press again the #9999FFD"+self.msgColor+" key to dock. The docking distance will depend on the planet or station size that you are docking to.")
        VS.IOmessage (33,"Oswald","Privateer",self.msgColor+"The larger the object the further away you can dock.")
        VS.IOmessage (36,"Oswald","Privateer",self.msgColor+"For Atlantis the docking distance is roughly about 990 kilometers.")
        # set comm animation parameters
        self.sequence = [[0,5,0],[5,5,0],[10,7,0],[17,6,0],[24,6,0],[33,3,0],[36,5,0]]
        self.speech = ["com_tutorial_oswald/oswald_spec_11.ogg", "", "", "", "com_tutorial_oswald/oswald_spec_12.ogg", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+42
        self.practice += 1
      if (self.practice==17 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==17 and VS.GetGameTime()>=self.timer):
          self.practice += 1
      if (self.practice==18 and self.destination.isDocked(self.player)):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"That concludes the navigation part of the tutorial.")
        self.speech = ["com_tutorial_oswald/oswald_spec_13.ogg"]
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.playSoundCockpit(self.speech[0])
        self.timer = VS.GetGameTime()+3
        self.practice = 99
      if (self.practice>=99):
        # make sure to reset the counter for the next practice loops
        self.practice = 0
        self.stage = COMPLETE_TUTORIAL4
        self.putSaveValue(self.stage)
      return 0

  # play the intermezzo when attacked
  def tutorialIntermezzo (self):
      # maybe he can have sympathy and make it a lesson if he beats you
      # like stop before you destroy the player's ship and mouth off about how he hopes you learned
      # something, 'pirates won't be so forgiving'
      if (self.practice==0 and self.fight==1):
        VS.IOmessage (0,"Oswald","Privateer","#FF0000"+"So you want to learn how to dodge lasers, eh?")
        VS.IOmessage (3,"Oswald","Privateer","#FF0000"+"You aren't the first newbie I've had to put down, and you won't be the last!")
        # set comm animation parameters
        self.sequence = [[0,3,0],[3,5,0]]
        self.speech = ["com_tutorial_oswald/oswald_intermezzo_01.ogg", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+10
        self.practice = 1
      if (self.practice==0 and self.fight==2):
        VS.IOmessage (0,"Oswald","Privateer","#FF0000"+"That's over the border, boy!")
        VS.IOmessage (2,"Oswald","Privateer","#FF0000"+"May God have mercy upon my enemies, because I won't!")
        self.speech = ["com_tutorial_oswald/oswald_intermezzo_02.ogg", ""]
        # set comm animation parameters
        self.sequence = [[0,2,0],[2,3,0]]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+5
        self.practice = 1
      if (self.practice==1 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==1 and VS.GetGameTime()>=self.timer):
          self.practice = 4
      if (self.practice==4 and self.player.GetHull()<=self.playerhull*0.90 and self.fight<2):
        self.drone.SetTarget(VS.Unit())
        VS.AdjustRelation(self.drone.getFactionName(),self.player.getFactionName(),99,10)
        VS.AdjustRelation(self.player.getFactionName(),self.drone.getFactionName(),99,10)
        self.drone.LoadAIScript("sitting_duck")
        self.drone.PrimeOrders()
        self.practice = 5
      if (self.practice==5):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Enough of this!")
        VS.IOmessage (1,"Oswald","Privateer",self.msgColor+"I hope you've learned something.")
        VS.IOmessage (3,"Oswald","Privateer",self.msgColor+"Pirates won't be so forgiving.")
        VS.IOmessage (5,"Oswald","Privateer",self.msgColor+"And neither will I if you ever play tricks on me again!")
        # set comm animation parameters
        self.sequence = [[0,2,0],[3,5,0]]
        self.speech = ["com_tutorial_oswald/oswald_intermezzo_03.ogg", "", "", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+8
        self.practice = 6
      if (self.practice==6 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.practice==6 and VS.GetGameTime()>=self.timer):
          self.practice = 7
      if (self.practice==7 and VS.GetGameTime()>=self.timer):
        VS.IOmessage (0,"Oswald","Privateer",self.msgColor+"Now let's get back to business, hotshot.")
        self.speech = ["com_tutorial_oswald/oswald_intermezzo_04.ogg"]
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.playSoundCockpit(self.speech[0])
        self.timer = VS.GetGameTime()+3
        self.practice = 8
      if (self.practice==8 and VS.GetGameTime()>=self.timer):
          self.practice = 99
      if (self.practice>=99):
        self.droneshield = unit.getShieldPercent(self.drone)
        # make sure to reset the counter for the next practice loops
        self.practice = 0
        self.stage = self.savestage
      return 0

  # the execute loop for (nearly) each frame
  def Execute (self):
    # do not do anything before the player undocks
    if (self.stage==STAGE_DOCKED):
      self.complete = self.getSaveValue()
      if (self.complete==0) and (self.newsplayed==0):
        # publish news
        text = "CEPHID SECURITY INITIATIVE GIVES TRAINING FOR FLIGHT SAFETY\\\The Cephid Security Initiative (CSI) is offering training for pilots with the purpose of enhancing flight safety in and out of the system. "
        text += "A representative said, this training is sponsored by volunteer contributors and open for pilots of all vessel classes. "
        text += "No matter if you're a greenhorn in space faring or a long-time pilot you may meet one of the volunteers when leaving from a planet or station into space and participate in the training or refresher."
        news.publishNews(text)
        self.newsplayed = 1
      if (self.complete>=98):
        self.stage = self.complete
      if (self.complete<98):
        self.playerIsUndocked()

    # launch drone if the player did not die
    if (not self.player.isNull()):
      # when in space, launch the drone
      if (self.stage==STAGE_AWAY and VS.GetGameTime()>self.timer):
        self.launchNewDrone()
        #self.stage = 7 #debug
      # play the talk animation
      if (self.stage==2 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.stage==2 and VS.GetGameTime()>=self.timer):
        self.stage = 3

    if (not self.player.isNull() and not self.drone.isNull()):
      # check if player is attacking drone 
      if (self.stage<90):
        self.checkDroneHealth()
      # check if player is still in system
      if (self.stage<90 and not VS.getSystemName()==self.system):
        self.stage = 97
      # check if player is flying off
      if (self.stage<90 and self.stage!=COMPLETE_TUTORIAL3 and self.stage!=COMPLETE_TUTORIAL4):
        self.checkDistance()
      # when drone is launched, then follow player
      if (self.stage==3):
        self.orbitMe()
        self.acceptTutorial()
      if (self.stage==STAGE_ACCEPT):
        self.tutorialComm()
      if (self.stage==COMPLETE_TUTORIAL1):
        self.tutorialNav()
      if (self.stage==COMPLETE_TUTORIAL2):
        self.practiceNav()
      if (self.stage==COMPLETE_TUTORIAL3):
        self.orbitMe()
        self.practiceSpec()
      # tutorial is incomplete, so a nice excuse is required
      if (self.stage==COMPLETE_TUTORIAL4 and VS.GetGameTime()>self.timer):
        VS.IOmessage (0,"Oswald","player",self.msgColor+"Oops. Sorry, pal. My boss at the Cephid Safety Initiative has an emergency situation I must handle now. I apologize.")
        VS.IOmessage (9,"Oswald","player",self.msgColor+"Have a nice journey. And come back for more.")
        self.sequence = [[0,8,0],[9,3,0]]
        self.speech = ["com_tutorial_oswald/oswald_cancel.ogg", ""]
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+12
        self.stage = 9
      # play the talk animation
      if (self.stage==9 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
        for index in range (len(self.sequence)):
          if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
            self.player.commAnimation(self.animations[self.sequence[index][2]][0])
            # play each soundfile once
            if (VS.GetGameTime()-self.talktime<=self.sequence[index][0]+1):
              self.playSoundCockpit(self.speech[index])
            self.anitime = VS.GetGameTime()+2
      if (self.stage==9 and VS.GetGameTime()>=self.timer):
        self.stage = 98
      # drone was attacked, give him a lesson!
      if (self.stage==96):
        self.tutorialIntermezzo()
      # player run off during tutorial, let's break it off then
      if (self.stage==97 and VS.GetGameTime()>self.timer):
        VS.IOmessage (0,"Oswald","player",self.msgColor+"Hey! Where are you heading? Come back for more anytime.")
        VS.IOmessage (4,"Oswald","player",self.msgColor+"Have a good flight and don't break the hull.")
        self.speech = ["com_tutorial_oswald/oswald_runoff_01.ogg"]
        self.playSoundCockpit(self.speech[0])
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.player.commAnimation("com_tutorial_oswald.ani")
        self.timer = VS.GetGameTime()+6
        self.stage = 98
      # if the turorial was declined
      if (self.stage==98 and VS.GetGameTime()>self.timer):
        self.drone.SetVelocity((2000,0,0))
        self.timer = VS.GetGameTime()+10
        self.putSaveValue(self.stage)
        print "Tutorial quest finished"
        self.stage = 99
      # let the drone disappear
      if (self.stage==99 and VS.GetGameTime()>self.timer):
        self.drone.PrimeOrders()
        self.playernum = -1
        self.name = "quest_tutorial"
        self.removeQuest()
        self.stage += 1 # don't enter this loop anymore
        return 0
    # keep the script alive for execution
    return 1

# call this from the adventure file
class quest_tutorial_factory (quest.quest_factory):
  def __init__ (self):
    quest.quest_factory.__init__ (self,"quest_tutorial")
  def create (self):
    return quest_tutorial()

# In order to work properly in mission scripts for testing purposes
# it must inherit the Director.Mission class *and* call its constructor.
# Unfortunately quests must inherit from the quest class, so you need to use a wrapper class
class Executor(Director.Mission):
  def __init__(self, classesToExecute):
    Director.Mission.__init__(self)
    self.classes = classesToExecute
  def Execute(self):
    for c in self.classes:
      c.Execute()
