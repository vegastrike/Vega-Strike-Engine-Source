/**
 * cargo.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#ifndef CARGO_H
#define CARGO_H

#include "SharedPool.h"
#include "gfxlib_struct.h"
#include "product.h"

#include <string>

struct Cargo : public Product {
public:
    //StringPool::Reference content; replaced by name in product
    std::string category;       // TODO: move to product
    std::string description;    // TODO: move to product
    //int quantity;
    //float price;
    float mass;
    float volume;
    bool mission;
    bool installed; // TODO: move down to ShipModule
    float functionality;
    float max_functionality;
    Cargo();
    Cargo(std::string name, std::string category, float price, int quantity, float mass, float volume, float functionality = 1.0f, float max_functionality= 1.0f);

    float GetFunctionality();
    float GetMaxFunctionality();
    void SetFunctionality(float func);
    void SetMaxFunctionality(float func);
    void SetMissionFlag(bool flag);
    void SetPrice(float price);
    void SetMass(float mass);
    void SetVolume(float vol);
    void SetQuantity(int quantity);
    void SetContent(const std::string &content);
    void SetCategory(const std::string &category);

    bool GetMissionFlag() const;
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

};

// A stupid struct that is only for grouping 2 different types of variables together in one return value
// Must come after Cargo for obvious reasons
class CargoColor {
public:
    Cargo cargo;
    GFXColor color;

    CargoColor() : cargo(), color(1, 1, 1, 1) {
    }
};

#endif // CARGO_H
