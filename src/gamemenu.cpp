#include "vegastrike.h"
#include "in_kb.h"
#include "in_kb_data.h"
#include "in_mouse.h"
#include "main_loop.h"
#include "lin_time.h"
#include "gui/modaldialog.h"
#include "gui/eventmanager.h"
#include "gui/newbutton.h"
#include "gui/staticdisplay.h"
#include "gui/textinputdisplay.h"
#include "gui/simplepicker.h"
#include "gui/groupcontrol.h"
#include "gui/scroller.h"
#include "networking/netclient.h"
#include "gamemenu.h"
#include "gfxlib_struct.h"
#include "cmd/music.h"

vector<unsigned int > gamemenu_keyboard_queue;

template <>
const GameMenu::WctlTableEntry WctlBase<GameMenu>::WctlCommandTable[] = {
	GameMenu::WctlTableEntry ( "SinglePlayer", "", &GameMenu::processSinglePlayerButton ),
	GameMenu::WctlTableEntry ( "HostButton", "", &GameMenu::processMultiPlayerHostButton ),
	GameMenu::WctlTableEntry ( "AcctButton", "", &GameMenu::processMultiPlayerAcctButton ),
	GameMenu::WctlTableEntry ( "ExitGame", "", &GameMenu::processExitGameButton ),
	GameMenu::WctlTableEntry ( "JoinGame", "", &GameMenu::processJoinGameButton ),
	GameMenu::WctlTableEntry ( "ReturnMainMenu", "", &GameMenu::processMainMenuButton ),
    GameMenu::WctlTableEntry ( "", "", NULL )
};

GameMenu::GameMenu(bool firstTime)
    : m_firstTime(firstTime)
{
}

GameMenu::~GameMenu() {
}

void GameMenu::run() {
	// processWindowCommand(networkScreenInitCommand, NULL);
	WindowController::run();
}

int shiftup(int);
void gamemenu_keyboard_handler( unsigned int  ch,unsigned int mod, bool release, int x, int y ) {
	// Set modifiers
	unsigned int amods = 0;
	amods |= (mod&(WSK_MOD_LSHIFT|WSK_MOD_RSHIFT)) ? KB_MOD_SHIFT : 0;
	amods |= (mod&(WSK_MOD_LCTRL |WSK_MOD_RCTRL )) ? KB_MOD_CTRL  : 0;
	amods |= (mod&(WSK_MOD_LALT  |WSK_MOD_RALT  )) ? KB_MOD_ALT   : 0;
	setActiveModifiers(amods);

	// Queue keystroke
	if (!release)
		gamemenu_keyboard_queue.push_back (((WSK_MOD_LSHIFT==(mod&WSK_MOD_LSHIFT))||(WSK_MOD_RSHIFT==(mod&WSK_MOD_RSHIFT)))?shiftup(ch):ch);
}

void gamemenu_draw() {
	UpdateTime();
    Music::MuzakCycle();
	GFXBeginScene();
	globalWindowManager().draw();
	GFXEndScene();
}

void GameMenu::startMenuInterface(bool firstTime) {
	winsys_set_keyboard_func(gamemenu_keyboard_handler);
	winsys_set_mouse_func(EventManager::ProcessMouseClick);
	winsys_set_passive_motion_func(EventManager::ProcessMousePassive);
	winsys_set_motion_func(EventManager::ProcessMouseActive);

	GameMenu* gm = new GameMenu(firstTime);
	gm->init();
	gm->run();
	
	GFXLoop(gamemenu_draw);
}

void GameMenu::init() {
	Window* w = new Window;
	setWindow(w);
	
	window()->setFullScreen();
	window()->setColor(GUI_CLEAR);
	window()->setTexture("basecomputer.png");
	
	createControls();
}


void GameMenu::createControls() {
	// Base info title.
	StaticDisplay* baseTitle = new StaticDisplay;
	baseTitle->setRect( Rect(-.96, .83, 1.9, .1) );
	baseTitle->setText("Vega Strike version 0.5.0");
	static GFXColor baseNameColor=getConfigColor("base_name_color",GFXColor(.1,.8,.1));
	baseTitle->setTextColor(baseNameColor);
	baseTitle->setColor(GUI_CLEAR);
	baseTitle->setFont( Font(.07, 2) );
	baseTitle->setId("GameTitle");
	// Put it on the window.

	GroupControl* mainMenuGroup = new GroupControl;
	mainMenuGroup->setId("MainMenu");
	window()->addControl(mainMenuGroup);

	mainMenuGroup->addChild(baseTitle);
	
	NewButton* singlePlayer = new NewButton;
	singlePlayer->setRect( Rect(-.75, .5, 1.5, .15) );
	singlePlayer->setColor( GFXColor(0,1,1,.1) );
	singlePlayer->setTextColor( GUI_OPAQUE_WHITE() );
	singlePlayer->setDownColor( GFXColor(0,1,1,.4) );
	singlePlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	singlePlayer->setFont( Font(.07, 1) );
	singlePlayer->setCommand("SinglePlayer");
	singlePlayer->setLabel("Single Player");
	mainMenuGroup->addChild(singlePlayer);
	
	NewButton *multiPlayer = new NewButton;
	multiPlayer->setRect( Rect(-.75, .25, 1.5, .15) );
	multiPlayer->setColor( GFXColor(1,.2,0,.1) );
	multiPlayer->setTextColor( GUI_OPAQUE_WHITE() );
	multiPlayer->setDownColor( GFXColor(1,.2,0,.4) );
	multiPlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	multiPlayer->setFont( Font(.07, 1) );
	multiPlayer->setCommand("AcctButton");
	multiPlayer->setLabel("Multi Player: Play Account");
	mainMenuGroup->addChild(multiPlayer);
	
	multiPlayer = new NewButton;
	multiPlayer->setRect( Rect(-.75, 0, 1.5, .15) );
	multiPlayer->setColor( GFXColor(1,.2,0,.1) );
	multiPlayer->setTextColor( GUI_OPAQUE_WHITE() );
	multiPlayer->setDownColor( GFXColor(1,.2,0,.4) );
	multiPlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	multiPlayer->setFont( Font(.07, 1) );
	multiPlayer->setCommand("HostButton");
	multiPlayer->setLabel("Multi Player: Connect to Server");
	mainMenuGroup->addChild(multiPlayer);
	
	NewButton *exitGame = new NewButton;
	exitGame->setRect( Rect(-.75, -.5, 1.5, .15) );
	exitGame->setColor( GFXColor(.7,0,1,.1) );
	exitGame->setTextColor( GUI_OPAQUE_WHITE() );
	exitGame->setDownColor( GFXColor(.7,0,1,.4) );
	exitGame->setDownTextColor( GFXColor(.2,.2,.2) );
	exitGame->setFont( Font(.07, 1) );
	exitGame->setCommand("ExitGame");
	exitGame->setLabel("Exit Game");
	mainMenuGroup->addChild(exitGame);
	

	GroupControl *serverConnGroup = new GroupControl;
	serverConnGroup->setId("MultiPlayerMenu");
	serverConnGroup->setHidden(true);
	window()->addControl(serverConnGroup);

	GroupControl *acctConnGroup = new GroupControl;
	acctConnGroup->setId("MultiPlayerAccountServer");
	acctConnGroup->setHidden(true);
	serverConnGroup->addChild(acctConnGroup);

	GroupControl *hostConnGroup = new GroupControl;
	hostConnGroup->setId("MultiPlayerHostPort");
	hostConnGroup->setHidden(true);
	serverConnGroup->addChild(hostConnGroup);

	StaticDisplay* mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.96, .83, .8, .1) );
	mplayTitle->setText("MultiPlayer Settings");
	mplayTitle->setTextColor( baseNameColor );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("GameTitle");
	// Put it on the window.
	serverConnGroup->addChild(mplayTitle);
	// Scroller for description.
	/*
	Scroller* inputTextScroller = new Scroller;
	inputTextScroller->setRect( Rect(.91, -0.95, .05, .2) );
	inputTextScroller->setColor( UnsaturatedColor(color.r,color.g,color.b,.1) );
	inputTextScroller->setThumbColor( UnsaturatedColor(color.r*.4,color.g*.4,color.b*.4), GUI_OPAQUE_WHITE() );
	inputTextScroller->setButtonColor( UnsaturatedColor(color.r*.4,color.g*.4,color.b*.4) );
	inputTextScroller->setTextColor(GUI_OPAQUE_WHITE());
	inputTextScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	*/
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, .6, 1, .1) );
	mplayTitle->setText("Independent Server IP Address:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("HostTitle");
	hostConnGroup->addChild(mplayTitle);
	
	// Description box.
	StaticDisplay* serverInputText = new TextInputDisplay(&gamemenu_keyboard_queue,"\x1b\n \t\r*?\\/|:<>\"^");
	serverInputText->setRect( Rect(-.6, .42, 1.2, .15) );
	serverInputText->setColor( GFXColor(1,.5,0,.1) );
	serverInputText->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	serverInputText->setFont( Font(.07) );
	serverInputText->setMultiLine(false);
	serverInputText->setTextColor(GUI_OPAQUE_WHITE());
	serverInputText->setTextMargins(Size(.02,.01));
	serverInputText->setId("VegaserverHost");
	serverInputText->setText(vs_config->getVariable("network", "server_ip", ""));
	hostConnGroup->addChild(serverInputText);

	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, .3, 1, .1) );
	mplayTitle->setText("Server Port: (default 6777)");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("PortTitle");
	hostConnGroup->addChild(mplayTitle);
	
	StaticDisplay* portInputText = new TextInputDisplay(&gamemenu_keyboard_queue,"\x1b\n \t\r*?\\/|:<>\"!@#$%^&*()[]{},.=_-+`~"
													"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	portInputText->setRect( Rect(-.6, .12, .4, .15) );
	portInputText->setColor( GFXColor(1,.5,0,.1) );
	portInputText->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	portInputText->setFont( Font(.07) );
	portInputText->setMultiLine(false);
	portInputText->setTextColor(GUI_OPAQUE_WHITE());
	portInputText->setTextMargins(Size(.02,.01));
	portInputText->setId("VegaserverPort");
	portInputText->setText(vs_config->getVariable("network", "server_port", "6777"));
	hostConnGroup->addChild(portInputText);
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, .6, 1, .1) );
	mplayTitle->setText("Account Server URL:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("AcctserverTitle");
	acctConnGroup->addChild(mplayTitle);
	
	StaticDisplay* acctserverInput = new TextInputDisplay(&gamemenu_keyboard_queue,"\x1b\n \t\r*\\|<>\"^");
	acctserverInput->setRect( Rect(-.6, .42, 1.2, .15) );
	acctserverInput->setColor( GFXColor(1,.5,0,.1) );
	acctserverInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	acctserverInput->setFont( Font(.07) );
	acctserverInput->setMultiLine(false);
	acctserverInput->setTextColor(GUI_OPAQUE_WHITE());
	acctserverInput->setTextMargins(Size(.02,.01));
	acctserverInput->setId("AccountServer");
	acctserverInput->setText(vs_config->getVariable("network", "server_ip",
			"http://vegastrike.sourceforge.net/cgi-bin/accountserver.py?"));
	acctConnGroup->addChild(acctserverInput);
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, 0, 1, .1) );
	mplayTitle->setText("Callsign:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("UsernameTitle");
	serverConnGroup->addChild(mplayTitle);
	
	StaticDisplay* usernameInput = new TextInputDisplay(&gamemenu_keyboard_queue,"\x1b\n \t\r*\\|<>\"^");
	usernameInput->setRect( Rect(-.6, -.18, 1.2, .15) );
	usernameInput->setColor( GFXColor(1,.5,0,.1) );
	usernameInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	usernameInput->setFont( Font(.07) );
	usernameInput->setMultiLine(false);
	usernameInput->setTextColor(GUI_OPAQUE_WHITE());
	usernameInput->setTextMargins(Size(.02,.01));
	usernameInput->setId("Username");
	usernameInput->setText(vs_config->getVariable("player", "callsign", ""));
	serverConnGroup->addChild(usernameInput);
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, -.3, 1, .1) );
	mplayTitle->setText("Password:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("PasswordTitle");
	serverConnGroup->addChild(mplayTitle);
	
	TextInputDisplay* passwordInput = new TextInputDisplay(&gamemenu_keyboard_queue,"\x1b\n\t\r");
	passwordInput->setPassword('*');
	passwordInput->setRect( Rect(-.6, -.48, 1.2, .15) );
	passwordInput->setColor( GFXColor(1,.5,0,.1) );
	passwordInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	passwordInput->setFont( Font(.07) );
	passwordInput->setMultiLine(false);
	passwordInput->setTextColor(GUI_OPAQUE_WHITE());
	passwordInput->setTextMargins(Size(.02,.01));
	passwordInput->setId("Password");
	passwordInput->setText(vs_config->getVariable("player", "password", ""));
	serverConnGroup->addChild(passwordInput);
	
	NewButton *returnMainMenu = new NewButton;
	returnMainMenu->setRect( Rect(0, .81, .75, .1) );
	returnMainMenu->setColor( GFXColor(1,.2,0,.1) );
	returnMainMenu->setTextColor( GUI_OPAQUE_WHITE() );
	returnMainMenu->setDownColor( GFXColor(1,.2,0,.4) );
	returnMainMenu->setDownTextColor( GFXColor(.2,.2,.2) );
	returnMainMenu->setFont( Font(.07, 1) );
	returnMainMenu->setCommand("ReturnMainMenu");
	returnMainMenu->setLabel("<-- Back to Main Menu");
	serverConnGroup->addChild(returnMainMenu);
	
	NewButton *multiStart = new NewButton;
	multiStart->setRect( Rect(-.25, -.65, .5, .15) );
	multiStart->setColor( GFXColor(1,.2,0,.1) );
	multiStart->setTextColor( GUI_OPAQUE_WHITE() );
	multiStart->setDownColor( GFXColor(1,.2,0,.4) );
	multiStart->setDownTextColor( GFXColor(.2,.2,.2) );
	multiStart->setFont( Font(.07, 1) );
	multiStart->setCommand("JoinGame");
	multiStart->setLabel("Join Game");
	serverConnGroup->addChild(multiStart);
	
	// Make a tab for mode switching...
	// (Add buttons for acctserver/modname) (acctserver mode is default).
	//
	// Add a user/password box (as well as create account button if in acctserver mode).
	// Connect button
	//
	// Scan local network button (I guess...)
	// Host game menu... also might take plenty of work...
	
	// Single Player button
	// Options button (Requires restart if not done at the beginning... static variables)
		// Options button requires porting vssetup code to be used inside VS... should be simple to do.  Is it worth it?
	// Network button
	// About
	// Exit game
	
	
	// Submenu of single player: (for now, call base computer like it does now))
	// 
	// New Game (taken from save/load dialog?)
	// Load Game (save/load dialog without save option?)
	// Simple space fight (equivalent to mission command line option?)
	
	
	// Submenu of multiplayer:
	// 
	// Account Server. // Mod name.
	// No downloading server lists...
	// User name
	// Password
	// Create account... goes through CGI page.
	
}

extern void bootstrap_main_loop();
extern void enableNetwork(bool usenet);

bool GameMenu::processSinglePlayerButton(const EventCommandId& command, Control *control) {
	enableNetwork(false);
	
	restore_main_loop();
	if (m_firstTime) {
		GFXLoop(bootstrap_main_loop);
	}
	window()->close();
	return true;
}

bool GameMenu::processMultiPlayerHostButton(const EventCommandId& command, Control *control) {
	window()->findControlById("MainMenu")->setHidden(true);
	window()->findControlById("MultiPlayerMenu")->setHidden(false);
	window()->findControlById("MultiPlayerAccountServer")->setHidden(true);
	window()->findControlById("MultiPlayerHostPort")->setHidden(false);
	return true;
}

bool GameMenu::processMultiPlayerAcctButton(const EventCommandId& command, Control *control) {
	window()->findControlById("MainMenu")->setHidden(true);
	window()->findControlById("MultiPlayerMenu")->setHidden(false);
	window()->findControlById("MultiPlayerAccountServer")->setHidden(false);
	window()->findControlById("MultiPlayerHostPort")->setHidden(true);
	return true;
}

bool GameMenu::processMainMenuButton(const EventCommandId& command, Control *control) {
	window()->findControlById("MainMenu")->setHidden(false);
	window()->findControlById("MultiPlayerMenu")->setHidden(true);
	return true;
}

bool GameMenu::processExitGameButton(const EventCommandId& command, Control *control) {
	winsys_exit(0);
	return true;
}

bool GameMenu::processJoinGameButton(const EventCommandId& command, Control *control) {
	// Magic goes here!
	vs_config->setVariable("player","callsign",
		static_cast<TextInputDisplay*>(window()->findControlById("Username"))->text());
	vs_config->setVariable("player","password",
		static_cast<TextInputDisplay*>(window()->findControlById("Password"))->text());
	
	if (window()->findControlById("MultiPlayerAccountServer")->hidden()) {
		vs_config->setVariable("network","use_account_server","false");
		vs_config->setVariable("network","server_ip",
			static_cast<TextInputDisplay*>(window()->findControlById("VegaserverHost"))->text());
		vs_config->setVariable("network","server_port",
			static_cast<TextInputDisplay*>(window()->findControlById("VegaserverPort"))->text());
	} else {
		vs_config->setVariable("network","use_account_server","true");
		vs_config->setVariable("network","server_ip",
			static_cast<TextInputDisplay*>(window()->findControlById("AccountServer"))->text());
	}
	
	enableNetwork(true);
	
	restore_main_loop();
	if (m_firstTime) {
		GFXLoop(bootstrap_main_loop);
	}
	window()->close();
	return true;
}

