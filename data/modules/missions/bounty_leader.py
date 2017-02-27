import bounty
import faction_ships
class bounty_leader(bounty.bounty):
	def __init__ (self,minnumsystemsaway, maxnumsystemsaway, creds, run_away, shipdifficulty, tempfaction,jumps=(),var_to_set='',dynfg='',dyntype="",displayLocation=1,dynhelperfg='',dynhelpertype='',greetingText=['It appears we have something in common, privateer.','My name may be on your list, but now your name is on mine.'],leader_upgrades=[],dockable_unit=False):
		bounty.bounty.__init__(self,minnumsystemsaway,maxnumsystemsaway,creds,run_away,0,tempfaction,jumps,var_to_set,dynfg,dyntype,displayLocation,greetingText,dockable_unit)
		self.helpertype=dynhelpertype
		self.numhelpers=shipdifficulty
		self.helperfg=dynhelperfg
		self.upgrades=leader_upgrades
	def Execute(self):
		bounty.bounty.Execute(self)
	def LaunchedEnemies(self,significant):
		bounty.bounty.LaunchedEnemies(self,significant)
		import launch
		L = launch.Launch()
		L.fg="Shadow"
		L.dynfg=self.helperfg
		L.faction = self.faction
		if type(self.helpertype)==type(""):
			L.type = self.helpertype
		else:
			L.type = self.helpertype[0]
			L.faction=self.helpertype[1]
		if L.type=="":
			L.type=faction_ships.getRandomFighter(L.faction)
		L.ai = "default"
		L.num=self.numhelpers
		L.minradius=3000.0
		L.maxradius = 4000.0
		try:
			L.minradius*=faction_ships.launch_distance_factor
			L.maxradius*=faction_ships.launch_distance_factor
		except:
			pass
		if (self.numhelpers):
			tempship=L.launch(significant)
			tempship.SetTarget(self.you)
		for u in self.upgrades:
			args=(u,0,0,False)
			if (type(u) is list or type (u) is tuple):
				if (len(u)>3):
					args=u
				elif (len(u)>2):
					args=(u[0],u[1],u[2],False)
				elif (len(u)>1):
					args=(u[0],u[1],0,False)
				else:
					args=(u[0],0,0,False)
			self.enemy.upgrade(str(args[0]),int(args[1]),int(args[2]),1,args[3])