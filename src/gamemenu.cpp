#include "vegastrike.h"
#include "in_kb.h"
#include "in_kb_data.h"
#include "in_mouse.h"
#include "main_loop.h"
#include "universe_util.h"
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

extern void TerminateCurrentBase( void);
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

//static
void GameMenu::createNetworkControls(GroupControl *serverConnGroup, std::vector <unsigned int> *inputqueue) {
	GroupControl *acctConnGroup = new GroupControl;
	acctConnGroup->setId("MultiPlayerAccountServer");
	acctConnGroup->setHidden(true);
	serverConnGroup->addChild(acctConnGroup);

	GroupControl *hostConnGroup = new GroupControl;
	hostConnGroup->setId("MultiPlayerHostPort");
	hostConnGroup->setHidden(true);
	serverConnGroup->addChild(hostConnGroup);
	StaticDisplay *mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, .6, 1, .1) );
	mplayTitle->setText("Independent Server IP Address:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("HostTitle");
	hostConnGroup->addChild(mplayTitle);
	
	// Description box.
	StaticDisplay* serverInputText = new TextInputDisplay(inputqueue,"\x1b\n \t\r*?\\/|:<>\"^");
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
	
	StaticDisplay* portInputText = new TextInputDisplay(inputqueue,"\x1b\n \t\r*?\\/|:<>\"!@#$%^&*()[]{},.=_-+`~"
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
	
	StaticDisplay* acctserverInput = new TextInputDisplay(inputqueue,"\x1b\n \t\r*\\|<>\"^");
	acctserverInput->setRect( Rect(-.6, .42, 1.2, .15) );
	acctserverInput->setColor( GFXColor(1,.5,0,.1) );
	acctserverInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	acctserverInput->setFont( Font(.07) );
	acctserverInput->setMultiLine(false);
	acctserverInput->setTextColor(GUI_OPAQUE_WHITE());
	acctserverInput->setTextMargins(Size(.02,.01));
	acctserverInput->setId("AccountServer");
	acctserverInput->setText(vs_config->getVariable("network", "account_server_url",
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

	string origpass (vs_config->getVariable("player", "password", ""));
	StaticDisplay* usernameInput = new TextInputDisplay(inputqueue,"\x1b\n\t\r*\\|<>\"^");
	usernameInput->setRect( Rect(-.6, -.18, 1.2, .15) );
	usernameInput->setColor( GFXColor(1,.5,0,.1) );
	usernameInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	usernameInput->setFont( Font(.07) );
	usernameInput->setMultiLine(false);
	usernameInput->setTextColor(GUI_OPAQUE_WHITE());
	usernameInput->setTextMargins(Size(.02,.01));
	usernameInput->setId("Username");
	if (!origpass.empty()) usernameInput->setText(vs_config->getVariable("player", "callsign", ""));
	serverConnGroup->addChild(usernameInput);
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, -.3, 1, .1) );
	mplayTitle->setText("Password: (Server password is usually blank on local games)");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("PasswordTitleHost");
	hostConnGroup->addChild(mplayTitle);
	
	mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.7, -.3, 1, .1) );
	mplayTitle->setText("Password:");
	mplayTitle->setTextColor( GUI_OPAQUE_WHITE() );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("PasswordTitle");
	acctConnGroup->addChild(mplayTitle);
	
	TextInputDisplay* passwordInput = new TextInputDisplay(inputqueue,"\x1b\n\t\r");
	passwordInput->setPassword('*');
	passwordInput->setRect( Rect(-.6, -.48, 1.2, .15) );
	passwordInput->setColor( GFXColor(1,.5,0,.1) );
	passwordInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
	passwordInput->setFont( Font(.07) );
	passwordInput->setMultiLine(false);
	passwordInput->setTextColor(GUI_OPAQUE_WHITE());
	passwordInput->setTextMargins(Size(.02,.01));
	passwordInput->setId("Password");
	passwordInput->setText(origpass);
	serverConnGroup->addChild(passwordInput);
	
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
}

namespace UniverseUtil {
  void startMenuInterface(bool firstTime, string error) {
	winsys_set_keyboard_func(gamemenu_keyboard_handler);
	winsys_set_mouse_func(EventManager::ProcessMouseClick);
	winsys_set_passive_motion_func(EventManager::ProcessMousePassive);
	winsys_set_motion_func(EventManager::ProcessMouseActive);

	GameMenu* gm = new GameMenu(firstTime);
	gm->init();
	gm->run();
	if (!error.empty()) {
		showAlert(error);
	}
	
	GFXLoop(gamemenu_draw);
  }
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
	baseTitle->setText("Vega Strike version 0.5");
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
	singlePlayer->setRect( Rect(-.75, .2, 1.5, .15) );
	singlePlayer->setColor( GFXColor(0,1,1,.1) );
	singlePlayer->setTextColor( GUI_OPAQUE_WHITE() );
	singlePlayer->setDownColor( GFXColor(0,1,1,.4) );
	singlePlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	singlePlayer->setFont( Font(.07, 1) );
	singlePlayer->setCommand("SinglePlayer");
	singlePlayer->setLabel("Single Player");
	mainMenuGroup->addChild(singlePlayer);
	
	NewButton *multiPlayer = new NewButton;
	multiPlayer->setRect( Rect(-.50, .7, .37, .09) );
	multiPlayer->setColor( GFXColor(1,.2,0,.1) );
	multiPlayer->setTextColor( GUI_OPAQUE_WHITE() );
	multiPlayer->setDownColor( GFXColor(1,.2,0,.4) );
	multiPlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	multiPlayer->setFont( Font(.07, 1) );
	multiPlayer->setCommand("AcctButton");
	multiPlayer->setLabel("Online Account Server");
	mainMenuGroup->addChild(multiPlayer);
	
	multiPlayer = new NewButton;
	multiPlayer->setRect( Rect(.05, .7, .37, .09) );
	multiPlayer->setColor( GFXColor(1,.2,0,.1) );
	multiPlayer->setTextColor( GUI_OPAQUE_WHITE() );
	multiPlayer->setDownColor( GFXColor(1,.2,0,.4) );
	multiPlayer->setDownTextColor( GFXColor(.2,.2,.2) );
	multiPlayer->setFont( Font(.07, 1) );
	multiPlayer->setCommand("HostButton");
	multiPlayer->setLabel("Independent Server");
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

	StaticDisplay* mplayTitle = new StaticDisplay;
	mplayTitle->setRect( Rect(-.96, .83, .8, .1) );
	mplayTitle->setText("MultiPlayer Settings");
	mplayTitle->setTextColor( baseNameColor );
	mplayTitle->setColor(GUI_CLEAR);
	mplayTitle->setFont( Font(.07, 2) );
	mplayTitle->setId("GameTitle");
	// Put it on the window.
	serverConnGroup->addChild(mplayTitle);
	
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
	
	createNetworkControls(serverConnGroup, &gamemenu_keyboard_queue);

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
	NetClient::CleanUp();
	enableNetwork(false);
	
	restore_main_loop();
	if (m_firstTime) {
		GFXLoop(bootstrap_main_loop);
	}
	window()->close();
	globalWindowManager().shutDown();
	TerminateCurrentBase();  //BaseInterface::CurrentBase->Terminate();
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


class ShipSelectorCallback: public ModalDialogCallback {
	NetActionConfirm *nac;
public:
    ShipSelectorCallback(NetActionConfirm *nac) : nac(nac) {}
    virtual void modalDialogResult(
            const std::string& id,
            int result,
            WindowController& controller) {
		nac->finalizeJoinGame(result);
    }
	virtual ~ShipSelectorCallback() {}
};

// Create the window and controls for the Options Menu.
void NetActionConfirm::init(void) {
	Window* window = new Window;
	setWindow(window);
	
	window->setSizeAndCenter(Size(.9,.5));
	window->setTexture("basecomputer.png");
	window->setColor( GFXColor(0,1,0,.1) );
	window->setOutlineColor( GFXColor(.7,.7,.7) );
	window->setOutlineWidth(2.0);
	window->setController(this);

	// Information.
	StaticDisplay* text = new StaticDisplay;
	text->setRect( Rect(-.4, -.15, .8, .3) );
	if (netAction==JOINGAME) {
		text->setText("Leaving your current game and joining a new one will lose all progress since your last save.");
	} else if (netAction==SAVEACCT) {
		text->setText("Do you want to save your current account progress?");
	} else if (netAction==DIE) {
		text->setText("Rejoining your current game will lose all progress since your last save.");
	}
	text->setTextColor(GFXColor(.7,1,.4));
	text->setMultiLine(true);
	text->setColor(GUI_CLEAR);
	text->setFont( Font(.07, 1.25) );
	text->setId("Information");
	// Put it on the window.
	window->addControl(text);

	// Save button.
	NewButton* cont = new NewButton;
	cont->setRect( Rect(.05, -.19, .30, .1) );
	if (netAction==SAVEACCT) {
		cont->setLabel("Save Account");
		cont->setCommand("Save");
	} else if (netAction==DIE) {
		cont->setLabel("Die");
		cont->setCommand("Load");
	} else if (netAction==JOINGAME) {
		cont->setLabel("Join New Game");
		cont->setCommand("JoinGame");
	}
	cont->setColor( GFXColor(1,.5,0,.25) );
	cont->setTextColor( GUI_OPAQUE_WHITE() );
	cont->setDownColor( GFXColor(1,.5,0,.6) );
	cont->setDownTextColor( GUI_OPAQUE_BLACK() );
	cont->setHighlightColor( GFXColor(0,1,0,.4) );
	cont->setFont(Font(.08, BOLD_STROKE));
	// Put the button on the window.
	window->addControl(cont);

	// Abort action button
	NewButton* resume = new NewButton;
	resume->setRect( Rect(-.35, -.20, .30, .12) );
	resume->setLabel("Cancel");
	resume->setCommand("Window::Close");
	resume->setColor( GFXColor(0,1,0,.25) );
	resume->setTextColor( GUI_OPAQUE_WHITE() );
	resume->setDownColor( GFXColor(0,1,0,.6) );
	resume->setDownTextColor( GUI_OPAQUE_BLACK() );
	resume->setHighlightColor( GFXColor(0,1,0,.4) );
	resume->setFont(Font(.08, BOLD_STROKE));
	// Put the button on the window.
	window->addControl(resume);

	window->setModal(true);
}

// Process a command event from the Options Menu window.
bool NetActionConfirm::processWindowCommand(const EventCommandId& command, Control* control) {
	if(command == "Save") {
		confirmedNetSaveGame();
		window()->close();
	} else if(command == "Load") {
		confirmedNetDie();
		window()->close();
	} else if(command == "JoinGame") {
		confirmedJoinGame();
	} else {
		// Not a command we know about.
		return WindowController::processWindowCommand(command, control);
	}

	return true;
}

void GameMenu::readJoinGameControls(Window *window, string &user, string &pass) {
	// Magic goes here!
	user = static_cast<TextInputDisplay*>(window->findControlById("Username"))->text();
	string::size_type pos=user.find(' ');
	while (pos!=string::npos) {
		user[pos]='_';
		pos = user.find(' ', pos);
	}
	pass = static_cast<TextInputDisplay*>(window->findControlById("Password"))->text();
	vs_config->setVariable("player","callsign",user);
	vs_config->setVariable("player","password",pass);
	
	if (window->findControlById("MultiPlayerAccountServer")->hidden()) {
		vs_config->setVariable("network","use_account_server","false");
		vs_config->setVariable("network","server_ip",
			static_cast<TextInputDisplay*>(window->findControlById("VegaserverHost"))->text());
		vs_config->setVariable("network","server_port",
			static_cast<TextInputDisplay*>(window->findControlById("VegaserverPort"))->text());
	} else {
		vs_config->setVariable("network","use_account_server","true");
		vs_config->setVariable("network","account_server_url",
			static_cast<TextInputDisplay*>(window->findControlById("AccountServer"))->text());
	}
	
	enableNetwork(true);
	if (Network!=NULL) {
		for (unsigned int i=0;i<_Universe->numPlayers();i++) {
			Network[i].Reinitialize();
		}
	} else {
		Network = new NetClient[_Universe->numPlayers()]; // Hardcode 1 player anyway.
	}
	
}

bool NetActionConfirm::confirmedNetSaveGame() {
	if (!Network) return false;
	Network[player].saveRequest();
	return true;
}

bool NetActionConfirm::confirmedNetDie() {
	if (!Network) return false;
	Network[player].dieRequest();
	return true;
}

bool NetActionConfirm::confirmedJoinGame() {
	string user, pass,  err;
	NetClient::CleanUp();
	GameMenu::readJoinGameControls(m_parent, user, pass);
	
	UniverseUtil::showSplashScreen(string());
	
	if (!Network) return false;
	int numships = Network[player].connectLoad(user, pass, err);
	if (numships) {
		const vector<string> &shipList = Network[player].shipSelections();
		if (shipList.size()>1) {
			UniverseUtil::hideSplashScreen();
			showListQuestion("Select a ship to fly", shipList,
				new ShipSelectorCallback(this), "ShipSelected" );
		} else {
			finalizeJoinGame(0);
		}
	} else {
		UniverseUtil::hideSplashScreen();
		if (window()) window()->close();
		showAlert("Error when joining game!\n\n"+err);
		NetClient::CleanUp();
		return false;
	}
	return true;
}

// Caller is responsible for closing the window afterwards. (?)
//static
bool NetActionConfirm::finalizeJoinGame(int launchShip) {
	if (!UniverseUtil::isSplashScreenShowing()) {
		UniverseUtil::showSplashScreen("");
		UniverseUtil::showSplashMessage("#cc66ffNETWORK: Loading saved game.");
	}

	if (!Network[player].loginSavedGame(launchShip)) {
		showAlert("Error when logging into game with this ship!");
		if (window()) window()->close();
		NetClient::CleanUp();
		return false;
	}
	Cockpit *cp = NULL;
	Unit *playun = NULL;
	if (_Universe) {
		cp = _Universe->AccessCockpit(player);
	}
	if (cp) {
		playun = cp->GetParent();
	}
	if (playun) {
		playun->Kill();
	}
	if (_Universe) {
		_Universe->clearAllSystems();
	}
	
	string err;
	
	restore_main_loop();
//	if (m_firstTime) {
//		GFXLoop(bootstrap_main_loop);
//	} else {
//		UniverseUtil::hideSplashScreen();
//	}
	{
		NetClient *playerClient = &Network[player];
		Window *parentWin = m_parent;
		
		if (window()) window()->close(); // THIS IS DELETED!
		
		if (parentWin) {
			parentWin->close();
		}
		
		globalWindowManager().shutDown();
		TerminateCurrentBase();  //BaseInterface::CurrentBase->Terminate();
		
		playerClient->startGame();
		UniverseUtil::hideSplashScreen();
		
		return true;
	}
}

bool GameMenu::processJoinGameButton(const EventCommandId& command, Control *control) {
	NetActionConfirm *nak = new NetActionConfirm(0, window(), NetActionConfirm::JOINGAME);
//	nak->init();
//	nak->run();
	nak->confirmedJoinGame();

	return true;
}

