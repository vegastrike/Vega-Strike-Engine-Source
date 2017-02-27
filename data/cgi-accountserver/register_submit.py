#! /usr/bin/python

import sys
import os
import string
import cgitb; cgitb.enable()
import db
import settings

header = """
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title>Account Creation Results</title>
</head>
<body>
"""

footer = """
</body>
</html>
"""

get_form=False
post_form=True
def execute(dbconn, mod, form):
	query = ''
	if mod:
		query = '?'+mod
	print "Content-Type: text/html"
	print ""
	
	errorvar="<br><a href=\"register.py"+query+"\">Return To Account Creation Form</a>"
	
	success = False
	failed_error = "Unknown error"
	
	try:
		username = dbconn.check_string( form.get("username",'') )
		password = dbconn.check_string( form.get("password",'') )
		faction = dbconn.check_string( form.get("faction",'') )
		type = dbconn.check_string( form.get("type",'') )
		
		if not dbconn.check_password(username, password, True):
			raise db.DBError,"Error password for username "+username+" does not match our records"
		
		dbconn.modify_account(username, type, faction)
		success = True
	except db.DBError:
		success = False
		failed_error = sys.exc_value
	
	print header
	print '<br>'
	if success:
		url = 'vsconfig.py?username='+username+'&password='+password+'&mod='+mod
		print "To play, start up the game, click on 'Multiplayer', type in your login and password, and then click 'Join Game'"
		#print 'Download a functional'
		#print '<a href="'+url+'" title="Download config file">vegastrike.config</a>'
		#print 'to put in your vegastrike folder that has your appropriate login and password<br>'
		#print 'Or, <a href="'+url+'&action=view">view the config file</a> in your browser.<br>'
	else:
		print '<b>Registration Error</b>: ' + str(failed_error)
		print errorvar
	
	print footer

if __name__=='__main__':
	post_args = {}
	if os.environ.get('REQUEST_METHOD','GET') == 'POST':
		leng = os.environ['CONTENT_LENGTH']
		post_args = db.urlDecode(sys.stdin.read(int(leng)))
	if post_args.has_key("mod"):
		mod = post_args["mod"]
	else:
		mod = os.environ.get('QUERY_STRING','')
	conn = db.connect(settings.dbconfig, mod)
	if post_args:
		execute(conn, mod, post_args)

