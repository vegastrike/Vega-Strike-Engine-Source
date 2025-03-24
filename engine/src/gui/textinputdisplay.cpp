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

#include "textinputdisplay.h"
#include "root_generic/lin_time.h"
#include "guidefs.h"
using std::vector;
using std::string;

TextInputDisplay::TextInputDisplay(vector<unsigned int> *keyboard_input_queue, const char *disallowed) {
    isFocused = false;
    if (keyboard_input_queue) {
        this->keyboard_queue = keyboard_input_queue;
    } else {
        this->keyboard_queue = &local_keyboard_queue;
    }
    passwordChar = '\0';
    keyboard_input_queue->clear();
    this->disallowed = new char[strlen(disallowed) + 1];
    strcpy(this->disallowed, disallowed);
}

bool TextInputDisplay::processMouseDown(const InputEvent &event) {
    if (event.code != WHEELUP_MOUSE_BUTTON && event.code != WHEELDOWN_MOUSE_BUTTON) {
        //If click is on me, set me focused... otherwise, clear my focus.
        this->isFocused = (hitTest(event.loc));
    }
    return StaticDisplay::processMouseDown(event);
}

void TextInputDisplay::processUnfocus(const InputEvent &event) {
    if (event.code != WHEELUP_MOUSE_BUTTON && event.code != WHEELDOWN_MOUSE_BUTTON) {
        //If click is on me, set me focused... otherwise, clear my focus.
        this->isFocused = false;
    }
    StaticDisplay::processUnfocus(event);
}

bool TextInputDisplay::processKeypress(unsigned int c) {
    return true;
}

void TextInputDisplay::draw() {
    string text = this->text();
    if (!this->isFocused) {
        if (passwordChar) {
            string text1;
            text1.insert(0u, text.length(), passwordChar);
            this->setText(text1);
        }
        this->StaticDisplay::draw();
        if (passwordChar) {
            this->setText(text);
        }
        return;
    }
    size_t LN = keyboard_queue->size();
    for (size_t i = 0; i < LN; ++i) {
        unsigned int c = (*keyboard_queue)[i];
        if (!processKeypress(c)) {
            continue;
        }
        if (c == 8 || c == 127) {
            text = text.substr(0, text.length() - 1);
        } else if (c != '\0' && c < 256) {
            bool allowed = true;
            for (int j = 0; disallowed[j]; ++j) {
                if (c == static_cast<unsigned int>(disallowed[j])) {
                    allowed = false;
                    break;
                }
            }
            if (allowed) {
                char tmp[2] = {0, 0};
                tmp[0] = (char) c;
                text += tmp;
            }
        }
    }
    keyboard_queue->clear();
    unsigned int x = (unsigned int) getNewTime();
    string text1;
    if (passwordChar) {
        text1.insert(0u, text.length(), passwordChar);
    } else {
        text1 = text;
    }
    if (x % 2) {
        text1 += "|";
    }
    this->setText(text1);
    this->StaticDisplay::draw();
    this->setText(text);
}

TextInputDisplay::~TextInputDisplay() {
    delete[] this->disallowed;
}

