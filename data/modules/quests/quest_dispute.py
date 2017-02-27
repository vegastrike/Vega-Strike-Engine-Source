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
# Description: A small dispute about docking priority between a merchant and an
#              ISO ship. Based on a story and original script by PeteyG
# Author: pyramid
# Version: 2008-04-20
#
#---------------------------------------------------------------------------------

# import used libraries
import quest
import Director
import VS
import Vector
import universe
import unit
import launch
import news

class quest_dispute (quest.quest):
    def __init__ (self):
        self.player = VS.getPlayer()
        self.stage = 1
        self.timer = VS.GetGameTime()
        self.talktime = VS.GetGameTime()
        self.anitime = VS.GetGameTime()
        self.merColor = "#99FFFF"
        self.socColor = "#FF9999"
        # This will get an agricultural station from the system, and assign it to 'station'
        # this station is the only one that has only one docking port, thus fits perfectly
        # into this quest theme
        self.station = unit.getUnitByName('Agricultural_Station')

    # the flow of the quest
    # (0) check if player is in system
    # (1) launch the actor units
    # (2) start docking dispute conversation
    # (3) play dispute comm animation
    # (4) start armed conflict
    # (5) socialist call for help
    # (6) play comm animation
    # (7) wait for player interaction - check who's still alive
    # (11) socialist reward
    # (21) merchant reward
    # (12/22) play the comm animation
    # (30) let the winner dock at the station
    # (99) finish quest
    # there could be more variety:
    # (a) confed militia flying in to get the trouble makers
    # (b) destoyed ships spawning escape pods
    # (c) some provoking conversation during the conflict
    # (d) breaking off the battle after some time (and off to dock)
    # (e) station calling for the guys to stop hitting her
    def Execute (self):
        if (self.player and self.station):
            # launches the particpating actors
            if (self.stage==1 and VS.GetGameTime()>self.timer and self.station.getDistance(self.player)<50000):
                self.stageLaunchActors()
                self.stage = 2

            # checks to see if the self.player is within 10km of the station to initiate the dispute
            if (self.stage==2 and VS.GetGameTime()>self.timer and self.station.getDistance(self.player)<15000):
                self.stageDockingDispute()
                self.talktime = VS.GetGameTime()
                self.stage = 3

            # play the talk animation
            if (self.stage==3 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
                # check which animation to play
                for index in range (len(self.sequence)):
                    if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
                        # play the animation
                        self.player.commAnimation(self.animations[self.sequence[index][2]][0])
                        # this is how long one animation takes
                        self.anitime = VS.GetGameTime()+2
            if (self.stage==3 and VS.GetGameTime()>=self.timer):
                self.stage = 4

            # get the merchant to attack
            if (self.stage==4 and VS.GetGameTime()>self.timer):
                # the two ships start to attack each other
                self.merchant.SetTarget(self.socialist)
                VS.AdjustRelation(self.merchant.getFactionName(),self.socialist.getFactionName(),-5,1)
                self.merchant.LoadAIScript("default")
                # attack directive - no ai change, no target change
                self.merchant.setFgDirective("A.")
                self.timer = VS.GetGameTime()+5
                self.stage = 5

            # get the socialist to attack
            if (self.stage==5 and VS.GetGameTime()>self.timer):
                # the privateer gets involved...   or does he?
                VS.IOmessage (0,"[Lenin's Mercy]","privateer",self.socColor+"Mayday! We are under attack! Privateer, please help us...  we are no match for them. We have wounded on board!")
                VS.IOmessage (6,"[VulCorp Transport A-5]","privateer",self.merColor+"Privateer, if you look the other way... you will be duly compensated.")
                self.animations = [["com_dispute_socialist.ani",2],["com_dispute_merchant.ani",2]]
                self.sequence = [[0,6,0],[6,4,1]]
                self.talktime = VS.GetGameTime()
                VS.AdjustRelation(self.socialist.getFactionName(),self.merchant.getFactionName(),-5,1)
                self.socialist.SetTarget(self.merchant)
                self.socialist.LoadAIScript("default")
                self.socialist.setFgDirective("A.")
                self.timer = VS.GetGameTime()+10
                self.stage = 6

            # play the talk animation
            if (self.stage==6 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
                for index in range (len(self.sequence)):
                    if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
                        self.player.commAnimation(self.animations[self.sequence[index][2]][0])
                        self.anitime = VS.GetGameTime()+2
            if (self.stage==6 and VS.GetGameTime()>=self.timer):
                self.stage = 7

            # we need to refresh the ai during battle since it lasts for only 7 seconds
            if (self.stage==7 and VS.GetGameTime()>self.timer):
                self.merchant.LoadAIScript("default")
                self.socialist.LoadAIScript("default")
                self.timer = VS.GetGameTime()+2

            # evaluate the conflict result
            if (self.stage==7 and self.merchant.isNull()):
                VS.IOmessage (0,"[VulCorp Transport A-5]","all",self.merColor+"Oh nooooo...!!!!!")
                self.player.commAnimation("com_dispute_merchant.ani")
                self.stage = 11
            if (self.stage==7 and self.socialist.isNull()):
                VS.IOmessage (0,"[Lenin's Mercy]","all",self.socColor+"Liberte! Egalite!! Fraternite...!!!!!")
                self.player.commAnimation("com_dispute_socialist.ani")
                self.stage = 21

            # if the merchant has died, give player the socialist reward
            if (self.stage==11 and VS.GetGameTime()>self.timer):
                self.socialist.PrimeOrders()
                VS.IOmessage (0,"[Lenin's Mercy]","privateer",self.socColor+"Thank you, Privateer! The Interstellar Socialist Organization is in your debt. We are getting our wounded to the base's medical facility.")
                VS.IOmessage (5,"[Lenin's Mercy]","privateer",self.socColor+"We have no money... but we are transmitting you the coordinates of the cargo we dumped to make room for the attack victims. Do with it what you will.")
                VS.IOmessage (10,"[Lenin's Mercy]","privateer",self.socColor+"You have made a friend with the ISO today. Have a safe journey.")
                self.animations = [["com_dispute_socialist.ani",2]]
                self.sequence = [[0,15,0]]
                self.talktime = VS.GetGameTime()
                # launches various types of cargo as reward
                # launch_wave_around_unit (fgname, faction, type, ai, nr_ships, minradius, maxradius, my_unit,logo='',useani=1,skipdj=0)
                self.cargo = launch.launch_wave_around_unit("Cargo",'neutral','iron_ore.cargo',"default",2,3000,6000,self.player)
                self.cargo = launch.launch_wave_around_unit("Cargo",'neutral','tungsten_ore.cargo',"default",2,3000,6000,self.player)
                self.cargo = launch.launch_wave_around_unit("Cargo",'neutral','generic_cargo',"default",16,3000,6000,self.player)
                # reputation with ISO goes up. Not sure of the numbers
                VS.AdjustRelation(self.player.getFactionName(),self.socialist.getFactionName(),1,5)
                # publish news
                text = "PRIVATEER SAVES SHIPLOAD OF WOUNDED\\\Today, an unprecedented dispute about landing priorities took place close to a station in the Regallis system of Sol sector. "
                text += "A merchant was delivering a priority shipment to a station in the system while an ISO transport vessel requested emergency landing having twelve rescued passengers on board who were previously wounded in a pirate attack. "
                text += "A privateer approaching that base at the same time, and assisting the dispute, reacted to the situation before security forces could arrive at the scene and promptly removed the capitalist bloodsucker, thus saving many lives. "
                text += "Presently, the injured are being taken care of at the medical facilities of the station with two heavily wounded remaining under intensive care."
                news.publishNews(text)
                # set next stage conditions
                self.timer = VS.GetGameTime()+15
                self.winner = self.socialist
                self.stage = 12

            # play the talk animation
            if (self.stage==12 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
                for index in range (len(self.sequence)):
                    if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
                        self.player.commAnimation(self.animations[self.sequence[index][2]][0])
                        self.anitime = VS.GetGameTime()+2
            if (self.stage==12 and VS.GetGameTime()>=self.timer):
                self.stage = 30

            # if the merchant ship is still alive
            if (self.stage==21 and VS.GetGameTime()>self.timer):
                self.merchant.PrimeOrders()
                # if the merchant is still friends with the self.player, the merchant gives him a nice chunk of cash
                if (5 > -.1):
                    VS.IOmessage (0,"[VulCorp Transport A-5]","privateer",self.merColor+"Privateer, thank you for your cooperation.")
                    VS.IOmessage (3,"[VulCorp Transport A-5]","privateer",self.merColor+"We will be able to make a killing on this shipment thanks to you. Here are 15000 credits for your trouble.")
                    self.animations = [["com_dispute_merchant.ani",2]]
                    self.sequence = [[0,8,0]]
                    self.talktime = VS.GetGameTime()
                    self.player.addCredits(15000)
                    # rep with merchants goes up
                    VS.AdjustRelation(self.player.getFactionName(),self.merchant.getFactionName(),.1,.5)
                    # publish news
                    text = "MALICIOUS MERCHANT MASSACRES MARXIST MERCY MISSION\\\Today, an unprecedented dispute about landing priorities took place close to a station in the Regallis system of Sol sector. "
                    text += "A merchant was delivering a priority shipment to a station in the system while an ISO transport vessel requested emergency landing having twelve rescued passengers on board who were previously wounded in a pirate attack. "
                    text += "Before security forces could arrive at the scene the merchant pilot promptly applied his own justice scheme thus reducing the other vessel cum content to space dust."
                    news.publishNews(text)
                    # set next stage conditions
                    self.timer = VS.GetGameTime()+8
                    self.winner = self.merchant
                    self.stage = 22

            # play the talk animation
            if (self.stage==22 and VS.GetGameTime()<self.timer and VS.GetGameTime()>=self.anitime):
                for index in range (len(self.sequence)):
                    if (VS.GetGameTime()-self.talktime>=self.sequence[index][0] and VS.GetGameTime()-self.talktime<=self.sequence[index][0]+self.sequence[index][1]):
                        self.player.commAnimation(self.animations[self.sequence[index][2]][0])
                        self.anitime = VS.GetGameTime()+2
            if (self.stage==22 and VS.GetGameTime()>=self.timer):
                self.stage = 30

            # let the remaining ship approach the station and dock
            if (self.stage==30 and VS.GetGameTime()>self.timer):
                self.timer = VS.GetGameTime()+5
                #if (not self.station.isDocked(self.winner)):
                if (not self.winner.isNull()):
                    # performDockingOperations is unusable
                    # 1st it lets the ship fly through the stations
                    # 2nd it doesn't dock the unit
                    #self.winner.performDockingOperations(self.station,1)
                    unit.approachAndDock(self.winner,self.station)
                else:
                    self.stage = 99

            if (self.stage==99 and VS.GetGameTime()>self.timer):
                self.playernum = -1
                self.name = "quest_dispute"
                self.removeQuest()
                self.stage += 1 # don't enter this loop anymore
                return 0
        return 1

    def stageLaunchActors(self):
        radius = self.station.rSize()
        # launch the two freighters
        self.merchant = launch.launch_wave_around_unit("VulCorp Transport A-5","merchant_guild","Plowshare","ai_sitting_duck.py",1,radius,radius*2,self.station)
        self.socialist = launch.launch_wave_around_unit("Lenin's Mercy","ISO","Llama","ai_sitting_duck.py",1,radius,radius*2,self.station)
        # make the ship a real sitting duck, won't accept any other orders
        self.merchant.PrimeOrders()
        self.socialist.PrimeOrders()
        # make the actors oriented towards the station
        unit.faceTaget(self.merchant, self.station)
        unit.faceTaget(self.socialist, self.station)
        # make the actors fly close to the docking port
        self.merchant.performDockingOperations(self.station,False)
        self.socialist.performDockingOperations(self.station,False)

    def stageDockingDispute(self):
        # the comm interchange between the two ships
        VS.IOmessage (0,"[VulCorp Transport A-5]","all",self.merColor+"VulCorp Transport alpha five requesting priority docking.")
        VS.IOmessage (5,"[VulCorp Transport A-5]","all",self.merColor+"We have a load of spare parts that needs to be delivered within the next half hour, or else we don't get paid.")
        VS.IOmessage (15,"[Lenin's Mercy]","all",self.socColor+"Negative, transport Lenin's Mercy requesting emergency docking. We have thirteen critically injured passengers.")
        VS.IOmessage (25,"[Lenin's Mercy]","all",self.socColor+"We picked them up after a squadron of pirates attacked their ship. They need immediate medical attention!")
        VS.IOmessage (35,"[VulCorp Transport A-5]","all",self.merColor+"Station control, might we remind you that we have a contract with your base? We demand priority in the docking queue so we can complete our transaction.")
        VS.IOmessage (45,"[Lenin's Mercy]","all",self.socColor+"You capitalist pigs! We have dying men and women on board, and all you can think about is your filthy money!")
        VS.IOmessage (55,"[VulCorp Transport A-5]","all",self.merColor+"Socialist vessel: Stay out of the docking queue or you will be fired upon. We will not let a bunch of bleeding communists turn this major deal sour!")
        VS.IOmessage (65,"[Lenin's Mercy]","all",self.socColor+"Negative, VulCorp Transport. The lives of our passengers are worth more than your profits!")
        VS.IOmessage (75,"[VulCorp Transport A-5]","all",self.merColor+"All batteries! Open fire!!")
        # initialize the animation parameters
        # the animations to be alternated - animation file and animation duration
        self.animations = [["com_dispute_merchant.ani",2],["com_dispute_socialist.ani",2]]
        # states the beginning, duration, and animation number to be played
        self.sequence = [[0,10,0],[15,15,1],[35,5,0],[45,5,1],[55,5,0],[65,5,1],[75,4,0]]
        # set the beginning of the talk sequence
        self.talktime = VS.GetGameTime()
        self.timer = VS.GetGameTime()+80

class quest_dispute_factory (quest.quest_factory):
# call this class from the adventure file
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_dispute")
    def create (self):
        return quest_dispute()
 
class Executor(Director.Mission):
# call this class from the mission file
   def __init__(self, classesToExecute):
      Director.Mission.__init__(self)
      self.classes = classesToExecute
   def Execute(self):
      for c in self.classes:
         c.Execute()
