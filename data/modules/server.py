import VS
import Director
import unit
import sys
import traceback

import custom

import server_lib

baseunits={}

def baseUnits():
	system = VS.getSystemFile()
	if system in baseunits:
		return baseunits[system]
	else:
		baselist=[]
		unitlist=VS.getUnitList()
		while unitlist.notDone():
			un = unitlist.current()
			#print 'Looking at unit '+un.getName()
			#print un.getFlightgroupName()
			#print un.isSun()
			#print un.isPlanet()
			if unit.isBase(un) or (un.isPlanet() and not un.isSun()):
				#print 'appending!'
				baselist.append(un)
			unitlist.advance()
		baseunits[system]=baselist
		return baselist
	

def getDocked(un):
	#print 'getDocked('+un.getName()
	bases = baseUnits()
	for b in bases:
		if b:
			#print 'testing '+b.getName()
			#print b.isDocked(un)
			#print un.isDocked(b)
			if b.isDocked(un):
				#print 'foundocked unit'
				return b
		#else:
		#	print 'null unit in bases'
		#	print b.getName()
	#print 'return none'
	return None

class objectives:
	def __init__(self):
		pass

_server_inst=None

def getDirector():
	return _server_inst

class server (Director.Mission):
	def __init__(self):
		Director.Mission.__init__(self)
		
		global _server_inst
		_server_inst=self
		
		self.playerlist=[]
		self.loopnum=0
		self.loops=[]
		
		server_lib.server_init(self)
	
	def playerJoined(self, pl):
		self.playerlist.append(pl)
	def getPlayerBy(self, func):
		for pl in self.playerlist:
			if func(pl):
				return pl
	def getPlayer(self, num):
		return self.getPlayerBy(lambda pl: pl.player_num==num)
	def getPlayerByCallsign(self, callsign):
		return self.getPlayerBy(lambda pl: pl.callsign==callsign)
	def getCallsignList(self):
		ret=[]
		for p in self.playerlist:
			if p.callsign:
				ret.append(p.callsign)
		return ret
	def Execute(self):
		self.loopnum+=1
		try:
			for i in self.loops:
				i.Execute()
			server_lib.server_execute(self)
		except:
			traceback.print_exc(file=sys.stderr)
	

class player (Director.Mission):
	def __init__(self):
		Director.Mission.__init__(self)
		_server_inst.playerJoined(self)
		self.reinit()
	
	def reinit(self):
		self.current_un=VS.Unit()
		self.objectives=0
		self.player_num=-1
		self.callsign=''
		self.ship=''
		self.docked_un=None
	
	def sendMessage(self, msg, fromname='game'):
		if self.player_num>=0:
			cpstr = 'p'+str(self.player_num)
			VS.IOmessage(0,fromname,cpstr,msg)
	
	def Execute(self):
		player_un = VS.getPlayer()
		if self.current_un.isNull() and player_un.isNull():
			VS.clearObjectives()
			self.reinit()
		if not player_un:
			return
		if self.current_un != player_un:
			self.player_num = player_un.isPlayerStarship()
			self.current_un = player_un
			self.callsign = player_un.getFlightgroupName()
			self.ship = player_un.getName()
			if not self.callsign:
				self.callsign = self.ship
			try:
				server_lib.player_joined(self)
			except:
				traceback.print_exc(file=sys.stderr)
		dockedUn = getDocked(self.current_un)
		if dockedUn is not self.docked_un:
			if dockedUn:
				self.docked_un = dockedUn
				try:
					server_lib.player_docked(self)
				except:
					traceback.print_exc(file=sys.stderr)
			else:
				try:
					server_lib.player_undocked(self)
				except:
					traceback.print_exc(file=sys.stderr)
				self.docked_un = dockedUn
		try:
			server_lib.player_execute(self)
		except:
			traceback.print_exc(file=sys.stderr)
	
	
def processMessage(cp, localhost, command, arglist=None, id=''):
	try:
		cmd = command
		args = arglist
		if not arglist:
			subcmds = command.split(' ')
			cmd = subcmds[0]
			args = subcmds[1:]
		cmd = cmd.lower()
		authlevel=0
		if localhost:
			authlevel=2 #Can exit the game.
		elif cp>=0:
			if Director.getSaveDataLength(cp, 'serveradmin')>0:
				if Director.getSaveData(cp, 'serveradmin', 0)==1:
					authlevel=1
		else:
			return
		pl = _server_inst.getPlayer(cp)
		if cmd=='reload1':
			if authlevel<1:
				return
			mod=server_lib
			reload(mod)
			VS.IOmessage(0,"game","all","The server python script has been reloaded!")
			print mod.__name__+' has been reloaded!'
		else:
			server_lib.processMessage(pl, authlevel, cmd, args, id)
	except:
		if cp<0:
			writer = sys.stderr
		else:
			writer = custom.IOmessageWriter(cp)
		argstr=''
		if type(arglist)=='list':
			argstr = ' ' + (' '.join(arglist))
		writer.write("An error occurred when processing command: \n"
			+ str(command)+argstr)
		traceback.print_exc(file=writer)

