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

#ifndef RESOURCE_H
#define RESOURCE_H

/**
 * @brief A resource is any part of the game that can be used up and filled up.
 * The purpose of this class is to simplify code throughout the game by placing it here.
 * Instead of adding to health and then checking it isn't above max health, it's
 * done automatically.
 */

template<typename T>
class Resource {
    T value_;
    T min_value_;
    T max_value_;
    T adjusted_max_value_;
    bool no_max_;
public:
    Resource(const T &value, const T &min_value = 0, const T &max_value = -1);

    Resource<T> operator=(const T &value);
    Resource<T> operator+=(const T &value);
    Resource<T> operator-=(const T &value);

    //Resource<T> operator=(T &value);
    Resource<T> operator+=(T &value);
    Resource<T> operator-=(T &value);

    void Downgrade(const T &value);
    void DowngradeByPercent(const T &value);

    T Percent() const;
    void ResetMaxValue();
    void Set(const T &value);
    void SetMaxValue(const T &value);
    void Upgrade(const T &value);
    void UpgradeByPercent(const T &value);
    void Zero();

    T Value() const;
    T MaxValue() const;
    T MinValue() const;
    T AdjustedValue() const;
};

template<typename T>
bool operator==(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator>(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator<(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator<=(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator>=(const Resource<T> &lhs, const T &rhs);
template<typename T>
bool operator==(const T &lhs, const Resource<T> &rhs);
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
#endif // RESOURCE_H
