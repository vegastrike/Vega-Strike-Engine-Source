/*
 * OldSyntax.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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

#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"

#include <cstdlib> // Needed for atoi 

using namespace std;
using namespace Converter;

class OldSyntaxHandler : public Module {
    NameList mSequence;
    NameList mNames;

    NameList::const_iterator mSequencePos;

    string mMode;

public:
    OldSyntaxHandler() {
        mSequence.push_back("--input");
        mSequence.push_back("--output");
        mSequence.push_back("--oldmode");
        mNames.push_back("--oldmode");
        mSequencePos = mSequence.begin();
    }

    virtual const NameList &getNames() const {
        return mNames;
    }

    virtual void help(const std::string &command, ParameterList &params) const {
        cout << "No help associated with " << command << endl;
    }

    virtual int execute(const std::string &command, ParameterList &params, unsigned int phase) {
        if (command == "--oldmode") {
            if (params.size()) {
                mMode = params[0];
                params.erase(params.begin());
                return 0;
            } else {
                // Ignore - shouldn't happen anyway.
                cerr << "Warning: expected mode, received nothing" << endl;
                return 0;
            }
        } else {
            if (phase == 0) {
                if (mSequencePos == mSequence.end()) {
                    // Ignore this... but warn the user.
                    cerr << "Warning: Too many parameters for old syntax" << endl;
                    params.erase(params.begin());
                    return 0;
                } else {
                    params.insert(params.begin(), *mSequencePos);
                    ++mSequencePos;
                    return 0;
                }
            } else {
                // Create execution parameters
                params.clear();
                if (mMode.length() != 3) {
                    cerr << "Error: wrong mode string" << endl;
                    return 1;
                } else {
                    string inf, outf, op;
                    switch (mMode[0]) {
                        case 'o':
                            inf = "Wavefront";
                            break;
                        case 'm':
                            inf = "Ogre";
                            break;
                        case 'b':
                            inf = "BFXM";
                            break;
                        case 'x':
                            inf = "XMesh";
                            break;
                        default:
                            cerr << "Error: unknown input format" << endl;
                    }
                    switch (mMode[1]) {
                        case 'o':
                            outf = "Wavefront";
                            break;
                        case 'm':
                            outf = "Ogre";
                            break;
                        case 'b':
                            outf = "BFXM";
                            break;
                        case 'x':
                            outf = "XMesh";
                            break;
                        default:
                            cerr << "Error: unknown output format" << endl;
                    }
                    switch (mMode[2]) {
                        case 'a':
                            op = "add";
                            break;
                        case 'c':
                            op = "create";
                            break;
                        default:
                            cerr << "Error: unknown operation" << endl;
                    }
                    if (!inf.empty() && (atoi(getNamedOption("dims", "0").c_str()))) {
                        params.push_back("--dims");
                        params.push_back(inf);
                        return 0;
                    } else if (!inf.empty() && !outf.empty() && !op.empty()) {
                        params.push_back("--convert");
                        params.push_back(inf);
                        params.push_back(outf);
                        params.push_back(op);
                        return 0;
                    } else {
                        return 1;
                    }
                }
            }
        }
    }
};

static Converter::ModuleDeclaration<OldSyntaxHandler, true> __osh_module_declaration;
