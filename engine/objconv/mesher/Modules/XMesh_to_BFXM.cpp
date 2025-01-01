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

#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace std;

namespace Converter {

/**
 * Helper function to parse forceNormals from the named options safely.
 * Returns `true` if the 'forcenormals' option exists and is non-zero, false otherwise.
 */
static bool parseForceNormals() {
    std::string forceNormalsStr = getNamedOption("forcenormals");
    if (forceNormalsStr.empty()) {
        return false;
    }
    // Safely parse integer value
    try {
        return (std::stoi(forceNormalsStr) != 0);
    } catch (const std::exception &e) {
        std::cerr << "[Warning] Could not parse 'forcenormals' option: "
                  << e.what() << ". Defaulting to false.\n";
        return false;
    }
}

/**
 * Helper function to load the XML file safely. Throws on error.
 */
static XML loadXMLFile(const std::string &inputPath) {
    if (inputPath.empty()) {
        throw std::runtime_error("Input path is empty. Cannot load XML.");
    }
    XML memfile = LoadXML(inputPath.c_str(), 1);
    // If your LoadXML can return invalid or nullptr, you might want to check that here
    // e.g. if (!memfile.isValid()) { throw ... }
    return memfile;
}

/**
 * Helper function to open a file with the specified mode. Throws on failure.
 */
static std::unique_ptr<FILE, decltype(&fclose)>
openFile(const std::string &path, const std::string &mode) {
    FILE *fp = fopen(path.c_str(), mode.c_str());
    if (!fp) {
        throw std::runtime_error("Failed to open file: " + path + " with mode: " + mode);
    }
    // Use a custom deleter so the file is automatically closed
    return std::unique_ptr<FILE, decltype(&fclose)>(fp, &fclose);
}

/**
 * Implementation of the XMesh to BFXM conversion logic.
 */
class XMeshToBFXMImpl : public ConversionImpl {
public:
    /*
     * ConversionImpl interface
     */
    virtual RetCodeEnum convert(const std::string &inputFormat,
                                const std::string &outputFormat,
                                const std::string &opCode) override {

        if (inputFormat != "XMesh" || outputFormat != "BFXM") {
            return RC_NOT_IMPLEMENTED;
        }

        bool forceNormals = parseForceNormals();
        std::string inputPath  = getNamedOption("inputPath");
        std::string outputPath = getNamedOption("outputPath");

        // Check for basic path validity
        if (inputPath.empty() || outputPath.empty()) {
            std::cerr << "[Error] inputPath or outputPath is empty.\n";
            return RC_ERROR;
        }

        // Decide operation
        if (opCode == "add") {
            // Append to existing BFXM file
            try {
                auto outputFile = openFile(outputPath, "rb+");
                // Move file pointer to the end
                fseek(outputFile.get(), 0, SEEK_END);

                XML memfile = loadXMLFile(inputPath);
                xmeshToBFXM(memfile, outputFile.get(), 'a', forceNormals);

                return RC_OK;
            } catch (const std::exception &ex) {
                std::cerr << "[Error] 'add' operation failed: " << ex.what() << std::endl;
                return RC_ERROR;
            }

        } else if (opCode == "create") {
            // Create (or overwrite) BFXM file
            try {
                auto outputFile = openFile(outputPath, "wb+");
                XML memfile = loadXMLFile(inputPath);
                xmeshToBFXM(memfile, outputFile.get(), 'c', forceNormals);

                return RC_OK;
            } catch (const std::exception &ex) {
                std::cerr << "[Error] 'create' operation failed: " << ex.what() << std::endl;
                return RC_ERROR;
            }

        } else {
            return RC_NOT_IMPLEMENTED;
        }
    }

    virtual void conversionHelp(const std::string &inputFormat,
                                const std::string &outputFormat,
                                const std::string &opCode) const override {
        // If called with partial or matching arguments, show usage
        if ((inputFormat.empty()  || inputFormat  == "XMesh") &&
            (outputFormat.empty() || outputFormat == "BFXM") &&
            (opCode.empty()       || (opCode == "add" || opCode == "create"))) {
            std::cout << "XMesh -> BFXM\n"
                      << "\tSupported operations: add, create\n"
                      << std::endl;
        }
    }
};

// Registration / declaration for this converter
static ConversionImplDeclaration<XMeshToBFXMImpl> __xbh_declaration;

} // namespace Converter
