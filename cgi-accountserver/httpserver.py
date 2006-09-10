#!/usr/bin/python

import os
import sys
if __name__=='__main__' and 'REQUEST_METHOD' in os.environ:
	print "HTTP/1.1 500 Internal Server Error"
	print "Content-Type: text/html\n\nThis is not a CGI script."
	sys.exit(1)

from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler

import traceback
import db
import settings

import accountserver
import register
import register_submit
import vsconfig

pages={
	'accountserver': accountserver,
	'register': register,
	'register_submit': register_submit,
	'vsconfig': vsconfig,
	}

connections = {}

def connect_db():
	for mod in settings.mods:
		connections[mod] = db.connect(settings.dbconfig, mod)

class VSHTTPHandler(BaseHTTPRequestHandler):
	def handle_error(self,type):
		sys.stdout = self.oldstdout
		err = traceback.format_exception(sys.exc_type, sys.exc_value, sys.exc_traceback)
		textformat = ''.join(err)
		htmlformat = textformat.replace('\n','<br>\n').replace(' ','&nbsp;')
		print textformat
		self.wfile.write('\n<h1>Python error occurred in '+type+' request.</h1>'+htmlformat)
	
	def do_request(self, postargs=''):
		getargs=''
		command=''
		querystart = self.path.find('?')
		path = self.path
		if (querystart!=-1):
			getargs = self.path[querystart+1:]
			path = self.path[:querystart]
		print path
		pathlist = path.split('/')
		if len(pathlist)>2 and pathlist[0]=='' and pathlist[1]=='cgi-bin':
			command = pathlist[2]
			extension = command.find('.')
			if extension!=-1:
				command=command[:extension]
		elif len(pathlist)==1 or len(pathlist)==2 and pathlist[1]=='':
			command='index'
		elif len(pathlist)==2:
			indexname = pathlist[1]
			extension = indexname.find('.')
			print pathlist
			if extension!=-1:
				indexname=indexname[:extension]
			if indexname=='index':
				command='index'
		if not command in pages:
			if command=='index':
				self.send_response(302, "Found")
				host = self.headers.get('Host', '')
				if host:
					host = 'http://'+host
				self.send_header('Location', host+'/cgi-bin/register.py')
				self.end_headers()
			else:
				self.send_error(404, "The requested file /cgi-bin/"+command+" was not found.")
			return
		module = pages[command]
		mod=''
		if module.post_form:
			postargs = db.urlDecode(postargs)
			mod = postargs.get('mod','')
		if module.get_form:
			getargs = db.urlDecode(getargs)
			if not mod:
				mod = getargs.get('mod','')
		if not mod and not module.get_form:
			mod = getargs
		if not mod in connections:
			self.send_error(402, "Payment Required")
			return
		self.oldstdout = sys.stdout
		sys.stdout = self.wfile
		self.send_response(200, "Script output follows")
		module.execute(connections[mod], getargs, postargs)
		sys.stdout = self.oldstdout
		
	def do_GET(self):
		self.oldstdout=None
		try:
			self.do_request()
		except:
			self.handle_error("GET")
	def do_POST(self):
		self.oldstdout=None
		try:
			len = int(self.headers.get('Content-Length', '0'))
			contents = self.rfile.read(len)
			self.do_request(contents)
		except:
			self.handle_error("POST")
		
		
	

def httpServer(port=8080, host=''):
	print "Starting HTTP server on port %d..." % (port,)
	server_address = (host, port)
	httpd = HTTPServer(server_address, VSHTTPHandler)
	httpd.serve_forever()

if __name__=='__main__':
	connect_db()
	httpServer()

