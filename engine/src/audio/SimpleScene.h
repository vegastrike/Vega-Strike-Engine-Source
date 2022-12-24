/*
 * SimpleScene.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
// C++ Interface: Audio::SimpleScene
//
#ifndef __AUDIO_SIMPLESCENE_H__INCLUDED__
#define __AUDIO_SIMPLESCENE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Scene.h"
#include "Listener.h"

#include <set>

namespace Audio {

// Forwards
class Source;
class SimpleSource;

/**
 * SimpleScene, basic implementation of the Scene interface
 *
 * @remarks This class implements the scene interface for a basic Scene manager.
 *
 */
class SimpleScene : public Scene, public EnableSharedFromThis<SimpleScene> {
    typedef std::set<vega_types::SharedPtr<Source> > SourceSet;

    Listener listener;

    SourceSet activeSources;

public:
    typedef SourceSet::iterator SourceIterator;

public:

    //
    // Standard Scene interface
    //

    /** Construct a new, empty scene */
    SimpleScene(const std::string &name);

    virtual ~SimpleScene();

    /** @copydoc Scene::add
     * @remarks source MUST be a SimpleSource
     */
    virtual void add(vega_types::SharedPtr<Source> source);

    /** @copydoc Scene::remove
     * @remarks source MUST be a SimpleSource
     */
    virtual void remove(vega_types::SharedPtr<Source> source);

    /** @copydoc Scene::getListener */
    virtual Listener &getListener();


    //
    // SimpleScene-specific interface
    //

    /** Notify the scene of a source that starts or stops playing. */
    virtual void notifySourcePlaying(vega_types::SharedPtr<Source> source, bool playing);

    /** Gets an iterator over active sources */
    SourceIterator getActiveSources();

    /** Gets the ending iterator of active sources */
    SourceIterator getActiveSourcesEnd();

protected:
    void attach(SimpleSource *source);
    void detach(SimpleSource *source);
};

};

#endif//__AUDIO_SIMPLESCENE_H__INCLUDED__
