/*
 * resource.h
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
#ifndef VEGA_STRIKE_ENGINE_RESOURCE_RESOURCE_H
#define VEGA_STRIKE_ENGINE_RESOURCE_RESOURCE_H

#include <string>

/**
 * @brief A resource is any part of the game that can be used up and filled up.
 * The purpose of this class is to simplify code throughout the game by placing it here.
 * Instead of adding to health and then checking it isn't above max health, it's
 * done automatically.
 */

template<typename T>
class Resource {
protected:
    T value_;
    T min_value_;
    T max_value_;
    T adjusted_max_value_;

    // TODO: make this const
    bool no_max_;
public:
    Resource(const T &value = 0, const T &min_value = 0, const T &max_value = -1);
    Resource(const std::string input, const T modifier = 1.0, const T minimum_functionality = 0.0);

    const std::string Serialize(const T modifier = 1.0) const;

    //const T operator=(Resource<T> value) const;
    Resource<T> operator=(const T &value);
    Resource<T> operator+=(const T &value);
    Resource<T> operator-=(const T &value);

    //Resource<T> operator=(T &value);
    Resource<T> operator+=(T &value);
    Resource<T> operator-=(T &value);

    // Declare prefix and postfix increment operators.
    // TODO: actually implement pre/postfix increment. Right now both implementations are the same!!!
    // Prefix increment operator.
    Resource<T>& operator++(){
        T tmp_value = value_;
        tmp_value++;
        this->Set(tmp_value);
        return *this;
    }
    // Postfix increment operator.
    Resource<T> operator++(int) {
        T tmp_value = value_;
        tmp_value++;
        this->Set(tmp_value);
        return *this;
    }

    // Declare prefix and postfix decrement operators.
    // Prefix decrement operator.
    Resource<T>& operator--() {
        T tmp_value = value_;
        tmp_value--;
        this->Set(tmp_value);
        return *this;
    }
    // Postfix decrement operator.
    Resource<T> operator--(int) {
        T tmp_value = value_;
        tmp_value--;
        this->Set(tmp_value);
        return *this;
    }

    operator T() { return value_; }

    

    double Percent() const;
    void ResetMaxValue();       // adjusted_max_value_ = max_value_;
    void Set(const T &value);
    void SetToMax();            // value_ = adjusted_max_value_ = max_value_
    void SetMaxValue(const T &value);       // value_ = adjusted_max_value_ = max_value_ = value;
    void SetAdjustedMaxValue(const T &value); // New adjusted max value
    
    void Zero();    // value_ = adjusted_max_value_ = min_value_;

    T Value() const;
    T MaxValue() const;
    T MinValue() const;
    T AdjustedValue() const;

    // Damage & Repair
    void Destroy(); // value_ = adjusted_max_value_ = min_value_;
    bool Destroyed() const;
    void RandomDamage();
    void DamageByValue(const T &value);
    void DamageByPercent(const T &value);
    bool Damaged() const;
    void RepairFully();
    void RepairByValue(const T &value);
    void RepairByPercent(const T &value);

    T* ValuePtr() { return &value_; }
    T* AdjustedMaxValuePtr() { return &adjusted_max_value_; }
    T* MaxValuePtr() { return &max_value_; }
};

template<typename T>
bool operator==(const Resource<T> &lhs, const Resource<T> &rhs);
template<typename T>
bool operator==(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator==(const T &lhs, const Resource<T> &rhs);
template<typename T>
bool operator>(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator<(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator<=(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator>=(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator>(const T &lhs, const Resource<T> &rhs);
template<typename T>
bool operator<(const T &lhs, const Resource<T> &rhs);
template<typename T>
bool operator<=(const T &lhs, const Resource<T> &rhs);
template<typename T>
bool operator>=(const T &lhs, const Resource<T> &rhs);
template<typename T>
T operator/(const Resource<T> &lhs, const T &rhs);
template<typename T>
T operator/(const T &lhs, const Resource<T> &rhs);

#endif //VEGA_STRIKE_ENGINE_RESOURCE_RESOURCE_H
