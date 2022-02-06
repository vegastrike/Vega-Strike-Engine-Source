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

#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"
#include "../from_BFXM.h"

using namespace std;
using namespace Converter;

class DimsHandler : public Module {
    NameList mNames;

public:
    DimsHandler()
    {
        mNames.push_back("--dims");
        mNames.push_back("-d");
    }

    virtual const NameList &getNames() const
    {
        return mNames;
    }

    virtual void help(const std::string &command, ParameterList &params) const
    {
        if (command == "--dims") {
            cout << "Show mesh's bounding box\n"
                 << "Usage:\n"
                 << "\tmesher (--input|-i) (path) (--dims|-d) (format)\n\n"
                 << "Supported formats:\n"
                 << "\tBFXM\n"
                 << endl;
        } else {
            cerr << "Warning: DimsHandler::help() received an unrecognized command." << endl;
        }
    }

    virtual int execute(const std::string &command, ParameterList &params, unsigned int phase)
    {
        // Skip phase 0
        if (!phase) {
            return 0;
        }
        if (command == "--dims" || command == "-d") {
            if (params.size() && (params[0] == "BFXM")) {
                if (!atoi(getNamedOption("quiet", "0").c_str())) {
                    cerr << "Processing " << getInputPath() << "... " << endl;
                }
                BFXMtoBoxDims(fopen(getInputPath().c_str(), "rb"), getInputPath().c_str());
                params.erase(params.begin());
                return 0;
            } else {
                // Ignore - shouldn't happen anyway.
                cerr << "Error: expected input format, received nothing." << endl;
                return 1;
            }
        } else {
            // Ignore - shouldn't happen anyway.
            cerr << "Warning: DimsHandler::execute() received an unrecognized command." << endl;
            return 0;
        }
    }
};

static Converter::ModuleDeclaration<DimsHandler, false> __dimsh_module_declaration;
