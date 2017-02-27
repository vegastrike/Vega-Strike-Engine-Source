import VS
import Director
import Vector
import faction_ships
import launch
import quest
#FIXME: need a way to ensure target maintains radio silence!
class tail (Director.Mission):
    
    WAITTIME=float(240)#must be float, how long to wait after enemy jumps before failure
    DELAYTIME=float(30)#how long to wait, after success, before friendlies come to finish job
    
    def __init__ (self,var_to_set,creds,direct,sdist,mdist,efaction,ffaction,efg,ffg,fnum=3,edyntype='',fdyntype='',edynfg='',fdynfg='',tooclose=["Thought you could hide from me?", "Try hiding from this!"],toofar=["We have lost the lock on the target vessel."],justright=["Thankyou.  You help in tracking this criminal has been appreciated."]):
        Director.Mission.__init__ (self)
        print 'tail: Starting'
        
        self.fail1 = tooclose
        self.fail2 = toofar
        self.succeed1 = justright
        
        self.enemyfg = efg
        self.enemyfaction = efaction
        self.enemydynfg = edynfg
        self.enemydyntype = edyntype
        self.enemynum=1#don't have this as multiple, as I haven't compensated for it when I move fgs
        self.enemy=None
        
        self.friendlyfg = ffg
        self.friendlyfaction = ffaction
        self.friendlydynfg = fdynfg
        self.friendlydyntype = fdyntype
        self.friendlynum=fnum

        
        self.success=False
        self.waiting=False
        self.tracking=(None,None)#(missiontimeout, distanceonjump)
        
        self.objref=None
        
        self.cp = VS.getCurrentPlayer()
        self.started = False
        self.directions = direct
        self.creds = creds
        self.var_to_set = var_to_set
        self.mindist = mdist
        self.startdist = sdist
        VS.getPlayerX(self.cp).upgrade("jump_drive",0,0,0,1)
        print 'tail: Started'
    
    def setupEnemy (self):
        if not self.started:
            print 'Tail Error checking: '+self.directions[0]
            print 'Tail Error checking: '+self.directions[0].lower()
        if self.started:
            return True
        elif self.directions[0].lower().find(VS.getPlayerX(self.cp).getUnitSystemFile().lower())!=-1:
                L = launch.Launch()
                L.fg=self.enemyfg
                L.dynfg=self.enemydynfg
                if self.enemydyntype=='':
                    self.enemydyntype=faction_ships.getRandomFighter(self.enemyfaction)
                L.type = self.enemydyntype
                L.faction = self.enemyfaction
                L.ai = "default"
                L.num=self.enemynum
                L.minradius=self.mindist
                L.maxradius=self.startdist
                try:
                    L.minradius*=faction_ships.launch_distance_factor
                    L.maxradius*=faction_ships.launch_distance_factor
                except:
                    pass
                self.enemy=L.launch(VS.getPlayerX(self.cp))
                self.enemy.upgrade("jump_drive",0,0,0,1)
                self.relation = self.enemy.getRelation(VS.getPlayerX(self.cp))
                self.enemy.setCombatRole("INERT")
                self.updateEnemyObjective()
                VS.IOmessage (0,"[Mission Computer]","all","Target %s detected!  Proceed as per mission instructions."%self.enemy.getFullname())
                self.objref=VS.addObjective("Follow the %s until it broadcasts the signal"%self.enemy.getFullname())
                self.started=True
                return True
        print "now NOT set up!"
        return False
    
    def setupFriendly (self, around):
                L = launch.Launch()
                L.fg=self.friendlyfg
                L.dynfg=self.friendlydynfg
                if self.friendlydyntype=='':
                    self.friendlydyntype=faction_ships.getRandomFighter(self.friendlyfaction)
                L.type = self.friendlydyntype
                L.faction = self.friendlyfaction
                L.ai = "default"
                L.num=self.friendlynum
                L.minradius=0
                L.maxradius=self.mindist
                try:
                    L.minradius*=faction_ships.launch_distance_factor
                    L.maxradius*=faction_ships.launch_distance_factor
                except:
                    pass
                friendly=L.launch(around)
                self.enemy.setCombatRole("FIGHTER")
                flead = friendly.getFlightgroupLeader()
                flead.SetTarget(self.enemy)
                flead.Threaten(self.enemy,1)
                self.enemy.Threaten(flead,1)
                friendly.setFgDirective('A.')
                friendly.DeactivateJumpDrive()
    
    def updateEnemyObjective(self):
        j = self.nextSystem()
        if j:
            targ=self.getJumpTo(j)
            if not targ:
                raise RuntimeError("Jumppoint going to %s cannot be found."%j)
            self.enemy.SetTarget(targ)
            self.enemy.ActivateJumpDrive(0)
            print "tail: Enemy running to "+targ.getName()
#            self.enemy.setFlightgroupLeader(targ)
#            self.enemy.setFgDirective('B')
#            self.enemy.setFgDirective('F')
#            self.enemy.AutoPilotTo(targ,True)
            print 'enemy objective updated '+j
            return targ
        return None
    
    def nextSystem(self,uni=None):
        if uni is None:
            uni_=self.enemy
        else:
            uni_=uni
        sys=uni_.getUnitSystemFile()
        nextsys=False
        for i in range(len(self.directions)-1):
            where=sys.find(self.directions[i])
            if (where>0):
                if (sys[where-1]=='/'):
                    where=0
            if (where==0):
                nextsys=self.directions[i+1]
                break
        return nextsys
    
    def getJumpTo(self,jto):
        jto=jto[1:]
        jto=jto[jto.find('/')+1:]
        itera = VS.getUnitList()
        while itera.notDone():
            if itera.current().isJumppoint():
                if itera.current().getName().lower().find(jto.lower())!=-1:
                    return itera.current()
            itera.advance()
        return False
    
    def getPlacementVectors(self, target):
        relpos=Vector.Sub(target.Position(),VS.getPlayerX(self.cp).Position())
        unitrel=Vector.Scale(relpos, 1.0/Vector.Mag(relpos))
        approx=list()
        approx.append(Vector.Scale(unitrel, self.tracking[1]))
        approx.append(Vector.Scale(relpos, (VS.GetGameTime() - self.tracking[0] + self.WAITTIME)/self.WAITTIME))
        for i in range(len(approx)):
            if approx[i] < self.mindist or approx[i] > Vector.Mag(relpos):
                approx[i] = False
        biggest=(0,0,0)
        for i in range(len(approx)):
            if approx[i]:
                if Vector.Mag(approx[i]) > Vector.Mag(biggest):
                    biggest=approx[i]
        if Vector.Mag(biggest) < 1:#arbitrary check to see if we've got a valid result
#            raise RuntimeError("Could not find a suitable approximation for unit placement.")
#            Runtime errors are ... less than optimal.  Lets pick a vector and hope.
            print 'tail: Pick a vector and hope'
            unitrel = (1,0,0)
            biggest = (self.startdist,0,0)
        return unitrel, biggest
    
    def doPlacement(self, target):
        unitvec, vec = self.getPlacementVectors(target)
        self.enemy.SetPosition(Vector.Add(VS.getPlayerX(self.cp).Position(),vec))
        self.enemy.SetOrientation((1,0,0),vec)
        self.enemy.SetVelocity(Vector.Scale(unitvec,1440.0))
    
    def SetVar (self,val):
        if (self.var_to_set!=''):
            quest.removeQuest (self.cp,self.var_to_set,val)
    
    def initSuccess (self):
        if self.success == 2:
            if VS.GetGameTime() > self.tracking[0]:
                t=self.getJumpTo(self.directions[len(self.directions)-2])
                self.setupFriendly(t)#FIXME: open wormhole
                VS.IOmessage (0,"[Mission Command]","all","Out of our way, let us finish the job!")
                VS.getPlayerX(self.cp).addCredits(self.creds)
                VS.terminateMission(1)
        else:
            t=self.getJumpTo(self.directions[len(self.directions)-2])
            self.enemy.SetPosition(Vector.Add(Vector.Scale((1,0,0),11063),t.Position()))
            VS.getPlayerX(self.cp).SetPosition(t.Position())#because self.enemy is at the jumpgate when I jump, so I get pushed off
            VS.setCompleteness(self.objref, 1.00)
            self.success=2
            VS.IOmessage (0,"[Mission Computer]","all","Broadcast intercepted.")
            for i in range(len(self.succeed1)):
                VS.IOmessage (i+1,"[Mission Command]","all",self.succeed1[i])
            self.SetVar(1)
            self.tracking=(VS.GetGameTime()+self.DELAYTIME, VS.getPlayerX(self.cp).getDistance(self.enemy))
        return
    
    def enemyThreatened (self):
        """Does the enemy feel different about the player?  If yes, the player
           must have talked to, or shot at the enemy."""
        if abs(self.enemy.getRelation(VS.getPlayerX(self.cp)) - self.relation) >= 0.003:
            print "tail: the relation between the player and target has changed"
            return True
        return False
    
    def outOfRange (self):
        print "tail: Out of range"
        i=0
        for j in range(len(self.fail2)):
            VS.IOmessage (j,"[Mission Command]","all",self.fail2[j])
            i+=1
        VS.IOmessage(i,"[Mission Computer]","all","You are not able to detect the target.")
        VS.IOmessage (i+1,"[Mission Computer]","all","Mission Failed.")
        self.fail()
    
    def tooClose (self):
        print "tail: Too close"
        VS.IOmessage (0,"[Mission Computer]","all","You have been detected by the target %s "%self.enemy.getFullname())
        VS.IOmessage (1,"[Mission Computer]","all","Mission Failed.")
        for i in range(len(self.fail1)):
            VS.IOmessage (i+2,"Target","all",self.fail1[i])
        self.enemy.SetTarget(VS.getPlayerX(self.cp))
        self.enemy.setFgDirective('A')
        self.fail()
    
    def tooDead (self):
        print "tail: Too dead"
        VS.IOmessage (0,"[Mission Computer]","all","The target %s has been destroyed."%self.enemy.getFullname())
        VS.IOmessage (1,"[Mission Computer]","all","Mission Failed.")
        self.fail()
    
    def fail (self):
        self.SetVar(-1)
        VS.setCompleteness(self.objref, -1.00)
        VS.terminateMission(0)
        return
    
    def Execute (self):
        you=VS.getPlayerX(self.cp)
        if you.isNull():
            print 'you is null'
            self.fail()
        elif self.success:
            self.initSuccess()
        elif self.setupEnemy():
            if self.enemy.isNull():
                self.tooDead()
            if self.waiting:
                if self.tracking[0] < VS.GetGameTime():
                    self.outOfRange()
                elif VS.getPlayerX(self.cp).getUnitSystemFile() == self.enemy.getUnitSystemFile():
                    self.waiting = False
                    t = self.updateEnemyObjective()
                    VS.IOmessage (0,"[Mission Computer]","all","Target %s detected in this system."%self.enemy.getFullname())
                    VS.setObjective(self.objref,"Follow the %s until it broadcasts the signal"%self.enemy.getFullname())
                    if t is not None:
                        self.doPlacement(t)
                    else:
                        self.success=True
            elif VS.getPlayerX(self.cp).getUnitSystemFile() != self.enemy.getUnitSystemFile():
                self.enemy.DeactivateJumpDrive()
                self.waiting = True
                self.tracking=(VS.GetGameTime()+self.WAITTIME, VS.getPlayerX(self.cp).getDistance(self.enemy))
                VS.IOmessage (0,"[Mission Computer]","all","Target %s detected entering %s"%(self.enemy.getFullname(),self.nextSystem(VS.getPlayerX(self.cp))))
                VS.setObjective(self.objref,"Follow the %s into %s"%(self.enemy.getFullname(),self.nextSystem(VS.getPlayerX(self.cp))))
            elif VS.getPlayerX(self.cp).getDistance(self.enemy)<self.mindist or self.enemyThreatened():
                self.tooClose()
