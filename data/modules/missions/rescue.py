from go_somewhere_significant import go_somewhere_significant
import vsrandom
import universe
import launch
import faction_ships
import Director
import unit
import quest
import VS
import go_to_adjacent_systems
import go_somewhere_significant
class rescue (Director.Mission):

    def __init__ (self, creds,numsysaway, fac,numenemy,enfac,enfg='', jumps=(),var_when_done=''):
        Director.Mission.__init__ (self)
        self.arrived=0
        self.faction = fac
        self.enfaction=enfac
        self.enflightgroup=enfg
        self.cred=creds
        self.donevar=var_when_done
        self.you = VS.getPlayer()
        self.mplay=universe.getMessagePlayer(self.you)
        self.adjsys = go_to_adjacent_systems.go_to_adjacent_systems(self.you,numsysaway,jumps)
        self.numenemy=numenemy
        self.eject=None
        self.enemy=None
        self.numejectors=0
        self.obj=0
    def Execute(self):
        if (not self.you):
            VS.terminateMission(0)
            return
        if (not self.adjsys.Execute()):
            return
        if (self.arrived==0):
            self.adjsys=go_somewhere_significant.go_somewhere_significant(self.you,0,10000.)
            sys = VS.getSystemFile()
            import dynamic_battle
            if sys in dynamic_battle.rescuelist:
                del dynamic_battle.rescuelist[sys]
            self.arrived=1
        elif (self.arrived==1):
            mymin=2500.
            mymax=3000.
#            try:
#                mymin*=faction_ships.launch_distance_factor
#                mymax*=faction_ships.launch_distance_factor
#            except:
#                pass            
            self.eject = launch.launch_wave_around_unit("Rescue Pilot",self.faction,"eject","printhello.py",1,mymin,mymax,self.adjsys.SignificantUnit())
            self.eject.setMissionRelevant()
            self.arrived=2
            self.eject.setName("Pilot")
            self.eject.SetHull(25);
            L = launch.Launch()
            L.fg = "Shadow"
            L.faction=self.enfaction
            L.dynfg = self.enflightgroup
            L.type = faction_ships.getRandomFighter(self.enfaction)
            L.ai="default"
            L.num=self.numenemy
            L.minradius=float(VS.vsConfig("dynamic_universe","missions.rescue.enemy.minradius","5000"))
            L.maxradius=float(VS.vsConfig("dynamic_universe","missions.rescue.enemy.maxradius","15000"))
            try:
                L.minradius*=faction_ships.launch_distance_factor
                L.maxradius*=faction_ships.launch_distance_factor
            except:
                pass
            self.enemy=L.launch(self.adjsys.SignificantUnit())
            self.enemy.SetTarget(self.eject)
            self.enemy.setFgDirective("A.")
            self.you.SetTarget(self.eject)
            VS.IOmessage(0,"eject",self.mplay,"This is ejector pod 0x4032 requesting immediate rescue and pickup")
            self.obj=VS.addObjective("Tractor Ejected Pilot")

        elif (self.arrived==2):
            if (self.eject):
                self.enemy.SetTarget(self.eject)
                self.numejectors=self.you.GetCargo("Pilot").GetQuantity()
            else:
                if (self.enemy):
                    self.enemy.SetTarget(self.you)
                    self.enemy.setFgDirective("A.")
                print 'numej '+str(self.numejectors)
                if (self.numejectors<self.you.GetCargo("Pilot").GetQuantity()):
                    self.you.removeCargo("Pilot",1,1)
                    carg=VS.getRandCargo(1,"Passengers/Economy")
                    carg.SetMissionFlag(1)
                    VS.setCompleteness(self.obj,1)
                    if (not self.you.addCargo(carg)):
                        self.Win(self.you,1)
                    else:
                        self.arrived=3
                        self.cargname=carg.GetContent()
                        print 'adding '+self.cargname
                        self.adjsys = go_somewhere_significant.go_somewhere_significant(self.you,1,25000,base_only=1)
                        VS.IOmessage(0,"Passenger",self.mplay,"Please take me to my home base: %s so I may begin to recover. Thank you!"%self.adjsys.SignificantUnit().getName())
                else:
                    VS.IOmessage(0,"rescue_mission",self.mplay,"#ff0000You Lost the Pilot before you could drop the Pilot at the base.  The Pilot's oxygen is currently empty. You fail.")
                    self.Lose(1)
        elif (self.arrived==3):
            #check for dockal
            if (self.you.GetCargo(self.cargname).GetQuantity()==0):
                self.Lose(1)
            elif (self.adjsys.SignificantUnit().isDocked(self.you) or self.you.isDocked(self.adjsys.SignificantUnit()) or self.adjsys.SignificantUnit().getDistance(self.you)<10):
                if (self.you.removeCargo(self.cargname,1,1)):
                    self.Win(self.you,1)
                else:
                    self.Lose(1)
    def Win (self,un,terminate):
        VS.IOmessage (0,"Passenger",self.mplay,"#00ff00Excellent work pilot.")
        VS.IOmessage (0,"Passenger",self.mplay,"#00ff00You have been rewarded for your effort as agreed.")
        VS.IOmessage (0,"Passenger",self.mplay,"#00ff00You saved my life. I owe you a drink, pal")
        un.addCredits(self.cred)
        VS.AdjustRelation(self.you.getFactionName(),self.faction,.02,1)
        if len(self.donevar):
            quest.removeQuest(self.you.isPlayerStarship(),self.donevar,1)
        if (terminate):
            VS.terminateMission(1)

    def Lose (self,terminate):
        VS.AdjustRelation(self.you.getFactionName(),self.faction,-.02,1)
        VS.IOmessage(0,"rescue_mission",self.mplay,"#ff0000Credit agency reports that you have failed the mission.")
        if len(self.donevar):
            quest.removeQuest(int(self.mplay[1:]),self.donevar,-1)
        if (terminate):
            print "lose plunder mission"
            VS.terminateMission(0)
    def initbriefing(self):
        print "ending briefing"
    def loopbriefing(self):
        print "loop briefing"
        Briefing.terminate();

    def endbriefing(self):
        print "ending briefing"
