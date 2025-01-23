/*
 * SceneManager.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SCENEMANAGER_H
#define VEGA_STRIKE_ENGINE_AUDIO_SCENEMANAGER_H

//
// C++ Interface: Audio::SceneManager
//

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"
#include "Types.h"
#include "VirtualIterator.h"

namespace Audio {

// Some forwards, we don't need to know the details of those classes
class Renderer;
class Scene;
class SourceTemplate;
class Source;
class Sound;
class Listener;

namespace __impl {

// Forward declaration of internal scene manager data
struct SceneManagerData;

};

/**
 * Audio Scene Manager.
 *
 * @remarks Use it to create and manage sources, listeners, instantiate source templates,
 *      scenes, and other various scene management tasks.
 *      @par SceneManager s will handle the complex task of prioritizing sources, managing
 *      source channels, volume levels, and doing all kinds of culling.
 *      @par Another responsability of scene managers is the manipulation and analysis of
 *      environments to feed environmental information to capable renderers.
 *      @par Inactive sources without external references are automatically destroyed when
 *      they become permanently inactive (notice this requirement is merely approximate -
 *      implementations are free to make reasonable approximations for the determination
 *      of such condition as "permanently inactive"). This allows for "fire & forget" sources,
 *      where the source is activated (by calling startPlaying), and then immediately
 *      unreferenced. The SceneManager will keep track of that source, as it would any other,
 *      until it stops playing (make sure though it's not a looping source!), and will remove
 *      it when that happens, freeing any associated resources.
 *      @par In fact this is so common as to make it part of the API: playSource does exactly
 *      that, it creates a source, plays it, and discards the reference.
 *      @par Thus, SceneManager s are the hub of all activity relating sound. If you're doing
 *      anything bypassing the SceneManager, you're doing it wrong.
 *      @par This class is NOT abstract, it provides a very basic implementation that does
 *      nothing special.
 * @note You must set a renderer or you won't hear anything ;)
 *
 */
class SceneManager : public Singleton<SceneManager> {
private:
    AutoPtr<__impl::SceneManagerData> data;

public:
    typedef VirtualIterator<SharedPtr<Scene> > SceneIterator;

protected:
    /** Returns the renderer
     * @remarks Throws an exception if no renderer has been set
     */
    const SharedPtr<Renderer> &internalRenderer() const;

public:
    /** Construct a new instance of the manager
     * @remarks End-users of the class shouldn't be using this. Singletons need it.
     *      Instead, end-users should use the Root class to find manager plugins they
     *      like.
     */
    SceneManager();

    virtual ~SceneManager();

    /** Create a new source based on the speicified sound
     * @remarks All the attributes will hold unspecified values, so you have to fill them in.}
     * @note The sound must be associated to the correct renderer, or bad things will happen.
     * @see Renderer, which creates sounds.
     */
    virtual SharedPtr<Source> createSource(SharedPtr<Sound> sound, bool looping = false);

    /** Create a new source based on the specified template
     * @remarks All location information will hold unspecified values, so you have to fill them in.
     */
    SharedPtr<Source> createSource(SharedPtr<SourceTemplate> tpl);

    /** Create a new source based on the specified template, but overriding its sound resource.
     * @remarks All location information will hold unspecified values, so you have to fill them in.
     *      @par It's useful to have "mode" templates that can be used to spawn many sources based
     *      on many different streams. Eg: a "music" template for spawning music tracks, a "radio"
     *      template for spawning radio voiceovers, etc...
     */
    SharedPtr<Source> createSource(SharedPtr<SourceTemplate> tpl, const std::string &name);

    /** Destroy a source created with this manager */
    virtual void destroySource(SharedPtr<Source> source);

    /** Convenience API to play a source once and forget.
     * @param tpl The source template from which a source should be instanced
     * @param sceneName The name of the scene to which the source should be attached
     * @param position The initial position of the source
     * @param direction The direction of the (if directional) source
     * @param velocity The movement velocity of the source
     * @param radius The base radius of the source
     * @remarks The source should not be looping, and an exception will be thrown if it is.
     */
    void playSource(
            SharedPtr<SourceTemplate> tpl,
            const std::string &sceneName,
            LVector3 position,
            Vector3 direction,
            Vector3 velocity,
            Scalar radius);

    /** Convenience API to play a source once and forget.
     * @param tpl The source template from which a source should be instanced
     * @param soundName The name of the sound stream from which the source will be created, overriding the template's
     * @param sceneName The name of the scene to which the source should be attached
     * @param position The initial position of the source
     * @param direction The direction of the (if directional) source
     * @param velocity The movement velocity of the source
     * @param radius The base radius of the source
     * @remarks The source should not be looping, and an exception will be thrown if it is.
     */
    void playSource(
            SharedPtr<SourceTemplate> tpl,
            const std::string &soundName,
            const std::string &sceneName,
            LVector3 position,
            Vector3 direction,
            Vector3 velocity,
            Scalar radius);

    /** Create a new named scene */
    virtual SharedPtr<Scene> createScene(const std::string &name);

    /** Get an existing scene by its name */
    virtual SharedPtr<Scene> getScene(const std::string &name) const;

    /** Destroy an existing scene by its name */
    virtual void destroyScene(const std::string &name);

    /** Sets the active state of a scene */
    virtual void setSceneActive(const std::string &name, bool active);

    /** Get the active state of a scene */
    virtual bool getSceneActive(const std::string &name);

    /** Get the root listener
     * @remarks Renderers can only have one listener. Sources attached to scenes
     *      require translation into the reference listener. It is usually convenient
     *      to apply some listener changes directly into the root listener rather than
     *      into the scenes. For instance, if one scene represents the cockpit and
     *      another the ship's exterior, the cockpit listener will be fixed and the
     *      exterior listener will move with the ship. But if the user wants to
     *      "look to the right", the best way to achieve this would be to rotate
     *      the renderer's listener to the right rather than rotate all sources
     *      to the left (what would be required if the listeners of each scene
     *      were rotated, since they're "artificial" listeners).
     */
    virtual SharedPtr<Listener> getRootListener() const;

    /** Get an iterator over all scenes */
    virtual SharedPtr<SceneIterator> getSceneIterator() const;

    /** Get an iterator over all active scenes */
    virtual SharedPtr<SceneIterator> getActiveSceneIterator() const;

    /** Set a new renderer
     * @param renderer A new renderer to be used.
     * @remarks Setting the renderer should be done at the very beginning. It is possible
     *      to switch renderers at any point, but the operation is rather costly: all
     *      sounds must be unloaded and recreated, all active sources must be detached and
     *      reattached.
     * @note Overriding implementations must call the base implementation, since
     *      getRenderer is not overridable.
     */
    virtual void setRenderer(SharedPtr<Renderer> renderer);

    /** Get the current renderer */
    SharedPtr<Renderer> getRenderer() const;


    /********* Scene cycle **********/

    /** Commit changes done between frames
     * @remarks Depending on the underlying implementation, changes applied to sources
     *      may or may not be immediately available to the renderer. Calling commit()
     *      ensures that they are. Furthermore, some scene managers may use commit
     *      boundaries to ensure frame coherence (the renderer receives information
     *      coherent to the state as it was at the commit), though this is certainly not
     *      a requirement.
     *      @par The process may be lengthy and throw various exceptions.
     */
    virtual void commit();

    /** Return position update frequency
     * @remarks Source position updates are a very important kind of update that needs to be
     *      performed regularly and quite often. They are costly (all active sources have
     *      to be updated). This value specifies how often they're updated, however the
     *      actual interval may vary depending on the implementation (it's a mere guideline).
     */
    Duration getPositionUpdateFrequency() const;

    /** Return listener update frequency
     * @remarks Position updates are a very important kind of update that needs to be
     *      performed regularly and quite often - even more so for listeners.
     *      Since there are very few listeners (one per scene), they're perhaps not so
     *      costly, or perhaps so? (depending on the underlying implementation).
     *      This value specifies how often they're updated, however the
     *      actual interval may vary depending on the implementation (it's a mere guideline).
     */
    Duration getListenerUpdateFrequency() const;

    /** Return attribute update frequency
     * @remarks Source attribute updates are rare but necessary.
     *      They are very costly since all active sources must be updated (and each update
     *      is far more costly than simple position updates).
     *      This value specifies how often they're updated, however the
     *      actual interval may vary depending on the implementation (it's a mere guideline).
     */
    Duration getAttributeUpdateFrequency() const;

    /** Return source activation frequency
     * @remarks Sources may become active or inactive over time, and depending on the impementation
     *      the effects may not be immediate. More so, since there's a "maximum renderable sources"
     *      limit, activation may depend on a lot of factors, and not just a call to startPlaying().
     *      So, activation updates are the costlier of all updates since it involves evaluating
     *      and prioritizing all sources in all scenes, not just active ones (sources). Frequent
     *      activation passes are however necessary (but perhaps not that often) since otherwise
     *      sources that didn't start off as active may never become so.
     */
    Duration getActivationFrequency() const;

    /** @see getPositionUpdateFrequency */
    virtual void setPositionUpdateFrequency(Duration interval) const;

    /** @see getListenerUpdateFrequency */
    virtual void setListenerUpdateFrequency(Duration interval) const;

    /** @see getAttributeUpdateFrequency */
    virtual void setAttributeUpdateFrequency(Duration interval) const;

    /** @see getActivationFrequency */
    virtual void setActivationFrequency(Duration interval) const;


    /********* Culling parameters **********/


    /** Get the maximum number of simultaneous sources that can be playing at a time
     * @remarks This value may be approximate, and it refers to the maximum number
     *      of Source class instances that can effectively be in playing state.
     *      Some sources, when environmental effects are being applied, can count
     *      more than once towards the effective limit, and the manager will try
     *      to compensate this when setting the "MaxSources" attribute. However,
     *      this compensation may not be perfect.
     */
    virtual unsigned int getMaxSources() const;

    /** Set the maximum number of simultaneous sources that can be playing at a time
     * @param n The maximum number of simultaneous playing sources desired.
     * @remarks This is not guaranteed to success. If failure arises, either no change
     *      or a seemingly approximate change will be made (ie: if the specified number
     *      is too high, the closest possible one will be set instead).
     * @see getMaxSources
     */
    virtual void setMaxSources(unsigned int n);

    /** Get the minimum gain that would be culled off
     * @remarks This value specifies the minimum gain of active sources. If a source
     *      ends having a lesser gain, it is ignored and deactivated, to conserve resources.
     *      @par The manager is free to approximate such determination in order to optimize
     *      culling: for instance, it could, given attenuation factors and the like, compute
     *      a maximum distance that would result in gains potentially greater than this value
     *      to avoid processing of any source beyond that distance. However, such a computation
     *      is required to be conservative (never cull sources that would fall outside of the
     *      culling rule based on actual gain), and as such may be difficult without aid.
     *      @see For more culling options: get/setMaxDistance
     */
    virtual float getMinGain() const;

    /** Set the minimum gain that would be culled off
     * @param gain The new minimum gain.
     * @see getMinGain
     */
    virtual void setMinGain(float gain);

    /** Get the maximum distance of active sources
     * @remarks This value specifies the maximum distance of active sources. If a source
     *      is at a greater distance from the listener, it is ignored and deactivated,
     *      to conserve resources.
     *      @see For more culling options: get/setMinGain
     */
    virtual double getMaxDistance() const;

    /** Set the maximum distance of active sources
     * @param distance The new limit.
     * @see getMaxDistance
     */
    virtual void setMaxDistance(double distance);


    /*********** Notification events ************/

    /** Notify the scene manager of a source that starts or stops playing. */
    virtual void notifySourcePlaying(SharedPtr<Source> source, SharedPtr<Scene> scene, bool playing);

protected:
    /** Add a new scene @see createScene */
    void addScene(SharedPtr<Scene> scene);

    /** Synchronize activation state with the scenes */
    virtual void activationPhaseImpl();

    /** Synchronize source positions/attributes with the renderer */
    virtual void updateSourcesImpl(bool withAttributes);

    /** Synchronize listeners
     * @remarks Since renderer implementations require one listener, this only updates
     *      the root listener. Scene listeners fall under the category of position updates.
     */
    virtual void updateListenerImpl(bool withAttributes);
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SCENEMANAGER_H
