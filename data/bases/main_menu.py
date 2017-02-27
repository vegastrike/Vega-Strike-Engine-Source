# -*- coding: utf-8 -*-
import Base
import VS
import GUI
import ShowProgress

import computer_lib
import dj_lib

# No good reason to disable it...
# dj_lib.disable()

def DoStartNewGame(self,params):
	ShowProgress.activateProgressScreen('loading',3)
	VS.loadGame(VS.getNewGameSaveName())
	enterMainMenu(self,params)

def StartNewGame(self,params):
    Base.SetCurRoom(introroom.getIndex())

#Comment the following line if using intro movies
#StartNewGame = DoStartNewGame

def QuitGame(self,params):
	Base.ExitGame()

# this uses the original coordinate system of Privateer
GUI.GUIInit()

time_of_day=''

# Base music
plist_menu=VS.musicAddList('maintitle.m3u')
plist_credits=VS.musicAddList('maincredits.m3u')

def enterMainMenu(self,params):
	global plist_menu
	VS.musicPlayList(plist_menu)

def enterCredits(self,params):
	global plist_credits
	VS.musicPlayList(plist_credits)

credits_title = """\t=== Vega Strike 0.5.0 ===
\t        ---Credits---"""

credits_text_col1 = """
#ff9999Developers:
* Daniel Horn (Hellcatv)
* Patrick Horn (Ace123)
* Jack Sampson
* Klauss Freire
* Mike Byron
* Ed Sweetman (Safemode)
* Daniel Aleksandrow
* Stephane Vaxelaire
* Alexander Rawass
* Sabarok Aresh
* Scheherazade
* Alan Shieh
* Spiritplumber

#3399ffArtists:
* James Carthew
* Julien Chateau
* Chuck_starchaser
* Coffeebot
* Cub of Judah's Lion
* Howard Day
* Esgaroth
* f109 Vampire
* Fendorin
* Jeff Graw (Hurleybird)
* Peter Griffin
* Fire Hawk
* Eliot Lash (Halleck)
"""



credits_text_col2 = """
#3399ffArtists (cont):
* Oblivion
* Kinnear Penman
* Phlogios
* Pontiac
* Pyramid
* RearAdmiralTolwyn
* Silverain
* Spiner
* Strangelet
* Balint Szilard
* Etheral Walker

#ff99ffMusic:
* Mike Ducharme
* Falik
* Peter Griffin
* M1ck
* Ken Suguro
* Zaydana

#ffcc33Web Site Design:
* Matthew Kruer
* Aazelone Pyoleri (Zaydana)

#99ff99Packaging:
* Vincent Fourmond
* Mike Furr
* Krister Kjellstroem
* Brian Lloyd
"""



credits_text_col3 = """
#99ffffCommunity:
* Martin Baldwin
* Bgaskey
* Dilloh
* DualJoe
* Eagle-1
* Henrik Eklund
* Electrotech
* Charlie G
* Gorruenwe
* loki1950
* Marcel Pare
* Paynalton
* Major A Payne
* Steelrush
* Vortis
* Jason Winzenried (Mamiya Otaru)
* Tobias Wollgarn
* Wolphin
* www2

#66ccffAcknowledgements and thanks:
Sourceforge
Crystal Space
gimp
Blender
NVidia (dds tools)
Open standards
Freedom of speech
User patience
#CCCCff... and any we forgot to mention :)
"""

intro_title = """\t=== Vega Strike 0.5.0 ===
\t ---Intro Monologue---
"""

try:
	intro_file=open("documentation/IntroMonologue.txt","rt")
except:
	intro_file=None
if intro_file:
	intro_text=intro_file.readlines()
	intro_file.close()
else:
	intro_text=["Find the IntroMonologue inside the documentation folder"]

# Create rooms (intro, menu)
#Uncomment the following lines to use intro movies
room_preintro = Base.Room ('XXXPreIntro')
room_intro = Base.Room ('XXXIntro')
room_menu = Base.Room ('XXXMain_Menu')

# Set up menu room
guiroom  = GUI.GUIRoom(room_menu)

# Set up preintro room
class PreIntroRoom(GUI.GUIMovieRoom):
    def onStart(self, video):
        Base.SetDJEnabled(0)
        GUI.GUIMovieRoom.onStart(self, video)
    def onSkip(self, button, params):
        GUI.GUIMovieRoom.onSkip(self, button, params)
        Base.SetDJEnabled(1)

#Uncomment the following lines to use intro movies
preintroroom = PreIntroRoom(room_preintro, 
    ( 'preintro.ogv',
      GUI.GUIRect(0, 0, 1, 1, "normalized")), 
    guiroom)
preintroroom.setAspectRatio(16.0/9.0)

# Set up intro room
class IntroRoom(PreIntroRoom):
    def onStart(self, video):
        Base.SetDJEnabled(0)
        GUI.GUIMovieRoom.onStart(self, video)
    def onSkip(self, button, params):
        PreIntroRoom.onSkip(self, button, params)
        DoStartNewGame(self, params)

#Uncomment the following lines to use intro movies
introroom = IntroRoom(room_intro, 
    ( 'intro.ogv',
      GUI.GUIRect(0, 0, 1, 1, "normalized")), 
    guiroom)
introroom.setAspectRatio(16.0/9.0)



# Create credits room
credits_guiroom = GUI.GUIRoom(Base.Room('XXXCredits'))
GUI.GUIStaticImage(credits_guiroom, 'background', ( 'interfaces/main_menu/credits.spr', GUI.GUIRect(0, 0, 1024, 768, "pixel", (1024,768)) ))

text_loc = GUI.GUIRect(408,8,300,50,"pixel",(1024,768))
GUI.GUIStaticText(credits_guiroom, 'mytitle', credits_title, text_loc, GUI.GUIColor.white())
text_loc = GUI.GUIRect(58,50,300,635,"pixel",(1024,768))
GUI.GUIStaticText(credits_guiroom, 'mytext_col1', credits_text_col1, text_loc, GUI.GUIColor.white())
text_loc = GUI.GUIRect(360,50,300,650,"pixel",(1024,768))
GUI.GUIStaticText(credits_guiroom, 'mytext_col2', credits_text_col2, text_loc, GUI.GUIColor.white())
text_loc = GUI.GUIRect(662,50,362,710,"pixel",(1024,768))
GUI.GUIStaticText(credits_guiroom, 'mytext_col3', credits_text_col3, text_loc, GUI.GUIColor.white())

# Button to go back to the main menu (from the credits)
sprite_loc = GUI.GUIRect(8,697,420,47,"pixel",(1024,768))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/credits_resume_button_pressed.spr', sprite_loc ) }
GUI.GUIRoomButton (credits_guiroom, guiroom, 'XXXMain Menu','Main_Menu',sprite,sprite_loc,clickHandler=enterMainMenu)


# Create intro room
intro_guiroom = GUI.GUIRoom(Base.Room('XXXIntro'))
GUI.GUIStaticImage(intro_guiroom, 'background', ( 'interfaces/main_menu/credits.spr', GUI.GUIRect(0, 0, 1024, 768, "pixel", (1024,768)) ))

text_loc = GUI.GUIRect(408,8,300,50,"pixel",(1024,768))
GUI.GUIStaticText(intro_guiroom, 'mytitle', intro_title, text_loc, GUI.GUIColor.white())
text_loc = GUI.GUIRect(58,50,850,640,"pixel",(1024,768))
# GUI.GUIStaticText(intro_guiroom, 'mytext', intro_text, text_loc, GUI.GUIColor.white())

GUI.GUIStaticText(intro_guiroom,'bg','',text_loc,GUI.GUIColor(0,0,0,.3))
intro_picker=GUI.GUISimpleListPicker(intro_guiroom,'XXXIntroduction','introp',text_loc,
	textcolor=GUI.GUIColor.white(), textbgcolor = GUI.GUIColor.clear(),
	selectedbgcolor=GUI.GUIColor.clear(),selectedcolor=GUI.GUIColor.white())
intro_picker.items=[ GUI.GUISimpleListPicker.listitem(el,el) for el in intro_text ]

screen_loc = GUI.GUIRect(960, 50, 50, 40,"pixel",(1024,768))
scroll_up=GUI.GUIButton(intro_guiroom,'Scroll Up','introu',{'*':None},screen_loc,
		clickHandler=lambda b,p:intro_picker.viewMove(-10),textbgcolor=GUI.GUIColor(0.6,0.3,0.,.5))
scroll_up.setCaption("\n /\\ \n")
screen_loc = GUI.GUIRect(960, 650, 50, 40,"pixel",(1024,768))
scroll_down=GUI.GUIButton(intro_guiroom,'Scroll Down','introu',{'*':None},screen_loc,
		clickHandler=lambda b,p:intro_picker.viewMove(10),textbgcolor=GUI.GUIColor(0.,0.6,0.,.5))
scroll_down.setCaption("\n \\/ \n")

# Button to go back to the main menu (from the credits)
sprite_loc = GUI.GUIRect(8,697,420,47,"pixel",(1024,768))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/credits_resume_button_pressed.spr', sprite_loc ) }
GUI.GUIRoomButton (intro_guiroom, guiroom, 'XXXMain Menu','Main_Menu',sprite,sprite_loc,clickHandler=enterMainMenu)


# Create background
GUI.GUIStaticImage(guiroom, 'background', ( 'interfaces/main_menu/menu.spr', GUI.GUIRect(0, 0, 1024, 768, "pixel", (1024,768)) ))

# Create the Quine 4000 screens
rooms_quine = computer_lib.MakePersonalComputer(room_menu, room_menu,
	0, # do not make links
	0, 0, 0, # no missions, finances or manifest
	1, # do enable load
	0, # but disable save
	1) # and return room map, rather than only root room
rooms_quine['computer'].setMode('load')

# Link 
sprite_loc = GUI.GUIRect(48,300,150,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/load_button_pressed.spr', sprite_loc ) }
GUI.GUIRoomButton(guiroom, rooms_quine['load'], 'XXXLoad Game','Load_Game',sprite,sprite_loc)

# New game
sprite_loc = GUI.GUIRect(48,370,128,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/net_button_pressed.spr', sprite_loc ) }
GUI.GUICompButton(guiroom, 'Network', 'XXXNetwork Game','Network_Game',sprite,sprite_loc)

# Link 
sprite_loc = GUI.GUIRect(48,510,92,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/credits_button_pressed.spr', sprite_loc ) }
GUI.GUIRoomButton(guiroom, credits_guiroom, 'XXXShow Credits','Show_Credits',sprite,sprite_loc,clickHandler=enterCredits)

# Link 
sprite_loc = GUI.GUIRect(48,440,150,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/intro_button_pressed.spr', sprite_loc ) }
GUI.GUIRoomButton(guiroom, intro_guiroom, 'XXXShow Introduction','Show_Introduction',sprite,sprite_loc,clickHandler=enterCredits)

# New game
sprite_loc = GUI.GUIRect(48,224,128,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/new_button_pressed.spr', sprite_loc ) }
btn = GUI.GUIButton(guiroom, 'XXXNew Game','New_Game',sprite,sprite_loc,'enabled',StartNewGame)

# Quit game
sprite_loc = GUI.GUIRect(48,580,58,32,"pixel",(1280,1024))
sprite = {
	'*':None,
	'down' : ( 'interfaces/main_menu/quit_button_pressed.spr', sprite_loc ) }
btn = GUI.GUIButton(guiroom, 'XXXQuit Game','Quit_Game',sprite,sprite_loc,'enabled',QuitGame)

# Draw everything
GUI.GUIRootSingleton.broadcastMessage('draw',None)

# Main menu room environment setup (music and stuff - comment out if you're using an intro movie)
enterMainMenu(None,None)

