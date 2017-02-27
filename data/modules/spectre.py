import VS
import Director
import Vector
import vsrandom
def isCar(c):
    nam = c.getName()
    return nam=='car' or nam == 'spectre' or nam=='porsche' or nam=='skart'

class Environment(Director.Mission):
    def toonear (self,vec,un):
        vec= Vector.Sub(un.Position(),vec)
        if (Vector.Dot(vec,vec)<1000):
            return True
        return False
    def nearflag(self,un,flag):
        return un.getDistance(flag)<0
    def TwoDdis (self,uvec,f):
        fvec=f.Position()
        x=fvec[0]-uvec[0];
        y=fvec[2]-uvec[2];
        return x*x+y*y
    def tweakHeight (playa,sub):
        self.unitheight= sub.Position()[1]
    def scoreFlag(self,un,f):
        f.Kill()
        VS.IOmessage(0,"game","all","Got a flag")
    def checkUn (self):
        for X in range (getNumPlayers()):
            un=getPlayerX (X)
            uvec=un.Position()
            rad = un.rSize()
            rad=rad*rad
            isnull=False
            for f in self.flags:
                if (not f):
                    isNull=True
                else:
                    if (TwoDdis(uvec,f)<rad):
                        scoreFlag (un,f)
            for u in self.subs:
                if (not f):
                    isNull=True
                else:
                    if (TwoDdis(uvec,u)<rad):
                        tweakHeight(un,u)
            if (isNull):
                for i in range(len(self.subs)):
                    if (not self.subs[i]):
                        del self.subs[i]
                        return
                for j in range(len(self.flags)):
                    if (not self.flags[j]):
                        del self.flags[j]
                        return

    def randomFlagLocation(self):
        import vsrandom
        if (len(self.flags)):
            i = vsrandom.randrange(0,len(self.flags))
            return Vector.Add((vsrandom.uniform(-20,20),vsrandom.uniform(-20,20),vsrandom.uniform(-20,20)),self.flags[i].Position())
        return (0,0,0)

    def randomLocation(self):
        import vsrandom
        for i in range (50):
            vec=(vsrandom.uniform(-self.arenasize,self.arenasize),
                     0,
                     vsrandom.uniform(-self.arenasize,self.arenasize))
            for i in self.flags:
                if (self.toonear (vec,i)):
                    continue
            for i in self.obstacles:
                if (self.toonear (vec,i)):
                    continue
            return vec
        return vec
    def CreateFlag (self,name):
        un = VS.launch("flags",name,"neutral","unit","default",1,1,self.randomLocation(),'')
        self.flags.append(un)

    def CreateObstacle(self,name):
        un = VS.launch("flags",name,"neutral","unit","default",1,1,self.randomLocation(),'')
        self.obstacles.append(un)
    def CreateSub(self,name):
        un = VS.launch("flags",name,"neutral","unit","default",1,1,self.randomLocation(),'')
        self.subs.append(un)

    def CreateUnit (self, name, defend):
        evenodd= (2*(self.num_un %2) - 1)
        if defend:
            mod='defend'
            vec=self.randomFlagLocation()
        else:
            mod='macho'
            vec=self.randomLocation()
        un = VS.launch ("cybernet",name,"unknown", "unit",mod,1,1,vec,'')
        un.SetOrientation ((0,1,0),(0,0,evenodd))
        self.num_un+=1
    def __init__ (self,numobst, numflag,numspec,numseek, numsubs):
        Director.Mission.__init__(self)
        self.unitheight=0
        self.arenasize=1000
        self.flags=[]
        self.obstacles=[]
        self.subs=[]
        print 'initing'
        self.iter=0
        self.num_un=0
        for i in range(numobst):
            self.CreateObstacle("box")
        for i in range(numflag):
            self.CreateFlag("flag")
        import vsrandom
        for i in range(numspec):
            self.CreateUnit ("spectre",vsrandom.randrange(0,2))
        for i in range(numseek):
            self.CreateUnit ("skart",0)
        for i in range (numsubs):
            self.CreateSubs("subs");
    def Execute(self):
        un = VS.getUnit (self.iter)
        if (un):
            if (isCar(un)):
                self.ApplyEventualEnvironment (un,VS.getPlayer())
            self.iter+=1
        else:
            self.iter=0
    def EventualExecture (self):
        self.Execute()
    def AlwaysExecute(self):
        self.EventualExecute()
        iter = VS.getUnitList ()
        un = iter.current()
        playa = VS.getPlayer()
        while (un):
            if (isCar(un)):
                self.ApplyPerFrameEnvironment (un,playa)
            iter.advance()
            un = iter.current()
    def ApplyEventualEnvironment(self,un,playa):
        unheight=0
        if (un.getName()=='skart' or un.isPlayerStarship()!=-1):
            unheight= self.unitheight
        pos=(un.Position()[0],unheight,un.Position()[2])
        un.SetCurPosition (pos)
#       def ApplyPerFrameEnvironment (self,un,playa):
#               pos = un.Position()
        vel = un.GetVelocity()
#               if (pos[1]!=0):
#                       pos=(pos[0],0,pos[2])
#                       un.SetCurPosition(pos)
        if (vel[1]!=0):
            vel = (vel[0],0,vel[2])
            un.SetVelocity(vel)
#               if (vel[0]!=0):
#                       vel = (0,vel[1],vel[2])
#                       un.SetVelocity(vel)
