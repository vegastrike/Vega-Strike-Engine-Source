import quest
import Vector
import VS
import unit
import vsrandom
import launch
import faction_ships
class quest_racene (quest.quest):
    def __init__ (self):
        self.sysfile = VS.getSystemFile()
        playa=VS.getPlayer()
        difficulty=4
        self.stage=0
        self.gametime = VS.GetGameTime()
        self.aera = []
        aer=faction_ships.factions[faction_ships.aera]
        conf=faction_ships.factions[faction_ships.confed]
        for i in range(difficulty):
            self.aera += [launch.launch_wave_around_unit("Recon_Lead",aer,faction_ships.getRandomFighterInt(faction_ships.aera),"default",1,500,2000,playa)]
        self.devil=launch.launch_wave_around_significant ('Devils_Fort',aer,'starfortress','default',1,50000,100000,4)
        launch.launch_wave_around_significant ('terran_rescue_fleet',conf,faction_ships.getRandomCapitolInt(faction_ships.confed),'default',1,10000,100000,4)
        launch.launch_wave_around_significant ('terran_rescue_fleet',conf,faction_ships.getRandomCapitolInt(faction_ships.confed),'default',1,10000,100000,4)
        launch.launch_wave_around_significant ('terran_rescue_fleet',conf,faction_ships.getRandomCapitolInt(faction_ships.confed),'default',1,10000,100000,4)
        VS.IOmessage (0,"game","all","[Aera] Scanning for terran capships... [ping] [ping]")
        VS.IOmessage (3,"game","all","[Computer] Analysis of pattern focuses on scan for attacks on Devils Fort...")
        VS.IOmessage (4,"game","all","[Computer] Area class fighter vessels outfitted with powerful radar appear to be used as scouts in the aera...")
    def Execute (self):
        playa=VS.getPlayer()
        if (playa.isNull()):
            return 1
        if (self.stage==0):
            if (len (self.aera)==0):
                if (VS.getSystemFile()!=self.sysfile):
                    return 0
                if (len (self.aera)==0):
                    VS.IOmessage (0,"game","all","[Aera Burst Transmission] No targets detected--resistance encountered ca--&$&(*!@^6")
                    self.gametime=VS.GetGameTime()
                    self.removeQuest();
                    self.stage=1
            else:
                if (self.aera[0].isNull()):
                    del self.aera[0]
        else:
            if (VS.GetGameTime()-self.gametime>100):
                if (not self.devil.isNull()):
                    self.devil.DealDamageToHull((10,0,0),self.devil.GetHull()*100)
                VS.IOmessage (0,"game","all","[Computer] Large energy release detected...")
                VS.IOmessage (0,"game","all","[Computer] Aera Starbase destroyed!")
                VS.IOmessage (0,"game","all","[Computer] Star Confederacy Starships detected on far side of planet!")
                VS.IOmessage (5,"game","all","[Computer] Message from Rescue Fleet")
                VS.IOmessage (6,"RescueFleet","all","Thank you for your help, privateer...we owe our victory to your cunning kill of those scouts.  We offer you 24,000 credits for this critical act!")
                playa.addCredits (24000)
                self.removeQuest()
                return 0
        return 1

class quest_racene_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_racene")
    def precondition(self,playernum):
        print 'precondition'
        return 1
        return quest.findQuest(playernum,'racene_star_intro') and quest.notLoadedQuest(playernum,'racene_star_loss')
    def create (self):
        return quest_racene()
