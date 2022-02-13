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
#include "../from_obj.h"

using namespace std;

namespace Converter {

class WavefrontToBFXMImpl : public ConversionImpl {
public:
    /*
     *     ConversionImpl interface
     */

    virtual RetCodeEnum convert(const std::string &inputFormat,
            const std::string &outputFormat,
            const std::string &opCode) {
        if (inputFormat == "Wavefront" && outputFormat == "BFXM") {
            if (opCode == "create") {
                bool forcenormals = atoi(getNamedOption("forcenormals").c_str()) != 0;
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                FILE *Inputfile = fopen(input.c_str(), "r");
                if (!Inputfile) {
                    return RC_INVALID_INPUT;
                }
                string mtl = ObjGetMtl(Inputfile, input.c_str());
                FILE *InputMtl = fopen(mtl.c_str(), "r");
                if (!InputMtl) {
                    cerr << "Warning: material file \"" << mtl.c_str()
                            << "\" not found. Using neutral material." << endl;
                }
                FILE *Outputfile = fopen(output.c_str(), "wb+");
                if (!Outputfile) {
                    fclose(Inputfile);
                    fclose(InputMtl);
                    return RC_INTERNAL_ERROR;
                }
                ObjToBFXM(Inputfile, InputMtl, Outputfile, forcenormals);
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
        if ((inputFormat.empty() || inputFormat == "Wavefront")
                && (outputFormat.empty() || outputFormat == "BFXM")
                && (opCode.empty() || (opCode == "create"))) {
            cout << "Wavefront -> BFXM\n"
                    << "\tSupported operations: create\n"
                    << "\tSupported options: standard, plus:\n"
                    << "\t\t-no-optimize: disables optimization (useful if it has already\n"
                    << "\t\t\tbeen optimized, but make sure the file contains accurate normal\n"
                    << "\t\t\tinformation (disabling optimization will impair autosmooth)\n"
                    << "\nNotes: Wavefront files usually come in pairs, with an .obj and a .mtl\n"
                    << "\tfile. The .mtl file is presumed to have the same base name, but .mtl\n"
                    << "\textension, when the .obj file does not reference it. Otherwise, the\n"
                    << "\tfilename is taken from the .obj file.\n"
                    << "\tSimplifying: have your .mtl file in the same directory as your .obj file.\n"
                    << endl;
        }
    }

};

static ConversionImplDeclaration<WavefrontToBFXMImpl> __wbh_declaration;

}
