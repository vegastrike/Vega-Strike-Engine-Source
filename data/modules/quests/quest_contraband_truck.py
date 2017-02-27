import quest
import Vector
import VS
import unit
import vsrandom
import save_util
import faction_ships
import universe
import launch
#import tuples_fg


class quest_contraband_truck_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_contraband_truck")
    def create (self):
        return quest_contraband_truck()
    def precondition(self,playernum):
        return 1

class quest_contraband_truck (quest.quest):


    def mission_fail(self):
        print "mission failed"
        VS.IOmessage (0,"game","news","AERAN WARP CORE EXPLODES: \n GNN reports the explosion of a warp core in Klondike system today.  The unstable core was apparently being smuggled out of Confederation space by cloaked Aeran ships.  It is understood that the Aeran ships, and an unknown privateer vessel were in the aera.  They are presumed destroyed.\n\n  GNN --- Your News Network Anywhere")
        print "mission terminating"

        print "mission terminated"
        self.quest_exit = 1


    def mission_success(self):
        self.confed_cruiser=launch.launch_wave_around_unit("Sonorous","confed","corvette","default",1,4000,8000,self.cargo_container)
        self.confed_epeels=launch.launch_wave_around_unit("Sonorous E1","confed","epeellcat","default",5,1000,1000,self.confed_cruiser)
        VS.IOmessage (3,"Sonorous Command","all","Attention Private Vessel!")
        VS.IOmessage (4,"Sonorous Command","all","Under Code 1530 of the Trade Practices Charter, we take posession of this cargo pod.")
        VS.IOmessage (6,"Sonorous Command","all","Please remove yourself or we will remove you.")
        self.jumpout = 0
        self.quest_exit = 1






    def setup_all(self):
        print
        print "Truck Launched"
        print
        self.truck_pirate=launch.launch_wave_around_unit("Smuggler","pirate","truck","modules/ai_qct_waitjump.py",1,3000,5000,self.playa)


        print
        print "Scrap Released"
        print

        self.cargo_container=launch.launch_wave_around_unit("Scrap","aera","cargo","default",1,7000,5000,self.playa)


        print
        print "Aera Released"
        print

        self.numaera = 4
        self.aera_specopp = ()
        for i in range(self.numaera):
            self.aera_specopp = self.aera_specopp + (launch.launch_wave_around_unit("Aera/SpecOpp","aera","dagger","default",1,2000,4000,self.playa),)

        print
        print "Aera Cloaked"
        print
        unit.TfgCloak(1,self.aera_specopp)

        self.repeat_more = 1
        self.repeat_less = 1
        self.repeat_end1 = 1
        self.repeat_end2 = 1
        self.timer1 = 0
        self.jumpout = 0
        self.quest_exit = 0
        global truck_exit
        truck_exit = 0

    def start_destruction(self):
        if self.repeat_end2 == 2 and not self.cargo_container.isNull():
            if self.timer1 == 0:
                self.timer1 = VS.GetGameTime()
            self.cargo_container.Split(75)
            VS.playAnimation("explosion_wave.ani",self.cargo_container.Position(),300)
            VS.playAnimation("explosion_wave.ani",self.cargo_container.Position(),100)
            VS.playAnimation("explosion_wave.ani",self.cargo_container.Position(),700)
            VS.playAnimation("explosion_wave.ani",self.cargo_container.Position(),1000)
            VS.playSound("sfx43.wav",self.cargo_container.Position(),self.cargo_container.GetVelocity())
            VS.playSound("Flux.wav",self.cargo_container.Position(),self.cargo_container.GetVelocity())
            VS.playSound("electricity.wav",self.cargo_container.Position(),self.cargo_container.GetVelocity())

            print "adding particle"
            VS.addParticle(self.cargo_container.Position(),self.cargo_container.GetVelocity(),(1,.2,.2))
            print "added particle"
            unit.setTfgDirective(self.aera_specopp,self.playa,'A')

            print "begin msgs"

            VS.IOmessage (0,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] Turn your attention <surprise> Entity/self triggered item warp core!")
            VS.IOmessage (5,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] <fear, anxiety, anger> Filthy human procreate entity/self!")
            VS.IOmessage (12,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] Group leave fast danger avo...")
            print "ended msgs"
            self.jumpout = 1
            self.repeat_end2 = 0
            print "ended start_destruction"

    def end_destruction(self):
#               print "testing timer"
        if (self.timer1 + 12) <= VS.GetGameTime():

            print "playing sounds"
            VS.playSound("sfx43.wav",self.playa.Position(),self.playa.GetVelocity())
            VS.playSound("Flux.wav",self.playa.Position(),self.playa.GetVelocity())
            VS.playSound("electricity.wav",self.playa.Position(),self.playa.GetVelocity())

            print "attempting the jump"

            unit.TfgJumpTo(self.aera_specopp,"gemini_sector/pestilence")

            print "attempted the jump"

            self.playa.JumpTo("gemini_sector/pestilence")
            print "jumped playa"
            VS.IOmessage (0,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] ...id")
            VS.IOmessage (5,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] <untranslatable> section of excretement <untranslatable> human <untranslatable> genitalia <untranslatable> fire <untranslatable> nice day.")

            VS.IOmessage (0,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] Flee smart to go entity/self <conditional> life value.")

            print "done all but fail"
            self.mission_fail()













    def __init__ (self):
        self.playa = VS.getPlayer()
        if (self.playa):
            self.setup_all()
            VS.IOmessage (3,"[Computer]","all","Scans show several peices of scrap in this system.  May contain valuable cargo.")



    def Execute (self):


# remove this test        self.sysfile = VS.getSystemFile()
#               if self.truck_pirate.getUnitSystemFile()!= VS.getSystemFile():
#                       print "Truck has Jumped Away :-)"
#               else:
#                       print "Truck is still here! :-("








        if self.quest_exit == 1:
            return 0
        if (self.playa):


            if self.jumpout == 1:
                self.end_destruction()

            elif self.cargo_container.getMinDis(self.playa.Position()) < 1000:
                self.start_destruction()

            elif self.cargo_container.getMinDis(self.playa.Position()) < 2500 and not self.cargo_container.isNull():
                if self.repeat_end2 == 1:
                    VS.IOmessage (3,"[Computer]","all","Warning! Annomalous warp echos detected.")
                    print
                    print "Aera Un-Cloaked"
                    print
                    unit.TfgCloak(0,self.aera_specopp)
                    self.repeat_end2 = 2

            elif self.cargo_container.isNull():
                if self.repeat_end2 == 1:
                    print "null activating"
                    VS.IOmessage (3,"[Computer]","all","Warning! De-Cloaking hostiles.")
                    print
                    print "Aera Un-Cloaked"
                    print
                    unit.TfgCloak(0,self.aera_specopp)
                    unit.setTfgDirective(self.aera_specopp,self.playa,'A')
                    VS.IOmessage (0,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] <surprise> Attention all! <anger> Entity/self valueless takes possession away value.")
                    VS.IOmessage (3,"Aera SpecOpp","all","[Translate: Aernoss -> Englysh] <anger> Filthy human procreate entity/self!  Death done!")
                    self.repeat_end2 = 0



            if unit.TfgisNull(self.aera_specopp):
                self.mission_success()

            if self.jumpout == 1:
                self.end_destruction()


            if self.truck_pirate.getMinDis(self.playa.Position()) < 200:
                if self.repeat_end1 == 2:
                    VS.IOmessage (0,"Sonorous E2","all","I am with Confed Special Service.")
                    VS.IOmessage (0,"Sonorous E2","all","You are hampering a priority 0 operation.")
                    self.confed_epeels2=launch.launch_wave_around_unit("Sonorous A3","confed","epeellcat","default",5,1000,5000,self.playa)
                    VS.IOmessage (5,"Sonorous E2","all","You are Terminated.")
                    self.confed_epeels2.SetTarget(self.playa)
                    self.confed_epeels2.setFgDirective('A')
                    self.repeat_end1 = 3

            elif self.truck_pirate.getMinDis(self.playa.Position()) < 1000:
                if self.repeat_end1 == 1:
                    VS.IOmessage (0,"Smuggler:truck","all","Back off mate, if you know what's good for you.")
                    print "My target is..."

                    global truck_exit
                    truck_exit = 1
                    print truck_exit

                    self.repeat_more = 0
                    self.repeat_less = 0
                    self.repeat_end1 = 2

            elif self.truck_pirate.getMinDis(self.playa.Position()) < 3000:
                if self.repeat_less == 1:
                    VS.IOmessage (0,"Smuggler:truck","all","Please stay away, we are carrying valuable cargo.")
                    self.repeat_more = 1
                    self.repeat_less = 0

            else:
                if self.repeat_more == 1:
                    VS.IOmessage (0,"Smuggler:truck","all","Keep your distance.")
                    self.repeat_more = 0
                    self.repeat_less = 1









#                       self.makeQuestPersistent()
            return 1
