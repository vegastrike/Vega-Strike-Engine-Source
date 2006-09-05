
# Full path to the directing holding data.
data_path='/path/to/cgi-bin'

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
	'create_user':False,
}

dbconfig = file_dbconfig


