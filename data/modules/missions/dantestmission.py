import quest
import Vector
import VS
import unit
import vsrandom
import save_util
import faction_ships
import universe
import launch
import Director
#import tuples_fg

class dantestmission_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"dantestmission")
    def create (self):
        return dantestmission()
    def precondition(self,playernum):
        return 1

class dantestmission (quest.quest):



    def __init__ (self):
        self.playa = VS.getPlayer()
        if (self.playa):
            VS.IOmessage (3,"The past version of you.","all","Daniel, this is your test mission...")

            self.un0=launch.launch_wave_around_unit("siege_win","trigger","cargo","default",1,1,3000,self.playa)
            self.un1=launch.launch_wave_around_unit("siege_loss","trigger","cargo","default",1,1,2000,self.playa)
            self.un2=launch.launch_wave_around_unit("siege_draw","trigger","cargo","default",1,1,1000,self.playa)

            self.numguard = 4
            self.guard = ()
            for i in range(self.numguard):
                self.guard = self.guard + (launch.launch_wave_around_unit("Guardians","neutral","drone","default",1,1,4000,self.playa),)

            self.triggkey = [1,1,1,1]

            print "Sending News"

            Director.pushSaveString(0,"dynamic_news","#TEST STORY!\\Will this thing ever work!\\	Dan.a")

            Director.pushSaveString(0,"dynamic_news","siege,start,rlaan,aera,-1,0.8,enigma_sector/boondoggles,all")
            Director.pushSaveString(0,"dynamic_news","siege,end,rlaan,aera,1,0.8,enigma_sector/boondoggles,all")
            Director.pushSaveString(0,"dynamic_news","my,arse,rlaan,aera,-1,0.8,is_sector/sore,all")



    def Execute (self):
        if 0 in self.triggkey and self.triggkey[3] == 1:
            print "0 in triggkey"
            unit.setTfgDirective(self.guard,self.playa,'A')
            self.triggkey[3] = 0

        if self.un0.isNull() and self.triggkey[0]:
            print "siege_win"
            self.triggkey[0] = 0
            Director.pushSaveString(0,"dynamic_news","siege,start,rlaan,aera,1,0.8,enigma_sector/boondoggles,all")

        if self.un1.isNull() and self.triggkey[1]:
            print "siege_loss"
            self.triggkey[1] = 0
            Director.pushSaveString(0,"dynamic_news","siege,start,rlaan,aera,-1,0.8,enigma_sector/boondoggles,all")

        if self.un2.isNull() and self.triggkey[2]:
            print "siege_draw"
            self.triggkey[2] = 0
            Director.pushSaveString(0,"dynamic_news","siege,start,rlaan,aera,-1,0.8,enigma_sector/boondoggles,all")

        return 1
