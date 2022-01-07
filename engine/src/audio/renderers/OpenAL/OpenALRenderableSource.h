/**
 * OpenALRenderableSource.h
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
// C++ Interface: Audio::OpenALRenderableSource
//
#ifndef __AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__
#define __AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__

#include "al.h"

#include "../../RenderableSource.h"

#include "../../Exceptions.h"
#include "../../Types.h"

namespace Audio {

/**
 * OpenAL Renderable Source class
 *
 * @remarks This class implements the RenderableSource interface for the
 *      OpenAL renderer.
 *
 */
class OpenALRenderableSource : public RenderableSource {
    ALuint alSource;
    bool alBuffersAttached;

public:
    OpenALRenderableSource(Source *source);

    virtual ~OpenALRenderableSource();

protected:
    /** @see RenderableSource::startPlayingImpl. */
    virtual void startPlayingImpl(Timestamp start);

    /** @see RenderableSource::stopPlayingImpl. */
    virtual void stopPlayingImpl();

    /** @see RenderableSource::isPlayingImpl. */
    virtual bool isPlayingImpl() const;

    /** @see RenderableSource::getPlayingTimeImpl. */
    virtual Timestamp getPlayingTimeImpl() const;

    /** @see RenderableSource::updateImpl. */
    virtual void updateImpl(int flags, const Listener &sceneListener);

    /** @see RenderableSource::seekImpl. */
    virtual void seekImpl(Timestamp time);

    /** Derived classes may use the underlying AL source handle to set additional attributes */
    ALuint getALSource() const
    {
        return alSource;
    }

    /** Attach AL buffers from the source's AL sound, if not attached already.
     * @note It will fail with an assertion if the attached sound isn't an OpenAL sound.
     */
    void attachALBuffers();
};

};

#endif//__AUDIO_OPENALRENDERABLESOURCE_H__INCLUDED__
