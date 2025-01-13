/*
 * BFXM_to_XMesh.cpp
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
#include "../from_BFXM.h"

using namespace std;

namespace Converter {

class BFXMToXMeshImpl : public ConversionImpl {
public:
    /*
     *     ConversionImpl interface
     */

    virtual RetCodeEnum convert(const std::string &inputFormat,
            const std::string &outputFormat,
            const std::string &opCode) {
        if (inputFormat == "BFXM" && outputFormat == "XMesh") {
            if (opCode == "create") {
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                string basename = input.substr(0, input.find_last_of("."));
                FILE *Inputfile = fopen(input.c_str(), "rb");
                FILE *Outputfile = fopen(output.c_str(), "w+"); //create file for text output
                FILE *OutputObj = NULL;
                FILE *OutputMtl = NULL;
                BFXMToXmeshOrOBJ(Inputfile, Outputfile, OutputObj, OutputMtl, basename, 'x');
                return RC_OK;
            } else {
                return RC_NOT_IMPLEMENTED;
            }
        } else {
            return RC_NOT_IMPLEMENTED;
        }
    }

    virtual void conversionHelp(const std::string &inputFormat,
            const std::string &outputFormat,
            const std::string &opCode) const {
        if ((inputFormat.empty() || inputFormat == "BFXM")
                && (outputFormat.empty() || outputFormat == "XMesh")
                && (opCode.empty() || (opCode == "create"))) {
            cout << "BFXM -> XMesh\n"
                    << "\tSupported operations: create\n"
                    << "\nNotes:\n"
                    << "\tMeshes will be created using the naming rules <submesh>_<lod>.xmesh,\n"
                    << "\twith the exception of 0_0.xmesh, which will be the output file.\n"
                    << "\tIt is recomended, though, that the output file be 0_0.xmesh so that\n"
                    << "\tthe resulting names make sense ;-)\n"
                    << endl;
        }
    }

};

static ConversionImplDeclaration<BFXMToXMeshImpl> __bxh_declaration;

}
