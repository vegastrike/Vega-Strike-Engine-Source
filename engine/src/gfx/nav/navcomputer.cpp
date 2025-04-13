/*
 * navcomputer.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Mike Byron specifically
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


#include "vegastrike.h"
#if defined (_WIN32) && !defined (__CYGWIN__) && !defined (__MINGW32__)
//For WIN32 debugging.
#include <crtdbg.h>
#endif

#include "navscreen.h"
#include "navpath.h"
#include "in_kb.h"
#include "in_kb_data.h"
#include "in_mouse.h"
#include "gfx/cockpit.h"
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

vector<unsigned int> nav_keyboard_queue;

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

static const ModeInfo displayModeInfo[] = {
        ModeInfo("List Mode", "Finished", "ListMode", "ListGroup"),
        ModeInfo("Edit Mode", "Edit", "EditMode", "EditGroup")
};

static const ModeInfo selectorModeInfo[] = {
        ModeInfo("Target Select", "System", "TargetSelectMode", "TargetSelectGroup"),
        ModeInfo("Criteria Select", "Criteria", "CriteriaSelectMode", "CriteriaSelectGroup"),
        ModeInfo("Chain Select", "Chain", "ChainSelectMode", "ChainSelectGroup")
};

//Dispatch table for commands.
//Make an entry here for each command you want to handle.
//WARNING:  The order of this table is important.  There are multiple entries for
//some commands. Basically, you can make an entry for a particular control, and then
//later have an entry with an empty control id to cover the other cases.
const NavComputer::WctlTableEntry NavComputer::WctlCommandTable[] = {
        NavComputer::WctlTableEntry("Visible",
                "",
                &NavComputer::toggleVisibility),
        NavComputer::WctlTableEntry("Picker::NewSelection",
                "PathLister",
                &NavComputer::pathListerChangedSelection),
        NavComputer::WctlTableEntry(displayModeInfo[LIST].command, "", &NavComputer::changeToListMode),
        NavComputer::WctlTableEntry(displayModeInfo[EDIT].command, "", &NavComputer::changeToEditMode),
        NavComputer::WctlTableEntry(selectorModeInfo[TARGET].command, "", &NavComputer::changeToTargetMode),
        NavComputer::WctlTableEntry(selectorModeInfo[CRITERIA].command, "", &NavComputer::changeToCriteriaMode),
        NavComputer::WctlTableEntry(selectorModeInfo[CHAIN].command, "", &NavComputer::changeToChainMode),
        NavComputer::WctlTableEntry("Add", "", &NavComputer::actionAdd),
        NavComputer::WctlTableEntry("ShowPath", "", &NavComputer::actionShowPath),
        NavComputer::WctlTableEntry("Rename", "", &NavComputer::actionRename),
        NavComputer::WctlTableEntry("Remove", "", &NavComputer::actionRemove),
        NavComputer::WctlTableEntry("ShowAll", "", &NavComputer::actionShowAll),
        NavComputer::WctlTableEntry("ShowNone", "", &NavComputer::actionShowNone),
        NavComputer::WctlTableEntry("Source", "", &NavComputer::actionSource),
        NavComputer::WctlTableEntry("Destination", "", &NavComputer::actionDestination),
        NavComputer::WctlTableEntry("Target", "", &NavComputer::actionTarget),
        NavComputer::WctlTableEntry("Current", "", &NavComputer::actionCurrent),
        NavComputer::WctlTableEntry("Absolute", "", &NavComputer::actionAbsolute),
        NavComputer::WctlTableEntry("And", "", &NavComputer::actionAnd),
        NavComputer::WctlTableEntry("Or", "", &NavComputer::actionOr),
        NavComputer::WctlTableEntry("Not", "", &NavComputer::actionNot),
        NavComputer::WctlTableEntry("RemoveCriteria", "", &NavComputer::actionRemoveCriteria),
        NavComputer::WctlTableEntry("Chain", "", &NavComputer::actionChain),
        NavComputer::WctlTableEntry("", "", NULL)
};

//Process a command from the window.
//This just dispatches to a handler.
bool NavComputer::processWindowCommand(const EventCommandId &command, Control *control) {
    //Iterate through the dispatch table.
    for (const WctlTableEntry *p = &WctlCommandTable[0]; p->function; p++) {
        if (p->command == command) {
            if (p->controlId.size() == 0 || p->controlId == control->id()) {
                //Found a handler for the command.
                return (this->*(p->function))(command, control);
            }
        }
    }
    //Let the base class have a try at the command first.
    if (WindowController::processWindowCommand(command, control)) {
        return true;
    }
    //Didn't find a handler.
    return false;
}

//CONSTRUCTOR.
NavComputer::NavComputer(NavigationSystem *navsystem) :
        navsys(navsystem), m_currentDisplay(NULL_DISPLAY), m_currentSelector(NULL_SELECTOR) {
    int i;
    pathman = navsystem->pathman;
    currentPath = NULL;
    currentNode = NULL;
    criteria = false;
    //Initialize display mode group controls array.
    for (i = 0; i < DISPLAY_MODE_COUNT; i++) {
        m_displayModeGroups[i] = NULL;
    }
    //Initialize selector mode group controls array.
    for (i = 0; i < SELECTOR_MODE_COUNT; i++) {
        m_selectorModeGroups[i] = NULL;
    }
    m_displayModes.push_back(LIST);
    m_displayModes.push_back(EDIT);

    m_selectorModes.push_back(TARGET);
    m_selectorModes.push_back(CRITERIA);
    m_selectorModes.push_back(CHAIN);

    m_visible = false;
}

//Destructor.
NavComputer::~NavComputer(void) {
    //Delete any group controls that the window doesn't "own".
    for (int i = 0; i < DISPLAY_MODE_COUNT; i++) {
        if (m_displayModeGroups[i] != nullptr) {
            delete m_displayModeGroups[i];
            m_displayModeGroups[i] = nullptr;
        }
    }
    for (int i = 0; i < SELECTOR_MODE_COUNT; i++) {
        if (m_selectorModeGroups[i] != nullptr) {
            delete m_selectorModeGroups[i];
            m_selectorModeGroups[i] = nullptr;
        }
    }
}

//Set up the window and get everything ready.
void NavComputer::init(void) {
    //Create a new window.
    Window *w = new Window;
    setWindow(w);

    m_window->setDeleteOnClose(false);
    m_window->setController(this);
    m_deleteOnWindowClose = false;

    //Read in the controls for all the modes.
    createControls();

    NavPath *path = new NavPath();
    path->setName("Target Search");
    path->setSourceNode(new CurrentPathNode());
    path->setDestinationNode(new TargetPathNode());
    pathman->paths.push_back(path);
}

//Create the controls that will be used for this window.
void NavComputer::createControls(void) {
    int i;
    //Set up the window.
    window()->setFullScreen();
    window()->setColor(GUI_CLEAR);
    window()->setTexture("basecomputer.png");

    //Put all the controls in the window.
    constructControls();
    //Take the mode group controls out of the window.
    for (i = 0; i < DISPLAY_MODE_COUNT; i++) {
        Control *group = window()->findControlById(displayModeInfo[i].groupId);
        if (group) {
            window()->removeControlFromWindow(group);
            m_displayModeGroups[i] = group;
        }
    }
    //Take the mode group controls out of the window.
    for (i = 0; i < SELECTOR_MODE_COUNT; i++) {
        Control *group = window()->findControlById(selectorModeInfo[i].groupId);
        if (group) {
            window()->removeControlFromWindow(group);
            m_selectorModeGroups[i] = group;
        }
    }
}

GFXColor NavComputer::getColorForGroup(std::string id) {
    if (id == "MainGroup") {
        return GFXColor(0, 1, 1);
    } else if (id == "ListGroup") {
        return GFXColor(0, 1, 1);
    } else if (id == "EditGroup") {
        return GFXColor(0, 1, 1);
    } else {
        return GFXColor(0, 1, 1);
    }
}

//Hack that constructs controls in code.
void NavComputer::constructControls(void) {
    {
        GroupControl *mainGroup = new GroupControl;
        mainGroup->setId("MainGroup");
        window()->addControl(mainGroup);
        GFXColor color = getColorForGroup("MainGroup");

        //Base info title.
        StaticDisplay *baseTitle = new StaticDisplay;
        baseTitle->setRect(Rect(-.96, .76, 1.9, .08));
        baseTitle->setText("Error");
        baseTitle->setTextColor(GFXColor(.4, 1, .4));
        baseTitle->setColor(GUI_CLEAR);
        baseTitle->setFont(Font(.07, 1.75));
        baseTitle->setId("NavigationTitle");
        mainGroup->addChild(baseTitle);

        //Done button.
        NewButton *done = new NewButton;
        done->setRect(Rect(.74, .74, .22, .12));
        done->setLabel("Done");
        done->setCommand("Visible");
        done->setColor(GFXColor(0, 1, 1, .25));
        done->setTextColor(GUI_OPAQUE_WHITE());
        done->setDownColor(GFXColor(0, 1, 1, .6));
        done->setDownTextColor(GUI_OPAQUE_BLACK());
        done->setHighlightColor(GFXColor(0, 0, 1, .4));
        done->setFont(Font(.08, BOLD_STROKE));
        mainGroup->addChild(done);

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.29, .05, .99));
        descScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(.24, -.29, .67, .99));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.07));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("Description");
        ms->setScroller(descScroller);
        mainGroup->addChild(ms);

        mainGroup->addChild(descScroller);         //Want scroller "over" description box.
    }

    {
        GroupControl *listGroup = new GroupControl;
        listGroup->setId(displayModeInfo[LIST].groupId);
        window()->addControl(listGroup);
        GFXColor color = getColorForGroup(displayModeInfo[LIST].groupId);

        //Scroller for picker.
        Scroller *pathScroller = new Scroller;
        pathScroller->setRect(Rect(-.50, -.95, .05, 1.65));
        pathScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        pathScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        pathScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        pathScroller->setTextColor(GUI_OPAQUE_WHITE());
        pathScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Save game picker.
        SimplePicker *pathList = new SimplePicker;
        pathList->setRect(Rect(-.96, -.95, .46, 1.65));
        pathList->setColor(GFXColor(color.r, color.g, color.b, .1));
        pathList->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        pathList->setTextColor(GUI_OPAQUE_WHITE());
        pathList->setSelectionColor(GFXColor(0, .6, 0, .8));
        pathList->setHighlightColor(GFXColor(0, .6, 0, .35));
        pathList->setHighlightTextColor(GUI_OPAQUE_WHITE());
        pathList->setFont(Font(.07));
        pathList->setTextMargins(Size(0.02, 0.01));
        pathList->setId("PathLister");
        pathList->setScroller(pathScroller);
        listGroup->addChild(pathList);

        listGroup->addChild(pathScroller);         //Want scroller "over" picker.

        NewButton *up = new NewButton;
        up->setRect(Rect(-.41, .58, .22, .12));
        up->setColor(GFXColor(0, 1, 1, .1));
        up->setTextColor(GUI_OPAQUE_WHITE());
        up->setDownColor(GFXColor(0, 1, 1, .4));
        up->setDownTextColor(GFXColor(.2, .2, .2));
        up->setVariableBorderCycleTime(1.0);
        up->setBorderColor(GFXColor(.2, .2, .2));
        up->setEndBorderColor(GFXColor(.4, .4, .4));
        up->setShadowWidth(2.0);
        up->setFont(Font(.08, BOLD_STROKE));
        up->setId("Up");
        up->setLabel("Up");
        up->setCommand("Up");
        listGroup->addChild(up);

        NewButton *add = new NewButton;
        add->setRect(Rect(-.41, .33, .22, .12));
        add->setColor(GFXColor(0, 1, 1, .1));
        add->setTextColor(GUI_OPAQUE_WHITE());
        add->setDownColor(GFXColor(0, 1, 1, .4));
        add->setDownTextColor(GFXColor(.2, .2, .2));
        add->setVariableBorderCycleTime(1.0);
        add->setBorderColor(GFXColor(.2, .2, .2));
        add->setEndBorderColor(GFXColor(.4, .4, .4));
        add->setShadowWidth(2.0);
        add->setFont(Font(.08, BOLD_STROKE));
        add->setId("Add");
        add->setLabel("Add");
        add->setCommand("Add");
        listGroup->addChild(add);

        NewButton *edit = new NewButton;
        edit->setRect(Rect(-.41, .07, .22, .12));
        edit->setColor(GFXColor(0, 1, 1, .1));
        edit->setTextColor(GUI_OPAQUE_WHITE());
        edit->setDownColor(GFXColor(0, 1, 1, .4));
        edit->setDownTextColor(GFXColor(.2, .2, .2));
        edit->setVariableBorderCycleTime(1.0);
        edit->setBorderColor(GFXColor(.2, .2, .2));
        edit->setEndBorderColor(GFXColor(.4, .4, .4));
        edit->setShadowWidth(2.0);
        edit->setFont(Font(.08, BOLD_STROKE));
        edit->setId(displayModeInfo[EDIT].button);
        edit->setLabel(displayModeInfo[EDIT].button);
        edit->setCommand(displayModeInfo[EDIT].command);
        listGroup->addChild(edit);

        NewButton *showpath = new NewButton;
        showpath->setRect(Rect(-.41, -.18, .22, .12));
        showpath->setColor(GFXColor(0, 1, 1, .1));
        showpath->setTextColor(GUI_OPAQUE_WHITE());
        showpath->setDownColor(GFXColor(0, 1, 1, .4));
        showpath->setDownTextColor(GFXColor(.2, .2, .2));
        showpath->setVariableBorderCycleTime(1.0);
        showpath->setBorderColor(GFXColor(.2, .2, .2));
        showpath->setEndBorderColor(GFXColor(.4, .4, .4));
        showpath->setShadowWidth(2.0);
        showpath->setFont(Font(.08, BOLD_STROKE));
        showpath->setId("ShowPath");
        showpath->setLabel("Show");
        showpath->setCommand("ShowPath");
        listGroup->addChild(showpath);

        NewButton *rename = new NewButton;
        rename->setRect(Rect(-.41, -.44, .22, .12));
        rename->setColor(GFXColor(0, 1, 1, .1));
        rename->setTextColor(GUI_OPAQUE_WHITE());
        rename->setDownColor(GFXColor(0, 1, 1, .4));
        rename->setDownTextColor(GFXColor(.2, .2, .2));
        rename->setVariableBorderCycleTime(1.0);
        rename->setBorderColor(GFXColor(.2, .2, .2));
        rename->setEndBorderColor(GFXColor(.4, .4, .4));
        rename->setShadowWidth(2.0);
        rename->setFont(Font(.08, BOLD_STROKE));
        rename->setId("Rename");
        rename->setLabel("Rename");
        rename->setCommand("Rename");
        listGroup->addChild(rename);

        NewButton *remove = new NewButton;
        remove->setRect(Rect(-.41, -.69, .22, .12));
        remove->setColor(GFXColor(0, 1, 1, .1));
        remove->setTextColor(GUI_OPAQUE_WHITE());
        remove->setDownColor(GFXColor(0, 1, 1, .4));
        remove->setDownTextColor(GFXColor(.2, .2, .2));
        remove->setVariableBorderCycleTime(1.0);
        remove->setBorderColor(GFXColor(.2, .2, .2));
        remove->setEndBorderColor(GFXColor(.4, .4, .4));
        remove->setShadowWidth(2.0);
        remove->setFont(Font(.08, BOLD_STROKE));
        remove->setId("Remove");
        remove->setLabel("Remove");
        remove->setCommand("Remove");
        listGroup->addChild(remove);

        NewButton *down = new NewButton;
        down->setRect(Rect(-.41, -.95, .22, .12));
        down->setColor(GFXColor(0, 1, 1, .1));
        down->setTextColor(GUI_OPAQUE_WHITE());
        down->setDownColor(GFXColor(0, 1, 1, .4));
        down->setDownTextColor(GFXColor(.2, .2, .2));
        down->setVariableBorderCycleTime(1.0);
        down->setBorderColor(GFXColor(.2, .2, .2));
        down->setEndBorderColor(GFXColor(.4, .4, .4));
        down->setShadowWidth(2.0);
        down->setFont(Font(.08, BOLD_STROKE));
        down->setId("Down");
        down->setLabel("Down");
        down->setCommand("Down");
        listGroup->addChild(down);

        NewButton *showall = new NewButton;
        showall->setRect(Rect(-.15, .58, .35, .12));
        showall->setColor(GFXColor(0, 1, 1, .1));
        showall->setTextColor(GUI_OPAQUE_WHITE());
        showall->setDownColor(GFXColor(0, 1, 1, .4));
        showall->setDownTextColor(GFXColor(.2, .2, .2));
        showall->setVariableBorderCycleTime(1.0);
        showall->setBorderColor(GFXColor(.2, .2, .2));
        showall->setEndBorderColor(GFXColor(.4, .4, .4));
        showall->setShadowWidth(2.0);
        showall->setFont(Font(.08, BOLD_STROKE));
        showall->setId("ShowAll");
        showall->setLabel("Show All");
        showall->setCommand("ShowAll");
        listGroup->addChild(showall);

        NewButton *shownone = new NewButton;
        shownone->setRect(Rect(-.15, .33, .35, .12));
        shownone->setColor(GFXColor(0, 1, 1, .1));
        shownone->setTextColor(GUI_OPAQUE_WHITE());
        shownone->setDownColor(GFXColor(0, 1, 1, .4));
        shownone->setDownTextColor(GFXColor(.2, .2, .2));
        shownone->setVariableBorderCycleTime(1.0);
        shownone->setBorderColor(GFXColor(.2, .2, .2));
        shownone->setEndBorderColor(GFXColor(.4, .4, .4));
        shownone->setShadowWidth(2.0);
        shownone->setFont(Font(.08, BOLD_STROKE));
        shownone->setId("ShowNone");
        shownone->setLabel("Show None");
        shownone->setCommand("ShowNone");
        listGroup->addChild(shownone);
    }

    {
        GroupControl *editGroup = new GroupControl;
        editGroup->setId(displayModeInfo[EDIT].groupId);
        window()->addControl(editGroup);
        GFXColor color = getColorForGroup(displayModeInfo[EDIT].groupId);

        //Apply hint title.
        StaticDisplay *applyHint = new StaticDisplay;
        applyHint->setRect(Rect(-.96, .56, .26, .20));
        applyHint->setText("Apply To:");
        applyHint->setTextColor(GFXColor(.4, 1, .4));
        applyHint->setColor(GUI_CLEAR);
        applyHint->setFont(Font(.07, 1.75));
        applyHint->setId("ApplyHint");
        editGroup->addChild(applyHint);

        NewButton *source = new NewButton;
        source->setRect(Rect(-.66, .50, .40, .20));
        source->setColor(GFXColor(0, 1, 1, .1));
        source->setTextColor(GUI_OPAQUE_WHITE());
        source->setDownColor(GFXColor(0, 1, 1, .4));
        source->setDownTextColor(GFXColor(.2, .2, .2));
        source->setVariableBorderCycleTime(1.0);
        source->setBorderColor(GFXColor(.2, .2, .2));
        source->setEndBorderColor(GFXColor(.4, .4, .4));
        source->setShadowWidth(2.0);
        source->setFont(Font(.08, BOLD_STROKE));
        source->setId("Source");
        source->setLabel("Source");
        source->setCommand("Source");
        editGroup->addChild(source);

        NewButton *destination = new NewButton;
        destination->setRect(Rect(-.20, .50, .40, .20));
        destination->setColor(GFXColor(0, 1, 1, .1));
        destination->setTextColor(GUI_OPAQUE_WHITE());
        destination->setDownColor(GFXColor(0, 1, 1, .4));
        destination->setDownTextColor(GFXColor(.2, .2, .2));
        destination->setVariableBorderCycleTime(1.0);
        destination->setBorderColor(GFXColor(.2, .2, .2));
        destination->setEndBorderColor(GFXColor(.4, .4, .4));
        destination->setShadowWidth(2.0);
        destination->setFont(Font(.08, BOLD_STROKE));
        destination->setId("Destination");
        destination->setLabel("Destination");
        destination->setCommand("Destination");
        editGroup->addChild(destination);

        NewButton *targetMode = new NewButton;
        targetMode->setRect(Rect(-.96, .26, .22, .12));
        targetMode->setColor(GFXColor(0, 1, 1, .1));
        targetMode->setTextColor(GUI_OPAQUE_WHITE());
        targetMode->setDownColor(GFXColor(0, 1, 1, .4));
        targetMode->setDownTextColor(GFXColor(.2, .2, .2));
        targetMode->setVariableBorderCycleTime(1.0);
        targetMode->setBorderColor(GFXColor(.2, .2, .2));
        targetMode->setEndBorderColor(GFXColor(.4, .4, .4));
        targetMode->setShadowWidth(2.0);
        targetMode->setFont(Font(.08, BOLD_STROKE));
        targetMode->setId(selectorModeInfo[TARGET].button);
        targetMode->setLabel(selectorModeInfo[TARGET].button);
        targetMode->setCommand(selectorModeInfo[TARGET].command);
        editGroup->addChild(targetMode);

        NewButton *stringMode = new NewButton;
        stringMode->setRect(Rect(-.96, -.04, .22, .12));
        stringMode->setColor(GFXColor(0, 1, 1, .1));
        stringMode->setTextColor(GUI_OPAQUE_WHITE());
        stringMode->setDownColor(GFXColor(0, 1, 1, .4));
        stringMode->setDownTextColor(GFXColor(.2, .2, .2));
        stringMode->setVariableBorderCycleTime(1.0);
        stringMode->setBorderColor(GFXColor(.2, .2, .2));
        stringMode->setEndBorderColor(GFXColor(.4, .4, .4));
        stringMode->setShadowWidth(2.0);
        stringMode->setFont(Font(.08, BOLD_STROKE));
        stringMode->setId(selectorModeInfo[CRITERIA].button);
        stringMode->setLabel(selectorModeInfo[CRITERIA].button);
        stringMode->setCommand(selectorModeInfo[CRITERIA].command);
        editGroup->addChild(stringMode);

        NewButton *chainMode = new NewButton;
        chainMode->setRect(Rect(-.96, -.34, .22, .12));
        chainMode->setColor(GFXColor(0, 1, 1, .1));
        chainMode->setTextColor(GUI_OPAQUE_WHITE());
        chainMode->setDownColor(GFXColor(0, 1, 1, .4));
        chainMode->setDownTextColor(GFXColor(.2, .2, .2));
        chainMode->setVariableBorderCycleTime(1.0);
        chainMode->setBorderColor(GFXColor(.2, .2, .2));
        chainMode->setEndBorderColor(GFXColor(.4, .4, .4));
        chainMode->setShadowWidth(2.0);
        chainMode->setFont(Font(.08, BOLD_STROKE));
        chainMode->setId(selectorModeInfo[CHAIN].button);
        chainMode->setLabel(selectorModeInfo[CHAIN].button);
        chainMode->setCommand(selectorModeInfo[CHAIN].command);
        editGroup->addChild(chainMode);

        NewButton *finished = new NewButton;
        finished->setRect(Rect(-.96, -.64, .22, .12));
        finished->setColor(GFXColor(0, 1, 1, .1));
        finished->setTextColor(GUI_OPAQUE_WHITE());
        finished->setDownColor(GFXColor(0, 1, 1, .4));
        finished->setDownTextColor(GFXColor(.2, .2, .2));
        finished->setVariableBorderCycleTime(1.0);
        finished->setBorderColor(GFXColor(.2, .2, .2));
        finished->setEndBorderColor(GFXColor(.4, .4, .4));
        finished->setShadowWidth(2.0);
        finished->setFont(Font(.08, BOLD_STROKE));
        finished->setId(displayModeInfo[LIST].button);
        finished->setLabel(displayModeInfo[LIST].button);
        finished->setCommand(displayModeInfo[LIST].command);
        editGroup->addChild(finished);

        //Scroller for description.
        Scroller *descScroller = new Scroller;
        descScroller->setRect(Rect(.91, -.95, .05, .62));
        descScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        descScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        descScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        descScroller->setTextColor(GUI_OPAQUE_WHITE());
        descScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Description box.
        StaticDisplay *ms = new StaticDisplay;
        ms->setRect(Rect(.24, -.95, .67, .62));
        ms->setColor(GFXColor(color.r, color.g, color.b, .1));
        ms->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        ms->setFont(Font(.07));
        ms->setMultiLine(true);
        ms->setTextColor(GUI_OPAQUE_WHITE());
        ms->setTextMargins(Size(.02, .01));
        ms->setId("NodeDescription");
        ms->setScroller(descScroller);
        editGroup->addChild(ms);

        editGroup->addChild(descScroller);         //Want scroller "over" description box.
    }

    {
        GroupControl *targetGroup = new GroupControl;
        targetGroup->setId(selectorModeInfo[TARGET].groupId);
        window()->addControl(targetGroup);

        NewButton *absolute = new NewButton;
        absolute->setRect(Rect(-.56, .26, .61, .12));
        absolute->setColor(GFXColor(0, 1, 1, .1));
        absolute->setTextColor(GUI_OPAQUE_WHITE());
        absolute->setDownColor(GFXColor(0, 1, 1, .4));
        absolute->setDownTextColor(GFXColor(.2, .2, .2));
        absolute->setVariableBorderCycleTime(1.0);
        absolute->setBorderColor(GFXColor(.2, .2, .2));
        absolute->setEndBorderColor(GFXColor(.4, .4, .4));
        absolute->setShadowWidth(2.0);
        absolute->setFont(Font(.08, BOLD_STROKE));
        absolute->setId("Absolute");
        absolute->setLabel("ERROR");
        absolute->setCommand("Absolute");
        targetGroup->addChild(absolute);

        NewButton *target = new NewButton;
        target->setRect(Rect(-.56, .10, .61, .12));
        target->setColor(GFXColor(0, 1, 1, .1));
        target->setTextColor(GUI_OPAQUE_WHITE());
        target->setDownColor(GFXColor(0, 1, 1, .4));
        target->setDownTextColor(GFXColor(.2, .2, .2));
        target->setVariableBorderCycleTime(1.0);
        target->setBorderColor(GFXColor(.2, .2, .2));
        target->setEndBorderColor(GFXColor(.4, .4, .4));
        target->setShadowWidth(2.0);
        target->setFont(Font(.08, BOLD_STROKE));
        target->setId("Target");
        target->setLabel("Target");
        target->setCommand("Target");
        targetGroup->addChild(target);

        NewButton *current = new NewButton;
        current->setRect(Rect(-.56, -.06, .61, .12));
        current->setColor(GFXColor(0, 1, 1, .1));
        current->setTextColor(GUI_OPAQUE_WHITE());
        current->setDownColor(GFXColor(0, 1, 1, .4));
        current->setDownTextColor(GFXColor(.2, .2, .2));
        current->setVariableBorderCycleTime(1.0);
        current->setBorderColor(GFXColor(.2, .2, .2));
        current->setEndBorderColor(GFXColor(.4, .4, .4));
        current->setShadowWidth(2.0);
        current->setFont(Font(.08, BOLD_STROKE));
        current->setId("Current");
        current->setLabel("Current");
        current->setCommand("Current");
        targetGroup->addChild(current);
    }

    {
        GroupControl *criteriaGroup = new GroupControl;
        criteriaGroup->setId(selectorModeInfo[CRITERIA].groupId);
        window()->addControl(criteriaGroup);
        GFXColor color = getColorForGroup(selectorModeInfo[CRITERIA].groupId);

        //Scroller for picker.
        Scroller *parameterScroller = new Scroller;
        parameterScroller->setRect(Rect(.15, .15, .05, .23));
        parameterScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        parameterScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        parameterScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        parameterScroller->setTextColor(GUI_OPAQUE_WHITE());
        parameterScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Parameter picker.
        SimplePicker *parameterList = new SimplePicker;
        parameterList->setRect(Rect(-.66, .15, .81, .23));
        parameterList->setColor(GFXColor(color.r, color.g, color.b, .1));
        parameterList->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        parameterList->setTextColor(GUI_OPAQUE_WHITE());
        parameterList->setSelectionColor(GFXColor(0, .6, 0, .8));
        parameterList->setHighlightColor(GFXColor(0, .6, 0, .35));
        parameterList->setHighlightTextColor(GUI_OPAQUE_WHITE());
        parameterList->setFont(Font(.07));
        parameterList->setTextMargins(Size(0.02, 0.01));
        parameterList->setId("ParameterLister");
        parameterList->setScroller(parameterScroller);
        criteriaGroup->addChild(parameterList);
        parameterList->clear();
        parameterList->addCell(new ValuedPickerCell<CriteriaType>(CONTAINS, "Contains"));
        parameterList->addCell(new ValuedPickerCell<CriteriaType>(OWNEDBY, "Owned By"));
        parameterList->addCell(new ValuedPickerCell<CriteriaType>(SECTOR, "Sector"));

        criteriaGroup->addChild(parameterScroller);         //Want scroller "over" picker.

        //Parameter value box.
        StaticDisplay *valueBox = new TextInputDisplay(&nav_keyboard_queue, "\x1b\n\t\r\\/&|:<>\"^");
        valueBox->setRect(Rect(-.66, .01, .86, .10));
        valueBox->setColor(GFXColor(color.r, color.g, color.b, .1));
        valueBox->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        valueBox->setFont(Font(.07));
        valueBox->setMultiLine(true);
        valueBox->setTextColor(GUI_OPAQUE_WHITE());
        valueBox->setTextMargins(Size(.02, .01));
        valueBox->setId("ParameterValueBox");
        criteriaGroup->addChild(valueBox);

        NewButton *andButton = new NewButton;
        andButton->setRect(Rect(-.66, -.15, .22, .12));
        andButton->setColor(GFXColor(0, 1, 1, .1));
        andButton->setTextColor(GUI_OPAQUE_WHITE());
        andButton->setDownColor(GFXColor(0, 1, 1, .4));
        andButton->setDownTextColor(GFXColor(.2, .2, .2));
        andButton->setVariableBorderCycleTime(1.0);
        andButton->setBorderColor(GFXColor(.2, .2, .2));
        andButton->setEndBorderColor(GFXColor(.4, .4, .4));
        andButton->setShadowWidth(2.0);
        andButton->setFont(Font(.08, BOLD_STROKE));
        andButton->setId("And");
        andButton->setLabel("And");
        andButton->setCommand("And");
        criteriaGroup->addChild(andButton);

        NewButton *orButton = new NewButton;
        orButton->setRect(Rect(-.34, -.15, .22, .12));
        orButton->setColor(GFXColor(0, 1, 1, .1));
        orButton->setTextColor(GUI_OPAQUE_WHITE());
        orButton->setDownColor(GFXColor(0, 1, 1, .4));
        orButton->setDownTextColor(GFXColor(.2, .2, .2));
        orButton->setVariableBorderCycleTime(1.0);
        orButton->setBorderColor(GFXColor(.2, .2, .2));
        orButton->setEndBorderColor(GFXColor(.4, .4, .4));
        orButton->setShadowWidth(2.0);
        orButton->setFont(Font(.08, BOLD_STROKE));
        orButton->setId("Or");
        orButton->setLabel("Or");
        orButton->setCommand("Or");
        criteriaGroup->addChild(orButton);

        NewButton *notButton = new NewButton;
        notButton->setRect(Rect(-.02, -.15, .22, .12));
        notButton->setColor(GFXColor(0, 1, 1, .1));
        notButton->setTextColor(GUI_OPAQUE_WHITE());
        notButton->setDownColor(GFXColor(0, 1, 1, .4));
        notButton->setDownTextColor(GFXColor(.2, .2, .2));
        notButton->setVariableBorderCycleTime(1.0);
        notButton->setBorderColor(GFXColor(.2, .2, .2));
        notButton->setEndBorderColor(GFXColor(.4, .4, .4));
        notButton->setShadowWidth(2.0);
        notButton->setFont(Font(.08, BOLD_STROKE));
        notButton->setId("Not");
        notButton->setLabel("Not");
        notButton->setCommand("Not");
        criteriaGroup->addChild(notButton);

        NewButton *removeButton = new NewButton;
        removeButton->setRect(Rect(-.20, -.31, .40, .12));
        removeButton->setColor(GFXColor(0, 1, 1, .1));
        removeButton->setTextColor(GUI_OPAQUE_WHITE());
        removeButton->setDownColor(GFXColor(0, 1, 1, .4));
        removeButton->setDownTextColor(GFXColor(.2, .2, .2));
        removeButton->setVariableBorderCycleTime(1.0);
        removeButton->setBorderColor(GFXColor(.2, .2, .2));
        removeButton->setEndBorderColor(GFXColor(.4, .4, .4));
        removeButton->setShadowWidth(2.0);
        removeButton->setFont(Font(.08, BOLD_STROKE));
        removeButton->setId("RemoveCriteria");
        removeButton->setLabel("Remove");
        removeButton->setCommand("RemoveCriteria");
        criteriaGroup->addChild(removeButton);

        //Scroller for picker.
        Scroller *criteriaScroller = new Scroller;
        criteriaScroller->setRect(Rect(.15, -.95, .05, .60));
        criteriaScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        criteriaScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        criteriaScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        criteriaScroller->setTextColor(GUI_OPAQUE_WHITE());
        criteriaScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Criteria picker.
        SimplePicker *criteriaList = new SimplePicker;
        criteriaList->setRect(Rect(-.66, -.95, .81, .60));
        criteriaList->setColor(GFXColor(color.r, color.g, color.b, .1));
        criteriaList->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        criteriaList->setTextColor(GUI_OPAQUE_WHITE());
        criteriaList->setSelectionColor(GFXColor(0, .6, 0, .8));
        criteriaList->setHighlightColor(GFXColor(0, .6, 0, .35));
        criteriaList->setHighlightTextColor(GUI_OPAQUE_WHITE());
        criteriaList->setFont(Font(.07));
        criteriaList->setTextMargins(Size(0.02, 0.01));
        criteriaList->setId("CriteriaLister");
        criteriaList->setScroller(criteriaScroller);
        criteriaGroup->addChild(criteriaList);

        criteriaGroup->addChild(criteriaScroller);         //Want scroller "over" picker.
    }

    {
        GroupControl *chainGroup = new GroupControl;
        chainGroup->setId(selectorModeInfo[CHAIN].groupId);
        window()->addControl(chainGroup);
        GFXColor color = getColorForGroup(selectorModeInfo[CHAIN].groupId);

        //Scroller for picker.
        Scroller *chainScroller = new Scroller;
        chainScroller->setRect(Rect(.15, -.31, .05, .69));
        chainScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        chainScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        chainScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        chainScroller->setTextColor(GUI_OPAQUE_WHITE());
        chainScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Path picker.
        SimplePicker *chainList = new SimplePicker;
        chainList->setRect(Rect(-.66, -.31, .81, .69));
        chainList->setColor(GFXColor(color.r, color.g, color.b, .1));
        chainList->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        chainList->setTextColor(GUI_OPAQUE_WHITE());
        chainList->setSelectionColor(GFXColor(0, .6, 0, .8));
        chainList->setHighlightColor(GFXColor(0, .6, 0, .35));
        chainList->setHighlightTextColor(GUI_OPAQUE_WHITE());
        chainList->setFont(Font(.07));
        chainList->setTextMargins(Size(0.02, 0.01));
        chainList->setId("ChainLister");
        chainList->setScroller(chainScroller);
        chainGroup->addChild(chainList);

        chainGroup->addChild(chainScroller);          //Want scroller "over" picker.

        //Scroller for picker.
        Scroller *chainTypeScroller = new Scroller;
        chainTypeScroller->setRect(Rect(.15, -.79, .05, .44));
        chainTypeScroller->setColor(GFXColor(color.r, color.g, color.b, .1));
        chainTypeScroller->setThumbColor(GFXColor(color.r * .4, color.g * .4, color.b * .4), GUI_OPAQUE_WHITE());
        chainTypeScroller->setButtonColor(GFXColor(color.r * .4, color.g * .4, color.b * .4));
        chainTypeScroller->setTextColor(GUI_OPAQUE_WHITE());
        chainTypeScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

        //Type picker.
        SimplePicker *chainTypeList = new SimplePicker;
        chainTypeList->setRect(Rect(-.66, -.79, .81, .44));
        chainTypeList->setColor(GFXColor(color.r, color.g, color.b, .1));
        chainTypeList->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
        chainTypeList->setTextColor(GUI_OPAQUE_WHITE());
        chainTypeList->setSelectionColor(GFXColor(0, .6, 0, .8));
        chainTypeList->setHighlightColor(GFXColor(0, .6, 0, .35));
        chainTypeList->setHighlightTextColor(GUI_OPAQUE_WHITE());
        chainTypeList->setFont(Font(.07));
        chainTypeList->setTextMargins(Size(0.02, 0.01));
        chainTypeList->setId("ChainTypeLister");
        chainTypeList->setScroller(chainTypeScroller);
        chainGroup->addChild(chainTypeList);
        chainTypeList->clear();
        chainTypeList->addCell(new ValuedPickerCell<ChainPathNode::PartType>(ChainPathNode::SOURCE, "Source"));
        chainTypeList->addCell(new ValuedPickerCell<ChainPathNode::PartType>(ChainPathNode::DESTINATION,
                "Destination"));
        chainTypeList->addCell(new ValuedPickerCell<ChainPathNode::PartType>(ChainPathNode::ALL_POINTS, "All Points"));

        chainGroup->addChild(chainTypeScroller);         //Want scroller "over" picker.

        NewButton *chain = new NewButton;
        chain->setRect(Rect(-.43, -.95, .40, .12));
        chain->setColor(GFXColor(0, 1, 1, .1));
        chain->setTextColor(GUI_OPAQUE_WHITE());
        chain->setDownColor(GFXColor(0, 1, 1, .4));
        chain->setDownTextColor(GFXColor(.2, .2, .2));
        chain->setVariableBorderCycleTime(1.0);
        chain->setBorderColor(GFXColor(.2, .2, .2));
        chain->setEndBorderColor(GFXColor(.4, .4, .4));
        chain->setShadowWidth(2.0);
        chain->setFont(Font(.08, BOLD_STROKE));
        chain->setId("Chain");
        chain->setLabel("Chain");
        chain->setCommand("Chain");
        chainGroup->addChild(chain);
    }
}

//Switch to the set of controls used for the specified mode.
void NavComputer::switchToMajorControls(DisplayMode mode) {
    if (m_currentDisplay != mode) {
        assert(m_displayModeGroups[mode] != NULL);         //We should have controls for this mode.
        if (m_currentDisplay != NULL_DISPLAY) {
            //Get the old controls out of the window.
            Control *oldControls = window()->findControlById(displayModeInfo[m_currentDisplay].groupId);
            if (oldControls) {
                window()->removeControlFromWindow(oldControls);
            }
            //We put this back in our table so that we "own" the controls.
            m_displayModeGroups[m_currentDisplay] = oldControls;
        }
        m_currentDisplay = mode;

        window()->addControl(m_displayModeGroups[mode]);
        //Take this group out of our table because we don't own it anymore.
        m_displayModeGroups[mode] = NULL;

        recalcTitle();
    }
}

//Switch to the set of controls used for the specified mode.
void NavComputer::switchToMinorControls(SelectorMode mode) {
    if (m_currentSelector != mode) {
        if (m_currentSelector != NULL_SELECTOR) {
            //Get the old controls out of the window.
            Control *oldControls = window()->findControlById(selectorModeInfo[m_currentSelector].groupId);
            if (oldControls) {
                window()->removeControlFromWindow(oldControls);
            }
            //We put this back in our table so that we "own" the controls.
            m_selectorModeGroups[m_currentSelector] = oldControls;
        }
        m_currentSelector = mode;
        if (mode != NULL_SELECTOR) {
            assert(m_selectorModeGroups[mode] != NULL);             //We should have controls for this mode.
            window()->addControl(m_selectorModeGroups[mode]);
            //Take this group out of our table because we don't own it anymore.
            m_selectorModeGroups[mode] = NULL;
        }
    }
}

//Change display mode to LIST
bool NavComputer::changeToListMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != LIST) {
        switchToMinorControls(NULL_SELECTOR);
        switchToMajorControls(LIST);
        loadPathLister();
    }
    return true;
}

//Change display mode to EDIT
bool NavComputer::changeToEditMode(const EventCommandId &command, Control *control) {
    if (m_currentDisplay != EDIT && currentPath != NULL) {
        switchToMinorControls(NULL_SELECTOR);
        switchToMajorControls(EDIT);
        setCurrentNode();
        criteria = false;
    }
    return true;
}

//Change display mode to TARGET
bool NavComputer::changeToTargetMode(const EventCommandId &command, Control *control) {
    assert(m_currentDisplay == EDIT);     //We should be in edit mode to have chosen a selector
    if (m_currentSelector != TARGET) {
        switchToMinorControls(TARGET);
        loadAbsoluteButton();
    }
    return true;
}

//Change display mode to CRITERIA
bool NavComputer::changeToCriteriaMode(const EventCommandId &command, Control *control) {
    assert(m_currentDisplay == EDIT);     //We should be in edit mode to have chosen a selector
    if (m_currentSelector != CRITERIA) {
        switchToMinorControls(CRITERIA);
        loadCriteriaLister();
    }
    return true;
}

//Change display mode to CHAIN
bool NavComputer::changeToChainMode(const EventCommandId &command, Control *control) {
    assert(m_currentDisplay == EDIT);     //We should be in edit mode to have chosen a selector
    if (m_currentSelector != CHAIN) {
        switchToMinorControls(CHAIN);
        loadChainLister();
    }
    return true;
}

//Open the window, etc.
void NavComputer::run(void) {
    toggleVisibility(EventCommandId(), NULL);
}

void nav_main_loop() {
    GFXBeginScene();
    globalWindowManager().draw();
    GFXEndScene();
}

int shiftup(int);

static void nav_keyboard_cb(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    if (!release) {
        nav_keyboard_queue.push_back(
                ((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT)) || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT))) ? shiftup(
                        ch) : ch);
    }
}

bool NavComputer::toggleVisibility(const EventCommandId &command, Control *control) {
    if (m_visible) {
        m_window->close();
        m_visible = false;

        pathman->updatePaths();

        RestoreKB();
        GFXLoop(main_loop);
        RestoreMouse();

        GameCockpit::NavScreen(KBData(), PRESS);
    } else {
        GameCockpit::NavScreen(KBData(), PRESS);
        //Initialize callback functions
        winsys_set_keyboard_func(nav_keyboard_cb);
        winsys_set_mouse_func(EventManager::ProcessMouseClick);
        winsys_set_motion_func(EventManager::ProcessMouseActive);
        winsys_set_passive_motion_func(EventManager::ProcessMousePassive);
        m_window->open();
        m_visible = true;
        GFXLoop(nav_main_loop);

        //Simulate clicking the leftmost mode button.
        //We don't actually use the button because there isn't a button if there's only one mode.
        processWindowCommand(displayModeInfo[m_displayModes[0]].command, NULL);
    }
    return m_visible;
}

//Redo the title strings for the display.
void NavComputer::recalcTitle() {
    //Generic nav title for the display.

    string baseTitle = "Navigational Computer";
    baseTitle += " (";
    baseTitle += displayModeInfo[m_currentDisplay].title;
    baseTitle += ")";

    //Set the string in the base title control.
    StaticDisplay *baseTitleDisplay = static_cast< StaticDisplay * > ( window()->findControlById("NavigationTitle"));
    assert(baseTitleDisplay != NULL);
    baseTitleDisplay->setText(baseTitle);
}

//Create the window and controls for the Options Menu.
void NavComputer::RenameConfirm::init(void) {
    Window *window = new Window;
    setWindow(window);

    window->setSizeAndCenter(Size(.9, .62));
    window->setTexture("basecomputer.png");
    window->setColor(GFXColor(0, 1, 0, .1));
    window->setOutlineColor(GFXColor(.7, .7, .7));
    window->setOutlineWidth(2.0);
    window->setController(this);

    //Information.
    StaticDisplay *text = new StaticDisplay;
    text->setRect(Rect(-.4, .11, .8, .15));
    text->setText("Enter the new name for this path.");
    text->setTextColor(GFXColor(.7, 1, .4));
    text->setMultiLine(true);
    text->setColor(GUI_CLEAR);
    text->setFont(Font(.07, 1.25));
    text->setId("Information");
    //Put it on the window.
    window->addControl(text);

    //Path name input box.
    StaticDisplay *pathNameBox = new TextInputDisplay(&nav_keyboard_queue, "\x1b\n\t\r");
    pathNameBox->setRect(Rect(-.4, -.09, .8, .15));
    pathNameBox->setColor(GFXColor(0, 1, 1, .1));
    pathNameBox->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    pathNameBox->setFont(Font(.07));
    pathNameBox->setMultiLine(true);
    pathNameBox->setTextColor(GUI_OPAQUE_WHITE());
    pathNameBox->setTextMargins(Size(.02, .01));
    pathNameBox->setId("PathNameBox");
    window->addControl(pathNameBox);

    //Confirm Button.
    NewButton *confirm = new NewButton;
    confirm->setRect(Rect(.05, -.25, .30, .10));
    confirm->setLabel("Rename");
    confirm->setCommand("Rename");
    confirm->setColor(GFXColor(1, .5, 0, .25));
    confirm->setTextColor(GUI_OPAQUE_WHITE());
    confirm->setDownColor(GFXColor(1, .5, 0, .6));
    confirm->setDownTextColor(GUI_OPAQUE_BLACK());
    confirm->setHighlightColor(GFXColor(0, 1, 0, .4));
    confirm->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(confirm);

    //Resume Game button.
    NewButton *resume = new NewButton;
    resume->setRect(Rect(-.35, -.26, .30, .12));
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
bool NavComputer::RenameConfirm::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Rename") {
        TextInputDisplay *input = static_cast< TextInputDisplay * > ( window()->findControlById("PathNameBox"));
        assert(input != NULL);
        m_parent->actionRenameConfirmed(input->text());
        window()->close();
    } else {
        //Not a command we know about.
        return WindowController::processWindowCommand(command, control);
    }
    return true;
}

//Load the paths to be put in the lister.
void NavComputer::loadPathLister() {
    SimplePicker *listPicker = static_cast< SimplePicker * > ( window()->findControlById("PathLister"));
    assert(listPicker != NULL);
    listPicker->clear();

    currentPath = NULL;
    for (vector<NavPath *>::iterator i = pathman->paths.begin(); i < pathman->paths.end(); ++i) {
        listPicker->addCell(new ValuedPickerCell<NavPath *>((*i), (*i)->getName()));
    }
    //Make sure the description is empty.
    StaticDisplay *desc = static_cast< StaticDisplay * > ( window()->findControlById("Description"));
    assert(desc != NULL);
    desc->setText("");
}

//Load the paths to be put in the lister.
void NavComputer::loadChainLister() {
    SimplePicker *chainPicker = static_cast< SimplePicker * > ( window()->findControlById("ChainLister"));
    assert(chainPicker != NULL);
    chainPicker->clear();
    for (vector<NavPath *>::iterator i = pathman->paths.begin(); i < pathman->paths.end(); ++i) {
        chainPicker->addCell(new ValuedPickerCell<NavPath *>((*i), (*i)->getName()));
    }
}

void NavComputer::loadCriteriaPickerCell(SimplePicker *picker,
        ValuedPickerCell<CriteriaNode *> *parent,
        CriteriaNode *node) {
    assert(node != NULL);
    ValuedPickerCell<CriteriaNode *> *cell = new ValuedPickerCell<CriteriaNode *>(node, node->getText());
    cell->setHideChildren(false);
    if (parent) {
        parent->addChild(cell);
    } else {
        assert(picker != NULL);
        picker->addCell(cell);
    }
    vector<CriteriaNode *> childList = node->getChildren();
    for (vector<CriteriaNode *>::iterator i = childList.begin(); i < childList.end(); ++i) {
        loadCriteriaPickerCell(picker, cell, (*i));
    }
}

//Load the criteria to be put in the lister.
void NavComputer::loadCriteriaLister() {
    SimplePicker *picker = static_cast< SimplePicker * > ( window()->findControlById("CriteriaLister"));
    assert(picker != NULL);
    picker->clear();
    if (criteria) {
        if (static_cast< CriteriaPathNode * > (currentNode)->getRoot()->getChild()) {
            loadCriteriaPickerCell(picker,
                    NULL,
                    static_cast< CriteriaPathNode * > (currentNode)->getRoot()->getChild());
        }
    }
}

//Load the absolute button.
void NavComputer::loadAbsoluteButton() {
    NewButton *absolute = static_cast< NewButton * > ( window()->findControlById("Absolute"));
    assert(absolute != NULL);
    absolute->setLabel(navsys->systemIter[navsys->systemselectionindex].GetName());
}

bool NavComputer::setCurrentNode(PathNode *source) {
    if (currentNode != nullptr) {
        delete currentNode;
        currentNode = nullptr;
    }
    currentNode = source;
    updateNodeDescription();
    return true;
}

void NavComputer::updateDescription() {
    StaticDisplay *desc = static_cast< StaticDisplay * > ( window()->findControlById("Description"));
    assert(desc != NULL);
    if (currentPath) {
        desc->setText(currentPath->getDescription());
    } else {
        desc->setText("");
    }
}

void NavComputer::updateNodeDescription() {
    StaticDisplay *desc = static_cast< StaticDisplay * > ( window()->findControlById("NodeDescription"));
    assert(desc != NULL);
    if (currentNode) {
        desc->setText(currentNode->getDescription());
    } else {
        desc->setText("");
    }
}

//The selection in the Path lister changed.
bool NavComputer::pathListerChangedSelection(const EventCommandId &command, Control *control) {
    assert(control != NULL);
    Picker *picker = static_cast< Picker * > (control);
    ValuedPickerCell<NavPath *> *cell = static_cast< ValuedPickerCell<NavPath *> * > ( picker->selectedCell());
    if (cell == NULL) {
        currentPath = NULL;
    } else {
        currentPath = cell->value();
    }
    updateDescription();

    return true;
}

bool NavComputer::actionAdd(const EventCommandId &command, Control *control) {
    pathman->addPath();
    loadPathLister();
    return true;
}

bool NavComputer::actionShowPath(const EventCommandId &command, Control *control) {
    if (currentPath) {
        currentPath->setVisible(!currentPath->getVisible());
    }
    updateDescription();
    return true;
}

bool NavComputer::actionRename(const EventCommandId &command, Control *control) {
    if (currentPath) {
        RenameConfirm *renamer = new RenameConfirm(this);
        renamer->init();
        renamer->run();
    }
    return true;
}

bool NavComputer::actionRemove(const EventCommandId &command, Control *control) {
    if (currentPath) {
        pathman->removePath(currentPath);
    }
    loadPathLister();
    return true;
}

bool NavComputer::actionShowAll(const EventCommandId &command, Control *control) {
    pathman->showAll();
    updateDescription();
    return true;
}

bool NavComputer::actionShowNone(const EventCommandId &command, Control *control) {
    pathman->showNone();
    updateDescription();
    return true;
}

bool NavComputer::actionSource(const EventCommandId &command, Control *control) {
    if (currentNode) {
        assert(currentPath);
        currentPath->setSourceNode(currentNode->clone());
        updateDescription();
    }
    return true;
}

bool NavComputer::actionDestination(const EventCommandId &command, Control *control) {
    if (currentNode) {
        assert(currentPath);
        currentPath->setDestinationNode(currentNode->clone());
        updateDescription();
    }
    return true;
}

bool NavComputer::actionCurrent(const EventCommandId &command, Control *control) {
    setCurrentNode(new CurrentPathNode());
    criteria = false;
    return true;
}

bool NavComputer::actionTarget(const EventCommandId &command, Control *control) {
    setCurrentNode(new TargetPathNode());
    criteria = false;
    return true;
}

bool NavComputer::actionAbsolute(const EventCommandId &command, Control *control) {
    setCurrentNode(new AbsolutePathNode(navsys->systemselectionindex));
    criteria = false;
    return true;
}

bool NavComputer::actionAnd(const EventCommandId &command, Control *control) {
    Picker *parameterPicker = static_cast< Picker * > ( window()->findControlById("ParameterLister"));
    assert(parameterPicker != NULL);

    Picker *criteriaPicker = static_cast< Picker * > ( window()->findControlById("CriteriaLister"));
    assert(criteriaPicker != NULL);

    TextInputDisplay *input = static_cast< TextInputDisplay * > ( window()->findControlById("ParameterValueBox"));
    assert(input != NULL);
    if (input->text() == "") {
        return true;
    }
    ValuedPickerCell<CriteriaType> *parameterCell =
            static_cast< ValuedPickerCell<CriteriaType> * > ( parameterPicker->selectedCell());
    ValuedPickerCell<CriteriaNode *> *criteriaCell =
            static_cast< ValuedPickerCell<CriteriaNode *> * > ( criteriaPicker->selectedCell());
    if (parameterCell == NULL) {
        return true;
    }
    if (!criteria) {
        setCurrentNode(new CriteriaPathNode());
        criteria = true;
    }
    CriteriaNode *newNode;
    if (parameterCell->value() == CONTAINS) {
        newNode = new CriteriaContains(input->text());
    } else if (parameterCell->value() == OWNEDBY) {
        newNode = new CriteriaOwnedBy(input->text());
    } else {
        newNode = new CriteriaSector(input->text());
    }
    if (static_cast< CriteriaPathNode * > (currentNode)->getRoot()->getChild() == NULL) {
        static_cast< CriteriaPathNode * > (currentNode)->getRoot()->setChild(newNode);
    } else {
        if (criteriaCell == NULL) {
            return true;
        }
        new CriteriaAnd(criteriaCell->value(), newNode);
    }
    updateNodeDescription();
    loadCriteriaLister();
    return true;
}

bool NavComputer::actionOr(const EventCommandId &command, Control *control) {
    Picker *parameterPicker = static_cast< Picker * > ( window()->findControlById("ParameterLister"));
    assert(parameterPicker != NULL);

    Picker *criteriaPicker = static_cast< Picker * > ( window()->findControlById("CriteriaLister"));
    assert(criteriaPicker != NULL);

    TextInputDisplay *input = static_cast< TextInputDisplay * > ( window()->findControlById("ParameterValueBox"));
    assert(input != NULL);
    if (input->text() == "") {
        return true;
    }
    ValuedPickerCell<CriteriaType> *parameterCell =
            static_cast< ValuedPickerCell<CriteriaType> * > ( parameterPicker->selectedCell());
    ValuedPickerCell<CriteriaNode *> *criteriaCell =
            static_cast< ValuedPickerCell<CriteriaNode *> * > ( criteriaPicker->selectedCell());
    if (parameterCell == NULL) {
        return true;
    }
    if (!criteria) {
        setCurrentNode(new CriteriaPathNode());
        criteria = true;
    }
    CriteriaNode *newNode;
    if (parameterCell->value() == CONTAINS) {
        newNode = new CriteriaContains(input->text());
    } else if (parameterCell->value() == OWNEDBY) {
        newNode = new CriteriaOwnedBy(input->text());
    } else {
        newNode = new CriteriaSector(input->text());
    }
    if (static_cast< CriteriaPathNode * > (currentNode)->getRoot()->getChild() == NULL) {
        static_cast< CriteriaPathNode * > (currentNode)->getRoot()->setChild(newNode);
    } else {
        if (criteriaCell == NULL) {
            return true;
        }
        new CriteriaOr(criteriaCell->value(), newNode);
    }
    updateNodeDescription();
    loadCriteriaLister();
    return true;
}

bool NavComputer::actionNot(const EventCommandId &command, Control *control) {
    Picker *criteriaPicker = static_cast< Picker * > ( window()->findControlById("CriteriaLister"));
    assert(criteriaPicker != NULL);
    ValuedPickerCell<CriteriaNode *> *criteriaCell =
            static_cast< ValuedPickerCell<CriteriaNode *> * > ( criteriaPicker->selectedCell());
    if (criteriaCell == NULL) {
        return true;
    }
    assert(criteriaCell->value() != NULL);
    new CriteriaNot(criteriaCell->value());

    updateNodeDescription();
    loadCriteriaLister();
    return true;
}

bool NavComputer::actionRemoveCriteria(const EventCommandId &command, Control *control) {
    Picker *criteriaPicker = static_cast< Picker * > ( window()->findControlById("CriteriaLister"));
    assert(criteriaPicker != NULL);
    ValuedPickerCell<CriteriaNode *> *criteriaCell =
            static_cast< ValuedPickerCell<CriteriaNode *> * > ( criteriaPicker->selectedCell());
    if (criteriaCell == NULL) {
        return true;
    }
    CriteriaNode *deleteHere = criteriaCell->value()->unhook();
    if (deleteHere != nullptr) {
        delete deleteHere;
        deleteHere = nullptr;
    }
    updateNodeDescription();
    loadCriteriaLister();
    return true;
}

bool NavComputer::actionChain(const EventCommandId &command, Control *control) {
    Picker *pathPicker = static_cast< Picker * > ( window()->findControlById("ChainLister"));
    assert(pathPicker != NULL);

    Picker *typePicker = static_cast< Picker * > ( window()->findControlById("ChainTypeLister"));
    assert(typePicker != NULL);

    ValuedPickerCell<NavPath *> *pathCell = static_cast< ValuedPickerCell<NavPath *> * > ( pathPicker->selectedCell());
    ValuedPickerCell<ChainPathNode::PartType> *typeCell =
            static_cast< ValuedPickerCell<ChainPathNode::PartType> * > ( typePicker->selectedCell());
    if (!pathCell || !typeCell) {
        return true;
    }
    setCurrentNode(new ChainPathNode(pathCell->value(), typeCell->value()));
    criteria = false;
    return true;
}

void NavComputer::actionRenameConfirmed(std::string name) {
    assert(currentPath != NULL);

    currentPath->setName(name);
    loadPathLister();
}
