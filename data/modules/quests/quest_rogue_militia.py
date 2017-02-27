import quest
import Vector
import VS
import unit
import vsrandom
import launch
import faction_ships
class quest_rogue_militia (quest.quest):
    def __init__ (self):
        self.sysfile = VS.getSystemFile()
        playa=VS.getPlayer()
        difficulty=4
        self.stage=0
        self.mark=VS.GetGameTime()
        self.gametime = VS.GetGameTime()
        self.militia = []
        for i in range(difficulty):
            self.militia += [launch.launch_wave_around_unit("Extortionist","pirates",faction_ships.getRandomFighterInt(faction_ships.homeland_security),"default",1,100,150,playa)]
        #        self.distractMilitia()
        VS.IOmessage (0,"Extortionist","all","This is the milita: You must pay toll to pass through these parts")
        VS.IOmessage (0,"Extortionist","all","Power down your engines for 10 seconds and allow us to remove 2000 credits")
        VS.IOmessage (5,"Extortionist","all","Those pirates who do not reduce speed will be shot")
    def ZeroVel (self,playa):
        vel =playa.GetVelocity()
        return (vel[0]>-.1 and vel[0] < .1 and vel[1]>-.1 and vel[1] < .1 and vel[2]>-.1 and vel[2] < .1)
    def deductToll(self,playa):
        playa.addCredits(-2000)
        VS.IOmessage (0,"Extortionist","all","We appreciate your donation and wish you safe passage through this system!")
        VS.IOmessage (3,"Extortionist","all","Thank you, come again!")
    def distractMilitia(self):
        targ = unit.getSignificant(2,1,1)
        for un in self.militia:
            un.SetTarget(targ)
            un.SetVelocity ((500,500,500))
    def Execute (self):
        playa=VS.getPlayer()
        if (playa.isNull()):
            return 1
        if (VS.getSystemFile()!=self.sysfile):
            self.gametime=VS.GetGameTime()
            return 1
        if (VS.GetGameTime()-self.gametime>10 and self.stage==0):
            self.stage=1
            if (self.ZeroVel(playa)):
                self.deductToll(playa)
                self.distractMilitia()
                return 0
            else:
                self.stage=1
                for un in self.militia:
                    un.SetTarget(playa)
                VS.IOmessage(0,"Extortionist","all","Ok you asked for it! Die scoundrel!")
        if (self.stage==1):
            if (len (self.militia)==0):
                VS.IOmessage (0,"Extortionist","all","*@&* taking work from honest securme($*@")
                VS.IOmessage(0,"game","news","ROGUE SECURITY FORCES CRUSHED:  With the bulk of the Confederate Navy deployed in the war against the Aera, the protection and defense of civilian trade lines have been left to local militias in many systems. Recent events have proven, though, that some of these militias walk a very fine line between enforcing the law and breaking it. In the Heinlein System, the militia began charging outrageous toll fees for passage through the system. On those occasions when their demands weren't met, they opened fire. This prompted the Navy to dispatch the HCS Hangzhou to the system, and today the cruiser finally reported arresting the last member of the rogue security forces.")
                l = faction_ships.capitols[faction_ships.confed]
                launch.launch_wave_around_unit("Hangzhou",faction_ships.factions[faction_ships.confed],faction_ships.getRandomCapitolInt(faction_ships.confed),"default",1,1000,2000,playa)
                self.removeQuest();
                return 0
            else:
                if (self.ZeroVel(playa)):
                    tim = VS.GetGameTime()
                    if (tim-self.mark>10):
                        self.mark = tim
                    elif (tim-self.mark>5):
                        self.deductToll(playa)
                        self.distractMilitia()
                        return 0
                if (self.militia[0].isNull()):
                    del self.militia[0]
        return 1

class quest_rogue_militia_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_rogue_militia")
    def create (self):
        return quest_rogue_militia()
