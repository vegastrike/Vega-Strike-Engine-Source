# -*- coding: utf-8 -*-
import VS
import Director
import directions_mission
import launch
import faction_ships
import universe

class ambush(directions_mission.directions_mission):
	def privateSetupPlayer(self):
		self.timer=0
		self.inescapable=0
		self.havelaunched=0
		self.terminated=0

	def __init__(self,savevar,systems,delay,faction,numenemies,dyntype='',dynfg='',greetingText=["Hello there, smuggler. Prepare to die!", "The price on your head is big enough that I missed my lunch"], directions=[], destination='',AdjustFaction=True):
		directions_mission.directions_mission.__init__ (self,savevar,directions,destination)
		print 'Ambush: Starting'
		self.faction=faction
		self.systems=systems
		if type(systems)!=tuple and type(systems)!=list :
			self.systems=(systems,)
		self.numenemies=numenemies
		self.dyntype=dyntype
		self.dynfg=dynfg
		self.greetingText=greetingText
		self.cp=VS.getCurrentPlayer()
		self.delay=delay
		self.privateSetupPlayer()
		self.AdjustFaction=AdjustFaction
	def setupPlayer(self,cp):
		print "ambush setting player up"
		directions_mission.directions_mission.setupPlayer(self,cp)
		self.privateSetupPlayer()

	def Launch(self,you):
		if (self.havelaunched==0):
			if (type(self.numenemies)==type(1)):
				self.numenemies=(self.numenemies,)
				self.faction=(self.faction,)
				self.dyntype=(self.dyntype,)
				self.dynfg=(self.dynfg,)
			if (type(self.AdjustFaction)!=type( () ) and type (self.AdjustFaction)!=type([])):
				self.AdjustFaction=(self.AdjustFaction,)
			for i in xrange(len(self.faction)):
				numenemies=self.numenemies[i]
				faction=self.faction[i]
				for z in xrange(numenemies):
					AdjustFaction=self.AdjustFaction[-1]
					if (i<len(self.AdjustFaction)):
						AdjustFaction=self.AdjustFaction[i]
					dynfg=""
					if (len(self.dynfg)>i):
						dynfg=self.dynfg[i]
					dyntype=""
					if (len(self.dyntype)>i):
						dyntype=self.dyntype[i]
					print 'Ambush: Launch ships!'
					self.havelaunched=1
					L=launch.Launch()
					L.fg="Shadow"
					if (dyntype==""):
						dyntype=faction_ships.getRandomFighter(faction)
					L.dynfg=dynfg
					L.type=dyntype
					L.num=1
					L.fgappend="X"
					L.minradius=6000
					L.maxradius=8000
					try:
						L.minradius*=faction_ships.launch_distance_factor
						L.maxradius*=faction_ships.launch_distance_factor
					except:
						pass
					L.faction=faction
					enemy=L.launch(you)
					lead=enemy.getFlightgroupLeader()
					enemy.SetTarget(you)
					if (lead):
						lead.SetTarget(you)
					else:
						enemy.setFlightgroupLeader(enemy)
					enemy.setFgDirective("A.")
					self.enemy=lead			
					rel=VS.GetRelation(faction,"privateer")
					if (AdjustFaction and rel>=0):
						VS.AdjustRelation(faction,"privateer",-.02-rel,1.0)
						rel=VS.GetRelation("privateer",faction)
						VS.AdjustRelation("privateer",faction,-.02-rel,1.0)
					if (i==len(self.faction)-1 and z==0):
						universe.greet(self.greetingText,enemy,you)
					#print "launchin"
					print 'Ambush: Ships have been launched. Exiting...'
	def terminate(self):
		self.terminated=1#VS.terminateMission(0)
	def Execute(self):
		directions_mission.directions_mission.Execute(self)
		if (self.terminated==1):
			return
		you=VS.getPlayerX(self.cp)
		if you.isNull():
			return
		sys=you.getUnitSystemFile()
		if(not self.inescapable):
			for i in self.systems:
				where=sys.find(i)
				if (where>0):
					if (sys[where-1]=='/'):
						where=0
				if (where==0):
					#print 'Ambush: wait before launching ship...'
					self.inescapable=1
					self.timer=VS.GetGameTime()
		if (self.inescapable and ((self.delay==0) or (VS.GetGameTime()-self.timer>=self.delay))):
			self.Launch(you)
			self.terminate()
#					print "it's unavoidable, my young apprentice... in "+str(self.delay)+" seconds from "+str(self.timer)
			
