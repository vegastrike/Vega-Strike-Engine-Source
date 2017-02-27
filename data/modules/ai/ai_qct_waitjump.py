import VS
import sys
import quest
import Vector
import unit
import vsrandom
import save_util
import faction_ships
import universe
import launch
import quest_contraband_truck


class waitjump(VS.PythonAI):

    def init(self,un):
        self.XMLScript ("++flystraight.xml")
        self.AddReplaceLastOrder(1)
        self.timer = 0
        self.got_target = 0
        self.autoed = 0

    def ChooseTarget(self):
        return

    def Execute(self):
        VS.PythonAI.Execute(self);
        if quest_contraband_truck.truck_exit == 1:
            if self.got_target == 0:
                self.GetParent().SetTarget(universe.getRandomJumppoint())
                self.trucktarget = (self.GetParent()).GetTarget()
                self.XMLScript ("++afterburn-turntowards.xml")
                self.AddReplaceLastOrder(1)
                self.got_target = 1

# starts him afterburning to target

            if self.timer == 0:
                self.timer = VS.GetGameTime()
                print "Timer Set"
            elif self.timer + 30 < VS.GetGameTime() and self.autoed == 0:
                self.GetParent().ActivateJumpDrive(1)
                self.GetParent().AutoPilotTo(self.trucktarget,1)
                self.autoed = 1
            elif self.timer + 5 < VS.GetGameTime():
                self.MoveTo(self.trucktarget.Position(),1)
                self.AddReplaceLastOrder(1)
#                       elif self.timer + 60 < VS.GetGameTime():
# gets him to auto to the jump and jump out
#                               self.GetParent().ActivateJumpDrive(1)
            print self.GetParent().getMinDis(self.trucktarget.Position())

        return 1
hi1 = waitjump()
print 'AI creation successful'
hi1 = 0
