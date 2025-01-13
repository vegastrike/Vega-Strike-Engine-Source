/*
 * SourceListener.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SOURCELISTENER_H
#define VEGA_STRIKE_ENGINE_AUDIO_SOURCELISTENER_H

//
// C++ Interface: Audio::SourceListener
//

#include "Exceptions.h"
#include "Types.h"
#include "RenderableSource.h"

namespace Audio {

// Forward declarations

class Source;

/**
 * Source event listener abstract class
 *
 * @remarks This is the interface clients must implement to be able to
 *      receive event notifications from audio sources.
 */
class SourceListener {
protected:
    struct {
        int attach: 1;
        int play: 1;
        int update: 1;
    } events;

    SourceListener() {
        // Want everything
        events.attach =
                events.update =
                        events.play = 1;
    }

public:
    virtual ~SourceListener() {
    }

    bool wantAttachEvents() const {
        return events.attach != 0;
    }

    bool wantPlayEvents() const {
        return events.play != 0;
    }

    bool wantUpdateEvents() const {
        return events.update != 0;
    }

    /**
     * Called when the source is ABOUT TO be attached or detached
     * @param source the source to be attached or detached
     * @param detach if true, it's about to be detached. If false,
     *      it's about to be attached
     * @remarks When reattaching, only the attach call is
     *      guaranteed to be issued. The detach may be left
     *      implied, or it may be explicitly performed.
     *      In any case, no event may happen between pre & post
     *      calls.
     */
    virtual void onPreAttach(Source &source, bool detach) = 0;

    /**
     * Called after the source has been attached or detached
     * @param source the source to be attached or detached
     * @param detach if true, it's been detached. If false,
     *      it's been attached
     */
    virtual void onPostAttach(Source &source, bool detach) = 0;

    /**
     * Called when the source is ABOUT TO be updated.
     * @param source the source to be updated
     * @param updateFlags the level of update the source will
     *      receive. See RenderableSource::UpdateFlags
     * @see RenderableSource::UpdateFlags
     */
    virtual void onUpdate(Source &source, int updateFlags) = 0;

    /**
     * Called when the source is ABOUT TO be played (rendered) or stopped
     * @param source the source to be played or stopped
     * @param detach if true, it's about to be played. If false,
     *      it's about to be stopped
     */
    virtual void onPrePlay(Source &source, bool stop) = 0;

    /**
     * Called after the source has been played or stopped
     * @param source the source to be played or stopped
     * @param detach if true, it's been played. If false,
     *      it's been stopped
     */
    virtual void onPostPlay(Source &source, bool stop) = 0;

    /**
     * Called after the source has been stopped due to
     * end-of-stream conditions.
     * It falls on the "play" event category.
     * @param source the source that reached its end
     */
    virtual void onEndOfStream(Source &source) = 0;
};

/**
 * Source update event listener abstract class
 *
 * @remarks This is the interface clients must implement to be able to
 *      receive update notifications from audio sources.
 *          This class only accepts updates (by default, it can be
 *      overriden, but you should not - use SourceListener instead).
 */
class UpdateSourceListener : public SourceListener {
public:
    UpdateSourceListener() {
        // Just updates
        events.attach =
                events.play = 0;
        events.update = 1;
    }

    /**
     * No-op implementation to apease the compiler.
     * @remarks This will never be called since attach events will be
     *      disabled by the constructor.
     */
    virtual void onPreAttach(Source &source, bool detach) {
    };

    /**
     * No-op implementation to apease the compiler.
     * @remarks This will never be called since attach events will be
     *      disabled by the constructor.
     */
    virtual void onPostAttach(Source &source, bool detach) {
    };

    /**
     * No-op implementation to apease the compiler.
     * @remarks This will never be called since play events will be
     *      disabled by the constructor.
     */
    virtual void onPrePlay(Source &source, bool stop) {
    };

    /**
     * No-op implementation to apease the compiler.
     * @remarks This will never be called since play events will be
     *      disabled by the constructor.
     */
    virtual void onPostPlay(Source &source, bool stop) {
    };

    /**
     * No-op implementation to apease the compiler.
     * @remarks This will never be called since play events will be
     *      disabled by the constructor.
     */
    virtual void onEndOfStream(Source &source) {
    };
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SOURCELISTENER_H
