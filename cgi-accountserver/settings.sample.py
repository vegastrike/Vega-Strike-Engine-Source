
# Full path to the directing holding data.
data_path='/path/to/cgi-bin'

mods = {
	# Default
	''       : {
		'path': '', #root directory
		'name': 'Vega Strike main',
		'servers':{
			'Crucible/Cephid_17': '171.67.76.67:6777',
			}
		},
	# Test server
	'test'   : {
		'path': 'test',
		'name': 'Vega Strike Test Servers',
		'servers':{
			'Crucible/Cephid_17': '127.0.0.1:6778',
			'testsystems/absolutelynothing': '127.0.0.1:6777',
			'Sol': '127.0.0.1:6779',
			'': '127.0.0.1:6780',
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


