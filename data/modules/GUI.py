# -*- coding: utf-8 -*-
""" """ # dummy docstring - do not remove

"""

VegaStrik GUI abstraction layer

This framework provides a set of classes that allow easy construction
of complex user interfaces based on the rather primitive base API
of the engine.

The framework operates with a GUIRoot singleton, responsible of all
general bookkeeping an a portal for accessing most operations, a
set of GUIRoom objects contained within this singleton (only one
base can be active at any moment in the engine, so this GUIRoot
singleton needs not handle multiple concurrent bases), and several
GUIElement objects within each room.

Hot spots, sprite locations and screen coordinates in general are
specified in a rather complex way through GUIRect. The complexity
is there for several reasons.

 * Vega Strike's base API has several coordinate systems for various
   applications, where hot spots use one convention, sprites another,
   etc... GUIRect provides a unified coordinate system which is far
   easier to work with.
 * Several uses of this framework involve careful placement of elements
   overlaid on top of fixed-resolution bitmaps. When this is the case,
   normalized coordinates tend to be a poor choice, being a lot easier
   to work instead with integral pixel coordinates. GUIRect provides
   an interface to specify coordinates in such a way, providing a
   reference resolution (the underlying bitmap's resolution), and thus
   resulting in pixel-perfect placement.
 * Bases have configurable margins in VS, but some applications of the
   base interface (ie: as a cutscene player) are better off disregarding
   those margins. GUIRect thus provides all the math necessary to leave
   those margins (by default), or to disregard them (only explicitly)
   as easily as possible.
 * Sometimes you'll want non-power-of-two bitmaps, but you won't want
   to store them with npot dimensions because of compatibility reasons.
   At those times, there's a handy specialization of GUIRect, 
   GUINPOTRect, which does the required math to properly render those
   images by just specifying their actual dimensions. GUIRect is thus
   a convenient interface for several alternatve (perhaps user-defined)
   coordinate systems.

See the module for details on each class, following is an overview of
the framework's usage:

1. Initialization
=================

The first step to take is the initialization of the framwork and the
GUIRoot singleton.

	import GUI
	GUI.GUIInit()

You can call it just like that (which will take screen dimensions,
margins and aspect ratio from the engine), or you can specify
those explicitly (see GUIInit's signature and inline documentation)

2. Creating rooms
=================

The second first thing to do, is create rooms. The first room you
create will be the initial room, so be attentive and be sure to
create them in the right order.

GUIRoom **does not** create the underlying room. Since creation order
matters that much, room creation is the one thing that is usually
done by hand, using the raw base API. But GUIRoom does *administer* it.

	# Create rooms (intro, menu)
	# Their order matter, here "preintro" is the initial room
	room_preintro = Base.Room ('XXXPreIntro')
	room_intro = Base.Room ('XXXIntro')
	room_menu = Base.Room ('XXXMain_Menu')

	# Create GUIRoom wrappers
	# Their order doesn't matter
	guiroom_menu = GUI.GUIRoom(room_menu)
	guiroom_intro = GUI.GUIRoom(room_intro)
	guiroom_preintro = GUI.GUIRoom(room_preintro)

Pay attention to the room's title, the "XXX" prefix makes the title
invisible, while omitting the prefix will render the room's title 
on the status bar (usually the bottom edge). 

Sometimes you want this, sometimes you don't.

2.1. Special rooms
==================

In the example above, we have to rooms, intro and preintro.

Preintro would be a short movie or animation clip to show when the
game launches (e.g: "Vega Strike" logo). Intro would be the story's
intro cutscene to display when starting a new campaign.

It is clear that rendering a complex cutscene with such basic elements
at our disposal (as given by the Base API) would be... cumbersome. So
instead of doing a CGI clip, we go for a theora movie.

How would we play it? 

We could add a movie stream element in guiroom_preintro, but there's
already a specialized room type that handles such a common task (and
lets the user skip the movie, an hides the mouse, all the detail
usually forgotten at first glance).

So scratch the above GUI.GUIRoom initialization for intro/preintro, 
and better use the followin:

	# Set up preintro room
	class PreIntroRoom(GUI.GUIMovieRoom):
	    def onSkip(self, button, params):
	        GUI.GUIMovieRoom.onSkip(self, button, params)
	        Base.SetDJEnabled(1)

	preintroroom = PreIntroRoom(room_preintro, 
	    ( 'preintro.ogv',
	      GUI.GUIRect(0, 0, 1, 1, "normalized")), 
	    guiroom)
	preintroroom.setAspectRatio(16.0/9.0)
	Base.SetDJEnabled(0)

	# Set up intro room
	class IntroRoom(PreIntroRoom):
	    def onSkip(self, button, params):
	        PreIntroRoom.onSkip(self, button, params)
	        DoStartNewGame(self, params)

	introroom = IntroRoom(room_intro, 
	    ( 'intro.ogv',
	      GUI.GUIRect(0, 0, 1, 1, "normalized")), 
	    guiroom_menu)
	introroom.setAspectRatio(16.0/9.0)
	Base.SetDJEnabled(0)

Here we have a lot of new stuff.

First, we don't use the plain GUIMovieRoom, we create a derived
class to add behavior to the onSkip event. That's called "decorating"
the class. In the case of the preintro, we have to enable the
music DJ after the movie clip has finished, so we can hear music
in the main menu. In the case of the intro room, after the intro
is done playing we must... er... actually start the campaign.

Then we create such a room. The first argumen tells it which
room number to bind with (room_preintro). The second one is
a "sprite definition" (a tuple) containing the movie's path
and the GUIRect where we want the movie - we give it fullscreen
with (0,0) for top-left an (1,1) for width-height (in normalized
screen coodinates). The third and last argument is the next room.
Since movie clips are usually transitions, GUIMovieRoom accepts
a "next room" and it will automatically switch to that room
when the movie's done playing.

So the initial room will be the preintro room, the preintro will
start playing immediately, and when done "onSkip" will be called,
which will call the base implementation (that switches to the next
room) and then re-enable the music DJ which we disabled with SetDJEnabled(0).

Lets leave the intro room as an excercise to the reader ;-)

But notice the aspect ratio stuff. Movies have an aspect ratio,
and GUIMovieRoom knows about that, and will adjust the location we've
given it so that aspect ratio is preserved. Ain't it cool?

3. Adding elements
==================

We have a menu, so we need stuff on it. We have tons of prebuilt
elements, or widgets, at our disposal.

All elements are constructed by giving their constructors both
a room and an element id. The element will automatically register
with the given room and from then on you can find the element by
its id if you need to.

Most elements take more parameters, but those two are universal.

3.1. Adding pictures
====================

We want pretty menues. We want a background. Which is to say,
a static image on the screen. Straigforward task:

	# Create background
	GUI.GUIStaticImage(guiroom, 'background', 
		( 'interfaces/main_menu/menu.spr', 
		  GUI.GUIRect(0, 0, 1024, 768, "pixel", (1024,768)) ))

GUIStaticImage takes, again, a sprite efinition. This time, we use
a "pixel" coordinate system, giving it (0,0) top-left and (1024x768)
dimensions to our sprite. The last (1024,768) tuple is the "reference"
dimensions, which is the "virtual screen resolution".

So (0,0,1024,768,"pixel",(1024,768)) 
is equivalent to (0,0,1,1,"normalized").

We could have used that. But when coordinates have to match a
specific background image to look right, this "pixel" coordinate system
is preferred, since it always aligns right.

3.2. Adding text
================

Now we want some text in the credits room (which we didn't show).

	text_loc = GUI.GUIRect(408,8,300,50,"pixel",(1024,768))
	GUI.GUIStaticText(credits_guiroom, 'mytitle', credits_title, text_loc, GUI.GUIColor.white())

Now you can see the usefulness of GUIRect and its "pixel" mode.
We give the text line a very precise location (so that it matches the room's background).

Text elements can be given a color, we use white, but we could have specified an RGB value or even
an RGBA (with transparency) one. We can even specify a font size (not sure it works right all the time,
but it eventually should be fixed) and a background color (again, potentially with transparency).

Don't think for a moment this maps directly to Base.TextBox, the base API is rather picky (and buggy),
and GUIStaticText handles all the tricks needed to get this flexibility.

3.3. Linking rooms
==================

Ok, we have a main menu, we have an "intro" room that ends up starting a campaign.

Now we need a "new game" button. Right?

	# New game
	sprite_loc = GUI.GUIRect(48,224,128,32,"pixel",(1280,1024))
	sprite = {
		'*':None,
		'down' : ( 'interfaces/main_menu/new_button_pressed.spr', sprite_loc ) }
	btn = GUI.GUIButton(guiroom, 'XXXNew Game','New_Game',sprite,sprite_loc,'enabled',StartNewGame)

Ok, buttons are more complex, aren't they?

Lets start with the sprite. It's gone from a simple tuple, to a dictionary. Why?

Well, buttons change states. They can be neutral, disabled, hot (mouse over), down (pressed).
The given mapping here maps those states to a sprite definition (like we've seen with GUIStaticText),
with the special state "*", which applies to any state not explicitly given. If the sprite definition
assigned to a state is None, then the button is transparent there.

Why would we wan it transparent? Well, in our case, the background picture already has the button
pre-rendered in. No need to render anything on top. When the button is pressed, we want to overlay
a highlighted version of the button, so we specify that sprite for 'down'.

Confusingly enough, GUIButton takes the button's "tooltip" or "title" before its element ID.
Again, the "XXX" prefix makes the tooltip invisible (it's still required though by the engine).
it also takes the initial state ('enabled'), and a callable, an action to be performed when clicked.

	# Base music
	plist_menu=VS.musicAddList('maintitle.m3u')
	
	def DoStartNewGame(self,params):
		ShowProgress.activateProgressScreen('loading',3)
		VS.loadGame(VS.getNewGameSaveName())
		enterMainMenu(self,params)

	def StartNewGame(self,params):
		Base.SetCurRoom(introroom.getIndex())
		Base.SetDJEnabled(0)

	def enterMainMenu(self,params):
		global plist_menu
		VS.musicPlayList(plist_menu)

The callable must take two arguments, "self", which will be the element clicked, and "params", which
will have other info, like which button was used and exact mouse coordinates, modifier keys and whatnot.

See, when the button is pressed, StartNewGame sends the user to the intro and disables the music DJ
(we only want the movie's music playing). Then when the movie is done playing, remember, DoStartNewGame
will do some stuff, among which is loading the new campaign.

There's also a very straightforward way of linking rooms with GUIRoomButton(from,to,title,id,...):

	sprite_loc = GUI.GUIRect(48,510,92,32,"pixel",(1280,1024))
	sprite = {
		'*':None,
		'down' : ( 'interfaces/main_menu/credits_button_pressed.spr', sprite_loc ) }
	GUI.GUIRoomButton(guiroom, credits_guiroom, 'XXXShow Credits','Show_Credits',sprite,sprite_loc,clickHandler=enterCredits)

See how you can even do extra stuff in a "clickHandler"

	plist_credits=VS.musicAddList('maincredits.m3u')
	
	def enterCredits(self,params):
		global plist_credits
		VS.musicPlayList(plist_credits)


4. Advanced widgets
===================

There are even more advanced widgets than images an push buttons.

There's GUICheckButton s, which toggle between "checked" and "unchecked"
states when clicked, and its cousin the GUIRadioButton, which when checked
also uncheck all other radio buttons within their "radio group".

There's a GUISimpleListPicker, used for instance to build the
game loading interface in the main menu, and a lifesaver in many
situations. You manipulate its content by adding/removing
GUISimpleListPicker.listitem instances to its "items" attribute,
as easy as that.

If you want scrolling buttons for the list picker. you must code
them yourself (using GUIButton), but it's certainly not difficult
since you can easily manipulate the list with its 
viewMove and pageMove member functions.

"""

import Base
import VS
from XGUIDebug import *

GUIRootSingleton = None
_doWhiteHack = 0
_GUITraceLevel = TRACE_VERBOSE

"""----------------------------------------------------------------"""
"""                                                                """
""" GUIRoot - root management interface for the GUI framework.     """
"""     Also acts as singleton "glue" since python snippets can't  """
"""     hold any kind of state (that's a To-Do)                    """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIRoot:
	"""global GUI setup"""
	def __init__(self,screenX=None,screenY=None,marginX=None,marginY=None,aspect=None):
		self.deregisterAllObjects()
		self.deregisterAllRooms()
		if not screenX or not screenY:
			# Get screen dimensions from config
			screenX = int(VS.vsConfig("graphics","base_max_width",
				VS.vsConfig("graphics","x_resolution","0")))
			screenY = int(VS.vsConfig("graphics","base_max_height",
				VS.vsConfig("graphics","y_resolution","0")))
			aspect = float(VS.vsConfig("graphics","aspect","0"))
		self.setScreenDimensions(screenX,screenY)
		self.aspect = aspect or (screenX * 1.0 / screenY)
		if (marginX == None):
			marginX = 0.00
		if (marginY == None):
			marginY = 0.00
		self.setScreenMargins(marginX,marginY)
		self.needRedraw = {}
		self.modalElement = None
		self.keyTarget = None
		Base.GlobalKeyPython('#\nfrom GUI import GUIRootSingleton\nGUIRootSingleton.keyEvent()\n')

	def setScreenDimensions(self,screenX,screenY):
		self.screenX=screenX
		self.screenY=screenY
		self.broadcastMessage('changedScreenDimensions', {'screenX':screenX,'screenY':screenY} )

	def getScreenDimensions(self):
		return (self.screenX,self.screenY)

	def setScreenMargins(self,marginX,marginY):
		self.marginX=marginX
		self.marginY=marginY
		self.broadcastMessage('changedScreenMargins', {'marginX':marginX,'marginY':marginY} )

	def getScreenAspectRatio(self):
		return self.aspect

	def setScreenAspectRatio(self, aspect):
		self.aspect = aspect

	def getScreenMargins(self):
		return (self.marginX,self.marginY)

	def dispatchMessage(self,id,message,params):
		trace(_GUITraceLevel, "::: calling GUI.GUIRoot.dispatchMessage(%s, %s, %s) :::" %(id,message,params))
		if id in self.objects:
			self.objects[id][1].onMessage(message,params)
		else:
			trace(_GUITraceLevel + 1, 'WARNING! - gui.py - GUIRoot::dispatchMessage(): Object id "' + str(id) + '" not found\n')

	def broadcastMessage(self,message,params):
		trace(_GUITraceLevel, "::: calling GUI.GUIRoot.broadcastMessage(%s,%s)" %(message,params) )
		for i in self.objects.keys():
			self.objects[i][1].onMessage(message,params)

	def broadcastRoomMessage(self,roomindex,message,params):
		trace(_GUITraceLevel, "::: calling GUI.GUIRoot.broadcastRoomMessage(%s,%s,%s)" %(roomindex,message,params) )
		for i in self.objects.keys():
			if self.objects[i][0]==roomindex:
				self.objects[i][1].onMessage(message,params)

	def registerObject(self,room,object):
		id = self.nextId
		self.objects[id] = (room,object)
		self.nextId = self.nextId + 1
		return id

	def keyEvent(self):
		eventdata = Base.GetEventData();
		if self.keyTarget is not None:
			eventTarget = self.keyTarget
		else:
			eventTarget = self.rooms.get(Base.GetCurRoom())
		if eventTarget is not None:
			if eventdata['type'] == 'keyup' and hasattr(eventTarget,'keyUp'):
				eventTarget.keyUp(eventdata['key'])
			if eventdata['type'] == 'keydown' and hasattr(eventTarget,'keyDown'):
				eventTarget.keyDown(eventdata['key'])

	def registerRoom(self,room):
		self.rooms[room.getIndex()] = room

	def deregisterObject(self,id):
		self.objects.erase(id)

	def deregisterAllObjects(self):
		self.objects = {}
		self.nextId = 0

	def deregisterAllRooms(self):
		self.rooms = {}

	def redrawIfNeeded(self):
		trace(_GUITraceLevel, "::: calling GUI.GUIRoot.redrawIfNeeded()" )
		for i in self.rooms:
			self.rooms[i].redrawIfNeeded()
		
	def getRoomById(self,id):
		return self.rooms.get(id,None)


def GUIInit(screenX=None,screenY=None,marginX=None,marginY=None,**kwargs):
	""" GUIInit sets up the GUIRootSingleton variable, which is used to store the state """
	global GUIRootSingleton
	GUIRootSingleton = GUIRoot(screenX,screenY,marginX,marginY,**kwargs)


"""----------------------------------------------------------------"""
"""                                                                """
""" GUIRect - defines rectangles on the screen.                    """
"""     By popular demand, those can be defined in a number of     """
"""     modes: pixel, normalized (and variantes).                  """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIRect:
	"""A rectangle on the screen                                                   """
	"""ref: may be (room), (screenx,screeny) or (screenx,screeny,marginx,marginy)  """
	"""   (first takes the dimensions from the room, the next from the tuple itself"""
	"""    just the dimensions but margins and stuff from the root, and the last   """
	"""    takes everything from the tuple)                                        """
	def __init__(self,x,y,wid,hei,mode="pixel",ref=None):
		self.x = x
		self.y = y
		self.w = wid
		self.h = hei
		self.ref = ref
		self.mode = mode
		
	def __repr__(self):
		return repr((self.x,self.y,self.w,self.h,self.mode,self.ref))
	def __str__(self):
		return str((self.x,self.y,self.w,self.h,self.mode,self.ref))

	def getNormalXYWH(self):
		""" returns (x,y,w,h) - x/y is top-left - in Base coordinates """

		""" Allow two kinds of screen dimension overrides, for design convenience """
		""" a) (screenx,screeny) - margins set as per root settings """
		""" b) (screenx,screeny,marginx,marginy) - set both dimensions and margin manually """
		if not self.ref or (type(self.ref)!=tuple) or ((len(self.ref)!=1)and(len(self.ref)!=2)and(len(self.ref)!=4)):
			(screenX,screenY) = GUIRootSingleton.getScreenDimensions()
			(marginX,marginY) = GUIRootSingleton.getScreenMargins()
		else:
			if len(self.ref)==4:
				(screenX,screenY,marginX,marginY) = self.ref
			elif len(self.ref)==2:
				(screenX,screenY) = self.ref
				(marginX,marginY) = GUIRootSingleton.getScreenMargins()
			else:
				(screenX,screenY) = self.ref[0].getScreenDimensions()
				(marginX,marginY) = self.ref[0].getScreenMargins()
		if (self.mode=="pixel"):
			""" pixel coordinates relative to current screen settings 
				to translate (0,0) (screenX,screenY) to (-1,1) (1,-1)
				x = 2*xi/screenX - 1
				y = 1 - 2*yi/screenY
			"""
			return ( (2.0 * self.x / screenX - 1.0)*(1.0-marginX) ,  \
                           (-2.0 * self.y / screenY + 1.0)*(1.0-marginY) , \
                           (2.0 * self.w / screenX * (1.0-marginX)) ,      \
                           (2.0 * self.h / screenY * (1.0-marginY))        \
                          )
		elif (self.mode=='normalized_biased_scaled'):
			""" direct coordinates: top-left = (-1,+1), bottom-right = (+1,-1) - margins WILL NOT be applied """
			return (self.x,self.y,self.w,self.h)
		elif (self.mode=='normalized_biased'):
			""" direct coordinates: top-left = (-1,+1), bottom-right = (+1,-1) - margins WILL be applied """
			return (self.x*(1.0-marginX),self.y*(1.0-marginY),self.w*(1.0-marginX),self.h*(1.0-marginY))
		elif (self.mode=='normalized_scaled'):
			""" normalized coordinates: top-left = (0,0), bottom-right = (1,1) - margins WILL NOT be applied """
			return ((2.0*self.x-1.0),(-2.0*self.y+1.0),2.0*self.w,2.0*self.h)
		elif (self.mode=='normalized'):
			""" normalized coordinates: top-left = (0,0), bottom-right = (1,1) - margins WILL be applied """
			return ((2.0*self.x-1.0)*(1.0-marginX),(-2.0*self.y+1.0)*(1.0-marginY),2.0*self.w*(1.0-marginX),2.0*self.h*(1.0-marginY))
		else:
			trace(_GUITraceLevel, "WARNING! - gui.py - GUIRect::getNormalizedCoords(): unknown coordinate mode\n")

	def getNormalCenter(self):
		aux = self.getNormalXYWH()
		return ( aux[0]+aux[2]/2, aux[1]-aux[3]/2 )

	def getNormalTL(self):
		""" returns TOP-LEFT coordinate pair """
		aux = self.getNormalXYWH()
		return ( aux[0], aux[1] )

	def getNormalBR(self):
		""" returns BOTTOM-RIGHT coordinate pair """
		aux = self.getNormalXYWH()
		return ( aux[0]+aux[2], aux[1]-aux[3] )

	def getNormalWH(self):
		""" returns WIDTH-HEIGHT dimensions """
		aux = self.getNormalXYWH()
		return ( aux[2], aux[3] )

	#
	# The Base module uses inconsistent coordinates.  
	# For the Link/Comp/Python methods, it is the bottom left.
	# For the Texture methods, it is the center point.
	# For the TextBox method, it is the top left?
	#
	def getHotRect(self):
		""" (BOTTOM, LEFT, WIDTH, HEIGHT) as needed by Base hotspots """
		aux = self.getNormalXYWH()
		return ( aux[0], aux[1]-aux[3], aux[2], aux[3] )

	def getSpriteRect(self):
		""" (CenterX, CenterY, WIDTH, -HEIGHT) as needed by Base sprites """
		aux = self.getNormalXYWH()
		return ( aux[0]+aux[2]/2, aux[1]-aux[3]/2, aux[2], -aux[3] )

	def getTextRect(self):
		""" (TOP, LEFT, WIDTH, HEIGHT) as needed by Base textboxes """
		return self.getNormalXYWH()


"""----------------------------------------------------------------"""
"""                                                                """
""" GUINPOTRect - defines rectangles on the screen.                """
"""     Adjusts a rectangle so that a Power-Of-Two texture         """
"""     with Non-Power-Of-Two contents (top/left aligned)          """
"""     displays its contents in the specified area.               """
"""                                                                """
"""----------------------------------------------------------------"""

class GUINPOTRect(GUIRect):
	def __init__(self,x,y,wid,hei,npotw,npoth,mode="pixel"):
		""" Adjust """
		potw = 1
		poth = 1
		while potw < npotw:
			potw = potw * 2
		while poth < npoth:
			poth = poth * 2

		""" Store original parameters """
		self.potw = potw
		self.poth = poth
		self.npotw = npotw
		self.npoth = npoth

		""" Initialized base class """
		GUIRect.__init__(self,x,y,wid*potw/npotw,hei*poth/npoth,mode)
	
	def __repr__(self):
		return GUIRect.__repr__(self)

	def __str__(self):
		return GUIRect.__str__(self)


"""----------------------------------------------------------------"""
"""                                                                """
""" GUIColor - an RGBA color representation                        """
"""      values range from 0.0 to 1.0.                             """
"""                                                                """
"""----------------------------------------------------------------"""
class GUIColor:
	def __init__(self,r,g,b,a=1.0):
		self.r = r
		self.g = g
		self.b = b
		self.a = a
		
	def __repr__(self):
		return "RGB(%f,%f,%f,%f)" % (self.r,self.g,self.b,self.a)
	
	def __str__(self):
		return "RGB(%.2f,%.2f,%.2f,%.2f)" % (self.r,self.g,self.b,self.a)
	
	def getRGB(self):
		# create a tuple
		t = (self.r, self.g, self.b)
		return (t)
	
	def getAlpha(self):
		# create a tuple
		return (self.a)

	@staticmethod
	def white():
		return GUIColor(1.0,1.0,1.0)

	@staticmethod
	def black():
		return GUIColor(0.0,0.0,0.0)

	@staticmethod
	def clear():
		return GUIColor(0.0,0.0,0.0,0.0)



"""----------------------------------------------------------------"""
"""                                                                """
""" GUIRoom - interface for rooms. Lets you define the layout of   """
"""     a room in high-level terms, and implements serialization   """
"""     of room layouts.                                           """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIRoom:
	def __init__(self,index):
		self.index = index
		self.needRedraw = 0
		self.needPreserveZ = 0
		self.screenDimensions = None
		self.screenMargins = None
		# add this GUIRoom to GUIRootSingleton
		GUIRootSingleton.registerRoom(self)
		
	def __repr__(self):
		return "<room %r>" % self.index
	
	def __str__(self):
		return "Room %s" % self.index

	def getIndex(self):
		return self.index

	def redrawIfNeeded(self):
		if self.needRedraw != 0:
			self.redraw()

	def redraw(self):
		if self.needPreserveZ != 0:
			""" Sadly, this mess is required to preserve z-order - some day we'll have a proper redraw in place """
			GUIRootSingleton.broadcastRoomMessage(self.getIndex(),'undraw',None)
			GUIRootSingleton.broadcastRoomMessage(self.getIndex(),'draw',None)
		else:
			GUIRootSingleton.broadcastRoomMessage(self.getIndex(),'redraw',None)
		self.needRedraw = 0
		self.needPreserveZ = 0

	def notifyNeedRedraw(self,preserveZ=1):
		self.needRedraw = 1
		if preserveZ != 0:
			self.needPreserveZ = 1
	
	def getScreenDimensions(self):
		global GUIRootSingleton
		if self.screenDimensions:
			return self.screenDimensions
		else:
			return GUIRootSingleton.getScreenDimensions()

	def getScreenMargins(self):
		global GUIRootSingleton
		if self.screenDimensions:
			return self.screenMargins
		else:
			return GUIRootSingleton.getScreenMargins()

	def setScreenDimensions(self,dimensions):
		global GUIRootSingleton
		self.screenDimensions = dimensions
		dims = self.getScreenDimensions()
		GUIRootSingleton.broadcastRoomMessage(self.index,'changedScreenDimensions', {'screenX':dims[0],'screenY':dims[1]} )

	def setScreenMargins(self,margins):
		global GUIRootSingleton
		self.screenMargins = margins
		mars = self.getScreenMargins()
		GUIRootSingleton.broadcastMessage(self.index,'changedScreenMargins', {'marginX':mars[0],'marginY':mars[1]} )


"""----------------------------------------------------------------"""
"""                                                                """
""" GUIElement - it's the base class for all GUI elements          """
"""     Implements standard methods and message handling           """
"""                                                                """
"""----------------------------------------------------------------"""


class GUIElement:
	def __init__(self,room,owner=None):
		self.room = room
		self.owner = owner
		self.visible = 1
		self.redrawPreservesZ = 0
		self.id = GUIRootSingleton.registerObject(room.getIndex(),self)

	def __str__(self):
		return "GUIElement: room = %s, visible = %s, redrawPreservesZ = %s, id = %s" %(self.room, self.visible, self.redrawPreservesZ, self.id)

	def __repr__(self):
		# this needs work
		# would like it to say <GUI.GUIElementSubclass a = 1, b= 2, ...> for any subclass of GUIElement
		r = "<%s: " %(type(self)) #, self.room, self.visible, self.redrawPreservesZ, self.id)
		for key in dir(self):
			try:
				value = getattr(self, key)
				if callable(value):
					value = ''
			except:
				value = ''
			if value != '':
				r = r + "%s = %s, " %(key, value)
		r = r + ">"
		return r
		
	def show(self):
		self.visible=1
		self.notifyNeedRedraw()

	def hide(self):
		""" Usually does not need a full redraw """
		self.visible=0
		self.undraw()

	def notifyNeedRedraw(self,preserveZ=1):
		self.room.notifyNeedRedraw(preserveZ)

	def draw(self):
		""" Intentionally blank """
	
	def undraw(self):
		""" Intentionally blank """

	def redraw(self):
		""" override to preserve Z-order if possible """
		""" (if so, mark self.redrawPreservesZ) """
		self.undraw()
		self.draw()

	def onMessage(self,message,params):
		"""Standard message dispatch"""
		if (message=='click'):
			self.onClick(params)
		elif (message=='show'):
			self.onShow(params)
		elif (message=='hide'):
			self.onHide(params)
		elif (message=='draw'):
			self.onDraw(params)
		elif (message=='undraw'):
			self.onUndraw(params)
		elif (message=='redraw'):
			self.onRedraw(params)
		else:
			return False
		return True


	def onClick(self,params):
		""" Intentionally blank """

	def onShow(self,params):
		self.show()

	def onHide(self,params):
		self.hide()

	def onDraw(self,params):
		if self.visible:
			self.draw()

	def onUndraw(self,params):
		if self.visible:
			self.undraw()

	def onRedraw(self,params):
		""" WARNING! does not necessarily preserve z-order, so only issue if you don't care about z-order """
		if self.visible:
			self.redraw()

	def focus(self,dofocus):
		if dofocus:
			GUIRootSingleton.keyTarget=self
		elif GUIRootSingleton.keyTarget==self:
			GUIRootSingleton.keyTarget=None

	def setModal(self,modal):
		if modal:
			GUIRootSingleton.modalElement=self
			GUIRootSingleton.broadcastMessage('disable',{})
			if 'enable' in dir(self):
				self.enable()
			GUIRootSingleton.broadcastRoomMessage('redraw',{})
		elif GUIRootSingleton.modalElement == self:
			GUIRootSingleton.modalElement=None
			GUIRootSingleton.broadcastMessage('enable',{})

	def isInteractive(self):
		return GUIRootSingleton.modalElement==self or GUIRootSingleton.modalElement==None
		

"""----------------------------------------------------------------"""
"""                                                                """
""" GUIGroup - allows grouping of elements. Notice that GUIGroup   """
"""     will relay messages to its attached elements, and will     """
"""     keep them alive (by holding references to them)            """
"""                                                                """
"""----------------------------------------------------------------"""


class GUIGroup(GUIElement):
	def __init__(self,room,**kwargs):
		GUIElement.__init__(self,room,**kwargs)
		self.children = []

	def show(self):
		for i in self.children:
			i.show()
		GUIElement.show(self)

	def hide(self):
		for i in self.children:
			i.hide()
		GUIElement.hide(self)

	def draw(self):
		""" Intentionally blank """
	
	def undraw(self):
		""" Intentionally blank """

	def redraw(self):
		""" Intentionally blank """

	def onMessage(self,message,params):
		for i in self.children:
			i.onMessage(message,params)




"""----------------------------------------------------------------"""
"""                                                                """
""" GUIStaticImage - a non-interactive image                       """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIStaticImage(GUIElement):
	def __init__(self,room,index,sprite,**kwarg):
		""" Sprite must be a tuple of the form:     """
		"""     ( path , location )                 """
		"""     with 'location' being a GUIRect     """
		""" NOTE: It is legal to set sprite to None """
		"""     This allows subclassing to create   """
		"""     non-static elements                 """

		GUIElement.__init__(self,room,**kwarg)
		
		self.sprite=sprite
		self.index=index
		self.spritestate=0
		self.redrawPreservesZ=1
		
	def spriteIsValid(self):
		return ( self.sprite 
			and type(self.sprite) is tuple 
			and len(self.sprite)>=2 
			and self.sprite[0] is not None 
			and self.sprite[1] is not None )

	def draw(self):
		""" Creates the element """
#		if (self.visible == 0):
#			print "::: GUIStaticImage draw called when self.visible == 0"
		if (self.visible == 1) and (self.spritestate==0) and self.spriteIsValid():
			(x,y,w,h) = self.sprite[1].getSpriteRect()
			Base.Texture(self.room.getIndex(),self.index,self.sprite[0],x,y)
			Base.SetTextureSize(self.room.getIndex(),self.index,w,h) # override spr file data... it's hideously unmantainable...
			self.spritestate=1
	
	def undraw(self):
		""" Hides the element """
		if self.spritestate==1:
			Base.EraseObj(self.room.getIndex(),self.index)
			self.spritestate=0

	def redraw(self):
		""" Sets a new image """
		if self.spritestate!=1:
			self.draw()
		elif self.spriteIsValid():
			(x,y,w,h) = self.sprite[1].getSpriteRect()
			Base.SetTexture(self.room.getIndex(),self.index,self.sprite[0]);
			Base.SetTexturePos(self.room.getIndex(),self.index,x,y);
			Base.SetTextureSize(self.room.getIndex(),self.index,w,h);
		else:
			# Avoid calling subclass implementations
			GUIStaticImage.undraw(self)

	def setSprite(self,newsprite):
		if self.sprite != newsprite:
			self.sprite = newsprite
			self.notifyNeedRedraw()



"""----------------------------------------------------------------"""
"""                                                                """
""" GUIStaticText - a non-interactive text box                     """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIStaticText(GUIElement):
	def __init__(self,room,index,text,location,color,fontsize=1.0,bgcolor=None,**kwarg):
		GUIElement.__init__(self,room,**kwarg)
		
		self.index=index
		self.textstate = 0
		self.location = location
		self.color = color
		if (bgcolor != None):
			self.bgcolor = bgcolor
		else:
			self.bgcolor = GUIColor.clear()
		self.fontsize = fontsize
		self.text = text

	def draw(self):
		""" Creates the element """
		if (self.textstate==0):
			(x,y,w,h) = self.location.getTextRect()
			# the dimensions for Base.TextBox are all screwed up.  the (width height multiplier) value is actually (x2, y2, unused)
			# and the text is always the same size, regardless of how the height or multiplier values get set
			Base.TextBox(self.room.getIndex(), str(self.index), str(self.text), x, y, (x + w, y - h, self.fontsize), self.bgcolor.getRGB(), self.bgcolor.getAlpha(), self.color.getRGB())
			if _doWhiteHack != 0:
				""" ugly hack, needed to counter a stupid bug """
				Base.TextBox(self.room.getIndex(),str(self.index)+"_white_hack","", -100.0, -100.0, (0.01, 0.01, 1), (0,0,0), 0, GUIColor.white().getRGB())
			self.textstate=1
	
	def undraw(self):
		""" Hides the element """
		if self.textstate==1:
			Base.EraseObj(self.room.getIndex(),str(self.index))
			if _doWhiteHack != 0:
				Base.EraseObj(self.room.getIndex(),self.index+"_white_hack")
			self.textstate=0

	def setText(self,newtext):
		self.text = newtext
		if self.textstate==1:
			Base.SetTextBoxText(self.room.getIndex(),str(self.index),str(self.text))

	def setColor(self,newcolor):
		self.color = newcolor
		self.notifyNeedRedraw()

	def getText(self):
		return self.text

	def getColor(self):
		return self.color


"""----------------------------------------------------------------"""
"""                                                                """
""" GUILineEdit - an interactive text box                          """
"""                                                                """
"""----------------------------------------------------------------"""

#todo: add optional frame

class GUILineEdit(GUIGroup):

	def focus_text(self,button,params):
		print 'focusing',self.index
		self.focus(True)
	def __init__(self,action,room,index,text,location,color,fontsize=1.0,bgcolor=None,focusbutton=None,**kwarg):
		GUIGroup.__init__(self,room,**kwarg)
		self.text = GUIStaticText(room,index,' '+text+'-',location,color,fontsize,bgcolor,**kwarg)
		self.children.append(self.text)
		if focusbutton:
			self.focusbutton = GUIButton(room,"XXXFocus Element",str(index)+'focus',{'*':None},location,
				clickHandler=self.focus_text)
			self.children.append(self.focusbutton)
		(x,y,w,h) = location.getNormalXYWH()
		(uw,uh) = GUIRect(0,0,10,10).getNormalWH()
		self.index=index
		Base.TextBox(room.getIndex(), str(self.index)+'line1', '---', x, y, (x+w, y+uh, 1), 
			     color.getRGB(), color.getAlpha(), color.getRGB())
		Base.TextBox(room.getIndex(), str(self.index)+'line2', '!', x, y, (x-uw, y+h, 1), 
			     color.getRGB(), color.getAlpha(), color.getRGB())
		Base.TextBox(room.getIndex(), str(self.index)+'line3', '---', x, y-h, (x+w, y+uh, 1), 
			     color.getRGB(), color.getAlpha(), color.getRGB())
		Base.TextBox(room.getIndex(), str(self.index)+'line4', '!', x+w, y, (x+uw, y+h, 1),
			     color.getRGB(), color.getAlpha(), color.getRGB())
		if _doWhiteHack != 0:
			""" ugly hack, needed to counter a stupid bug """
			Base.TextBox(self.room.getIndex(),str(self.index)+"_white_hack","", -100.0, -100.0, (0.01, 0.01, 1), (0,0,0), 0, GUIColor.white().getRGB())
		self.action=action
		self.draw()
		self.canceled = False

	def getText(self):
		return self.text.getText()[1:-1]

	def undraw(self):
		Base.EraseObj(self.room.getIndex(),str(self.index)+"line1")
		Base.EraseObj(self.room.getIndex(),str(self.index)+"line2")
		Base.EraseObj(self.room.getIndex(),str(self.index)+"line3")
		Base.EraseObj(self.room.getIndex(),str(self.index)+"line4")
		if _doWhiteHack != 0:
			Base.EraseObj(self.room.getIndex(),self.index+"_white_hack")
		GUIGroup.undraw(self)

	def keyDown(self,key):
		print "got key: %i" % key 
		if key == 13 or key == 10: #should be some kind of return
			self.action(self)
		elif key == 27: #escape is always 27, isn't it?
			self.canceled = True
			self.action(self)		
		elif key == 127 or key == 8: #avoid specifying the platform by treating del and backspace alike
			self.text.setText(' ' + self.getText()[:-1] + '-')
		elif key<127 and key>0:
			try:
				self.text.setText(' '+self.getText() + ('%c' % key) + '-');
			except:
				print "Character value too high "+str(key)
		#self.notifyNeedRedraw()

"""------------------------------------------------------------------"""
"""                                                                  """
""" GUITextInputDialog - a little dialog in which you can enter text """
"""                                                                  """
"""------------------------------------------------------------------"""


class GUITextInputDialog(GUIGroup):
	def __init__(self,room,index,location,text,action,color,**kwargs):
		GUIGroup.__init__(self,room,kwargs)
		self.children.append(GUILineEdit(self.editcallback,room,index,text,location,color))
		



class GUIMouseOver(GUIElement):
	def __init__(self,room,linkdesc,index,hotspot,**kwarg):
		self.linkdesc=linkdesc
		self.index=index
		self.hotspot=hotspot
		self.linkstate=0
		GUIElement.__init__(self,room,**kwarg)
	
	def draw(self):
		if self.visible and self.linkstate==0:
			(x,y,w,h) = self.hotspot.getHotRect()
			Base.Python(self.room.getIndex(),self.index,x,y,w,h,self.linkdesc,'#',True)
			self.linkstate=1
	
	def undraw(self):
		if self.linkstate==1:
			self.linkstate=0
			Base.EraseLink(self.room.getIndex(),self.index)
			
	def redraw(self):
		self.undraw()
		self.draw()
	
	
"""----------------------------------------------------------------"""
"""                                                                """
""" GUIButton - a button you can click on.                         """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIButton(GUIStaticImage):
	def __init__(self,room,linkdesc,index,spritefiles,hotspot,initialstate='enabled',clickHandler=None,textcolor=GUIColor.white(),textbgcolor=None,textfontsize=1.0,**kwarg):
		""" Initializes the button (but does not draw it; use drawobjs())  """
		""" spritefiles: a dictionary, mapping states to sprites           """
		"""     'enabled' : normal, default, enabled state                 """
		"""     'disabled': disabled (grayed) state                        """
		"""     'hot'     : hot state (mouse over)                         """
		"""     'down'    : down state (mouse clicking)                    """
		"""     '*'       : fallback state                                 """
		""" Each state in spriteifiles must be a tuple of the form:        """
		"""     ( path , location )                                        """
		"""     or ( path, location, text [,textattrs] )                   """
		"""     ( ) - empty, for "no change"                               """
		"""      with 'location' being a GUIRect                           """
		"""      and  'text' being an optional overlaid text element       """

		self.sprites=spritefiles
		self.hotspot=hotspot
		self.linkdesc=linkdesc
		self.linkstate=0
		self.state=initialstate
		self.group=index
		self.enabled=(initialstate!='disabled')
		self.clickHandler = clickHandler
		self.textOverlay = None
		self.textcolor = textcolor
		self.textbgcolor = textbgcolor
		self.textfontsize = textfontsize

		""" Init base class """
		GUIStaticImage.__init__(self,room,index,self._getStateSprite(self.state),**kwarg)
		self.textOverlay = GUIStaticText(self.room,self.index+"__text_overlay","",
			self.hotspot,self.textcolor,self.textfontsize,self.textbgcolor)
		self.textOverlay.hide()

		self.pythonstr = \
                  "# <-- this disables precompiled python objects\n" \
                 +"from GUI import GUIRootSingleton\n" \
		     +"evData = Base.GetEventData()\n" \
		     +"typeToMessage = {'click':'click','up':'up','down':'down','move':'move','enter':'enter','leave':'leave'}\n" \
		     +"if ('type' in evData) and (evData['type'] in typeToMessage):\n" \
                 +"\tGUIRootSingleton.dispatchMessage("+str(self.id)+",typeToMessage[evData['type']],evData)\n" \
                 +"\tGUIRootSingleton.redrawIfNeeded()\n"

	def _getStateSprite(self,state):
		if self.sprites:
			if (state in self.sprites):
				sprite = self.sprites[state]
			elif ('*' in self.sprites):
				sprite = self.sprites['*']
			else:
				sprite = None
				# this is frequently ok (ie, when a button is just a region on the screen, not a separate sprite)
				trace(_GUITraceLevel + 2, "WARNING! - gui.py - GUIButton::_getStateSprite(): can't map sprite, %s not in %s\n" % (state,self.sprites))
			if sprite and ( (type(sprite)!=tuple) or (len(sprite)<2) ):
				if sprite:
					trace(_GUITraceLevel, "WARNING! - gui.py - GUIButton::_getStateSprite(): type error in sprite map\n")
				sprite = None
		else:
			sprite = None
		return sprite

	def onMouseUp(self,params):
		if self.getState()=='down':
			self.setNeutralState()

	def onMouseDown(self,params):
		if self.isEnabled() and self.hasState('down'):
			self.setState('down')

	def onMouseEnter(self,params):
		if self.isEnabled() and self.hasState('hot'):
			self.setState('hot')

	def onMouseLeave(self,params):
		if self.getState()=='hot' or self.getState()=='down':
			self.setNeutralState()

	def onMouseMove(self,params):
		""" Intentionally blank """

	def onClick(self,params):
		if self.clickHandler:
			self.clickHandler(self,params)
		GUIStaticImage.onClick(self,params)

	def draw(self):
		""" Creates the button """
		if (self.linkstate==0) and (self.visible==1) and self.enabled:
			# getHotRect returns the BOTTOM-left x,y needed by Base.Python 
			(x,y,w,h) = self.hotspot.getHotRect()
			Base.Python(self.room.getIndex(),self.index,x,y,w,h,self.linkdesc,self.pythonstr,True)
			Base.SetLinkEventMask(self.room.getIndex(),self.index,'cduel')
			self.linkstate=1
		self.setState(self.state)
		GUIStaticImage.draw(self)
		if self.textOverlay.visible:
			self.textOverlay.draw()
	
	def setNeutralState(self):
		if self.isEnabled():
			self.setState('enabled')
		else:
			self.setState('disabled')

	def undraw(self):
		"""Hides the button"""
		if self.linkstate==1:
			Base.EraseLink(self.room.getIndex(),self.index)
			self.linkstate=0
		GUIStaticImage.undraw(self)
		self.textOverlay.undraw()

	def hide(self):
		GUIStaticImage.hide(self)
		self.textOverlay.hide()
	
	def show(self):
		GUIStaticImage.show(self)
		self.textOverlay.show()

	def redraw(self):
		""" Creates the button """
		if (self.linkstate==1) and (self.visible==1) and self.enabled:
			(x,y,w,h) = self.hotspot.getHotRect()
			Base.SetLinkArea(self.room.getIndex(),self.index,x,y,w,h)
			Base.SetLinkText(self.room.getIndex(),self.index,self.linkdesc)
			Base.SetLinkPython(self.room.getIndex(),self.index,self.pythonstr)
			self.linkstate=1
			self.setState(self.state)
			GUIStaticImage.redraw(self)
			if self.textOverlay.visible:
				self.textOverlay.redraw()
		else:
			self.undraw()
			self.draw()
			
	def setCaption(self,caption,attrs=None):
		if caption is not None:
			if not self.textOverlay.visible:
				self.textOverlay.show()
			if attrs is None:
				self.textOverlay.setText(str(caption))
			else:
				self.textOverlay.text = str(caption)
				self.textOverlay.color = attrs.get('color',self.textOverlay.color)
				self.textOverlay.bgcolor = attrs.get('bgcolor',self.textOverlay.bgcolor)
				self.textOverlay.fontsize = attrs.get('fontsize',self.textOverlay.fontsize)
				self.textOverlay.notifyNeedRedraw()
		else:
			self.textOverlay.hide()


	def setState(self,newstate):
		self.state = newstate
		spr = self._getStateSprite(self.state)
		self.setSprite(spr)
		if spr and len(spr)>2:
			if len(spr)>3:
				self.setCaption(spr[2],spr[3])
			else:
				self.setCaption(spr[2])
		elif spr:
			self.setCaption(None)

	def getState(self):
		return self.state

	def hasState(self,state):
		return self.sprites and (state in self.sprites)

	def isEnabled(self):
		return self.enabled

	def enable(self):
		self.enabled=True
		self.setNeutralState()

	def disable(self):
		self.enabled=False
		self.setNeutralState()

	def setEnable(self,state):
		if state:
			self.enable()
		else:
			self.disable()

	def getGroup(self):
		return self.group

	def setGroup(self,group):
		self.group = group

	def onMessage(self,message,params):
		# Button-specific actions
		if (message=='enable'):
			if (not ('group' in params) or (self.getGroup() == params['group'])) and (not ('exclude' in params) or (self.id != params['exclude'])):
				self.enable()
		elif (message=='disable'):
			if (not ('group' in params) or (self.getGroup() == params['group'])) and (not ('exclude' in params) or (self.id != params['exclude'])):
				self.disable()
		# Button-specific mouse events
		elif self.isInteractive():
			if (message=='move'):
				self.onMouseMove(params)
			elif (message=='up'):
				self.onMouseUp(params)
			elif (message=='down'):
				self.onMouseDown(params)
			elif (message=='enter'):
				self.onMouseEnter(params)
			elif (message=='leave'):
				self.onMouseLeave(params)		
			# Fallback
			else:
				GUIStaticImage.onMessage(self,message,params)



"""----------------------------------------------------------------"""
"""                                                                """
""" GUICompButton - a button you can click on that takes you       """
"""                 to the original computer interface             """
"""                                                                """
"""----------------------------------------------------------------"""

class GUICompButton(GUIButton):
	def __init__(self,room,modes,*parg,**kwarg):
		self.compmodes = modes

		""" Init base class """
		GUIButton.__init__(self,room,*parg,**kwarg)

	def draw(self):
		""" Creates the button """
		if (self.linkstate==0) and (self.visible==1) and self.enabled:
			# getHotRect returns the BOTTOM-left x,y needed by Base.Python 
			(x,y,w,h) = self.hotspot.getHotRect()
			# possible bug: not set to frontmost.
			Base.CompPython(self.room.getIndex(),self.index,self.pythonstr,x,y,w,h,self.linkdesc,self.compmodes)
			Base.SetLinkEventMask(self.room.getIndex(),self.index,'cduel')
			self.linkstate=1
		self.setState(self.state)
		GUIStaticImage.draw(self)
		if self.textOverlay.visible:
			self.textOverlay.draw()

"""----------------------------------------------------------------"""
"""                                                                """
""" GUIRoomButton - a button you can click on that takes you       """
"""                 to another room                                """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIRoomButton(GUIButton):
	def __init__(self,room,targetroom,*parg,**kwarg):
		self.target = targetroom

		""" Init base class """
		GUIButton.__init__(self,room,*parg,**kwarg)

	def onClick(self,params):
		Base.SetCurRoom(self.target.getIndex())
		GUIButton.onClick(self,params)


"""----------------------------------------------------------------"""
"""                                                                """
""" GUICheckButton - a button you can click on, which toggles      """
"""      between on/off states.                                    """
"""                                                                """
"""----------------------------------------------------------------"""

class GUICheckButton(GUIButton):
	def __init__(self,room,linkdesc,index,spritefiles,hotspot,**kwarg):
		""" Initializes the button (but does not draw it; use drawobjs())  """
		""" spritefiles: a dictionary, mapping states to sprites           """
		"""     'checked'   : normal, enabled and checked state            """
		"""     'unchecked' : normal, enabled and unchecked state          """
		"""     'disabled'  : disabled (grayed) state                      """
		"""     'hot'       : hot state (mouse over)                       """
		"""     'down'      : down state (mouse clicking)                  """
		"""     '*'         : fallback state                               """
		""" Each state in spriteifiles must be a tuple of the form:        """
		"""     ( path , location )                                        """
		"""     ( ) - empty, for "no change"                               """
		"""      with 'location' being a GUIRect                           """

		GUIButton.__init__(self,room,linkdesc,index,spritefiles,hotspot,'unchecked',**kwarg)
		self.checked = 0

	def setNeutralState(self):
		if self.isEnabled():
			if self.isChecked():
				self.setState('checked')
			else:
				self.setState('unchecked')
		else:
			self.setState('disabled')

	def isChecked(self):
		return self.checked != 0

	def setChecked(self,check=1):
		if self.checked != check:
			self.checked = check
			self.onChange(None)
		if self.isEnabled():
			self.enable()

	def check(self):
		self.setChecked(1)

	def uncheck(self):
		self.setChecked(0)

	def toggleChecked(self):
		if not self.isChecked():
			self.setChecked(1)
		else:
			self.setChecked(0)

	def onClick(self,params):
		self.toggleChecked()

	def onChange(self,params):
		""" Intentionally blank """

	def onMessage(self,message,params):
		""" Intercept group reset """
		if (    (message=='setcheck' or message=='check' or message=='uncheck') \
		    and (not ('group' in params) or (self.group == params['group'])) \
		    and (not ('exclude' in params) or (self.id != params['exclude'])) \
		    and (not ('index' in params) or (self.index == params['index']))   ):
			if (message=='setcheck'):
				if ('state' in params):
					self.setChecked(params['state'])
			elif (message=='check'):
				self.check()
			elif (message=='uncheck'):
				self.uncheck()
		else:
			GUIButton.onMessage(self,message,params)



"""----------------------------------------------------------------"""
"""                                                                """
""" GUIRadioButton - a button you can click on, which toggles      """
"""      between on/off states. Only one button on a group will    """
"""      be allowed to be in the checked state.                    """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIRadioButton(GUICheckButton):
	def __init__(self,room,linkdesc,index,spritefiles,hotspot,radiogroup,value=None,onChange=(lambda group,newval,caller:None),**kwarg):
		""" Initializes the button (but does not draw it; use drawobjs())  """
		""" spritefiles: a dictionary, mapping states to sprites           """
		"""     'checked'   : normal, enabled and checked state            """
		"""     'unchecked' : normal, enabled and unchecked state          """
		"""     'disabled'  : disabled (grayed) state                      """
		"""     'hot'       : hot state (mouse over)                       """
		"""     'down'      : down state (mouse clicking)                  """
		"""     '*'         : fallback state                               """
		""" Each state in spriteifiles must be a tuple of the form:        """
		"""     ( path , location )                                        """
		"""     ( ) - empty, for "no change"                               """
		"""     with 'location' being a GUIRect                            """
		""" NOTE FOR SUBLCASSERS:                                          """
		"""     onChange() may and will be issued for ANY changed element  """
		"""     that means that it would be wise to only perform an action """
		"""     if self.isChecked() - also, be sure to call the base       """
		"""     implementation, or the radio button won't work properly.   """


		GUICheckButton.__init__(self,room,linkdesc,index,spritefiles,hotspot,**kwarg)
		self.setGroup(radiogroup)
		self.uncheck()
		self.onChange_fn = onChange
		self.value = value

	def onChange(self,params):
		if self.isChecked():
			GUIRootSingleton.broadcastRoomMessage(self.room.getIndex(),'uncheck', { 'group':self.group,'exclude':self.id } )
			self.onChange_fn(self.group,self.value,self)

	def onClick(self,params):
		# radio buttons can't be unchecked; another button in the group has to be clicked instead
		if (not self.isChecked()):
			GUICheckButton.onClick(self,params)
		
	def groupUncheck(self):
		GUIRadioButton.staticGroupUncheck(self.room,self.getGroup())
	
	@staticmethod
	def staticGroupUncheck(room,group):
		GUIRootSingleton.broadcastRoomMessage(room.getIndex(),'uncheck', { 'group':group } )


"""----------------------------------------------------------------"""
"""                                                                """
""" GUISimpleListPicker - a simple (featureless) list picker       """
"""                                                                """
"""----------------------------------------------------------------"""
class GUISimpleListPicker(GUIElement):
	class managedlist(list):
		def __init__(self,iterable=[],onChange=(lambda me,kind,key:None),owner=None):
			self.onChange = onChange
			self.owner = owner
			self[:] = iterable
		def __setitem__(self,key,value):
			self.onChange(self,'set',key)
			return super(type(self),self).__setitem__(key,value)
		def __delitem__(self,key):
			self.onChange(self,'del',key)
			return super(type(self),self).__delitem__(key)
		def __setslice__(self,i,j,sequence):
			self.onChange(self,'setslice',(i,j,sequence))
			return super(type(self),self).__setslice__(i,j,sequence)
		def __delslice__(self,i,j):
			self.onChange(self,'delslice',(i,j))
			return super(type(self),self).__delslice__(i,j)
			return setattr(super(type(self),self),name,value)
		def append(self,item):
			self.onChange(self,'append',None)
			return super(type(self),self).append(item)
		def remove(self,item):
			del self[self.index(item)]
		def extend(self,iterable):
			self[len(self):] = iterable
		def insert(self,i,value):
			self[i:i] = [value]
		
	class listitem:
		def __init__(self,string,data):
			self.string = string
			self.data = data
		def __repr__(self):
			return "[%r,%r]" % (self.string,self.data)
		def __str__(self):
			return self.string
		
		
	def __init__(self,room,linkdesc,index,hotspot,textcolor=GUIColor.white(),textbgcolor=GUIColor.clear(),textfontsize=1.0,selectedcolor=GUIColor.white(),selectedbgcolor=GUIColor.black(),**kwargs):
		GUIElement.__init__(self,room,**kwargs)
		self.linkdesc = linkdesc
		self.index = index
		self.hotspot = hotspot
		self.__dict__['items'] = GUISimpleListPicker.managedlist(onChange=self._notifyListChange,owner=self)
		self.selection = None
		self.firstVisible = 0
		self._listitems = []
		self.textcolor = textcolor
		self.textbgcolor = textbgcolor
		self.selectedcolor = selectedcolor
		self.selectedbgcolor = selectedbgcolor
		self.textfontsize = textfontsize
		self.selectedattrs = dict(color=selectedcolor,bgcolor=selectedbgcolor,fontsize=textfontsize)
		self.unselectedattrs = dict(color=textcolor,bgcolor=textbgcolor,fontsize=textfontsize)
		self.createListItems()
		
	def __setattr__(self,name,value):
		if name == 'items':
			# Preserves type
			self.items[:] = value
		else:
			if name in ['textcolor','textbgcolor','textfontsize']:
				self.notifyNeedRedraw(0)
			self.__dict__[name] = value

	@staticmethod
	def _notifyListChange(lst,kind,key):
		lst.owner.notifyNeedRedraw(0)
		
	@staticmethod
	def _notifySelectionChange(group,newval,caller):
		print "New selection: %s" % newval
		caller.owner.selection = newval + caller.owner.firstVisible
		
	def _radiogroup(self):
		return self.index+"_slp_rg"
	
	def _recheck(self):
		if self.selection is not None:
			visindex = self.selection - self.firstVisible
			if visindex < 0 or visindex >= len(self._listitems):
				self._listitems[0].groupUncheck()
			else:
				self._listitems[visindex].check()
	
	def destroyListItems(self):
		for l in self._listitems:
			l.undraw()
		self._listitems = []
		
	def createListItems(self):
		self.destroyListItems()
		
		theight = Base.GetTextHeight('|',tuple([self.textfontsize]*3))*1.15 # Need a small margin to avoid text clipping
		spr = { 'checked':(None,None,''), 'unchecked':(None,None,'') }
		if theight<=0:
			return
		nlines = int(self.hotspot.getTextRect()[3]/theight)
		if nlines<=0:
			return
		hotx,hoty,hotw,hoth = self.hotspot.getNormalXYWH()
		theight = hoth / nlines 
		for i in range(nlines):
			hot = GUIRect(hotx,hoty-i*hoth/float(nlines),hotw,theight,'normalized_biased_scaled')
			self._listitems.append( GUIRadioButton(self.room,self.linkdesc,"%s[%s]" % (self.index,i),spr,hot,self._radiogroup(),i,onChange=self._notifySelectionChange,owner=self) )
			i += 1
			
	def _visItemText(self,i):
		if i+self.firstVisible < len(self.items):
			txt = str(self.items[i+self.firstVisible])
		else:
			txt = ""
		return txt
		
	def _updateListItemText(self):
		for i in range(len(self._listitems)):
			txt = self._visItemText(i)
			self._listitems[i].sprites = { 
				'checked':(None,None,txt,self.selectedattrs), 
				'unchecked':(None,None,txt,self.unselectedattrs),
				'disabled':(None,None) }
			self._listitems[i].setEnable((self.firstVisible + i) < len(self.items))
			self._listitems[i].notifyNeedRedraw()
	
	def draw(self):
		self._updateListItemText()
		for item in self._listitems:
			item.draw()
	def undraw(self):
		for item in self._listitems:
			item.undraw()
	def redraw(self):
		if len(self._listitems) <= 0:
			self.createListItems()
		self._updateListItemText()
		for it in self._listitems:
			it.redraw()
	def show(self):
		GUIElement.show(self)
		for item in self._listitems:
			item.show()
	def hide(self):
		GUIElement.hide(self)
		for item in self._listitems:
			item.hide()

	def pageMove(self,nPages):
		self.viewMove(nPages*len(self._listitems))
		self._recheck()
	
	def viewMove(self,lines):
		self.firstVisible = max(0,min(len(self.items)-1-len(self._listitems)/2,self.firstVisible + lines))
		self.notifyNeedRedraw()
		self._recheck()
	


"""----------------------------------------------------------------"""
"""                                                                """
""" GUIVideoTexture - a non-interactive video texture              """
"""                     (no audio)                                 """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIVideoTexture(GUIStaticImage):
    def __init__(self,room,index,sprite,**kwarg):
        """ Sprite must be a tuple of the form:     """
        """     ( path , location )                 """
        """     with 'location' being a GUIRect     """
        """ NOTE: It is legal to set sprite to None """
        """     This allows subclassing to create   """
        """     non-static elements                 """

        GUIStaticImage.__init__(self,room,index,sprite,**kwarg)
        
        # it does not for this subclass
        self.redrawPreservesZ=0
        
    def draw(self):
        """ Creates the element """
        if (self.visible == 1) and (self.spritestate==0) and self.spriteIsValid():
            (x,y,w,h) = self.sprite[1].getSpriteRect()
            Base.Video(self.room.getIndex(),self.index,self.sprite[0],"",x,y)
            Base.SetTextureSize(self.room.getIndex(),self.index,w,h) # override spr file data... it's hideously unmantainable...
            self.spritestate=1
    
    def redraw(self):
        """ Sets a new image """
        if self.spritestate==1:
            self.undraw()
            self.draw()



"""----------------------------------------------------------------"""
"""                                                                """
""" GUIVideoStream - a non-interactive video stream                """
"""                   (with audio)                                 """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIVideoStream(GUIStaticImage):
    def __init__(self,room,index,sprite,**kwarg):
        """ Sprite must be a tuple of the form:     """
        """     ( path , location )                 """
        """     with 'location' being a GUIRect     """
        """ NOTE: It is legal to set sprite to None """
        """     This allows subclassing to create   """
        """     non-static elements                 """
        self.eosHandler = kwarg.pop('eosHandler',None)
        self.startHandler = kwarg.pop('startHandler',None)
        self.stopHandler = kwarg.pop('stopHandler',None)

        GUIStaticImage.__init__(self,room,index,sprite,**kwarg)
        
        # it does not for this subclass
        self.redrawPreservesZ=0
        
        self.nextRoom = None
        self.aspect = None
        
    def draw(self):
        """ Creates the element """
        if (self.visible == 1) and (self.spritestate==0) and self.spriteIsValid():
            pythoncallback = lambda id,event : (
                  "# <-- this disables precompiled python objects\n" 
                 +"from GUI import GUIRootSingleton\n" 
                 +"GUIRootSingleton.dispatchMessage(%r,%r,None)\n" 
                 +"GUIRootSingleton.redrawIfNeeded()\n" ) % (id,event)
            
            (x,y,w,h) = self.sprite[1].getSpriteRect()
            self.spritestate = Base.VideoStream(self.room.getIndex(),self.index,self.sprite[0],x,y,w,h)
            if self.spritestate is None:
                self.spritestate = 1
            if self.spritestate:
                Base.SetVideoCallback(self.room.getIndex(),self.index,pythoncallback(self.id, "eos"))
                self.setAspectRatio(self.aspect)
                Base.RunScript(self.room.getIndex(),self.index+"PLAY",pythoncallback(self.id, "play"),0.0)
            else:
                # Movies are optional, so don't break - skip (immediate EOS) instead
                # Enqueue it to happen as soon as the movie screen is shown 
                Base.RunScript(self.room.getIndex(),self.index+"EOS",pythoncallback(self.id, "eos"),0.0)
    
    def undraw(self):
        """ Hides the element """
        self.stopPlaying()
        GUIStaticImage.undraw(self)

    def redraw(self):
        """ Sets a new image """
        if self.spritestate==1:
            self.undraw()
            self.draw()

    def stopPlaying(self):
        if self.spritestate==1:
            Base.StopVideo(self.room.getIndex(), self.index)
            if self.stopHandler:
                self.stopHandler(self)

    def startPlaying(self):
        if self.spritestate==1:
            Base.PlayVideo(self.room.getIndex(), self.index)
            if self.startHandler:
                self.startHandler(self)

    def setNextRoom(self, nextRoom):
        self.nextRoom = nextRoom

    def setAspectRatio(self, aspect):
        self.aspect = aspect
        if self.spritestate:
            (x,y,w,h) = self.sprite[1].getSpriteRect()
            screenAspect = GUIRootSingleton.getScreenAspectRatio()
            
            xf = yf = 1.0
            if aspect is not None:
                if screenAspect < aspect:
                    # horizontal bars needed
                    yf = screenAspect / aspect
                elif aspect < screenAspect:
                    # vertical bars needed
                    xf = aspect / screenAspect
            
            Base.SetTextureSize(self.room.getIndex(), self.index, w * xf, h * yf)

    def onMessage(self, message, params):
        if not GUIStaticImage.onMessage(self, message, params):
            if message == 'eos':
                self.onEOS(params)
            elif message == 'play':
                self.startPlaying()
            elif message == 'stop':
                self.stopPlaying()
            else:
                return False
            return True
        else:
            return True

    def onEOS(self, params):
        """
        Callback for End-Of-Stream.
        """
        if self.eosHandler:
            self.eosHandler(self, params)


"""----------------------------------------------------------------"""
"""                                                                """
""" GUIMovieRoom - implements a cutscene movie as a room with just """
"""     the movie playing, it will transition to another room      """
"""     when the movie ends, or a "skip" key is pressed, or        """
"""     the mouse clicked anywhere on the screen                   """
"""                                                                """
"""----------------------------------------------------------------"""

class GUIMovieRoom(GUIRoom):
    def __init__(self, index, moviesprite, nextroom):
        GUIRoom.__init__(self, index)
        
        sx, sw = GUIRootSingleton.getScreenDimensions()
        self.aspect = sw * 1.0 / sx
        
        self.video = GUIVideoStream(self, "movie", moviesprite, 
            eosHandler=self.onSkip,
            startHandler=self.onStart,
            stopHandler=self.onStop)
        self.video.setNextRoom(nextroom)
        self.video.setAspectRatio(self.aspect)
        
        self.skipzone = GUIRoomButton(self, nextroom, 
            "XXXskip", "skipmovie", 
            {'*':None},
            GUIRect(0,0,1,1,"normalized"),
            clickHandler = self.onSkip )

    def __repr__(self):
        return "<movie room %r - %r>" % (self.index, self.moviepath)
    
    def __str__(self):
        return "MovieRoom %s %s" % (self.index, self.moviepath)

    def keyDown(self,key):
        if key in (13, 10, 27): # return, return, escape
            self.skipzone.onClick({})

    def onSkip(self, button, params):
        self.video.stopPlaying()
        if self.video.nextRoom:
            Base.SetCurRoom(self.video.nextRoom.getIndex())

    def onStart(self, video):
        pass

    def onStop(self, video):
        pass

    def setAspectRatio(self, ratio):
        self.aspect = ratio
        self.video.setAspectRatio(self.aspect)
