/*
 * gamemenu.h
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
#ifndef VEGA_STRIKE_ENGINE_GAME_MENU_H
#define VEGA_STRIKE_ENGINE_GAME_MENU_H

#include "gui/windowcontroller.h"
#include "gui/simplepicker.h"

class GameMenu : public WctlBase<GameMenu> {
    friend class WctlBase<GameMenu>;
public:
    virtual void init(void);
    virtual void run(void);

    GameMenu(bool firsttime);
    virtual ~GameMenu(void);

protected:
    bool m_firstTime;

    bool processSinglePlayerButton(const EventCommandId &command, Control *control);
    bool processMultiPlayerHostButton(const EventCommandId &command, Control *control);
    bool processMultiPlayerAcctButton(const EventCommandId &command, Control *control);
    bool processMultiPlayerButton(const EventCommandId &command, Control *control);
    bool processExitGameButton(const EventCommandId &command, Control *control);
    bool processJoinGameButton(const EventCommandId &command, Control *control);
    bool processMainMenuButton(const EventCommandId &command, Control *control);

    void createControls();

public:
//Helper functions for use in a submenu of BaseComputer.

//Caller is responsible for making a return button, as well as adding the "JoinGame" WctlTableEntry
    static void createNetworkControls(GroupControl *serverConnGroup, std::vector<unsigned int> *keyboard_input_queue);
    static void readJoinGameControls(Window *window, string &user, string &pass);
};

class NetActionConfirm : public WindowController {
public:
    enum ActionMode { SAVEACCT, JOINGAME, DIE };
private:
    int player;
    Window *m_parent;
    string type;
    string text;
    string savefile;
    ActionMode netAction;
public:
//CONSTRUCTION.
    NetActionConfirm(int pnum, Window *w, ActionMode action) :
            player(pnum), m_parent(w), netAction(action) {
    }

    virtual ~NetActionConfirm(void) {
    }

//Set up the window and get everything ready.
    virtual void init(void);

//Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId &command, Control *control);

    virtual bool finalizeJoinGame(int launchShip = 0);
    virtual bool confirmedJoinGame();

    virtual bool confirmedNetDie();
    virtual bool confirmedNetSaveGame();
};

#endif //VEGA_STRIKE_ENGINE_GAME_MENU_H

