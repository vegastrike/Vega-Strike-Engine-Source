import VS
import Director
def isCar(c):
    nam = c.getName()
    return nam=='car' or nam=='porsche'

class Environment(Director.Mission):
    def CreateUnit (self, name):
        evenodd= (2*(self.num_un %2) - 1)
        x_coord = -evenodd  * 30
        un = VS.launch ("traffic",name,"neutral", "unit","modules/traffic.py",1,1,(x_coord,0,self.num_un*100),'')
        un.SetOrientation ((0,1,0),(0,0,evenodd))
        self.num_un+=1
    def __init__ (self,numcar, numspec):
        Director.Mission.__init__(self)
        print 'initing'
        self.iter=0
        self.num_un=0
        for i in range(numcar):
            self.CreateUnit ("porsche")
        for i in range(numspec):
            self.CreateUnit ("car")
    def Execute(self):
        un = VS.getUnit (self.iter)
        if (un):
            if (isCar(un)):
                self.ApplyEventualEnvironment (un,VS.getPlayer())
            self.iter+=1
        else:
            self.iter=0
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
        if (playa):
            if (playa!=un):
                posdiff = playa.Position()[2]-un.Position()[2]
                if ((posdiff*(2*(posdiff>0)-1))>1500):
                    R = un.GetOrientation ()[2]

                    if (posdiff*R[2]<0):
                        pos=(un.Position()[0],0,playa.Position()[2]+4000*(2*(R[2]<0)-1),)
                        un.SetCurPosition (pos)
                        print "repositioning"
#       def ApplyPerFrameEnvironment (self,un,playa):
        pos = un.Position()
        vel = un.GetVelocity()
        if (pos[1]!=0):
            pos=(pos[0],0,pos[2])
            un.SetCurPosition(pos)
        if (vel[1]!=0):
            vel = (vel[0],0,vel[2])
            un.SetVelocity(vel)
        if (vel[0]!=0):
            vel = (0,vel[1],vel[2])
            un.SetVelocity(vel)
