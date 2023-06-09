/**
 * basecomputer.h
 *
 * Copyright (C) 2003 Mike Byron
 * Copyright (C) 2019-2020 Stephen G. Tuggy, pyramid3d, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#ifndef VEGA_STRIKE_ENGINE_CMD_BASECOMPUTER_H
#define VEGA_STRIKE_ENGINE_CMD_BASECOMPUTER_H

#include "gui/windowcontroller.h"
#include "cmd/unit_generic.h"
#include "gui/simplepicker.h"

//The BaseComputer class displays an interactive screen that supports a
//number of functions in a base.
//Current list:
//Buying and selling cargo.
//Upgrading and downgrading your ship.
//Replacing your current ship with a new one.
//News articles.
//Mission bulletin board.
//Player info.

class BaseComputer : public WctlBase<BaseComputer> {
    friend class WctlBase<BaseComputer>;
public:

    static int dirty;

//The Computer displays that are possible.
    enum DisplayMode {
        CARGO = 0,            //Buy and sell cargo.
        UPGRADE,            //Buy and sell ship upgrades.
        SHIP_DEALER,        //Replace current ship.
        MISSIONS,           //Show available missions.
        NEWS,               //Show news items.
        INFO,                           //Show basic info.
        LOADSAVE,                   //LOAD SAVE
        NETWORK,                    //Network submenu of Loadsave.
        DISPLAY_MODE_COUNT, //Number of display modes.
        NULL_DISPLAY = DISPLAY_MODE_COUNT,     //No display.
    };

//Set up the window and get everything ready.
    virtual void init(void);

//Start it up!
    virtual void run(void);

//Check if we are dirty.
    virtual void draw(void);

//Process a command event from the window. Handled in parent class's WctlCommandTable.
//virtual bool processWindowCommand(const EventCommandId& command, Control* control);

//CONSTRUCTION
    BaseComputer(Unit *player, Unit *base, const vector<DisplayMode> &modes);
    virtual ~BaseComputer(void);

//These are the transactions that can happen using this object.
//Transactions are operations that modify the player's state.  Reading news isn't
//a transaction.
    enum TransactionType {
        BUY_CARGO,              //Buy item and put in ship.
        SELL_CARGO,             //Sell item to base.
        BUY_UPGRADE,            //Buy an improvement for current ship.
        SELL_UPGRADE,           //Sell an improvement on current ship.
        BUY_SHIP,               //Replace our current ship with a new one.
        ACCEPT_MISSION,         //Accept a mission.
        NULL_TRANSACTION,       //Not initialized yet.
    };

protected:

//Association between CargoColor list, picker, and transaction type.
    struct TransactionList {
        vector<CargoColor> masterList;          //All the items that could be in the picker.
        Picker *picker;                         //The picker loaded with the list.
        TransactionType transaction;        //The kind of transaction these items will generate.
        TransactionList() : picker(NULL), transaction(NULL_TRANSACTION) {
        }
    };

    class LoadSaveQuitConfirm : public WindowController {
        BaseComputer *m_parent;
        std::string type;
        std::string text;
        std::string savefile;
    public:
//CONSTRUCTION.
        LoadSaveQuitConfirm(BaseComputer *player, std::string confirmtype, std::string text)
                : m_parent(player), type(confirmtype), text(text) {
        }

        virtual ~LoadSaveQuitConfirm(void) {
        }

//Set up the window and get everything ready.
        virtual void init(void);

//Process a command event from the window.
        virtual bool processWindowCommand(const EventCommandId &command, Control *control);
    };
    friend class LoadSaveQuitConfirm;
//HANDLERS
//The selection in the News picker changed.
    bool newsPickerChangedSelection(const EventCommandId &command, Control *control);
//The selection in the News picker changed.
    bool loadSavePickerChangedSelection(const EventCommandId &command, Control *control);
//Something in a Picker was selected.
    bool pickerChangedSelection(const EventCommandId &command, Control *control);
//Buy an item from the cargo list.
    bool buyCargo(const EventCommandId &command, Control *control);
//Buy an item (quantity 10) from the cargo list.
    bool buy10Cargo(const EventCommandId &command, Control *control);
//Buy all of an item from the cargo list.
    bool buyAllCargo(const EventCommandId &command, Control *control);
//Sell an item from ship's cargo.
    bool sellCargo(const EventCommandId &command, Control *control);
//Sell an item (quantity 10) from ship's cargo.
    bool sell10Cargo(const EventCommandId &command, Control *control);
//Sell all of an item from ship's cargo.
    bool sellAllCargo(const EventCommandId &command, Control *control);
//Buy a ship upgrade.
    bool buyUpgrade(const EventCommandId &command, Control *control);
//Sell an upgrade on your ship.
    bool sellUpgrade(const EventCommandId &command, Control *control);
//Fix an upgrade on your ship.
    bool fixUpgrade(const EventCommandId &command, Control *control);
//Buy ship from the base.
    bool buyShip(const EventCommandId &command, Control *control);
//Sell ship from your stock
    bool sellShip(const EventCommandId &command, Control *control);
//Accept a mission.
    bool acceptMission(const EventCommandId &command, Control *control);
//Quit Game
    bool actionConfirmedLoadGame();
    bool actionLoadGame(const EventCommandId &command, Control *control);
//Save Game
    bool actionConfirmedSaveGame();
    bool actionSaveGame(const EventCommandId &command, Control *control);
    bool actionNewGame(const EventCommandId &command, Control *control);
//Load Game
    bool actionConfirmedQuitGame();
    bool actionQuitGame(const EventCommandId &command, Control *control);

    bool actionNetSaveGame(const EventCommandId &command, Control *control);
    bool actionNetDie(const EventCommandId &command, Control *control);
    bool actionJoinGame(const EventCommandId &command, Control *control);
    bool actionShowAccountMenu(const EventCommandId &command, Control *control);
    bool actionShowServerMenu(const EventCommandId &command, Control *control);

    bool actionDone(const EventCommandId &command, Control *control);

//Show the player's basic information.
    bool showPlayerInfo(const EventCommandId &command, Control *control);
//Show the stats on the player's current ship.
    bool showShipStats(const EventCommandId &command, Control *control);
//Change display mode to CARGO.
    bool changeToCargoMode(const EventCommandId &command, Control *control);
//Change display mode to UPGRADE.
    bool changeToUpgradeMode(const EventCommandId &command, Control *control);
//Change display mode to SHIP_DEALER.
    bool changeToShipDealerMode(const EventCommandId &command, Control *control);
//Change display mode to NEWS.
    bool changeToNewsMode(const EventCommandId &command, Control *control);
//Change display mode to MISSIONS.
    bool changeToMissionsMode(const EventCommandId &command, Control *control);
//Change display mode to INFO.
    bool changeToInfoMode(const EventCommandId &command, Control *control);
//Change display mode to LOAD/SAVE
    bool changeToLoadSaveMode(const EventCommandId &command, Control *control);
//Change display mode to NETWORK.
    bool changeToNetworkMode(const EventCommandId &command, Control *control);
    void showNetworkStatus(bool show = true);
    bool hideNetworkStatus(const EventCommandId &command, Control *control);

//Redo the title string.
    void recalcTitle(void);

//Refresh both picker lists and the title.
    void refresh(void);

//Return whether or not this transaction is possible for the player now.
    bool isTransactionOK(const Cargo &originalItem, TransactionType transType, int quantity = 1);

//Create whatever cells are needed to add a category to the picker.
    SimplePickerCell *createCategoryCell(SimplePickerCells &cells, const string &category, bool skipFirstCategory);

//Load a picker with a list of items.
    void loadListPicker(TransactionList &list,
            SimplePicker &picker,
            TransactionType transType,
            bool skipFirstCategory = false);

//Scroll to a specific item in a picker, and optionally select it.
//Returns true if the specified item is found.
    bool scrollToItem(Picker *picker, const Cargo &item, bool select, bool skipFirstCategory);

public:
//Update the transaction controls after a transaction.
    void updateTransactionControls(const Cargo &item, bool skipFirstCategory = false);

//Update the controls when the selection for a transaction changes.
    void updateTransactionControlsForSelection(TransactionList *list);
//Load the controls for the MISSIONS display.
    void loadShipDealerControls(void);

    void secondStageJoinGame(void);
    void finalizeJoinGame(int launchShip = 0);
    void finalizeNetSaveGame(void);
protected:

//Hide the controls that commit transactions.
    void hideCommitControls(void);

//Update the commit controls in the Cargo screen, since we have three of them.
    void configureCargoCommitControls(const Cargo &item, TransactionType trans);

//Update the commit controls in the Cargo screen, since we have three of them.
//returns the state of whether the description should have a warning.
    bool configureUpgradeCommitControls(const Cargo &item, TransactionType trans);

//Make sure the info in the transaction lists is gone.
    void resetTransactionLists(void);

//Load the controls for the CARGO display.
    void loadCargoControls(void);

//Buy some items from the Cargo list.  Use -1 for quantity to buy all of the item.
    bool buySelectedCargo(int requestedQuantity);

//The max number of a particular item this player can buy.  Limits by price, cargo space, etc.
    int maxQuantityForPlayer(const Cargo &item, int suggestedQuantity);

//Sell some items from the Cargo list.  Use -1 for quantity to buy all of the item.
    bool sellSelectedCargo(int requestedQuantity);

//Load the controls for the NEWS display.
    void loadNewsControls(void);

    void loadLoadSaveControls(void);

    void loadNetworkControls(void);

//Load the controls for the MISSIONS display.
    void loadMissionsControls(void);

//Load the controls for the UPGRADE display.
    void loadUpgradeControls(void);
    void loadBuyUpgradeControls(void);
    void loadSellUpgradeControls(void);

//Return a pointer to the selected Cargo struct.
    Cargo *selectedItem(void);

//Switch to the set of controls used for the specified mode.
    void switchToControls(DisplayMode mode);

//Create the mode buttons.
    void createModeButtons(void);

//Create the controls that will be used for this window.
    void createControls(void);

//Contruct the controls in code.
    void constructControls(void);

    GFXColor getColorForGroup(std::string id);

//Get a filtered list of items from a unit.
    void loadMasterList(Unit *un,
            const vector<string> &filterthis,
            const vector<string> &invfilterthis,
            bool removezero,
            TransactionList &list);

//Load a master list with missions.
    void loadMissionsMasterList(TransactionList &list);

//VARIABLES
    vector<DisplayMode> m_displayModes;    //List of diaplays to provide.
public:

    UnitContainer m_player;              //Ship info, etc.
    UnitContainer m_base;                //The base we are in.
protected:
    DisplayMode m_currentDisplay;      //The current display mode.
    TransactionList m_transList1;          //The commonly-used list/picker.
    TransactionList m_transList2;          //If there are two pickers, the second one.
    TransactionList *m_selectedList;        //Which transaction list has the selection. NULL = none.
    Control *m_modeGroups[DISPLAY_MODE_COUNT]; //Array of GroupControls, one for each mode.
    bool m_playingMuzak;                //True = We are playing muzak for some mode.

//INTERNAL CLASSES.
    class UpgradeOperation;
    class BuyUpgradeOperation;
    class SellUpgradeOperation;

    friend class UpgradeOperation;
    friend class BuyUpgradeOperation;
    friend class SellUpgradeOperation;
};
bool buyShip(Unit *base, Unit *player, std::string, bool myfleet, bool force_base_inventory, BaseComputer *);
bool sellShip(Unit *base, Unit *player, std::string, BaseComputer *);

#endif   //VEGA_STRIKE_ENGINE_CMD_BASECOMPUTER_H
