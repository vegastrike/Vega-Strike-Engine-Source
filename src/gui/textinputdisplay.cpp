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

#include "textinputdisplay.h"
#include "lin_time.h"

using namespace std;

TextInputDisplay::TextInputDisplay(std::vector <unsigned int> *keyboard_input_queue, const char * disallowed) {
  if (keyboard_input_queue) {
    this->keyboard_queue=keyboard_input_queue;
  }else {
    this->keyboard_queue=&local_keyboard_queue;
  }
  keyboard_input_queue->clear();
  this->disallowed= new char[strlen(disallowed)+1];
  strcpy(this->disallowed, disallowed);
}

void TextInputDisplay::draw() {
  std::string text = this->text();
  size_t LN = keyboard_queue->size();
  for (size_t i=0;i<LN;++i) {
    unsigned int c=(*keyboard_queue)[i];
    if (c==8||c==127) {
      text=text.substr(0,text.length()-1);
    }else if (c!='\0'&&c<256) {
      bool allowed=true;
      for (int j=0;disallowed[j];++j) {
	if (c==disallowed[j]) {
	  allowed=false;
	  break;
	}
      }
      if (allowed ) {
	char tmp[2]={0,0};
	tmp[0]=(char)c;
	text+=tmp;
      }				
    }
  }
  keyboard_queue->clear();
  unsigned int x= (unsigned int)getNewTime();
  string text1=text;
  if (x%2) {
    text1+="|";
  }
  this->setText(text1);
  this->StaticDisplay::draw();
  this->setText(text);
}

TextInputDisplay::~TextInputDisplay() {
  delete this->disallowed;
}
