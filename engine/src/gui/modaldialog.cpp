/*
 * Copyright (C) 2001-2022 Daniel Horn, Mike Byron, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "src/vegastrike.h"

#include "modaldialog.h"

//Need these until we can read XML window descriptions.
#include "staticdisplay.h"
#include "newbutton.h"
#include "simplepicker.h"
#include "scroller.h"
#include "eventmanager.h"

//The class that gets called when the window closes.
void ModalDialog::setCallback(ModalDialogCallback *cb, const std::string &id) {
    m_callback = cb;
    m_callbackId = id;
}

//Make everything happen.
void ModalDialog::run(void) {
    if (window()) {
        window()->setModal(true);
    }
    WindowController::run();
}

void ModalDialog::modalFinished() {
    globalEventManager().removeResponder(window());
    //One of our buttons was clicked.  We're done.
    m_deleteOnWindowClose = false;     //Prevent deleting until we are done.
    window()->close();
    if (m_callback) {
        m_callback->modalDialogResult(m_callbackId, m_result, *this);
    }
    delete this;
}

//Process a command from the window.
bool ModalDialog::processWindowCommand(const EventCommandId &command, Control *control) {
    return WindowController::processWindowCommand(command, control);
}

//CONSTRUCTOR
ModalDialog::ModalDialog() : m_callbackId(), m_callback(NULL), m_result(0) {
}

//////////////  SUPPORT FOR SHOWQUESTION() FUNCTIONS ////////////////////////

//Hack for creating the alert window.
static void CreateControlsForAlertWindow(Window *window) {
    window->setSizeAndCenter(Size(1, .8));
    window->setTexture("basecomputer.png");
    window->setColor(SaturatedColor(1, 0, 0, .1));
    window->setOutlineColor(SaturatedColor(.7, .7, .7));
    window->setOutlineWidth(2.0);

    //Title text display.
    StaticDisplay *title = new StaticDisplay;
    title->setRect(Rect(-.46, -.25, .92, .55));
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GUI_OPAQUE_WHITE());
    title->setColor(GUI_CLEAR);
    title->setFont(Font(.07, BOLD_STROKE));
    title->setId("Title");
    //Put it on the window.
    window->addControl(title);

    //OK button.
    NewButton *ok = new NewButton;
    ok->setRect(Rect(-.11, -.35, .22, .1));
    ok->setLabel("OK");
    ok->setCommand("Window::Close");
    ok->setColor(GFXColor(1, 0, 0, .25));
    ok->setTextColor(GUI_OPAQUE_WHITE());
    ok->setDownColor(GFXColor(1, 0, 0, .6));
    ok->setDownTextColor(GUI_OPAQUE_BLACK());
    ok->setHighlightColor(GFXColor(1, 0, 0, .4));
    ok->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(ok);
}

//Hack for creating the Yes/No window.
static void CreateControlsForYesNoWindow(Window *window) {
    window->setSizeAndCenter(Size(1, .8));
    window->setTexture("basecomputer.png");
    window->setColor(SaturatedColor(1, 0, 0, .1));
    window->setOutlineColor(SaturatedColor(.7, .7, .7));
    window->setOutlineWidth(2.0);

    //Title text display.
    StaticDisplay *title = new StaticDisplay;
    title->setRect(Rect(-.46, -.25, .92, .55));
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GUI_OPAQUE_WHITE());
    title->setColor(GUI_CLEAR);
    title->setFont(Font(.07, BOLD_STROKE));
    title->setId("Title");
    //Put it on the window.
    window->addControl(title);

    //Yes button.
    NewButton *yes = new NewButton;
    yes->setRect(Rect(-.27, -.35, .22, .1));
    yes->setLabel("Yes");
    yes->setCommand("Yes");
    yes->setColor(GFXColor(1, 0, 0, .25));
    yes->setTextColor(GUI_OPAQUE_WHITE());
    yes->setDownColor(GFXColor(1, 0, 0, .6));
    yes->setDownTextColor(GUI_OPAQUE_BLACK());
    yes->setHighlightColor(GFXColor(1, 0, 0, .4));
    yes->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(yes);

    //No button.
    NewButton *no = new NewButton;
    no->setRect(Rect(.05, -.35, .22, .1));
    no->setLabel("No");
    no->setCommand("No");
    no->setColor(GFXColor(1, 0, 0, .25));
    no->setTextColor(GUI_OPAQUE_WHITE());
    no->setDownColor(GFXColor(1, 0, 0, .6));
    no->setDownTextColor(GUI_OPAQUE_BLACK());
    no->setHighlightColor(GFXColor(1, 0, 0, .4));
    no->setFont(Font(.08, BOLD_STROKE));
    //Put the button on the window.
    window->addControl(no);
}

//Hack for creating the List Question window.
void ListQuestionDialog::CreateControlsForListWindow(Window *window) {
    window->setSizeAndCenter(Size(1.2, 1.2));
    window->setTexture("basecomputer.png");
    window->setColor(SaturatedColor(0, 0, 1, .1));
    window->setOutlineColor(SaturatedColor(.7, .7, .7));
    window->setOutlineWidth(2.0);

    //Title text display.
    StaticDisplay *title = new StaticDisplay;
    title->setRect(Rect(-.57, .42, 1.14, .15));
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GUI_OPAQUE_WHITE());
    title->setColor(GUI_CLEAR);
    title->setFont(Font(.08, BOLD_STROKE));
    title->setId("Title");
    //Put it on the window.
    window->addControl(title);

    //OK button.
    NewButton *ok = new NewButton;
    ok->setRect(Rect(-.27, -.57, .22, .1));
    ok->setLabel("OK");
    ok->setCommand("OK");
    ok->setColor(GFXColor(0, 0, 1, .25));
    ok->setTextColor(GUI_OPAQUE_WHITE());
    ok->setDownColor(GFXColor(0, 0, 1, .6));
    ok->setDownTextColor(GUI_OPAQUE_BLACK());
    ok->setHighlightColor(GFXColor(0, 0, 1, .4));
    ok->setFont(Font(.08, BOLD_STROKE));
    ok->setId("OK");
    //Put the button on the window.
    window->addControl(ok);

    //Cancel button.
    NewButton *cancel = new NewButton;
    cancel->setRect(Rect(.05, -.57, .22, .1));
    cancel->setLabel("Cancel");
    cancel->setCommand("Cancel");
    cancel->setColor(GFXColor(0, 0, 1, .25));
    cancel->setTextColor(GUI_OPAQUE_WHITE());
    cancel->setDownColor(GFXColor(0, 0, 1, .6));
    cancel->setDownTextColor(GUI_OPAQUE_BLACK());
    cancel->setHighlightColor(GFXColor(0, 0, 1, .4));
    cancel->setFont(Font(.08, BOLD_STROKE));
    cancel->setId("Cancel");
    //Put the button on the window.
    window->addControl(cancel);

    //Scroller for description.
    Scroller *pickScroller = new Scroller;
    pickScroller->setRect(Rect(.52, -.4, .05, .85));
    pickScroller->setColor(SaturatedColor(0, 0, 1, .1));
    pickScroller->setThumbColor(SaturatedColor(0, 0, .4), GUI_OPAQUE_WHITE());
    pickScroller->setButtonColor(SaturatedColor(0, 0, .4));
    pickScroller->setTextColor(GUI_OPAQUE_WHITE());
    pickScroller->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());

    //List picker.
    SimplePicker *pick = new SimplePicker;
    pick->setRect(Rect(-.57, -.4, 1.09, .85));
    pick->setColor(SaturatedColor(0, 0, 1, .1));
    pick->setOutlineColor(GUI_OPAQUE_MEDIUM_GRAY());
    pick->setTextColor(GUI_OPAQUE_WHITE());
    pick->setSelectionColor(SaturatedColor(.2, .2, .7, .8));
    pick->setHighlightColor(SaturatedColor(0, 0, .6, .35));
    pick->setHighlightTextColor(GUI_OPAQUE_WHITE());
    pick->setFont(Font(.065));
    pick->setTextMargins(Size(0.02, 0.01));
    pick->setId("Picker");
    pick->setScroller(pickScroller);
    //Put the picker on the window.
    window->addControl(pick);

    window->addControl(pickScroller);         //Want scroller "over" picker.
}

//Initialization of our Question Dialog class.
//Create a window, load controls, and set the title.
void QuestionDialog::init(const std::string &title) {
    setWindow(new Window);
    initControls();

    StaticDisplay *titleControl = static_cast< StaticDisplay * > ( window()->findControlById("Title"));
    assert(titleControl != NULL);
    std::string fixedTitle;
    std::string::size_type lastpos = 0, pos;
    for (pos = title.find('\n'); pos != std::string::npos; pos = title.find('\n', lastpos)) {
        fixedTitle.append(title, lastpos, pos - lastpos);
        fixedTitle.append("#n#");
        lastpos = pos + 1;
    }
    fixedTitle.append(title, lastpos, std::string::npos);
    titleControl->setText(fixedTitle);

    ModalDialog::init();
}

//Make everything happen.
void QuestionDialog::run(void) {
    //We need to delete the Window we created until the Window is opened.
    m_deleteWindow = false;

    ModalDialog::run();
}

//Class that will handle Alert correctly.
class AlertDialog : public QuestionDialog {
//Load the controls for this dialog into the window.
    virtual void initControls(void) {
        CreateControlsForAlertWindow(window());
    }
};

//Display a modal message to the user.  The message will be shown until the user
//hits the OK button.
void showAlert(const std::string &title) {
    AlertDialog *dialog = new AlertDialog;
    dialog->init(title);
    dialog->run();
}

//Class that will handle Yes/No dialog correctly.
class YesNoDialog : public QuestionDialog {
//Load the controls for this dialog into the window.
    virtual void initControls(void) {
        CreateControlsForYesNoWindow(window());
    }

//Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId &command, Control *control);
};

//Process a command from the window.
bool YesNoDialog::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Yes") {
        m_result = YES_ANSWER;
        modalFinished();
    } else if (command == "No") {
        m_result = NO_ANSWER;
        modalFinished();
    } else {
        //We only care about buttons clicked.
        return QuestionDialog::processWindowCommand(command, control);
    }
    return true;
}

//Display a modal yes/no question.
//The result is supplied in the callback.
void showYesNoQuestion(const std::string &title, ModalDialogCallback *cb, const std::string &id) {
    YesNoDialog *dialog = new YesNoDialog;
    dialog->init(title);
    dialog->setCallback(cb, id);
    dialog->run();
}

//Process a command from the window.
bool ListQuestionDialog::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Cancel") {
        //Just close down, with the result  unknown.
        m_result = (-1);
        modalFinished();
        return true;
    } else if (command == "OK") {
        //The OK button was clicked.
        Picker *picker = static_cast< Picker * > ( window()->findControlById("Picker"));
        assert(picker != NULL);
        m_result = picker->selectedItem();
        if (m_result >= 0) {
            //We have a selection.  We are done.
            modalFinished();
        } else {
            //Ignore OK button until we have a selection.
            //Do nothing here.
        }
        return true;
    } else {
        //We only care about buttons clicked.
        return QuestionDialog::processWindowCommand(command, control);
    }
    assert(false);     //Should never get here.
}

SimplePicker *ListQuestionDialog::getPicker() {
    return static_cast< SimplePicker * > ( window()->findControlById("Picker"));
}

//Display a modal list of options.
//The result is supplied in the callback.
void showListQuestion(const std::string &title,
        const std::vector<std::string> &options,
        ModalDialogCallback *cb,
        const std::string &id) {
    ListQuestionDialog *dialog = new ListQuestionDialog;
    dialog->init(title);
    dialog->setCallback(cb, id);

    //Fill the picker with the choices supplied by the caller.
    SimplePicker *picker = dialog->getPicker();
    assert(picker != NULL);
    for (size_t i = 0; i < options.size(); i++) {
        picker->addCell(new SimplePickerCell(options[i]));
    }
    dialog->run();
}

