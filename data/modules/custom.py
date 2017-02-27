import VS
import traceback
import sys

procedures = {
	}

def add(name, proc):
	procedures[name.lower()] = proc

import weapons_lib #adds to procedures list
import guilds #adds to procedures list
import campaign_lib
import dialog_box
import computer

running_cmds = {}

maxid = 2
def generateID():
	global maxid;
	maxid=maxid+1
	return str(maxid)

def splitArgs(argstr):
	ret=[]
	while argstr:
		arg = ''
		qadd = ''
		empty = True
		while argstr and argstr[0]=='"':
			end=argstr.find('"', 1)
			if end!=-1:
				arg += qadd + argstr[1:end]
				qadd='"'
				argstr = argstr[end+1:]
				empty = False
			else:
				arg = argstr[1:]
				empty = False
				argstr = ''
		space = argstr.find(' ')
		if space != -1:
			arg += argstr[:space]
			argstr = argstr[space+1:]
		else:
			arg += argstr
			argstr = ''
		if arg or not empty:
			ret.append(arg)
	return ret

def joinArgs(arglist):
	ret = ''
	for arg in arglist:
		if ret:
			ret += ' '
		arg = str(arg).replace('\'','') # Remove all single-quotes
		space = arg.find(' ')
		quote = arg.find('"')
		newstr = arg.replace('"','""')
		if not newstr or space!=-1 or quote!=-1:
			ret += '"' + newstr + '"'
		else:
			ret += newstr
	return ret

def putFunction(continuation, id, cp):
	global running_cmds;
	if not id:
		id = generateID()
	key = str(cp)+","+id
	running_cmds[key] = continuation
	return id

def getFunction(id, cp):
	key = str(cp)+","+id
	if running_cmds.has_key(key):
		func = running_cmds[key]
		del running_cmds[key]
		return func
	return None

# custom.run should be the last thing that happens in a function/
# it might either be synchronous or asynchronous (this could be considered a bug)

def run(cmd, args, continuation, id=None, cp=-1):
	if -1==cp:
		cp = VS.getCurrentPlayer()
	if continuation:
		id = putFunction(continuation, id, cp)
	if not isinstance(id,str):
		id = "null"
	print "running: "+cmd+", "+str(args)+"; id: "+id
	VS.sendCustom(cp, cmd, joinArgs(args), id)
	return id

def respond(args, continuation, id, cp=-1):
	run("response", args, continuation, id, cp)

class IOmessageWriter:
	def __init__(self,cpnum):
		self.line=''
		if cpnum<0:
			self.cpstr='all'
		else:
			self.cpstr = 'p'+str(cpnum)
	def write(self, text):
		lines = text.split('\n')
		self.line=lines[-1]
		lines = lines[:-1]
		for l in lines:
			VS.IOmessage(0,"game",self.cpstr,l)
	

def processMessage(local, cmd, argstr, id):
	cp = VS.getCurrentPlayer();
	cmd = cmd.lower()
	print "======= Processing message "+str(id)+" ======="
	try:
		args = splitArgs(argstr)
		print "Command: "+cmd
		for arg in args:
			print arg
		if cmd=='reloadlib' and local and len(args)>=1:
			reload(__import__(args[0]))
			VS.IOmessage(0, "game", "p"+str(cp), "Reloaded "+str(args[0]))
		elif cmd=='local':
			# simple way of bouncing back message to client....
			if id:
				def localresponse(args):
					respond(args, None, id, cp)
			else:
				localresponse = None
			run(args[0], args[1:], localresponse, id, cp)
		elif (cmd=='response'):
			func = getFunction(id, cp)
			if func:
				ret = func(args)
				if ret and isinstance(ret, tuple) and len(ret)==2:
					respond(ret[0], ret[1], id, cp)
				elif ret==True:
					putFunction(func, id, cp)
				elif ret:
					respond(ret, None, id, cp)
		elif procedures.has_key(cmd):
			ret = procedures[cmd](local, cmd, args, id)
			if ret and isinstance(ret, tuple) and len(ret)==2:
				respond(ret[0], ret[1], id, cp)
			elif ret:
				respond(ret, None, id, cp)
		elif VS.isserver():
			import server
			server.processMessage(cp, local, cmd, args, id)
		else:
			print "Command "+repr(cmd)+" does not exist. Available functions:"
			print procedures.keys()
	except:
		if id or cp<0:
			writer = sys.stderr
		else:
			writer = IOmessageWriter(cp)
		writer.write("An error occurred when processing custom command: \n"
			+ str(cmd)+" "+argstr + "\n")
		traceback.print_exc(file=writer)
	print "-------------------------- " +str(id)+" -------"

