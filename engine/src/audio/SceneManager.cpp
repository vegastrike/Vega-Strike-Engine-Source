/**
 * SceneManager.cpp
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
// C++ Implementation: Audio::SceneManager
//
#include "SceneManager.h"

#include "Renderer.h"
#include "RenderableSource.h"
#include "RenderableListener.h"
#include "SourceTemplate.h"
#include "SimpleSource.h"
#include "SimpleScene.h"
#include "Sound.h"
#include "SourceListener.h"

#include <limits>
#include <assert.h>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "vs_math.h"

template<> Audio::SceneManager *Singleton<Audio::SceneManager>::_singletonInstance = 0;

namespace Audio {

namespace __impl {

struct SceneManagerData {
    struct SourceRef {
        SharedPtr<Source> source;
        SharedPtr<Scene> scene;

        /**
         * If true, an update phase will start playing the source.
         * Set when the activation phase attaches a renderable (or decides to resume one),
         * since it cannot do so immediately or positional glitches would be apparent.
         */
        mutable int needsActivation: 1;

        SourceRef(const SharedPtr<Source> &src, const SharedPtr<Scene> &scn) :
                source(src),
                scene(scn),
                needsActivation(0) {
        }

        bool operator==(const SourceRef &o) const {
            // Ignore scene...
            return (source == o.source);
        }

        bool operator<(const SourceRef &o) const {
            // Ignore scene...
            return (source < o.source);
        }
    };

    // The many required indexes
    typedef std::map<std::string, SharedPtr<Scene> > SceneMap;
    typedef std::set<SourceRef> SourceRefSet;

    SceneMap activeScenes;
    SceneMap inactiveScenes;

    // Being rendered, they need frequent updates
    SourceRefSet activeSources;

    SharedPtr<Renderer> renderer;
    SharedPtr<Listener> rootListener;

    unsigned int maxSources;
    float minGain;
    double maxDistance;

    Timestamp lastPositionUpdateTime;
    Timestamp lastAttributeUpdateTime;
    Timestamp lastListenerUpdateTime;
    Timestamp lastListenerAttUpdateTime;
    Timestamp lastActivationTime;

    Duration positionUpdateFrequency;
    Duration attributeUpdateFrequency;
    Duration listenerUpdateFrequency;
    Duration activationFrequency;

    SceneManagerData() :
            rootListener(new Listener()),

            maxSources(16),
            minGain(1.0 / 16384.0),
            maxDistance(std::numeric_limits<double>::infinity()),

            lastPositionUpdateTime(-std::numeric_limits<Timestamp>::infinity()),
            lastAttributeUpdateTime(-std::numeric_limits<Timestamp>::infinity()),
            lastListenerUpdateTime(-std::numeric_limits<Timestamp>::infinity()),
            lastListenerAttUpdateTime(-std::numeric_limits<Timestamp>::infinity()),
            lastActivationTime(-std::numeric_limits<Timestamp>::infinity()),

            positionUpdateFrequency(1.0 / 20.0),
            attributeUpdateFrequency(1.0 / 5.0),
            listenerUpdateFrequency(1.0 / 30.0),
            activationFrequency(1.0 / 10.0) {
    }
};

};

using namespace __impl;

SceneManager::SceneManager() :
        data(new SceneManagerData) {
}

SceneManager::~SceneManager() {
}

const SharedPtr<Renderer> &SceneManager::internalRenderer() const {
    if (!data->renderer.get()) {
        throw Exception("No renderer");
    }
    return data->renderer;
}

SharedPtr<Source> SceneManager::createSource(SharedPtr<Sound> sound, bool looping) {
    if (!internalRenderer()->owns(sound)) {
        throw Exception("Invalid sound: incompatible renderers used");
    }

    return SharedPtr<Source>(new SimpleSource(sound, looping));
}

SharedPtr<Source> SceneManager::createSource(SharedPtr<SourceTemplate> tpl) {
    return createSource(tpl, tpl->getSoundName());
}

SharedPtr<Source> SceneManager::createSource(SharedPtr<SourceTemplate> tpl, const std::string &name) {
    SharedPtr<Source> source = createSource(
            internalRenderer()->getSound(
                    name,
                    tpl->getSoundType(),
                    tpl->isStreaming()),
            tpl->isLooping());

    source->setCosAngleRange(tpl->getCosAngleRange());
    source->setPerFrequencyRadiusRatios(tpl->getPerFrequencyRadiusRatios());
    source->setReferenceFreqs(tpl->getReferenceFreqs());
    source->setGain(tpl->getGain());
    source->setAttenuated(tpl->isAttenuated());
    source->setRelative(tpl->isRelative());

    return source;
}

void SceneManager::destroySource(SharedPtr<Source> source) {
    // By simply unreferencing, it should get destroyed when all references are released.
    // Which is good for multithreading - never destroy something that is being referenced.

    // We cannot have playing sources without references within the manager
    // Since it's stopped, it will eventually be removed from the active list if there.
    if (source->isPlaying()) {
        source->stopPlaying();
    }

    // Remove all references to it within every scene
    SceneManagerData::SceneMap::iterator it;
    for (it = data->activeScenes.begin(); it != data->activeScenes.end(); ++it) {
        it->second->remove(source);
    }
    for (it = data->inactiveScenes.begin(); it != data->inactiveScenes.end(); ++it) {
        it->second->remove(source);
    }
}

void SceneManager::addScene(SharedPtr<Scene> scene) {
    if (data->activeScenes.count(scene->getName())
            || data->inactiveScenes.count(scene->getName())) {
        throw (DuplicateObjectException(scene->getName()));
    }

    data->inactiveScenes[scene->getName()] = scene;
}

SharedPtr<Scene> SceneManager::createScene(const std::string &name) {
    SharedPtr<Scene> scenePtr(new SimpleScene(name));
    addScene(scenePtr);
    return scenePtr;
}

SharedPtr<Scene> SceneManager::getScene(const std::string &name) const {
    SceneManagerData::SceneMap::const_iterator it;

    it = data->activeScenes.find(name);
    if (it != data->activeScenes.end()) {
        return it->second;
    }

    it = data->inactiveScenes.find(name);
    if (it != data->inactiveScenes.end()) {
        return it->second;
    }

    throw (NotFoundException(name));
}

void SceneManager::destroyScene(const std::string &name) {
    // By simply unreferencing, it should get destroyed when all references are released.
    // Which is good for multithreading - never destroy something that is being referenced.
    // Any active sources will get deactivated in the next update since there aren't any active scenes
    // containing them.
    data->activeScenes.erase(name);
    data->inactiveScenes.erase(name);
}

void SceneManager::setSceneActive(const std::string &name, bool active) {
    // Simply move the pointer from one map to the other.
    // The next update will take care of activating sources as necessary.
    SharedPtr<Scene> scene = getScene(name);
    if (active) {
        data->inactiveScenes.erase(name);
        data->activeScenes[name] = scene;
    } else {
        data->activeScenes.erase(name);
        data->inactiveScenes[name] = scene;
    }
}

bool SceneManager::getSceneActive(const std::string &name) {
    return data->activeScenes.count(name) > 0;
}

void SceneManager::setRenderer(SharedPtr<Renderer> renderer) {
    if (data->renderer.get()) {
        // Detach all active sources
        for (SceneManagerData::SourceRefSet::const_iterator it = data->activeSources.begin();
                it != data->activeSources.end(); ++it) {
            data->renderer->detach(it->source);
        }

        // Detach the root listener
        data->renderer->detach(data->rootListener);
    }

    // Swap renderers
    data->renderer.swap(renderer);

    if (data->renderer.get()) {
        // Attach the root listener
        data->renderer->attach(data->rootListener);

        // Attach all active sources
        for (SceneManagerData::SourceRefSet::const_iterator it = data->activeSources.begin();
                it != data->activeSources.end(); ++it) {
            data->renderer->attach(it->source);
        }
    }
}

SharedPtr<Renderer> SceneManager::getRenderer() const {
    return data->renderer;
}

unsigned int SceneManager::getMaxSources() const {
    return data->maxSources;
}

void SceneManager::setMaxSources(unsigned int n) {
    data->maxSources = n;
}

void SceneManager::playSource(
        SharedPtr<SourceTemplate> tpl,
        const std::string &sceneName,
        LVector3 position,
        Vector3 direction,
        Vector3 velocity,
        Scalar radius) {
    if (tpl->isLooping()) {
        throw (Exception("Cannot fire a looping source and forget!"));
    }

    SharedPtr<Source> src = createSource(tpl);

    src->setPosition(position);
    src->setDirection(direction);
    src->setVelocity(velocity);
    src->setRadius(radius);

    getScene(sceneName)->add(src);

    src->startPlaying();
}

void SceneManager::playSource(
        SharedPtr<SourceTemplate> tpl,
        const std::string &soundName,
        const std::string &sceneName,
        LVector3 position,
        Vector3 direction,
        Vector3 velocity,
        Scalar radius) {
    if (tpl->isLooping()) {
        throw (Exception("Cannot fire a looping source and forget!"));
    }

    SharedPtr<Source> src = createSource(tpl, soundName);

    src->setPosition(position);
    src->setDirection(direction);
    src->setVelocity(velocity);
    src->setRadius(radius);

    getScene(sceneName)->add(src);

    src->startPlaying();
}

float SceneManager::getMinGain() const {
    return data->minGain;
}

void SceneManager::setMinGain(float gain) {
    assert(gain >= 0.f);
    data->minGain = gain;
}

double SceneManager::getMaxDistance() const {
    return data->maxDistance;
}

void SceneManager::setMaxDistance(double distance) {
    assert(distance >= 0.f);
    data->maxDistance = distance;
}

SharedPtr<SceneManager::SceneIterator> SceneManager::getSceneIterator() const {
    return SharedPtr<SceneIterator>(
            new ChainingIterator<VirtualValuesIterator<SceneManagerData::SceneMap::iterator> >(
                    VirtualValuesIterator<SceneManagerData::SceneMap::iterator>(
                            data->activeScenes.begin(),
                            data->activeScenes.end()),
                    VirtualValuesIterator<SceneManagerData::SceneMap::iterator>(
                            data->inactiveScenes.begin(),
                            data->inactiveScenes.end())
            )
    );
}

SharedPtr<SceneManager::SceneIterator> SceneManager::getActiveSceneIterator() const {
    return SharedPtr<SceneIterator>(
            new VirtualValuesIterator<SceneManagerData::SceneMap::iterator>(
                    data->activeScenes.begin(),
                    data->activeScenes.end()));
}

void SceneManager::commit() {
    Timestamp realTime = getRealTime();
    bool needActivation = ((realTime - getActivationFrequency()) >= data->lastActivationTime);
    bool needPosUpdates = ((realTime - getPositionUpdateFrequency()) >= data->lastPositionUpdateTime);
    bool needAttUpdates = ((realTime - getAttributeUpdateFrequency()) >= data->lastAttributeUpdateTime);
    bool needListenerUpdate = ((realTime - getListenerUpdateFrequency()) >= data->lastListenerUpdateTime);
    bool needListenerAttUpdate = ((realTime - getAttributeUpdateFrequency()) >= data->lastListenerAttUpdateTime);

    // If we have an activation phase, in order for it to be effective
    // we'll need an update phase as well.
    needPosUpdates = needPosUpdates || needActivation;

    internalRenderer()->beginTransaction();

    if (needActivation) {
        activationPhaseImpl();

        data->lastActivationTime = realTime;
    }

    if (needListenerUpdate) {
        updateListenerImpl(needListenerAttUpdate);

        data->lastListenerUpdateTime = realTime;
        if (needListenerAttUpdate) {
            data->lastListenerAttUpdateTime = realTime;
        }
    }

    if (needPosUpdates || needAttUpdates) {
        updateSourcesImpl(needAttUpdates);

        data->lastPositionUpdateTime = realTime;
        if (needAttUpdates) {
            data->lastAttributeUpdateTime = realTime;
        }
    }

    internalRenderer()->commitTransaction();
}

struct SourcePriorityRef {
    SimpleScene::SourceIterator iter;
    SimpleScene *scene;
    Scalar gain;

    SourcePriorityRef() {
    }

    SourcePriorityRef(SimpleScene::SourceIterator itr, const Listener &listener, SimpleScene *scn) :
            iter(itr),
            scene(scn),
            gain(estimateGain(**iter, listener)) {
    }

    bool operator<(const SourcePriorityRef &o) const {
        return gain > o.gain;
    }
};

void SceneManager::activationPhaseImpl() {
    // Just clear the active source set and recreate it from scratch.
    // Use a "source ref heap" to find the most relevant sources (using the approximated
    // intensity as priority). Since the heap will copy things all over, use cheap
    // "SourceIterator"s as entries. These are SimpleScene-specific, so any subclass of
    // SceneManager will probably want to override the activation phase.

    const SharedPtr<Renderer> &renderer = internalRenderer();

    LScalar maxDistanceSq = data->maxDistance * data->maxDistance;

    std::vector<SourcePriorityRef> selection;
    bool heapified = false;
    selection.reserve(data->maxSources + 1);

    for (SceneManagerData::SceneMap::iterator it = data->activeScenes.begin();
            it != data->activeScenes.end();
            ++it) {
        SimpleScene *scene = dynamic_cast<SimpleScene *>(it->second.get());
        Listener &listener = scene->getListener();

        for (SimpleScene::SourceIterator sit = scene->getActiveSources(),
                send = scene->getActiveSourcesEnd();
                sit != send;
                ++sit) {
            if ((*sit)->getSourceListener().get()) {
                // Must invoke the listener to get updated positions
                (*sit)->getSourceListener()->onUpdate(**sit, RenderableSource::UPDATE_LOCATION);
            }

            if (listener.getPosition().distanceSquared((*sit)->getPosition()) < maxDistanceSq) {
                SourcePriorityRef ref(sit, listener, scene);
                if (ref.gain > data->minGain) {
                    selection.push_back(ref);
                    if (selection.size() > data->maxSources) {
                        if (!heapified) {
                            make_heap(selection.begin(), selection.end());
                            heapified = true;
                        } else {
                            push_heap(selection.begin(), selection.end());
                        }
                        while (selection.size() > data->maxSources) {
                            pop_heap(selection.begin(), selection.end());
                            selection.resize(selection.size() - 1);
                        }
                    }
                }
            }
        }
    }

    SceneManagerData::SourceRefSet newSources;
    for (std::vector<SourcePriorityRef>::const_iterator it = selection.begin(); it != selection.end(); ++it) {
        newSources.insert(
                SceneManagerData::SourceRef(
                        *(it->iter),
                        dynamic_cast<SimpleScene *>(it->scene)->shared_from_this()
                ));
    }

    // Detach deactivated sources
    for (SceneManagerData::SourceRefSet::iterator sit = data->activeSources.begin(); sit != data->activeSources.end();
            ++sit) {
        if (newSources.find(*sit) == newSources.end()) {
            renderer->detach(sit->source);
        }
    }

    // Attach newly activated sources, detach and remove finished ones
    for (SceneManagerData::SourceRefSet::iterator nit = newSources.begin(); nit != newSources.end();) {
        bool erase = false;
        if (data->activeSources.find(*nit) == data->activeSources.end()) {
            // Newly activated source
            renderer->attach(nit->source);
            nit->needsActivation = true;
        } else {
            // Pre-existing source - check if it's finished
            if (!nit->source->getRenderable()->isPlaying()) {
                // Give the renderable an opportunity to restart itself
                // (by calling update without any update flag set)
                nit->source->getRenderable()->update(0, nit->scene->getListener());

                if (!nit->source->getRenderable()->isPlaying()) {
                    // Finished - detach stop and remove
                    renderer->detach(nit->source);
                    nit->source->stopPlaying();
                    erase = true;

                    // Check if it has a listener, notify in that case
                    SharedPtr<SourceListener> listener = nit->source->getSourceListener();
                    if (listener.get() != NULL && listener->wantPlayEvents()) {
                        listener->onEndOfStream(*nit->source);
                    }
                }
            }
        }
        if (erase) {
            newSources.erase(nit++);
        } else {
            ++nit;
        }
    }

    // Swap sets
    data->activeSources.swap(newSources);
}

void SceneManager::updateSourcesImpl(bool withAttributes) {
    // Two-pass stuff.

    // First, update attributes (mostly location)
    RenderableSource::UpdateFlags updateFlags =
            withAttributes ?
                    RenderableSource::UPDATE_ALL :
                    RenderableSource::UPDATE_LOCATION;
    for (SceneManagerData::SourceRefSet::const_iterator it = data->activeSources.begin();
            it != data->activeSources.end(); ++it) {
        // Update the renderable (attributes)
        it->source->updateRenderable(
                updateFlags | (it->needsActivation ? RenderableSource::UPDATE_ALL : 0),
                it->scene->getListener());

        // Then, start playing if not playing and should be playing
        if (it->needsActivation) {
            it->source->getRenderable()->startPlaying(
                    it->source->getWouldbePlayingTime());
            it->needsActivation = false;
        }
    }
}

void SceneManager::updateListenerImpl(bool withAttributes) {
    // Update root listener
    RenderableListener::UpdateFlags updateFlags =
            withAttributes ?
                    RenderableListener::UPDATE_ALL :
                    RenderableListener::UPDATE_LOCATION;
    if (data->rootListener.get()) {
        data->rootListener->update(updateFlags);
    }

    // And all scene listeners
    for (SceneManagerData::SceneMap::const_iterator i = data->activeScenes.begin(); i != data->activeScenes.end();
            ++i) {
        i->second->getListener().update(updateFlags);
    }
}

Duration SceneManager::getPositionUpdateFrequency() const {
    return data->positionUpdateFrequency;
}

Duration SceneManager::getListenerUpdateFrequency() const {
    return data->listenerUpdateFrequency;
}

Duration SceneManager::getAttributeUpdateFrequency() const {
    return data->attributeUpdateFrequency;
}

Duration SceneManager::getActivationFrequency() const {
    return data->activationFrequency;
}

void SceneManager::setPositionUpdateFrequency(Duration interval) const {
    data->positionUpdateFrequency = interval;
}

void SceneManager::setListenerUpdateFrequency(Duration interval) const {
    data->listenerUpdateFrequency = interval;
}

void SceneManager::setAttributeUpdateFrequency(Duration interval) const {
    data->attributeUpdateFrequency = interval;
}

void SceneManager::setActivationFrequency(Duration interval) const {
    data->activationFrequency = interval;
}

SharedPtr<Listener> SceneManager::getRootListener() const {
    return data->rootListener;
}

void SceneManager::notifySourcePlaying(SharedPtr<Source> source, SharedPtr<Scene> scene, bool playing) {
    // If the source is within maxDistance from its scene's listener,
    // schedule an immediate activation phase
    double maxDistanceSq = getMaxDistance();
    maxDistanceSq *= maxDistanceSq;

    if (scene->getListener().getPosition().distanceSquared(source->getPosition()) <= maxDistanceSq) {
        data->lastActivationTime = -std::numeric_limits<Timestamp>::infinity();
    }
}

};

