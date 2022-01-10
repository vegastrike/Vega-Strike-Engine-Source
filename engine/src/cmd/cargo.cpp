/**
 * cargo.cpp
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


#include "cargo.h"

#include "unit_generic.h"

Cargo::Cargo()
{
    mass = 0;
    volume = 0;
    price = 0;
    quantity = 1;
    mission = false;
    installed = false;
    functionality = maxfunctionality = 1.0f;
}

Cargo::Cargo(std::string name, std::string cc, float pp, int qq, float mm, float vv, float func, float maxfunc) :
        content(name), category(cc)
{
    quantity = qq;
    price = pp;
    mass = mm;
    volume = vv;
    mission = false;
    installed = false;
    functionality = func;
    maxfunctionality = maxfunc;
}

Cargo::Cargo(std::string name, std::string cc, float pp, int qq, float mm, float vv) :
        content(name), category(cc)
{
    quantity = qq;
    price = pp;
    mass = mm;
    volume = vv;
    mission = false;
    installed = false;
    functionality = maxfunctionality = 1.0f;
}

float Cargo::GetFunctionality()
{
    return functionality;
}

float Cargo::GetMaxFunctionality()
{
    return maxfunctionality;
}

void Cargo::SetFunctionality(float func)
{
    functionality = func;
}

void Cargo::SetMaxFunctionality(float func)
{
    maxfunctionality = func;
}

void Cargo::SetMissionFlag(bool flag)
{
    this->mission = flag;
}

void Cargo::SetPrice(float price)
{
    this->price = price;
}

void Cargo::SetMass(float mass)
{
    this->mass = mass;
}

void Cargo::SetVolume(float vol)
{
    this->volume = vol;
}

void Cargo::SetQuantity(int quantity)
{
    this->quantity = quantity;
}

void Cargo::SetContent(const std::string &content)
{
    this->content = content;
}

void Cargo::SetCategory(const std::string &category)
{
    this->category = category;
}

bool Cargo::GetMissionFlag() const
{
    return this->mission;
}

const std::string &Cargo::GetCategory() const
{
    return category;
}

const std::string &Cargo::GetContent() const
{
    return content;
}

const std::string &Cargo::GetDescription() const
{
    return description;
}

std::string Cargo::GetCategoryPython()
{
    return GetCategory();
}

std::string Cargo::GetContentPython()
{
    return GetContent();
}

std::string Cargo::GetDescriptionPython()
{
    return GetDescription();
}

int Cargo::GetQuantity() const
{
    return quantity;
}

float Cargo::GetVolume() const
{
    return volume;
}

float Cargo::GetMass() const
{
    return mass;
}

float Cargo::GetPrice() const
{
    return price;
}

bool Cargo::operator==(const Cargo &other) const
{
    return content == other.content;
}

bool Cargo::operator<(const Cargo &other) const
{
    return (category == other.category) ? (content < other.content) : (category < other.category);
}


