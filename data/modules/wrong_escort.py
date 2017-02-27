import escort_mission
import faction_ships
class wrong_escort (escort_mission.escort_mission):
	def __init__ (self,factionname,missiondifficulty,distance_from_base,creds,numsysaway,jumps=(),var_to_set='',dynfg='',dyntype='',alternatesystems=(),alternatefactions=(),alternateflightgroups=(),alternatetypes=(),alternategreeting=(),alternatevariable='diverted'):
		escort_mission.escort_mission.__init__(self,factionname,missiondifficulty,0,0,distance_from_base,creds,0,numsysaway,jumps,var_to_set,dynfg,dyntype)
		self.alternatesystems=alternatesystems
		self.alternatevariable=alternatevariable
		self.alternateflightgroups=alternateflightgroups
		self.alternatetypes=alternatetypes
		self.alternategreeting=alternategreeting
		self.alternatefactions=alternatefactions
		import quest
		import VS
		self.cp = VS.getCurrentPlayer()
		quest.removeQuest(self.cp,alternatevariable,-1)
	def Execute(self):
		escort_mission.escort_mission.Execute(self)
		sys = self.escortee.getUnitSystemFile()
		if sys in self.alternatesystems:
			for i in range(len(self.alternatesystems)):
				if sys==self.alternatesystems[i]:
					import quest
					quest.removeQuest(self.cp,self.alternatevariable,i)
					quest.removeQuest(self.cp,self.var_to_set,-1)
					import VS
					import launch	
					L = launch.Launch()
					L.fg="Escorts"	
					L.faction=self.alternatefactions[i]
					L.dynfg=self.alternateflightgroups[i]
					L.type=self.alternatetypes[i]
					L.ai="default"
					L.num=6
					L.minradius=3000.0
					L.maxradius=4000.0
					try:
						L.minradius*=faction_ships.launch_distance_factor
						L.minradius*=faction_ships.launch_distance_factor
					except:
						pass
					launched=L.launch(self.escortee)
					self.escortee.setFgDirective('f')
					self.escortee.setFlightgroupLeader(launched)
					self.escortee.setFactionName(self.alternatefactions[i])
					import universe					
					universe.greet(self.alternategreeting[i],launched,VS.getPlayerX(self.cp))
					VS.terminateMission(1)
					return

		
