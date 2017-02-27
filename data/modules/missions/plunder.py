from go_somewhere_significant import go_somewhere_significant
import vsrandom
import universe
import launch
import faction_ships
import Director
import unit
import quest
import VS

class plunder (Director.Mission):

    def __init__ (self, creds, faction, quantity, category, returntobase, var_when_done=''):
        Director.Mission.__init__ (self)
        self.newship=""
        self.mplay="all"
        self.obj=0
        self.enemy=VS.Unit()
        self.curiter=0
        self.content=""
        self.quantity=quantity
        self.arrived=0
        self.faction = faction
        self.cred=creds
        self.category=category
        self.donevar=var_when_done
        sysfile = VS.getSystemFile()
        self.newship=faction_ships.getRandomFighter('merchant')
        self.you=VS.getPlayer()
        self.pos=self.you.Position()
        self.gosig=go_somewhere_significant(self.you,0,10000.)
        self.mplay=universe.getMessagePlayer(self.you)
        if (self.you):
            VS.IOmessage (0,"plunder mission",self.mplay,"Your mission is to destroy a %s merchant unit." % (self.newship))
            VS.IOmessage (1,"plunder mission",self.mplay,"It is orbiting around the %s planet in the system." % (unit.getUnitFullName(self.gosig.SignificantUnit())))
            VS.IOmessage (2,"plunder mission",self.mplay,"After it is destroyed, pick up all %s cargo that got ejected."%self.category)
            VS.IOmessage (3,"plunder mission",self.mplay,"Then return to a %s base with your cargo. #00ff00Good luck!"%self.faction)
        else:
            print "aboritng plunder constructor..."
            VS.terminateMission (0)

    def Win (self,un,terminate):
        VS.IOmessage (0,"plunder mission",self.mplay,"#00ff00Excellent work pilot.")
        VS.IOmessage (0,"plunder mission",self.mplay,"#00ff00You have been rewarded for your effort as agreed.")
        VS.IOmessage (0,"plunder mission",self.mplay,"#00ff00Your contribution to the war effort will be remembered.")
        print "do you win?"
        un.addCredits(self.cred)
        if len(self.donevar):
            quest.removeQuest(self.you.isPlayerStarship(),self.donevar,1)
        if (terminate):
            print "you win plunder mission!"
            VS.terminateMission(1)

    def Lose (self,terminate):
        VS.IOmessage(0,"plunder mission",self.mplay,"#ff0000You have failed this mission and will not be rewarded.")
        if len(self.donevar):
            quest.removeQuest(int(self.mplay[1:]),self.donevar,-1)
        if (terminate):
            print "lose plunder mission"
            VS.terminateMission(0)

    def Execute (self):
        isSig=0
        if (self.you.isNull()):
            self.Lose (1)
            return
        if (self.arrived==3):
            self.gosig.Execute()
            cargquant=self.you.GetCargo(self.content).GetQuantity()
            if cargquant<self.quantity:
#             print "ohnooohnoohno!!!!!!!"
                VS.IOmessage(0,"plunder mission",self.mplay,'Not enough of %s cargo... Get more until you have %d.'%(self.quantity))
                VS.setCompleteness(self.obj,0.)
            elif self.you.isDocked(self.gosig.SignificantUnit()) or self.gosig.SignificantUnit().isDocked(self.you):
#             print "du hast gewonnen!"
                self.you.removeCargo(self.content,self.quantity,1)
                self.Win(self.you,1)
#           else:
#             print 'hihohohiho'
        elif (self.arrived==2):
            cargquant=self.you.GetCargo(self.content).GetQuantity()
            VS.setCompleteness(self.obj,float(cargquant)/float(self.quantity))
            if cargquant==self.quantity:
                self.arrived=3
                self.gosig=go_somewhere_significant(self.you,1,3000.,1,"pirates")
                self.gosig.SignificantUnit().setCombatRole("INERT")
                VS.IOmessage(0,"plunder mission",self.mplay,'Give all of your cargo to the %s unit.'%(unit.getUnitFullName(self.gosig.SignificantUnit())))
        elif (self.arrived==1):
            if (self.enemy):
                self.pos=self.enemy.Position()
                curun=VS.getUnit(self.curiter)
                self.curiter+=1
                if (curun==self.enemy):
                    self.enemy.SetTarget(self.you)
                elif (curun.isNull()):
                    self.curiter=0
            if (self.enemy.isNull()):
                VS.setCompleteness(self.obj,1.)
                self.arrived=2
                self.enemy=[]
                for i in range(self.quantity*2):
                    launch.launch_wave_around_area("shadow","upgrades","generic_cargo","sitting_duck",1,5.,10.,self.pos,'',0).setName(self.content)
                self.obj=VS.addObjective("Pick up %d %s cargo"%(self.quantity,self.content))
                VS.IOmessage(0,"plunder mission",self.mplay,'You must now pick up at least %d of the %s cargo.'%(self.quantity,self.content))
        else:
            significant=self.gosig.SignificantUnit()
            if (significant.isNull ()):
                print "sig null"
                VS.terminateMission(0)
                return
            else:
                if (self.you.getSignificantDistance(significant)<10000.0):
                    if (self.newship==""):
                        self.newship=faction_ships.getRandomFighter('merchant')
                    carg=VS.getRandCargo(self.quantity,self.category)
                    if (carg.GetQuantity()==0):
                        carg = VS.getRandCargo(self.quantity,"") #oh no... could be starships...
                    self.content=carg.GetContent()
                    carg.SetQuantity(self.quantity*2)
                    self.enemy=launch.launch_wave_around_unit("shadow","merchant",self.newship,"default",1,200.0,500.0,self.you)
                    self.enemy.addCargo(carg)
                    VS.setCompleteness(self.gosig.obj,1.)
                    VS.IOmessage(0,"plunder mission",self.mplay,'You must now destroy the %s ship.'%self.newship)
                    VS.IOmessage(0,"plunder mission",self.mplay,'That fighter contains the wanted %s cargo in its hold.'%self.content)
                    self.obj=VS.addObjective("Destroy the %s ship." % (unit.getUnitFullName(self.enemy)))
                    if (self.enemy):
                        self.arrived=1
                    else:
                        print "enemy null"
                        VS.terminateMission(0)
                        return

    def initbriefing(self):
        print "ending briefing"

    def loopbriefing(self):
        print "loop briefing"
        Briefing.terminate();

    def endbriefing(self):
        print "ending briefing"


##def initrandom (minns, maxns, credsmin, credsmax, run_away, minshipdifficulty, maxshipdifficulty):
##  you=VS.getPlayer()
##  tempfaction
##  if (you):
##    name = you.getFaction ()
##    factionname=vsrandom.randrange(0,faction_ships.getMaxFactions())
##    tempfaction=faction_ships.intToFaction(factionname)
##    i=0
##    while (name==tempfaction and i<10):
##      factionname=vsrandom.randrange(0,faction_ships.getMaxFactions())
##      tempfaction=faction_ships.intToFaction(factionname)
##      i+=1
##    sd = vsrandom.random()*(maxshipdifficulty-minshipdifficulty)+minshipdifficulty
##    return bounty (minns,maxns,(1.0+(sd*0.5))*(vsrandom.random ()*(credsmax-credsmin)+credsmin),run_away,sd,tempfaction)
##  else:
##    print "aborting bounty initrandom"
##    VS.terminateMission(0)
