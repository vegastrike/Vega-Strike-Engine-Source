from go_to_adjacent_systems import *
from go_somewhere_significant import *
import vsrandom
import launch
import faction_ships
import VS
import Briefing
import universe
import unit
import Vector
import Director
import quest
class spawner (Director.Mission):
    def __init__ (self):
        Director.Mission.__init__(self)
        
        self.fac=[]
        self.facnames=["confed","homeland-security","merchant","hunter","pirates","luddites","aera","rlaan","ISO","andolian","highborn","shaper","unadorned","purist","forsaken","LIHW","klkk","mechanist","shmrn","rlaan_briin"]
        self.facnames=self.facnames+self.facnames
        for i in range(len(self.facnames)):
            nam=self.facnames[i]+"_SPAWN"
            if (i>=len(self.facnames)/2): 
               nam+="_CAPSHIP"
            self.fac.append(VS.launchJumppoint(nam,"neutral","75 jump.png %s (ONE ONE)"%nam,"planet","sitting_duck",1,1,Vector.Add((0.,-1000.+i*200.,1000.),VS.getPlayer().Position()),"",""))
        self.delay=VS.GetGameTime()
    def Execute (self):
        if (VS.getPlayer()):
            iter=0
            for base in self.fac:
                
                if (base.getDistance(VS.getPlayer())<0):
                    if (VS.GetGameTime()-self.delay>10):
                        self.delay=VS.GetGameTime()
                        type=faction_ships.getRandomFighter(self.facnames[iter])
                        if (iter>=len(self.facnames)/2):
                            type=faction_ships.getRandomCapitol(self.facnames[iter])
                        launch.launch_wave_around_unit("Shadow_"+self.facnames[iter],self.facnames[iter],type,"default",1,200,250,VS.getPlayer(),'',0)
                        VS.getPlayer().upgrade("tungsten",0,0,1,0)
                        VS.getPlayer().upgrade("tungsten_hull",0,0,1,0)
                    else:
                        VS.IOmessage(0,"spawner","all","Going to spawn "+self.facnames[iter]+"starships in %d seconds at."%(10-(VS.GetGameTime()-self.delay)))
                iter+=1