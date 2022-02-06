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
#include "../to_BFXM.h"

using namespace std;

namespace Converter {

class XMeshToBFXMImpl : public ConversionImpl {
public:
    /*
     *     ConversionImpl interface
     */

    virtual RetCodeEnum convert(const std::string &inputFormat,
                                const std::string &outputFormat,
                                const std::string &opCode)
    {
        if (inputFormat == "XMesh" && outputFormat == "BFXM") {
            if (opCode == "add") {
                bool forcenormals = atoi(getNamedOption("forcenormals").c_str()) != 0;
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                FILE *Outputfile = fopen(output.c_str(),
                                         "rb+"); //append to end, but not append, which doesn't do what you want it to.
                fseek(Outputfile, 0, SEEK_END);
                XML memfile = (LoadXML(input.c_str(), 1));
                xmeshToBFXM(memfile, Outputfile, 'a', forcenormals);
                return RC_OK;
            } else if (opCode == "create") {
                bool forcenormals = atoi(getNamedOption("forcenormals").c_str()) != 0;
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                FILE *Outputfile = fopen(output.c_str(), "wb+"); //create file for BFXM output
                XML memfile = (LoadXML(input.c_str(), 1));
                xmeshToBFXM(memfile, Outputfile, 'c', forcenormals);
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
                                const std::string &opCode) const
    {
        if ((inputFormat.empty() || inputFormat == "XMesh")
                && (outputFormat.empty() || outputFormat == "BFXM")
                && (opCode.empty() || (opCode == "add" || opCode == "create"))) {
            cout << "XMesh -> BFXM\n"
                 << "\tSupported operations: add, create\n"
                 << endl;
        }
    }

};

static ConversionImplDeclaration<XMeshToBFXMImpl> __xbh_declaration;

}
