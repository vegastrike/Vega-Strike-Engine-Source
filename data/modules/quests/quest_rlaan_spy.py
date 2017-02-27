import quest
import Vector
import VS
import unit
import vsrandom
import launch
class quest_rlaan_spy (quest.quest):
    def __init__ (self):
        self.sysfile = VS.getSystemFile()
        self.stage=0
        self.sig=VS.Unit()
    def SignificantsNear(self,significant):
        if (significant.isJumppoint()):
            import faction_ships
            VS.IOmessage (0,"game","all","[Computer] Target detected matching engine signature reported for rlaan spy in %s" % self.sysfile)
            VS.IOmessage (0,"game","all","[Computer] Confed was hunting this target...");
            self.spy=launch.launch_wave_around_unit("Spy",faction_ships.factions[faction_ships.rlaan],faction_ships.getRandomFighterInt(faction_ships.rlaan),"default",1,30000,50000,significant)
            self.spy.ActivateJumpDrive(0)
            self.spy.SetTarget(significant)
            VS.getPlayer().SetTarget(self.spy)
            self.spy.setFgDirective('B')
            self.sig=significant
            self.stage=1
    def Victory(self):
        VS.IOmessage (0,"game","all","[Confed] You stopped the Rlaan spy from delivering his information to Rlaan HQ.")
        VS.IOmessage (0,"game","all","[Confed] A price of 20,000 will be awarded to you for your expert effort.")
        self.removeQuest()
        playa = VS.getPlayer()
        VS.IOmessage (0,"game","news","RLAAN SPY CAPTURED:  After miners in the Novaya Kiev system exposed the rlaan spy and its escape ship, privateers across the sector began searching for the spy.  The Rlaan ship made best speed to the local jump point, putting it right in the path of a hungry privateer.  20 thousand credits later, that privateer crushed the Rlaan spy and prevented it from communicating whatever finds it had made to the headquarters.  Officially the Star Confederacy discourages vigilante policing; however, in this case the information snagged by the spy was valuable enough to justify putting a hefty sum on the rlaan's head.")
        if (playa):
            playa.addCredits(20000)
    def Lose(self):
        VS.IOmessage (0,"game","all","[Computer] Jump signature matching Rlaan spy reported earlier detected.")
        VS.IOmessage (0,"game","all","[Computer] unable to track.")
        VS.IOmessage (10,"game","all","[Computer] burst transmission to Rlaan space detected.")
        self.removeQuest()

    def Execute (self):
        if (VS.getSystemFile()!=self.sysfile):
            return 1
        if (self.stage==1):
            if (self.spy.isNull()):
                self.Victory()
                return 0
            else:
                if (self.spy.getUnitSystemFile()!=self.sysfile):
                    self.Lose()
                    return 0
                else:
                    self.spy.SetTarget(self.sig)
        return 1

class quest_rlaan_spy_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_rlaan_spy")
    def precondition(self,playernum):
        return quest.findQuest(playernum,'rlaan_mining_spy')
    def create (self):
        return quest_rlaan_spy()
