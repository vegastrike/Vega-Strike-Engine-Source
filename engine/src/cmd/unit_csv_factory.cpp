/*
 * unit_csv_factory.cpp
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


#include "unit_csv_factory.h"

#include <iostream>
#include <vector>
#include <string>

// Required definition of static variable
std::map<std::string, std::map<std::string, std::string>> UnitCSVFactory::units;

// This is probably unique enough to ensure no collision
std::string UnitCSVFactory::DEFAULT_ERROR_VALUE = "UnitCSVFactory::_GetVariable DEFAULT_ERROR_VALUE";

void ExtractColumns(std::string &line) {
    std::string data(line);
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;

    while ((pos = data.find(delimiter)) != std::string::npos) {
        token = data.substr(0, pos);
        data.erase(0, pos + delimiter.length());
    }
}

int line_num = 1;

/**
 * @brief ProcessLine is a slightly complicated CSV parsing, as it
 * needs to account for quotes and commas within these quotes.
 * This code won't work if there are quotes within quotes.
 * @param line - the input string
 */
std::vector<std::string> ProcessLine(std::string &line) {
    std::string data(line);
    std::vector<std::string> cells;

    std::string token;

    size_t comma_index = data.find(",");
    size_t quote_index = data.find("\"");

    while (comma_index != std::string::npos) {
        // Start quote
        if (quote_index < comma_index) {
            // End quote
            quote_index = data.find("\"", quote_index + 1);
            // Comma after quote
            comma_index = data.find(",", quote_index);
        }

        token = data.substr(0, comma_index);
        data.erase(0, comma_index + 1);

        // If token starts and ends with a quote, remove them
        if (token[0] == '"' && token[token.size() - 1] == '"') {
            token = token.substr(1, token.size() - 2);
        }

        cells.push_back(token);

        comma_index = data.find(",");
        quote_index = data.find("\"");
    }

    token = data.substr(0, comma_index);
    cells.push_back(token);

    return cells;
}

void UnitCSVFactory::ParseCSV(VSFileSystem::VSFile &file, bool saved_game) {
    std::vector<std::string> columns;
    std::string data = file.ReadFull();
    std::string delimiter = "\n";
    size_t pos = 0;
    std::string token;
    bool first_line = true;

    line_num = 1;

    // Add newline to end of file, so last line will be processed.
    if (!data.empty() && data.back() != '\n') {
        data.append("\n");
    }

    while ((pos = data.find(delimiter)) != std::string::npos) {
        token = data.substr(0, pos);
        if (first_line) {
            columns = ProcessLine(token);

            first_line = false;
        } else {

            std::vector<std::string> line = ProcessLine(token);
            std::map<std::string, std::string> unit_attributes;

            for (unsigned int i = 1; i < columns.size(); i++) {
                unit_attributes[columns[i]] = line[i];
            }

            // Add root
            unit_attributes["root"] = file.GetRoot();

            std::string key = (saved_game ? "player_ship" : line[0]);

            if(!key.empty()) {
                UnitCSVFactory::units[key] = unit_attributes;
            }
        }
        data.erase(0, pos + delimiter.length());
    }
}


// TODO: place this somewhere else with similar code from unit_csv
std::string GetUnitKeyFromNameAndFaction(const std::string unit_name, const std::string unit_faction) {
    std::string hash_name = unit_name + "__" + unit_faction;

    if(UnitCSVFactory::HasUnit(hash_name)) {
        return hash_name;
    }

    if(UnitCSVFactory::HasUnit(unit_name)) {
        return unit_name;
    }

    return std::string();
}
