#!/usr/bin/env python

import cgitb
import sys
import os
import cgi
import urllib

import db
import settings

conn = db.connect(settings.dbconfig)

def urlDecode(args):
	argsplit = args.split('&')
	arglist = {}
	for arg in argsplit:
		if not arg:
			continue
		argsp= arg.split('=')
		name = urllib.unquote(argsp[0])
		if len(argsp)>1:
			value = urllib.unquote(argsp[1])
		else:
			value = ''
		arglist[name] = value
	return arglist

class Packet:
	def __init__(self, input=''):
		self.input = input;
		self.pos = 0
	
	def getChar(self):
		retval=self.input[self.pos];
		self.pos += 1
		return retval;
	
	def getInt(self):
		where=self.input.find(" ", self.pos);
		if (where!=-1):
			num=self.input[self.pos:where];
			try:
				len=int(num);
			except:
				len=0
			self.pos = where+1
			return len;
		return 0;
	
	def getString(self):
		leng=self.getInt()
		if leng:
			retval=self.input[self.pos:self.pos+leng]
			self.pos += leng
			return retval
		return ""
	
	def getCheckedString(self):
		return conn.check_string(self.getString())
	
	def addString(self, adder):
		self.addInt(len(adder));
		self.input+=adder;
	
	def addStringField(self, name, value):
		self.addString(value);
	
	def addChar(self, adder):
		self.input+=str(adder);
	
	def addInt(self, adder):
		self.input+=str(adder)+" ";
	
	def __str__(self):
		return self.input
	
	def __repr__(self):
		return repr(self.input)
	
	def __len__(self):
		return len(self.input)

ACCT_LOGIN='l'
ACCT_LOGIN_DATA='d'
ACCT_LOGIN_ERROR='e'
ACCT_LOGIN_ALREADY='f'
ACCT_LOGIN_NEW='n'
ACCT_LOGIN_ACCEPT='a'
ACCT_LOGOUT='o'
ACCT_SAVE_LOGOUT='S'
ACCT_RESYNC='r'
ACCT_SAVE='s'
ACCT_NEWCHAR='c'
ACCT_NEWSUBSCRIBE='u'
ACCT_SUCCESS='!'

defaultsave="""Crucible/Cephid_17^13500.000000^llama.begin 0 0 0 neutral
0 mission data  4
31337ness 2 0.050000 0.050000
436457r1K3574r7uP71m35 1 5.000000
stardate 1 151.199997
unit_to_dock_with 10 1.000000 0.000000 116.000000 108.000000 97.000000 110.000000 116.000000 105.000000 115.000000 0.000000 
0 missionstring data 0
0 python data 1 0
0 news data 1
"""

def getDefaultSave():
	try:
		return conn.open_default_file(".save")
	except IOError:
		return defaultsave;
def getDefaultXML():
	try:
		return conn.open_default_file(".xml")
	except IOError:
		return ",Directory,Name\nKey,string,string\nllame,llame,llame\n"

def getSystem(savegame):
	where=savegame.find("^")
	if where==-1:
		return "testsystems/absolutelynothing.system"
	else:
		return savegame[0:where]
def getServer(system):
	sysstr = conn.check_string(system.replace("/","-"))
	if not sysstr:
		return "0.0.0.0:4364"
	try:
		f=open(settings.data_path+sysstr+".system","r")
	except IOError:
		try:
			f=open(settings.data_path+"testsystems-absolutelynothing.system","r")
		except IOError:
			return "0.0.0.0:4364"
	ret=f.read()
	f.close()
	return ret
def getLoginInfo(conn, user, passwd, dologin):
	p = Packet()
	result = conn.get_login_info(user, passwd)
	if result:
		if 0 and dologin and result['logged_in_server']:
			#server doesn't support LOGIN_ALREADY very well.
			p.addChar(ACCT_LOGIN_ALREADY)
			p.addStringField('username', user)
			print p
			return
		if not (result['savegame'] and result['csv']):
			defcsv=getDefaultXML()
			defsavegame=getDefaultSave()
			result['savegame'] = defsavegame
			result['csv'] = defcsv
			#result['csv'] = res['csv']
		serverstrings=getServer(getSystem(result['savegame'])).split(":")
		if dologin:
			conn.set_connected(user,True)
			p.addChar(ACCT_LOGIN_ACCEPT)
			p.addStringField('username',user)
			p.addStringField('password',passwd)
			p.addStringField('serverip',serverstrings[0])
			p.addStringField('serverport',serverstrings[1])
			p.addStringField('savegame', result['savegame'])
			p.addStringField('csv', result['csv'])
		else:
			p.addChar(ACCT_LOGIN_DATA)
			p.addStringField('username',user)
			p.addStringField('password',passwd)
			p.addStringField('serverip',serverstrings[0])
			p.addStringField('serverport',serverstrings[1])
		print p
		return
	else:
		p.addChar(ACCT_LOGIN_ERROR)
		p.addStringField('username', user)
		print p
		return

# Common environment variables:
# QUERY_STING (after '?')
# SCRIPT_URL (/cgi-bin/accountserver/query/user)
# SCRIPT_URI (above, including http://vegastrike.sourceforge.net)
# HTTP_HOST (vegastrike.sourceforge.net)
# REMOTE_ADDR/HOST (remote address.. could be used for finding fastest server?)
# CONTENT_TYPE/LENGTH (POST data)
# PATH_INFO (extra path info after CGI script [/cgi-bin/accountserver/test/blah => /test/blah])
def vegastrike(conn,url,post):
	try:
		packet = Packet(post) #if it crashes we dont care..this is python
		command = packet.getChar()
	except IndentationError:
		command='UNKNOWN'
		print "EXCEPTION: "+str(url)+"\n"+str(post);
		packet = Packet('')
	if command==ACCT_LOGIN:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		getLoginInfo(conn,username,password,True);
	elif command==ACCT_LOGIN_DATA:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		getLoginInfo(conn,username,password,False);
	elif command==ACCT_NEWSUBSCRIBE:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		# Unimplemented?
		#if conn.check_password(conn,username,password,True):
		#	conn.modify_account()
		print ACCT_SUCCESS
	elif command==ACCT_RESYNC:
		pass
	elif command==ACCT_SAVE:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		save=packet.getString()
		xml=packet.getString()
		if conn.check_password(username, password):
			conn.save_account(username,save,xml)
		print ACCT_SUCCESS
	elif command==ACCT_SAVE_LOGOUT:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		save=packet.getString()
		xml=packet.getString()
		if conn.check_password(username, password):
			conn.save_account(username,save,xml)
			conn.set_connected(username, False)
		print ACCT_SUCCESS
	elif command==ACCT_LOGOUT:
		username=packet.getCheckedString()
		password=packet.getCheckedString()
		if conn.check_password(username, password):
			conn.set_connected(username, False)
		print ACCT_SUCCESS
	else:
		if command==ACCT_NEWCHAR:
			pass
		print "UNKNOWN"
	
if __name__=='__main__':
	print "Content-Type: text/html"
	print ""
	cgitb.enable()
	post_args = ''
	if os.environ['REQUEST_METHOD'] == 'POST':
		leng = os.environ['CONTENT_LENGTH']
		post_args = sys.stdin.read(int(leng))
	get_args = os.environ['QUERY_STRING']
	vegastrike(conn, urlDecode(get_args), post_args)
