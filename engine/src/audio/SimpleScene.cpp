/**
 * SimpleScene.cpp
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
// C++ Implementation: Audio::SimpleScene
//

#include <cassert>
#include "SimpleScene.h"
#include "SimpleSource.h"

#include "SceneManager.h"

namespace Audio {

SimpleScene::SimpleScene(const std::string &name) :
        Scene(name) {
}

SimpleScene::~SimpleScene() {
    SourceSet::iterator it;

    for (it = activeSources.begin(); it != activeSources.end(); ++it) {
        (*it)->stopPlaying();
        SimpleSource *p_simple_source = dynamic_cast<SimpleSource *>(it->get());
        assert(p_simple_source != nullptr);
        detach(p_simple_source);
    }
}

void SimpleScene::add(SharedPtr<Source> source) {
    SimpleSource *p_simple_source = dynamic_cast<SimpleSource *>(source.get());
    assert(p_simple_source != nullptr);
    attach(p_simple_source);
}

void SimpleScene::remove(SharedPtr<Source> source) {
    SimpleSource *p_simple_source = dynamic_cast<SimpleSource *>(source.get());
    assert(p_simple_source != nullptr);
    detach(p_simple_source);
}

Listener &SimpleScene::getListener() {
    return listener;
}

void SimpleScene::notifySourcePlaying(SharedPtr<Source> source, bool playing) {
    if (playing) {
        activeSources.insert(source);
    } else {
        activeSources.erase(source);
    }

    SceneManager::getSingleton()->notifySourcePlaying(source, shared_from_this(), playing);
}

void SimpleScene::attach(SimpleSource *source) {
    source->notifySceneAttached(this);
}

void SimpleScene::detach(SimpleSource *source) {
    source->notifySceneAttached(0);
}

/** Gets an iterator over active sources */
SimpleScene::SourceIterator SimpleScene::getActiveSources() {
    return activeSources.begin();
}

/** Gets the ending iterator of active sources */
SimpleScene::SourceIterator SimpleScene::getActiveSourcesEnd() {
    return activeSources.end();
}

};
