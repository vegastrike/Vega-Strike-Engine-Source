/*
 * force_feedback.cpp
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

/*
 *  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
 *  Rewritten for the SDL3 haptic API; see force_feedback.h for an overview.
 */

#include "src/force_feedback.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <type_traits>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>

#include "configuration/configuration.h"
#include "src/in_joystick.h"
#include "src/vs_logging.h"
#include "vegadisk/vsfilesystem.h"

namespace {

constexpr size_t kNumEvents = static_cast<size_t>(HapticEvent::Count);

// A beam effect stops when its beam hasn't refreshed it for this long.
constexpr Uint64 kKeepAliveMs = 250;

// Effect names used in force_feedback.json, in HapticEvent order.
const std::array<const char *, kNumEvents> kEffectNames = {
    "stick_feel", "gun_fire", "missile_fire", "beam", "afterburner", "shield_hit", "armor_hit", "hull_hit"
};

HapticEffectSpec MakeSpec(const char *type, float strength, Uint32 length_ms, Uint16 period_ms,
                          Uint16 attack_ms, Uint16 fade_ms, float direction_deg, Uint32 min_interval_ms) {
    HapticEffectSpec spec;
    spec.type = type;
    spec.strength = strength;
    spec.length_ms = length_ms;
    spec.period_ms = period_ms;
    spec.attack_ms = attack_ms;
    spec.fade_ms = fade_ms;
    spec.direction_deg = direction_deg;
    spec.min_interval_ms = min_interval_ms;
    return spec;
}

// Built-in effects, in HapticEvent order. Tuned on a Microsoft SideWinder
// Force Feedback 2; force_feedback.json can override any field.
//                 type       strength length period attack fade direction min_interval
const std::array<HapticEffectSpec, kNumEvents> kDefaultSpecs = {
    MakeSpec("damper",   0.15F,     0,    0,     0,     0,    0.0F,   0),    // stick_feel: light resistance
    MakeSpec("square",   0.35F,    60,   30,     0,    30,    0.0F,  60),    // gun_fire: short recoil buzz
    MakeSpec("sine",     0.55F,   300,   60,     0,   200,    0.0F, 200),    // missile_fire: launch thump
    MakeSpec("square",   0.30F,     0,  200,     0,     0,    0.0F,   0),    // beam: pulses while firing
    MakeSpec("sine",     0.25F,     0,   40,   400,     0,    0.0F,   0),    // afterburner: rumble, ramps up
    MakeSpec("constant", 0.40F,   120,    0,     0,    80,    0.0F,  80),    // shield_hit
    MakeSpec("constant", 0.65F,   180,    0,     0,   100,    0.0F,  80),    // armor_hit
    MakeSpec("constant", 0.90F,   250,    0,     0,   120,    0.0F,  80),    // hull_hit
};

// Converts an HapticEffectSpec into an SDL effect. Returns false for an unknown
// type or zero strength, which leaves that event without an effect.
bool BuildEffect(const HapticEffectSpec &spec, const SDL_HapticDirection &direction, SDL_HapticEffect &effect) {
    SDL_zero(effect);
    const float strength = std::min(std::max(spec.strength, 0.0F), 1.0F);
    if (strength <= 0.0F) {
        return false;
    }
    const auto level = static_cast<Sint16>(std::lround(strength * 32767.0F));
    const Uint32 length = spec.length_ms == 0 ? SDL_HAPTIC_INFINITY : spec.length_ms;

    if (spec.type == "constant") {
        SDL_HapticConstant &c = effect.constant;
        c.type = static_cast<Uint16>(SDL_HAPTIC_CONSTANT);
        c.direction = direction;
        c.length = length;
        c.level = level;
        c.attack_length = spec.attack_ms;   // envelopes ramp from/to zero
        c.fade_length = spec.fade_ms;
        return true;
    }

    Uint32 periodic_type = 0;
    if (spec.type == "sine") {
        periodic_type = SDL_HAPTIC_SINE;
    } else if (spec.type == "square") {
        periodic_type = SDL_HAPTIC_SQUARE;
    } else if (spec.type == "triangle") {
        periodic_type = SDL_HAPTIC_TRIANGLE;
    } else if (spec.type == "sawtooth_up") {
        periodic_type = SDL_HAPTIC_SAWTOOTHUP;
    } else if (spec.type == "sawtooth_down") {
        periodic_type = SDL_HAPTIC_SAWTOOTHDOWN;
    }
    if (periodic_type != 0) {
        SDL_HapticPeriodic &p = effect.periodic;
        p.type = static_cast<Uint16>(periodic_type);
        p.direction = direction;
        p.length = length;
        p.period = std::max<Uint16>(spec.period_ms, 1);
        p.magnitude = level;
        p.attack_length = spec.attack_ms;
        p.fade_length = spec.fade_ms;
        return true;
    }

    // Condition effects react to the stick's position or movement, so they
    // have a strength (coefficient) but no direction.
    Uint32 condition_type = 0;
    if (spec.type == "spring") {
        condition_type = SDL_HAPTIC_SPRING;
    } else if (spec.type == "damper") {
        condition_type = SDL_HAPTIC_DAMPER;
    } else if (spec.type == "friction") {
        condition_type = SDL_HAPTIC_FRICTION;
    } else if (spec.type == "inertia") {
        condition_type = SDL_HAPTIC_INERTIA;
    }
    if (condition_type != 0) {
        SDL_HapticCondition &c = effect.condition;
        c.type = static_cast<Uint16>(condition_type);
        c.length = length;
        for (int axis = 0; axis < 3; ++axis) {
            c.right_sat[axis] = 0xFFFF;
            c.left_sat[axis] = 0xFFFF;
            c.right_coeff[axis] = level;
            c.left_coeff[axis] = level;
        }
        return true;
    }

    VS_LOG(warning, (boost::format("force feedback: unknown effect type '%1%'") % spec.type));
    return false;
}

// SDL directions say where the force comes from: polar, in hundredths of a
// degree clockwise from "away from the pilot".
SDL_HapticDirection PolarDirection(float degrees) {
    SDL_HapticDirection direction;
    SDL_zero(direction);
    direction.type = static_cast<Uint8>(SDL_HAPTIC_POLAR);
    const float wrapped = std::fmod(std::fmod(degrees, 360.0F) + 360.0F, 360.0F);
    direction.dir[0] = static_cast<Sint32>(std::lround(wrapped * 100.0F)) % 36000;
    return direction;
}

// Sets the fields present in a JSON effect object; the rest keep their value.
void ApplyFields(const boost::json::object &fields, HapticEffectSpec &spec) {
    auto number = [&fields](const char *key, auto &out) {
        if (const boost::json::value *v = fields.if_contains(key)) {
            out = static_cast<std::remove_reference_t<decltype(out)>>(v->to_number<double>());
        }
    };
    if (const boost::json::value *v = fields.if_contains("type")) {
        spec.type = v->as_string().c_str();
    }
    number("strength", spec.strength);
    number("length_ms", spec.length_ms);
    number("period_ms", spec.period_ms);
    number("attack_ms", spec.attack_ms);
    number("fade_ms", spec.fade_ms);
    number("direction", spec.direction_deg);
    number("min_interval_ms", spec.min_interval_ms);
}

// Reads one force_feedback.json and applies whatever it sets on top of the
// current values; missing fields keep their value. Example:
//   {
//     "gain": 80,              // overall strength 0..100
//     "autocenter": 0,         // centring spring 0..100 (omit to leave as is)
//     "effects": {
//       "gun_fire": { "type": "square", "strength": 0.5, "length_ms": 60 },
//       "stick_feel": { "strength": 0 }       // strength 0 disables an effect
//     },
//     "weapons": {             // by weapon name; unset fields come from the
//                              // weapon's category (gun_fire, missile_fire, beam)
//       "Laser": { "strength": 0.25 },
//       "ProtonTorpedo": { "strength": 0.9, "length_ms": 500 }
//     }
//   }
void LoadEffectOverrides(const std::string &path, std::array<HapticEffectSpec, kNumEvents> &specs,
                         std::map<std::string, std::string> &weapon_overrides, int &gain, int &autocenter) {
    if (!boost::filesystem::exists(path)) {
        return;
    }
    try {
        std::ifstream file(path);
        std::stringstream text;
        text << file.rdbuf();
        const boost::json::object root = boost::json::parse(text.str()).as_object();

        if (const boost::json::value *v = root.if_contains("gain")) {
            gain = std::min(std::max(static_cast<int>(v->to_number<double>()), 0), 100);
        }
        if (const boost::json::value *v = root.if_contains("autocenter")) {
            autocenter = std::min(std::max(static_cast<int>(v->to_number<double>()), -1), 100);
        }
        if (const boost::json::value *effects = root.if_contains("effects")) {
            for (const auto &entry : effects->as_object()) {
                const std::string key(entry.key().data(), entry.key().size());
                const auto name = std::find(kEffectNames.begin(), kEffectNames.end(), key);
                if (name == kEffectNames.end()) {
                    VS_LOG(warning, (boost::format("force feedback: %1%: unknown effect '%2%'") % path % key));
                    continue;
                }
                ApplyFields(entry.value().as_object(), specs[name - kEffectNames.begin()]);
            }
        }
        // Weapon entries are kept as JSON until the weapon first fires, since
        // only then is its category (and so the effect it starts from) known.
        // A home dir entry adds to, or replaces fields of, the data dir entry.
        if (const boost::json::value *weapons = root.if_contains("weapons")) {
            for (const auto &entry : weapons->as_object()) {
                const std::string key = boost::to_upper_copy(std::string(entry.key().data(), entry.key().size()));
                boost::json::object merged;
                const auto existing = weapon_overrides.find(key);
                if (existing != weapon_overrides.end()) {
                    merged = boost::json::parse(existing->second).as_object();
                }
                for (const auto &field : entry.value().as_object()) {
                    merged[field.key()] = field.value();
                }
                weapon_overrides[key] = boost::json::serialize(merged);
            }
        }
        VS_LOG(info, (boost::format("force feedback: loaded %1%") % path));
    } catch (const std::exception &e) {
        VS_LOG(error, (boost::format("force feedback: error reading %1%: %2%") % path % e.what()));
    }
}

}  // namespace

ForceFeedback::ForceFeedback() {
    std::array<HapticEffectSpec, kNumEvents> specs = kDefaultSpecs;
    // Data dir first (the mod's tuning), then the home dir (the player's).
    LoadEffectOverrides(VSFileSystem::datadir + "/force_feedback.json", specs, weapon_overrides, gain, autocenter);
    LoadEffectOverrides(VSFileSystem::homedir + "/force_feedback.json", specs, weapon_overrides, gain, autocenter);
    for (size_t i = 0; i < kNumEvents; ++i) {
        events[i].spec = specs[i];
    }
    events[static_cast<size_t>(HapticEvent::Beam)].keep_alive = true;
    // Open the device now so the stick feel starts with the game, rather than
    // on the first event.
    EnsureDevice();
}

ForceFeedback::~ForceFeedback() {
    CloseDevice();
    if (subsystem_ready) {
        SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    }
}

bool ForceFeedback::haveFF() {
    return EnsureDevice();
}

// Makes sure the haptic device matches the current settings and joystick.
// Called by every public method; when nothing has changed it is a couple of
// comparisons. The joystick handle identifies the device: it changes when the
// stick is replugged, ff_device is changed, or force feedback is switched off.
bool ForceFeedback::EnsureDevice() {
    const auto &settings = configuration().joystick;
    const int slot = settings.ff_device;
    SDL_Joystick *joy = nullptr;
    if (settings.force_feedback && slot >= 0 && slot < MAX_JOYSTICKS && slot != MOUSE_JOYSTICK
            && joystick[slot] != nullptr && joystick[slot]->joy != nullptr
            && SDL_JoystickConnected(joystick[slot]->joy)) {
        joy = joystick[slot]->joy;
    }
    if (joy != haptic_joystick) {
        CloseDevice();
        // Remembered even if opening fails, so a stick without force feedback
        // is only tried (and logged) once.
        haptic_joystick = joy;
        if (joy != nullptr) {
            OpenDevice(joy);
        }
    }
    return haptic != nullptr;
}

void ForceFeedback::OpenDevice(SDL_Joystick *joy) {
    if (!subsystem_ready) {
        if (!SDL_InitSubSystem(SDL_INIT_HAPTIC)) {
            VS_LOG(error, (boost::format("force feedback: SDL haptic init failed: %1%") % SDL_GetError()));
            return;
        }
        subsystem_ready = true;
    }
    const char *name = SDL_GetJoystickName(joy);
    if (!SDL_IsJoystickHaptic(joy)) {
        VS_LOG(important_info, (boost::format("force feedback: joystick '%1%' has no force feedback")
                % (name ? name : "?")));
        return;
    }
    haptic = SDL_OpenHapticFromJoystick(joy);
    if (haptic == nullptr) {
        VS_LOG(error, (boost::format("force feedback: can't open '%1%': %2%") % (name ? name : "?") % SDL_GetError()));
        return;
    }

    features = SDL_GetHapticFeatures(haptic);
    VS_LOG(important_info, (boost::format("force feedback: using '%1%' (%2% effects, %3% at once, features 0x%4$x)")
            % (name ? name : "?") % SDL_GetMaxHapticEffects(haptic) % SDL_GetMaxHapticEffectsPlaying(haptic) % features));
    if (features & SDL_HAPTIC_GAIN) {
        SDL_SetHapticGain(haptic, gain);
    }
    if (autocenter >= 0 && (features & SDL_HAPTIC_AUTOCENTER)) {
        SDL_SetHapticAutocenter(haptic, autocenter);
    }

    // Upload the event effects now; afterwards they are only started and
    // stopped. Weapon effects are uploaded when the weapon first fires.
    for (size_t i = 0; i < kNumEvents; ++i) {
        Upload(events[i], kEffectNames[i]);
    }
    SetRunning(events[static_cast<size_t>(HapticEvent::StickFeel)], true);
}

void ForceFeedback::CloseDevice() {
    if (haptic != nullptr) {
        SDL_StopHapticEffects(haptic);
        for (Effect &effect : events) {
            if (effect.id >= 0) {
                SDL_DestroyHapticEffect(haptic, effect.id);
            }
        }
        for (auto &weapon : weapons) {
            if (weapon.second.id >= 0) {
                SDL_DestroyHapticEffect(haptic, weapon.second.id);
            }
        }
        SDL_CloseHaptic(haptic);
        haptic = nullptr;
    }
    // Forget device state so everything is uploaded again on the next open.
    for (Effect &effect : events) {
        effect.id = -1;
        effect.upload_tried = false;
        effect.running = false;
    }
    for (auto &weapon : weapons) {
        weapon.second.id = -1;
        weapon.second.upload_tried = false;
        weapon.second.running = false;
    }
}

// Puts an effect on the device if it isn't there yet. Returns whether it is.
bool ForceFeedback::Upload(Effect &effect, const char *name) {
    if (haptic == nullptr) {
        return false;
    }
    if (effect.upload_tried) {
        return effect.id >= 0;
    }
    effect.upload_tried = true;
    SDL_HapticEffect data;
    if (!BuildEffect(effect.spec, PolarDirection(effect.spec.direction_deg), data)) {
        return false;
    }
    if (!(features & data.type)) {
        VS_LOG(warning, (boost::format("force feedback: device can't play '%1%' (%2%)") % name % effect.spec.type));
        return false;
    }
    effect.id = SDL_CreateHapticEffect(haptic, &data);
    if (effect.id < 0) {
        VS_LOG(warning, (boost::format("force feedback: can't create '%1%': %2%") % name % SDL_GetError()));
        return false;
    }
    return true;
}

// The effect to play for a weapon: its own if force_feedback.json lists it
// and it fits on the device, otherwise its category's.
ForceFeedback::Effect &ForceFeedback::EffectFor(const std::string &weapon, HapticEvent category) {
    Effect &fallback = events[static_cast<size_t>(category)];
    const std::string key = boost::to_upper_copy(weapon);
    auto found = weapons.find(key);
    if (found == weapons.end()) {
        const auto fields = weapon_overrides.find(key);
        if (fields == weapon_overrides.end()) {
            return fallback;
        }
        Effect effect;
        effect.spec = fallback.spec;
        effect.keep_alive = fallback.keep_alive;
        try {
            ApplyFields(boost::json::parse(fields->second).as_object(), effect.spec);
        } catch (const std::exception &e) {
            VS_LOG(error, (boost::format("force feedback: weapon '%1%': %2%") % weapon % e.what()));
        }
        found = weapons.emplace(key, effect).first;
    }
    return Upload(found->second, weapon.c_str()) ? found->second : fallback;
}

// Plays a one-shot effect, at most once per min_interval_ms. If direction is
// given the effect is re-uploaded pointing that way first.
void ForceFeedback::Play(Effect &effect, const char *name, const SDL_HapticDirection *direction) {
    if (effect.id < 0) {
        return;
    }
    const Uint64 now = SDL_GetTicks();
    if (now - effect.last_played_ms < effect.spec.min_interval_ms) {
        return;
    }
    effect.last_played_ms = now;
    if (direction != nullptr) {
        SDL_HapticEffect data;
        if (BuildEffect(effect.spec, *direction, data)) {
            SDL_UpdateHapticEffect(haptic, effect.id, &data);
        }
    }
    if (!SDL_RunHapticEffect(haptic, effect.id, 1)) {
        VS_LOG(debug, (boost::format("force feedback: can't play '%1%': %2%") % name % SDL_GetError()));
    }
}

// Starts or stops a continuous effect; only talks to the device on a change.
void ForceFeedback::SetRunning(Effect &effect, bool run) {
    if (run) {
        effect.last_refresh_ms = SDL_GetTicks();
    }
    if (effect.id < 0 || effect.running == run) {
        return;
    }
    effect.running = run;
    if (run) {
        SDL_RunHapticEffect(haptic, effect.id, 1);
    } else {
        SDL_StopHapticEffect(haptic, effect.id);
    }
}

void ForceFeedback::playWeaponFire(const std::string &weapon, bool missile) {
    if (!EnsureDevice()) {
        return;
    }
    Play(EffectFor(weapon, missile ? HapticEvent::MissileFire : HapticEvent::GunFire), weapon.c_str());
}

void ForceFeedback::playBeam(const std::string &weapon) {
    if (!EnsureDevice()) {
        return;
    }
    SetRunning(EffectFor(weapon, HapticEvent::Beam), true);
}

void ForceFeedback::playAfterburner(bool activate) {
    if (!EnsureDevice()) {
        return;
    }
    SetRunning(events[static_cast<size_t>(HapticEvent::Afterburner)], activate);
}

void ForceFeedback::playHit(int layer, float local_x, float local_z) {
    if (!EnsureDevice()) {
        return;
    }
    const HapticEvent event = layer == 0 ? HapticEvent::HullHit
                            : layer == 1 ? HapticEvent::ArmorHit
                            : HapticEvent::ShieldHit;
    Effect &effect = events[static_cast<size_t>(event)];
    const char *name = kEffectNames[static_cast<size_t>(event)];
    if (local_x == 0.0F && local_z == 0.0F) {
        Play(effect, name);  // no side to push from: use the configured direction
        return;
    }
    // The force comes from the side that was hit. In SDL's frame +x is right
    // and -y is away from the pilot; in ship coordinates +x is left and +z is
    // forward. So a hit on the nose pushes the stick back towards the pilot.
    SDL_HapticDirection direction;
    SDL_zero(direction);
    direction.type = static_cast<Uint8>(SDL_HAPTIC_CARTESIAN);
    const float scale = 10000.0F / std::max(std::fabs(local_x), std::fabs(local_z));
    direction.dir[0] = static_cast<Sint32>(-local_x * scale);
    direction.dir[1] = static_cast<Sint32>(-local_z * scale);
    Play(effect, name, &direction);
}

void ForceFeedback::update() {
    if (!EnsureDevice()) {
        return;
    }
    const Uint64 now = SDL_GetTicks();
    auto expire = [this, now](Effect &effect) {
        if (effect.keep_alive && effect.running && now - effect.last_refresh_ms > kKeepAliveMs) {
            SetRunning(effect, false);
        }
    };
    for (Effect &effect : events) {
        expire(effect);
    }
    for (auto &weapon : weapons) {
        expire(weapon.second);
    }
}
