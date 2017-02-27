#---------------------------------------------------------------------------------
# Vega Strike script for a quest
# Copyright (C) 2008 Vega Strike team
# Contact: hellcatv@users.sourceforge.net
# Internet: http://vegastrike.sourceforge.net/
#.
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# Description: Shipyard bomb explodes in the system
#              adapted from original story by richard
# Author: pyramid
# Version: 2008-04-24
#
#---------------------------------------------------------------------------------

import VS
import quest
import Director
import Vector
import unit
import vsrandom
import news

class quest_shipyardbomb (quest.quest):

    def __init__ (self):
        self.player = VS.getPlayer()
        self.stage = 1
        self.timer = VS.GetGameTime()
        self.msgColor = ("#FF9999","#FF99FF")
        self.system = VS.getSystemName()
        self.shipyard = unit.getUnitByName('Shipyard')

    # things to add:
    # (a) the ship being built and debris
    # (b) patrol disptach from shipyards
    # (c) late arriving police forces
    def Execute (self):
        if (VS.getSystemName()==self.system and self.player and self.shipyard):
            if (self.stage==1 and VS.GetGameTime()>=self.timer):
                if (self.shipyard.getSignificantDistance(self.player) > 20000):
                    print 'shipy ret'
                    print "distance" + str(self.shipyard.getSignificantDistance(self.player))
                    return 1
                else:
                    print 'killing'
                    pos = self.shipyard.Position()
                    size = 10*self.shipyard.rSize()
                    VS.playAnimation("explosion_orange.ani",pos,size)
                    pos = (pos[0]+.5*size,pos[1],pos[2])
                    VS.playAnimation("explosion_orange.ani",pos,size)
                    pos = (pos[0]-size,pos[1],pos[2])
                    VS.playAnimation("explosion_orange.ani",pos,size)
                    VS.playSound("explosion.wav",pos,(1,0,0))
                    self.shipyard.DealDamageToHull ((10,0,0),self.shipyard.GetHull()*.9)
                    self.timer = VS.GetGameTime()+10
                    self.stage = 2
            if (self.stage==2 and VS.GetGameTime()>=self.timer):
                VS.IOmessage(0,unit.getUnitFullName(self.shipyard),"all",self.msgColor[0]+"Large Explosion detected... standby...%#@*")
                text = "NAVAL SHIPYARDS HIT BY BOMB\\\Disaster struck the Confederate Naval Shipyards orbiting Alpha Centauri hours ago, when a powerful explosive device detonated, crippling a fleet carrier that was nearing completion. "
                text += "At least a dozen casualties were reported with an unknown number of injured, and salvage crews are still working hard to clear the area of wreckage. "
                text += "A team from the CSP (Confederate Security Police) arrived at the shipyards mere hours after the incident, and an investigation has been launched to determine who the perpetrators of this attack were, whether they were human terrorists or agents of an alien power."
                news.publishNews(text)
                self.stage = 99
            if (self.stage==99 and not VS.getSystemName()==self.system):
                self.stage += 1 # don't enter this loop anymore
                self.playernum = -1
                self.name = "quest_shipyardbomb"
                self.removeQuest()
                return 0
        return 1

class quest_shipyardbomb_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_shipyardbomb")
    def precondition(self,playernum):
        return 1
    def create (self):
        return quest_shipyardbomb()

class MissionExecutor(Director.Mission):
# call this class from the mission file
   def __init__(self, classesToExecute):
      Director.Mission.__init__(self)
      self.classes = classesToExecute
   def Execute(self):
      for c in self.classes:
         c.Execute()
