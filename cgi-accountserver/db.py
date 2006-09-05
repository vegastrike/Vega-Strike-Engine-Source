import random
import settings
import os

class DBError(RuntimeError):
	def __init__(self, args):
		RuntimeError.__init__(self, args)


class DBBase:
	def check_password(self, username, password):
		return False
	def modify_account(self, username, type="llama.begin", faction="confed"):
		self.save_account(username, self.get_default_save(type, faction),
			self.get_default_csv(type))
	
	def save_account(self, username, save, csv):
		pass
	def get_login_info(self, user, password):
		pass
	def hash_password(self, password):
		import md5
		return md5.md5(password).hexdigest()
	
	def compare_password(self, hash, password):
		phash = self.hash_password(password)
		if len(hash)<len(phash):
			return password == hash
		else:
			return phash.lower() == hash.lower()
	
	def check_string(self, s):
		if not s:
			raise DBError, "All fields must be non-empty."
		for c in s:
			oc = ord(c)
			if oc < 32 or oc >= 127:
				raise DBError, "Invalid character "+str(oc)+" in field."
		return s
	def open_default_file(self, ext):
		f=open(settings.data_path+"/units/default."+ext,"rb")
		ret = f.read()
		f.close()
		return ret#demands that these files exist
	def get_default_save(self, shiptype, faction):
		try:
			s=self.open_default_file('save')
		except IOError:
			raise DBError, "Not able to open the default saved game."
		s=s.replace("^hyena 120000000000 40000000 -110000000000 pirates","^"+shiptype+" "+str(120000000000+random.uniform(-10000,10000))+" "+str(40000000+random.uniform(-10000,10000))+" "+str(-110000000000+random.uniform(-10000,10000))+" "+faction)
		return s
	def get_default_csv(self, shiptype):
		try:
			unfp=open(settings.data_path+'/units/units.csv',"rb")
		except IOError:
			raise DBError, "Not able to open units.csv"
		type_dat = unfp.readlines()
		unfp.close()
		s = ''
		if len(type_dat)>3:
			s += (type_dat[0])
			s += (type_dat[1])
		for line in type_dat[2:]:
			# Turrets do not work server-side, so avoid any ships with turrets for now.
			if (not len(line) or line.find("turret")!=-1):
				continue
			name=""
			if line.find("./weapons")!=-1:
				continue
			if line[0]=='"':
				endl=line[1:].find('"')
				if endl!=-1:
					name=line[1:1+endl] 
			else:
				endl=line.find(",")
				if endl!=-1:
					name=line[:endl]
			if (len(name) and name.find("__")==-1 and name.find(".blank")==-1 and name!="beholder"):
				if name==shiptype:
					s += line
					return s;
		raise DBError, "Can not find information for unit '"+shiptype+"'"

class FileDB(DBBase):
	def __init__(self, config):
		self.storage_path = config['storage']
		if self.storage_path[-1]=='/':
			self.storage_path = self.storage_path[:-1]
		try:
			os.mkdir(self.storage_path)
		except:
			pass
		self.storage_path += '/'
		if config.get('create_user',True):
			self.create_user=True
		else:
			self.create_user=False
	def check_password(self, username, password, can_create = False):
		success=False
		try:
			f=open(self.storage_path+username+".password","rb")
			s=f.read()
			f.close()
			if self.compare_password(s, password):
				success=True
		except IOError:
			if self.create_user and can_create:
				f=open(self.storage_path+username+".password","wb")
				f.write(self.hash_password(password))
				f.close()
				success=True
			else:
				success=False
		return success
	# Checks a string for valid username characters.
	def check_string(self, s):
		if not s:
			raise db.DBError, "You must fill out the field"
		
		for c in s:
			if not (c.isalnum() or c=='_' or c=='-' or c=='.' or c=='$'):
				raise DBError, "Invalid character "+c+" in input "+s
		
		if s.find("..")!=-1 or s.find(".xml")!= -1 or s.find(".save")!=-1 or s.find("accounts")!=-1 or s.find("default")!=-1:
			raise db.DBError, "Invalid character . in input "+s
		
		return s
	
	def save_account(self, username, save, csv):
		o=open(self.storage_path+username+".save","wb")
		o.write(save)
		o.close()
		o=open(self.storage_path+username+".xml","wb")
		o.write(csv)
		o.close()
	
	def get_login_info(self, user, password):
		result={}
		f=None
		if not self.check_string(user):
			return None
		try:
			f=open(self.storage_path+user+".password","rb")
			tpass=f.read()
			f.close()
			if self.compare_password(tpass, password):
				try:
					f=open(self.storage_path+user+".save","rb")
					result['savegame']=f.read()
					f.close()
				except IOError:
					result['savegame']=None
				try:
					f=open(self.storage_path+user+".xml","rb")
					result['csv']=f.read()
					f.close()
				except IOError:
					result['csv']=None
				try:
					f=open(self.storage_path+user+'.logged',"rb")
					result['logged_in_server']=f.read()
					f.close()
				except IOError:
					result['logged_in_server']=None
				return result
		except IOError:
			pass
		return None

	def set_connected(self, user, isconnected):
		if not self.check_string(user):
			return
		f=open(self.storage_path+user+'.logged',"wb")
		if isconnected:
			f.write("1")
		f.close()#empty file

class MysqlDB(DBBase):
	def __init__(self, config):
		try:
			import MySQLdb
			self.conn = MySQLdb.connect(
				host   = config['host'],
				port   = int(config['port']),
				passwd = config['passwd'],
				user   = config['user'],
				db     = config['db'])
			self.dict_cursor = MySQLdb.cursors.DictCursor
		except:
			self.conn = None
			self.dict_cursor = None
		self.user_table = config['user_table']
		if config.get('create_user',True):
			self.create_user = True
		else:
			self.create_user = False
	
	def check_password(self, username, password, can_create=False):
		c = self.conn.cursor() #self.dict_cursor
		c.execute('SELECT user_id, user_password FROM ' + 
			self.user_table + ' WHERE username=%s',
			(username,) )
		result = c.fetchone()
		if result and result[0]:
			return self.compare_password(result[1], password)
		else:
			if self.create_user and can_create:
				maxquery = self.conn.cursor()
				maxquery.execute('SELECT MAX(user_id) FROM ' + self.user_table)
				answer_row = maxquery.fetchone()
				user_id = answer_row[0]+1
				c = self.conn.cursor()
				c.execute('INSERT INTO '+self.user_table +
					' (username, user_id, user_password) VALUES ' +
					' (%s, '+str(int(user_id))+', %s)',
					(username, self.hash_password(password)) )
				return True
			else:
				return False
	
	def save_account(self, username, save, csv):
		c = self.conn.cursor()
		c.execute('UPDATE ' + self.user_table +
			' SET savegame=%s, csv=%s WHERE username=%s',
			(save, csv, username) )
	
	def get_login_info(self, username, password):
		c = self.conn.cursor(self.dict_cursor)
		c.execute('SELECT logged_in_server, user_password, savegame, csv FROM ' +
			self.user_table + ' WHERE username=%s',
			(username,))
		result = c.fetchone()
		if (result):
			if self.compare_password(result['user_password'], password):
				return result
		return None
	
	def set_connected(self, user, isconnected):
		c = self.conn.cursor()
		logged_in_str = '0'
		if isconnected:
			logged_in_str = '1'
		c.execute('UPDATE '+self.user_table+' SET logged_in_server='+
			logged_in_str+' WHERE username=%s', (user,) )

def connect(config):
	if config['type'] == 'file':
		return FileDB(config)
	elif config['type'] == 'mysql':
		return MysqlDB(config)
	else:
		raise DBError('invalid database type: '+str(dbconfig.type))

