/*
 * gamemenu.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "src/vegastrike.h"
#include "src/in_kb.h"
#include "src/in_kb_data.h"
#include "src/in_mouse.h"
#include "src/main_loop.h"
#include "src/universe_util.h"
#include "root_generic/lin_time.h"
#include "gui/modaldialog.h"
#include "gui/eventmanager.h"
#include "gui/newbutton.h"
#include "gui/staticdisplay.h"
#include "gui/textinputdisplay.h"
#include "gui/simplepicker.h"
#include "gui/groupcontrol.h"
#include "gui/scroller.h"
#include "src/gamemenu.h"
#include "src/gfxlib_struct.h"
#include "cmd/music.h"
#include "root_generic/options.h"
#include "root_generic/configxml.h"
#include "src/vs_logging.h"

extern void TerminateCurrentBase(void);
vector<unsigned int> gamemenu_keyboard_queue;

template<>
const GameMenu::WctlTableEntry WctlBase<GameMenu>::WctlCommandTable[] = {
        GameMenu::WctlTableEntry("SinglePlayer", "", &GameMenu::processSinglePlayerButton),
        GameMenu::WctlTableEntry("ShowJoinServer", "", &GameMenu::processMultiPlayerHostButton),
        GameMenu::WctlTableEntry("ShowJoinAccount", "", &GameMenu::processMultiPlayerAcctButton),
        GameMenu::WctlTableEntry("ShowMultiPlayer", "", &GameMenu::processMultiPlayerButton),
        GameMenu::WctlTableEntry("ExitGame", "", &GameMenu::processExitGameButton),
        GameMenu::WctlTableEntry("JoinGame", "", &GameMenu::processJoinGameButton),
        GameMenu::WctlTableEntry("ReturnMainMenu", "", &GameMenu::processMainMenuButton),
        GameMenu::WctlTableEntry("", "", NULL)
};

GameMenu::GameMenu(bool firstTime) :
        m_firstTime(firstTime) {
}

GameMenu::~GameMenu() {
}

void GameMenu::run() {
    WindowController::run();
}

int shiftup(int);

void gamemenu_keyboard_handler(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    //Set modifiers
    unsigned int amods = 0;
    amods |= (mod & (WSK_MOD_LSHIFT | WSK_MOD_RSHIFT)) ? KB_MOD_SHIFT : 0;
    amods |= (mod & (WSK_MOD_LCTRL | WSK_MOD_RCTRL)) ? KB_MOD_CTRL : 0;
    amods |= (mod & (WSK_MOD_LALT | WSK_MOD_RALT)) ? KB_MOD_ALT : 0;
    setActiveModifiers(amods);
    //Queue keystroke
    if (!release) {
        gamemenu_keyboard_queue.push_back(((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT))
                || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT))) ? shiftup(ch) : ch);
    }
}

void gamemenu_draw() {
    UpdateTime();
    Music::MuzakCycle();
    GFXBeginScene();
    globalWindowManager().draw();
    GFXEndScene();
}

//static
void GameMenu::createNetworkControls(GroupControl *serverConnGroup, std::vector<unsigned int> *inputqueue) {
    GFXColor color(1, .5, 0, .1);
    //Account Server button.
    NewButton *joinAcct = new NewButton;
    joinAcct->setRect(Rect(-.50, .7, .37, .09));
    joinAcct->setLabel("Online Account Server");
    joinAcct->setCommand("ShowJoinAccount");

    joinAcct->setColor(GFXColor(color.r, color.g, color.b, .25));
    joinAcct->setTextColor(GUI_OPAQUE_WHITE());
    joinAcct->setDownColor(GFXColor(color.r, color.g, color.b, .5));
    joinAcct->setDownTextColor(GUI_OPAQUE_BLACK());
    joinAcct->setHighlightColor(GFXColor(color.r, color.g, color.b, .4));
    joinAcct->setFont(Font(.07));
    serverConnGroup->addChild(joinAcct);

    //Ship Stats button.
    NewButton *joinServer = new NewButton;
    joinServer->setRect(Rect(.05, .7, .37, .09));
    joinServer->setLabel("Independent Server");
    joinServer->setCommand("ShowJoinServer");
    joinServer->setColor(GFXColor(color.r, color.g, color.b, .25));
    joinServer->setTextColor(GUI_OPAQUE_WHITE());
    joinServer->setDownColor(GFXColor(color.r, color.g, color.b, .5));
    joinServer->setDownTextColor(GUI_OPAQUE_BLACK());
    joinServer->setHighlightColor(GFXColor(color.r, color.g, color.b, .4));
    joinServer->setFont(Font(.07));
    serverConnGroup->addChild(joinServer);

    GroupControl *acctConnGroup = new GroupControl;
    acctConnGroup->setId("MultiPlayerAccountServer");
    acctConnGroup->setHidden(!game_options()->use_account_server);
    serverConnGroup->addChild(acctConnGroup);

    GroupControl *hostConnGroup = new GroupControl;
    hostConnGroup->setId("MultiPlayerHostPort");
    hostConnGroup->setHidden(game_options()->use_account_server);
    serverConnGroup->addChild(hostConnGroup);
    StaticDisplay *mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, .6, 1, .1));
    mplayTitle->setText("Independent Server IP Address:");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("HostTitle");
    hostConnGroup->addChild(mplayTitle);

    //Description box.
    StaticDisplay *serverInputText = new TextInputDisplay(inputqueue, "\x1b\n \t\r*?\\/|:<>\"^");
    serverInputText->setRect(Rect(-.6, .42, 1.2, .15));
    serverInputText->setColor(GFXColor(1, .5, 0, .1));
    serverInputText->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    serverInputText->setFont(Font(.07));
    serverInputText->setMultiLine(false);
    serverInputText->setTextColor(GUI_OPAQUE_WHITE());
    serverInputText->setTextMargins(Size(.02, .01));
    serverInputText->setId("VegaserverHost");
    serverInputText->setText(game_options()->server_ip);
    hostConnGroup->addChild(serverInputText);

    mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, .3, 1, .1));
    mplayTitle->setText("Server Port: (default 6777)");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("PortTitle");
    hostConnGroup->addChild(mplayTitle);

    StaticDisplay *portInputText = new TextInputDisplay(inputqueue, "\x1b\n \t\r*?\\/|:<>\"!@#$%^&*()[]{},.=_-+`~"
                                                                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    portInputText->setRect(Rect(-.6, .12, .4, .15));
    portInputText->setColor(GFXColor(1, .5, 0, .1));
    portInputText->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    portInputText->setFont(Font(.07));
    portInputText->setMultiLine(false);
    portInputText->setTextColor(GUI_OPAQUE_WHITE());
    portInputText->setTextMargins(Size(.02, .01));
    portInputText->setId("VegaserverPort");
    portInputText->setText(game_options()->server_port);
    hostConnGroup->addChild(portInputText);

    mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, .6, 1, .1));
    mplayTitle->setText("Account Server URL:");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("AcctserverTitle");
    acctConnGroup->addChild(mplayTitle);

    StaticDisplay *acctserverInput = new TextInputDisplay(inputqueue, "\x1b\n \t\r*\\|<>\"^");
    acctserverInput->setRect(Rect(-.6, .42, 1.2, .15));
    acctserverInput->setColor(GFXColor(1, .5, 0, .1));
    acctserverInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    acctserverInput->setFont(Font(.07));
    acctserverInput->setMultiLine(false);
    acctserverInput->setTextColor(GUI_OPAQUE_WHITE());
    acctserverInput->setTextMargins(Size(.02, .01));
    acctserverInput->setId("AccountServer");
    acctserverInput->setText(game_options()->account_server_url);
    acctConnGroup->addChild(acctserverInput);

    mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, 0, 1, .1));
    mplayTitle->setText("Callsign:");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("UsernameTitle");
    serverConnGroup->addChild(mplayTitle);

    StaticDisplay *usernameInput = new TextInputDisplay(inputqueue, "\x1b\n\t\r*\\|<>\"^");
    usernameInput->setRect(Rect(-.6, -.18, 1.2, .15));
    usernameInput->setColor(GFXColor(1, .5, 0, .1));
    usernameInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    usernameInput->setFont(Font(.07));
    usernameInput->setMultiLine(false);
    usernameInput->setTextColor(GUI_OPAQUE_WHITE());
    usernameInput->setTextMargins(Size(.02, .01));
    usernameInput->setId("Username");
    if (!game_options()->password.empty()) {
        usernameInput->setText(game_options()->callsign);
    }
    serverConnGroup->addChild(usernameInput);

    mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, -.3, 1, .1));
    mplayTitle->setText("Password: (Server password is usually blank on local games)");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("PasswordTitleHost");
    hostConnGroup->addChild(mplayTitle);

    mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.7, -.3, 1, .1));
    mplayTitle->setText("Password:");
    mplayTitle->setTextColor(GUI_OPAQUE_WHITE());
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("PasswordTitle");
    acctConnGroup->addChild(mplayTitle);

    TextInputDisplay *passwordInput = new TextInputDisplay(inputqueue, "\x1b\n\t\r");
    passwordInput->setPassword('*');
    passwordInput->setRect(Rect(-.6, -.48, 1.2, .15));
    passwordInput->setColor(GFXColor(1, .5, 0, .1));
    passwordInput->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    passwordInput->setFont(Font(.07));
    passwordInput->setMultiLine(false);
    passwordInput->setTextColor(GUI_OPAQUE_WHITE());
    passwordInput->setTextMargins(Size(.02, .01));
    passwordInput->setId("Password");
    passwordInput->setText(game_options()->password);
    serverConnGroup->addChild(passwordInput);

    NewButton *multiStart = new NewButton;
    multiStart->setRect(Rect(-.25, -.65, .5, .15));
    multiStart->setColor(GFXColor(1, .2, 0, .1));
    multiStart->setTextColor(GUI_OPAQUE_WHITE());
    multiStart->setDownColor(GFXColor(1, .2, 0, .4));
    multiStart->setDownTextColor(GFXColor(.2, .2, .2));
    multiStart->setFont(Font(.07, 1));
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

    GameMenu *gm = new GameMenu(firstTime);
    gm->init();
    gm->run();
    if (!error.empty()) {
        gm->window()->findControlById("MainMenu")->setHidden(true);
        gm->window()->findControlById("MultiPlayerMenu")->setHidden(false);
        showAlert(error);
    }
    GFXLoop(gamemenu_draw);
}
}

void GameMenu::init() {
    Window *w = new Window;
    setWindow(w);

    window()->setFullScreen();
    window()->setColor(GUI_CLEAR);
    window()->setTexture("basecomputer.png");

    createControls();
}

void GameMenu::createControls() {
    //Base info title.
    StaticDisplay *baseTitle = new StaticDisplay;
    baseTitle->setRect(Rect(-.96, .83, 1.9, .1));
    baseTitle->setText("Vega Strike menu");
    static GFXColor baseNameColor = vs_config->getColor("base_name_color", GFXColor(.1, .8, .1));
    baseTitle->setTextColor(baseNameColor);
    baseTitle->setColor(GUI_CLEAR);
    baseTitle->setFont(Font(.07, 2));
    baseTitle->setId("GameTitle");
    //Put it on the window.

    GroupControl *mainMenuGroup = new GroupControl;
    mainMenuGroup->setId("MainMenu");
    window()->addControl(mainMenuGroup);

    mainMenuGroup->addChild(baseTitle);

    NewButton *singlePlayer = new NewButton;
    singlePlayer->setRect(Rect(-.75, .2, 1.5, .15));
    singlePlayer->setColor(GFXColor(0, 1, 1, .1));
    singlePlayer->setTextColor(GUI_OPAQUE_WHITE());
    singlePlayer->setDownColor(GFXColor(0, 1, 1, .4));
    singlePlayer->setDownTextColor(GFXColor(.2, .2, .2));
    singlePlayer->setFont(Font(.07, 1));
    singlePlayer->setCommand("SinglePlayer");
    singlePlayer->setLabel("Single Player Game");
    mainMenuGroup->addChild(singlePlayer);

    NewButton *multiPlayer = new NewButton;
    multiPlayer->setRect(Rect(-.75, 0, 1.5, .15));
    multiPlayer->setColor(GFXColor(1, .2, 0, .1));
    multiPlayer->setTextColor(GUI_OPAQUE_WHITE());
    multiPlayer->setDownColor(GFXColor(1, .2, 0, .4));
    multiPlayer->setDownTextColor(GFXColor(.2, .2, .2));
    multiPlayer->setFont(Font(.07, 1));
    multiPlayer->setCommand("ShowMultiPlayer");
    multiPlayer->setLabel("MultiPlayer");
    mainMenuGroup->addChild(multiPlayer);

    NewButton *exitGame = new NewButton;
    exitGame->setRect(Rect(-.75, -.5, 1.5, .15));
    exitGame->setColor(GFXColor(.7, 0, 1, .1));
    exitGame->setTextColor(GUI_OPAQUE_WHITE());
    exitGame->setDownColor(GFXColor(.7, 0, 1, .4));
    exitGame->setDownTextColor(GFXColor(.2, .2, .2));
    exitGame->setFont(Font(.07, 1));
    exitGame->setCommand("ExitGame");
    exitGame->setLabel("Exit Game");
    mainMenuGroup->addChild(exitGame);

    GroupControl *serverConnGroup = new GroupControl;
    serverConnGroup->setId("MultiPlayerMenu");
    serverConnGroup->setHidden(true);
    window()->addControl(serverConnGroup);

    StaticDisplay *mplayTitle = new StaticDisplay;
    mplayTitle->setRect(Rect(-.96, .83, .8, .1));
    mplayTitle->setText("MultiPlayer Settings");
    mplayTitle->setTextColor(baseNameColor);
    mplayTitle->setColor(GUI_CLEAR);
    mplayTitle->setFont(Font(.07, 2));
    mplayTitle->setId("GameTitle");
    //Put it on the window.
    serverConnGroup->addChild(mplayTitle);

    NewButton *returnMainMenu = new NewButton;
    returnMainMenu->setRect(Rect(.7, .81, .25, .1));
    returnMainMenu->setColor(GFXColor(1, .2, 0, .1));
    returnMainMenu->setTextColor(GUI_OPAQUE_WHITE());
    returnMainMenu->setDownColor(GFXColor(1, .2, 0, .4));
    returnMainMenu->setDownTextColor(GFXColor(.2, .2, .2));
    returnMainMenu->setFont(Font(.07, 1));
    returnMainMenu->setCommand("ReturnMainMenu");
    returnMainMenu->setLabel("Done");
    serverConnGroup->addChild(returnMainMenu);

    exitGame = new NewButton;
    exitGame->setRect(Rect(-.95, -.91, .3, .1));
    exitGame->setColor(GFXColor(.7, 0, 1, .1));
    exitGame->setTextColor(GUI_OPAQUE_WHITE());
    exitGame->setDownColor(GFXColor(.7, 0, 1, .4));
    exitGame->setDownTextColor(GFXColor(.2, .2, .2));
    exitGame->setFont(Font(.07, 1));
    exitGame->setCommand("ExitGame");
    exitGame->setLabel("Quit Game");
    serverConnGroup->addChild(exitGame);

    createNetworkControls(serverConnGroup, &gamemenu_keyboard_queue);

    //Make a tab for mode switching...
    //(Add buttons for acctserver/modname) (acctserver mode is default).
    //
    //Add a user/password box (as well as create account button if in acctserver mode).
    //Connect button
    //
    //Scan local network button (I guess...)
    //Host game menu... also might take plenty of work...

    //Single Player button
    //Options button (Requires restart if not done at the beginning... static variables)
    //Options button requires porting vegasettings code to be used inside VS... should be simple to do.  Is it worth it?
    //Network button
    //About
    //Exit game

    //Submenu of single player: (for now, call base computer like it does now))
    //
    //New Game (taken from save/load dialog?)
    //Load Game (save/load dialog without save option?)
    //Simple space fight (equivalent to mission command line option?)

    //Submenu of multiplayer:
    //
    //Account Server. // Mod name.
    //No downloading server lists...
    //User name
    //Password
    //Create account... goes through CGI page.
}

extern void bootstrap_main_loop();
extern void enableNetwork(bool usenet);

bool GameMenu::processSinglePlayerButton(const EventCommandId &command, Control *control) {
    enableNetwork(false);

    restore_main_loop();
    if (m_firstTime) {
        GFXLoop(bootstrap_main_loop);
    }
    window()->close();
    globalWindowManager().shutDown();
    TerminateCurrentBase();
    return true;
}

bool GameMenu::processMultiPlayerHostButton(const EventCommandId &command, Control *control) {
    window()->findControlById("MainMenu")->setHidden(true);
    window()->findControlById("MultiPlayerMenu")->setHidden(false);
    window()->findControlById("MultiPlayerAccountServer")->setHidden(true);
    window()->findControlById("MultiPlayerHostPort")->setHidden(false);
    return true;
}

bool GameMenu::processMultiPlayerAcctButton(const EventCommandId &command, Control *control) {
    window()->findControlById("MainMenu")->setHidden(true);
    window()->findControlById("MultiPlayerMenu")->setHidden(false);
    window()->findControlById("MultiPlayerAccountServer")->setHidden(false);
    window()->findControlById("MultiPlayerHostPort")->setHidden(true);
    return true;
}

bool GameMenu::processMultiPlayerButton(const EventCommandId &command, Control *control) {
    window()->findControlById("MainMenu")->setHidden(true);
    window()->findControlById("MultiPlayerMenu")->setHidden(false);
    return true;
}

bool GameMenu::processMainMenuButton(const EventCommandId &command, Control *control) {
    window()->findControlById("MainMenu")->setHidden(false);
    window()->findControlById("MultiPlayerMenu")->setHidden(true);
    return true;
}

bool GameMenu::processExitGameButton(const EventCommandId &command, Control *control) {
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    winsys_exit(0);
    return true;
}

class ShipSelectorCallback : public ModalDialogCallback {
    NetActionConfirm *nac;
    bool onlyMessage;
public:
    ShipSelectorCallback(NetActionConfirm *nac, bool onlyMessage) :
            nac(nac), onlyMessage(onlyMessage) {
    }

    virtual void modalDialogResult(const std::string &id, int result, WindowController &controller) {
        if (onlyMessage) {
            //The result is slightly different (OK=1 and Cancel=0)
            if (result == YES_ANSWER) {
                result = 0;
            } else {
                result = -1;
            }
        }
        //Ship = 0 or above, Cancel = -1
        nac->finalizeJoinGame(result);
    }

    virtual ~ShipSelectorCallback() {
    }
};

//Create the window and controls for the Options Menu.
void NetActionConfirm::init(void) {
    Window *window = new Window;
    setWindow(window);

    window->setSizeAndCenter(Size(.9, .5));
    window->setTexture("basecomputer.png");
    window->setColor(GFXColor(0, 1, 0, .1));
    window->setOutlineColor(GFXColor(.7, .7, .7));
    window->setOutlineWidth(2.0);
    window->setController(this);

    //Information.
    StaticDisplay *text = new StaticDisplay;
    text->setRect(Rect(-.4, -.15, .8, .3));
    if (netAction == JOINGAME) {
        text->setText("Leaving your current game and joining a new one will lose all progress since your last save.");
    } else if (netAction == SAVEACCT) {
        text->setText("Do you want to save your current account progress?");
    } else if (netAction == DIE) {
        text->setText("Rejoining your current game will lose all progress since your last save.");
    }
    text->setTextColor(GFXColor(.7, 1, .4));
    text->setMultiLine(true);
    text->setColor(GUI_CLEAR);
    text->setFont(Font(.07, 1.25));
    text->setId("Information");
    //Put it on the window.
    window->addControl(text);

    //Save button.
    NewButton *cont = new NewButton;
    cont->setRect(Rect(.05, -.19, .30, .1));
    if (netAction == SAVEACCT) {
        cont->setLabel("Save Account");
        cont->setCommand("Save");
    } else if (netAction == DIE) {
        cont->setLabel("Die");
        cont->setCommand("Load");
    } else if (netAction == JOINGAME) {
        cont->setLabel("Join New Game");
        cont->setCommand("JoinGame");
    }
    cont->setColor(GFXColor(1, .5, 0, .25));
    cont->setTextColor(GUI_OPAQUE_WHITE());
    cont->setDownColor(GFXColor(1, .5, 0, .6));
    cont->setDownTextColor(GUI_OPAQUE_BLACK());
    cont->setHighlightColor(GFXColor(0, 1, 0, .4));
    cont->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(cont);

    //Abort action button
    NewButton *resume = new NewButton;
    resume->setRect(Rect(-.35, -.20, .30, .12));
    resume->setLabel("Cancel");
    resume->setCommand("Window::Close");
    resume->setColor(GFXColor(0, 1, 0, .25));
    resume->setTextColor(GUI_OPAQUE_WHITE());
    resume->setDownColor(GFXColor(0, 1, 0, .6));
    resume->setDownTextColor(GUI_OPAQUE_BLACK());
    resume->setHighlightColor(GFXColor(0, 1, 0, .4));
    resume->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(resume);

    window->setModal(true);
}

//Process a command event from the Options Menu window.
bool NetActionConfirm::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Save") {
        confirmedNetSaveGame();
        window()->close();
    } else if (command == "Load") {
        confirmedNetDie();
        window()->close();
    } else if (command == "JoinGame") {
        confirmedJoinGame();
    } else {
        //Not a command we know about.
        return WindowController::processWindowCommand(command, control);
    }
    return true;
}

// TODO: delete
void GameMenu::readJoinGameControls(Window *window, string &user, string &pass) {
}

bool NetActionConfirm::confirmedNetSaveGame() {
    return false;
}

bool NetActionConfirm::confirmedNetDie() {
    return false;
}

bool NetActionConfirm::confirmedJoinGame() {
    return true;
}

//Caller is responsible for closing the window afterwards. (?)
//static
bool NetActionConfirm::finalizeJoinGame(int launchShip) {
    return true;
}

bool GameMenu::processJoinGameButton(const EventCommandId &command, Control *control) {
    return true;
}

