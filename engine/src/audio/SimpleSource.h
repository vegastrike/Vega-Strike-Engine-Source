/*
 * SimpleSource.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SIMPLESOURCE_H
#define VEGA_STRIKE_ENGINE_AUDIO_SIMPLESOURCE_H

//
// C++ Interface: Audio::SimpleSource
//

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

#include "Source.h"
#include "SimpleScene.h"

namespace Audio {

/**
 * SimpleSource implementation of the Source interface for the basic SceneManager
 *
 * @remarks This implementation merely tracks playing state with a boolean, and notifies
 *      the attached scene of any changes. It also keeps track of the scene to which
 *      it is attached through a raw pointer, to avoid circular references.
 *      @par The scene attached must be a SimpleScene derivative, since the scene itself is
 *      responsible for detaching itself when destroyed.
 *
 */
class SimpleSource : public Source, public SharedFromThis<SimpleSource> {
private:
    bool playing;
    SimpleScene *scene;

public:
    virtual ~SimpleSource();

    /** Construct a simple source */
    SimpleSource(SharedPtr<Sound> sound, bool looping = false);

    /** Notify attachment to a scene */
    void notifySceneAttached(SimpleScene *scene);

    /** Get the scene to which it is attached */
    SimpleScene *getScene() const;

    // The following section contains all the virtual functions that need be implemented
    // by a concrete Sound class. All are protected, so the stream interface is independent
    // of implementations.
protected:

    /** @copydoc Source::startPlayingImpl */
    virtual void startPlayingImpl(Timestamp start);

    /** @copydoc Source::stopPlayingImpl */
    virtual void stopPlayingImpl();

    /** @copydoc Source::isPlayingImpl*/
    virtual bool isPlayingImpl() const;
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SIMPLESOURCE_H
