import VS
import unit
import cleansweep
class cleansweep_escort(cleansweep.cleansweep):
	def __init__(self,numsystemsaway, num_points,distance,creds,jumps,donevar,minships,maxships,encounterprob,capshipprob,faction, forceattack,canrunaway, friendlyfaction,allygreetingtext=[],lastnum=3,enemygreetingtext=[]):
		cleansweep.cleansweep.__init__(self,numsystemsaway,num_points,distance,creds,jumps,donevar,minships,maxships,encounterprob,capshipprob,faction,forceattack,canrunaway)
		self.friendlyfaction=friendlyfaction
		self.allygreetingtext=allygreetingtext
		self.launchedyet=False
		self.ally=None
		self.lastnum=lastnum
		self.allyobj=0
		self.enemygreetingtext=enemygreetingtext
		self.engreetedyet=0
		self.enemygreetingtime=0
	def LaunchAlly(self):
		import launch
		import faction_ships
		import universe
		L = launch.Launch()
		L.faction=self.friendlyfaction
		L.fg="Patrol_Wing"
		L.dynfg=""
		L.minradius=3000.0
		L.maxradius=4000.0
		try:
			L.minradius*=faction_ships.launch_distance_factor
			L.maxradius*=faction_ships.launch_distance_factor
		except:
			pass
		L.num=1
		L.ai="default"
		L.type = faction_ships.getRandomCapitol(self.friendlyfaction)
		self.ally = L.launch(self.you)
		self.ally.setMissionRelevant()
		self.allyobj=VS.addObjective("Protect the %s"%unit.getUnitFullName(self.ally))
		L.type = faction_ships.getRandomFighter(self.friendlyfaction)
		import vsrandom
		L.num=vsrandom.randrange(3,6)
		L.launch(self.you)

		universe.greet(self.allygreetingtext,self.ally,self.you)
	
	def Execute(self):
		if (self.adjsys.HaveArrived()):
			if (self.you and not self.launchedyet):
				self.launchedyet=True
				self.LaunchAlly()
			else:
				if (not self.ally):
					VS.setCompleteness(self.allyobj,-1.0)
		cleansweep.cleansweep.Execute(self)
	def DeletePatrolPoint(self,num,nam):
		tmp=self.encounterprob
		if (self.engreetedyet!=1 or VS.GetGameTime()>self.enemygreetingtime+10):
			self.enemygreetingtime=VS.GetGameTime()
			self.engreetedyet+=1
		if (len(self.patrolpoints)==1 and self.launchedyet):
			if (self.ally):
				pos= self.you.Position()
				pos=(pos[0],pos[1]+2*self.ally.rSize(),pos[2]+self.you.rSize()*2+2*self.ally.rSize())
				self.ally.SetPosition(pos)#move him nearby to last nav point
				self.encounterprob=1.0
				self.minships=self.lastnum
				self.maxships=self.lastnum
		elif(self.engreetedyet==2):
			import universe
			universe.greet(self.enemygreetingtext,None,self.you)
			self.encounterprob=1.0
		cleansweep.cleansweep.DeletePatrolPoint(self,num,nam)
		self.encounterprob=tmp
	def RealSuccessMission(self):
		if (self.ally or not self.launchedyet):
			cleansweep.cleansweep.RealSuccessMission(self)
		else:
			self.FailMission()
			