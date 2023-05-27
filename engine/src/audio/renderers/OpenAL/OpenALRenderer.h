/**
 * OpenALRenderer.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERER_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERER_H

//
// C++ Interface: Audio::SceneManager
//

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
    AutoPtr<__impl::OpenAL::RendererData> data;

public:
    /** Initialize the renderer with default or config-driven settings. */
    OpenALRenderer();

    virtual ~OpenALRenderer();

    /** @copydoc Renderer::getSound */
    virtual SharedPtr<Sound> getSound(
            const std::string &name,
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
            bool streaming = false);

    /** @copydoc Renderer::owns */
    virtual bool owns(SharedPtr<Sound> sound);

    /** @copydoc Renderer::attach(SharedPtr<Source>) */
    virtual void attach(SharedPtr<Source> source);

    /** @copydoc Renderer::attach(SharedPtr<Listener>) */
    virtual void attach(SharedPtr<Listener> listener);

    /** @copydoc Renderer::detach(SharedPtr<Source>) */
    virtual void detach(SharedPtr<Source> source);

    /** @copydoc Renderer::detach(SharedPtr<Listener>) */
    virtual void detach(SharedPtr<Listener> listener);

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

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERER_H
