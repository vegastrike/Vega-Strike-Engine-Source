/*
 * test.cpp
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


//
// C++ Implementation: Audio::Test
//

#include "Types.h"
#include "Exceptions.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Sound.h"
#include "Listener.h"
#include "Source.h"
#include "SourceListener.h"
#include "SourceTemplate.h"
#include "renderers/OpenAL/OpenALRenderer.h"

#include <iostream>
#include <string>
//#include <limits>
#include <cassert>
#include "vega_cast_utils.h"

#include "utils.h"
#include "vs_math.h"

using namespace std;

namespace Audio {
namespace Test {

void initSceneManager() {
    cerr << "Creating scene manager..." << flush;
    new SceneManager();

    if (SceneManager::getSingleton() == 0) {
        throw Exception("Singleton null after SceneManager instantiation");
    }

    cerr << "OK" << endl;
}

void clearScene() {
    cerr << "  Clearing Scene" << endl;

    SceneManager *sm = SceneManager::getSingleton();
    for (SharedPtr<SceneManager::SceneIterator> it = sm->getSceneIterator(); !it->eos();) {
        string sceneName = it->get()->getName();
        cerr << "    destroying \"" << sceneName.c_str() << "\"" << flush;
        it->next();
        cerr << "..." << flush;
        sm->destroyScene(sceneName);
        cerr << "done" << endl;
    }

    if (!sm->getSceneIterator()->eos()) {
        throw Exception("WTF: SceneManager has scenes after clearing them all?");
    }

}

void smTick() {
    cerr << "t" << flush;
    SceneManager::getSingleton()->commit();
    sleep(100);
}

void smQuickTick() {
    SceneManager::getSingleton()->commit();
    sleep(10);
}

void testRendererless() {
    // Test clearing and creating scenes repeatedly
    //   (tests management structures)
    clearScene();

    SceneManager *sm = SceneManager::getSingleton();

    cerr << "  Creating empty inactive scene" << endl;
    sm->createScene("testScene");
    sm->getScene("testScene");

    clearScene();

    cerr << "  Creating empty active scene" << endl;
    sm->createScene("testScene");
    sm->getScene("testScene");
    sm->setSceneActive("testScene", true);

    clearScene();
}

void testSimpleScene() {
    cerr << " Simple scene (low level services only)" << endl;

    clearScene();

    int i;

    SceneManager *sm = SceneManager::getSingleton();

    // Create (and verify that) a test scene
    SharedPtr<Scene> scene = sm->createScene("testScene");
    sm->getScene("testScene");
    sm->setSceneActive("testScene", true);

    // Simple test scene:
    //   1. A looping, afterburner source at the back.
    //   2. A fire-and-forget beam sound to the left (4s)
    //   3. A fire-and-forget beam sound to the right (8s)
    //   4. Finish (16s)
    cerr << "  Creating resources" << endl;
    cerr << "    creating sounds..." << flush;
    SharedPtr<Sound> absound = sm->getRenderer()->getSound("afterburner.wav", VSFileSystem::SoundFile);
    SharedPtr<Sound> beamsound = sm->getRenderer()->getSound("beam.wav", VSFileSystem::SoundFile);
    cerr << " ok" << endl;
    cerr << "    loading sounds..." << flush;
    absound->load();
    beamsound->load();
    cerr << " ok" << endl;

    cerr << "    setting up listener..." << flush;
    scene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    scene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    creating sources..." << flush;
    SharedPtr<Source> absource = sm->createSource(absound, true);
    SharedPtr<Source> beamsource1 = sm->createSource(beamsound);
    SharedPtr<Source> beamsource2 = sm->createSource(beamsound);
    absource->setPosition(Vector3(0, 0, -1));
    beamsource1->setPosition(Vector3(-1, 0, 0));
    beamsource2->setPosition(Vector3(+1, 0, 0));
    beamsource1->setGain(0.5f);
    beamsource2->setGain(0.5f);
    scene->add(absource);
    scene->add(beamsource1);
    scene->add(beamsource2);
    cerr << " ok" << endl;

    cerr << "  playing out scene..." << flush;

    absource->startPlaying();
    for (i = 0; i < 40; ++i) { // 4s = 40 ticks
        smTick();
    }

    beamsource1->startPlaying();
    for (i = 0; i < 40; ++i) { // 4s = 40 ticks
        smTick();
    }

    beamsource2->startPlaying();
    for (i = 0; i < 80; ++i) { // 8s = 80 ticks
        smTick();
    }

    absource->stopPlaying();
    for (i = 0; i < 10; ++i) { // 1s = 10 ticks
        smTick();
    }

    cerr << " ok" << endl;
}

void testSimpleSceneWDynTemplates() {
    cerr << " Simple scene (dynamic templates)" << endl;

    SharedPtr<SourceTemplate> abtpl(
            new SourceTemplate("afterburner.wav", VSFileSystem::SoundFile, true));
    SharedPtr<SourceTemplate> beamtpl(
            new SourceTemplate("beam.wav", VSFileSystem::SoundFile, false));
    beamtpl->setGain(0.5f);
}

void testSimpleSceneWFileTemplates() {
    cerr << " Simple scene (persistent templates)" << endl;
}

class EngParticleListener : public UpdateSourceListener {
    const vector<LVector3> &engPaths; // x=phase, y=speed, z=wobble

public:
    double time;

public:
    EngParticleListener(const vector<LVector3> &engpaths, double worldsize) :
            engPaths(engpaths) {
    }

    LVector3 computePosition(long which, double phaseOffset = 0.0) const {
        assert(which >= 0 && which < (long) engPaths.size());

        double phase = engPaths[which].x + phaseOffset;
        double speed = engPaths[which].y;
        double radii = engPaths[which].z;

        LVector3 vBase = LVector3(
                cos(phase + time * speed) * radii,
                -sin(phase + time * speed) * radii,
                0
        );

        return vBase;
    }

    LVector3 computeVelocity(long which, double phaseOffset = 0.0) const {
        assert(which >= 0 && which < (long) engPaths.size());

        double phase = engPaths[which].x + phaseOffset;
        double speed = engPaths[which].y;
        double radii = engPaths[which].z;

        LVector3 dvBase = LVector3(
                -sin(phase + time * speed) * radii * speed,
                -cos(phase + time * speed) * radii * speed,
                0
        );

        return dvBase;
    }

    /**
    * Called when the source is ABOUT TO be updated.
    * @param source the source to be updated
    * @param updateFlags the level of update the source will
    *      receive. See RenderableSource::UpdateFlags
    * @see SourceListener::onUpdate
    */
    virtual void onUpdate(Source &source, int updateFlags) {
        if (updateFlags & RenderableSource::UPDATE_LOCATION) {
            long which = source.getUserDataLong();

            source.setPosition(computePosition(which));
            source.setDirection(computeVelocity(which).normalized());
            source.setVelocity(computeVelocity(which));
        }
    }
};

void testComplexScene(bool doppler) {
    cerr << " Complex scene" << endl;

    clearScene();

    int i;
    double t;

    SceneManager *sm = SceneManager::getSingleton();

    // Create (and verify that) the test scene set
    SharedPtr<Scene> cpscene = sm->createScene("testSceneCP");
    sm->getScene("testSceneCP");
    sm->setSceneActive("testSceneCP", true);

    SharedPtr<Scene> spcscene = sm->createScene("testSceneSPC");
    sm->getScene("testSceneSPC");
    sm->setSceneActive("testSceneSPC", true);

    // Complex test scene:
    //   1. A looping, afterburner source at the back,
    //      in the cockpit.
    //   2. A set of random fire-and-forget cockpit sounds
    //      (beeps of various sorts)
    //   3. Numerous random fire-and-forget, low-volume
    //      explosions (in space)
    //   4. A fleet of listener-driven engine sounds
    //      with parametric flight paths.
    //   5. The cockpit listener is fixed
    //   6. The space listener moves around the scene,
    //      which is bigger than the maximum activation
    //      distance (forcing sources on/off)

    const size_t nengs = 100;
    const double worldsize = 500.0;

    // Set scene manager settings for test
    sm->setMaxSources(10);
    sm->setMaxDistance(worldsize / 4);
    sm->getRenderer()->setDopplerFactor(doppler ? 0.25 : 0.0);
    sm->getRenderer()->setMeterDistance(1.0);

    cerr << "    setting up CP listener..." << flush;
    cpscene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    cpscene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    setting up SPC listener..." << flush;
    spcscene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    spcscene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    creating looping sources..." << flush;

    vector<SharedPtr<Source> > engsources;
    vector<LVector3> engpaths; // x=phase, y=speed, z=wobble

    // Fill in engpaths
    SharedPtr<SourceListener> englistener(
            new EngParticleListener(engpaths, worldsize));

    {
        const double phase_step = 2.0 * M_PI / nengs;
        for (size_t i = 0; i < nengs; ++i) {
            double phase = i * phase_step;
            double speed = 2.0 * phase_step * (double(rand() + RAND_MAX / 2) / RAND_MAX);
            double radii = worldsize * (1.0 + 0.25 * double(rand() - RAND_MAX / 2) / RAND_MAX);

            engpaths.push_back(LVector3(phase, speed, radii));
        }
    }

    cerr << " ok" << endl;

    cerr << "  playing out scene..." << flush;

    {
        for (vector<SharedPtr<Source> >::iterator i = engsources.begin(); i != engsources.end(); ++i) {
            (*i)->startPlaying();
        }
    }

    for (t = getRealTime(); (getRealTime() - t) < 20.0;) { // 20s = 200 ticks
        const double phase_step = 2.0 * M_PI / 20;
        double phase = (getRealTime() - t) * phase_step;
        double speed = phase_step;

        LVector3 pos = LVector3(
                cos(phase) * worldsize,
                -sin(phase) * worldsize,
                0
        );

        LVector3 dpos = LVector3(
                -sin(phase) * worldsize * speed,
                -cos(phase) * worldsize * speed,
                0
        );

        spcscene->getListener().setOrientation(dpos, Vector3(0, 1, 0));
        spcscene->getListener().setPosition(pos);

        EngParticleListener *p_listener = vega_dynamic_cast_ptr<EngParticleListener>(englistener.get());
        p_listener->time = (getRealTime() - t) / 20.0;

        smQuickTick();
    }

    {
        for (vector<SharedPtr<Source> >::iterator i = engsources.begin(); i != engsources.end(); ++i) {
            (*i)->stopPlaying();
        }
    }

    for (i = 0; i < 10; ++i) { // 1s = 10 ticks
        smTick();
    }

    cerr << " ok" << endl;
}

void testStreaming() {
    cerr << " Simple streaming (stream a music track)" << endl;

    clearScene();

    int i;
    double timeDelta = 0.0;

    SceneManager *sm = SceneManager::getSingleton();

    // Create (and verify that) a test scene
    SharedPtr<Scene> scene = sm->createScene("testScene");
    sm->getScene("testScene");
    sm->setSceneActive("testScene", true);

    // Simple streaming test scene:
    //   1. A streaming music track playing as usual
    cerr << "  Creating resources" << endl;
    cerr << "    creating sounds..." << flush;
    SharedPtr<Sound> music = sm->getRenderer()->getSound(
            "AyMambo.ogg", VSFileSystem::MusicFile, true);
    cerr << " ok" << endl;
    cerr << "    loading sounds..." << flush;
    music->load();
    cerr << " ok" << endl;

    cerr << "    setting up listener..." << flush;
    scene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    scene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    creating sources..." << flush;
    SharedPtr<Source> musicsource = sm->createSource(music);
    musicsource->setPosition(Vector3(0, 0, 0));
    musicsource->setGain(1.0f);
    musicsource->setAttenuated(false);
    musicsource->setRelative(true);
    scene->add(musicsource);
    cerr << " ok" << endl;

    cerr << "  playing out scene..." << flush;

    double startPlayingTime = getRealTime();
    musicsource->startPlaying();
    for (i = 0; i < 200; ++i) { // 20s = 200 ticks
        smTick();

        // Track drift
        double realTime = getRealTime();
        double curPlayingTime = musicsource->getPlayingTime();
        double shouldPlayingTime = realTime - startPlayingTime;
        double delta = fabs(curPlayingTime - shouldPlayingTime);
        if (delta > timeDelta) {
            timeDelta = delta;
        }

        // Resync supposed start time to account for systematic
        // drift (which isn't that bad, and quite normal in fact)
        startPlayingTime = realTime - curPlayingTime;

        cerr << "Time delta: " << int(delta * 1000) << "ms  (max " << int(timeDelta * 1000) << "ms)\t\r" << flush;
    }

    musicsource->stopPlaying();
    for (i = 0; i < 10; ++i) { // 1s = 10 ticks
        smTick();
    }

    if (timeDelta > (1.0 / 30.0)) {
        cerr << "\nWARNING: Poor time tracking resolution\n";
    }
    cerr << "  Time drift at " << int(timeDelta * 1000) << "ms" << endl;

    cerr << " ok" << endl;
}

void testMultiStreaming() {
    cerr << " Multiple streaming (stream two music tracks)" << endl;

    clearScene();

    int i;
    double timeDelta = 0.0;

    SceneManager *sm = SceneManager::getSingleton();

    // Create (and verify that) a test scene
    SharedPtr<Scene> scene = sm->createScene("testScene");
    sm->getScene("testScene");
    sm->setSceneActive("testScene", true);

    // Multi streaming test scene:
    //   1. Two streaming music tracks playing simultaneously
    cerr << "  Creating resources" << endl;
    cerr << "    creating sounds..." << flush;
    SharedPtr<Sound> music = sm->getRenderer()->getSound(
            "AyMambo.ogg", VSFileSystem::MusicFile, true);
    SharedPtr<Sound> music2 = sm->getRenderer()->getSound(
            "Bliss.ogg", VSFileSystem::MusicFile, true);
    cerr << " ok" << endl;
    cerr << "    loading sounds..." << flush;
    music->load();
    music2->load();
    cerr << " ok" << endl;

    cerr << "    setting up listener..." << flush;
    scene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    scene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    creating sources..." << flush;

    SharedPtr<Source> musicsource = sm->createSource(music);
    musicsource->setPosition(Vector3(0, 0, 0));
    musicsource->setGain(1.0f);
    musicsource->setAttenuated(false);
    musicsource->setRelative(true);

    SharedPtr<Source> musicsource2 = sm->createSource(music2);
    musicsource2->setPosition(Vector3(0, 0, 0));
    musicsource2->setGain(1.0f);
    musicsource2->setAttenuated(false);
    musicsource2->setRelative(true);

    scene->add(musicsource);
    scene->add(musicsource2);

    cerr << " ok" << endl;

    cerr << "  playing out scene..." << flush;

    double startPlayingTime = getRealTime();
    musicsource->startPlaying();
    musicsource2->startPlaying();
    for (i = 0; i < 200; ++i) { // 20s = 200 ticks
        smTick();

        // Track drift
        double realTime = getRealTime();
        double curPlayingTime = musicsource->getPlayingTime();
        double shouldPlayingTime = realTime - startPlayingTime;
        double delta = fabs(curPlayingTime - shouldPlayingTime);
        if (delta > timeDelta) {
            timeDelta = delta;
        }

        // Resync supposed start time to account for systematic
        // drift (which isn't that bad, and quite normal in fact)
        startPlayingTime = realTime - curPlayingTime;

        cerr << "Time delta: " << int(delta * 1000) << "ms  (max " << int(timeDelta * 1000) << "ms)\t\r" << flush;
    }

    musicsource->stopPlaying();
    for (i = 0; i < 50; ++i) { // 5s = 50 ticks
        smTick();
    }

    musicsource2->stopPlaying();
    for (i = 0; i < 10; ++i) { // 1s = 10 ticks
        smTick();
    }

    if (timeDelta > (1.0 / 30.0)) {
        cerr << "\nWARNING: Poor time tracking resolution\n";
    }
    cerr << "  Time drift at " << int(timeDelta * 1000) << "ms" << endl;

    cerr << " ok" << endl;
}

void testMultiStreaming2() {
    cerr << " Multiple streaming (stream one music track - twice - from different positions)" << endl;

    clearScene();

    int i;
    double timeDelta = 0.0;

    SceneManager *sm = SceneManager::getSingleton();

    // Create (and verify that) a test scene
    SharedPtr<Scene> scene = sm->createScene("testScene");
    sm->getScene("testScene");
    sm->setSceneActive("testScene", true);

    // Multi streaming test scene:
    //   1. Two streaming music tracks playing simultaneously
    cerr << "  Creating resources" << endl;
    cerr << "    creating sounds..." << flush;
    SharedPtr<Sound> music = sm->getRenderer()->getSound(
            "SonOfSandFleaRemix.ogg", VSFileSystem::MusicFile, true);
    SharedPtr<Sound> music2 = sm->getRenderer()->getSound(
            "SonOfSandFleaRemix.ogg", VSFileSystem::MusicFile, true);
    cerr << " ok" << endl;
    cerr << "    loading sounds..." << flush;
    music->load();
    music2->load();
    cerr << " ok" << endl;

    cerr << "    setting up listener..." << flush;
    scene->getListener().setOrientation(Vector3(0, 0, 1), Vector3(0, 1, 0));
    scene->getListener().setPosition(LVector3(0, 0, 0));
    cerr << " ok" << endl;

    cerr << "    creating sources..." << flush;

    SharedPtr<Source> musicsource = sm->createSource(music);
    musicsource->setPosition(Vector3(0, 0, 0));
    musicsource->setGain(1.0f);
    musicsource->setAttenuated(false);
    musicsource->setRelative(true);

    SharedPtr<Source> musicsource2 = sm->createSource(music2);
    musicsource2->setPosition(Vector3(0, 0, 0));
    musicsource2->setGain(1.0f);
    musicsource2->setAttenuated(false);
    musicsource2->setRelative(true);

    scene->add(musicsource);
    scene->add(musicsource2);

    cerr << " ok" << endl;

    cerr << "  playing out scene..." << flush;

    double startPlayingTime = getRealTime();
    musicsource->startPlaying();
    musicsource2->startPlaying(60.0);
    for (i = 0; i < 200; ++i) { // 20s = 200 ticks
        smTick();

        // Track drift
        double realTime = getRealTime();
        double curPlayingTime = musicsource->getPlayingTime();
        double shouldPlayingTime = realTime - startPlayingTime;
        double delta = fabs(curPlayingTime - shouldPlayingTime);
        if (delta > timeDelta) {
            timeDelta = delta;
        }

        // Resync supposed start time to account for systematic
        // drift (which isn't that bad, and quite normal in fact)
        startPlayingTime = realTime - curPlayingTime;

        cerr << "Time delta: " << int(delta * 1000) << "ms  (max " << int(timeDelta * 1000) << "ms)\t\r" << flush;
    }

    musicsource->stopPlaying();
    for (i = 0; i < 50; ++i) { // 5s = 50 ticks
        smTick();
    }

    musicsource2->stopPlaying();
    for (i = 0; i < 10; ++i) { // 1s = 10 ticks
        smTick();
    }

    if (timeDelta > (1.0 / 30.0)) {
        cerr << "\nWARNING: Poor time tracking resolution\n";
    }
    cerr << "  Time drift at " << int(timeDelta * 1000) << "ms" << endl;

    cerr << " ok" << endl;
}

void initALRenderer() {
    cerr << "  Initializing renderer..." << endl;
    SceneManager *sm = SceneManager::getSingleton();

    SharedPtr<Renderer> renderer(new OpenALRenderer);
    renderer->setOutputFormat(Format(44100, 16, 2));

    sm->setRenderer(renderer);
}

void closeRenderer() {
    cerr << "Shutting down renderer..." << endl;
    SceneManager::getSingleton()->setRenderer(SharedPtr<Renderer>());
}

void VSExit(int code) {
    VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
    STATIC_VARS_DESTROYED = true;
    exit(code);
}

int main(int argc, char **argv) {
    int rv = 0;
    try {
        initSceneManager();

        // Rendererless tests
        cerr << "Running rendererless tests..." << endl;
        testRendererless();

        // Rendererful tests
        cerr << "Running rendererful tests..." << endl;
        initALRenderer();
        testRendererless(); // <-- whatever worked without renderer must work again with it
        testSimpleScene();
        testSimpleScene();  // Test replay ability
        testSimpleSceneWDynTemplates();
        testSimpleSceneWFileTemplates();

        // Now more complex scenes...
        testComplexScene(false);
        testComplexScene(false); // Test replay ability
        testComplexScene(true);

        // Streaming tests
        testStreaming();
        testMultiStreaming();
        testMultiStreaming2();

        // Tidy close
        closeRenderer();

    } catch (const Exception &e) {
        cerr << "Uncaught exception: "
                << e.what()
                << endl;
        rv = 1;
    };
    if (rv) {
        cout << "FAILED" << endl;
    }
    return rv;
}

};
};
