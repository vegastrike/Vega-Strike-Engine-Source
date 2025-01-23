/*
 * navcomputer.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAV_COMPUTER_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAV_COMPUTER_H

#include "navscreen.h"
#include "navpath.h"
#include "gui/windowcontroller.h"
#include "gui/simplepicker.h"

class PathManager;
class NavigationSystem;

//The NavComputer class displays an interactive screen that supports a
//number of function.
//Current list:
//Setting descriptor seacrhes
class NavComputer : public WindowController {
public:
//The Computer displays that are possible.
    enum DisplayMode {
        LIST = 0,       //
        EDIT,         //
        DISPLAY_MODE_COUNT, //Number of display modes.
        NULL_DISPLAY = DISPLAY_MODE_COUNT,     //No display.
    };

    enum SelectorMode {
        TARGET = 0,       //
        CRITERIA,         //
        CHAIN,          //
        SELECTOR_MODE_COUNT, //Number of display modes.
        NULL_SELECTOR = SELECTOR_MODE_COUNT,     //No display.
    };

//Set up the window and get everything ready.
    virtual void init(void);

//Start it up!
    virtual void run(void);

//Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
    NavComputer(NavigationSystem *navsystem);
    virtual ~NavComputer(void);

protected:
/***********************************************************************
 *                                                                      *
 *                  Inner class to confirm choices                      *
 *                                                                      *
 ************************************************************************/

    class RenameConfirm : public WindowController {
        NavComputer *m_parent;

    public:
//CONSTRUCTION.
        RenameConfirm(NavComputer *player) : m_parent(player) {
        }

        virtual ~RenameConfirm(void) {
        }

//Set up the window and get everything ready.
        virtual void init(void);

//Process a command event from the window.
        virtual bool processWindowCommand(const EventCommandId &command, Control *control);
    };
    friend class RenameConfirm;

/***********************************************************************
 *                                                                      *
 *                       Sets up the window                             *
 *                                                                      *
 ************************************************************************/

//Gets the color to be used for each control group
    GFXColor getColorForGroup(std::string id);
//Create the controls that will be used for this window.
    void createControls(void);
//Contruct the controls in code.
    void constructControls(void);

//Toggles the navcomputer's visibility
    bool toggleVisibility(const EventCommandId &command, Control *control);

/***********************************************************************
 *                                                                      *
 *                 Functions to switch between modes                    *
 *                                                                      *
 ************************************************************************/

//Change display mode to LIST
    bool changeToListMode(const EventCommandId &command, Control *control);
//Change display mode to EDIT
    bool changeToEditMode(const EventCommandId &command, Control *control);
//Change display mode to TARGET
    bool changeToTargetMode(const EventCommandId &command, Control *control);
//Change display mode to STRING
    bool changeToCriteriaMode(const EventCommandId &command, Control *control);
//Change display mode to CHAIN
    bool changeToChainMode(const EventCommandId &command, Control *control);

//Redo the title string.
    void recalcTitle(void);
//Switch to the set of controls used for the specified mode.
    void switchToMajorControls(DisplayMode mode);
//Switch to the set of controls used for the specified mode.
    void switchToMinorControls(SelectorMode mode);

/************************************************************************
*                                                                      *
*                       Control Functions                              *
*                                                                      *
************************************************************************/

    void loadPathLister();
    void loadChainLister();
    void loadCriteriaLister();
    void loadCriteriaPickerCell(SimplePicker *picker, ValuedPickerCell<CriteriaNode *> *parent, CriteriaNode *node);
    void loadAbsoluteButton();
    bool setCurrentNode(PathNode *source = NULL);
    void updateDescription();
    void updateNodeDescription();

    bool pathListerChangedSelection(const EventCommandId &command, Control *control);
    bool chainListerChangedSelection(const EventCommandId &command, Control *control);

    bool actionAdd(const EventCommandId &command, Control *control);
    bool actionShowPath(const EventCommandId &command, Control *control);
    bool actionRename(const EventCommandId &command, Control *control);
    bool actionRemove(const EventCommandId &command, Control *control);
    bool actionShowAll(const EventCommandId &command, Control *control);
    bool actionShowNone(const EventCommandId &command, Control *control);
    bool actionSource(const EventCommandId &command, Control *control);
    bool actionDestination(const EventCommandId &command, Control *control);
    bool actionApply(const EventCommandId &command, Control *control);
    bool actionCurrent(const EventCommandId &command, Control *control);
    bool actionTarget(const EventCommandId &command, Control *control);
    bool actionAbsolute(const EventCommandId &command, Control *control);
    bool actionAnd(const EventCommandId &command, Control *control);
    bool actionOr(const EventCommandId &command, Control *control);
    bool actionNot(const EventCommandId &command, Control *control);
    bool actionRemoveCriteria(const EventCommandId &command, Control *control);
    bool actionChain(const EventCommandId &command, Control *control);

    void actionRenameConfirmed(std::string name);

/***********************************************************************
 *                                                                      *
 *           Table to store command-function relationships              *
 *                                                                      *
 ************************************************************************/

//Dispatch table declarations.
//This is a member table so the handler functions don't need to be public.
    typedef bool (NavComputer::*WCtlHandler)(const EventCommandId &command, Control *control);
    struct WctlTableEntry {
        EventCommandId command;
        std::string controlId;
        WCtlHandler function;

        WctlTableEntry(const EventCommandId &cmd, const std::string &cid, const WCtlHandler &func) :
                command(cmd), controlId(cid), function(func) {
        }
    };
    static const WctlTableEntry WctlCommandTable[];

/***********************************************************************
 *                                                                      *
 *                        Feature Member variables                      *
 *                                                                      *
 ************************************************************************/

//VARIABLES
    NavigationSystem *navsys;              //The navigation system to interface with
    PathManager *pathman;                  //The path manager to interface with

    NavPath *currentPath;              //The currently selected path
    PathNode *currentNode;              //The currently edited node
    bool criteria;                         //The currently selected node is a criteria node

/***********************************************************************
 *                                                                      *
 *                        Inner Member variables                        *
 *                                                                      *
 ************************************************************************/

//VARIABLES
    bool m_visible;
    vector<DisplayMode> m_displayModes;   //List of displays to provide.
    vector<SelectorMode> m_selectorModes;    //List of selectors to provide.
    DisplayMode m_currentDisplay;          //The current display mode.
    SelectorMode m_currentSelector;           //The current selector mode.
    Control *m_displayModeGroups[DISPLAY_MODE_COUNT]; //Array of GroupControls, one for each display mode.
    Control *m_selectorModeGroups[SELECTOR_MODE_COUNT]; //Array of GroupControls, one for each selector mode.
};

#endif   //VEGA_STRIKE_ENGINE_GFX_NAV_NAV_COMPUTER_H
