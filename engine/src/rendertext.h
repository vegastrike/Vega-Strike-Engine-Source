/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
// NO HEADER GUARD

#include <string>
#include <vector>
#include <sstream>

class RText {
private:
//text rendering
    struct cline {
        cline() {
            outtime = 0;
        }

        cline(const cline &in) {
            if (in.cref.size() > 0) {
                cref.append(in.cref);
            }
            outtime = in.outtime;
        }

        std::string cref;
        int outtime;
    };
    std::vector<cline> conlines;
    std::vector<std::string> vhistory;
    int ndraw;
    int WORDWRAP;
    int conskip;
    int histpos;
    std::string commandbuf;
public:
    RText();
    virtual ~RText();
    std::string getcurcommand();
    int text_width(char *str);           //set the text width?
    void draw_text(std::string &str, float left, float top, int gl_num);           //creates textplane object
    void renderconsole();             //renders the text in the console
    void conline(std::string &sf, bool highlight);           //add a line to
//the console
    void conoutf(char *);
    virtual void conoutf(std::string &s, int a = 0, int b = 0, int c = 0);
//add a line to the console(Use this one.)
    void saycommand(const char *init);           //actually does the appending of
//the string to the commandbuf, and seperates entries
    void ConsoleKeyboardI(int code, bool isdown);
//interpret keyboard input to the console
    void conoutn(std::string &in, int a, int b, int c);
};

