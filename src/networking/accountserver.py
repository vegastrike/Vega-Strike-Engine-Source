#!/usr/bin/env python

import cgitb
import sys
import os
import cgi
import urllib
import MySQLdb
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
	'conn_pass': '73vega',
	'table_prefix': 'phpbb_',
	}

def connect():
	return MySQLdb.connect(
		host = settings['conn_host'],
		user = settings['conn_user'],
		passwd = settings['conn_pass'],
		db     = settings['conn_db'])

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

def printField(name, value):
	print name + ' ' + str(len(value))
	print value

def getLoginQuery(conn, user, passwd):
	c = conn.cursor(MySQLdb.cursors.DictCursor)
	c.execute('SELECT logged_in_server, username, user_id, savegame, csv FROM ' + 
		tableName('users') + ' WHERE username=%s AND user_password=%s',
		(user, md5.md5(passwd).hexdigest()) )
	result = c.fetchone()
	return result

def getLoginInfo(conn, user, passwd):
	result = getLoginQuery(conn, user, passwd)
	if result:
		if result['logged_in_server']:
			print 'LOGIN_ALREADY'
			printField('username', result['username'])
			return
		if not result['savegame'] or result['csv']:
			res = getLoginQuery(conn, 'default', '')
			result['savegame'] = res['savegame']
			result['csv'] = res['csv']
		print 'LOGIN_ACCEPT'
		printField('username', result['username'])
		printField('password', passwd)
		printField('savegame', result['savegame'])
		printField('csv', result['csv'])
		c = conn.cursor()
		c.execute('UPDATE '+tableName('users')+' SET logged_in_server=logged_in_server+1 WHERE ' +
			'username=%s', (user))
	else:
		print 'LOGIN_ERROR'
		printField('username', user)
		return

def loginAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	passwd = post.get('password','')
	if not (user and passwd) or user == 'default':
		print 'LOGIN_ERROR\nIncorrect username or password'
		return
	getLoginInfo(conn, user, passwd)

def logoutAccount(conn, path, get, post):
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	passwd = post.get('password','')
	if not (user and passwd) or user == 'default':
		return
	c = conn.cursor()
	c.execute('UPDATE '+tableName('users')+' SET logged_in_server=0 WHERE ' +
		'username=%s AND user_password=%s', (user, md5.md5(passwd).hexdigest()))

def addAccount(conn, path, get, post):
	print 'Register at the <a href="/forums/profile.php?mode=register">Forum</a>'
	return
	if len(path)<1 or not path[0]:
		user = get.get('user', post.get('user', ''))
	else:
		user = path[0]
	passwd = post['password']
	if not (user and passwd):
		print 'Invalid user/password combination'
		return
	maxquery = conn.cursor()
	maxquery.execute('SELECT MAX(user_id) FROM ' + tableName('users'))
	answer_row = maxquery.fetchone()
	user_id = answer_row[0]+1
	c = conn.cursor()
	c.execute('INSERT INTO ' + tableName('users') +
		' (username, user_id, user_password) VALUES (%s, '+str(int(user_id))+', %s)',
		(user, md5.md5(passwd).hexdigest()))
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

handler_function = {
	'user': queryAccount,
	'signup': addAccount,
	'login': loginAccount,
	'logout': logoutAccount,
	'': indexPage
	}

if __name__=='__main__':
	print "Content-Type: text/html"
	print ""
	cgitb.enable()
	conn = connect()
	if not conn:
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
		path_handler(conn, path_info, urlDecode(get_args), urlDecode(post_args))

