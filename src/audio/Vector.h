//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_VECTOR_H__INCLUDED__
#define __AUDIO_VECTOR_H__INCLUDED__

#include <math.h>

namespace Audio {

    template<typename T> class TVector3 {
    public:
        T x,y,z;
        
        TVector3() {}
        TVector3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
        explicit TVector3(T s) : x(s), y(s), z(s) {}
        
        template<typename Y>
        TVector3(const TVector3<Y> &other) : x(T(other.x)), y(T(other.y)), z(T(other.z)) {}
        
        template<typename Y>
        TVector3<T>& operator=(const TVector3<Y> &other)
        {
            x = T(other.x);
            y = T(other.y);
            z = T(other.z);
            return *this;
        }
        
        TVector3<T>& operator+=(const TVector3<T> &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }
        
        TVector3<T>& operator-=(const TVector3<T> &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }
        
        TVector3<T>& operator*=(const TVector3<T> &other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }
        
        TVector3<T>& operator*=(T t)
        {
            x *= t;
            y *= t;
            z *= t;
            return *this;
        }
        
        TVector3<T>& operator/=(const TVector3<T> &other)
        {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }
        
        TVector3<T>& operator/=(T t)
        {
            x /= t;
            y /= t;
            z /= t;
            return *this;
        }
        
        TVector3<T> operator+(const TVector3<T> &other) const
        { 
            return TVector3<T>(x+other.x, y+other.y, z+other.z); 
        }
        
        TVector3<T> operator-(const TVector3<T> &other) const
        { 
            return TVector3<T>(x-other.x, y-other.y, z-other.z); 
        }
        
        TVector3<T> operator*(const TVector3<T> &other) const
        { 
            return TVector3<T>(x*other.x, y*other.y, z*other.z); 
        }
        
        TVector3<T> operator/(const TVector3<T> &other) const
        { 
            return TVector3<T>(x/other.x, y/other.y, z/other.z); 
        }
        
        TVector3<T> operator*(T t) const
        { 
            return TVector3<T>(x*t, y*t, z*t); 
        }
        
        TVector3<T> operator/(T t) const
        { 
            return TVector3<T>(x/t, y/t, z/t); 
        }
        
        T dot(TVector3<T> other) const
        {
            return x*other.x + y*other.y + z*other.z;
        }
        
        T normSquared() const
        {
            return dot(*this);
        }
        
        T norm() const
        {
            return sqrt(normSquared());
        }
        
        T lengthSquared() const
        {
            return normSquared();
        }
        
        T length() const
        {
            return norm();
        }
        
        T distanceSquared(const TVector3<T> &other) const
        {
            return normSquared(other - *this);
        }
        
        T distance(const TVector3<T> &other) const
        {
            return sqrt(distanceSquared(other));
        }
        
        TVector3<T> cross(const TVector3<T> &v) const
        {
            return TVector3<T>(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            );
        }
        
        void normalize()
        {
            *this /= norm();
        }
    };

};

#endif//__AUDIO_VECTOR_H__INCLUDED__
