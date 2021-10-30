#ifndef RESOURCE_H
#define RESOURCE_H


/**
 * @brief A resource is any part of the game that can be used up and filled up.
 * The purpose of this class is to simplify code throughout the game by placing it here.
 * Instead of adding to health and then checking it isn't above max health, it's
 * done automatically.
 */

template <typename T>
class Resource
{
    T value_;
    T min_value_;
    T max_value_;
    T adjusted_max_value_;

public:
    Resource(const T& value, const T& min_value = 0);

    Resource<T> operator=(const T &value);
    Resource<T> operator+=(const T &value);
    Resource<T> operator-=(const T &value);

    void Downgrade(const T &value);
    void DowngradeByPercent(const T &value);

    T Percent() const;
    void ResetMaxValue();
    void Set(const T& value, const T& min_value = 0);
    void SetMaxValue(const T &value);
    void Upgrade(const T &value);
    void UpgradeByPercent(const T &value);
    void Zero();

    T Value() const;
    T MaxValue() const;
    T MinValue() const;
    T AdjustedValue() const;
};



template <typename T>
bool operator==(const Resource<T>& lhs, const T& rhs);
template <typename T>
bool operator>(const Resource<T>& lhs, const T& rhs);
template <typename T>
bool operator<(const Resource<T>& lhs, const T& rhs);
template <typename T>
bool operator<=(const Resource<T>& lhs, const T& rhs);
template <typename T>
bool operator>=(const Resource<T>& lhs, const T& rhs);
template <typename T>
bool operator==(const T& lhs, const Resource<T>& rhs);
template <typename T>
bool operator>(const T& lhs, const Resource<T>& rhs);
template <typename T>
bool operator<(const T& lhs, const Resource<T>& rhs);
template <typename T>
bool operator<=(const T& lhs, const Resource<T>& rhs);
template <typename T>
bool operator>=(const T& lhs, const Resource<T>& rhs);
template <typename T>
T operator/(const Resource<T>& lhs, const T& rhs);
template <typename T>
T operator/(const T& lhs, const Resource<T>& rhs);
#endif // RESOURCE_H
