#!/usr/bin/env python

import cgitb
#;print "Content-Type: text/html\n";cgitb.enable()
import sys
import os

import db
import settings

class Packet:
	def __init__(self, conn, input=''):
		self.input = input;
		self.conn = conn;
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
		return self.conn.check_string(self.getString())
	
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

def getDefaultSave(conn):
	return conn.get_default_save()
def getDefaultXML(conn,shipname):
	return conn.get_default_csv(shipname)

def getShipname(savegame):
	where = savegame.find("^")
	space = savegame.find(" ")
	if where==-1 or space==-1:
		return "llama.begin"
	else:
		return savegame[where+1:space]

def getSystem(savegame):
	where=savegame.find("^")
	if where==-1:
		return "testsystems/absolutelynothing.system"
	else:
		return savegame[0:where]
def getLoginInfo(conn, user, passwd, dologin):
	p = Packet(conn)
	result = conn.get_login_info(user, passwd)
	if result:
		if 0 and dologin and result['logged_in_server']:
			#server doesn't support LOGIN_ALREADY very well.
			p.addChar(ACCT_LOGIN_ALREADY)
			p.addStringField('username', user)
			print p
			return
		if not (result['savegame'] and result['csv']):
			defsavegame=getDefaultSave(conn)
			defcsv=getDefaultXML(conn,getShipname(defsavegame))
			result['savegame'] = defsavegame
			result['csv'] = defcsv
			#result['csv'] = res['csv']
		serverstrings=conn.get_server(getSystem(result['savegame'])).split(":")
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
get_form=False
post_form=False
def execute(conn,url,post):
	print '\r'
	sys.stderr.write('Executing post query: '+post+'\n')
	try:
		packet = Packet(conn, post) #if it crashes we dont care..this is python
		command = packet.getChar()
	except:
		command='UNKNOWN'
		print "EXCEPTION: "+str(url)+"\n"+str(post);
		packet = Packet(conn, '')
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
	cgitb.enable()
	get_args = os.environ.get('QUERY_STRING','')
	conn = db.connect(settings.dbconfig, get_args)
	post_args = ''
	if os.environ.get('REQUEST_METHOD','GET') == 'POST':
		leng = os.environ['CONTENT_LENGTH']
		post_args = sys.stdin.read(int(leng))
	#get_args = db.urlDecode(get_args)
	execute(conn, get_args, post_args)

