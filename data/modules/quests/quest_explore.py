import quest
import Vector
import VS
import unit
import universe
import vsrandom
import Director
class quest_explore (quest.quest):
    def createjumppoint(self,userealname):
        str='unknown_sector/unknown'
#Commented because people will think that it jumps to itself.
#        str=self.sysfile
        if userealname:
            str=self.newsys
        str=str.split('/')[-1]
        str=str[0].upper()+str[1:]
        str="Jump_To_"+str
        self.jumppoint=VS.launchJumppoint("Base","neutral","1000 jump.png %s (ONE ONE)"%str,"planet","sitting_duck",1,1,(861946880,2132313,31337),"",self.newsys)
        self.notcreatedyet=2
    def __init__ (self):
        self.sysfile = VS.getSystemFile()
        self.newsys=self.sysfile
        self.timer = VS.GetGameTime();
        self.notcreatedyet=0
        self.you=VS.getPlayer()
        self.playernum=self.you.isPlayerStarship()
        self.navpoint=VS.Unit()
        self.jumppoint=VS.Unit()
    def setsystem(self,sys):
        self.newsys=sys
#        if quest.findQuest(self.playernum,(self.sysfile+"_nav"),2):
#            self.createjumppoint(False)
#        elif quest.findQuest(self.playernum,(self.sysfile+"_nav"),3):
        self.createjumppoint(True)
    def Execute (self):
        sys=VS.getSystemFile()
        plyr=VS.getPlayerX(self.playernum)
        if not plyr:
            return 1
        if (sys==self.sysfile):
            if quest.findQuest(self.playernum,(self.sysfile+"_nav"),1):
                if self.notcreatedyet==0:
                    self.navpoint=VS.launch("Base","navpoint","neutral","unit","sitting_duck",1,1,(861946880,2132313,31337),"")
                    self.notcreatedyet=1
                elif self.notcreatedyet==1 and self.navpoint and VS.getPlayerX(self.playernum).getDistance(self.navpoint)<=2000:
#                    VS.IOmessage(0,'game','all','[Computer] Energy source identified as a jump point, destination: unknown')
                    self.createjumppoint(True)
                    self.navpoint.Kill()
                    Director.putSaveData(self.playernum,(self.sysfile+"_nav"),0,2)
#        elif (sys==self.newsys):
#            if not (quest.checkSaveValue(self.playernum,self.newsys+'_navpoint',0)):
#                print "ret0becausenewsys+_navpoint==0!!!!!!!!!!!"
#                return 1
#            if plyr!=self.you:
#                if self.notcreatedyet==5:
#                    VS.LoadMission('patrol/explore.mission')
#                    self.you=plyr
#            if quest.findQuest(self.playernum,(self.sysfile+"_nav"),2):
#                VS.LoadMission('patrol/explore.mission')
#                self.notcreatedyet=5
#                Director.putSaveData(self.playernum,(self.sysfile+"_nav"),0,3)
#            elif self.notcreatedyet!=5 and quest.findQuest(self.playernum,(self.sysfile+"_nav"),3):
#                VS.LoadMission('patrol/explore.mission')
#                self.notcreatedyet=5
        return 1

class quest_explore_factory (quest.quest_factory):
    def __init__ (self):
        quest.quest_factory.__init__ (self,"quest_explore")
    def create (self):
        return quest_explore()
