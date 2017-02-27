import Base
import GUI
from XGUIDebug import *
import Director
import universe
import VS
import ShowProgress
import methodtype
import mission_lib

pirate_bases = {
}

quadrants = {
}

savefilters = set(["test2_main_menu","New_Game"])

class NewSaveGame: pass

def time_sorted_listdir(dir):
	import os
	def time_key(filename):
		return (-os.stat(dir+'/'+filename).st_mtime, filename)
	return sorted(os.listdir(dir), key=time_key)

def savelist():
	global savefilters
	return [ GUI.GUISimpleListPicker.listitem(path,path) 
		for path in time_sorted_listdir(VS.getSaveDir())
		if path[:1] != '.' and path not in savefilters ]

def makeNewSaveName():
	saves = [ i.data for i in savelist() ]
	prefix = "%s_%s_%s_" % (get_system_text()[2] , get_base_text()[1][0] , get_ship_text())
	i = 1
	while ("%s_%02d" % (prefix,i)) in saves:
		i += 1
	return "%s_%02d" % (prefix,i)

def MakePersonalComputer(room_landing_pad, room_concourse, make_links=1, enable_missions=1, enable_finances=1, enable_manifest=1, enable_load=1, enable_save=1, return_room_map=0):
	 
	# create the screen
	room_id = Base.Room ('XXXQuine_4025')
	
	# create an object to keep the state
	comp = QuineComputer(room_id, room_concourse, enable_missions, enable_finances, enable_manifest, enable_load, enable_save)

	# link this screen with the landing pad and the concourse
	# this will be replaced with a keybinding, eventually
	if make_links:
		Base.Link (room_concourse,   'quine_pc', -1, 0.75, 0.25, 0.25, 'Quine_4025', room_id)
		Base.Link (room_landing_pad, 'quine_pc', -1, 0.75, 0.25, 0.25, 'Quine_4025', room_id)

	if return_room_map:	
		# They want a room map, pointing to each section separately (right now... only root,
		# but eventually, something else...) and the computer object, so you can do stuff
		room = GUI.GUIRootSingleton.getRoomById(room_id)
		return { 'root':room, 'load':room, 'save':room, 'computer':comp }
	else:
		return room_id

def change_text_click(self,params):
	GUI.GUIButton.onClick(self,params)
	self.room.owner.change_text(self.index)

def scroll_click(self,params):
	GUI.GUIButton.onClick(self,params)
	self.room.owner.scroll(self.index[4:])

class QuineComputer:
	def __init__(self, room_start, room_exit_to, enable_missions, enable_finances, enable_manifest, enable_load, enable_save):
		# Quine computer is made up of several "rooms" aka screens
		# the room_start show the user's location
		# this is made a lot more complex because of the need to update the cargo manifest
		guiroom = GUI.GUIRoom(room_start)
		self.guiroom = guiroom

		# when a button is clicked, this will allow us to get the QuineComputer instance from the x_click functions
		guiroom.owner = self

		# add background sprite; no need to keep a variable around for this, as it doesn't change
		GUI.GUIStaticImage(guiroom, 'background', ( 'interfaces/quine/main.spr' , GUI.GUIRect(0, 0, 1, 1, "normalized") )).draw()
	
		# add buttons
		self.buttons = {}
		self.mode = ''

		if enable_finances:
			hot_loc = GUI.GUIRect(545, 287, 105, 60, "pixel", (800,600))
			spr_loc = hot_loc
			spr = ("interfaces/quine/fin_pressed.spr",spr_loc)
			sprites = { 'enabled':None, 'disabled':None, 'down':spr }
			self.add_button( GUI.GUIButton(guiroom,'XXXFinances','btn_finances', sprites, hot_loc), change_text_click )
		if enable_manifest:
			hot_loc = GUI.GUIRect(644, 285, 96, 64, "pixel", (800,600))
			spr_loc = hot_loc
			spr = ("interfaces/quine/man_pressed.spr",spr_loc)
			sprites = { 'enabled':None, 'disabled':None, 'down':spr }
			self.add_button( GUI.GUIButton(guiroom,'XXXManifest','btn_manifest', sprites, hot_loc), change_text_click )
		if enable_load:
			hot_loc = GUI.GUIRect(610, 185, 97, 55, "pixel", (800,600))
			spr_loc = hot_loc
			spr = ("interfaces/quine/load_pressed.spr",spr_loc)
			sprites = { 'enabled':None, 'disabled':None, 'down':spr }
			self.add_button( GUI.GUIButton(guiroom,'XXXLoad'    ,'btn_load'    , sprites, hot_loc), change_text_click )
		if enable_save:
			hot_loc = GUI.GUIRect(541, 173, 97, 55, "pixel", (800,600))
			spr_loc = hot_loc
			spr = ("interfaces/quine/save_pressed.spr",spr_loc)
			sprites = { 'enabled':None, 'disabled':None, 'down':spr }
			self.add_button( GUI.GUIButton(guiroom,'XXXSave'    ,'btn_save'    , sprites, hot_loc), change_text_click )
		if enable_missions:
			hot_loc = GUI.GUIRect(540, 227, 200, 60,"pixel", (800,600))
			spr_loc = hot_loc
			spr = ("interfaces/quine/missions_pressed.spr",spr_loc)
			sprites = { 'enabled':None, 'disabled':None, 'down':spr }
			self.add_button( GUI.GUIButton(guiroom,'XXXMissions','btn_missions', sprites, hot_loc), change_text_click )
		
		if enable_load or enable_save:
			hot_loc = [ GUI.GUIRect(624, 346, 55, 69, "pixel", (800,600)),
						GUI.GUIRect(624, 412, 55, 69, "pixel", (800,600)),
						GUI.GUIRect(553, 389, 73, 56, "pixel", (800,600)),
						GUI.GUIRect(675, 389, 73, 56, "pixel", (800,600)) ]
			spr_loc = hot_loc
			spr = [ ("interfaces/quine/up_pressed.spr"   ,spr_loc[0]),
					("interfaces/quine/down_pressed.spr" ,spr_loc[1]),
					("interfaces/quine/left_pressed.spr" ,spr_loc[2]),
					("interfaces/quine/right_pressed.spr",spr_loc[3]) ]
			sprites = [ { 'enabled':None, 'disabled':None, 'down':spr[0] },
						{ 'enabled':None, 'disabled':None, 'down':spr[1] },
						{ 'enabled':None, 'disabled':None, 'down':spr[2] },
						{ 'enabled':None, 'disabled':None, 'down':spr[3] } ]
			self.add_button( GUI.GUIButton(guiroom,'XXXUp'   ,'btn_up'   , sprites[0], hot_loc[0]), scroll_click )
			self.add_button( GUI.GUIButton(guiroom,'XXXDown' ,'btn_down' , sprites[1], hot_loc[1]), scroll_click )
#			self.add_button( GUI.GUIButton(guiroom,'XXXLeft' ,'btn_left' , sprites[2], hot_loc[2]), scroll_click )
#			self.add_button( GUI.GUIButton(guiroom,'XXXRight','btn_right', sprites[3], hot_loc[3]), scroll_click )
	
		current_base = universe.getDockedBase()
		player = VS.getPlayer()
		
		# this doesn't change while docked, so only call it once
		self.str_start = get_location_text(current_base)
		
		screen_loc = GUI.GUIRect(80,90,350,380,"pixel",(800,600))
		screen_color = GUI.GUIColor(20/255.0, 22/255.0 ,10/255.0)		# first I tried rgb(56 60 24) and rgb(40 44 20); both were too light
		screen_bgcolor = GUI.GUIColor.clear()
		screen_bgcolor_nc = GUI.GUIColor(0.44,0.47,0.17)
	
		# text screen
		self.txt_screen = GUI.GUIStaticText(guiroom, 'txt_screen', self.str_start, screen_loc, 
			color=screen_color,
			bgcolor=screen_bgcolor)
		self.txt_screen.hide()

		# picker screen
		self.picker_screen = GUI.GUISimpleListPicker(guiroom,'XXXSelect item','picker_screen', screen_loc,
			textcolor    =screen_color     , textbgcolor    =screen_bgcolor,
			selectedcolor=screen_bgcolor_nc, selectedbgcolor=screen_color   )
		self.picker_screen.hide()
			
		# 
		# much of this is temporary, until something better can be worked out:
		# 
	
		# Save/Load screen
		#if enable_save:
		#	x, y, w, h = GUI.GUIRect(217, 56, 40, 18).getHotRect()
		#	Base.Comp (room_start, 'save_comp', x, y, w, h, 'XXXSave/Load/Quit', 'LoadSave')
		#if enable_load:
		#	x, y, w, h = GUI.GUIRect(257, 56, 39, 18).getHotRect()
		#	Base.Comp (room_start, 'load_comp', x, y, w, h, 'XXXSave/Load/Quit', 'LoadSave')
	
		# Missions
		#if enable_missions: 
		#	x, y, w, h = GUI.GUIRect(217, 75, 79, 18).getHotRect()
		#	Base.Comp (room_start, 'missions', x, y, w, h, 'XXXMissions', 'Missions Info Cargo ')
	
		# Finances
		#if enable_finances:
		#	x, y, w, h = GUI.GUIRect(219, 94, 40, 18).getHotRect()
		#	Base.Comp (room_start, 'missions', x, y, w, h, 'Finances', 'Info ')
	
		# Manifest
		#if enable_manifest:
		#	x, y, w, h = GUI.GUIRect(260, 94, 40, 18).getHotRect()
		#	Base.Comp (room_start, 'missions', x, y, w, h, 'Manifest', 'Cargo ')
	
	
		# Exit button, returns us to concourse
		rect = GUI.GUIRect(227, 165, 35, 14, "pixel", (320,200))
		x, y, w, h = rect.getHotRect()
#		Base.Link (room_start, 'exit', x, y, w, h, 'Exit', room_exit_to)
		Base.LinkPython (room_start, 'exit', "#\nimport GUI\nGUI.GUIRootSingleton.getRoomById(%r).owner.reset()\n" %(guiroom.getIndex()), x, y, w, h, 'XXXExit', room_exit_to)

	def reset(self):
		trace(TRACE_DEBUG,"::: QuineComputer.reset()")
		self.txt_screen.setText( self.str_start )

	def change_text(self, button_index):
		text_screens = {
			'btn_finances' : lambda:get_relations_text(VS.getPlayer()),
			'btn_manifest' : lambda:get_manifest_text(VS.getPlayer()),
			'btn_missions' : lambda:get_missions_text()
			}
		if button_index in text_screens:
			self.txt_screen.setText( text_screens[button_index]() )
			self.txt_screen.show()
			self.picker_screen.hide()
		elif button_index == "btn_load":
			if self.mode != button_index:
				self.picker_screen.items = savelist()
				self.picker_screen.show()
				self.txt_screen.hide()
			elif self.picker_screen.selection is not None:
				ShowProgress.activateProgressScreen('loading',3)
				import dj_lib
				dj_lib.enable()
				VS.loadGame(self.picker_screen.items[self.picker_screen.selection].data)
		elif button_index == "btn_save":
			if self.mode != button_index:
				self.picker_screen.items = [GUI.GUISimpleListPicker.listitem("New Game",NewSaveGame)]+savelist()
				self.picker_screen.show()
				self.txt_screen.hide()
			elif self.picker_screen.visible and self.picker_screen.items[self.picker_screen.selection].data is not NewSaveGame:
				self.picker_screen.hide()
				self.txt_screen.setText( 
					"\n"*7+
					"Are you sure you want to overwrite the savegame?\n(%s)"%
						self.picker_screen.items[self.picker_screen.selection]
					+"\n"*3
					+"Press SAVE again to do it." )
				self.txt_screen.show()
			elif self.picker_screen.selection is not None:
				if self.picker_screen.items[self.picker_screen.selection].data is NewSaveGame:
					VS.saveGame(makeNewSaveName())
				else:
					VS.saveGame(self.picker_screen.items[self.picker_screen.selection].data)
				self.picker_screen.items = [GUI.GUISimpleListPicker.listitem("New Game",NewSaveGame)]+savelist()
				self.picker_screen.show()
				self.txt_screen.hide()
		else:
			self.picker_screen.hide()
			self.txt_screen.hide()
		self.mode = button_index
	
	def scroll(self,direction):
		list_screens = set(['btn_load','btn_save'])
		if self.mode in list_screens:
			if direction == 'up':
				self.picker_screen.pageMove(-1)
			elif direction == 'down':
				self.picker_screen.pageMove(1)
			elif direction == 'left':
				self.picker_screen.viewMove(-1)
			elif direction == 'right':
				self.picker_screen.viewMove(1)


	def add_button(self, guibutton, onclick_handler):
		# add the button to the "buttons" dictionary, draw it, and add onclick handler
		self.buttons[guibutton.index] = guibutton
		guibutton.draw()
		guibutton.onClick = methodtype.methodtype(onclick_handler, guibutton, type(guibutton))
	
	def setMode(self,mode):
		aliases = dict(load='btn_load',save='btn_save')
		mode = aliases.get(mode,mode)
		if self.mode != mode:
			self.change_text(mode)
		self.guiroom.redrawIfNeeded()

def get_system_text(str_system_file=None, current_base=None):
	if str_system_file is None:
		if current_base is None:
			current_base = universe.getDockedBase()
		# get sector, quadrant, system, and base name
		str_system_file = current_base.getUnitSystemFile()

	n = str_system_file.find('/')
	if (n >= 0):
		str_sector   = str_system_file[:n]
		str_system   = str_system_file[n+1:]
	else:
		str_sector   = 'Unknown'
		str_system   = 'Unknown'

	try:
		str_quadrant = quadrants[str_system_file]
	except KeyError:
		str_quadrant = 'Unknown'
		
	return (str_quadrant,str_sector,str_system)
	
def get_base_text(current_base = None, str_system_file = None):
	if current_base is None:
		current_base = universe.getDockedBase()

	if not current_base:
		return ((0,"neutral"),("Unknown","Main Menu"))
	if str_system_file is None:
		# get sector, quadrant, system, and base name
		str_system_file = current_base.getUnitSystemFile()
	
	# get faction
	int_faction = current_base.getFactionIndex()
	str_faction = current_base.getFactionName()

	# bases and planets aren't consistent in their usage of Name and Fullname values
	if not current_base:
		import debug
		debug.error('getDockedBase() returns null Unit!')
	if current_base.isPlanet():
		str_base = current_base.getName()
		str_base_type = current_base.getFullname() + " planet"
	else:
		str_base = current_base.getFullname()
		str_base_type = current_base.getName()
		if str_base == '':
			str_base = 'Unknown'
			if str_faction == "pirates":
				try:
					str_base = pirate_bases[str_system_file]
				except KeyError:
					str_base = 'Unknown'

	# adjust the base type for certain planets
	if str_base_type == 'new_constantinople':
		str_base_type = "government base"
	elif str_base_type == 'perry':
		str_base_type = "military base"
	elif str_base_type == 'church_of_man planet':
		str_base_type = "agricultural planet"

	return ((int_faction,str_faction),(str_base,str_base_type))

def get_ship_text(unit = None):
	if unit is None:
		player = VS.getPlayer()
	
	name = player.getName()
	if name.index('.') is None:
		return name.capitalize()
	else:
		return name[:name.index('.')].capitalize()

def get_missions_text():
	missionlist = mission_lib.GetMissionList()
	
	parth = lambda s:(s and "("+s+")") or s
	
	full_layout = "\n\nMissions:\n\n%(ENTRIES)s\n\nTotal active missions: %(NUM_MISSIONS)s\n";
	entry_process = lambda e: { 
		'MISSION_TYPE'		:e.get('MISSION_TYPE','MISSION').replace('_',' ').capitalize(), 
		'SHORT_DESCRIPTION'	:e.get('MISSION_SHORTDESC','').split('/',1)[-1],
		'GUILD_NAME' 		:parth(e.get('GUILD_NAME',e.get('MISSION_NAME','').split('/',1)[0]).replace('_',' ').title()) }
	entry_layout = "%(MISSION_TYPE)s %(GUILD_NAME)s:\n%(SHORT_DESCRIPTION)s\n\n"
	
	return full_layout % {
		'NUM_MISSIONS':len(missionlist),
		'ENTRIES':''.join( entry_layout % entry_process(entry) for entry in missionlist ) }

#
#   helper functions
#
def get_location_text(current_base):

	str_quadrant, str_sector, str_system = get_system_text(current_base=current_base)
	(int_faction,str_faction),(str_base,str_base_type) = get_base_text(current_base)

	
	str_location = """
Location:
   %s
   %s
   %s

System:
   %s

Quadrant:
   %s
   %s Sector


Ready!
""" %(str_base, str_base_type.capitalize(), str_faction, str_system, str_quadrant, str_sector)

	return str_location

def get_manifest_text(player):
	cargo_dict = {}

	# get the hold volume
	int_hold_volume = int( VS.LookupUnitStat( player.getName(), player.getFactionName(), "Hold_Volume" ) )
	if (player.hasCargo("add_cargo_volume")):
		# capacity increases by 50% if they have the cargo expansion
		int_hold_volume = int( int_hold_volume * 1.5 )

	int_total_quantity = 0
	for i in range(player.numCargo()):
		cargo = player.GetCargoIndex(i)

		name     = cargo.GetContent()
		category = cargo.GetCategory()
		quantity = cargo.GetQuantity()

		if name == '': continue
		if category[:8] == 'upgrades': continue
		if category[:9] == 'starships': continue

		if (quantity > 0):
			cargo_dict[name] = quantity
			int_total_quantity += quantity

	int_space_left = int_hold_volume - int_total_quantity
	keys = cargo_dict.keys()
	if len(keys) > 0:
		str_manifest = "Space left: %s\n\n" %(int_space_left)
		keys.sort()
		for i in keys:
			count = cargo_dict[i]
			# try to pad the columns so they line up
			str_pad = "   "
			int_pad_len = len(str_pad) - len(str(count))
			if int_pad_len < 1:
				str_pad = ""
			else:
				str_pad = str_pad[:int_pad_len]
			str_manifest += "%s%s  %s\n" %(str_pad, count, i)
	else:
		str_manifest = "Space left: %s\nNo cargo loaded.\n" %(int_space_left)

	return str_manifest
		

def get_relations_text(player):
	str_relations = "Cash:  %s\n\nKill breakdown:\n" %( int(player.getCredits()) )

	# length of faction_kills = VS.GetNumFactions() + 1
	# could the last entry be the total?  or maybe the number of times the user has died?
	faction_kills = [];
	for i in range(Director.getSaveDataLength( VS.getCurrentPlayer(), 'kills' )):
		faction_kills.append( Director.getSaveData( VS.getCurrentPlayer(), 'kills', i ) )
	
	displayed_factions = ['confed', 'aera', 'rlaan']

	for i in range(VS.GetNumFactions()):
		# VS.GetFactionIndex(s) expects a string
		# VS.GetFactionIndex(s) expects a name
		faction = VS.GetFactionName(i)
		if faction in displayed_factions:
			# note: the following calls do not always return equal values:
			#   VS.GetRelation(a, b)
			#   VS.GetRelation(b, a)
			relation = int( VS.GetRelation(faction, player.getFactionName()) * 100 )
			if relation > 100:
				relation = 100
			elif relation < -100:
				relation = -100

			# not sure if the AI or Friend/Foe radar agree with these figures, but this ought to work, mostly
			if relation > 20:
				str_relation = "Friendly"
			elif relation > 0:
				str_relation = "Neutral"
			elif relation > -20:
				str_relation = "Hostile"
			else:
				str_relation = "Kill On Sight"

			try:
				kills = int( faction_kills[i] )
			except:
				kills = 0
			
			# try to pad the columns 
			# (this doesn't work perfectly, since we're using a variable-width font)
			str_pad = "    "
			int_pad_len = len(str_pad) - len(str(kills))
			if int_pad_len < 1:
				str_pad = ""
			else:
				str_pad = str_pad[:int_pad_len]

			# add current faction to the output string
			str_relations = str_relations + "%s%s  %s\t(%s: %s)\n" %(str_pad, kills, faction.capitalize(), str_relation, relation)

	return str_relations
