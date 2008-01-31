
# Full path to the directing holding data.
data_path='/path/to/vegastrike/data4.x'

mods = {
	# Default
	''       : {
		'path': '', #root directory
		'name': 'Vega Strike main',
		'servers':{
			'': '1.2.3.4:6777',
			}
		},
	# Test server -- Mod example
	'test'   : {
		'path': 'test',
		'name': 'Vega Strike Test Server',
		'servers':{
			'Sol': '127.0.0.1:6779',
			'': '127.0.0.1:6778',
			}
		},
	}

file_dbconfig = {
	'type':    'file',
	'storage': '/path/to/writable/dir',
}

mysql_dbconfig = {
	'type':       'mysql',
	'host':       '127.0.0.1', #'mysql4-v.sourceforge.net'
	'port':       '3306',
	'passwd':     '',
	'user':       'user_admin',
	'db':         'vegastrike',
	'user_table': 'phpbb_users',
	'account_table': 'accounts',
	'create_user':False,
}

dbconfig = file_dbconfig

http_port = 8080 # Port number for httpserver.py
