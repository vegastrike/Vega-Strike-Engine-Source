import vsrandom
import launch
import faction_ships
import VS
import Briefing
import universe
import unit
import Director
class total_war (Director.Mission):
    def __init__(self):
        print "init1INITNITNITNTINTINTINTINTITNINTINTINTINTINT\nININTINTINTITNITNITNTINTINTINTINTITNITNITNITNITNITNITNITNITNIT"
        VS.SetDifficulty(.9)
        Director.Mission.__init__(self)
        self.lasttime=-1000
        self.waittime=10.0
    def launch_new_wave(self):
        side = vsrandom.randrange(0,2)
        faction="confed"
        ai = vsrandom.randrange(0,8)
        if (0 and ai==0):
            ai = "printhello.py"
        else:
            ai = "default"
        if (side==0):
            faction=faction_ships.get_enemy_of("confed")
        else:
            faction="merchant"
        launched = launch.launch_wave_around_unit ("Shadow",faction,faction_ships.getRandomFighter(faction),ai,vsrandom.randrange(1,5),100.0,2000.0,VS.getPlayer(),'')
        if (vsrandom.randrange(0,10)==0 or faction=="luddites"):
            launch.launch_wave_around_unit ("ShadowCap"+str(vsrandom.randrange(0,10)),faction,faction_ships.getRandomCapitol(faction),ai,1,2000.0,4000.0,VS.getPlayer(),'')
    def Execute (self):
        un=VS.getUnit(0);
        i=0
        while (un):
            i+=1
            un=  VS.getUnit(i)
        time = VS.GetGameTime()
        if (time-self.lasttime>self.waittime):
            self.launch_new_wave()
            self.waittime=vsrandom.randrange(4.0,6.0)
            self.lasttime=time
    def initbriefing(self):
        print "ending briefing"
    def loopbriefing(self):
        print "loop briefing"
        Briefing.terminate();
    def endbriefing(self):
        print "ending briefing"
