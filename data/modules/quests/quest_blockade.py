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
# Description: Jump point blockade to an aera system
#              adapted from original story by richard
# Author: pyramid
# Version: 2008-04-23
#
#---------------------------------------------------------------------------------

import VS
import Director
import Vector
import quest
import unit
import launch
import faction_ships
import fg_util
import universe
import vsrandom
import news

class quest_blockade (quest.quest):
    def __init__ (self):
        self.stage = 1
        self.player = VS.getPlayer()
        self.timer = VS.GetGameTime()
        self.system = VS.getSystemName()
        self.object = unit.getUnitByName('Jump_To_Nethuuleil')
        self.groups = 2 #vsrandom.randrange(4,8)
        self.capital = []
        self.fighter = []
        self.msgColor = ("#FFFF99","#FF99FF")

    # things that could be added
    # (a) more comm chatter
    # (b) include animations for the different chatting characters
    # (c) station some ships in the beginning and make more jump in later
    # (d) there seems to be a bug or slowness in the ai,
    #     the fighters don't stop shooting until after some seconds after launching
    #     when they have a capship as a target at the start
    def Execute (self):
        if (self.player):
            #print str(self.player.Position())
            # launch the actors
            if (self.stage<=self.groups and VS.GetGameTime()>=self.timer):
                #print "group: " +str(self.stage)
                self.names = fg_util.GetRandomFGNames(self.groups,"confed")
                shiptype = faction_ships.getRandomCapitolInt(faction_ships.confed)
                #print "group/name: " + str(self.stage) + "/"+names[self.stage-1]
                # launch capital ships
                number = vsrandom.randrange(2,4)
                #print ">capital/type: " +str(number)+"/"+shiptype
                location = Vector.Add (self.object.Position(),(vsrandom.uniform(-10000,10000),
                                       vsrandom.uniform(-10000,10000),
                                       vsrandom.uniform(-10000,10000)))
                offset = (vsrandom.uniform(1000,2000),vsrandom.uniform(1000,2000),vsrandom.uniform(1000,2000))
                for i in range(number):
                    #self.capital += [launch.launch_wave_around_unit(names[self.stage-1],'confed',shiptype,'sitting_duck',1,5000,20000,self.object)]
                    # launch(name,type,faction,unittype,ai,nr,nrwaves,pos,squadlogo)
                    pos = Vector.Add (location,Vector.Scale(offset,i))
                    self.capital += [VS.launch(self.names[self.stage-1],shiptype,'confed','unit','sitting_duck',1,1,pos,'')]
                # launch fighters
                number = number*vsrandom.randrange(3,6)
                # test: Lancelot, Schroedinger, Quicksilver, Convolution
                shiptype = faction_ships.getRandomFighterInt(faction_ships.confed)
                #print ">fighters/type: " + str(number) + "/" + shiptype
                ship = self.capital[0]
                for i in range(number):
                    self.fighter += [launch.launch_wave_around_unit(self.names[self.stage-1],'confed',shiptype,'sitting_duck',1,1000,2000,ship)]
                # change orders for capital ships
                for ship in self.capital:
                    ship.PrimeOrders()
                    unit.faceTaget(ship, self.object)
                # change orders for fighter ships
                for ship in self.fighter:
                    #unit.faceTaget(ship, self.capital[0])
                    ship.PrimeOrders()
                    #ship.SetTarget(self.capital[0])
                    ship.SetTarget(self.object)
                    #ship.UnFire()
                    #ship.setFgDirective("E.") # escort
                    ship.LoadAIScript("modules/ai_escortpatrol.py")
                # time between spawning the groups
                self.timer = VS.GetGameTime() #+10
                self.stage += 1
            if (self.stage==self.groups+1 and VS.GetGameTime()>self.timer):
                self.stage = 10
            # publish news
            if (self.stage==10 and VS.GetGameTime()>self.timer):
                text = "JUMP POINT BLOCKADE IN " + self.system + " SYSTEM\\\Intelligence has uncovered Aeran plans to invade Confederation space."
                text += "As a preventive measure " + str(self.groups) + " flight groups were dispatched and stationed near the Nethuuleil jump point. "
                text += "Multiple capital vessels are guarding the jump point while a multitude of fighter escorts is patrolling nearby space."
                news.publishNews(text)
                self.timer = VS.GetGameTime()+5
                self.stage = 11
            #make some random chatter
            if (self.stage==11 and VS.GetGameTime()>self.timer):
                # select two pilots to chatter
                flightgroup = 1 #vsrandom.randrange(1,self.groups)
                numchatters = 0
                chatters = []
                while (numchatters<2):
                    chatter = vsrandom.randrange(0,len(self.fighter)-1)
                    if (self.fighter[chatter].getFlightgroupName()==self.names[flightgroup-1]):
                        chatters += [chatter]
                        numchatters += 1
                VS.IOmessage (0,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"... lousy bar on our mothership?")
                VS.IOmessage (5,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Yeah, bad lighting, and no bar company besides your dull face.")
                VS.IOmessage (10,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"The drinks selection is not the best I have seen.")
                VS.IOmessage (15,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"And this bartender droid really gets on my nerves.")
                VS.IOmessage (20,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"He never stops talking for an instant.")
                VS.IOmessage (25,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"Guess, we've got to live with that.")
                VS.IOmessage (30,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Yeah.")
                self.timer = VS.GetGameTime()+240
                self.stage = 12
            if (self.stage==12 and VS.GetGameTime()>self.timer):
                # select two pilots to chatter
                flightgroup = 2 #vsrandom.randrange(1,self.groups)
                numchatters = 0
                chatters = []
                while (numchatters<2):
                    chatter = vsrandom.randrange(0,len(self.fighter)-1)
                    if (self.fighter[chatter].getFlightgroupName()==self.names[flightgroup-1]):
                        chatters += [chatter]
                        numchatters += 1
                VS.IOmessage (0,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"Phil?")
                VS.IOmessage (10,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"What's up, buddy?")
                VS.IOmessage (15,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"The rumor is the Aera are planning an invasion.")
                VS.IOmessage (25,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Maybe no rumor anymore. Probably it's all over the news now.")
                VS.IOmessage (35,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"That's why we are here? Will we have to fight?")
                VS.IOmessage (40,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Calm down, youngling.")
                VS.IOmessage (45,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"The war hasn't quite started yet.")
                VS.IOmessage (50,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"I tell ya. This operation is to prepare the worlds for the war.")
                VS.IOmessage (55,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"All staged by the higherups.")
                VS.IOmessage (65,unit.getUnitFullName(self.fighter[chatters[0]]),"all",self.msgColor[0]+"You think so? And what if you're not right? Will you take care of my 'lil sister if I die?")
                VS.IOmessage (75,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Ho, ho. Now stop that blubber. Nobody's gonna die here.")
                VS.IOmessage (80,unit.getUnitFullName(self.fighter[chatters[1]]),"all",self.msgColor[1]+"Just stick to your orders and by dinner time we'll have a big jar of Pilsner and a nice long talk.")
                self.timer = VS.GetGameTime()+300
                self.stage = 13
            # just a temporary backup
            if (self.stage==99):
                #for ship in self.fighter:
                #    ship = VS.getUnit(i)
                #    if (ship.getFlightgroupName()==names[flightgroup]):
                #        print "shipname: " + ship.getFullName()
                #        #ship.setFgDirective("E.") # escort
                #        #VS.PythonAI.XMLScript("++evade.xml")
                self.stage = 98
            #if (self.stage==99 and not VS.getSystemName()==self.system):
            if (not VS.getSystemName()==self.system):
                self.stage += 1 # don't enter this loop anymore
                self.playernum = -1
                self.name = "quest_capital"
                self.removeQuest()
                return 0
        #return 1

class quest_blockade_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_blockade")
    def precondition(self,playernum):
        return quest.findQuest (playernum,"quest_blockade")
    def create (self):
        return quest_blockade()

class MissionExecutor(Director.Mission):
# call this class from the mission file
   def __init__(self, classesToExecute):
      Director.Mission.__init__(self)
      self.classes = classesToExecute
   def Execute(self):
      for c in self.classes:
         c.Execute()
