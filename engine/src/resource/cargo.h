/*
 * cargo.h
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
#ifndef VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H
#define VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H

#include "product.h"

#include <string>

class Cargo : public Product {
protected:
    std::string category;       // TODO: move to product
    std::string description;    // TODO: move to product

    float mass;
    float volume;
    bool mission;
    bool installed; // TODO: move down to ShipModule
    float functionality;
    float max_functionality;

    friend class Manifest;
public:
    Cargo();
    Cargo(std::string name, std::string category, float price, int quantity, float mass, float volume,
          float functionality = 1.0f, float max_functionality= 1.0f, bool mission = false, bool installed = false);

    float GetFunctionality();
    float GetMaxFunctionality();
    void SetDescription(const std::string &description);
    void SetFunctionality(float func);
    void SetInstalled(bool installed);
    void SetMaxFunctionality(float func);
    void SetMissionFlag(bool flag);
    void SetPrice(float price);
    void SetMass(float mass);
    void SetVolume(float vol);
    void SetQuantity(int quantity);
    void SetContent(const std::string &content);
    void SetCategory(const std::string &category);

    bool GetMissionFlag() const;
    bool GetInstalled() const;
    const std::string &GetCategory() const;
    const std::string &GetContent() const;
    const std::string &GetDescription() const;
    std::string GetCategoryPython();
    std::string GetContentPython();
    std::string GetDescriptionPython();
    int GetQuantity() const;
    float GetVolume() const;
    float GetMass() const;
    float GetPrice() const;
    bool operator==(const Cargo &other) const;
    bool operator<(const Cargo &other) const;

    // Only for enslave
    // TODO: replace this hack
    int reduce() { quantity--; return quantity; }
};

#endif //VEGA_STRIKE_ENGINE_RESOURCE_CARGO_H
