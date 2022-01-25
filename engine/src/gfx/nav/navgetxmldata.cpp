/**
 * navgetxmldata.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "navgetxmldata.h"

using std::string;

string retrievedata(string data, string type)
{
    int length = data.size();
    if (length == 0) {
        return "";
    }
    if (type.size() == 0) {
        return "";
    }
    char testchar = 'p';
    string teststring = "";

    int counter = 0;
    while (counter < length) {
        if (data[counter] == '=') {
            //find an =
            int tempcounter = counter;
            teststring = "";
            while (tempcounter > 0) {
                //count backwards to find its start
                tempcounter = tempcounter - 1;
                testchar = data[tempcounter];
                if ((testchar == '"') || (tempcounter == 0) || (testchar == ' ')) {
                    //found its start
                    if (tempcounter == 0) {
                        teststring = testchar + teststring;
                    }
                    //if(testchar == ' ')
                    //teststring = ' ' + teststring;

                    //kill spaces at front and back
                    //***********************************
                    int startspaces = 0;
                    int endspaces = teststring.size() - 1;
                    string possibletype = "";
                    while (teststring[startspaces] == ' ') {
                        startspaces += 1;
                    }
                    while (teststring[endspaces] == ' ') {
                        endspaces -= 1;
                    }
                    for (int j = startspaces; j <= endspaces; j++) {
                        possibletype = possibletype + teststring[j];
                    }
                    //***********************************
                    //if match, return the data afterwards
                    //***********************************
                    if (possibletype == type) {
                        size_t returncounter = counter;
                        string returnstring = "";
                        returncounter += 1;                                                         //pass the =
                        while (data[returncounter] == ' ') {
                            returncounter += 1;
                        }
                        returncounter += 1;                                                         //pass the "
                        while (data[returncounter]
                                == ' ') {                                        //pass starting spaces in answer
                            returncounter += 1;
                        }
                        while (returncounter < (data.size() - 1)) {
                            if ((data[returncounter] == '"')
                                    || (data[returncounter] == ' ')) {   //read upto the second comment
                                break;
                            }
                            returnstring = returnstring + data[returncounter];
                            returncounter += 1;
                        }
                        return returnstring;
                    }
                        //***********************************
                    else {
                        break;
                    }
                    break;
                }
                teststring = testchar + teststring;
            }
        }
        counter += 1;
    }
    return "";
}

