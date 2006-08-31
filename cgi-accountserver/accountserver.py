#!/usr/bin/env python
USE_SQL=False
import cgitb
import sys
import os
import cgi
import urllib
try:
	import MySQLdb
except:
	USE_SQL=False
import md5

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

settings = {
	'conn_hostname': 'mysql4-v.sourceforge.net', # hostname broken?
	'conn_host': '10.5.1.16',
	'conn_user': 'v19507rw',
	'conn_db':   'v19507_vegastrike',
	'table_prefix': 'phpbb_',
	}
def getSimpleChar(input):
  retval=input[0][0];
  input[0]=input[0][1:];
  return retval;

def getSimpleInt(input):
  where=input[0].find(" ");
  if (where!=-1):
    num=input[0][0:where];
    try:
      len=int(num);
    except:
      len=0
    input[0]=input[0][where+1:];
    return len;
  return 0;  
def getSimpleString(input):
	leng=getSimpleInt(input)
	if leng:
		retval=input[0][0:leng]
		input[0]=input[0][leng:]
		return retval
	return ""
def addSimpleString(input, adder):
  addSimpleInt(input,len(adder));
  input[0]+=adder;
def addSimpleChar(input, adder):
  input[0]+=str(adder)
def addSimpleInt(input, adder):
  input[0]+=str(adder)+" ";

def connect():
	if USE_SQL:
		return MySQLdb.connect(
			host = settings['conn_host'],
			user = settings['conn_user'],
			passwd = settings['conn_pass'],
			db     = settings['conn_db'])
	else:
		return None

def tableName(table):
	return settings.get('table_prefix', '') + table

def queryAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', '')
	else:
		user = path[0]
	if not user:
		print 'No user.'
		return
	c = conn.cursor(MySQLdb.cursors.DictCursor)
	c.execute('SELECT username, user_id, logged_in_server, user_real_name FROM ' +
		tableName('users') + ' WHERE username=%s',
		( user ,) )
	print '<table>'
	for result in c:
		for item in result:
			print '<tr><td>%s</td><td>%s</td></tr>' % (item, result[item])
	print '</table>'
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
HOMEPATH='/tmp/persistent/vegastrike_forum/accounts/'#
try:
	os.mkdir(HOMEPATH[0:-1])	
except:
	pass
def checkStringPrint(s):
  foundyet=False
  for c in s:
    if "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-0123456789.".find(c)==-1:
        if not foundyet:
            print header
            foundyet=True
        print "Error: invalid character "+c+" in input "+s+"<br>"
  if s.find("..")!=-1 or s.find(".xml")!= -1 or s.find(".save")!=-1 or s.find("accounts")!=-1 or s.find("default")!=-1:
    if not foundyet:
        print header
        foundyet=True
    print "Error: invalid characters .. or .xml or .save in input "+s+"<br>"
  if foundyet:
    print errorvar
    print footer
    sys.exit(0)

def checkString(s):
  foundyet=False
  for c in s:
    if "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-0123456789.".find(c)==-1:
        if not foundyet:
            foundyet=True
        return False
  if s.find("..")!=-1 or s.find(".xml")!= -1 or s.find(".save")!=-1 or s.find("accounts")!=-1 or s.find("default")!=-1:
    if not foundyet:
        foundyet=True
    return False
  if foundyet:
    return False
  return True

def printField(name, value):
	print name + ' ' + str(len(value))
	print value
def stringField(name, value):
	return str(len(value))+' '+value;

def getLoginQuerySQL(conn, user, passwd):
	c = conn.cursor(MySQLdb.cursors.DictCursor)
	c.execute('SELECT logged_in_server, username, user_id, savegame, csv FROM ' + 
		tableName('users') + ' WHERE username=%s AND user_password=%s',
		(user, md5.md5(passwd).hexdigest()) )
	result = c.fetchone()
	return result
def getLoginQueryFS(conn,user,password):
	result={}
	f=None
	if checkString(user):
		try:
			f=open(HOMEPATH+user+".password","rb")
			tpass=f.read()

			f.close()
			if tpass==password:
				try:
					f=open(HOMEPATH+user+".save","rb")
					result['savegame']=f.read()
					f.close()
				except IOError:
					result['savegame']=None
				try:
					f=open(HOMEPATH+user+".xml","rb")
					result['csv']=f.read()
					f.close()
				except IOError:
					result['csv']=None
				try:
					f=open(HOMEPATH+user+'.logged',"rb")
					result['logged_in_server']=f.read()
					f.close()
				except IOError:
					result['logged_in_server']=None
				return result#should back this ass up
		except IOError:
			print "Unpossible, file not found"
	return None
def getLoginQuery(conn,user,password):
	if USE_SQL:
		return getLoginQuerySQL(conn,user,password)
	else:
		return getLoginQueryFS(conn,user,password)
def setConnectedAcctSQL(conn,user,passwd,setOne):
	c = conn.cursor()
	if setOne:
		c = conn.cursor()
		c.execute('UPDATE '+tableName('users')+' SET logged_in_server='#+'logged_in_server+'
			+'1 WHERE ' + 'username=%s', (user))
		
	else:
		c.execute('UPDATE '+tableName('users')+' SET logged_in_server=0 WHERE ' +
			'username=%s AND user_password=%s', (user, md5.md5(passwd).hexdigest()))
def setConnectedAcctFS(conn,user,passwd,setOne):
	if checkString(user):
		try:
			f=open(HOMEPATH+user+".password","rb")
		except IOError:
			print "Unpossible, password file "+HOMEPATH+user+".password"+" not found"
			return
		tpass=f.read()
		f.close()
		if tpass==passwd:
			f=open(HOMEPATH+user+".logged","wb")
			if setOne:
				f.write("1")
			f.close()#empty file
def setConnectedAcct(conn,user,passwd,b):
	if USE_SQL:
		setConnectedAcctSQL(conn,user,passwd,b)
	else:
		setConnectedAcctFS(conn,user,passwd,b)

def logoutAcct(conn,user,passwd):
	if USE_SQL:
		setConnectedAcctSQL(conn,user,passwd,False)
	else:
		setConnectedAcctFS(conn,user,passwd,False)
def getDefaultFile(ext):
	f=open(HOMEPATH+"default"+ext,"r")
	ret=f.read()
	f.close()
	return ret#demands that these files exist
defaultdefault="""Crucible/Cephid_17^13500.000000^llama.begin 0 0 0 neutral
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
		return getDefaultFile(".save")
	except IOError:
		return defaultdefault;
def getDefaultXML():
	try:
		return getDefaultFile(".xml")
	except IOError:
		return ",Directory,Name\nKey,string,string\nllame,llame,llame\n"

def getSystem(savegame):
	where=savegame.find("^")
	if where==-1:
		return "testsystems/absolutelynothing.system"
	else:
		return savegame[0:where]
def getServer(system):
	if not checkString(system.replace("/","-")):
		return "0.0.0.0:4364"
	try:
		f=open(HOMEPATH+system.replace("/","-")+".system","r")
	except IOError:
		try:
			f=open(HOMEPATH+"testsystems-absolutelynothing.system","r")
		except IOError:
			return "0.0.0.0:4364"
	ret=f.read()
	f.close()
	return ret
def getLoginInfo(conn, user, passwd, dologin):
	result = getLoginQuery(conn, user, passwd)
	if result:
		if result['logged_in_server']:
			print ACCT_LOGIN_ALREADY+stringField('username', user)
			return
		if not (result['savegame'] and result['csv']):
			defcsv=getDefaultXML()
			defsavegame=getDefaultSave()
			result['savegame'] = defsavegame
			result['csv'] = defcsv
			#result['csv'] = res['csv']
		serverstrings=getServer(getSystem(result['savegame'])).split(":")
		if dologin:
			setConnectedAcct(conn,user,passwd,True)
			print ACCT_LOGIN_ACCEPT+stringField('username',user)+stringField('password',passwd)+stringField('serverip',serverstrings[0])+stringField('serverport',serverstrings[1])+stringField('savegame', result['savegame'])+stringField('csv', result['csv'])
		else:
			print ACCT_LOGIN_DATA+stringField('username',user)+stringField('password',passwd)+stringField('serverip',serverstrings[0])+stringField('serverport',serverstrings[1])
	else:
		print ACCT_LOGIN_ERROR+stringField('username', user)
		return

def loginAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	try:
		passwd = post['password']
	except:
		passwd=path[1]
	if not (user and passwd) or user == 'default':
		print user
		print passwd
		print 'LOGIN_ERROR\nIncorrect username or password'
		return
	getLoginInfo(conn, user, passwd)

def logoutAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	try:
		passwd = post['password']
	except:
		passwd=path[1]
	if not (user and passwd) or user == 'default':
		return
	logoutAcct(conn,user,passwd)
def addAcctSQL(conn,user,passwd):
	maxquery = conn.cursor()
	maxquery.execute('SELECT MAX(user_id) FROM ' + tableName('users'))
	answer_row = maxquery.fetchone()
	user_id = answer_row[0]+1
	c = conn.cursor()
	c.execute('INSERT INTO ' + tableName('users') +
		' (username, user_id, user_password) VALUES (%s, '+str(int(user_id))+', %s)',
		(user, md5.md5(passwd).hexdigest()))
def addAcctFS(conn,user,passwd):
	if checkString(user):
		f=open(HOMEPATH+user+".password","wb")
		f.write(passwd)
		f.close()

def addAcct(conn,user,passwd):
	if USE_SQL:
		addAcctSQL(conn,user,passwd)
	else:
		addAcctFS(conn,user,passwd)
def saveAcctSQL(conn,user,passwd,save,xml):
	maxquery = conn.cursor()
	maxquery.execute('SELECT MAX(user_id) FROM ' + tableName('users'))
	answer_row = maxquery.fetchone()
	user_id = answer_row[0]+1
	c = conn.cursor()
	c.execute('UPDATE ' + tableName('users') +
		" SET savegame = '%s', csv = '%s' WHERE username=%s AND user_password=%s",
		(save,xml,user, md5.md5(passwd).hexdigest()))#this has to be wrong..svegames have spaces and stuff

def saveAcctFS(conn,user,password,save,csv):
	result={}
	f=None
	if checkString(user):
		try:
			f=open(HOMEPATH+user+".password","rb")
			tpass=f.read()
			f.close()
			if tpass==password:
				f=open(HOMEPATH+user+".save","wb")
				f.write(save)
				f.close()
				f=open(HOMEPATH+user+".xml","wb")
				f.write(csv)
				f.close()
		except IOError:
			print "Unpossible the password file not found\n"


def saveAcct(conn,user,passwd,save,xml):
	if USE_SQL:
		saveAcctSQL(conn,user,passwd,save,xml)
	else:
		saveAcctFS(conn,user,passwd,save,xml)

def addAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	try:
		passwd = post['password']
	except:
		passwd=path[1]
	if not (user and passwd):
		print 'Invalid user/password combination'
		return
	addAcct(conn,user,passwd)
	#user_id = c.insert_id()
	print 'Your user, '+user+' was created as '+str(user_id)+'.'

def indexPage(conn, path, get, post):
	print '''<!DOCTYPE html public>
<html>
 <head>
  <title>Vega Strike Accountserver</title>
 </head>
 <body bgcolor="white" color="black" link="blue" vlink="purple" alink="red">
  <h1>Welcome to the Vega Strike Accountserver</h1>
  <h2>Query a user</h2>
  <form action="user" method="GET">
   Username:
   <input name="user" type="text"><input type="submit" value="Go!">
  </form>
  <h2>Sign up</h2>
  <p>You should sign up as a forum user from the <a href="/forums/profile.php?mode=register">forum</a></p>
  <form method="POST" action="signup" onsubmit="javascript:if (this.password.value==this.password2.value) return true; else {alert('Passwords do not match');return false;}"><table>
   <tr><td align="right">Username:</td><td><input name="user" type="text"></td></tr>
   <tr><td align="right">Password:</td><td><input name="password" type="password"></td></tr>
   <tr><td align="right">Password confirm:</td><td><input name="password2" type="password"></td></tr>
   <tr><td></td><td><input type="submit" value="Add a user!"></td></tr>
  </table></form>
  <h2>Log in</h2>
  <p>test123</p>
  <form method="POST" action="login" ><table>
   <tr><td align="right">Username:</td><td><input name="user" type="text"></td></tr>
   <tr><td align="right">Password:</td><td><input name="password" type="password"></td></tr>
   <tr><td></td><td><input type="submit" value="Log it in!"></td></tr>
  </table></form>
  <form method="POST" action="logout" ><table>
   <tr><td align="right">Username:</td><td><input name="user" type="text"></td></tr>
   <tr><td align="right">Password:</td><td><input name="password" type="password"></td></tr>
   <tr><td></td><td><input type="submit" value="Log me out"></td></tr>
  </table></form>
  <h2>Last ten users</h2>
  <table>
   <tr><td>No idea how to do this anyway :-P</td></tr>
  </table>
 </body>
</html>
'''

# Common environment variables:
# QUERY_STING (after '?')
# SCRIPT_URL (/cgi-bin/accountserver/query/user)
# SCRIPT_URI (above, including http://vegastrike.sourceforge.net)
# HTTP_HOST (vegastrike.sourceforge.net)
# REMOTE_ADDR/HOST (remote address.. could be used for finding fastest server?)
# CONTENT_TYPE/LENGTH (POST data)
# PATH_INFO (extra path info after CGI script [/cgi-bin/accountserver/test/blah => /test/blah])
def vegastrike(conn,path,url,post):
	try:
		command=post[0]#if it crashes we dont care..this is python
		post=[post[1:]]
	except:
		command='UNKNOWN'
		print "EXCEPTION: "+str(url)+"\n"+str(post);
		post=[post]
	if command==ACCT_LOGIN:
		username=getSimpleString(post)
		password=getSimpleString(post)
		getLoginInfo(conn,username,password,True);
	elif command==ACCT_LOGIN_DATA:
		username=getSimpleString(post)
		password=getSimpleString(post)
		getLoginInfo(conn,username,password,False);
	elif command==ACCT_LOGOUT:
		username=getSimpleString(post)
		password=getSimpleString(post)
		logoutAcct(conn,username,password);
		print "!"
	elif command==ACCT_NEWSUBSCRIBE:
		username=getSimpleString(post)
		password=getSimpleString(post)
		addAcct(conn,username,password)
		print "!"		
	elif command==ACCT_RESYNC:
		pass
	elif command==ACCT_SAVE:
		username=getSimpleString(post)
		password=getSimpleString(post)
		save=getSimpleString(post)
		xml=getSimpleString(post)
		saveAcct(conn,username,password,save,xml)
		print "!"
	elif command==ACCT_SAVE_LOGOUT:
		username=getSimpleString(post)
		password=getSimpleString(post)
		save=getSimpleString(post)
		xml=getSimpleString(post)
		saveAcct(conn,username,password,save,xml)
		logoutAcct(conn,username,password)
		print "!"
	else:
		if command==ACCT_NEWCHAR:
			pass
		print "UNKNOWN"
	
handler_function = {
	#'user': queryAccount,
	#'signup': addAccount,
	#'login': loginAccount,
	#'logout': logoutAccount,
	'vegastrike':vegastrike,
	#'': indexPage
	}

if __name__=='__main__':
	print "Content-Type: text/html"
	print ""
	cgitb.enable()
	conn = connect()
	if not conn and USE_SQL:
		print 'Failed to connect to database'
	path_info = os.environ['PATH_INFO'].split('/')
	if len(path_info)>1:
		path_handler = handler_function[path_info[1]]
	else:
		path_handler = handler_function['']
	path_info = path_info[2:]
	post_args = ''
	if os.environ['REQUEST_METHOD'] == 'POST':
		leng = os.environ['CONTENT_LENGTH']
		post_args = sys.stdin.read(int(leng))
	get_args = os.environ['QUERY_STRING']
	if path_handler:
		path_handler(conn, path_info, urlDecode(get_args),post_args)
