# -*- coding: utf-8 -*-
import VS
import Director
import launch
from Vector import Add

class balancer (Director.Mission):
    loops=()
    def __init__ (self, ships, num_ships=1, num_waves=1):
        Director.Mission.__init__(self)
        you = VS.getPlayer()
        for i in xrange(len(ships)):
            launch.launch(you.getFlightgroupName(), you.getFactionName(), ships[i], "ai_sitting_duck.py", num_ships, num_waves, Add(you.Position(),((i+1)*500,0,0)), '', False)
            print(" ++ Balancer mission launching %s unit..."%ships[i])
        launch.launch(you.getFlightgroupName(), you.getFactionName(), "Archimedes", "ai_sitting_duck.py", num_ships, num_waves, Add(you.Position(),(0,0,8000)), '', False)
        print(" ++ Balancer mission launching the playground")

    def Execute(self):
        pass
