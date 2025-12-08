/*
 * basecomputer.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Mike Byron
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "src/vegastrike.h"
#if defined (_WIN32) && !defined (__CYGWIN__) && !defined (__MINGW32__)
                                                                                                                        //For WIN32 debugging.
#include <crtdbg.h>
#endif
#include "cmd/basecomputer.h"
using VSFileSystem::SaveFile;
#include "vegadisk/savegame.h"
#include "src/universe_util.h"
#include "src/save_util.h"
#include <algorithm>                //For std::sort.
#include <set>
#include "root_generic/load_mission.h"
#include "cmd/planet.h"
#include "cmd/unit_util.h"
#include "cmd/music.h"
#include "cmd/unit_const_cache.h"
#include "gui/modaldialog.h"
#include "src/main_loop.h"              //For QuitNow().
#include "root_generic/lin_time.h"
//FIXME mbyron -- Hack instead of reading XML.
#include "gui/newbutton.h"
#include "gui/staticdisplay.h"
#include "gui/textinputdisplay.h"
#include "gui/simplepicker.h"
#include "gui/groupcontrol.h"
#include "gui/scroller.h"
#include "cmd/unit_xml.h"
#include "gfx/sprite.h"
#include "gfx/aux_texture.h"
#include "src/audiolib.h"
#include "src/vs_math.h"
#include "cmd/damageable.h"
#include "src/universe.h"
#include "cmd/mount_size.h"
#include "cmd/weapon_info.h"
#include "src/vs_logging.h"
#include "controls_factory.h"
#include "src/python/infra/get_string.h"
#include "root_generic/configxml.h"
#include "resource/manifest.h"
#include "cmd/reload_utils.h"
#include "components/component_utils.h"

#include <boost/python.hpp>
#include "configuration/configuration.h"

//for directory thing
#if defined (_WIN32) && !defined (__CYGWIN__)
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <cstdlib>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include "src/vega_cast_utils.h"

// Can't declare in header because PyObject is problematic
extern const std::string GetString(const std::string function_name,
                            const std::string module_name,
                            const std::string file_name,
                            PyObject* args);

using namespace XMLSupport; // FIXME -- Shouldn't include an entire namespace, according to Google Style Guide -- stephengtuggy 2021-09-07

//end for directory thing
extern const char *DamagedCategory;

int BaseComputer::dirty = 0;

static GFXColor UnsaturatedColor(float r, float g, float b, float a = 1.0f) {
    GFXColor ret(r, g, b, a);
    return ret;
}

std::string emergency_downgrade_mode;
extern std::string CurrentSaveGameName;

std::vector<std::string> getWeapFilterVec() {
    std::vector<std::string> weapfiltervec;
    ///// FIXME: the following may change in the future if we ever redo the master part list.
    weapfiltervec.push_back("upgrades/Weapon");
    weapfiltervec.push_back("SubUnits");
    weapfiltervec.push_back("upgrades/Ammunition");
    return weapfiltervec;
}

std::vector<std::string> weapfiltervec = getWeapFilterVec();

bool upgradeNotAddedToCargo(std::string category) {
    for (const auto & i : weapfiltervec) {
        if (i.find(category) == 0) {
            return true;
        }
    }
    return false;
}

extern vector<unsigned int> base_keyboard_queue;

// This will break because we have cargo_hold and upgrade_space
std::string getDisplayCategory(const Cargo &cargo) {
    std::string::size_type where = cargo.GetDescription().find("<");
    if (where != string::npos) {
        std::string category = cargo.GetDescription().substr(where + 1);
        where = category.find(">");
        return category.substr(0, where);
    }
    return cargo.GetCategory();
}

//The separator used between categories in a category string.
static const char CATEGORY_SEP = '/';
//Tag that says this is a category not an item.
static const char CATEGORY_TAG = (-1);

//Color of an item that there isn't enough money to buy.
//We read this out of the config file (or use a default).
static bool color_prohibited_upgrade_flag = false;
static bool color_downgrade_or_noncompatible_flag = false;
static bool color_insufficient_space_flag = false;
static bool color_insufficient_money_flag = false;

static GFXColor NO_MONEY_COLOR() {
    static GFXColor NMC = vs_config->getColor("no_money", GFXColor(1, 1, .3, 1));
    return NMC;       //Start out with bogus color.
}

//Make the variable static, so it won't print so many annoying messages!
static GFXColor PROHIBITED_COLOR() {
    static GFXColor PU = vs_config->getColor("prohibited_upgrade", GFXColor(1, .1, 0, 1));
    return PU;
}

static GFXColor DOWNGRADE_OR_NONCOMPAT_COLOR() {
    static GFXColor DNC = vs_config->getColor("downgrade_or_noncompatible", GFXColor(.75, .5, .5, 1));
    return DNC;
}

static GFXColor NO_ROOM_COLOR() {
    static GFXColor NRFU = vs_config->getColor("no_room_for_upgrade", GFXColor(1, 0, 1, 1));
    return NRFU;
}

static GFXColor ITEM_DESTROYED_COLOR() {
    static GFXColor IDC = vs_config->getColor("upgrade_item_destroyed", GFXColor(0.2, 0.2, 0.2, 1));
    return IDC;
}

//Color of the text of a category.
static GFXColor CATEGORY_TEXT_COLOR() {
    static GFXColor CTC = vs_config->getColor("base_category_color", GFXColor(0, .75, 0, 1));
    return CTC;
}

static GFXColor MISSION_COLOR() {
    static GFXColor MiC = vs_config->getColor("base_mission_color", GFXColor(.66, .2, 0, 1));
    return MiC;
}

//Space between mode buttons.
static const float MODE_BUTTON_SPACE = 0.03;

//Default color in CargoColor.
static GFXColor DEFAULT_UPGRADE_COLOR() {
    static GFXColor DuC = vs_config->getColor("base_upgrade_color", GFXColor(1, 1, 1, 1));
    return DuC;
}

//MOUNT ENTRY COLORS
//Mount point that cannot be selected.
static GFXColor MOUNT_POINT_NO_SELECT() {
    return GFXColor(1, .7, .7);
}

//Empty mount point.
static GFXColor MOUNT_POINT_EMPTY() {
    return GFXColor(.2, 1, .2);
}

//Mount point that contains weapon.
static GFXColor MOUNT_POINT_FULL() {
    return GFXColor(1, 1, 0);
}

//Some mission declarations.
//These should probably be in a header file somewhere.
static const char *const MISSION_SCRIPTS_LABEL = "mission_scripts";
static const char *const MISSION_NAMES_LABEL = "mission_names";
static const char *const MISSION_DESC_LABEL = "mission_descriptions";

//Some new declarations.
//These should probably be in a header file somewhere.
static const char *const NEWS_NAME_LABEL = "news";

//Some upgrade declarations.
//These should probably be in a header file somewhere.

extern int GetModeFromName(const char *);  //1=add, 2=mult, 0=neither.
extern Unit &GetUnitMasterPartList();
static const string LOAD_FAILED = "LOAD_FAILED";

//Some ship dealer declarations.
//These should probably be in a header file somewhere.
extern void SwitchUnits(Unit *ol, Unit *nw);
extern void TerminateCurrentBase(void);
extern void CurrentBaseUnitSet(Unit *un);
//For ships stats.
extern string MakeUnitXMLPretty(std::string, Unit *);
//For Options menu.
extern void RespawnNow(Cockpit *cockpit);

//headers for functions used internally
//build the previous description for a ship purchase item
string buildShipDescription(Cargo &item, string &descriptiontexture);
//build the previous description from a cargo purchase item
string buildCargoDescription(const Cargo &item, BaseComputer &computer, float price);
//put in buffer a pretty prepresentation of the POSITIVE float f (ie 4,732.17)
void prettyPrintFloat(char *buffer, float f, int digitsBefore, int digitsAfter, int bufferLen = 128);
string buildUpgradeDescription(Cargo &item, std::map<std::string, std::string> ship_map);
int basecargoassets(Unit *base, string cargoname);

//"Basic Repair" item that is added to Buy UPGRADE mode.
const string BASIC_REPAIR_NAME = "Basic Repair & Refuel";

const GFXColor BASIC_REPAIR_TEXT_COLOR() {
    return GFXColor(0, 1, 1);
}

const string BASIC_REPAIR_DESC =
        "Hire starship mechanics to examine and assess any wear and tear on your craft. They will replace any damaged components on your vessel with the standard components of the vessel you initially purchased.  Further upgrades above and beyond the original will not be replaced free of charge.  The total assessment and repair cost applies if any components are damaged or need servicing (fuel, wear and tear on jump drive, etc...). If such components are damaged you may save money by repairing them on your own. Your vessel will also be refuelled.";
//Repair price is a config variable.

//Info about each mode.
struct ModeInfo {
    string title;
    string button;
    string command;
    string groupId;

    ModeInfo(string t = "", string b = "", string c = "", string g = "") :
            title(t), button(b), command(c), groupId(g) {
    }
};

static const ModeInfo modeInfo[] = {
        ModeInfo("Cargo Dealer  ", "Cargo", "CargoMode", "CargoGroup"),
        ModeInfo("Ship Upgrades  ", "Upgrades", "UpgradeMode", "UpgradeGroup"),
        ModeInfo("New Ships  ", "Ships", "ShipDealerMode", "ShipDealerGroup"),
        ModeInfo("Missions BBS  ", "Missions", "MissionsMode", "MissionsGroup"),
        ModeInfo("GNN News  ", "News", "NewsMode", "NewsGroup"),
        ModeInfo("Info/Stats  ", "Info", "InfoMode", "InfoGroup"),
        ModeInfo("Load / Save ", "LoadSave", "LoadSaveMode", "LoadSaveGroup"),
        ModeInfo("Network ", "Network", "NetworkMode", "NetworkGroup")
};

bool BaseComputer::actionDone(const EventCommandId &command, Control *control) {
    AUDStopAllSounds();
    window()->close();
    return true;
}

//Dispatch table for commands.
//Make an entry here for each command you want to handle.
//WARNING:  The order of this table is important.  There are multiple entries for
//some commands. Basically, you can make an entry for a particular control, and then
//later have an entry with an empty control id to cover the other cases.
template<>
const BaseComputer::WctlTableEntry WctlBase<BaseComputer>::WctlCommandTable[] = {
        BaseComputer::WctlTableEntry("Picker::NewSelection",
                "NewsPicker",
                &BaseComputer::newsPickerChangedSelection),
        BaseComputer::WctlTableEntry("Picker::NewSelection",
                "LoadSavePicker",
                &BaseComputer::loadSavePickerChangedSelection),
        BaseComputer::WctlTableEntry("Picker::NewSelection",
                "",
                &BaseComputer::pickerChangedSelection),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::CARGO].command, "", &BaseComputer::changeToCargoMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::UPGRADE].command, "", &BaseComputer::changeToUpgradeMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::SHIP_DEALER].command,
                "",
                &BaseComputer::changeToShipDealerMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::NEWS].command, "", &BaseComputer::changeToNewsMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::MISSIONS].command, "", &BaseComputer::changeToMissionsMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::INFO].command, "", &BaseComputer::changeToInfoMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::LOADSAVE].command, "", &BaseComputer::changeToLoadSaveMode),
        BaseComputer::WctlTableEntry(modeInfo[BaseComputer::NETWORK].command, "", &BaseComputer::changeToNetworkMode),
        BaseComputer::WctlTableEntry("BuyCargo", "", &BaseComputer::buyCargo),
        BaseComputer::WctlTableEntry("Buy10Cargo", "", &BaseComputer::buy10Cargo),
        BaseComputer::WctlTableEntry("BuyAllCargo", "", &BaseComputer::buyAllCargo),
        BaseComputer::WctlTableEntry("SellCargo", "", &BaseComputer::sellCargo),
        BaseComputer::WctlTableEntry("Sell10Cargo", "", &BaseComputer::sell10Cargo),
        BaseComputer::WctlTableEntry("SellAllCargo", "", &BaseComputer::sellAllCargo),
        BaseComputer::WctlTableEntry("BuyUpgrade", "", &BaseComputer::buyUpgrade),
        BaseComputer::WctlTableEntry("SellUpgrade", "", &BaseComputer::sellUpgrade),
        BaseComputer::WctlTableEntry("FixUpgrade", "", &BaseComputer::fixUpgrade),
        BaseComputer::WctlTableEntry("ReloadUpgrade", "", &BaseComputer::reloadUpgrade),

        BaseComputer::WctlTableEntry("BuyShip", "", &BaseComputer::buyShip),
        BaseComputer::WctlTableEntry("SellShip", "", &BaseComputer::sellShip),
        BaseComputer::WctlTableEntry("AcceptMission", "", &BaseComputer::acceptMission),
        BaseComputer::WctlTableEntry("ShowPlayerInfo", "", &BaseComputer::showPlayerInfo),
        BaseComputer::WctlTableEntry("ShowShipStats", "", &BaseComputer::showShipStats),
        BaseComputer::WctlTableEntry("ShowOptionsMenu", "", &BaseComputer::changeToLoadSaveMode),
        BaseComputer::WctlTableEntry("Quit", "", &BaseComputer::actionQuitGame),
        BaseComputer::WctlTableEntry("Load", "", &BaseComputer::actionLoadGame),
        BaseComputer::WctlTableEntry("New", "", &BaseComputer::actionNewGame),
        BaseComputer::WctlTableEntry("Save", "", &BaseComputer::actionSaveGame),
        BaseComputer::WctlTableEntry("ShowNetworkMenu", "", &BaseComputer::changeToNetworkMode),
        BaseComputer::WctlTableEntry("DoneComputer", "", &BaseComputer::actionDone),

        BaseComputer::WctlTableEntry("", "", nullptr)
};

template<typename T>
inline T mymin(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
inline T mymax(T a, T b) {
    return (a > b) ? a : b;
}

//Take underscores out of a string and capitalize letters after spaces.
static std::string beautify(const string &input) {
    std::string result;

    bool wordStart = true;
    for (std::string::const_iterator i = input.begin(); i != input.end(); i++) {
        if (*i == '_') {
            //Turn this into a space, and make sure next letter is capitalized.
            result += ' ';
            wordStart = true;
        } else if (wordStart) {
            //Start or a word.  Capitalize the character, and turn off start of word.
            result += toupper(*i);
            wordStart = false;
        } else {
            //Normal character in middle of word.
            result += *i;
        }
    }
    return result;
}

//The "used" value of an item.
static double usedValue(double originalValue) {
    return .5 * originalValue;
}


static float basicRepairPrice(void) {
    const float price = configuration().economics.repair_price_flt;
    return price * g_game.difficulty;
}

static double GetOperational(Unit *playerUnit, const Cargo *item) {
    ComponentType type = GetComponentTypeFromName(item->GetName());
    Component* component = playerUnit->GetComponentByType(type);

    if(component) {
        return component->PercentOperational();
    }

    return 0.0;
}

static double RepairPrice(Unit *playerUnit, const Cargo *item) {
    // TODO: * configuration()->general.difficulty;

    // TODO: add this to configuration
    constexpr double kRepairPriceCoefficient = 0.8;

    return kRepairPriceCoefficient * (1-GetOperational(playerUnit, item)) * item->GetPrice();
}


static float SellPrice(Unit *playerUnit, const Cargo *item) {
    return usedValue(item->GetPrice()) - RepairPrice(playerUnit, item);
}


// This generate an upgrade unit from the name and faction.
// An upgrade unit is the unit representation of a specific upgrade.
// e.g. Micro_Driver_ammo__upgrades in units.json
const Unit *getUnitFromUpgradeName(const string &upgradeName, int myUnitFaction = 0);



#define PRETTY_ADD(str, val, digits)                        \
    do {                                                      \
        text += "#n#";                                        \
        text += prefix;                                       \
        text += str;                                          \
        prettyPrintFloat( conversionBuffer, val, 0, digits ); \
        text += conversionBuffer;                             \
    }                                                         \
    while (0)

#define PRETTY_ADDN(str, val, digits)                       \
    do {                                                      \
        text += str;                                          \
        prettyPrintFloat( conversionBuffer, val, 0, digits ); \
        text += conversionBuffer;                             \
    }                                                         \
    while (0)

#define PRETTY_ADDU(str, val, digits, unit)                 \
    do {                                                      \
        text += "#n#";                                        \
        text += prefix;                                       \
        text += str;                                          \
        prettyPrintFloat( conversionBuffer, val, 0, digits ); \
        text += conversionBuffer;                             \
        text += " ";                                          \
        text += unit;                                         \
    }                                                         \
    while (0)

#define MODIFIES(mode, playerUnit, blankUnit, what)                                 \
    (   (((playerUnit) -> what) != 0)                                               \
     && ( (mode != 0) || (((playerUnit) -> what) != ((blankUnit) -> what)) )   )

#define MODIFIES_ALTEMPTY(mode, playerUnit, blankUnit, what, empty)                 \
    (   (((playerUnit) -> what) != (empty))                                         \
     && ( (mode != 0) || (((playerUnit) -> what) != ((blankUnit) -> what)) )   )

//CONSTRUCTOR.
BaseComputer::BaseComputer(Unit *player, Unit *base, const std::vector<DisplayMode> &modes) :
        m_displayModes(modes),
        m_player(player),
        m_base(base),
        m_currentDisplay(NULL_DISPLAY),
        m_selectedList(NULL),
        m_playingMuzak(false) {
    //Make sure we get this color loaded.
    //Initialize mode group controls array.
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        m_modeGroups[i] = NULL;
    }
}

//Destructor.
BaseComputer::~BaseComputer(void) {
    m_player.SetUnit(NULL);
    m_base.SetUnit(NULL);
    //Delete any group controls that the window doesn't "own".
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        if (m_modeGroups[i] != nullptr) {
            delete m_modeGroups[i];
            m_modeGroups[i] = nullptr;
        }
    }
    //If we are playing muzak, stop it.
    if (m_playingMuzak) {
        muzak->Skip();
    }
}

GFXColor BaseComputer::getColorForGroup(std::string id) {
    const bool use_faction_background = configuration().graphics.use_faction_gui_background_color;
    const float faction_color_darkness = configuration().graphics.base_faction_color_darkness_flt;
    if (use_faction_background) {
        int fac = m_base.GetUnit()->faction;
        if (FactionUtil::GetFactionName(fac) == "neutral") {
            fac = FactionUtil::GetFactionIndex(_Universe->getGalaxyProperty(UniverseUtil::getSystemFile(), "faction"));
        }
        const float *stuff = FactionUtil::GetSparkColor(fac);

        return GFXColor(stuff[0] * faction_color_darkness,
                stuff[1] * faction_color_darkness,
                stuff[2] * faction_color_darkness);
    } else {
        if (id == "CargoGroup") {
            return GFXColor(0, 0, faction_color_darkness);
        } else if (id == "NewsGroup") {
            return GFXColor(faction_color_darkness, 0, faction_color_darkness);
        } else if (id == "UpgradeGroup") {
            return GFXColor(0, faction_color_darkness, 0);
        } else if (id == "InfoGroup") {
            return GFXColor(0, faction_color_darkness, faction_color_darkness);
        } else if (id == "MissionsGroup") {
            return GFXColor(faction_color_darkness, 0, 0);
        } else if (id == "ShipDealerGroup") {
            return GFXColor(faction_color_darkness, faction_color_darkness, 0);
        } else if (id == "LoadSaveGroup") {
            return GFXColor(0, faction_color_darkness, faction_color_darkness);
        } else if (id == "NetworkGroup") {
            return GFXColor(0, faction_color_darkness, faction_color_darkness);
        } else {
            return GFXColor(0, 0, 0);
        }
    }
}




//Hack that constructs controls in code.
void BaseComputer::constructControls(void) {
    std::map<std::string, std::map<std::string, std::string>> controls;

    VSFileSystem::VSFile jsonFile;
    VSFileSystem::VSError err = jsonFile.OpenReadOnly("controls.json");
    if (err <= VSFileSystem::Ok) {
        controls = parseControlsJSON(jsonFile);
    } else {
        VS_LOG(error, "controls.json not found");
        return;
    }

    if (m_displayModes.size() != 1 || m_displayModes.at(0) != NETWORK) {
        //Base info title.
        StaticDisplay *baseTitle = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["baseTitle"]));
        window()->addControl(baseTitle);

        //Player info title.
        StaticDisplay *playerTitle = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["playerTitle"]));
        window()->addControl(playerTitle);

        //Options button.
        NewButton *options = vega_dynamic_cast_ptr<NewButton>(getControl(controls["saveLoad"]));
        window()->addControl(options);
    }

    NewButton *done = vega_dynamic_cast_ptr<NewButton>(getControl(controls["done"]));
    window()->addControl(done);

    //Mode button.
    NewButton *mode = vega_dynamic_cast_ptr<NewButton>(getControl(controls["mode"]));
    window()->addControl(mode);
    {
        //CARGO group control.
        GroupControl *cargoGroup = new GroupControl;
        cargoGroup->setId("CargoGroup");
        window()->addControl(cargoGroup);
        GFXColor color = getColorForGroup("CargoGroup");

        //Seller text display.
        StaticDisplay *sellLabel = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["seller"]));
        cargoGroup->addChild(sellLabel);

        //Player inventory text display.
        StaticDisplay *inv = new StaticDisplay;
        *inv = *sellLabel;
        inv->setRect(Rect(.15, .56, .81, .1));
        inv->setText("Inventory");
        cargoGroup->addChild(inv);

        //Total price text display.
        StaticDisplay *totalPrice = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["totalPrice"]));
        cargoGroup->addChild(totalPrice);

        //"Max" text display.
        StaticDisplay *maxForPlayer = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["maxQuantity"]));
        cargoGroup->addChild(maxForPlayer);

        //Scroller for seller.
        Scroller *sellerScroller = vega_dynamic_cast_ptr<Scroller>(getControl(controls["sellerScroller"]));


        //Seller picker.
        SimplePicker *sellpick = vega_dynamic_cast_ptr<SimplePicker>(getControl(controls["sellerPicker"]));
        sellpick->setScroller(sellerScroller);
        cargoGroup->addChild(sellpick);

        cargoGroup->addChild(sellerScroller);                 //Want this "over" the picker.

        //Scroller for inventory.
        Scroller *inventoryScroller = vega_dynamic_cast_ptr<Scroller>(getControl(controls["inventoryScroller"]));

        //Inventory picker.
        SimplePicker *inventoryPick = vega_dynamic_cast_ptr<SimplePicker>(getControl(controls["inventoryPicker"]));
        inventoryPick->setScroller(inventoryScroller);
        cargoGroup->addChild(inventoryPick);

        cargoGroup->addChild(inventoryScroller);            //Want this "over" the picker.

        //Buy button.
        NewButton *buy = vega_dynamic_cast_ptr<NewButton>(getControl(controls["buy"]));
        cargoGroup->addChild(buy);

        //"Buy 10" button.
        NewButton *buy10 = vega_dynamic_cast_ptr<NewButton>(getControl(controls["buy10"]));
        cargoGroup->addChild(buy10);

        //"Buy 1" button.
        NewButton *buy1 = vega_dynamic_cast_ptr<NewButton>(getControl(controls["buy1"]));
        cargoGroup->addChild(buy1);

        //Scroller for description.
        Scroller *descScroller = vega_dynamic_cast_ptr<Scroller>(getControl(controls["descriptionScroller"]));

        //Description box.
        StaticDisplay *ms = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["description"]));
        StaticImageDisplay *picture = vega_dynamic_cast_ptr<StaticImageDisplay>(getControl(controls["picture"]));
        ms->setScroller(descScroller);
        cargoGroup->addChild(ms);

        cargoGroup->addChild(descScroller);           //Want this "over" the description.
        cargoGroup->addChild(picture);
    }
    {
        //UPGRADE group control.
        GroupControl *upgradeGroup = new GroupControl;
        upgradeGroup->setId("UpgradeGroup");
        window()->addControl(upgradeGroup);
        GFXColor color = getColorForGroup("UpgradeGroup");

        //Seller text display.
        StaticDisplay *sellLabel = vega_dynamic_cast_ptr<StaticDisplay>(getControl(controls["sellLabel"]));
        upgradeGroup->addChild(sellLabel);

        //Player inventory text display.
        StaticDisplay *inv = new StaticDisplay;
        *inv = *sellLabel;
        inv->setRect(Rect(.15, .55, .81, .1));
        inv->setText("Improvements To Sell");
        upgradeGroup->addChild(inv);

        //Scroller for seller.
        Scroller *sellerScroller = vega_dynamic_cast_ptr<Scroller>(getControl(controls["sellerScroller"]));

        //Seller picker.
        SimplePicker *sellpick = new SimplePicker;
        sellpick->setRect(Rect(-.96, -.4, .76, .95));
        sellpick->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        sellpick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        sellpick->setTextColor(GUI_OPAQUE_WHITE());
        sellpick->setFont(Font(.07));
        sellpick->setTextMargins(Size(0.02, 0.01));
        sellpick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        sellpick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        sellpick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        sellpick->setId("BaseUpgrades");
        sellpick->setScroller(sellerScroller);
        upgradeGroup->addChild(sellpick);

        upgradeGroup->addChild(sellerScroller);         //Want this "over" the picker.

        //Scroller for inventory.
        Scroller *invScroller = new Scroller;
        invScroller->setRect(Rect(.91, -.4, .05, .95));
        invScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        invScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        invScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        invScroller->setTextColor(GUI_OPAQUE_WHITE());
        invScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Inventory picker.
        SimplePicker *ipick = new SimplePicker;
        ipick->setRect(Rect(.15, -.4, .76, .95));
        ipick->setColor(GFXColor(color.r, color.g, color.b, .1));
        ipick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ipick->setTextColor(GUI_OPAQUE_WHITE());
        ipick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        ipick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        ipick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        ipick->setFont(Font(.07));
        ipick->setTextMargins(Size(0.02, 0.01));
        ipick->setId("PlayerUpgrades");
        ipick->setScroller(invScroller);
        upgradeGroup->addChild(ipick);

        upgradeGroup->addChild(invScroller);          //Want this "over" picker.

        //Buy button.
        NewButton *buy = new NewButton;
        buy->setRect(Rect(-.11, .2, .22, .12));
        buy->setColor(GFXColor(0, 1, 1, .1));
        buy->setTextColor(GUI_OPAQUE_WHITE());
        buy->setDownColor(GFXColor(0, 1, 1, .4));
        buy->setDownTextColor(GFXColor(.2, .2, .2));
        buy->setVariableBorderCycleTime(1.0);
        buy->setBorderColor(GFXColor(.2, .2, .2));
        buy->setEndBorderColor(GFXColor(.4, .4, .4));
        buy->setShadowWidth(2.0);
        buy->setFont(Font(.1, BOLD_STROKE));
        buy->setId("Commit");
        upgradeGroup->addChild(buy);

        //Fix button.
        NewButton *fix = new NewButton;
        fix->setRect(Rect(-.11, .0, .22, .12));
        fix->setColor(GFXColor(0, 1, 1, .1));
        fix->setTextColor(GUI_OPAQUE_WHITE());
        fix->setDownColor(GFXColor(0, 1, 1, .4));
        fix->setDownTextColor(GFXColor(.2, .2, .2));
        fix->setVariableBorderCycleTime(1.0);
        fix->setBorderColor(GFXColor(.2, .2, .2));
        fix->setEndBorderColor(GFXColor(.4, .4, .4));
        fix->setShadowWidth(2.0);
        fix->setFont(Font(.1, BOLD_STROKE));
        fix->setId("CommitFix");
        upgradeGroup->addChild(fix);

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.95, .05, .5));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        StaticImageDisplay *picture = new StaticImageDisplay;
        picture->setRect(Rect(-.96, -.45, .46 * .75, -.47));
        picture->setTexture("blackclear.png");
        picture->setId("DescriptionImage");
        ms->setRect(Rect(-.6, -.95, 1.51, .5));

        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.06));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        upgradeGroup->addChild(ms);

        upgradeGroup->addChild(descScroller);         //Want this "over" description box.
        upgradeGroup->addChild(picture);
    }
    {
        //NEWS group control.
        GroupControl *newsGroup = new GroupControl;
        newsGroup->setId("NewsGroup");
        window()->addControl(newsGroup);
        GFXColor color = getColorForGroup("NewsGroup");

        //Scroller for picker.
        Scroller *pickScroller = new Scroller;
        pickScroller->setRect(Rect(.91, 0, .05, .65));
        pickScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        pickScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        pickScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        pickScroller->setTextColor(GUI_OPAQUE_WHITE());
        pickScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //News picker.
        SimplePicker *pick = new SimplePicker;
        pick->setRect(Rect(-.96, 0, 1.87, .65));
        pick->setColor(GFXColor(color.r, color.g, color.b, .1));
        pick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        pick->setTextColor(GUI_OPAQUE_WHITE());
        pick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        pick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        pick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        pick->setFont(Font(.07));
        pick->setTextMargins(Size(0.02, 0.01));
        pick->setId("NewsPicker");
        pick->setScroller(pickScroller);
        newsGroup->addChild(pick);

        newsGroup->addChild(pickScroller);            //Want scroller "over" picker.

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.95, .05, .90));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(-.96, -.95, 1.87, .90));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.07));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        newsGroup->addChild(ms);

        newsGroup->addChild(descScroller);         //Want scroller "over" description box.
    }
    {
        GroupControl *loadSaveGroup = new GroupControl;
        loadSaveGroup->setId("LoadSaveGroup");
        window()->addControl(loadSaveGroup);
        GFXColor color = getColorForGroup("LoadSaveGroup");
        //Scroller for picker.
        Scroller *pickScroller = new Scroller;
        pickScroller->setRect(Rect(-.20, -.7, .05, 1.4));
        pickScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        pickScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        pickScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        pickScroller->setTextColor(GUI_OPAQUE_WHITE());
        pickScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Save game picker.
        SimplePicker *pick = new SimplePicker;
        pick->setRect(Rect(-.96, -.7, .76, 1.4));
        pick->setColor(GFXColor(color.r, color.g, color.b, .1));
        pick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        pick->setTextColor(GUI_OPAQUE_WHITE());
        pick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        pick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        pick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        pick->setFont(Font(.07));
        pick->setTextMargins(Size(0.02, 0.01));
        pick->setId("LoadSavePicker");
        pick->setScroller(pickScroller);
        loadSaveGroup->addChild(pick);

        loadSaveGroup->addChild(pickScroller);         //Want scroller "over" picker.

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.7, .05, 1.4));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(.15, -.7, .76, 1.4));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.07));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        loadSaveGroup->addChild(ms);

        loadSaveGroup->addChild(descScroller);         //Want scroller "over" description box.

        //Scroller for description.
        Scroller *inputTextScroller = new Scroller;
        inputTextScroller->setRect(Rect(.61, -0.95, .05, .2));
        inputTextScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        inputTextScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4),
                GUI_OPAQUE_WHITE());
        inputTextScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        inputTextScroller->setTextColor(GUI_OPAQUE_WHITE());
        inputTextScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *inputText = new TextInputDisplay(&base_keyboard_queue, "\x1b\n \t\r*?\\/|:<>\"^");
        inputText->setRect(Rect(-.6, -.95, 1.21, .2));
        inputText->setColor(GFXColor(color.r, color.g, color.b, .1));
        inputText->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        inputText->setFont(Font(.07));
        inputText->setMultiLine(true);
        inputText->setTextColor(GUI_OPAQUE_WHITE());
        inputText->setTextMargins(Size(.02, .01));
        inputText->setId("InputText");
        inputText->setScroller(inputTextScroller);
        loadSaveGroup->addChild(inputText);

        loadSaveGroup->addChild(inputTextScroller); //Want scroller "over" description box.
        //Accept button.
        //no save in network mode!
        NewButton *buy10 = new NewButton;
        buy10->setRect(Rect(-.11, 0, .22, .12));
        buy10->setColor(GFXColor(0, 1, 1, .1));
        buy10->setTextColor(GUI_OPAQUE_WHITE());
        buy10->setDownColor(GFXColor(0, 1, 1, .4));
        buy10->setDownTextColor(GFXColor(.2, .2, .2));
        buy10->setVariableBorderCycleTime(1.0);
        buy10->setBorderColor(GFXColor(.2, .2, .2));
        buy10->setEndBorderColor(GFXColor(.4, .4, .4));
        buy10->setShadowWidth(2.0);
        buy10->setFont(Font(.08, BOLD_STROKE));
        buy10->setId("Commit10");
        buy10->setLabel("Save");
        buy10->setCommand("Save");
        loadSaveGroup->addChild(buy10);

        NewButton *accept = new NewButton;
        accept->setRect(Rect(-.11, -.2, .22, .12));
        accept->setColor(GFXColor(0, 1, 1, .1));
        accept->setTextColor(GUI_OPAQUE_WHITE());
        accept->setDownColor(GFXColor(0, 1, 1, .4));
        accept->setDownTextColor(GFXColor(.2, .2, .2));
        accept->setVariableBorderCycleTime(1.0);
        accept->setBorderColor(GFXColor(.2, .2, .2));
        accept->setEndBorderColor(GFXColor(.4, .4, .4));
        accept->setShadowWidth(2.0);
        accept->setFont(Font(.08, BOLD_STROKE));
        accept->setId("Commit");
        accept->setLabel("Load");
        accept->setCommand("Load");
        loadSaveGroup->addChild(accept);

        NewButton *quit = new NewButton;
        quit->setRect(Rect(-.95, -.9, .3, .1));
        quit->setColor(GFXColor(.8, 1, .1, .1));
        quit->setTextColor(GUI_OPAQUE_WHITE());
        quit->setDownColor(GFXColor(.8, 1, .1, .4));
        quit->setDownTextColor(GFXColor(.2, .2, .2));
        quit->setVariableBorderCycleTime(1.0);
        quit->setBorderColor(GFXColor(.5, .2, .2));
        quit->setEndBorderColor(GFXColor(.7, .4, .4));
        quit->setShadowWidth(2.0);
        quit->setFont(Font(.07, BOLD_STROKE));
        quit->setId("CommitAll");
        quit->setLabel("Quit Game");
        quit->setCommand("Quit");
        loadSaveGroup->addChild(quit);

        NewButton *newgame = new NewButton;
        newgame->setRect(Rect(-.11, -.4, .22, .12));
        newgame->setColor(GFXColor(0, 1, 1, .1));
        newgame->setTextColor(GUI_OPAQUE_WHITE());
        newgame->setDownColor(GFXColor(0, 1, 1, .4));
        newgame->setDownTextColor(GFXColor(.2, .2, .2));
        newgame->setVariableBorderCycleTime(1.0);
        newgame->setBorderColor(GFXColor(.2, .2, .2));
        newgame->setEndBorderColor(GFXColor(.4, .4, .4));
        newgame->setShadowWidth(2.0);
        newgame->setFont(Font(.08, BOLD_STROKE));
        newgame->setId("NewGame");
        newgame->setLabel("New");
        newgame->setCommand("New");
        loadSaveGroup->addChild(newgame);
    }
    {
        GroupControl *networkGroup = new GroupControl;
        networkGroup->setId("NetworkGroup");
        window()->addControl(networkGroup);
        GroupControl *netJoinGroup = new GroupControl;
        netJoinGroup->setId("NetworkJoinGroup");
        networkGroup->addChild(netJoinGroup);
        GroupControl *netStatGroup = new GroupControl;
        netStatGroup->setId("NetworkStatGroup");
        netStatGroup->setHidden(true);
        networkGroup->addChild(netStatGroup);

        //GameMenu::createNetworkControls( netJoinGroup, &base_keyboard_queue );

        if (m_displayModes.size() != 1 || m_displayModes.at(0) != NETWORK) {
            NewButton *loadsave = new NewButton;
            loadsave->setRect(Rect(.7, -.9, .25, .1));
            loadsave->setColor(GFXColor(1, .5, .1, .1));
            loadsave->setTextColor(GUI_OPAQUE_WHITE());
            loadsave->setDownColor(GFXColor(1, .5, .1, .4));
            loadsave->setDownTextColor(GFXColor(.2, .2, .2));
            loadsave->setVariableBorderCycleTime(1.0);
            loadsave->setBorderColor(GFXColor(.2, .5, .2));
            loadsave->setEndBorderColor(GFXColor(.4, .7, .4));
            loadsave->setShadowWidth(2.0);
            loadsave->setFont(Font(.07, 1));
            loadsave->setId("CommitAll");
            loadsave->setLabel("Save/Load");
            loadsave->setCommand("ShowOptionsMenu");
            networkGroup->addChild(loadsave);
        }
        if ((m_displayModes.size() == 1 && m_displayModes.at(0) == NETWORK)) {
            NewButton *quit = new NewButton;
            quit->setRect(Rect(-.95, -.9, .3, .1));
            quit->setColor(GFXColor(.8, 1, .1, .1));
            quit->setTextColor(GUI_OPAQUE_WHITE());
            quit->setDownColor(GFXColor(.8, 1, .1, .4));
            quit->setDownTextColor(GFXColor(.2, .2, .2));
            quit->setVariableBorderCycleTime(1.0);
            quit->setBorderColor(GFXColor(.5, .2, .2));
            quit->setEndBorderColor(GFXColor(.7, .4, .4));
            quit->setShadowWidth(2.0);
            quit->setFont(Font(.07, BOLD_STROKE));
            quit->setId("CommitAll");
            quit->setLabel("Quit Game");
            quit->setCommand("Quit");
            networkGroup->addChild(quit);
        }
    }
    {
        //MISSIONS group control.
        GroupControl *missionsGroup = new GroupControl;
        missionsGroup->setId("MissionsGroup");
        window()->addControl(missionsGroup);
        GFXColor color = getColorForGroup("MissionsGroup");

        //Scroller for picker.
        Scroller *pickScroller = new Scroller;
        pickScroller->setRect(Rect(-.20, -.8, .05, 1.45));
        pickScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        pickScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        pickScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        pickScroller->setTextColor(GUI_OPAQUE_WHITE());
        pickScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Picker.
        SimplePicker *pick = new SimplePicker;
        pick->setRect(Rect(-.96, -.8, .76, 1.45));
        pick->setColor(GFXColor(color.r, color.g, color.b, .1));
        pick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        pick->setTextColor(GUI_OPAQUE_WHITE());
        pick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        pick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        pick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        pick->setFont(Font(.07));
        pick->setTextMargins(Size(0.02, 0.01));
        pick->setId("Missions");
        pick->setScroller(pickScroller);
        missionsGroup->addChild(pick);

        missionsGroup->addChild(pickScroller);         //Want scroller "over" picker.

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.8, .05, 1.45));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(-.10, -.8, 1.01, 1.45));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.06));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        missionsGroup->addChild(ms);

        missionsGroup->addChild(descScroller);         //Want scroller "over" description box.

        //Accept button.
        NewButton *accept = new NewButton;
        accept->setRect(Rect(-.23, -.95, .22, .11));
        accept->setColor(GFXColor(0, 1, 1, .1));
        accept->setTextColor(GUI_OPAQUE_WHITE());
        accept->setDownColor(GFXColor(0, 1, 1, .4));
        accept->setDownTextColor(GFXColor(.2, .2, .2));
        accept->setVariableBorderCycleTime(1.0);
        accept->setBorderColor(GFXColor(.2, .2, .2));
        accept->setEndBorderColor(GFXColor(.4, .4, .4));
        accept->setShadowWidth(2.0);
        accept->setFont(Font(.08, BOLD_STROKE));
        accept->setId("Commit");
        missionsGroup->addChild(accept);
    }
    {
        //SHIP_DEALER group control.
        GroupControl *shipDealerGroup = new GroupControl;
        shipDealerGroup->setId("ShipDealerGroup");
        window()->addControl(shipDealerGroup);
        GFXColor color = getColorForGroup("ShipDealerGroup");

        //Scroller for picker.
        Scroller *pickScroller = new Scroller;
        pickScroller->setRect(Rect(-.20, -.8, .05, 1.45));
        pickScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        pickScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        pickScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        pickScroller->setTextColor(GUI_OPAQUE_WHITE());
        pickScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Picker.
        SimplePicker *pick = new SimplePicker;
        pick->setRect(Rect(-.96, -.8, .76, 1.45));
        pick->setColor(GFXColor(color.r, color.g, color.b, .1));
        pick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        pick->setTextColor(GUI_OPAQUE_WHITE());
        pick->setSelectionColor(UnsaturatedColor(0, .6, 0, .8));
        pick->setHighlightColor(UnsaturatedColor(0, .6, 0, .35));
        pick->setHighlightTextColor(GUI_OPAQUE_WHITE());
        pick->setFont(Font(.07));
        pick->setTextMargins(Size(0.02, 0.01));
        pick->setId("Ships");
        pick->setScroller(pickScroller);
        shipDealerGroup->addChild(pick);

        shipDealerGroup->addChild(pickScroller);              //Want scroller to be "over" picker.

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.5, .05, 1.15));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        StaticImageDisplay *picture = new StaticImageDisplay;
        picture->setRect(Rect(-.10, -.51, .48 * .75, -.48));
        picture->setTexture("blackclear.png");
        picture->setId("DescriptionImage");
        ms->setRect(Rect(-.10, -.5, 1.01, 1.15));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.06));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        shipDealerGroup->addChild(ms);

        shipDealerGroup->addChild(descScroller);         //Want scroller "over" description box.
        shipDealerGroup->addChild(picture);
        //Buy button.
        NewButton *buy = new NewButton;
        buy->setRect(Rect(-.53, -.95, .22, .11));
        buy->setColor(GFXColor(0, 1, 1, .1));
        buy->setTextColor(GUI_OPAQUE_WHITE());
        buy->setDownColor(GFXColor(0, 1, 1, .4));
        buy->setDownTextColor(GFXColor(.2, .2, .2));
        buy->setVariableBorderCycleTime(1.0);
        buy->setBorderColor(GFXColor(.2, .2, .2));
        buy->setEndBorderColor(GFXColor(.4, .4, .4));
        buy->setShadowWidth(2.0);
        buy->setFont(Font(.08, BOLD_STROKE));
        buy->setId("Commit");
        shipDealerGroup->addChild(buy);
        NewButton *sell = new NewButton;
        sell->setRect(Rect(-.23, -.95, .22, .11));
        sell->setColor(GFXColor(0, 1, 1, .1));
        sell->setTextColor(GUI_OPAQUE_WHITE());
        sell->setDownColor(GFXColor(0, 1, 1, .4));
        sell->setDownTextColor(GFXColor(.2, .2, .2));
        sell->setVariableBorderCycleTime(1.0);
        sell->setBorderColor(GFXColor(.2, .2, .2));
        sell->setEndBorderColor(GFXColor(.4, .4, .4));
        sell->setShadowWidth(2.0);
        sell->setFont(Font(.08, BOLD_STROKE));
        sell->setId("Commit10");
        shipDealerGroup->addChild(sell);
    }
    {
        //INFO group control.
        GroupControl *infoGroup = new GroupControl;
        infoGroup->setId("InfoGroup");
        window()->addControl(infoGroup);
        GFXColor color = getColorForGroup("InfoGroup");

        //Player Info button.
        NewButton *playerInfo = new NewButton;
        playerInfo->setRect(Rect(-.40, .52, .27, .09));
        playerInfo->setLabel("Player Info");
        static GFXColor pinfo_col = vs_config->getColor("player_info", GFXColor(0, .4, 0));
        playerInfo->setCommand("ShowPlayerInfo");

        playerInfo->setColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .25));
        playerInfo->setTextColor(GUI_OPAQUE_WHITE());
        playerInfo->setDownColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .5));
        playerInfo->setDownTextColor(GUI_OPAQUE_BLACK());
        playerInfo->setHighlightColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .4));
        playerInfo->setFont(Font(.07));
        infoGroup->addChild(playerInfo);

        //Ship Stats button.
        NewButton *shipStats = new NewButton;
        shipStats->setRect(Rect(-.05, .52, .27, .09));
        shipStats->setLabel("Ship Stats");
        shipStats->setCommand("ShowShipStats");
        shipStats->setColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .25));
        shipStats->setTextColor(GUI_OPAQUE_WHITE());
        shipStats->setDownColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .5));
        shipStats->setDownTextColor(GUI_OPAQUE_BLACK());
        shipStats->setHighlightColor(GFXColor(pinfo_col.r, pinfo_col.g, pinfo_col.b, .4));
        shipStats->setFont(Font(.07));
        infoGroup->addChild(shipStats);

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.95, .05, 1.4));
        descScroller->setColor(UnsaturatedColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(UnsaturatedColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(-.96, -.95, 1.87, 1.4));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.07));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        infoGroup->addChild(ms);

        infoGroup->addChild(descScroller);
    }
}

//Create the controls that will be used for this window.
void BaseComputer::createControls(void) {
    //Set up the window.
    window()->setFullScreen();
    window()->setColor(GUI_CLEAR);
    window()->setTexture("basecomputer.png");

    //Put all the controls in the window.
    constructControls();
    //Take the mode group controls out of the window.
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        Control *group = window()->findControlById(modeInfo[i].groupId);
        if (group) {
            window()->removeControlFromWindow(group);
            m_modeGroups[i] = group;
        }
    }
    createModeButtons();
}

//Create the mode buttons.
void BaseComputer::createModeButtons(void) {
    NewButton *originalButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("ModeButton"));
    assert(originalButton != NULL);
    if (m_displayModes.size() > 1) {
        //Create a button for each display mode, copying the original button.
        Rect rect = originalButton->rect();
        for (const auto mode : m_displayModes) {
            NewButton *newButton = new NewButton(*originalButton);
            newButton->setRect(rect);
            newButton->setLabel(modeInfo[mode].button);
            newButton->setCommand(modeInfo[mode].command);
            window()->addControl(newButton);
            rect.origin.x += rect.size.width + MODE_BUTTON_SPACE;
        }
    }
    //Make sure this original doesn't show.
    originalButton->setHidden(true);
}

//Make sure the info in the transaction lists is gone.
void BaseComputer::resetTransactionLists(void) {
    m_transList1 = TransactionList();
    m_transList2 = TransactionList();
}

//Switch to the set of controls used for the specified mode.
void BaseComputer::switchToControls(DisplayMode mode) {
    if (m_currentDisplay != mode) {
        assert(m_modeGroups[mode] != NULL);         //We should have controls for this mode.
        if (mode == CARGO) {
            window()->setTexture("basecomputer_cargo.png");
        }
        if (mode == MISSIONS) {
            window()->setTexture("basecomputer_missions.png");
        }
        if (mode == UPGRADE) {
            window()->setTexture("basecomputer_upgrade.png");
        }
        if (mode == SHIP_DEALER) {
            window()->setTexture("basecomputer_ship_dealer.png");
        }
        if (mode == NEWS) {
            window()->setTexture("basecomputer_news.png");
        }
        if (mode == INFO) {
            window()->setTexture("basecomputer_info.png");
        }
        if (mode == LOADSAVE) {
            window()->setTexture("basecomputer_loadsave.png");
        }
        if (mode == NETWORK) {
            window()->setTexture("basecomputer_network.png");
        }
        if (m_currentDisplay != NULL_DISPLAY) {
            //Get the old controls out of the window.
            Control *oldControls = window()->findControlById(modeInfo[m_currentDisplay].groupId);
            if (oldControls) {
                window()->removeControlFromWindow(oldControls);
            }
            //We put this back in our table so that we "own" the controls.
            m_modeGroups[m_currentDisplay] = oldControls;
            //Stop playing muzak for the old mode.
            if (m_playingMuzak) {
                muzak->Skip();
                m_playingMuzak = false;
            }
        }
        m_currentDisplay = mode;

        window()->addControl(m_modeGroups[mode]);
        //Take this group out of our table because we don't own it anymore.
        m_modeGroups[mode] = NULL;
    }
}

//Change controls to CARGO mode.
bool BaseComputer::changeToCargoMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != CARGO) {
        switchToControls(CARGO);
    }
    loadCargoControls();
    updateTransactionControlsForSelection(NULL);
    return true;
}

bool BaseComputer::changeToLoadSaveMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != LOADSAVE) {
        switchToControls(LOADSAVE);
    }
    loadLoadSaveControls();
    return true;
}

bool BaseComputer::changeToNetworkMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != NETWORK) {
        switchToControls(NETWORK);
    }
    return true;
}


//Set up the window and get everything ready.
void BaseComputer::init(void) {
    //Create a new window.
    Window *w = new Window;
    setWindow(w);

    //Read in the controls for all the modes.
    createControls();
}

//Open the window, etc.
void BaseComputer::run(void) {
    //Simulate clicking the leftmost mode button.
    //We don't actually use the button because there isn't a button if there's only one mode.
    processWindowCommand(modeInfo[m_displayModes[0]].command, NULL);

    WindowController::run();
}

//Redo the title strings for the display.
void BaseComputer::recalcTitle() {
    if (m_displayModes.size() == 1) {
        return;
    }

    //Generic base title for the display.
    string baseTitle = modeInfo[m_currentDisplay].title;

    //Base name.
    Unit *baseUnit = m_base.GetUnit();
    string baseName;
    if (baseUnit) {
        if (baseUnit->getUnitType() == Vega_UnitType::planet) {
            const string temp = vega_dynamic_cast_ptr<Planet>(baseUnit)->getHumanReadablePlanetType() + " Planet";
            // think "<planet type> <name of planet>"
            baseName = temp + " " + baseUnit->name;
        } else {
            // as above, but e.g. mining bases have 'mining_base' in baseUnit->name
            // so we need to come up with something a little bit better
            baseName = baseUnit->name + " " + baseUnit->getFullname();
        }
    }
    // at this point, baseName will be e.g. "Agricultural planet Helen" or "mining_base Achilles"
    baseTitle += emergency_downgrade_mode;
    const bool include_base_name = configuration().graphics.bases.include_base_name_on_dock;
    if (include_base_name) {
        baseTitle += baseName;

        //Faction name for base.
        const std::string baseFaction = FactionUtil::GetFactionName(baseUnit->faction);
        if (!baseFaction.empty()) {
            baseTitle += " [" + baseFaction + ']';
        }
    }
    //Set the string in the base title control.
    StaticDisplay *baseTitleDisplay = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("BaseInfoTitle"));
    baseTitleDisplay->setText(baseTitle);

    //Generic player title for display
    std::string playerTitle = "";

    const bool showStardate = configuration().graphics.show_stardate;

    //Credits the player has.
    const std::string stardateString = _Universe->current_stardate.GetFullTrekDate();
    const char *stardate = stardateString.c_str();
    Unit *playerUnit = m_player.GetUnit();
    switch (m_currentDisplay) {
        default:
            if (showStardate) {
                playerTitle = (boost::format("Stardate: %1$s      Credits: %2$.2f")
                        % stardate % ComponentsManager::credits.Value())
                        .str();
            } else {
                playerTitle = (boost::format("Credits: %1$.2f") % ComponentsManager::credits.Value()).str();
            }
            break;
        case MISSIONS: {
            const int count = guiMax(0, int(active_missions.size())-1);
            if (showStardate) {
                playerTitle = (boost::format("Stardate: %1$s      Credits: %2$.2f      Active missions: %3$d")
                        % stardate % ComponentsManager::credits.Value() % count)
                        .str();
            } else {
                playerTitle = (boost::format("Credits: %1$.2f      Active missions: %2$d")
                        % ComponentsManager::credits.Value() % count)
                        .str();
            }
            break;
        }
        case UPGRADE:
            playerTitle = playerUnit->GetTitle(false, showStardate, stardateString);
            break;
        case CARGO: {
            playerTitle = playerUnit->GetTitle(true, showStardate, stardateString);
            break;
        }
    }
    //Set the string in the player title control.
    StaticDisplay *playerTitleDisplay = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("PlayerInfoTitle"));
    playerTitleDisplay->setText(playerTitle);
}

//Scroll to a specific item in a picker, and optionally select it.
//Returns true if we selected an item.
bool BaseComputer::scrollToItem(Picker *picker, const Cargo &item, bool select, bool skipFirstCategory) {
    PickerCells *cells = picker->cells();
    if (!cells) {
        return false;
    }
    PickerCell *categoryCell = NULL;     //Need this if we can't find the item.

    //Walk through the category list(s).
    std::string category = getDisplayCategory(item);
    if (category.size() > 0) {
        //Make sure we have a category.
        string::size_type categoryStart = 0;
        if (skipFirstCategory) {
            //We need to skip the first category in the string.
            //Generally need to do this when there's a category level that's not in the UI, like
            //"upgrades" in the Upgrade UI.
            categoryStart = category.find(CATEGORY_SEP, 0);
            if (categoryStart != string::npos) {
                categoryStart++;
            }
        }
        while (true) {
            //See if we have multiple categories left.
            const string::size_type categoryEnd = category.find(CATEGORY_SEP, categoryStart);
            const string currentCategory = category.substr(categoryStart, categoryEnd - categoryStart);

            PickerCell *cell = cells->cellWithId(currentCategory);
            if (!cell || !cell->children()) {
                //The category has no children, or we have no matching category.  We are done.
                //WARNING:  We return from here!
                picker->scrollToCell(categoryCell);
                return false;
            }
            //Found the category in the right place.
            categoryCell = cell;
            categoryCell->setHideChildren(false);
            picker->setMustRecalc();
            cells = categoryCell->children();
            //Next piece of the category string.
            if (categoryEnd == string::npos) {
                break;
            }
            categoryStart = categoryEnd + 1;
        }
    }
    //We have the parent category, now we need the child itself.
    assert(cells != NULL);
    PickerCell *cell = cells->cellWithId(item.GetName());
    picker->setMustRecalc();
    if (!cell) {
        //Item is not here.
        int count = cells->count();
        if (count == 0) {
            //The category is empty.
            picker->scrollToCell(categoryCell);
            return false;
        }
        //Try to find the place where the item used to be.
        //We assume here that the list is sorted by id, which is the
        //original, un-beautified name.
        int i = 0;
        for (; i < count; i++) {
            if (item.GetName() < cells->cellAt(i)->id()) {
                break;
            }
        }
        if (i == count) {
            i--;
        }
        cell = cells->cellAt(i);
        assert(cell != NULL);
        //Drop through to get cell handled.
    }
    if (select) {
        picker->selectCell(cell, true);
        //This may not be a selectable cell.
        return picker->selectedCell() != NULL;
    } else {
        //Make sure we scroll it into view.
        //Since it's not selected, we assume it's in the "other" list and scroll
        //it into the middle.
        picker->scrollToCell(cell, true);
        return false;
    }
    //Shouldn't ever get here.
    assert(false);
    return false;
}

//Hide the controls that commit transactions.
void BaseComputer::hideCommitControls(void) {
    //The three buy/sell buttons.
    NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
    commitButton->setHidden(true);
    NewButton *commit10Button = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit10"), true);
    if (commit10Button != nullptr) {
        commit10Button->setHidden(true);
    }
    NewButton *commitAllButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitAll"), true);
    if (commitAllButton != nullptr) {
        commitAllButton->setHidden(true);
    }
    NewButton *commitFixButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitFix"), true);
    if (commitFixButton != nullptr) {
        commitFixButton->setHidden(true);
    }
    //The price and "max" displays.
    StaticDisplay *totalPrice = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("TotalPrice"), true);
    if (totalPrice != nullptr) {
        totalPrice->setText("");
    }
    StaticDisplay *maxForPlayer = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("MaxQuantity"), true);
    if (maxForPlayer != nullptr) {
        maxForPlayer->setText("");
    }
}

//Update the commit controls in the Cargo screen, since we have three of them.
void BaseComputer::configureCargoCommitControls(const Cargo &item, TransactionType trans) {
    if (trans == BUY_CARGO) {
        //"Buy 1" button.
        NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
        assert(commitButton != NULL);
        commitButton->setHidden(false);
        commitButton->setLabel("Buy 1");
        commitButton->setCommand("BuyCargo");

        //"Buy 10" button.
        NewButton *commit10Button = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit10"));
        assert(commit10Button != NULL);
        commit10Button->setHidden(false);
        commit10Button->setLabel("Buy 10");
        commit10Button->setCommand("Buy10Cargo");

        //"Buy All" button.
        NewButton *commitAllButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitAll"));
        assert(commitAllButton != NULL);
        commitAllButton->setHidden(false);
        commitAllButton->setLabel("Buy");
        commitAllButton->setCommand("BuyAllCargo");

        const int maxQuantity = maxQuantityForPlayer(item, item.GetQuantity());

        //Total price display.
        const double totalPrice = (double)item.GetPrice() * (double)maxQuantity;
        std::string tempString = (boost::format("Total: #b#%1$.2f#-b") % totalPrice).str();
        StaticDisplay *totalDisplay = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("TotalPrice"));
        assert(totalDisplay != NULL);
        totalDisplay->setText(tempString);

        //Limit if we have one.
        StaticDisplay *maxForPlayer = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("MaxQuantity"));
        assert(maxForPlayer != NULL);
        if (maxQuantity >= item.GetQuantity()) {
            //No limits, so let's not mention anything.
            maxForPlayer->setText("");
        } else {
            std::string maxString = (boost::format("Max: #b#%1$d#-b") % maxQuantity).str();
            maxForPlayer->setText(maxString);
        }
    } else {
        assert(trans == SELL_CARGO);

        //"Sell" button.
        NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
        assert(commitButton != NULL);
        commitButton->setHidden(false);
        commitButton->setLabel(item.IsMissionFlag() ? "Dump 1" : "Sell 1");
        commitButton->setCommand("SellCargo");

        //"Sell 10" button.
        NewButton *commit10Button = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit10"));
        assert(commit10Button != NULL);
        commit10Button->setHidden(false);
        commit10Button->setLabel(item.IsMissionFlag() ? "Dump 10" : "Sell 10");
        commit10Button->setCommand("Sell10Cargo");

        //"Sell All" button.
        NewButton *commitAllButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitAll"));
        assert(commitAllButton != NULL);
        commitAllButton->setHidden(false);
        commitAllButton->setLabel(item.IsMissionFlag() ? "Dump" : "Sell");
        commitAllButton->setCommand("SellAllCargo");

        //Total price display.
        const double totalPrice = (double)item.GetPrice() * (double)item.GetQuantity() * (double)(item.IsMissionFlag() ? 0 : 1);
        std::string tempString = (boost::format("Total: #b#%1$.2f#-b") % totalPrice).str();
        StaticDisplay *totalDisplay = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("TotalPrice"));
        assert(totalDisplay != NULL);
        totalDisplay->setText(tempString);

        //No limit.
        StaticDisplay *maxForPlayer = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("MaxQuantity"));
        assert(maxForPlayer != NULL);
        maxForPlayer->setText("");
    }
}

//Update the commit controls in the Cargo screen, since we have three of them.
bool BaseComputer::configureUpgradeCommitControls(const Cargo &item, TransactionType trans) {
    bool damaged_mode = false;
    Unit *unit = m_player.GetUnit();
    if (trans == BUY_UPGRADE) {
        //base inventory
        NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
        assert(commitButton != NULL);
        commitButton->setHidden(false);
        commitButton->setLabel("Buy");
        commitButton->setCommand("BuyUpgrade");

        NewButton *commitFixButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitFix"));
        assert(commitButton != NULL);
        commitFixButton->setHidden(true);
        commitFixButton->setLabel("Fix");
        commitFixButton->setCommand("FixUpgrade");
    } else {
        //Sell Upgrade - Local Inventory
        NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
        assert(commitButton != NULL);
        if (unit) {
            bool damaged = unit->ShipDamaged();
            bool can_sell = item.CanSell(damaged);

            if(item.IsIntegral()) {
                commitButton->setHidden(true);
            } else if (can_sell) {
                commitButton->setHidden(false);
                commitButton->setLabel("Sell");
                commitButton->setCommand("SellUpgrade");
            } else if(damaged) {
                damaged_mode = true;
                commitButton->setHidden(true);
                commitButton->setLabel("Fix1st");
                commitButton->setCommand("");
            }
        }
        NewButton *commitFixButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("CommitFix"));
        bool unhidden = true;
        if (m_player.GetUnit()
                && UnitUtil::PercentOperational(item, m_player.GetUnit(), item.GetName(), item.GetCategory(), false) < 1) {
            if (m_base.GetUnit()) {
                if (RepairPrice(m_player.GetUnit(), &item) <= ComponentsManager::credits) {
                    assert(commitFixButton != NULL);
                    if (commitFixButton) {
                        commitFixButton->setHidden(false);
                        commitFixButton->setLabel("Fix");
                        commitFixButton->setCommand("FixUpgrade");
                        unhidden = false;
                    }
                }
            } 
        } else if(item.IsWeapon()) {
            if(canReload(m_player.GetUnit(), item.GetName())) {
                if (commitFixButton) {
                    commitFixButton->setHidden(false);
                    commitFixButton->setLabel("Reload");
                    commitFixButton->setCommand("ReloadUpgrade");
                    unhidden = false;
                }
            }
        }
        if (unhidden && commitFixButton) {
            commitFixButton->setHidden(true);
        }
    }
    return damaged_mode;
}

//string buildShipDescription(Cargo &item,string & descriptiontexture); //Redundant definition
//Update the controls when the selection for a transaction changes.
void BaseComputer::updateTransactionControlsForSelection(TransactionList *tlist) {
    //Get the controls we need.
    NewButton *commitButton = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit"));
    assert(commitButton != NULL);
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    std::string descriptiontexture;
    assert(desc != NULL);
    if (!tlist) {
        //We have no selection.  Turn off UI that commits a transaction.
        m_selectedList = NULL;
        hideCommitControls();
        desc->setText("");
        //Make sure there is no selection.
        if (m_transList1.picker) {
            m_transList1.picker->selectCell(NULL);
        }
        if (m_transList2.picker) {
            m_transList2.picker->selectCell(NULL);
        }
        return;
    }
    //We have a selection of some sort.

    //Set the button state.
    m_selectedList = tlist;

    //Clear selection from other list.
    TransactionList &otherList = ((&m_transList1 == m_selectedList) ? m_transList2 : m_transList1);
    if (otherList.picker) {
        otherList.picker->selectCell(NULL);
    }
    //They selected a cell that has a description.
    //The selected item.
    const PickerCell *cell = tlist->picker->selectedCell();
    assert(cell != NULL);
    Cargo &item = tlist->masterList.at(cell->tag()).cargo;
    bool damaged_mode = false;
    if (!isTransactionOK(item, tlist->transaction)) {
        //We can't do the transaction. so hide the transaction button.
        //This is an odd state.  We have a selection, but no transaction is possible.
        hideCommitControls();
    } else {
        //We can do the transaction.
        commitButton->setHidden(false);
        switch (tlist->transaction) {
            case BUY_CARGO:
                configureCargoCommitControls(item, BUY_CARGO);
                break;
            case BUY_UPGRADE:
                configureUpgradeCommitControls(item, BUY_UPGRADE);
                break;
            case BUY_SHIP:
                commitButton->setLabel("Buy");
                commitButton->setCommand("BuyShip");
                if (item.GetCategory().find("My_Fleet") != string::npos) {
                    //note can only sell it if you can afford to ship it over here.
                    NewButton *commit10Button = vega_dynamic_cast_ptr<NewButton>(window()->findControlById("Commit10"));
                    assert(commit10Button != NULL);
                    commit10Button->setHidden(false);
                    commit10Button->setLabel("Sell");
                    commit10Button->setCommand("SellShip");
                }
                break;
            case SELL_CARGO:
                configureCargoCommitControls(item, SELL_CARGO);
                break;
            case SELL_UPGRADE:
                damaged_mode = configureUpgradeCommitControls(item, SELL_UPGRADE);
                break;
            case ACCEPT_MISSION:
                if (item.GetCategory().find("Active_Missions") != string::npos) {
                    commitButton->setLabel("Abort");
                    const bool allow_abort_mission = configuration().physics.allow_mission_abort;
                    if (allow_abort_mission == false) {
                        commitButton->setHidden(true);
                    }
                } else {
                    commitButton->setLabel("Accept");
                }
                commitButton->setCommand("AcceptMission");
                break;
            default:
                assert(false);             //Missed enum in transaction.
                break;
        }
    }
    //The description string.
    char conversionBuffer[128];
    string text = "";
    string descString;
    string tailString;
    string tempString = "";
    Unit *baseUnit = m_base.GetUnit();
    if (tlist->transaction != ACCEPT_MISSION) {
        //Do the money.
        switch (tlist->transaction) {
            case BUY_CARGO:
                if (item.GetDescription() == "" || item.GetDescription()[0] != '@') {
                    buildShipDescription(item, descriptiontexture); //Check for ship
                    string temp = item.GetDescription(); //do first, so can override default image, if so desired
                    if ((string::npos != temp.find('@'))
                            && ("" != descriptiontexture)) { //not already pic-annotated, has non-null ship pic
                        temp = "@" + descriptiontexture + "@" + temp;
                    }
                    item.SetDescription(temp);
                }
                if (item.GetCategory().find("My_Fleet") != string::npos) {
                    //This ship is in my fleet -- the price is just the transport cost to get it to
                    //the current base.  "Buying" this ship makes it my current ship.
                    //Note: This is a positional format string, as per the Unix98 Open-group printf precise syntax.
                    //This is supported by Boost Format; see
                    //https://www.boost.org/doc/libs/latest/libs/format/doc/format.html#printf_directives .
                    tempString = (boost::format("#b#Transport cost: %1$.2f#-b#n1.5#") % item.GetPrice()).str();
                } else {
                    tempString = (boost::format("Price: #b#%1$.2f#-b#n#")
                            % baseUnit->PriceCargo(item.GetName())).str();
                    descString += tempString;
                    tempString = (boost::format("Cargo volume: %1$.2f cubic meters;  "
                                                "Mass: %2$.2f metric tons#n1.5#") % item.GetVolume() % item.GetMass()).str();
                }
                descString += tempString;
                tailString = buildCargoDescription(item, *this, item.GetPrice());
                break;
            case BUY_UPGRADE:
                if (item.GetName() == BASIC_REPAIR_NAME) {
                    //Basic repair is implemented entirely in this module.
                    //PriceCargo() doesn't know about it.
                    Unit *playerUnit = m_player.GetUnit();
                    int multiplier = 1;
                    if (playerUnit) {
                        multiplier = playerUnit->RepairCost();
                    }
                    tempString = (boost::format("Price: #b#%1$.2f#-b#n1.5#")
                            % (basicRepairPrice() * multiplier))
                            .str();
                } else {
                    tempString = (boost::format("Price: #b#%1$.2f#-b#n1.5#") % baseUnit->PriceCargo(item.GetName()))
                            .str();
                }
                descString += tempString;
                if (item.GetDescription() == "" || item.GetDescription()[0] != '#') {
                    item.SetDescription(buildUpgradeDescription(item, std::map<std::string, std::string>()));
                }
                break;
            case BUY_SHIP:
                if (item.GetCategory().find("My_Fleet") == string::npos) {
                    UniverseUtil::StopAllSounds();
                    if (item.GetPrice() < ComponentsManager::credits) {
                        std::string tmp = item.GetName().substr(0, item.GetName().find("."));
                        UniverseUtil::playSound("sales/salespitch" + tmp + ".wav", QVector(0, 0, 0), Vector(0, 0, 0));
                    } else {
                        UniverseUtil::playSound("sales/salespitchnotenoughmoney.wav",
                                QVector(0, 0, 0),
                                Vector(0, 0, 0));
                    }
                }
                if (item.GetDescription() == "" || item.GetDescription()[0] != '@') {
                    item.SetDescription(buildShipDescription(item, descriptiontexture));
                }
                if (item.GetCategory().find("My_Fleet") != string::npos) {
                    //This ship is in my fleet -- the price is just the transport cost to get it to
                    //the current base.  "Buying" this ship makes it my current ship.
                    tempString = (boost::format("#b#Transport cost: %1$.2f#-b#n1.5#") % item.GetPrice()).str();
                } else {
                    PRETTY_ADDN("", baseUnit->PriceCargo(item.GetName()), 2);
                    tempString = (boost::format("Price: #b#%1%#-b#n#") % text).str();
                    const bool printvolume = configuration().graphics.bases.print_cargo_volume;
                    if (printvolume) {
                        descString += tempString;
                        tempString = (boost::format("Vessel volume: %1$.2f cubic meters;  "
                                                    "Mass: %2$.2f metric tons#n1.5#") % item.GetVolume() % item.GetMass()).str();
                    }
                }
                descString += tempString;
                break;
            case SELL_CARGO:
                if (item.GetDescription() == "" || item.GetDescription()[0] != '@') {
                    buildShipDescription(item, descriptiontexture); //Check for ship
                    string temp = item.GetDescription(); //do first, so can override default image, if so desired
                    if ((string::npos != temp.find('@'))
                            && ("" != descriptiontexture)) { //not already pic-annotated, has non-null ship pic
                        temp = "@" + descriptiontexture + "@" + item.GetDescription();
                    }
                    item.SetDescription(temp);
                }
                if (item.IsMissionFlag()) {
                    tempString = "Destroy evidence of mission cargo. Credit received: 0.00.";
                } else {
                    tempString = (boost::format("Value: #b#%1$.2f#-b, purchased for %2$.2f#n#")
                            % baseUnit->PriceCargo(item.GetName())
                            % item.GetPrice())
                            .str();
                }
                descString += tempString;
                tempString = (boost::format("Cargo volume: %1$.2f cubic meters;  Mass: %2$.2f metric tons#n1.5#")
                        % item.GetVolume() % item.GetMass()).str();
                descString += tempString;

                if (!item.IsMissionFlag()) {
                    tailString = buildCargoDescription(item, *this, baseUnit->PriceCargo(item.GetName()));
                }
                break;
            case SELL_UPGRADE:

                //********************************************************************************************
            {
                double percent_working = GetOperational(m_player.GetUnit(), &item);
                if (percent_working < 1) {
                    //IF DAMAGED
                    tempString = (boost::format("Damaged and Used value: #b#%1$.2f#-b, purchased for %2$.2f#n1.5#")
                            % SellPrice(m_player.GetUnit(), &item)
                            % item.GetPrice())
                            .str();
                    descString += tempString;
                    double repair_price = RepairPrice(m_player.GetUnit(), &item);

                    tempString = (boost::format("Percent Working: #b#%1$.2f#-b, Repair Cost: %2$.2f#n1.5#")
                            % (percent_working * 100)
                            % repair_price)
                            .str();
                    descString += tempString;
                } else {
                    tempString = (boost::format("Used value: #b#%1$.2f#-b, purchased for %2$.2f#n1.5#")
                            % usedValue(baseUnit->PriceCargo(item.GetName())) % item.GetPrice()).str();
                    descString += tempString;
                }
                if (damaged_mode) {
                    descString +=
                            "#c1:0:0#Warning: #b#Because pieces of your ship are damaged, you will not be able to "
                            "sell this item until you fix those damaged items in this column in order to allow the "
                            "mechanics to remove this item.#-c#-b#n1.5#";
                }

                if(item.IsWeapon()) {
                    if(canReload(m_player.GetUnit(), item.GetName())) {
                        descString += getReloadDescription(m_player.GetUnit(), item.GetPrice(), item.GetName());
                    }
                }
                
                //********************************************************************************************
                if (item.GetDescription() == "" || item.GetDescription()[0] != '#') {
                    std::map<std::string, std::string> ship_map = m_player.GetUnit()->UnitToMap();
                    item.SetDescription(buildUpgradeDescription(item, ship_map));
                }
                break;
            }
            default:
                assert(false);             //Missed transaction enum in switch statement.
                break;
        }
    }
    //Description.
    // TODO: this adds the description to the price. We should move everything to component.
    descString += item.GetDescription();
    descString += tailString;

    //Change the description control.
    string::size_type pic = descString.find('@');
    StaticImageDisplay
            *desc_image = vega_dynamic_cast_ptr<StaticImageDisplay>(window()->findControlById("DescriptionImage"), true);
    if (desc_image == nullptr) {
        return;
    }
    if (pic != string::npos) {
        std::string texture = descString.substr(pic + 1);
        descString = descString.substr(0, pic);
        string::size_type picend = texture.find('@');
        if (picend != string::npos) {
            descString += texture.substr(picend + 1);
            texture = texture.substr(0, picend);
        }
        if (desc_image) {
            desc_image->setTexture(texture);
        }
    } else {
        if (desc_image && descriptiontexture.empty()) {
            desc_image->setTexture("blackclear.png");
        } else if (desc_image) {
            desc_image->setTexture(descriptiontexture);
        }
    }
    {
        pic = descString.find('<');
        if (pic != string::npos) {
            std::string tmp = descString.substr(pic + 1);
            descString = descString.substr(0, pic);
            pic = tmp.find('>');
            if (pic != string::npos) {
                descString += tmp.substr(pic + 1);
            }
        }
    }
    desc->setText(descString);
}

//Something in a Picker was selected.
bool BaseComputer::pickerChangedSelection(const EventCommandId &command, Control *control) {
    assert(control != NULL);
    Picker *picker = static_cast< Picker * > (control);
    PickerCell *cell = picker->selectedCell();

    //Figure out which transaction list we are using.
    assert(picker == m_transList1.picker || picker == m_transList2.picker);
    TransactionList *tlist = ((picker == m_transList1.picker) ? &m_transList1 : &m_transList2);
    if (m_base.GetUnit()) {
        if (!cell) {
            //The selection just got cleared.
            TransactionList &otherList = ((&m_transList1 == tlist) ? m_transList2 : m_transList1);
            if (otherList.picker && otherList.picker->selectedCell()) {
                //Special case.  The other picker has a selection -- we are seeing the selection
                //cleared in this picker as result.  Do nothing.
            } else {
                updateTransactionControlsForSelection(NULL);
            }
        } else if (cell->tag() == CATEGORY_TAG) {
            //They just selected a category.  Clear the selection no matter what.
            updateTransactionControlsForSelection(NULL);
        } else {
            //Make the controls right for this item.
            updateTransactionControlsForSelection(tlist);
        }
    }
    return true;
}

bool UpgradeAllowed(const Cargo &item, Unit *playerUnit) {
    if(!playerUnit->AllowedUpgrade(item)) {
        color_prohibited_upgrade_flag = true;
        return false;
    }

    return true;
}

//Return whether or not the current item and quantity can be "transacted".
bool BaseComputer::isTransactionOK(const Cargo &originalItem, TransactionType transType, int quantity) {
    if (originalItem.IsMissionFlag() && transType != SELL_CARGO) {
        color_downgrade_or_noncompatible_flag = true;
        return false;
    }
    //Make sure we have somewhere to put stuff.
    Unit *playerUnit = m_player.GetUnit();
    if (!playerUnit) {
        return false;
    }

    if (!playerUnit->IsPlayerShip()) {
        return false;
    }
    //Need to fix item so there is only one for cost calculations.
    Cargo item = originalItem;
    item.SetQuantity(quantity);
    Unit *baseUnit = m_base.GetUnit();
    bool havemoney = true;
    bool havespace = true;
    switch (transType) {
        case BUY_CARGO:
            //Enough credits and room for the item in the ship.
            havemoney = item.GetPrice() * quantity <= ComponentsManager::credits;
            havespace = playerUnit->cargo_hold.CanAddCargo(item);
            if (havemoney && havespace) {
                return true;
            } else {
                if (!havemoney) {
                    color_insufficient_money_flag = true;
                }
                if (!havespace) {
                    color_insufficient_space_flag = true;
                }
            }
            break;
        case SELL_CARGO:
            // Change this code if you want to consider shop cargo space or money.
            // Also, consider mission cargo behavior. Can it be sold?
            return true;

        case BUY_SHIP:
            //Either you are buying this ship for your fleet, or you already own the
            //ship and it will be transported to you.
            if (baseUnit) {
                if (item.GetPrice() * quantity <= ComponentsManager::credits) {
                    return true;
                } else {
                    color_insufficient_money_flag = true;
                }
            }
            break;
        case ACCEPT_MISSION:
            //Make sure the player doesn't take too many missions.
            if (item.GetCategory().find("Active_Missions") != string::npos
                    || active_missions.size() < UniverseUtil::maxMissions()) {
                return true;
            }
            break;
        case SELL_UPGRADE:
            // Change this code if you want to consider shop cargo space or money.
            return true;

        case BUY_UPGRADE:
            //cargo.mission == true means you can't do the transaction.
            havemoney = item.GetPrice() * quantity <= ComponentsManager::credits;
            havespace = (playerUnit->upgrade_space.CanAddCargo(item) || upgradeNotAddedToCargo(item.GetCategory()));

            //UpgradeAllowed must be first -- short circuit && operator
            if (UpgradeAllowed(item, playerUnit) && havemoney && havespace && !item.IsMissionFlag()) {
                return true;
            } else {
                if (!havemoney) {
                    color_insufficient_money_flag = true;
                }
                if (!havespace) {
                    color_insufficient_space_flag = true;
                }
            }
            break;
        default:
            assert(false);            //Missed an enum in transaction switch statement.
            break;
    }
    return false;
}

//Create whatever cells are needed to add a category to the picker.
SimplePickerCell *BaseComputer::createCategoryCell(SimplePickerCells &cells,
        const string &origCategory,
        bool skipFirstCategory) {
    string category = origCategory;
    if (skipFirstCategory) {
        //Skip over the first category piece.
        const string::size_type sepLoc = category.find(CATEGORY_SEP);
        if (sepLoc == string::npos) {
            //No category separator.  At most one category.
            category.erase(0, category.size());
        } else {
            //Skip the first piece.
            category = origCategory.substr(sepLoc + 1);
        }
    }
    if (category.size() == 0) {
        //No category at all.
        return NULL;
    }
    //Create or reuse a cell for the first part of the category.
    const string::size_type loc = category.find(CATEGORY_SEP);
    const string currentCategory = category.substr(0, loc);
    if (cells.count() > 0 && cells.cellAt(cells.count() - 1)->id() == currentCategory) {
        //Re-use the category we have.
    } else {
        //Need to make a new cell for this.
        cells.addCell(new SimplePickerCell(beautify(currentCategory),
                currentCategory,
                CATEGORY_TEXT_COLOR(),
                CATEGORY_TAG));
    }
    SimplePickerCell
            *parentCell = static_cast< SimplePickerCell * > ( cells.cellAt(cells.count() - 1));     //Last cell in list.
    const bool showDefault = configuration().graphics.open_picker_categories;
    parentCell->setHideChildren(!showDefault);
    if (loc == string::npos) {
        //This is a simple category -- we are done.
        return parentCell;
    } else {
        //The category string has more stuff in it -- finish it up.
        SimplePickerCells *childCellList = static_cast< SimplePickerCells * > ( parentCell->children());
        if (!childCellList) {
            //If parent doesn't have room children, we create the room manually.
            parentCell->createEmptyChildList();
            childCellList = static_cast< SimplePickerCells * > ( parentCell->children());
        }
        const string newCategory = category.substr(loc + 1);
        return createCategoryCell(*childCellList, newCategory, false);
    }
    assert(false);
    //Never get here.
}

//Load a picker with a list of items.
void BaseComputer::loadListPicker(TransactionList &tlist,
        SimplePicker &picker,
        TransactionType transType,
        bool skipFirstCategory) {
    //Make sure the transactionList has the correct info.
    tlist.picker = &picker;
    tlist.transaction = transType;

    //Make sure there is nothing old lying around in the picker.
    picker.clear();

    //Iterate through the list and load the picker from it.
    string currentCategory = "--ILLEGAL CATEGORY--";     //Current category we are adding cells to.
    SimplePickerCell *parentCell = NULL;                //Place to add new items.  NULL = Add to picker.
    for (size_t i = 0; i < tlist.masterList.size(); i++) {
        Cargo &item = tlist.masterList.at(i).cargo;

        std::string icategory = getDisplayCategory(item);
        if (icategory != currentCategory) {
            //Create new cell(s) for the new category.
            parentCell = createCategoryCell(
                    *static_cast< SimplePickerCells * > ( picker.cells()), icategory, skipFirstCategory);
            currentCategory = icategory;
        }
        //Construct the cell for this item.
        //JS_NUDGE -- this is where I'll need to do the upgrades colorations goop hooks
        const bool transOK = isTransactionOK(item, transType);

        string itemName = beautify(UniverseUtil::LookupUnitStat(item.GetName(), "upgrades", "Name"));
        string originalName = itemName;
        if (itemName == "") {
            itemName = beautify(item.GetName());
        }
        if (item.GetQuantity() > 1) {
            //If there is more than one item, show the number of items.
            itemName += " (" + tostring(item.GetQuantity()) + ")";
        }
//*******************************************************************************

        //Clear color means use the text color in the picker.
        GFXColor bad_trans_color = NO_MONEY_COLOR();
        if (color_downgrade_or_noncompatible_flag) {
            bad_trans_color = DOWNGRADE_OR_NONCOMPAT_COLOR();
        } else if (color_prohibited_upgrade_flag) {
            bad_trans_color = PROHIBITED_COLOR();
        } else if (color_insufficient_space_flag) {
            bad_trans_color = NO_ROOM_COLOR();
        } else if (color_insufficient_money_flag) {
            //Just in case we want to change the default reason for non-purchase
            bad_trans_color = NO_MONEY_COLOR();
        }
        GFXColor base_color = (transOK ? (item.IsMissionFlag() ? MISSION_COLOR() : GUI_CLEAR) : bad_trans_color);
        //Reset cause-color flags
        color_prohibited_upgrade_flag = false;
        color_downgrade_or_noncompatible_flag = false;
        color_insufficient_space_flag = false;
        color_insufficient_money_flag = false;
        GFXColor final_color;
        if (transType == SELL_UPGRADE && m_player.GetUnit()) {
            //Adjust the base color if the item is 'damaged'
            double percent_working =
                    UnitUtil::PercentOperational(item, m_player.GetUnit(), item.GetName(), item.GetCategory(), false);

            final_color = GFXColor(
                    (1.0 * percent_working) + (1.0 * (1.0 - percent_working)),
                    (1.0 * percent_working) + (0.0 * (1.0 - percent_working)),
                    (0.0 * percent_working) + (0.0 * (1.0 - percent_working)),
                    (1.0 * percent_working) + (1.0 * (1.0 - percent_working))
            );
            if (percent_working == 1.0) {
                final_color = base_color;
            }               //working = normal color
            if (percent_working == 0.0) {
                final_color = ITEM_DESTROYED_COLOR();
            }                   //dead = grey
        } else {
            final_color = base_color;
        }
        SimplePickerCell *cell = new SimplePickerCell(itemName, item.GetName(), final_color, i);

        //*******************************************************************************
        //Add the cell.
        if (parentCell) {
            parentCell->addChild(cell);
        } else {
            picker.addCell(cell);
        }
    }
}

//Load the controls for the CARGO display.

extern int SelectDockPort(Unit *utdw, Unit *parent);

void BaseComputer::loadCargoControls(void) {
    //Make sure there's nothing in the transaction lists.
    resetTransactionLists();
    const bool requireportforlaunch = configuration().physics.cargo_wingmen_only_with_dockport;
    const bool portallowsupgrades = configuration().physics.dockport_allows_upgrade_storage;
    //Set up the base dealer's transaction list. Note that you need a docking port to buy starships!
    vector<string> donttakethis;
    donttakethis.push_back("missions");
//if no docking port OR no permission, no upgrades
    if ((SelectDockPort(m_player.GetUnit(), m_player.GetUnit()) < 0) || (!portallowsupgrades)) {
        donttakethis.push_back("upgrades");
    }
//if no docking port AND no permission, no ships (different from above for VS compatibility)
    if ((SelectDockPort(m_player.GetUnit(), m_player.GetUnit()) < 0) && (requireportforlaunch)) {
        donttakethis.push_back("starships");
    }
    const bool starship_purchase = configuration().physics.starships_as_cargo;
    if (!starship_purchase) {
        donttakethis.push_back("starships");
        donttakethis.push_back("starship");
    }
    loadMasterList(m_base.GetUnit(), m_base.GetUnit()->cargo_hold, vector<string>(), donttakethis, true, m_transList1);     //Anything but a mission.
    SimplePicker *basePicker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("BaseCargo"));
    assert(basePicker != NULL);
    loadListPicker(m_transList1, *basePicker, BUY_CARGO);

    //Set up the player's transaction list.
    loadMasterList(m_player.GetUnit(), m_player.GetUnit()->cargo_hold,
            vector<string>(),
            donttakethis,
            true,
            m_transList2);     //Anything but a mission.
    SimplePicker *inventoryPicker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("PlayerCargo"));
    assert(inventoryPicker != NULL);
    loadListPicker(m_transList2, *inventoryPicker, SELL_CARGO);

    //Make the title right.
    recalcTitle();
}

//Need this class to sort CargoColor's.
class CargoColorSort {
public:
    bool operator()(const CargoColor &a, const CargoColor &b) {
        std::string acategory(a.cargo.GetCategory());
        std::string bcategory(b.cargo.GetCategory());
        std::string::size_type aless = a.cargo.GetDescription().find("<");
        std::string::size_type agreater = a.cargo.GetDescription().find(">");
        std::string::size_type bless = b.cargo.GetDescription().find("<");
        std::string::size_type bgreater = b.cargo.GetDescription().find(">");
        if (aless != string::npos && agreater != string::npos) {
            acategory = a.cargo.GetDescription().substr(aless + 1, agreater);
        }
        if (bless != string::npos && bgreater != string::npos) {
            bcategory = b.cargo.GetDescription().substr(bless + 1, bgreater);
        }
        if (acategory == bcategory) {
            return a.cargo < b.cargo;
        }
        return acategory < bcategory;
    }
};

//Get a filtered list of items from a unit.
void BaseComputer::loadMasterList(Unit *un,  CargoHold &hold,
        const vector<string> &filtervec,
        const vector<string> &invfiltervec,
        bool removezero,
        TransactionList &tlist) {
    vector<CargoColor> *items = &tlist.masterList;
    for (size_t i = 0; i < hold.Size(); i++) {
        bool filter = filtervec.empty();
        bool invfilter = true;
        size_t vecindex;
        for (vecindex = 0; !filter && (vecindex < filtervec.size()); vecindex++) {
            if (hold.GetCargo(i).GetCategory().find(filtervec.at(vecindex)) != string::npos) {
                filter = true;
            }
        }
        for (vecindex = 0; invfilter && (vecindex < invfiltervec.size()); vecindex++) {
            if (hold.GetCargo(i).GetCategory().find(invfiltervec.at(vecindex)) != string::npos) {
                invfilter = false;
            }
        }
        if (filter && invfilter) {
            if ((!removezero) || hold.GetCargo(i).GetQuantity() > 0) {
                CargoColor col;
                col.cargo = hold.GetCargo(i);
                if (col.cargo.GetCategory() == "") {
                    col.cargo.SetCategory("#c.5:1:.3#Uncategorized Cargo");
                }
                items->push_back(col);
            }
        }
    }
    std::sort(items->begin(), items->end(), CargoColorSort());
}

//Return a pointer to the selected item in the picker with the selection.
Cargo *BaseComputer::selectedItem(void) {
    Cargo *result = NULL;
    if (m_selectedList) {
        assert(m_selectedList->picker);
        PickerCell *cell = m_selectedList->picker->selectedCell();
        if (cell) {
            result = &m_selectedList->masterList.at(cell->tag()).cargo;
        }
    }
    return result;
}

//Update the transaction controls after a transaction.
void BaseComputer::updateTransactionControls(const Cargo &item, bool skipFirstCategory) {
    //Go reselect the item.
    if (m_selectedList == NULL) {
        return;
    }
    const bool success = scrollToItem(m_selectedList->picker, item, true, skipFirstCategory);
    //And scroll to that item in the other picker, too.
    TransactionList &otherList = ((&m_transList1 == m_selectedList) ? m_transList2 : m_transList1);
    if (otherList.picker) {
        //Scroll to the item in the other list, but don't select it.
        scrollToItem(otherList.picker, item, false, skipFirstCategory);
    }
    if (success) {
        //We selected the item successfully.
        updateTransactionControlsForSelection(m_selectedList);
    } else {
        //Didn't find item.  Clear the selection.
        updateTransactionControlsForSelection(NULL);
    }
}

//The max number of a particular item this player can buy.  Limits by price, cargo space, etc.
int BaseComputer::maxQuantityForPlayer(const Cargo &item, int suggestedQuantity) {
    int result = 0;

    Unit *playerUnit = m_player.GetUnit();
    if (playerUnit) {
        //Limit by cargo capacity.
        const float volumeLeft = playerUnit->cargo_hold.AvailableCapacity();
        result = (int) guiMin(suggestedQuantity, volumeLeft / item.GetVolume());

        //Limit by price.
        result = (int) guiMin(result, ComponentsManager::credits / item.GetPrice());
    }
    return result;
}



void BaseComputer::refresh() {
    if (m_player.GetUnit()) {
        //Reload the UI -- inventory has changed.  Because we reload the UI, we need to
        //find, select, and scroll to the thing we bought.  The item might be gone from the
        //list (along with some categories) after the transaction.
        std::list<std::list<std::string> > list1save, list2save;
        if (m_transList1.picker) {
            m_transList1.picker->saveOpenCategories(list1save);
        }
        if (m_transList2.picker) {
            m_transList2.picker->saveOpenCategories(list2save);
        }
        processWindowCommand(modeInfo[m_currentDisplay].command, NULL);
        if (m_transList1.picker) {
            m_transList1.picker->restoreOpenCategories(list1save);
        }
        if (m_transList2.picker) {
            m_transList2.picker->restoreOpenCategories(list2save);
        }
        recalcTitle();
    }
}

void BaseComputer::draw() {
    if ((!m_player.GetUnit()) || m_player.GetUnit()->Destroyed()) {
        globalWindowManager().shutDown();
        TerminateCurrentBase();
    }

    if (BaseComputer::dirty && m_player.GetUnit()) {
        refresh();
    }
}

//Buy some items from the Cargo list.  Use -1 for quantity to buy all of the item.
bool BaseComputer::buySelectedCargo(int requestedQuantity) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    Cargo *item = selectedItem();

    if (!(playerUnit && baseUnit && item)) {
        return true;
    }

    playerUnit->BuyCargo(baseUnit, item, requestedQuantity);
    //Reload the UI -- inventory has changed.  Because we reload the UI, we need to
    //find, select, and scroll to the thing we bought.  The item might be gone from the
    //list (along with some categories) after the transaction.
    refresh();         //This will reload master lists.
    return true;
}

//Buy an item from the cargo list.
bool BaseComputer::buyCargo(const EventCommandId &command, Control *control) {
    return buySelectedCargo(1);
}

//Buy an item (quantity 10) from the cargo list.
bool BaseComputer::buy10Cargo(const EventCommandId &command, Control *control) {
    return buySelectedCargo(10);
}

//Buy all of an item from the cargo list.
bool BaseComputer::buyAllCargo(const EventCommandId &command, Control *control) {
    return buySelectedCargo(9999);
}

//Sell some items from the Cargo list.  Use -1 for quantity to buy all of the item.
bool BaseComputer::sellSelectedCargo(int requestedQuantity) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        return true;
    }
    Cargo *item = selectedItem();
    if (item) {
        const int quantity = (requestedQuantity <= 0 ? item->GetQuantity() : requestedQuantity);
        playerUnit->SellCargo(baseUnit, item, quantity);

        //Reload the UI -- inventory has changed.  Because we reload the UI, we need to
        //find, select, and scroll to the thing we bought.  The item might be gone from the
        //list (along with some categories) after the transaction.
        refresh();                //This will reload master lists.
    }
    return true;
}

//Sell an item from ship's cargo.
bool BaseComputer::sellCargo(const EventCommandId &command, Control *control) {
    return sellSelectedCargo(1);
}

//Sell an item (quantity 10) from the cargo list.
bool BaseComputer::sell10Cargo(const EventCommandId &command, Control *control) {
    return sellSelectedCargo(10);
}

//Sell all of an item from the cargo list.
bool BaseComputer::sellAllCargo(const EventCommandId &command, Control *control) {
    return sellSelectedCargo(9999);
}

//Change controls to NEWS mode.
bool BaseComputer::changeToNewsMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != NEWS) {
        switchToControls(NEWS);
    }
    loadNewsControls();
    return true;
}

//The selection in the News picker changed.
bool BaseComputer::newsPickerChangedSelection(const EventCommandId &command, Control *control) {
    assert(control != NULL);
    Picker *picker = static_cast< Picker * > (control);
    PickerCell *cell = picker->selectedCell();

    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    assert(desc != NULL);
    if (cell == NULL) {
        //No selection.  Clear desc.  (Not sure how this can happen, but it's easy to cover.)
        desc->setText("");
    } else {
        desc->setText(cell->text());
    }
    //Turn on some cool music.
    std::string newssong = configuration().audio.news_song;
    muzak->GotoSong(newssong);
    m_playingMuzak = true;
    return true;
}

static std::string GarnerInfoFromSaveGame(const string &filename) {
    return UniverseUtil::getSaveInfo(filename, true);
}

//The selection in the News picker changed.
bool BaseComputer::loadSavePickerChangedSelection(const EventCommandId &command, Control *control) {
    assert(control != NULL);
    Picker *picker = static_cast< Picker * > (control);
    PickerCell *cell = picker->selectedCell();

    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    StaticDisplay *inputbox = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    assert(desc != NULL);
    if (cell == NULL) {
        //No selection.  Clear desc.  (Not sure how this can happen, but it's easy to cover.)
        desc->setText("");
    } else {
        desc->setText(GarnerInfoFromSaveGame(cell->text()));
        if (inputbox != NULL) {
            inputbox->setText(cell->text());
        }
    }
    return true;
}

//Load the controls for the News display.
void BaseComputer::loadNewsControls(void) {
    SimplePicker *picker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("NewsPicker"));
    assert(picker != NULL);
    picker->clear();

    //Load the picker.
    const bool newsFromCargolist = configuration().cargo.news_from_cargo_list;
    if (newsFromCargolist) {
        gameMessage last;
        int i = 0;
        vector<std::string> who;
        who.push_back("news");
        while ((mission->msgcenter->last(i++, last, who))) {
            picker->addCell(new SimplePickerCell(last.message));
        }
    } else {
        //Get news from save game.
        Unit *playerUnit = m_player.GetUnit();
        if (playerUnit) {
            const int playerNum = _Universe->whichPlayerStarship(playerUnit);
            int len = getSaveStringLength(playerNum, NEWS_NAME_LABEL);
            for (int i = len - 1; i >= 0; i--) {
                picker->addCell(new SimplePickerCell(getSaveString(playerNum, NEWS_NAME_LABEL, i)));
            }
        }
    }
    //Make sure the description is empty.
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    assert(desc != NULL);
    desc->setText("");

    //Make the title right.
    recalcTitle();
}

//static int nodirs(const struct dirent *entry) {
//#if defined (_WIN32) || defined(__HAIKU__)
//                                                                                                                            //Have to check if we have the full path or just relative (which would be a problem)
//    std::string tmp = VSFileSystem::homedir + "/save/" + entry->d_name;
//    struct stat s;
//    if (stat( tmp.c_str(), &s ) < 0) {
//        return string( entry->d_name ) != "." && string( entry->d_name ) != "..";
//    }
//    if ( (s.st_mode & S_IFDIR) == 0 && string( entry->d_name ) != "." && string( entry->d_name ) != ".." ) {
//        return 1;
//    }
//#else
//    if (entry->d_type != DT_DIR && string(entry->d_name) != "." && string(entry->d_name) != "..") {
//        return 1;
//    }
//#endif
//    return 0;
//}

//static int datesort(const void *v1, const void *v2) {
//    const struct dirent *d1 = *(const struct dirent **) v1;
//    const struct dirent *d2 = *(const struct dirent **) v2;
//    struct stat s1, s2;
//    std::string tmp = VSFileSystem::homedir + "/save/" + d1->d_name;
//    if (stat(tmp.c_str(), &s1)) {
//        return 0;
//    }
//    tmp = VSFileSystem::homedir + "/save/" + d2->d_name;
//    if (stat(tmp.c_str(), &s2)) {
//        return 0;
//    }
//    return s1.st_mtime - s2.st_mtime;
//}

//typedef int (*scancompare)(const struct dirent **v1, const struct dirent **v2);

// 2022-06-26 Stephen G. Tuggy -- I think this comment is out of sync.
//Load the controls for the News display.
void BaseComputer::loadLoadSaveControls() {
    SimplePicker *picker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("LoadSavePicker"));
    picker->clear();

    //Get news from save game.
    Unit *playerUnit = m_player.GetUnit();
    if (playerUnit) {
        std::string savedir = VSFileSystem::homedir + "/save/";
        boost::filesystem::path savedir_path(savedir);
        std::multimap<std::time_t, std::string, std::greater<std::time_t>> save_files{};
        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(savedir_path)) {
            const boost::filesystem::path& filename = entry.path().filename();
            const std::string filename_string = filename.string();
            if (is_regular_file(entry.status())) {
                const boost::filesystem::path& full_file_path{boost::filesystem::absolute(filename, savedir_path)};
                std::time_t last_write_time = boost::filesystem::last_write_time(full_file_path);
                save_files.emplace(std::make_pair(last_write_time, filename_string));
            }
        }
        for (const auto& x : save_files) {
            picker->addCell(new SimplePickerCell(x.second));
        }

//        struct dirent **dirlist;
//        std::string savedir = VSFileSystem::homedir + "/save/";
//        int ret = scandir(savedir.c_str(), &dirlist, nodirs, (scancompare) &datesort);
//        while (ret-- > 0) {
//            picker->addCell(new SimplePickerCell(dirlist[ret]->d_name));
//        }
    }
    //Make sure the description is empty.
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    desc->setText("");

    //Make the title right.
    recalcTitle();
}

//Change display mode to MISSIONS.
bool BaseComputer::changeToMissionsMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != MISSIONS) {
        switchToControls(MISSIONS);
    }
    loadMissionsControls();
    updateTransactionControlsForSelection(NULL);
    return true;
}

//Load a master list with missions.
void BaseComputer::loadMissionsMasterList(TransactionList &tlist) {
    //Make sure the list is clear.
    tlist.masterList.clear();

    Unit *unit = _Universe->AccessCockpit()->GetParent();
    int playerNum = _Universe->whichPlayerStarship(unit);
    if (playerNum < 0) {
        VS_LOG(error, "Docked ship not a player.");
        return;
    }
    //Number of strings to look at.  And make sure they match!
    const size_t stringCount = getSaveStringLength(playerNum, MISSION_NAMES_LABEL);

    assert(stringCount == getSaveStringLength(playerNum, MISSION_SCRIPTS_LABEL));
    assert(stringCount == getSaveStringLength(playerNum, MISSION_DESC_LABEL));
    //Make sure we have different names for all the missions.
    //This changes the savegame -- it removes ambiguity for good.
    for (size_t current = 0; current < stringCount; current++) {
        const string currentName = getSaveString(playerNum, MISSION_NAMES_LABEL, current);
        size_t count = 1;
        //Check whether any after the current one have the same name.
        for (unsigned int check = current + 1; check < stringCount; ++check) {
            const string checkName = getSaveString(playerNum, MISSION_NAMES_LABEL, check);
            if (check == current) {
                //Found identical names.  Add a "count" at the end.
                putSaveString(playerNum,
                        MISSION_NAMES_LABEL,
                        current,
                        checkName + "_" + tostring(static_cast<int>(count)));
                ++count;
            }
        }
    }
    //Create an entry for each mission.
    for (size_t i = 0; i < stringCount; i++) {
        CargoColor c;
        //Take any categories out of the name and put them in the cargo.category.

        const string finalScript = getSaveString(playerNum, MISSION_SCRIPTS_LABEL, i);
        if (finalScript[0] == '#') {
            continue;
        }                  //Ignore any missions with comments. (those are fixer missions.)

        const string originalName = getSaveString(playerNum, MISSION_NAMES_LABEL, i);
        const string::size_type lastCategorySep = originalName.rfind(CATEGORY_SEP);
        if (lastCategorySep != string::npos) {
            //We have a category.
            c.cargo.SetName(originalName.substr(lastCategorySep + 1));
            c.cargo.SetCategory(originalName.substr(0, lastCategorySep));
        } else {
            //No category in name.
            c.cargo.SetName(originalName);
            c.cargo.SetCategory("");
        }
        c.cargo.SetName(c.cargo.GetName());
        //Description gets name at the top.
        c.cargo.SetDescription("#b#" + beautify(c.cargo.GetName()) + ":#-b#n1.75#"
                + getSaveString(playerNum, MISSION_DESC_LABEL, i));

        tlist.masterList.push_back(c);
    }
    //Sort the list.  Better for display, easier to compile into categories, etc.
    std::sort(tlist.masterList.begin(), tlist.masterList.end(), CargoColorSort());
    if (!active_missions.empty()) {
        for (unsigned int i = 1; i < active_missions.size(); ++i) {
            CargoColor amission;
            amission.cargo.SetName(XMLSupport::tostring(i) + " " + active_missions[i]->mission_name);
            amission.cargo.SetPrice(0);
            amission.cargo.SetQuantity(1);
            amission.cargo.SetCategory("Active_Missions");
            amission.cargo.SetDescription("Objectives\\");
            for (unsigned int j = 0; j < active_missions[i]->objectives.size(); ++j) {
                amission.cargo.SetDescription(
                        amission.cargo.GetDescription() + active_missions[i]->objectives.at(j).objective + ": "
                                + XMLSupport::tostring((int) (100
                                        * active_missions[i]->objectives.at(j).completeness))
                                + "%\\");
            }
            amission.color = DEFAULT_UPGRADE_COLOR();
            tlist.masterList.push_back(amission);
        }
    }
}

//Load the controls for the MISSIONS display.
void BaseComputer::loadMissionsControls(void) {
    //Make sure there's nothing in the transaction lists.
    resetTransactionLists();

    //Load up the list of missions.
    loadMissionsMasterList(m_transList1);
    SimplePicker *picker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("Missions"));
    assert(picker != NULL);
    loadListPicker(m_transList1, *picker, ACCEPT_MISSION);

    //Make the title right.
    recalcTitle();
}

//Accept a mission.
bool BaseComputer::acceptMission(const EventCommandId &command, Control *control) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        return true;
    }
    Cargo *item = selectedItem();
    if (!item || !isTransactionOK(*item, ACCEPT_MISSION)) {
        //Better reload the UI -- we shouldn't have gotten here.
        loadMissionsControls();
        updateTransactionControlsForSelection(NULL);
        return true;
    }
    if (item->GetCategory().find("Active_Missions") != string::npos) {
        unsigned int whichmission = atoi(item->GetName().c_str());
        if (whichmission > 0 && whichmission < active_missions.size()) {
            Mission *miss = active_missions[whichmission];
            miss->terminateMission();
            if (miss == mission) {
                mission = active_missions[0];
            }
            refresh();

            return true;
        }
        return false;
    }
    const int playernum = _Universe->whichPlayerStarship(playerUnit);
    const size_t stringCount = getSaveStringLength(playernum, MISSION_NAMES_LABEL);

    assert(stringCount == getSaveStringLength(playernum, MISSION_SCRIPTS_LABEL));
    string qualifiedName;
    if (item->GetCategory().empty()) {
        qualifiedName = item->GetName();
    } else {
        qualifiedName = item->GetCategory() + CATEGORY_SEP + item->GetName();
    }
    string finalScript;
    for (size_t i = 0; i < stringCount; i++) {
        if (getSaveString(playernum, MISSION_NAMES_LABEL, i) == qualifiedName) {

            finalScript = getSaveString(playernum, MISSION_SCRIPTS_LABEL, i);
            eraseSaveString(playernum, MISSION_SCRIPTS_LABEL, i);
            eraseSaveString(playernum, MISSION_NAMES_LABEL, i);
            eraseSaveString(playernum, MISSION_DESC_LABEL, i);

            break;
        }
    }
    if (finalScript.empty()) {
        return false;
    } else {
        LoadMission(("#" + item->GetCategory()).c_str(),
                finalScript, false);
        refresh();

    }
    //We handled the command, whether we successfully accepted the mission or not.
    return true;
}

//Load the all the controls for the UPGRADE display.
void BaseComputer::loadUpgradeControls(void) {
    //Make sure there's nothing in the transaction lists.
    resetTransactionLists();

    //Load the controls.
    loadBuyUpgradeControls();
    loadSellUpgradeControls();

    //Make the title right.
    recalcTitle();
}

//Load the BUY controls for the UPGRADE display.
void BaseComputer::loadBuyUpgradeControls(void) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();

    TransactionList &tlist = m_transList1;
    tlist.masterList.clear();     //Just in case

    //Get all the upgrades.
    assert(equalColors(CargoColor().color, DEFAULT_UPGRADE_COLOR()));
    std::vector<std::string> filtervec;
    filtervec.push_back("upgrades");
    loadMasterList(baseUnit, baseUnit->cargo_hold, filtervec, std::vector<std::string>(), true, tlist);
    playerUnit->FilterUpgradeList(tlist.masterList);

    //Mark all the upgrades that we can't do.
    //cargo.mission == true means we can't upgrade this.
    for (auto iter = tlist.masterList.begin(); iter != tlist.masterList.end(); ++iter) {
        iter->cargo.SetMissionFlag((!equalColors(iter->color, DEFAULT_UPGRADE_COLOR())));
    }

    // Filter integral from masterList
    auto integral_items = std::stable_partition(tlist.masterList.begin(), tlist.masterList.end(),
                        [](const CargoColor& cc) {
                          return !cc.cargo.IsIntegral();
                        });
    tlist.masterList.erase(integral_items, tlist.masterList.end());

    //Add Basic Repair.
    CargoColor repair;
    repair.cargo.SetName(BASIC_REPAIR_NAME);
    repair.cargo.SetPrice(basicRepairPrice() * playerUnit->RepairCost());
    repair.cargo.SetDescription(BASIC_REPAIR_DESC);
    tlist.masterList.push_back(repair);

    //Load the upgrade picker from the master tlist.
    SimplePicker *basePicker = vega_dynamic_cast_ptr<SimplePicker> ( window()->findControlById("BaseUpgrades"));
    assert(basePicker != nullptr);
    loadListPicker(tlist, *basePicker, BUY_UPGRADE, true);

    //Fix the Basic Repair color.
    SimplePickerCells *baseCells = vega_dynamic_cast_ptr<SimplePickerCells> ( basePicker->cells());
    SimplePickerCell *repairCell = vega_dynamic_cast_ptr<SimplePickerCell> ( baseCells->cellAt(baseCells->count() - 1));
    assert(repairCell->text() == BASIC_REPAIR_NAME);
    if (isClear(repairCell->textColor())) {
        //Have repair cell, and its color is normal.
        repairCell->setTextColor(BASIC_REPAIR_TEXT_COLOR());
    }
}

//Load the SELL controls for the UPGRADE display.
void BaseComputer::loadSellUpgradeControls(void) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        return;
    }
    TransactionList &tlist = m_transList2;
    tlist.masterList.clear();     //Just in case

    //Get a list of upgrades on our ship we could sell.
    Unit *partListUnit = &GetUnitMasterPartList();

    loadMasterList(partListUnit, partListUnit->cargo_hold, weapfiltervec, std::vector<std::string>(), false, tlist);
    ClearDowngradeMap();
    playerUnit->FilterDowngradeList(tlist.masterList);
    const bool clearDowngrades = configuration().physics.only_show_best_downgrade;
    if (clearDowngrades) {
        std::set<std::string> downgradeMap = GetListOfDowngrades();
        for (unsigned int i = 0; i < tlist.masterList.size(); ++i) {
            if (downgradeMap.find(tlist.masterList.at(i).cargo.GetName()) == downgradeMap.end()) {
                tlist.masterList.erase(tlist.masterList.begin() + i);
                i--;
            }
        }
    }
    //Mark all the upgrades that we can't do.
    //cargo.mission == true means we can't upgrade this.
    vector<CargoColor>::iterator iter;
    for (iter = tlist.masterList.begin(); iter != tlist.masterList.end(); iter++) {
        iter->cargo.SetMissionFlag((!equalColors(iter->color, DEFAULT_UPGRADE_COLOR())));
    }
    std::vector<std::string> invplayerfiltervec = weapfiltervec;
    std::vector<string> playerfiltervec;
    playerfiltervec.push_back("upgrades");
    loadMasterList(playerUnit, playerUnit->upgrade_space, playerfiltervec, invplayerfiltervec, false, tlist);     //Get upgrades, but not weapons.

    //Sort the tlist.  Better for display, easier to compile into categories, etc.
    std::sort(tlist.masterList.begin(), tlist.masterList.end(), CargoColorSort());

    //Load the upgrade picker form the master list.
    SimplePicker *basePicker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("PlayerUpgrades"));
    assert(basePicker != NULL);
    loadListPicker(tlist, *basePicker, SELL_UPGRADE, true);
}

//Change display mode to UPGRADE.
bool BaseComputer::changeToUpgradeMode(const EventCommandId &command, Control *control) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        return true;
    }
    if (m_currentDisplay != UPGRADE) {
        switchToControls(UPGRADE);
    }
    loadUpgradeControls();
    updateTransactionControlsForSelection(NULL);
    return true;
}


static void BasicRepair(Unit *parent) {
    // This function does nothing. Kept for compatibility with python API.
    // TODO: remove
}

//The "Operation" classes deal with upgrades.
//There are a number of potential questions that get asked, and a bunch of state that needs
//to be maintained between the questions.  Rather than cluttering up the main class with this
//stuff, it's all declared internally here.

//Base class for operation.  Support functions and common data.
//Should delete itself when done.
class BaseComputer::UpgradeOperation : public ModalDialogCallback {
protected:
    UpgradeOperation(BaseComputer &p) :
            m_parent(p), m_newPart(NULL), m_part(), m_selectedMount(0), m_selectedTurret(0), m_selectedItem() {
    };

    virtual ~UpgradeOperation(void) {
    }

    bool commonInit(void);            //Initialization.
    void finish(void);                //Finish up -- destruct the object.  MUST CALL THIS LAST.
    bool endInit(void);               //Finish initialization.  Returns true if successful.
    bool gotSelectedMount(int index); //We have the mount number.  Returns true if the operation was completed.
    bool gotSelectedTurret(int index); //We have the turret number.  Returns true if the operation was completed.
    void updateUI(void);              //Make the UI right after we are done.

//OVERRIDES FOR DERIVED CLASSES.
    virtual bool checkTransaction(void) = 0; //Check, and verify user wants transaction.
    virtual void concludeTransaction(void) = 0; //Finish the transaction.
    virtual void selectMount(void) = 0;               //Let the user pick a mount.
    virtual void showTurretPicker(void);      //Let the user pick a turret.

    virtual void modalDialogResult( //Dispatch to correct function after some modal UI.
            const std::string &id, int result, WindowController &controller);

    BaseComputer &m_parent;         //Parent class that created us.
    const Unit *m_newPart;
    Cargo m_part;                   //Description of upgrade part.
    int m_selectedMount;          //Which mount to use.
    int m_selectedTurret;         //Which turret to use.
    Cargo m_selectedItem;           //Selection from original UI.
};

//Buy an upgrade for our ship.
class BaseComputer::BuyUpgradeOperation : public BaseComputer::UpgradeOperation {
public:
    void start(void);             //Start the operation.

    BuyUpgradeOperation(BaseComputer &p) : UpgradeOperation(p) {
    };
protected:
    virtual bool checkTransaction(void);              //Check, and verify user wants transaction.
    virtual void concludeTransaction(void);   //Finish the transaction.
    virtual void selectMount(void);                           //Let the user pick a mount.

    virtual ~BuyUpgradeOperation(void) {
    }
};

//Sell an upgrade from our ship.
class BaseComputer::SellUpgradeOperation : public BaseComputer::UpgradeOperation {
public:
    void start(void);             //Start the operation.

    SellUpgradeOperation(BaseComputer &p) : UpgradeOperation(p), m_downgradeLimiter(NULL) {
    };
protected:
    virtual bool checkTransaction(void);      //Check, and verify user wants transaction.
    virtual void concludeTransaction(void);   //Finish the transaction.
    virtual void selectMount(void);                           //Let the user pick a mount.

    virtual ~SellUpgradeOperation(void) {
    }

    const Unit *m_downgradeLimiter;
};

//Id's for callbacks.
static const string GOT_MOUNT_ID = "GotMount";
static const string GOT_TURRET_ID = "GotTurret";
static const string CONFIRM_ID = "Confirm";

//Some common initialization.
bool BaseComputer::UpgradeOperation::commonInit(void) {
    Cargo *selectedItem = m_parent.selectedItem();
    if (selectedItem) {
        m_selectedItem = *selectedItem;
        return true;
    } else {
        return false;
    }
}

//Update the UI controls after a transaction has been concluded successfully.
void BaseComputer::UpgradeOperation::updateUI(void) {
    m_parent.refresh();
}

//Finish this operation.
void BaseComputer::UpgradeOperation::finish(void) {
    //Destruct us now.
    delete this;
}

//Finish initialization.  Returns true if successful.
bool BaseComputer::UpgradeOperation::endInit(void) {
    if (m_parent.m_player.GetUnit()) {
        m_newPart = getUnitFromUpgradeName(m_selectedItem.GetName(), m_parent.m_player.GetUnit()->faction);
        if (m_newPart->name != LOAD_FAILED) {
            selectMount();
        } else {
            return false;
        }
    }
    return true;
}

//Let the user pick a turret.
void BaseComputer::UpgradeOperation::showTurretPicker(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!playerUnit) {
        finish();
        return;
    }
    vector<string> mounts;
    for (un_iter unitIter = playerUnit->getSubUnits(); *unitIter != NULL; unitIter++) {
        mounts.push_back((*unitIter)->name);
    }
    showListQuestion("Select turret mount for your turret:", mounts, this, GOT_TURRET_ID);
}

//Got the mount number.
bool BaseComputer::UpgradeOperation::gotSelectedMount(int index) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (index < 0 || !playerUnit) {
        //The user cancelled somehow.
        finish();
        return false;         //kill the window.
    } else {
        m_selectedMount = index;
        if (!(*m_newPart->viewSubUnits())) {
            //Not a turret.  Proceed with the transaction.
            return checkTransaction();
        } else {
            //Is a turret.
            if (*playerUnit->getSubUnits() != NULL) {
                //Need to get selected turret.
                showTurretPicker();
                return false;
            } else {
                //Ship can't take turrets.
                finish();
                showAlert("Your ship does not support turrets.");
                return false;                 //kill the window.
            }
        }
    }
}

//Got the mount number.
bool BaseComputer::UpgradeOperation::gotSelectedTurret(int index) {
    if (index < 0) {
        //The user cancelled somehow.
        finish();
        return false;         //kill the window.
    } else {
        m_selectedTurret = index;
        return checkTransaction();
    }
}

//Dispatch to correct function after some modal UI.
void BaseComputer::UpgradeOperation::modalDialogResult(const std::string &id,
        int result,
        WindowController &controller) {
    if (id == GOT_MOUNT_ID) {
        //Got the selected mount from the user.
        gotSelectedMount(result);
    } else if (id == GOT_TURRET_ID) {
        //Got the selected turret from the user.
        gotSelectedTurret(result);
    } else if (id == CONFIRM_ID) {
        //User answered whether or not to conclude the transaction.
        if (result == YES_ANSWER) {
            //User wants to do this.
            concludeTransaction();
        } else {
            //User doesn't want to do it.  All done.
            finish();
        }
    }
}

//Start the Buy Upgrade Operation.
void BaseComputer::BuyUpgradeOperation::start(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    Unit *baseUnit = m_parent.m_base.GetUnit();
    if (!(playerUnit && baseUnit && commonInit())) {
        finish();
        return;
    }

    int index = baseUnit->cargo_hold.GetIndex(m_selectedItem);
    if( index == -1) {
        finish();
        //The object may be deleted now. Be careful here.
        return;
    }

    Cargo part = baseUnit->cargo_hold.GetCargo(index);     //Whether the base has any of these.

    if (part.GetQuantity() == 0) {
        // Shouldn't happen, but just in case.
        finish();
        return;
    }

    m_part = part;
    endInit();
}

//Custom class that handles picking a mount point.
class UpgradeOperationMountDialog : public ListQuestionDialog {
public:
//Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId &command, Control *control);
};

//Process a command from the window.
bool UpgradeOperationMountDialog::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Picker::NewSelection") {
        assert(control != NULL);
        Picker *picker = static_cast< Picker * > (control);
        PickerCell *cell = picker->selectedCell();
        if (cell && cell->tag() == 0) {
            //An "unselectable" cell was selected.  Turn the selection back off.
            picker->selectCell(NULL);
        }
        return true;
    }
    //Only thing we care about is the selection changing.
    return ListQuestionDialog::processWindowCommand(command, control);
}

//Select the mount to use for selling.
void BaseComputer::BuyUpgradeOperation::selectMount(void) {
    if (m_newPart->getNumMounts() <= 0) {
        //Part doesn't need a mount point.
        gotSelectedMount(0);
        return;
    }
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!playerUnit) {
        finish();
        return;
    }
    //Create a custom list dialog to get the mount point.
    UpgradeOperationMountDialog *dialog = new UpgradeOperationMountDialog;
    dialog->init("Select mount for your item:");
    dialog->setCallback(this, GOT_MOUNT_ID);

    //Fill the dialog picker with the mount points.
    SimplePicker *picker = static_cast< SimplePicker * > ( dialog->window()->findControlById("Picker"));
    assert(picker != NULL);
    for (int i = 0; i < playerUnit->getNumMounts(); i++) {
        //Mount is selectable if we can upgrade with the new part using that mount.
        double percent;             //Temp.  Not used.
        const bool selectable = playerUnit->canUpgrade(m_newPart, i, m_selectedTurret, 0, false, percent);

        //Figure color and label based on weapon that is in the slot.
        GFXColor mountColor = MOUNT_POINT_NO_SELECT();
        string mountName;
        string ammoexp;
        if (playerUnit->mounts.at(i).status == Mount::ACTIVE || playerUnit->mounts.at(i).status == Mount::INACTIVE) {
            mountName = tostring(i + 1) + " " + playerUnit->mounts.at(i).type->name;
            ammoexp =
                    (playerUnit->mounts.at(i).ammo == -1) ? string("") : string((" ammo: "
                            + tostring(playerUnit->mounts.at(i).ammo)));
            mountName += ammoexp;
            mountColor = MOUNT_POINT_FULL();
        } else {
            const std::string temp = getMountSizeString(playerUnit->mounts.at(i).size);
            mountName = tostring(i + 1) + " (Empty) " + temp.c_str();
            mountColor = MOUNT_POINT_EMPTY();
        }
        //If the mount point won't work with the weapon, don't let user select it.
        if (!selectable) {
            mountColor = MOUNT_POINT_NO_SELECT();
        }
        //Now we add the cell.  Note that "selectable" is stored in the tag property.
        picker->addCell(new SimplePickerCell(mountName, "", mountColor, (selectable ? 1 : 0)));
    }
    dialog->run();
}

//Check, and verify user wants Buy Upgrade transaction.  Returns true if more input is required.
bool BaseComputer::BuyUpgradeOperation::checkTransaction(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!playerUnit) {
        finish();
        return false;         //We want the window to die to avoid accessing of deleted memory.
    }
    double percent;         //Temp.  Not used.
    if (playerUnit->canUpgrade(m_newPart, m_selectedMount, m_selectedTurret, 0, false, percent)) {
        //We can buy the upgrade.
        concludeTransaction();
        return false;
    } else {
        showYesNoQuestion("The item cannot fit the frame of your starship.  Do you want to buy it anyway?",
                this, CONFIRM_ID);
        return true;
    }
}

//Finish the transaction.
void BaseComputer::BuyUpgradeOperation::concludeTransaction(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    Unit *baseUnit = m_parent.m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        finish();
        return;
    }
    //Get the upgrade percentage to calculate the full price.
    double percent;
    int numleft = basecargoassets(baseUnit, m_part.GetName());
    while (numleft > 0
            && playerUnit->canUpgrade(m_newPart, m_selectedMount, m_selectedTurret, 0, true, percent)) {
        const float price = m_part.GetPrice();         //* (1-usedValue(percent));
        if (ComponentsManager::credits >= price) {
            //Have enough money.  Buy it.
            ComponentsManager::credits -= price;

            //Upgrade the ship.
            playerUnit->Upgrade(m_newPart, m_selectedMount, m_selectedTurret, 0, true, percent);
            const bool allow_special_with_weapons = configuration().physics.allow_special_and_normal_gun_combo;
            if (!allow_special_with_weapons) {
                playerUnit->ToggleWeapon(false, /*backwards*/ true);
                playerUnit->ToggleWeapon(false, /*backwards*/ false);
            }
            //Remove the item from the base, since we bought it.
            int index = baseUnit->cargo_hold.GetIndex(m_part.GetName());
            Cargo upgrade = baseUnit->cargo_hold.GetCargo(index);
            upgrade.SetInstalled(true);
            baseUnit->cargo_hold.RemoveCargo(baseUnit, index, 1);
            playerUnit->upgrade_space.AddCargo(playerUnit, upgrade);
        } else {
            break;
        }
        if (m_newPart->mounts.size() == 0) {
            break;
        } else if (m_newPart->mounts.at(0).ammo <= 0) {
            break;
        }
        numleft = basecargoassets(baseUnit, m_part.GetName());
    }
    updateUI();

    finish();
}

int basecargoassets(Unit *baseUnit, string cargoname) {
    unsigned int dummy;
    Cargo& somecargo = baseUnit->cargo_hold.GetCargo(baseUnit->cargo_hold.GetIndex(cargoname));
    return somecargo.GetQuantity();
}

//Start the Sell Upgrade Operation.
void BaseComputer::SellUpgradeOperation::start(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!(playerUnit && commonInit())) {
        finish();
        return;
    }
    const string unitDir = GetUnitDir(playerUnit->name.get().c_str());
    const string limiterName = unitDir + ".blank";
    const int faction = playerUnit->faction;

    //If its limiter is not available, just assume that there are no limits.
    try {
        m_part = Manifest::MPL().GetCargoByName(m_selectedItem.GetName());
        endInit();
    } catch (const std::exception& e) {
        VS_LOG(error, (boost::format("Error in GetCargoByName: %1%") % e.what()));
        finish();
        //The object may be deleted now. Be careful here.
    }
}

//Try to match a mounted weapon name with the cargo name.
//Returns true if they are the same.
static bool matchCargoToWeapon(const std::string &cargoName, const std::string &weaponName) {
    //Weapon names have capitalized words, and no spaces between the words.
    //Cargo names are lower-case, and have underscores between words.
    //Also, anything in the Ammo category ends with "_ammo" in cargo, and not in weapon.
    //We try to make a cargo name look like a weapon name, then match them.

    std::string convertedCargoName;

    //Take off "_ammo" if it's there.
    int end = cargoName.size();
    const string::size_type ammoOffset = cargoName.rfind("_ammo");
    if (ammoOffset != std::string::npos) {
        end = ammoOffset;
    }
    bool wordStart = true;              //Start of word.
    for (int i = 0; i < end; i++) {
        const char c = cargoName[i];
        if (c == '_') {
            //Skip this, and make sure next letter is capitalized.
            wordStart = true;
        } else if (wordStart) {
            //Start or a word.  Capitalize the character, and turn off start of word.
            convertedCargoName += toupper(c);
            wordStart = false;
        } else {
            //Normal character in middle of word.
            convertedCargoName += c;
        }
    }
    return strtoupper(convertedCargoName) == strtoupper(weaponName);
}

//Select the mount to use for selling.
void BaseComputer::SellUpgradeOperation::selectMount(void) {
    if (m_newPart->getNumMounts() <= 0) {
        //Part doesn't need a mount point.
        gotSelectedMount(0);
        return;
    }
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!playerUnit) {
        finish();
        return;
    }
    //Create a custom list dialog to get the mount point.
    UpgradeOperationMountDialog *dialog = new UpgradeOperationMountDialog;
    dialog->init("Select mount for your item:");
    dialog->setCallback(this, GOT_MOUNT_ID);

    //Fill the dialog picker with the mount points.
    SimplePicker *picker = static_cast< SimplePicker * > ( dialog->window()->findControlById("Picker"));
    assert(picker != NULL);
    int mount = -1;                     //The mount if there was only one.
    int selectableCount = 0;
    for (int i = 0; i < playerUnit->getNumMounts(); i++) {
        //Whether or not the entry is selectable -- the same as the thing we are selling.
        bool selectable = false;

        //Get the name.
        string mountName;
        if (playerUnit->mounts.at(i).status == Mount::ACTIVE || playerUnit->mounts.at(i).status == Mount::INACTIVE) {
            //Something is mounted here.
            const std::string unitName = playerUnit->mounts.at(i).type->name;
            const Unit *partUnit =
                    UnitConstCache::getCachedConst(StringIntKey(m_part.GetName(), FactionUtil::GetUpgradeFaction()));
            string ammoexp;
            mountName = tostring(i + 1) + " " + unitName.c_str();
            ammoexp =
                    (playerUnit->mounts.at(i).ammo == -1) ? string("") : string((" ammo: "
                            + tostring(playerUnit->mounts.at(i).ammo)));
            mountName += ammoexp;
            if (partUnit) {
                if (partUnit->getNumMounts()) {
                    if (partUnit->mounts.at(0).type == playerUnit->mounts.at(i).type) {
                        selectable = true;
                        selectableCount++;
                        mount = i;
                    }
                }
            } else if (matchCargoToWeapon(m_part.GetName(), unitName)) {
                selectable = true;
                selectableCount++;
                mount = i;
            }
        } else {
            //Nothing at this mount point.
            const std::string temp = getMountSizeString(playerUnit->mounts.at(i).size);
            mountName = tostring(i + 1) + " (Empty) " + temp.c_str();
        }
        //Now we add the cell.  Note that "selectable" is stored in the tag property.
        const GFXColor mountColor = (selectable ? MOUNT_POINT_FULL() : MOUNT_POINT_NO_SELECT());
        picker->addCell(new SimplePickerCell(mountName, "", mountColor, (selectable ? 1 : 0)));
    }
    assert(selectableCount > 0);              //We should have found at least one unit mounted.
    if (selectableCount > 1) {
        //Need to have the user choose.
        dialog->run();
    } else {
        //Get rid of the dialog -- we only have one choice.
        delete dialog;
        gotSelectedMount(mount);
    }
}

//Check, and verify user wants Sell Upgrade transaction.  Returns true if more input is required.
// Only applies to mounts!
bool BaseComputer::SellUpgradeOperation::checkTransaction(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    if (!playerUnit) {
        finish();
        return false;         //We want the window to die to avoid accessing of deleted memory.
    }
    double percent;         //Temp.  Not used.
    if (playerUnit->canDowngrade(m_newPart, m_selectedMount, m_selectedTurret, percent, m_downgradeLimiter)) {
        //We can sell the upgrade.
        concludeTransaction();
        return false;
    } else {
        showYesNoQuestion("You don't have exactly what you wish to sell.  Continue?",
                this, CONFIRM_ID);
        return true;
    }
}

//Finish the transaction.
// Only applies to mounts!
void BaseComputer::SellUpgradeOperation::concludeTransaction(void) {
    Unit *playerUnit = m_parent.m_player.GetUnit();
    Unit *baseUnit = m_parent.m_base.GetUnit();
    if (!(playerUnit && baseUnit)) {
        finish();
        return;
    }
    //Get the upgrade percentage to calculate the full price.
    double percent;
    playerUnit->canDowngrade(m_newPart, m_selectedMount, m_selectedTurret, percent, m_downgradeLimiter);
    const float price = m_part.GetPrice() * usedValue(percent);
    //Adjust the money.
    ComponentsManager::credits += price;
    //Change the ship.
    if (playerUnit->Downgrade(m_newPart, m_selectedMount, m_selectedTurret, percent, m_downgradeLimiter)) {
        //Remove the item from the ship, since we sold it, and add it to the base.
        m_part.SetQuantity(1);
        m_part.SetPrice(baseUnit->PriceCargo(m_part.GetName()));
        baseUnit->cargo_hold.AddCargo(baseUnit, m_part);
    }
    updateUI();

    finish();
}

extern int GetModeFromName(const char *);

//Buy a ship upgrade.
bool BaseComputer::buyUpgrade(const EventCommandId &command, Control *control) {
    //Take care of Basic Repair, which is implemented entirely in this module.
    Cargo *item = selectedItem();
    if (item) {
        Unit *playerUnit = m_player.GetUnit();
        if (item->GetName() == BASIC_REPAIR_NAME) {
            if (playerUnit) {
                BasicRepair(playerUnit);
                if (m_selectedList == NULL) {
                    return true;
                }
                refresh();
                m_transList1.picker->selectCell(NULL);                     //Turn off selection.
            }
            return true;
        }
        if (!item->IsWeapon()) {
            if (playerUnit) {
                Unit *baseUnit = m_base.GetUnit();
                if (baseUnit) {
                    const int quantity = 1;
                    unsigned int index = baseUnit->cargo_hold.GetIndex(*item);
                    playerUnit->BuyUpgrade(baseUnit, item, quantity);
                    playerUnit->Upgrade(item->GetName(), 0, 0, true, false);
                    refresh();
                }
            }
            return true;
        }
    }
    //This complicated operation is done in a separate object.
    BuyUpgradeOperation *op = new BuyUpgradeOperation(*this);
    op->start();

    return true;
}

//Sell an upgrade on your ship.
bool BaseComputer::sellUpgrade(const EventCommandId &command, Control *control) {
    Cargo *item = selectedItem();
    if (item) {
        if (!item->IsWeapon()){
            Cargo sold;
            const int quantity = 1;
            Unit *playerUnit = m_player.GetUnit();
            Unit *baseUnit = m_base.GetUnit();
            if (baseUnit && playerUnit) {
                playerUnit->SellUpgrade(baseUnit, item, quantity);
                UpgradeOperationResult result = playerUnit->UpgradeUnit(item->GetName(), false, true);
                refresh();
            }
            return true;
        }
    }
    //This complicated operation is done in a separate object.
    SellUpgradeOperation *op = new SellUpgradeOperation(*this);
    op->start();

    return true;
}

//Sell an upgrade on your ship.
bool BaseComputer::fixUpgrade(const EventCommandId &command, Control *control) {
    Cargo *item = selectedItem();
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();

    if (baseUnit && playerUnit && item) {
        if (playerUnit->RepairUpgradeCargo(item, baseUnit, RepairPrice(playerUnit, item))) {
            if (UnitUtil::PercentOperational(*item, playerUnit, item->GetName(), "upgrades/", false) < 1.0) {
                emergency_downgrade_mode = "EMERGENCY MODE ";
            }
        }
        refresh();
    }
    return true;
}

// Reload a gun.
bool BaseComputer::reloadUpgrade(const EventCommandId &command, Control *control) {
    Cargo *item = selectedItem();
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();

    if (!baseUnit || !playerUnit || !item) {
        return false;
    }

    
    // Mount name is the name of the weapon as a unit in units.json
    const Unit *weapon = getUnitFromUpgradeName(item->GetName(), FactionUtil::GetUpgradeFaction());
    
    // Something went wrong
    if(!weapon) {
        return false;
    }

    Mount weapon_mount = weapon->mounts[0];
    // Weapon type has no ammo
    if(weapon_mount.ammo == 0) {
        return false;
    }

    // Weapon name is the name of the weapon in weapons.json.
    const std::string weapon_name = weapon->mounts[0].type->name;

    for(Mount& mount : playerUnit->mounts) {
        if(weapon_name != mount.type->name) {
            continue;
        }

        const int max_ammo = weapon_mount.ammo;
        int ammo_to_reload = max_ammo - mount.ammo;
        if(ammo_to_reload == 0) {
            continue;
        }

        // Full reload price is 5% of gun. 
        // TODO: make it configurable
        double reload_price = ammo_to_reload/max_ammo * 0.05 * item->GetPrice();
        if(reload_price < playerUnit->credits) {
            playerUnit->credits -= reload_price;
            mount.ammo = max_ammo;
            refresh();
            return true;
        }
    }

    refresh();
    return true;
}

//Change controls to SHIP_DEALER mode.
bool BaseComputer::changeToShipDealerMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != SHIP_DEALER) {
        switchToControls(SHIP_DEALER);
    }
    loadShipDealerControls();
    updateTransactionControlsForSelection(NULL);
    return true;
}

//Create a Cargo for the specified starship.
Cargo CreateCargoForOwnerStarship(const Cockpit *cockpit, const Unit *base, int i) {
    Cargo cargo;
    cargo.SetQuantity(1);
    cargo.SetVolume(1);
    cargo.SetPrice(0);

    string locationSystemName = cockpit->GetUnitSystemName(i);
    string locationBaseName = cockpit->GetUnitBaseName(i);
    string destinationSystemName = _Universe->activeStarSystem()->getFileName();
    string destinationBaseName = (base != NULL) ? Cockpit::MakeBaseName(base) : "";

    bool needsJumpTransport = (locationSystemName != destinationSystemName);
    bool needsInsysTransport = (locationBaseName != destinationBaseName);

    const float shipping_price_base = configuration().economics.shipping_price_base_flt;
    const float shipping_price_insys = configuration().economics.shipping_price_insys_flt;
    const float shipping_price_perjump = configuration().economics.shipping_price_perjump_flt;

    cargo.SetPrice(shipping_price_base);
    cargo.SetName(cockpit->GetUnitFileName(i));
    cargo.SetCategory("starships/My_Fleet");

    if (needsJumpTransport) {
        vector<string> jumps;
        _Universe->getJumpPath(
                locationSystemName,
                destinationSystemName,
                jumps);
        VS_LOG(trace, (boost::format("Player ship needs transport from %1% to %2% across %3% systems")
                % locationBaseName % destinationSystemName % jumps.size()));
        cargo.SetPrice(cargo.GetPrice() + shipping_price_perjump * (jumps.size() - 1));
    } else if (needsInsysTransport) {
        VS_LOG(trace, (boost::format("Player ship needs insys transport from %1% to %2%")
                % locationBaseName % destinationBaseName));
        cargo.SetPrice(cargo.GetPrice() + shipping_price_insys);
    }

    return cargo;
}

//Create a Cargo for an owned starship from the name.
Cargo CreateCargoForOwnerStarshipName(const Cockpit *cockpit, const Unit *base, std::string name, int &index) {
    for (size_t i = 1, n = cockpit->GetNumUnits(); i < n; ++i) {
        if (cockpit->GetUnitFileName(i) == name) {
            index = i;
            return CreateCargoForOwnerStarship(cockpit, base, i);
        }
    }
    //Didn't find it.
    return Cargo();
}

void SwapInNewShipName(Cockpit *cockpit, Unit *base, const std::string &newFileName, int swappingShipsIndex) {
    Unit *parent = cockpit->GetParent();
    if (parent) {
        size_t putpos = (swappingShipsIndex >= 0) ? swappingShipsIndex : cockpit->GetNumUnits();
        cockpit->GetUnitFileName(putpos) = parent->name;
        cockpit->GetUnitSystemName(putpos) = _Universe->activeStarSystem()->getFileName();
        cockpit->GetUnitBaseName(putpos) = (base != NULL) ? Cockpit::MakeBaseName(base) : string("");
        if (swappingShipsIndex != -1) {
            for (size_t i = 1, n = cockpit->GetNumUnits(); i < n; ++i) {
                if (cockpit->GetUnitFileName(i) == newFileName) {
                    cockpit->RemoveUnit(i);
                    --i; //then ++;
                }
            }
        }
    } else if (swappingShipsIndex != -1) {
        //if parent is dead
        cockpit->RemoveUnit(swappingShipsIndex);
    }
    cockpit->GetUnitFileName() = newFileName;
}

string buildShipDescription(Cargo &item, std::string &texturedescription) {
    VS_LOG(debug, "Entering buildShipDescription");
    //load the Unit
    string newModifications;
    if (item.GetCategory().find("My_Fleet") != string::npos) {
        //Player owns this starship.
        newModifications = _Universe->AccessCockpit()->GetUnitModifications();
    }
    Flightgroup *flightGroup = new Flightgroup();
    int fgsNumber = 0;
    current_unit_load_mode = NO_MESH;

    VS_LOG(debug, "buildShipDescription: creating newPart");
    Unit *newPart = new Unit(item.GetName().c_str(), false, 0, newModifications, flightGroup, fgsNumber);
    current_unit_load_mode = DEFAULT;
    string sHudImage;
    string sImage;
    if (newPart->getHudImage()) {
        if (newPart->getHudImage()->getTexture()) {
            sHudImage = newPart->getHudImage()->getTexture()->texfilename;
            string::size_type delim = sHudImage.find('|');             //cut off alpha texture
            if (delim != string::npos) {
                sImage = sHudImage.substr(delim + 1);
                sHudImage = sHudImage.substr(0,
                        delim
                                - sImage.length());                 //assumes RGBname == Alphaname for ships
            }
            delim = sHudImage.rfind('.');             //cut off mangled base directory
            if (delim != string::npos) {
                sHudImage = sHudImage.substr(delim + 2);
            }
            texturedescription = "../units/" + sHudImage + "/" + sImage;
        }
    }

    std::map<std::string, std::string> ship_map = newPart->UnitToMap();
    std::string str = GetString("get_ship_description", "ship_view",
                          "ship_view.py",
                          ship_map);

    VS_LOG(debug, "buildShipDescription: killing newPart");
    newPart->Kill();
    // VS_LOG(debug, "buildShipDescription: deleting newPart");
    // delete newPart;
    // newPart = nullptr;
    if (texturedescription != "" && (string::npos == str.find('@'))) {
        str = "@" + texturedescription + "@" + str;
    }
    VS_LOG(debug, (boost::format("buildShipDescription: texturedescription == %1%") % texturedescription));
    VS_LOG(debug, (boost::format("buildShipDescription: return value       == %1%") % str));
    VS_LOG(debug, "Leaving buildShipDescription");
    return str;
}

//UNDER CONSTRUCTION
string buildUpgradeDescription(Cargo &item, std::map<std::string, std::string> ship_map) {
    const std::string key = item.GetName() + "__upgrades";
    ship_map["upgrade_key"] = key;
    const std::string text = GetString("get_upgrade_info", "upgrade_view",
        "upgrade_view.py", ship_map);
    return text;
}

class PriceSort {
    const vector<float> &price;
    bool reverse;

public:
    PriceSort(const vector<float> &_price, bool _reverse)
            : price(_price), reverse(_reverse) {
    }

    bool operator()(size_t a, size_t b) {
        if (reverse) {
            return price.at(a) > price.at(b);
        } else {
            return price.at(a) < price.at(b);
        }
    }
};

void trackPrice(int whichplayer, const Cargo &item, float price, const string &systemName, const string &baseName,
        /*out*/ vector<string> &highest, /*out*/ vector<string> &lowest) {
    const size_t toprank = static_cast<size_t>(configuration().general.trade_interface_tracks_prices_top_rank);

    VS_LOG(info, (boost::format("Ranking item %1%/%2% at %3%/%4%")
            % item.GetCategory() % item.GetName() % systemName % baseName));

    // Recorded prices are always sorted, so we first do a quick check to avoid
    // triggering savegame serialization without reason
    string itemkey = string(item.GetCategory()) + "/" + item.GetName();
    string hilock = itemkey + "?hi?loc";
    string lolock = itemkey + "?lo?loc";
    string hipricek = itemkey + "?hi?pcs";
    string lopricek = itemkey + "?lo?pcs";

    // First record the given item's price and update the ranking lists
    {
        string locname = "#b#" + baseName + "#-b# in the #b#" + systemName + "#-b# system";

        {
            bool resort = false;
            {
                // Limited lifetime iterator (points to invalid data after save data manipulation)
                const vector<string> &recordedHighestLocs = getStringList(whichplayer, hilock);
                const vector<float> &recordedHighestPrices = getSaveData(whichplayer, hipricek);
                vector<string>::const_iterator prev = std::find(
                        recordedHighestLocs.begin(), recordedHighestLocs.end(),
                        locname);

                if (prev != recordedHighestLocs.end()) {
                    size_t index = prev - recordedHighestLocs.begin();
                    putSaveData(whichplayer, hipricek, index, price);
                    resort = true;
                } else if (recordedHighestPrices.size() < toprank || recordedHighestPrices.back() < price) {
                    // Track new top price
                    pushSaveString(whichplayer, hilock, locname);
                    pushSaveData(whichplayer, hipricek, price);
                    resort = true;
                }
            }

            if (resort) {
                // Re-get lists, the ones we got earlier could be empty stubs
                const vector<string> &locs = getStringList(whichplayer, hilock);
                const vector<float> &prices = getSaveData(whichplayer, hipricek);
                vector<size_t> indices;

                indices.resize(mymin(prices.size(), locs.size()));
                {
                    for (size_t i = 0; i < indices.size(); ++i) {
                        indices.at(i) = i;
                    }
                }

                std::sort(indices.begin(), indices.end(), PriceSort(prices, true));

                vector<string> newlocs;
                vector<float> newprices;

                newlocs.reserve(indices.size());
                newprices.reserve(indices.size());
                {
                    for (size_t i = 0; i < indices.size() && i < toprank; ++i) {
                        newlocs.push_back(locs[indices.at(i)]);
                        newprices.push_back(prices[indices.at(i)]);
                    }
                }

                // Save new rank list
                saveDataList(whichplayer, hipricek, newprices);
                saveStringList(whichplayer, hilock, newlocs);
            }
        }

        {
            bool resort = false;
            {
                // Limited lifetime iterator (points to invalid data after save data manipulation)
                const vector<string> &recordedLowestLocs = getStringList(whichplayer, lolock);
                const vector<float> &recordedLowestPrices = getSaveData(whichplayer, lopricek);
                auto prev = std::find(
                        recordedLowestLocs.begin(), recordedLowestLocs.end(),
                        locname);

                if (prev != recordedLowestLocs.end()) {
                    size_t index = prev - recordedLowestLocs.begin();
                    putSaveData(whichplayer, lopricek, index, price);
                    resort = true;
                } else if (recordedLowestPrices.size() < toprank || recordedLowestPrices.back() > price) {
                    // Track new top price
                    pushSaveString(whichplayer, lolock, locname);
                    pushSaveData(whichplayer, lopricek, price);
                    resort = true;
                }
            }

            if (resort) {
                // Re-get lists, the ones we got earlier could be empty stubs
                const vector<string> &locs = getStringList(whichplayer, lolock);
                const vector<float> &prices = getSaveData(whichplayer, lopricek);
                vector<size_t> indices;

                indices.resize(mymin(prices.size(), locs.size()));
                {
                    for (size_t i = 0; i < indices.size(); ++i) {
                        indices.at(i) = i;
                    }
                }

                std::sort(indices.begin(), indices.end(), PriceSort(prices, false));

                vector<string> newlocs;
                vector<float> newprices;

                newlocs.reserve(indices.size());
                newprices.reserve(indices.size());
                {
                    for (size_t i = 0; i < indices.size() && i < toprank; ++i) {
                        newlocs.push_back(locs.at(indices.at(i)));
                        newprices.push_back(prices.at(indices.at(i)));
                    }
                }

                // Save new rank list
                saveDataList(whichplayer, lopricek, newprices);
                saveStringList(whichplayer, lolock, newlocs);
            }
        }
    }

    // Now build the top-ranking descriptions
    {
        const vector<string> &recordedHighestLocs = getStringList(whichplayer, hilock);
        const vector<string> &recordedLowestLocs = getStringList(whichplayer, lolock);
        const vector<float> &recordedHighestPrices = getSaveData(whichplayer, hipricek);
        const vector<float> &recordedLowestPrices = getSaveData(whichplayer, lopricek);

        string prefix = "   ";
        char conversionBuffer[128];

        VS_LOG(info, "Tracking data:");
        VS_LOG(info, (boost::format("  highest locs: (%1%)") % recordedHighestLocs.size()));
        {
            for (size_t i = 0; i < recordedHighestLocs.size(); ++i) {
                VS_LOG(info, (boost::format("    %1% : %2%") % i % recordedHighestLocs.at(i)));
            }
        }

        VS_LOG(info, (boost::format("  highest prices: (%1%)") % recordedHighestPrices.size()));
        {
            for (size_t i = 0; i < recordedHighestPrices.size(); ++i) {
                // POSIX-printf style
                VS_LOG(info, (boost::format("    %1$d : %2$.2f") % i % recordedHighestPrices.at(i)));
            }
        }

        VS_LOG(info, (boost::format("  lowest locs: (%1%)") % recordedLowestLocs.size()));
        {
            for (size_t i = 0; i < recordedLowestLocs.size(); ++i) {
                VS_LOG(info, (boost::format("    %1% : %2%") % i % recordedLowestLocs.at(i)));
            }
        }

        VS_LOG(info, (boost::format("  lowest prices: (%1%)") % recordedLowestPrices.size()));
        {
            for (size_t i = 0; i < recordedLowestPrices.size(); ++i) {
                // POSIX-printf style
                VS_LOG(info, (boost::format("    %1$d : %2$.2f") % i % recordedLowestPrices.at(i)));
            }
        }

        VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogs();

        highest.clear();
        highest.resize(recordedHighestPrices.size());
        {
            for (size_t i = 0; i < recordedHighestPrices.size(); ++i) {
                string &text = highest[i];
                PRETTY_ADD("", recordedHighestPrices.at(i), 2);
                text += " (at " + recordedHighestLocs.at(i) + ")";

                VS_LOG(info, (boost::format("Highest item %1%") % text));
            }
        }

        lowest.clear();
        lowest.resize(recordedLowestPrices.size());
        {
            for (size_t i = 0; i < recordedLowestPrices.size(); ++i) {
                string &text = lowest[i];
                PRETTY_ADD("", recordedLowestPrices.at(i), 2);
                text += " (at " + recordedLowestLocs.at(i) + ")";

                VS_LOG(info, (boost::format("Lowest item %1%") % text));
            }
        }
    }
}

string buildCargoDescription(const Cargo &item, BaseComputer &computer, float price) {
    const bool trackBestPrices = configuration().general.trade_interface_tracks_prices;

    string desc;

    if (trackBestPrices && computer.m_base.GetUnit() != NULL) {
        int cp = _Universe->whichPlayerStarship(computer.m_player.GetUnit());
        vector<string> highest, lowest;

        const string &baseName = (computer.m_base.GetUnit()->getUnitType() == Vega_UnitType::planet) ?
                computer.m_base.GetUnit()->name.get()
                : computer.m_base
                        .GetUnit()
                        ->getFullname();

        trackPrice(cp, item, price, UniverseUtil::getSystemName(), baseName, highest, lowest);

        if (highest.size()) {
            desc += "#n##n##b#Highest prices seen#-b#:";
            for (vector<string>::const_iterator i = highest.begin(); i != highest.end(); ++i) {
                desc += *i;
            }
        }

        if (lowest.size()) {
            desc += "#n##n##b#Lowest prices seen#-b#:";
            for (vector<string>::const_iterator i = lowest.begin(); i != lowest.end(); ++i) {
                desc += *i;
            }
        }
    }

    return desc;
}

//Load the controls for the SHIP_DEALER display.
void BaseComputer::loadShipDealerControls(void) {
    //Make sure there's nothing in the transaction lists.
    resetTransactionLists();

    //Set up the base dealer's transaction list.
    std::vector<std::string> filtervec;
    filtervec.push_back("starships");
    loadMasterList(m_base.GetUnit(), m_base.GetUnit()->cargo_hold, filtervec, std::vector<std::string>(), true, m_transList1);

    //Add in the starships owned by this player.
    Cockpit *cockpit = _Universe->AccessCockpit();
    for (size_t i = 1, n = cockpit->GetNumUnits(); i < n; ++i) {
        CargoColor cargoColor;
        cargoColor.cargo = CreateCargoForOwnerStarship(cockpit, m_base.GetUnit(), i);
        m_transList1.masterList.push_back(cargoColor);
    }
    //remove the descriptions, we don't build them all here, it is a time consuming operation
    vector<CargoColor> *items = &m_transList1.masterList;
    for (vector<CargoColor>::iterator it = items->begin(); it != items->end(); it++) {
        (*it).cargo.SetDescription("");
    }
    //Load the picker from the master list.
    SimplePicker *basePicker = vega_dynamic_cast_ptr<SimplePicker>(window()->findControlById("Ships"));
    assert(basePicker != NULL);
    loadListPicker(m_transList1, *basePicker, BUY_SHIP, true);

    //Make the title right.
    recalcTitle();
}

bool sellShip(Unit *baseUnit, Unit *playerUnit, std::string shipname, BaseComputer *bcomputer) {
    Cockpit *cockpit = _Universe->isPlayerStarship(playerUnit);
    unsigned int tempInt = 1;

    Cargo shipCargo;
    try {
        shipCargo = Manifest::MPL().GetCargoByName(shipname);
    } catch (const std::exception& e) {
        VS_LOG(error, (boost::format("Error in GetCargoByName: %1%") % e.what()));
        return false;
    }


    //now we can actually do the selling
    for (size_t i = 1, n = cockpit->GetNumUnits(); i < n; ++i) {
        if (cockpit->GetUnitFileName(i) == shipname) {
            if (cockpit->GetUnitSystemName(i) == _Universe->activeStarSystem()->getFileName()) {
                const float shipping_price = configuration().economics.sellback_shipping_price_flt;
                ComponentsManager::credits -= shipping_price; //transportation cost
            }
            cockpit->RemoveUnit(i);
            const float shipSellback = configuration().economics.ship_sellback_price_flt;
            ComponentsManager::credits += shipSellback * shipCargo.GetPrice(); //sellback cost
            break;
        }
    }

    if (bcomputer) {
        bcomputer->loadShipDealerControls();
        bcomputer->updateTransactionControlsForSelection(NULL);
    }
    return true;
}

bool BaseComputer::sellShip(const EventCommandId &command, Control *control) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    Cargo *item = selectedItem();
    
    if (!(playerUnit && baseUnit && item && playerUnit->IsPlayerShip())) {
        return true;
    }
    return ::sellShip(baseUnit, playerUnit, item->GetName(), this);
}

bool buyShip(Unit *baseUnit,
        Unit *playerUnit,
        std::string content,
        bool myfleet,
        bool force_base_inventory,
        BaseComputer *bcomputer) {
    Cargo ship_cargo;

    int index = baseUnit->cargo_hold.GetIndex(content);
    if(index > -1) {
        ship_cargo = baseUnit->cargo_hold.GetCargo(index);
    } else {
        try {
            ship_cargo = Manifest::MPL().GetCargoByName(content);
        } catch (const std::exception& e) {
            VS_LOG(error, (boost::format("Error in GetCargoByName: %1%") % e.what()));
            return false;
        }
    }

    Cargo *shipCargo = &ship_cargo;
    Cargo myFleetShipCargo;
    int swappingShipsIndex = -1;
    if (myfleet) {
        //Player owns this starship.
        shipCargo = &myFleetShipCargo;
        myFleetShipCargo =
                CreateCargoForOwnerStarshipName(_Universe->AccessCockpit(), baseUnit, content, swappingShipsIndex);
        if (shipCargo->GetName().empty()) {
            //Something happened -- can't find ship by name.
            shipCargo = NULL;
            swappingShipsIndex = -1;
        }
    } else {
        // This is where we solve the bug where you can't own two ships of the same type
        Cockpit *cockpit = _Universe->AccessCockpit();
        for (size_t i = 1, n = cockpit->GetNumUnits(); i < n; ++i) {
            if (cockpit->GetUnitFileName(i) == content) {
                return false;
            }
        }
        //can't buy a ship you own
    }
    if (shipCargo) {
        if (shipCargo->GetPrice() < ComponentsManager::credits) {

            Flightgroup *flightGroup = playerUnit->getFlightgroup();
            int fgsNumber = 0;
            if (flightGroup != NULL) {
                fgsNumber = flightGroup->nr_ships;
                flightGroup->nr_ships++;
                flightGroup->nr_ships_left++;
            }
            string newModifications;
            std::string tmpnam = CurrentSaveGameName;
            if (swappingShipsIndex != -1) {
                //if we're swapping not buying load the olde one
                newModifications = _Universe->AccessCockpit()->GetUnitModifications();
                CurrentSaveGameName = "~" + CurrentSaveGameName;
            }
            WriteSaveGame(_Universe->AccessCockpit(), true);             //oops saved game last time at wrong place
            UniverseUtil::StopAllSounds();
            UniverseUtil::playSound("sales/salespitch" + content.substr(0, content.find(".")) + "accept.wav", QVector(0,
                            0,
                            0),
                    Vector(0, 0, 0));
            Unit *newPart =
                    new Unit(content.c_str(),
                            false,
                            baseUnit->faction,
                            newModifications,
                            flightGroup,
                            fgsNumber);
            CurrentSaveGameName = tmpnam;
            newPart->SetFaction(playerUnit->faction);
            if (newPart->name != LOAD_FAILED) {
                if (newPart->nummesh() > 0) {
                    ComponentsManager::credits -= shipCargo->GetPrice();
                    newPart->curr_physical_state = playerUnit->curr_physical_state;
                    newPart->SetPosAndCumPos(UniverseUtil::SafeEntrancePoint(playerUnit->Position(), newPart->rSize()));
                    newPart->prev_physical_state = playerUnit->prev_physical_state;
                    _Universe->activeStarSystem()->AddUnit(newPart);
                    SwapInNewShipName(_Universe->AccessCockpit(), baseUnit, content, swappingShipsIndex);
                    for (int j = 0; j < 2; ++j) {
                        for (int i = playerUnit->cargo_hold.Size() - 1; i >= 0; --i) {
                            Cargo c = playerUnit->cargo_hold.GetCargo(i);
                            if ((c.IsMissionFlag() != 0 && j == 0)
                                    || (c.IsMissionFlag() == 0 && j == 1 && (!myfleet)
                                            && c.IsComponent())) {
                                for (int k = c.GetQuantity(); k > 0; --k) {
                                    c.SetQuantity(k);
                                    if (newPart->cargo_hold.CanAddCargo(c)) {
                                        newPart->cargo_hold.AddCargo(newPart, c);
                                        playerUnit->cargo_hold.RemoveCargo(playerUnit, i, c.GetQuantity());
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    WriteSaveGame(_Universe->AccessCockpit(),
                            true);                     //oops saved game last time at wrong place

                    _Universe->AccessCockpit()->SetParent(newPart, content.c_str(),
                            _Universe->AccessCockpit()->GetUnitModifications().c_str(),
                            playerUnit->curr_physical_state
                                    .position);                     //absolutely NO NO NO modifications...you got this baby clean off the slate

                    //We now put the player in space.
                    SwitchUnits(NULL, newPart);
                    playerUnit->UnDock(baseUnit);
                    if (bcomputer) {
                        bcomputer->m_player.SetUnit(newPart);
                    }
                    WriteSaveGame(_Universe->AccessCockpit(), true);
                    if (baseUnit) {
                        newPart->ForceDock(baseUnit, 0);
                    }
                    CurrentBaseUnitSet(newPart);
                    if (bcomputer) {
                        bcomputer->m_player.SetUnit(newPart);
                    }
                    const bool persistent_missions_across_ship_switch = configuration().general.persistent_mission_across_ship_switch;
                    if (persistent_missions_across_ship_switch) {
                        _Universe->AccessCockpit()->savegame->LoadSavedMissions();
                    }
                    newPart = NULL;
                    playerUnit->Kill();
                    if (bcomputer) {
                        bcomputer->window()->close();
                    }
                    return true;
                }
            }
            newPart->Kill();
            newPart = NULL;
        }
    }
    return false;
}

//Buy ship from the base.
bool BaseComputer::buyShip(const EventCommandId &command, Control *control) {
    Unit *playerUnit = m_player.GetUnit();
    Unit *baseUnit = m_base.GetUnit();
    Cargo *item = selectedItem();
    if (!(playerUnit && baseUnit && item)) {
        return true;
    }
    ::buyShip(baseUnit,
            playerUnit,
            item->GetName(),
            item->GetCategory().find("My_Fleet") != string::npos,
            true,
            this);     //last was false BUCO
    return true;
}

//Change controls to INFO mode.
bool BaseComputer::changeToInfoMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != INFO) {
        switchToControls(INFO);
        //Initialize description with player info.
        window()->sendCommand("ShowPlayerInfo", NULL);
        recalcTitle();
    }
    return true;
}

//Faction colors 2-Sept-03.  mbyron.
/*
 *  0. r=0.5 g=0.5 b=1
 *  1. r=0 g=0 b=1
 *  2. r=0 g=1 b=0
 *  3. r=0.5 g=0.5 b=1
 *  4. r=0.75 g=0.5 b=0.25
 *  5. r=0 g=0.5 b=1
 *  6. r=0.5 g=0 b=1
 *  7. r=0.5 g=0.5 b=1
 *  8. r=0.5 g=0.5 b=1
 *  9. r=1 g=0.5 b=0
 *  10. r=0.4 g=0.2 b=0.7
 *  11. r=1 g=1 b=1
 *  12. r=0.5 g=0.5 b=1
 *  13. r=1 g=0 b=0
 *  14. r=0.5 g=0.5 b=1
 */

//Given a faction number, return a PaintText color command for the faction.
//This lightens up the faction colors to make them more easily seen on the dark background.
static std::string factionColorTextString(int faction) {
    //The following gets the spark (faction) color.
    const float *spark = FactionUtil::GetSparkColor(faction);

    //Brighten up the raw colors by multiplying each channel by 2/3, then adding back 1/3.
    //The darker colors are too hard to read.
    std::string result =
            colorsToCommandString(spark[0] / 1.5 + 1.0 / 3, spark[1] / 1.5 + 1.0 / 3, spark[2] / 1.5 + 1.0 / 3);

    return result;
}


// A utility to convert vector to list
boost::python::list VectorToList(const std::vector<std::string> v) {
    boost::python::list l;
    for (const std::string& value : v) {
        l.append(value);
    }

    return l;
}

//Show the player's basic information.
bool BaseComputer::showPlayerInfo(const EventCommandId &command, Control *control) {
    //Number of kills for each faction.
    vector<float> *kill_list = &_Universe->AccessCockpit()->savegame->getMissionData(string("kills"));

    const std::vector<std::string> names_vector = FactionUtil::GetFactionNames();
    const std::vector<std::string> relations_vector = FactionUtil::GetFactionRelations();
    const std::vector<std::string> kills_vector = FactionUtil::GetFactionKills(kill_list);

    boost::python::list names_list = VectorToList(names_vector);
    boost::python::list relations_list = VectorToList(relations_vector);
    boost::python::list kills_list = VectorToList(kills_vector);

    PyObject* args = PyTuple_Pack(3, names_list.ptr(), relations_list.ptr(), kills_list.ptr());

    const std::string text = GetString("get_player_info", "player_info",
        "player_info.py", args);

    //Put this in the description.
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    assert(desc != NULL);

    desc->setText(text);

    return true;
}

//does not work with negative numbers!!
void prettyPrintFloat(char *buffer, float f, int digitsBefore, int digitsAfter, int bufferLen) {
    int bufferPos = 0;
    if (!FINITE(f) || ISNAN(f)) {
        buffer[0] = 'n';
        buffer[1] = '/';
        buffer[2] = 'a';
        buffer[3] = '\0';
        return;
    }
    if (f < 0) {
        buffer[0] = '-';
        bufferPos = 1;
        f = (-f);
    }
    float temp = f;
    int before = 0;
    while (temp >= 1.0f) {
        before++;
        temp /= 10.0f;
    }
    while (bufferPos < (bufferLen - 4 - digitsAfter) && before < digitsBefore) {
        buffer[bufferPos++] = '0';
        digitsBefore--;
    }
    if (before) {
        for (int p = before; bufferPos < (bufferLen - 4 - digitsAfter) && p > 0; p--) {
            temp = f;
            float substractor = 1;
            for (int i = 0; i < p - 1; i++) {
                temp /= 10.0f;
                substractor *= 10.0;
            }                                                                          //whe cant't cast to int before in case of overflow
            int digit = ((int) temp) % 10;
            buffer[bufferPos++] = '0' + digit;
            //reason for the folowing line: otherwise the  "((int)temp)%10" may overflow when converting
            f = f - ((float) digit * substractor);
            if ((p != 1) && (p % 3 == 1)) {
                buffer[bufferPos++] = ' ';
            } // thousand separator
        }
    } else {
        buffer[bufferPos++] = '0';
    }
    if (digitsAfter == 0) {
        buffer[bufferPos] = 0;
        return;
    }

    if (bufferPos < bufferLen) {
        buffer[bufferPos++] = '.';
    }

    temp = f;
    for (int i = 0; bufferPos < (bufferLen - 1) && i < digitsAfter; i++) {
        temp *= 10;
        buffer[bufferPos++] = '0' + (((int) temp) % 10);
    }
    if (bufferPos < bufferLen) {
        buffer[bufferPos] = 0;
    }
}

static const char *WeaponTypeStrings[] = {
        "UNKNOWN",
        "BEAM",
        "BALL",
        "BOLT",
        "PROJECTILE"
};



//Show the stats on the player's current ship.
bool BaseComputer::showShipStats(const EventCommandId &command, Control *control) {
    current_unit_load_mode = NO_MESH;
    Unit *playerUnit = m_player.GetUnit();
    current_unit_load_mode = DEFAULT;
    const string rawText = MakeUnitXMLPretty(playerUnit->WriteUnitString(), playerUnit);

    Cargo uninitcargo;

    std::map<std::string, std::string> ship_map = playerUnit->UnitToMap();
    std::string text = GetString("get_ship_description", "ship_view",
                          "ship_view.py",
                          ship_map);

    //remove picture, if any
    string::size_type pic;
    if ((pic = text.find("@")) != string::npos) {
        std::string texture = text.substr(pic + 1);
        text = text.substr(0, pic);
        string::size_type picend = texture.find("@");
        if (picend != string::npos) {
            text += texture.substr(picend + 1);
        }
    }     //picture removed

    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("Description"));
    assert(desc != NULL);
    desc->setText(text);

    return true;
}

namespace CockpitKeys {
void QuitNow();
}

//Create the window and controls for the Options Menu.
void BaseComputer::LoadSaveQuitConfirm::init(void) {
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
    text->setText(this->text);
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
    cont->setLabel(type);
    cont->setCommand(type);
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
bool BaseComputer::LoadSaveQuitConfirm::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Save") {
        m_parent->actionConfirmedSaveGame();
        window()->close();
    } else if (command == "Load") {
        m_parent->actionConfirmedLoadGame();
    } else if (command == "Quit") {
        m_parent->actionConfirmedQuitGame();
    } else {
        //Not a command we know about.
        return WindowController::processWindowCommand(command, control);
    }
    return true;
}

//Show options.

bool BaseComputer::actionConfirmedQuitGame() {
    CockpitKeys::QuitNow();
    return true;
}

bool BaseComputer::actionQuitGame(const EventCommandId &command, Control *control) {
    LoadSaveQuitConfirm *saver = new LoadSaveQuitConfirm(this, "Quit", "Are you sure that you want to quit?");
    saver->init();
    saver->run();
    return true;
}

bool BaseComputer::actionConfirmedSaveGame() {
    Unit *player = m_player.GetUnit();
    if (player && player->name == "return_to_cockpit") {
        showAlert("Return to a base to save.");
        return false;         //should be false, but causes badness.
    }
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    if (desc) {
        std::string tmp = desc->text();
        VSFileSystem::VSFile fp;
        VSFileSystem::VSError err = fp.OpenCreateWrite(tmp, SaveFile);
        if (err > VSFileSystem::Ok) {
            showAlert(
                    "Could not create the saved game because it contains invalid characters or you do not have permissions or free space.");
        } else {
            fp.Close();
            if (tmp.length() > 0) {
                Cockpit *cockpit = player ? _Universe->isPlayerStarship(player) : 0;
                if (player && cockpit) {
                    UniverseUtil::setCurrentSaveGame(tmp);
                    WriteSaveGame(cockpit, false);
                    loadLoadSaveControls();
                    showAlert("Game saved successfully.");
                } else {
                    showAlert("Oops - unexpected error (player or cockpit is null)");
                }
            } else {
                showAlert("You Must Type In a Name To Save.");
            }
        }
    } else {
        showAlert("Oops - unexpected error (desc control not found!)");
    }
    return true;
}

bool BaseComputer::actionSaveGame(const EventCommandId &command, Control *control) {
    Unit *player = m_player.GetUnit();
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    bool ok = true;
    std::string tmp;
    if (desc) {
        tmp = desc->text();
        if (tmp.length() <= 0) {
            ok = false;
        }
    }
    if (player && ok) {
        if (player->IsPlayerShip()) {
            VSFileSystem::VSFile fp;
            VSFileSystem::VSError err = fp.OpenReadOnly(tmp, SaveFile);
            if (err > VSFileSystem::Ok) {
                actionConfirmedSaveGame();
            } else {
                fp.Close();
                if (string("New_Game") != tmp) {
                    LoadSaveQuitConfirm *saver = new LoadSaveQuitConfirm(this,
                            "Save",
                            "Do you want to overwrite your old saved game?");
                    saver->init();
                    saver->run();
                } else {
                    showAlert("You may not save to the name New_Game.");
                }
            }
        }
    }
    if (!ok) {
        showAlert("You Must Type In a Name To Save.");
    }
    return true;
}

bool BaseComputer::actionConfirmedLoadGame() {
    Unit *player = m_player.GetUnit();
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    if (desc) {
        std::string tmp = desc->text();
        if (tmp.length() > 0) {
            if ((string("New_Game") != tmp) && (!isUtf8SaveGame(tmp))) {
                showAlert(tmp + " is not UTF-8, convert it before loading");
                return true;
            }
            Cockpit *cockpit = player ? _Universe->isPlayerStarship(player) : 0;
            if (player && cockpit) {
                UniverseUtil::showSplashScreen("");
                UniverseUtil::showSplashMessage("Loading saved game.");
                UniverseUtil::setCurrentSaveGame(tmp);
                player->Kill();
                RespawnNow(cockpit);
                globalWindowManager().shutDown();
                TerminateCurrentBase();                  //BaseInterface::CurrentBase->Terminate();
            } else {
                showAlert("Oops - unexpected error (player or cockpit is null)");
            }
        } else {
            showAlert("You Must Type In a Name To Load....");
        }
    } else {
        showAlert("Oops - unexpected error (desc control not found!)");
    }
    return true;
}

bool BaseComputer::actionNewGame(const EventCommandId &command, Control *control) {
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    desc->setText(UniverseUtil::getNewGameSaveName());
    return this->actionLoadGame(command, control);
}

bool BaseComputer::actionLoadGame(const EventCommandId &command, Control *control) {
    Unit *player = m_player.GetUnit();
    StaticDisplay *desc = vega_dynamic_cast_ptr<StaticDisplay>(window()->findControlById("InputText"));
    if (desc) {
        std::string tmp = desc->text();
        if (tmp.length() > 0) {
            if (player && player->IsPlayerShip()) {
                LoadSaveQuitConfirm *saver = new LoadSaveQuitConfirm(this,
                        "Load",
                        "Are you sure that you want to load this game?");
                saver->init();
                saver->run();
                return true;
            }
        }
    }
    showAlert("You Must Type In a Name To Load....");
    return true;
}


//Process a command event from the Options Menu window.
