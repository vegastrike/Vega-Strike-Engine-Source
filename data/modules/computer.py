import custom
import dialog_box
import VS

def runBountyMenu(cp=-1):
	if cp==-1:
		cp=VS.getCurrentPlayer()
	un = VS.getPlayerX(cp)
	id=None
	
	def callback(dialog,result):
		print 'id is: ',id
		action,inputs = dialog_box.fromValues(result)
		if action=='Cancel':
			dialog.undraw()
			custom.respond(['close'],None,id)
			return False
		if action=='OK':
			dialog.undraw()
			custom.respond(['close'],None,id)

			callsign = inputs['logged_in_users']
			if callsign=='' or callsign=='OR, Type in a user:':
				callsign = inputs['callsign']

			print 'I would now place a bounty on '+str(callsign)+' for '+str(float(inputs['credits']))+' credits.'
			print 'User value IS: '+str(inputs['logged_in_users'])
			server_lib.placeBounty(callsign,float(inputs['credits']))
			return False
		return True
	
	items=['width', 1.5,
		'text', "Place a bounty on someone's head",
]
	items+=['height',0.05,
		'row',
		'textwidth','Credits:',0.3,
		'textinput','credits',('%g'%max(1000.,un.getCredits()/100.)),
		'endrow',
		'height',0.05]
	if VS.isserver():
		import server
		logged_in_users=server.getDirector().getCallsignList()
		logged_in_users.append('OR, Type in a user:')
		items+=['text','Select a player in this system:',
			'list','logged_in_users',len(logged_in_users)]+logged_in_users
		items+=['row',
			'textwidth','Other Player:',0.3,
			'textinput','callsign','',
			'endrow']
	else:
		items+=['list','logged_in_users',15,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o']
	items += dialog_box.button_row(1.5,'OK','Cancel')
	id = dialog_box.dialog(items,callback)
	

def runComputer(local,cmd,args,id,cp=-1):
	#def a():
	#	print "A() called"
	#	def b(result):
	#		print result
	#		if not result:
	#			result='NOT'
	#		def c(result):
	#			dialog_box.alert(result)
	#		dialog_box.confirm("Test message 2 "+str(result),c)
	#	dialog_box.confirm("Test message",b,buttons=('A','B','C','D','E',"F",'G','H','I','J'))
	#dialog_box.alert("TEST\nALERT\nBox",callback=a)
	
	def callback(dialog,result):
		print 'id is: ',id
		action,inputs = dialog_box.fromValues(result)
		if action=='Test':
			print 'test clicked!!!!'
		if action=='Bounty Hunt':
			print 'hunting bounty'
			runBountyMenu(cp)
		if action=='OK' or action=="Exit Menu" or action=="Cancel":
			dialog.undraw()
			custom.respond(['close'],None,id)
			return False
		return True
	
	items=['width', 1.0,
		'text', 'Public Computer Menu',
		'space', 0., .1,
		'button', 'Test',
		'button', 'View Score',
		'button', 'Bounty Hunt',
		'button', 'Exit Menu']
	id = dialog_box.dialog(items,callback)
	print 'id is set to: ',id

custom.add("computer",runComputer)

