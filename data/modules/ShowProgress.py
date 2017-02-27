import VS
import vsrandom

__active = set()
__showing = None
__delay = 0

def activateProgressScreen(screen_id,delay=0,force=False):
    # Sometimes we have to force it, because VS engine forceably unloads some splash screens at the very start.
	global __active, __showing, __delay
	if screen_id not in __active or force:
		__active.add(screen_id)
	if True:
		screen  = VS.vsConfig("splash","%s_sprite" % screen_id,"")
		message = VS.vsConfig("splash","%s_message" % screen_id,"")
		screensplit  = screen.split(" ")
		screen  = screensplit[vsrandom.randrange(0,len(screensplit))]
		if screen != "" or message != "":
			__showing = screen_id
			__delay = delay
			VS.showSplashScreen(screen)
			VS.showSplashMessage(message)

def activeProgressScreen(screen_id):
	global __active
	return screen_id in __active

def showingProgressScreen(screen_id=None):
	global __showing
	if screen_id is None:
		return __showing
	else:
		return __showing == screen_id

def deactivateProgressScreen(screen_id):
	global __active, __showing, __delay
	if screen_id in __active:
		if screen_id != __showing or __delay == 0:
			__active.remove(screen_id)
		else:
			__delay -= 1
			return
	for screen_id in __active:
		activateProgressScreen(screen_id)
		return
	VS.hideSplashScreen()
	__showing = None

def setProgressMessage(screen_id,message):
	global __showing
	if screen_id == __showing:
		prefix = VS.vsConfig("splash","%s_message" % screen_id,"")
		if prefix != "":
			prefix += ": "
		VS.showSplashMessage(prefix+message)

def setProgressBar(screen_id,progress):
	global __showing
	if screen_id == __showing:
		VS.showSplashProgress(progress)

