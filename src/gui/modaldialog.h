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

#ifndef __MODALDIALOG_H__
#define __MODALDIALOG_H__

#include "windowcontroller.h"


// Callback called when modal dialog closes.
// The callback has 3 parameters:
//  1. The controller.  This can be used to interrogate the controller or the
//     controls in the window for more complex dialogs.
//  2. The string "id".  This is specified by the dialog creator to help handling
//     code identify which dialog is responding.
//  3. The integer "result".  A simple result from the dialog.  More complex
//     results could be obtained from the window.
// The callback returns true if the window is to be closed, false otherwise
// (it will then return true on a later callback).
class ModalDialogCallback
{
public:
    virtual bool modalDialogResult(
        const std::string& id,
        int result,
        WindowController& controller
        ) = 0;
};


// This class controls a modal "dialog" with the user.  It puts up a modal
//  window, gets an answer, and calls a callback function.
// Like all WindowController's, by default, this object deletes itself when
//  the window closes.
// Note that this class requires a callback because there is no access to the
//  GLUT event loop.  The common way to implement modal UI is to recursively
//  call the event loop while the modal UI is showing, so that the UI can
//  return directly back to the routine that called it.  Can't do that in GLUT,
//  so there is no way to return a value to the caller.  The caller must
//  eventually exit back to the event loop to get the modal UI serviced.
class ModalDialog : public WindowController
{
public:
    // The class that gets called when the window closes.
    virtual void setCallback(ModalDialogCallback* cb, const std::string& id);

    // Set up the window and get everything ready.
    virtual void init(void) {};

    // Start everything up.
    virtual void run(void);

    // Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId& command, Control* control);

    // CONSTRUCTION
    ModalDialog();
    virtual ~ModalDialog(void) {};

protected:

    // VARIABLES
    std::string m_callbackId;           // "Id" to pass callback function.
    ModalDialogCallback* m_callback;    // The callback class.
    int m_result;                       // A simple result for the dialog.
};

// Display a modal message to the user.  The message will be shown until the user
//  hits the OK button.
void showAlert(const std::string& title);

// Display a modal yes/no question.
// The result is supplied in the callback.
void showYesNoQuestion(const std::string& title, ModalDialogCallback* cb, const std::string& id);
static const int YES_ANSWER = 1;
static const int NO_ANSWER = 0;

// Display a modal list of options.
// The result is supplied in the callback.
void showListQuestion(const std::string& title, const std::vector<std::string>& options,
                      ModalDialogCallback* cb, const std::string& id);

#endif   // __MODALDIALOG_H__
