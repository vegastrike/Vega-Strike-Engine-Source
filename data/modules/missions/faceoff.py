import vsrandom
import launch
import faction_ships
import VS
import Briefing
import universe
import unit
import Director
class faceoff (Director.Mission):
    def __init__(self):
        VS.SetDifficulty(0.1)
        Director.Mission.__init__(self)
        self.lasttime=-100
        self.waittime=100.0
        self.launchShip("nova","confed")
        self.launchShip("nova","confed")
        self.launchShip("revoker","pirates")
        self.launchShip("revoker","pirates")
        self.launchShip("revoker","pirates")

    def launchShip (self,type,faction):
        return launch.launch_wave_around_unit("Shadow",faction,type,"default",1,100.0,2000.0,VS.getPlayer(),'')
    def launch_new_wave(self):
        side = vsrandom.randrange(0,2)
        faction="confed"
        ai = vsrandom.randrange(0,2)
        if (ai==0):
            ai = "printhello.py"
        else:
            ai = "default"
        if (side==0):
            faction=faction_ships.get_enemy_of("confed")
        else:
            faction=faction_ships.get_friend_of("confed")
        launched = launchShip(faction_ships.getRandomFighter(faction),faction);

    def Execute (self):
        time = VS.GetGameTime()
        if (time-self.lasttime>self.waittime):
            #self.launch_new_wave()
            VS.IOmessage (0,"game","all","[Confed] This is a test of the emergency broadcast system.")
            self.lasttime=time
