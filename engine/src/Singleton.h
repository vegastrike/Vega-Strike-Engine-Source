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
#ifndef VEGA_STRIKE_ENGINE_SINGLETON_H
#define VEGA_STRIKE_ENGINE_SINGLETON_H

//
//C++ Template: Singleton<T>
//

namespace InitializationFunctors {
template<typename T>
class DefaultConstructor {
public:
    T *operator()() const {
        return new T();
    }
};
};

/**
 * Singleton template
 *
 * @remarks
 *      The default initialization functor can handle classes with a default constructor.
 *      If you require special initialization specify a different functor.
 *      The class is NOT threadsafe.
 *
 * @remarks
 *      You must add an extern Singleton<T,INIT>::_singletonInstance in your class implementation
 *      or linking against derived classes will fail.
 */
template<typename T, typename INIT = InitializationFunctors::DefaultConstructor<T> >
class Singleton {
protected:
    static T *_singletonInstance;

    static void initializeSingleton() {
        if (_singletonInstance != 0) {
            delete _singletonInstance;
        }
        //GCC 3.3 errors out if you do '(INIT())()'
        INIT singletonConstructor;
        _singletonInstance = singletonConstructor();
    }

    static void deinitializeSingleton() {
        _singletonInstance = 0;
    }

    ~Singleton() {
        if (_singletonInstance == this) {
            deinitializeSingleton();
        }
    }

public:
    static T *getSingleton() {
        if (_singletonInstance == 0) {
            initializeSingleton();
        }
        return _singletonInstance;
    }
};

#endif //VEGA_STRIKE_ENGINE_SINGLETON_H
