/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vegastrike.h"

#include "modaldialog.h"

// Need these until we can read XML window descriptions.
#include "staticdisplay.h"
#include "newbutton.h"
#include "simplepicker.h"


// The class that gets called when the window closes.
void ModalDialog::setCallback(ModalDialogCallback* cb, const std::string& id) {
    m_callback = cb;
    m_callbackId = id;
}

// Make everything happen.
void ModalDialog::run(void) {
    if(window()) {
        window()->setModal(true);
    }

    WindowController::run();
}

// Process a command from the window.
bool ModalDialog::processWindowCommand(const EventCommandId& command, Control* control) {
    if(m_callback && command == "Window::Close") {
        m_callback->modalDialogResult(m_callbackId, m_result, *this);
    }

    return WindowController::processWindowCommand(command, control);
}

// CONSTRUCTOR
ModalDialog::ModalDialog() : m_callbackId(), m_callback(NULL), m_result(0)
{
}




//////////////  SUPPORT FOR SHOWQUESTION() FUNCTIONS ////////////////////////



// Hack for creating the alert window.
static void CreateControlsForAlertWindow(Window* window) {

    window->setSizeAndCenter(Size(1,.8));
    window->setColor(GFXColor(.8,.5,.5));

    // Title text display.
    StaticDisplay* title = new StaticDisplay;
    title->setRect( Rect(-.46, -.25, .92, .55) );
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GFXColor(GUI_OPAQUE_BLACK));
    title->setColor(GUI_CLEAR);
    title->setFont( Font(.07) );
    title->setId("Title");
    // Put it on the window.
    window->addControl(title);

    // OK button.
    NewButton* ok = new NewButton;
    ok->setRect( Rect(-.11, -.35, .22, .1) );
    ok->setLabel("OK");
    ok->setCommand("Window::Close");
    ok->setColor( GFXColor(.4,.2,.2) );
    ok->setTextColor(GUI_OPAQUE_WHITE);
    ok->setFont(Font(.08, BOLD_STROKE));
    // Put the button on the window.
    window->addControl(ok);
}

// Hack for creating the Yes/No window.
static void CreateControlsForYesNoWindow(Window* window) {

    window->setSizeAndCenter(Size(1,.8));
    window->setColor(GFXColor(.8,.5,.5));

    // Title text display.
    StaticDisplay* title = new StaticDisplay;
    title->setRect( Rect(-.46, -.25, .92, .55) );
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GFXColor(GUI_OPAQUE_BLACK));
    title->setColor(GUI_CLEAR);
    title->setFont( Font(.07) );
    title->setId("Title");
    // Put it on the window.
    window->addControl(title);

    // Yes button.
    NewButton* yes = new NewButton;
    yes->setRect( Rect(-.27, -.35, .22, .1) );
    yes->setLabel("Yes");
    yes->setCommand("Yes");
    yes->setColor( GFXColor(.4,.2,.2) );
    yes->setTextColor(GUI_OPAQUE_WHITE);
    yes->setFont(Font(.08, BOLD_STROKE));
    // Put the button on the window.
    window->addControl(yes);

    // No button.
    NewButton* no = new NewButton;
    no->setRect( Rect(.05, -.35, .22, .1) );
    no->setLabel("No");
    no->setCommand("No");
    no->setColor( GFXColor(.4,.2,.2) );
    no->setTextColor(GUI_OPAQUE_WHITE);
    no->setFont(Font(.08, BOLD_STROKE));
    // Put the button on the window.
    window->addControl(no);
}

// Hack for creating the List Question window.
static void CreateControlsForListWindow(Window* window) {

    window->setSizeAndCenter(Size(1.2,1.2));
    window->setColor(GFXColor(.8,.5,.5));

    // Title text display.
    StaticDisplay* title = new StaticDisplay;
    title->setRect( Rect(-.57, .42, 1.14, .15) );
    title->setMultiLine(true);
    title->setJustification(CENTER_JUSTIFY);
    title->setTextColor(GFXColor(GUI_OPAQUE_BLACK));
    title->setColor(GUI_CLEAR);
    title->setFont( Font(.07) );
    title->setId("Title");
    // Put it on the window.
    window->addControl(title);

    // OK button.
    NewButton* ok = new NewButton;
    ok->setRect( Rect(-.27, -.57, .22, .1) );
    ok->setLabel("OK");
    ok->setCommand("OK");
    ok->setColor( GFXColor(.4,.2,.2) );
    ok->setTextColor(GUI_OPAQUE_WHITE);
    ok->setFont(Font(.08, BOLD_STROKE));
    // Put the button on the window.
    window->addControl(ok);

    // Cancel button.
    NewButton* cancel = new NewButton;
    cancel->setRect( Rect(.05, -.57, .22, .1) );
    cancel->setLabel("Cancel");
    cancel->setCommand("Cancel");
    cancel->setColor( GFXColor(.4,.2,.2) );
    cancel->setTextColor(GUI_OPAQUE_WHITE);
    cancel->setFont(Font(.08, BOLD_STROKE));
    // Put the button on the window.
    window->addControl(cancel);

    // List picker.
    SimplePicker* pick = new SimplePicker;
    pick->setRect( Rect(-.57, -.46, 1.14, .85) );
    pick->setColor( GFXColor(0,0,.4, 1.0) );
    pick->setTextColor(GUI_OPAQUE_WHITE);
    pick->setFont( Font(.06) );
    pick->setTextMargins(Size(0.02,0.01));
    pick->setSelectionColor(GFXColor(.4,.6,.4));
    pick->setHighlightColor(GFXColor(.2,.2,.2));
    pick->setHighlightTextColor(GUI_OPAQUE_WHITE);
    pick->setId("Picker");
    window->addControl(pick);
}

// Common code for showing various modal questions.
static void SetTitleAndShowQuestion(ModalDialog* dialog, Window* window, const std::string& title,
                                    ModalDialogCallback* cb, const std::string& id) {
    StaticDisplay* titleControl = dynamic_cast<StaticDisplay*>( window->findControlById("Title") );
    assert(titleControl != NULL);
    titleControl->setText(title);

    dialog->setWindow(window);
    dialog->setCallback(cb, id);
    dialog->run();
}

// Display a modal message to the user.  The message will be shown until the user
//  hits the OK button.
void showAlert(const std::string& title) {
    Window* window = new Window;
    CreateControlsForAlertWindow(window);        // Want to read an XML file for this.

    ModalDialog* dialog = new ModalDialog;
    SetTitleAndShowQuestion(dialog, window, title, NULL, "");
}


// Class that will handle Yes/No dialog correctly.
class YesNoDialog : public ModalDialog
{
    // Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId& command, Control* control);
};
// Process a command from the window.
bool YesNoDialog::processWindowCommand(const EventCommandId& command, Control* control) {
    if(command == "Yes") {
        m_result = YES_ANSWER;
    } else if(command == "No") {
        m_result = NO_ANSWER;
    } else {
        // We only care about buttons clicked.
        return ModalDialog::processWindowCommand(command, control);
    }

    // One of our buttons was clicked.  We're done.
    window()->close();
    return true;
}

// Display a modal yes/no question.
// The result is supplied in the callback.
void showYesNoQuestion(const std::string& title, ModalDialogCallback* cb, const std::string& id) {
    Window* window = new Window;
    CreateControlsForYesNoWindow(window);        // Want to read an XML file for this.

    YesNoDialog* dialog = new YesNoDialog();
    SetTitleAndShowQuestion(dialog, window, title, cb, id);
}


// Class that will handle List Question dialog correctly.
class ListQuestionDialog : public ModalDialog
{
    // Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId& command, Control* control);
};
// Process a command from the window.
bool ListQuestionDialog::processWindowCommand(const EventCommandId& command, Control* control) {
    if(command == "Cancel") {
        // Just close down, with the result  unknown.
        m_result = (-1);
        window()->close();
        return true;
    } else if(command == "OK") {
        // The OK button was clicked.
        Picker* picker = dynamic_cast<Picker*>( window()->findControlById("Picker") );
        assert(picker != NULL);
        m_result = picker->selectedItem();
        if(m_result >= 0) {
            // We have a selection.  We are done.
            window()->close();
        } else {
            // Ignore OK button until we have a selection.
            // Do nothing here.
        }
        return true;
    } else {
        // We only care about buttons clicked.
        return ModalDialog::processWindowCommand(command, control);
    }

    assert(false);      // Should never get here.
}

// Display a modal list of options.
// The result is supplied in the callback.
void showListQuestion(const std::string& title, const std::vector<std::string>& options,
                      ModalDialogCallback* cb, const std::string& id) {
    Window* window = new Window;
    CreateControlsForListWindow(window);        // Want to read an XML file for this.

    // Fill the picker with the choices supplied by the caller.
    SimplePicker* picker = dynamic_cast<SimplePicker*>( window->findControlById("Picker") );
    assert(picker != NULL);
    for(int i=0; i<options.size(); i++) {
        picker->addCell(SimplePickerCell(options[i]));
    }

    ListQuestionDialog* dialog = new ListQuestionDialog();
    SetTitleAndShowQuestion(dialog, window, title, cb, id);
}
