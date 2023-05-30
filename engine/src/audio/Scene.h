/**
 * Scene.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


//
// C++ Interface: Audio::Scene
//
#ifndef __AUDIO_SCENE_H__INCLUDED__
#define __AUDIO_SCENE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"

namespace Audio {

// Forward declarations
class Source;
class Listener;

/**
 * Scene abstract class
 *
 * @remarks This class represents a scene and the interface to manipulating them.
 *      A scene is a collection of sources and ONE listener. The ability to have
 *      multiple scenes (active or not) is of great use, so the possibility of
 *      a scene manager containing multiple scenes is given.
 *      @par Implementations of the Scene interface is required for any given
 *      SceneManager. In fact, it is expected that each SceneManager will provide
 *      its own Scene implementation. Therefore, a concrete Scene class can be
 *      thought of as part of a SceneManager's implementation.
 *
 */
class Scene {
    std::string name;

protected:
    /** Internal constructor used by derived classes */
    Scene(const std::string &name);

public:
    virtual ~Scene();

    const std::string &getName() const {
        return name;
    }

    /** Attach a source to this scene.
     * @remarks The must be stopped. Adding a playing source is an error.
     */
    virtual void add(SharedPtr<Source> source) = 0;

    /** Detach a source from this scene
     * @remarks The source is implicitly stopped.
     */
    virtual void remove(SharedPtr<Source> source) = 0;

    /** Get the scene's listener */
    virtual Listener &getListener() = 0;
};

};

#endif//__AUDIO_SCENE_H__INCLUDED__
