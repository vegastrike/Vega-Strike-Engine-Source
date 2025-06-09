/*
 * XMesh_to_Ogre.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#include "../to_OgreMesh.h"

#ifdef HAVE_OGRE

namespace Converter {

    class XMeshToOgreImpl : public ConversionImpl
    {
    public:
        /*
         *     ConversionImpl interface
         */

        virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode)
        {
            if (  inputFormat == "XMesh" && outputFormat== "Ogre"  ) {
                if ( opCode == "create" ) {
                    string input = getNamedOption("inputPath");
                    string output= getNamedOption("outputPath");
                    string base  = input.substr(0,input.rfind('.'));
                    string mtl   = base + string(".material");
                    XML memfile=(LoadXML(input.c_str(),1));
                    OgreMeshConverter::ConverterInit();
                    void *data = OgreMeshConverter::Init();
                    OgreMeshConverter::Add(data, memfile);
                    OgreMeshConverter::DoneMeshes(data);
                    OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
                    OgreMeshConverter::ConverterClose();
                    return RC_OK;
                } else if (opCode == "append") {
                    string input = getNamedOption("inputPath");
                    string output= getNamedOption("outputPath");
                    string base  = input.substr(0,input.rfind('.'));
                    string mtl   = base + string(".material");
                    XML memfile=(LoadXML(input.c_str(),1));
                    OgreMeshConverter::ConverterInit();
                    void *data = OgreMeshConverter::Init(output.c_str(), mtl.c_str());
                    OgreMeshConverter::Add(data, memfile);
                    OgreMeshConverter::DoneMeshes(data);
                    OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
                    OgreMeshConverter::ConverterClose();
                    return RC_OK;
                } else if (opCode == "optimize") {
                    string input = getNamedOption("inputPath");
                    string output= getNamedOption("outputPath");
                    string base  = input.substr(0,input.rfind('.'));
                    string mtl   = base + string(".material");
                    OgreMeshConverter::ConverterInit();
                    void *data = OgreMeshConverter::Init(output.c_str(), mtl.c_str());
                    OgreMeshConverter::Optimize(data);
                    OgreMeshConverter::DoneMeshes(data);
                    OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
                    OgreMeshConverter::ConverterClose();
                    return RC_OK;
                } else {
                    return RC_NOT_IMPLEMENTED;
                }
            } else {
                return RC_NOT_IMPLEMENTED;
            }
        }

        virtual void conversionHelp(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode) const
        {
            if (  (inputFormat.empty() || inputFormat == "BFXM")
                &&(outputFormat.empty()|| outputFormat== "Ogre")
                &&(opCode.empty() || (opCode == "create") || (opCode == "append") || (opCode == "optimize"))  )
            {
                std::cout << "BFXM -> Ogre\n"
                     << "\tSupported operations: create, append, optimize\n"
                     << "\nNotes: create and append will work as usual.\n"
                     << "\tThey will produce a .mesh and .material file.\n"
                     << "\toptimize will take the output and optimize it,\n"
                     << "\tignoring its input.\n"
                     << std::endl;
            }
        }

    };

    ConversionImplDeclaration<XMeshToOgreImpl> my_converter_declaration;

}

#endif
