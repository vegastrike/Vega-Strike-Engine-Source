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

#ifndef __TEXTINPUTDISPLAY_H__
#define __TEXTINPUTISPLAY_H__

#include "staticdisplay.h"
#include <vector>

// See cpp file for detailed descriptions of classes, functions, etc.

// The TextInputDisplay class is used to get text input from the user.

class TextInputDisplay: public StaticDisplay{
  std::vector <unsigned int> local_keyboard_queue;
  std::vector <unsigned int> *keyboard_queue;
  char * disallowed;
  bool isFocused;
  char passwordChar;
public:
  TextInputDisplay(std::vector <unsigned int> *keyboard_input_queue, const char * disallowed);
  // OVERRIDES - used for focusing input.
  virtual void processUnfocus(const InputEvent& event);
  virtual bool processMouseDown(const InputEvent& event);
  virtual bool processKeypress(unsigned int pressedKey);

  void setPassword(char passchar) { passwordChar = passchar; }
  
  virtual ~TextInputDisplay();
  virtual void draw();
};

#endif   // __TEXTINPUTDISPLAY_H__
