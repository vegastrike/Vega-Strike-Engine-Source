/*
 * OpenALRenderer.h
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
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_OPENAL_RENDERER_H__INCLUDED__
#define __AUDIO_OPENAL_RENDERER_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Types.h"
#include "../../Renderer.h"
#include "../../Format.h"

namespace Audio {

namespace __impl {

namespace OpenAL {
// Forward declaration of internal renderer data
struct RendererData;
};

};

/**
 * OpenAL Renderer implementation
 *
 * @remarks Audio renderer implementation based on OpenAL.
 *
 */
class OpenALRenderer : public Renderer {
protected:
    vega_types::AutoPtr<__impl::OpenAL::RendererData> data;

public:
    /** Initialize the renderer with default or config-driven settings. */
    OpenALRenderer();

    virtual ~OpenALRenderer();

    /** @copydoc Renderer::getSound */
    virtual vega_types::SharedPtr<Sound> getSound(
            const std::string &name,
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
            bool streaming = false);

    /** @copydoc Renderer::owns */
    virtual bool owns(vega_types::SharedPtr<Sound> sound);

    /** @copydoc Renderer::attach(SharedPtr<Source>) */
    virtual void attach(vega_types::SharedPtr<Source> source);

    /** @copydoc Renderer::attach(SharedPtr<Listener>) */
    virtual void attach(vega_types::SharedPtr<Listener> listener);

    /** @copydoc Renderer::detach(SharedPtr<Source>) */
    virtual void detach(vega_types::SharedPtr<Source> source);

    /** @copydoc Renderer::detach(SharedPtr<Listener>) */
    virtual void detach(vega_types::SharedPtr<Listener> listener);

    /** @copydoc Renderer::setMeterDistance */
    virtual void setMeterDistance(Scalar distance);

    /** @copydoc Renderer::setDopplerFactor */
    virtual void setDopplerFactor(Scalar factor);

    /** @copydoc Renderer::setOutputFormat */
    virtual void setOutputFormat(const Format &format);

    /** @copydoc Renderer::beginTransaction */
    virtual void beginTransaction();

    /** @copydoc Renderer::commitTransaction */
    virtual void commitTransaction();
protected:

    /** Makes sure the AL context is valid, creating one if necessary */
    virtual void checkContext();

    /** Sets expected defaults into the context */
    virtual void initContext();

    /** Sets doppler effect globals into the context */
    void setupDopplerEffect();
};

};

#endif//__AUDIO_OPENAL_RENDERER_H__INCLUDED__
