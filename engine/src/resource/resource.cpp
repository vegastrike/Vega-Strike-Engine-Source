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

#include "resource.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "random_utils.h"


template<typename T>
T _convert(const std::string input, const T modifier);

template<>
float _convert<float>(const std::string input, const float modifier) {
    try {
        return std::stod(input) * modifier;
    } catch(...) {
        return 0.0;
    }
}


template<>
double _convert<double>(const std::string input, const double modifier) {
    try {
        return std::stod(input) * modifier;
    } catch(...) {
        return 0.0;
    }
}

template<>
int _convert<int>(const std::string input, const int modifier) {
    try {
        return std::stoi(input) * modifier;
    } catch(...) {
        return 0;
    }
}

template<>
long _convert<long>(const std::string input, const long modifier) {
    try {
        return std::stol(input) * modifier;
    } catch(...) {
        return 0l;
    }
}



/*
 * Constructors
 */
template<typename T>
Resource<T>::Resource(const T &value, const T &min_value, const T &max_value):
        value_(value),
        min_value_(min_value),
        max_value_(max_value),
        adjusted_max_value_(max_value),
        no_max_(max_value==-1) {}

template<typename T>
Resource<T>::Resource(const std::string input, const T modifier, const T minimum_functionality):
        value_(0),
        min_value_(0),
        max_value_(1),
        adjusted_max_value_(1),
        no_max_(false) {
    if(input.empty()) {
        value_ = max_value_ = adjusted_max_value_ = 0.0;
        return;
    }

    std::vector<std::string> result; 
    boost::split(result, input, boost::is_any_of("/"));

    switch(result.size()) {
        case 1:
            value_ = max_value_ = adjusted_max_value_ = _convert(result[0], modifier);
            break;
        case 2:
            value_ = adjusted_max_value_ = _convert(result[0], modifier);
            max_value_ = _convert(result[1], modifier);
            break;
        case 3:
            value_ = _convert(result[0], modifier);
            adjusted_max_value_ = _convert(result[1], modifier);
            max_value_ = _convert(result[2], modifier);
            break;
        default:
            value_ = max_value_ = adjusted_max_value_ = 0.0;
    }

    min_value_ = max_value_ * minimum_functionality;
}

/*
 * Methods
 */
template<>
const std::string Resource<double>::Serialize(const double modifier) const {
    return (boost::format("%1$.2f/%2$.2f/%3$.2f") % (value_/modifier) %
            (adjusted_max_value_/modifier) % (max_value_/modifier)).str();
}

template<>
const std::string Resource<int>::Serialize(const int modifier) const {
    return (boost::format("%1%/%2%/%3%") % (value_/modifier) %
            (adjusted_max_value_/modifier) % (max_value_/modifier)).str();
}

template<>
const std::string Resource<long>::Serialize(const long modifier) const {
    return (boost::format("%1%/%2%/%3%") % (value_/modifier) %
            (adjusted_max_value_/modifier) % (max_value_/modifier)).str();
}


template<typename T>
double Resource<T>::Percent() const {
    if(no_max_) {   // Can't calculate percent if there's no max
        return 0;
    }

    if(max_value_ == 0) {   // Can't calculate percent if divider is 0
        return 0;
    }

    return static_cast<double>(value_) / static_cast<double>(max_value_);
}

template<typename T>
void Resource<T>::ResetMaxValue() {
    if(no_max_) {   // Can't reset max if there's no max
        return;
    }

    adjusted_max_value_ = max_value_;
}

template<typename T>
void Resource<T>::Set(const T &value) {
    value_ = value;
    if(!no_max_) {
        value_ = std::min(adjusted_max_value_, value_);
    }
    value_ = std::max(min_value_, value_);
}

template<typename T>
void Resource<T>::SetToMax() {
    if(no_max_) {   // Can't set to max if there's no max
        return;
    }

    value_ = adjusted_max_value_ = max_value_;
}

template<typename T>
void Resource<T>::SetMaxValue(const T &value) {
    if(no_max_) {   // Can't set max if there's no max
        return;
    }

    value_ = adjusted_max_value_ = max_value_ = value;
}

template<typename T>
void Resource<T>::SetAdjustedMaxValue(const T &value) {
    T v = value;

    if(no_max_) {   // Can't set max if there's no max
        return;
    }

    v = std::max(min_value_, v);
    v = std::min(max_value_, v);

    adjusted_max_value_ = v;

    value_ = std::min(value_, adjusted_max_value_);
}

template<typename T>
T Resource<T>::Value() const {
    return value_;
}

template<typename T>
T Resource<T>::MaxValue() const {
    return max_value_;
}

template<typename T>
T Resource<T>::MinValue() const {
    return min_value_;
}

template<typename T>
T Resource<T>::AdjustedValue() const {
    return adjusted_max_value_;
}

template<typename T>
void Resource<T>::Zero() {
    value_ = adjusted_max_value_ = min_value_;
}

// Damage & Repair
template<typename T>
void Resource<T>::Destroy() {
    value_ = adjusted_max_value_ = min_value_;
}
    
template<typename T>
bool Resource<T>::Destroyed() const {
    return adjusted_max_value_ == min_value_;
}

template<typename T>
void Resource<T>::RandomDamage() {
    // Can't damage a destroyed resource
    if(Destroyed()) {
        return;
    }

    const double severity = randomDouble();
    DamageByPercent(severity);
}

template<typename T>
void Resource<T>::DamageByValue(const T &value) {
    if(no_max_) {   // Can't downgrade if there's no max
        return;
    }

    adjusted_max_value_ = std::max(min_value_, adjusted_max_value_ - value);
    value_ = std::min(value_, adjusted_max_value_);
}

template<typename T>
void Resource<T>::DamageByPercent(const T &value) {
    if(no_max_) {   // Can't downgrade if there's no max
        return;
    }

    if(value > .95) {
        Destroy();
    } else {
        DamageByValue(max_value_ * value);
    }
}

template<typename T>
bool Resource<T>::Damaged() const {
    return adjusted_max_value_ < max_value_;
}

// TODO: partial repair
template<typename T>
void Resource<T>::RepairFully() {
    // Can't fix a destroyed resource
    if(Destroyed()) {
        return;
    }

    value_ = adjusted_max_value_ = max_value_;
}

template<typename T>
void Resource<T>::RepairByValue(const T &value) {
    // Can't fix a destroyed resource
    if(Destroyed()) {
        return;
    }

    if(no_max_) {   // Can't upgrade max if there's no max
        return;
    }

    adjusted_max_value_ = std::min(max_value_, adjusted_max_value_ + value);
    value_ = adjusted_max_value_;
}

template<typename T>
void Resource<T>::RepairByPercent(const T &value) {
    // Can't fix a destroyed resource
    if(Destroyed()) {
        return;
    }

    if(no_max_) {   // Can't upgrade max if there's no max
        return;
    }

    adjusted_max_value_ = std::min(max_value_, adjusted_max_value_ + (max_value_ * value));
    value_ = adjusted_max_value_;
}

/*
 * Overloaded operators
 */

/*template<typename T>
const T Resource<T>::operator=(Resource<T> value) const {
    return value.value_;
}*/


template<typename T>
Resource<T> Resource<T>::operator=(const T &value) {
    value_ = value;
    if(!no_max_) {
        value_ = std::min(adjusted_max_value_, value_);
    }
    value_ = std::max(min_value_, value_);

    return *this;
}

template<typename T>
Resource<T> Resource<T>::operator+=(const T &value) {
    if(!no_max_) {   // Only applicable if there's max
        value_ = std::min(value_ + value, adjusted_max_value_);
    } else {
        value_ += value;
    }

    return *this;
}

template<typename T>
Resource<T> Resource<T>::operator-=(const T &value) {
    value_ = std::max(value_ - value, min_value_);
    return *this;
}


template<typename T>
Resource<T> Resource<T>::operator+=(T &value) {
    if(!no_max_) {   // Only applicable if there's max
        value_ = std::min(value_ + value, adjusted_max_value_);
    } else {
        value_ += value;
    }

    return *this;
}

template<typename T>
Resource<T> Resource<T>::operator-=(T &value) {
    value_ = std::max(value_ - value, min_value_);
    return *this;
}

template<typename T>
bool operator==(const Resource<T> &lhs, const Resource<T> &rhs) {
    return lhs.Value() == rhs.Value();
}

template<typename T>
bool operator==(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() == rhs;
}

template<typename T>
bool operator==(const T &lhs, const Resource<T> &rhs) {
    return lhs == rhs.Value();
}

template<typename T>
bool operator>(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() > rhs;
}

template<typename T>
bool operator<(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() < rhs;
}

template<typename T>
bool operator<=(const Resource<T> &lhs, const T &rhs) {
    return !(lhs.Value() > rhs);
}

template<typename T>
bool operator>=(const Resource<T> &lhs, const T &rhs) {
    return !(lhs.Value() < rhs);
}

template<typename T>
bool operator>(const T &lhs, const Resource<T> &rhs) {
    return lhs > rhs.Value();
}

template<typename T>
bool operator<(const T &lhs, const Resource<T> &rhs) {
    return lhs < rhs.Value();
}

template<typename T>
bool operator<=(const T &lhs, const Resource<T> &rhs) {
    return !(lhs > rhs.Value());
}

template<typename T>
bool operator>=(const T &lhs, const Resource<T> &rhs) {
    return !(lhs < rhs.Value());
}

template<typename T>
T operator/(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() / rhs;
}

template<typename T>
T operator/(const T &lhs, const Resource<T> &rhs) {
    return lhs / rhs.Value();
}

// Template Instantiation
// Must come after definition in clang!
template
class Resource<float>;

template bool operator==(const Resource<float> &lhs, const Resource<float> &rhs);
template bool operator==(const Resource<float> &lhs, const float &rhs);
template bool operator>(const Resource<float> &lhs, const float &rhs);
template bool operator<(const Resource<float> &lhs, const float &rhs);
template bool operator<=(const Resource<float> &lhs, const float &rhs);
template bool operator>=(const Resource<float> &lhs, const float &rhs);
template bool operator==(const float &lhs, const Resource<float> &rhs);
template bool operator>(const float &lhs, const Resource<float> &rhs);
template bool operator<(const float &lhs, const Resource<float> &rhs);
template bool operator<=(const float &lhs, const Resource<float> &rhs);
template bool operator>=(const float &lhs, const Resource<float> &rhs);
template float operator/(const Resource<float> &lhs, const float &rhs);
template float operator/(const float &lhs, const Resource<float> &rhs);




template
class Resource<double>;

template bool operator==(const Resource<double> &lhs, const Resource<double> &rhs);
template bool operator==(const Resource<double> &lhs, const double &rhs);
template bool operator>(const Resource<double> &lhs, const double &rhs);
template bool operator<(const Resource<double> &lhs, const double &rhs);
template bool operator<=(const Resource<double> &lhs, const double &rhs);
template bool operator>=(const Resource<double> &lhs, const double &rhs);
template bool operator==(const double &lhs, const Resource<double> &rhs);
template bool operator>(const double &lhs, const Resource<double> &rhs);
template bool operator<(const double &lhs, const Resource<double> &rhs);
template bool operator<=(const double &lhs, const Resource<double> &rhs);
template bool operator>=(const double &lhs, const Resource<double> &rhs);
template double operator/(const Resource<double> &lhs, const double &rhs);
template double operator/(const double &lhs, const Resource<double> &rhs);

template
class Resource<int>;

template bool operator==(const Resource<int> &lhs, const Resource<int> &rhs);
template bool operator==(const Resource<int> &lhs, const int &rhs);
template bool operator>(const Resource<int> &lhs, const int &rhs);
template bool operator<(const Resource<int> &lhs, const int &rhs);
template bool operator<=(const Resource<int> &lhs, const int &rhs);
template bool operator>=(const Resource<int> &lhs, const int &rhs);
template bool operator==(const int &lhs, const Resource<int> &rhs);
template bool operator>(const int &lhs, const Resource<int> &rhs);
template bool operator<(const int &lhs, const Resource<int> &rhs);
template bool operator<=(const int &lhs, const Resource<int> &rhs);
template bool operator>=(const int &lhs, const Resource<int> &rhs);
template int operator/(const Resource<int> &lhs, const int &rhs);
template int operator/(const int &lhs, const Resource<int> &rhs);

template
class Resource<long>;

template bool operator==(const Resource<long> &lhs, const Resource<long> &rhs);
template bool operator==(const Resource<long> &lhs, const long &rhs);
template bool operator>(const Resource<long> &lhs, const long &rhs);
template bool operator<(const Resource<long> &lhs, const long &rhs);
template bool operator<=(const Resource<long> &lhs, const long &rhs);
template bool operator>=(const Resource<long> &lhs, const long &rhs);
template bool operator==(const long &lhs, const Resource<long> &rhs);
template bool operator>(const long &lhs, const Resource<long> &rhs);
template bool operator<(const long &lhs, const Resource<long> &rhs);
template bool operator<=(const long &lhs, const Resource<long> &rhs);
template bool operator>=(const long &lhs, const Resource<long> &rhs);
template long operator/(const Resource<long> &lhs, const long &rhs);
template long operator/(const long &lhs, const Resource<long> &rhs);