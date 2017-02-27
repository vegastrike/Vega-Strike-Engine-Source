#---------------------------------------------------------------------------------
# Vega Strike script for a quest
# Copyright (C) 2008 Vega Strike team
# Contact: hellcatv@sourceforge.net
# Internet: http://vegastrike.sourceforge.net/
#.
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# Description: SlaverGuild fight with Confed flight group
#              adapted from original script by hellcatv
# Author: pyramid
# Version: 2008-05-12
#
#---------------------------------------------------------------------------------

import VS
import quest
import Director
import Vector
import unit
import vsrandom
import launch
import faction_ships

class quest_slaver (quest.quest):
    def __init__ (self):
        self.system = VS.getSystemName()
        playa = VS.getPlayer()
        self.msgColor = ("#9999FF","#FF9999")
        if (playa):
            confed = faction_ships.factions[faction_ships.confed]
            pirates = faction_ships.factions[faction_ships.pirates]
            illustrious=launch.launch_wave_around_unit ('Illustrious',confed,faction_ships.getRandomCapitol(confed),'default',1,1000,4000,playa)
            launch.launch_wave_around_unit ('Illustrious',confed,faction_ships.getRandomCapitol(confed),'default',2,1000,2000,illustrious)
            launch.launch_wave_around_unit ('Illustrious',confed,faction_ships.getRandomFighter(confed),'default',4,100,200,illustrious)
            launch.launch_wave_around_unit ('SlaverGuild',pirates,faction_ships.getRandomFighter(pirates),'default',4,100,200,illustrious)
            launch.launch_wave_around_unit ('SlaverGuild',pirates,faction_ships.getRandomCapitol(pirates),'default',2,100,200,illustrious)
            VS.IOmessage (3,"[Computer]","privateer",self.msgColor[0]+"Scans show the remnants of the Slaver Guild being cleaned up by Special Forces.")

    # things that could be added/improved
    # (a) some comm chatter
    # (b) reward for killing either one fg or the other
    def Execute (self):
        if (not VS.getSystemName()==self.system):
            self.playernum = -1
            self.name = "quest_slaver"
            self.removeQuest()
            return 0

class quest_slaver_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_slaver")
    def precondition(self,playernum):
        return 1
    def create (self):
        return quest_slaver()

class MissionExecutor(Director.Mission):
# call this class from the mission file
   def __init__(self, classesToExecute):
      Director.Mission.__init__(self)
      self.classes = classesToExecute
   def Execute(self):
      for c in self.classes:
         c.Execute()
