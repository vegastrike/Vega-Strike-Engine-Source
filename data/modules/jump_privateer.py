from trading import trading
from random_encounters import random_encounters
#print "done ran"
from difficulty import difficulty
import dynamic_universe
import total_jump
#print "difficulty begin"
#from garbage_collect import garbage_collect
import Director
#print "directing"
import Briefing
#print "briefd"
class jump_privateer (Director.Mission):
    loops=()
    def __init__ (self,sigdis, detectiondis, gendis, minships, genships, fighterprob, enemyprob, capprob, credits_to_maximize_difficulty, capdist):#negative garbage collect dist disables that feature
#print "initing direct"
        Director.Mission.__init__(self)
#print "done direct"
        self.loops=(difficulty (credits_to_maximize_difficulty),
              random_encounters (sigdis, detectiondis, gendis, minships,genships,fighterprob,enemyprob,capprob,capdist),
              trading (),
              dynamic_universe,
              total_jump.total_jump()
    #          garbage_collect (),

              )

    def Execute(self): #this execute function should not need to be changed...
        for i in self.loops:
            i.Execute()
    def initbriefing(self):
        print "ending briefing"
    def loopbriefing(self):
        print "loop briefing"
        Briefing.terminate();
    def endbriefing(self):
        print "ending briefing"

#def initstarsystem():
#  random_encounters.initstarsystem() #??? that isn't there
