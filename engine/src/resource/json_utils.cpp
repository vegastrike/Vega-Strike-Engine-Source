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
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
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

/**
 * @class XMeshToBFXMImpl
 * @brief Handles conversion between the XMesh and BFXM formats.
 *
 * XMESH is a textual or XML-like format for representing 3D geometry, 
 * materials, and hierarchical structure in an easily editable way.
 * BFXM is a more compact, binary-based 3D model exchange format
 * used by the engine for efficient loading and rendering.
 *
 * This class implements the logic to parse an XMesh data structure
 * and produce a BFXM-based output file, optionally appending or
 * creating a fresh BFXM file depending on the opCode requested.
 */
class XMeshToBFXMImpl : public ConversionImpl {
public:
    /*
     * ConversionImpl interface
     */

    virtual RetCodeEnum convert(const std::string &inputFormat,
                                const std::string &outputFormat,
                                const std::string &opCode) {
        if (inputFormat == "XMesh" && outputFormat == "BFXM") {
            if (opCode == "add") {
                bool forcenormals = atoi(getNamedOption("forcenormals").c_str()) != 0;
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                FILE *Outputfile = fopen(output.c_str(),
                                         "rb+"); // append to end, but not append, which doesn't do what you want it to.
                fseek(Outputfile, 0, SEEK_END);
                XML memfile = (LoadXML(input.c_str(), 1));
                xmeshToBFXM(memfile, Outputfile, 'a', forcenormals);
                return RC_OK;
            } else if (opCode == "create") {
                bool forcenormals = atoi(getNamedOption("forcenormals").c_str()) != 0;
                string input = getNamedOption("inputPath");
                string output = getNamedOption("outputPath");
                FILE *Outputfile = fopen(output.c_str(), "wb+"); // create file for BFXM output
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
                                const std::string &opCode) const {
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

} // namespace Converter

/*
 * json_utils.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include <string>
#include <vector>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

/**
 * @brief Retrieve a nested JSON value, given a pipe-delimited path.
 *        If no key is found at any step, returns an empty JSON value.
 * @param object   The root JSON object.
 * @param key      A string containing one or more subkeys, delimited by '|'.
 * @return boost::json::value   The found value, or an empty value on failure.
 */
static boost::json::value JsonGetValue(const boost::json::object& object,
                                       const std::string& key)
{
    std::vector<std::string> key_sections;
    boost::split(key_sections, key, boost::is_any_of("|"));

    const boost::json::object* current_obj = &object;
    for (size_t i = 0; i < key_sections.size(); ++i) {
        const auto& k = key_sections[i];

        if (auto it = current_obj->find(k); it != current_obj->end()) {
            // If this is the final part of the path, return the value
            if (i == key_sections.size() - 1) {
                return it->value();
            }
            // Otherwise, we expect this to be another nested object
            if (it->value().is_object()) {
                current_obj = &it->value().get_object();
            } else {
                // Path ended with a non-object before the last subkey
                return {};
            }
        } else {
            // Key not found
            return {};
        }
    }
    // If we somehow get here, return empty
    return {};
}

/**
 * @brief Retrieve a string from a JSON object by key, or return a default value.
 * @param object         The JSON object to search.
 * @param key            Pipe-delimited path, e.g. "foo|bar".
 * @param default_value  A fallback string if not found or not convertible.
 * @return std::string
 */
std::string JsonGetStringWithDefault(const boost::json::object& object,
                                     const std::string& key,
                                     const char* default_value)
{
    const boost::json::value val = JsonGetValue(object, key);
    if (!val.is_null()) {
        try {
            return boost::json::value_to<std::string>(val);
        } catch (...) {
            // In case of type mismatch
        }
    }
    return default_value;
}

/**
 * @brief Retrieve a boolean from a JSON object by key, or return a default value.
 * @param object         The JSON object to search.
 * @param key            Pipe-delimited path.
 * @param default_value  Fallback if not found or conversion fails.
 */
bool GetBool(const boost::json::object& object,
             const std::string& key,
             bool default_value)
{
    const boost::json::value val = JsonGetValue(object, key);
    if (!val.is_null()) {
        try {
            return boost::json::value_to<bool>(val);
        } catch (...) {
            // Type mismatch
        }
    }
    return default_value;
}

/**
 * @brief Retrieve a double from a JSON object by key, or return a default value.
 * @param object         The JSON object to search.
 * @param key            Pipe-delimited path.
 * @param default_value  Fallback if not found or conversion fails.
 */
double GetDouble(const boost::json::object& object,
                 const std::string& key,
                 double default_value)
{
    const boost::json::value val = JsonGetValue(object, key);
    if (!val.is_null()) {
        try {
            return boost::json::value_to<double>(val);
        } catch (...) {
            // Type mismatch
        }
    }
    return default_value;
}

/**
 * @brief Initial support for retrieving integer values from JSON (non-string).
 * @param object         The JSON object to search.
 * @param key            Pipe-delimited path, e.g. "some|nested|int".
 * @param default_value  Value to return if not found or conversion fails.
 * @return int  The integer value, or the default value on error.
 */
int GetInt(const boost::json::object& object,
           const std::string& key,
           int default_value)
{
    const boost::json::value val = JsonGetValue(object, key);
    if (!val.is_null()) {
        try {
            // If the JSON value is a double, we can convert it to int
            // (or throw an exception if it is out of range).
            double asDouble = boost::json::value_to<double>(val);
            return static_cast<int>(asDouble);
        } catch (...) {
            // Type mismatch or out_of_range
        }
    }
    return default_value;
}
