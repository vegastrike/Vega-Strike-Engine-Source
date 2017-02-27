#!/usr/bin/python
import cgitb; cgitb.enable()
import db
import os

def myreplace(s,str,replacer):
	where = s.find('"'+str+'"')
	if where==-1:
		return s
	keyfind='value="'
	where2=s[where:].find(keyfind)
	if where2==-1:
		return s
	where2+=len(keyfind)+where
	where3=s[where2:].find('"')
	if where3==-1:
		return s
	where3+=where2
	return s[:where2]+replacer+s[where3:]

def comment(s, str):
	pos=s.find(str)
	if pos==0 or pos==-1:
		return s
	prevendcomment = s.rfind('-->', 0, pos)
	prevbegincomment = s.rfind('<!--', 0, pos)
	if prevbegincomment!=-1:
		if prevendcomment==-1 or prevendcomment<prevbegincomment:
			return s
	return s[:pos]+'<!--'+str+'-->'+s[pos+len(str)+1:]

get_form=True
post_form=False
def execute(filedb, form, post_args):
	
	username = form["username"]
	password = form["password"]
	
	f=filedb.open_default_file("vegastrike.config")
	s=f.read()
	f.close()
	
	s = comment(s, '<bind key="y" modifier="none" command="SwitchCombatMode" />')
	#s = comment(s, '<bind key="a" modifier="none" command="ToggleWarpDrive" />');
	
	s=myreplace (s,"callsign",username);
	s=myreplace (s,"password",password);
	s=myreplace (s,"realname",username);
	s=myreplace (s,"force_client_connect","true")
	s=myreplace (s,"default_mission","net.mission")
	
	# Make browser treat this as an XML file:
	print "Content-Type: text/xml"
	if (not (form.has_key("action") and form["action"]=="view")):
		print "Content-Disposition: attachment; filename=vegastrike.config"
	print ""
	
	print s

if __name__=='__main__':
	form = db.urlDecode(os.environ.get('QUERY_STRING',''))
	if form.has_key("mod"):
		mod = form["mod"]
	else:
		mod = ''
	
	filedb = db.DBBase(mod)
	
	execute(filedb, form, '')

