/*
 * force_feedback.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H
#define VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H

/*
 * Force feedback for joysticks, using SDL3's haptic API.
 *
 * Originally written by Alexander Rawass <alexannika@users.sourceforge.net>
 * against the Linux evdev interface. Rewritten to use SDL3 so it works on every
 * platform SDL supports and on the joystick the engine already has open.
 *
 * How it works:
 * - The haptic device is the joystick in slot input.joystick.ff_device (the
 *   same joystick number used by the bindings), opened with
 *   SDL_OpenHapticFromJoystick(). It is opened lazily and re-checked on every
 *   call, so hotplugging the stick, or changing force_feedback/ff_device in the
 *   options screen, takes effect without a restart.
 * - Each game event (gun fire, a hit, ...) has one effect. The effects are
 *   uploaded to the device once when it opens and are then only started and
 *   stopped, which keeps USB traffic to a minimum. Hits update the effect's
 *   direction before playing it.
 * - Weapons can have their own effect, keyed by weapon name. It starts as a
 *   copy of its category's effect (gun_fire, missile_fire or beam) with the
 *   listed fields changed, and is uploaded the first time the weapon fires.
 *   Weapons without one use the category effect.
 * - Beams have no "stopped firing" event, so a player's beam refreshes its
 *   effect every physics frame and update() stops it once refreshes stop.
 * - The effect shapes have built-in defaults (force_feedback.cpp) which can be
 *   tuned per mod or per user with an optional force_feedback.json, read from
 *   the data dir and then the home dir. See LoadEffectOverrides().
 *
 * Every public method is cheap and does nothing when force feedback is off or
 * no haptic device is attached, so callers don't need to check haveFF().
 */

#include <SDL3/SDL_haptic.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_stdinc.h>

#include <array>
#include <map>
#include <string>

// The game events that have a force feedback effect. The names used in
// force_feedback.json are listed in force_feedback.cpp (kEffectNames).
enum class HapticEvent {
    StickFeel,    // continuous: runs while the device is open
    GunFire,      // bolt and ball weapons
    MissileFire,  // anything on a missile mount, e.g. torpedoes
    Beam,         // continuous: runs while a beam is on
    Afterburner,  // continuous: runs while the afterburner is engaged
    ShieldHit,
    ArmorHit,
    HullHit,
    Count
};

// How one effect feels. Strengths are 0..1 and times are milliseconds; they are
// converted to SDL's units when the effect is built (see BuildEffect()).
struct HapticEffectSpec {
    std::string type = "constant";  // constant, sine, square, triangle, sawtooth_up,
                                     // sawtooth_down, spring, damper, friction, inertia
    float strength = 0.5F;
    Uint32 length_ms = 100;          // 0 = play until stopped
    Uint16 period_ms = 50;           // periodic types only
    Uint16 attack_ms = 0;            // ramp-up time, constant and periodic types
    Uint16 fade_ms = 0;              // ramp-down time, constant and periodic types
    float direction_deg = 0.0F;      // where the force comes from: 0 = ahead of the
                                     // pilot, 90 = right. Ignored for hits, which use
                                     // the side of the ship that was hit
    Uint32 min_interval_ms = 0;      // ignore re-triggers sooner than this, so e.g.
                                     // four guns firing on one frame play once
};

class ForceFeedback {
public:
    ForceFeedback();
    ~ForceFeedback();

    bool haveFF();

    // The player fired a bolt, ball or missile weapon (by WeaponInfo name).
    void playWeaponFire(const std::string &weapon, bool missile);
    // Call every physics frame while one of the player's beams is on.
    void playBeam(const std::string &weapon);
    // Starts or stops the afterburner rumble; safe to call every frame.
    void playAfterburner(bool activate);
    // A hit on the player's ship. layer is 0 = hull, 1 = armor, 2 = shield
    // (the order of InflictedDamage::inflicted_damage_by_layer). local_x and
    // local_z are the hit point in ship coordinates (+x left, +z forward); the
    // stick is pushed away from the side that was hit.
    void playHit(int layer, float local_x, float local_z);
    // Call once per frame: picks up a newly attached stick and stops beam
    // effects that are no longer being refreshed.
    void update();

private:
    // One effect and its state on the device.
    struct Effect {
        HapticEffectSpec spec;
        int id = -1;                 // SDL effect id, -1 = not on the device
        bool upload_tried = false;   // so a failed upload isn't retried every shot
        bool running = false;        // continuous effects only
        bool keep_alive = false;     // stops unless refreshed (beams)
        Uint64 last_played_ms = 0;
        Uint64 last_refresh_ms = 0;
    };

    bool EnsureDevice();
    void OpenDevice(SDL_Joystick *joy);
    void CloseDevice();
    bool Upload(Effect &effect, const char *name);
    Effect &EffectFor(const std::string &weapon, HapticEvent category);
    void Play(Effect &effect, const char *name, const SDL_HapticDirection *direction = nullptr);
    void SetRunning(Effect &effect, bool run);

    std::array<Effect, static_cast<size_t>(HapticEvent::Count)> events;
    // Weapon effects, keyed by upper-case weapon name: the JSON fields read
    // from force_feedback.json, and the effects built from them on first use.
    std::map<std::string, std::string> weapon_overrides;
    std::map<std::string, Effect> weapons;
    int gain = 80;        // 0..100, applied if the device supports it
    int autocenter = -1;  // 0..100, or -1 to leave the device's setting alone

    SDL_Haptic *haptic = nullptr;
    SDL_Joystick *haptic_joystick = nullptr;  // the joystick handle haptic was opened from
    Uint32 features = 0;                      // SDL_GetHapticFeatures() of haptic
    bool subsystem_ready = false;
};

#endif //VEGA_STRIKE_ENGINE_FORCE_FEEDBACK_H
