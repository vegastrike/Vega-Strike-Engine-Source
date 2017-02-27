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
# Version: 2011-03-01
#
#---------------------------------------------------------------------------------

# This script is used for testing the game
# and or displaying test variables available through python
# It is called through the quest_debug.mission and shouldn't
# be activated in the game release

# things that can be improved

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
import stardate
import string

# predefine stages
SAVE_KEY = "quest_debug"

# the class that will be executed
class quest_debug (quest.quest):
  # initialize quest variables
  def __init__ (self):
    self.msgColor = "#FFFF00"
    self.timer = VS.GetGameTime()     # controls the interval timing
    self.player = VS.getPlayer()
    # show if quest was initialized
    #print "###debug: quest_debug initialized"

  def LightMinuteToMeter(self,lightminute):
    meter = 17987547500 * lightminute
    return meter

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

  # the execute loop for (nearly) each frame
  def Execute (self):
    #if (not self.player.isNull() and self.playerIsUndocked()):
    if ( not self.player.isNull() ):
      # execute every N seconds
      if (VS.GetGameTime()>self.timer):
        # print something on the stdout console
        #print "startime: ", VS.getStarTime(), VS.getStarDate(), stardate.formatStarDate("confed",VS.getStarTime())
        # print something to the ingame messages
        VS.IOmessage (0, self.msgColor+"debug"+"#FFFFFF", "Privateer", self.msgColor+"stardate: " + stardate.formatStarDate("confed",VS.getStarTime()) )
        self.timer = VS.GetGameTime()+20
    # keep the script alive for execution
    return 1

# call this from the adventure file
class quest_debug_factory(quest.quest_factory):
  def __init__ (self):
    quest.quest_factory.__init__ (self,"quest_debug")
  def create (self):
    return quest_debug()

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
