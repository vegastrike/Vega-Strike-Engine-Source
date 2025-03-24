/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "unit_generic.h"
#include "missile.h"
#include "beam.h"
#include "cmd/bolt.h"
#include "audiolib.h"
#include "missile.h"
#include "ai/order.h"
#include "ai/fireall.h"
#include "ai/script.h"
#include "ai/navigation.h"
#include "ai/flybywire.h"
#include "root_generic/configxml.h"
#include "gfx_generic/cockpit_generic.h"
#include "force_feedback.h"
#include "ai/aggressive.h"
#include "root_generic/lin_time.h"
#include "root_generic/vsfilesystem.h"
#include "src/star_system.h"
#include "src/universe.h"
#include "cmd/weapon_info.h"
#include "resource/resource.h"
#include "gfx_generic/boltdrawmanager.h"

extern char SERVER;

Mount::Mount() {
    static WeaponInfo wi(WEAPON_TYPE::BEAM);
    functionality = 1;
    maxfunctionality = 1;
    type = &wi;
    size = as_integer(MOUNT_SIZE::NOWEAP);
    functionality = maxfunctionality = 1.0f;
    ammo = -1;
    status = UNCHOSEN;
    processed = Mount::PROCESSED;
    sound = -1;
    last_sound_refire_time = 0.0;
    static float xyscalestat = XMLSupport::parse_float(vs_config->getVariable("graphics", "weapon_xyscale", "1"));

    static float zscalestat = XMLSupport::parse_float(vs_config->getVariable("graphics", "weapon_zscale", "1"));
    xyscale = xyscalestat;
    zscale = zscalestat;
}

extern double interpolation_blend_factor;

void DestroyMount(Mount *mount) {
    mount->UnFire();
    AUDStopPlaying(mount->sound);
    mount->status = Mount::DESTROYED;
}

float Mount::ComputeAnimatedFrame(Mesh *gun) {
    if (type->type == WEAPON_TYPE::BEAM) {
        if (ref.gun) {
            if (ref.gun->Ready()) {
                return getNewTime() + type->Refire() - ref.gun->refireTime()
                        - interpolation_blend_factor * simulation_atom_var;
            } else {
                return getNewTime() * gun->getFramesPerSecond();
            }
        } else {
            return 0;
        }
    } else {
        if (ref.refire < type->Refire()) {
            return getNewTime() * gun->getFramesPerSecond();
        } else {
            return getNewTime() + type->Refire() - ref.refire - interpolation_blend_factor * simulation_atom_var;
        }
    }
}

Mount::Mount(const string &filename, int am, int vol, float xyscale, float zscale, float func, float maxfunc,
        bool banked) : bank(banked) {
    //short fix
    functionality = func;
    maxfunctionality = maxfunc;
    static WeaponInfo wi(WEAPON_TYPE::BEAM);
    size = as_integer(MOUNT_SIZE::NOWEAP);
    static float xyscalestat = XMLSupport::parse_float(vs_config->getVariable("graphics", "weapon_xyscale", "1"));

    static float zscalestat = XMLSupport::parse_float(vs_config->getVariable("graphics", "weapon_zscale", "1"));
    if (xyscale == -1) {
        xyscale = xyscalestat;
    }
    if (zscale == -1) {
        zscale = zscalestat;
    }
    this->zscale = zscale;
    this->xyscale = xyscale;
    ammo = am;
    sound = -1;
    type = &wi;
    this->volume = vol;
    ref.gun = NULL;
    status = (UNCHOSEN);
    processed = Mount::PROCESSED;
    WeaponInfo *temp = getWeapon(filename);
    if (temp == NULL) {
        status = UNCHOSEN;
        time_to_lock = 0;
    } else {
        type = temp;
        status = ACTIVE;
        time_to_lock = temp->lock_time;
        if (type->type != WEAPON_TYPE::BEAM) {
            ref.refire = type->Refire();
        }
    }
}

extern bool AdjustMatrix(Matrix &mat, const Vector &velocity, Unit *target, float speed, bool lead, float cone);

void AdjustMatrixToTrackTarget(Matrix &mat, const Vector &velocity, Unit *target, float speed, bool lead, float cone) {
    AdjustMatrix(mat, velocity, target, speed, lead, cone);
}

void Mount::UnFire() {
    processed = UNFIRED;
    if (status != ACTIVE || ref.gun == NULL || type->type != WEAPON_TYPE::BEAM) {
        return;
    }
    ref.gun->Destabilize();
}

void Mount::ReplaceMounts(Unit *un, const Mount *other) {
    int thisvol = volume;     //short fix
    int thissize = size;     //short fix
    float xyscale = this->xyscale;
    float zscale = this->zscale;
    bool thisbank = this->bank;
    Quaternion q = this->GetMountOrientation();
    Vector v = this->GetMountLocation();
    *this = *other;
    this->size = thissize;
    volume = thisvol;
    this->SetMountPosition(v);
    this->SetMountOrientation(q);
    this->xyscale = xyscale;
    this->zscale = zscale;
    this->bank = thisbank;
    ref.gun = NULL;
    if (type->type != WEAPON_TYPE::BEAM) {
        ref.refire = type->Refire();
    }
    this->ReplaceSound();
    if (other->ammo == -1) {
        ammo = -1;
    } else if (other->ammo != -1 && ammo == -1) {
        ammo = 0;
    }          //zero ammo if other was not zero earlier.
    un->setAverageGunSpeed();
}

double Mount::Percentage(const Mount *newammo) const {
    float percentage = 1. / 1024;
    int thingstocompare = 0;
    if (status == UNCHOSEN || status == DESTROYED) {
        return percentage;
    }
    if (newammo->ammo == -1) {
        if (ammo != -1) {
            thingstocompare++;
        } else {
            if (newammo->type->range == type->range && newammo->type->damage == type->damage
                    && newammo->type->phase_damage
                            == type->phase_damage) {
                return 1;
            }
            if (newammo->type->name == type->name) {
                return 1;
            }
        }
    } else if (newammo->ammo > 0) {
        percentage += .25;
        thingstocompare++;
        if (ammo > 0) {
            if (newammo->type->range == type->range && newammo->type->damage == type->damage
                    && newammo->type->phase_damage
                            == type->phase_damage) {
                return 1;
            }
            if (newammo->type->name == type->name) {
                return 1;
            }
        }
    }
    if (newammo->type->range) {
        if (type->range > newammo->type->range) {
            percentage += .25;
        }
        thingstocompare++;
    }
    if (newammo->type->damage + 100 * newammo->type->phase_damage) {
        if (type->damage + 100 * type->phase_damage > newammo->type->damage + 100 * newammo->type->phase_damage) {
            percentage += .75;
        }
        thingstocompare++;
    }
    if (thingstocompare) {
        return percentage / thingstocompare;
    } else {
        return 1. / 1024;
    }
}

extern void GetMadAt(Unit *un, Unit *parent, int numhits = 0);

//bool returns whether to refund the cost of firing
bool Mount::PhysicsAlignedFire(Unit *caller,
        const Transformation &Cumulative,
        const Matrix &m,
        const Vector &velocity,
        void *owner,
        Unit *target,
        signed char autotrack,
        float trackingcone,
        CollideMap::iterator hint[]) {
    using namespace VSFileSystem;
    if (time_to_lock > 0) {
        target = NULL;
    }
    static bool lock_disrupted_by_false_fire =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "out_of_arc_fire_disrupts_lock", "false"));
    if (lock_disrupted_by_false_fire) {
        time_to_lock = type->lock_time;
    }
    if (processed == FIRED) {
        if (type->type == WEAPON_TYPE::BEAM || type->isMissile()) {
            //Missiles and beams set to processed.
            processed = PROCESSED;
        } else if (ref.refire < type->Refire() || type->energy_rate > caller->energy.Level()) {
            //Wait until refire has expired and reactor has produced enough energy for the next bolt.
            return true;
        }              //Not ready to refire yet.  But don't stop firing.

        Unit *temp;
        Transformation tmp(orient, pos.Cast());
        tmp.Compose(Cumulative, m);
        Matrix mat;
        tmp.to_matrix(mat);
        mat.p = Transform(mat, (type->offset + Vector(0, 0, zscale)).Cast());
        static bool firemissingautotrackers =
                XMLSupport::parse_bool(vs_config->getVariable("physics", "fire_missing_autotrackers", "true"));
        if (autotrack && NULL != target) {
            if (!AdjustMatrix(mat, velocity, target, type->speed, autotrack >= 2, trackingcone)) {
                if (!firemissingautotrackers) {
                    return false;
                }
            }
        } else if (this->size & as_integer(MOUNT_SIZE::AUTOTRACKING)) {
            if (!firemissingautotrackers) {
                return false;
            }
        }
        if (type->type != WEAPON_TYPE::BEAM) {
            ref.refire = 0;
            if (ammo > 0) {
                ammo--;
            }
        } else {
            static bool reduce_beam_ammo =
                    XMLSupport::parse_bool(vs_config->getVariable("physics", "reduce_beam_ammo", "0"));
            if (ammo > 0 && reduce_beam_ammo) {
                ammo--;
            }
        }
        time_to_lock = type->lock_time;
        switch (type->type) {
            case WEAPON_TYPE::UNKNOWN:
                break;
            case WEAPON_TYPE::BEAM:
                if (ref.gun) {
                    ref.gun->Reinitialize();
                }
                break;
            case WEAPON_TYPE::BOLT:
                caller->energy.Deplete(true, type->energy_rate);
                hint[Unit::UNIT_BOLT] = Bolt(type,
                        mat,
                        velocity,
                        owner,
                        hint[Unit::UNIT_BOLT]).location;             //FIXME turrets won't work! Velocity
                break;

            case WEAPON_TYPE::BALL: {
                caller->energy.Deplete(true, type->energy_rate);
                hint[Unit::UNIT_BOLT] =
                        BoltDrawManager::GetInstance().AddBall(type, mat, velocity, owner, hint[Unit::UNIT_BOLT]);
                break;
            }
            case WEAPON_TYPE::PROJECTILE:
                static bool match_speed_with_target =
                        XMLSupport::parse_float(vs_config->getVariable("physics", "match_speed_with_target", "true"));
                string skript = /*string("ai/script/")+*/ type->file + string(".xai");
                VSError err = LookForFile(skript, AiFile);
                if (err <= Ok) {
                    temp = new Missile(
                            type->file.c_str(),
                            caller->faction,
                            "",
                            type->damage,
                            type->phase_damage,
                            type->range / type->speed,
                            type->radius,
                            type->radial_speed,
                            type->pulse_speed /*detonation_radius*/);
                    if (!match_speed_with_target) {
                        temp->drive.speed = type->speed + velocity.Magnitude();
                        temp->afterburner.speed = type->speed + velocity.Magnitude();
                    }
                } else {
                    Flightgroup *testfg = caller->getFlightgroup();
                    if (testfg == NULL) {
                        static Flightgroup bas;
                        bas.name = "Base";
                        testfg = &bas;
                    }
                    if (testfg->name == "Base") {
                        int fgsnumber = 0;
                        Flightgroup *fg = Flightgroup::newFlightgroup("Base_Patrol",
                                type->file,
                                FactionUtil::GetFactionName(caller->faction),
                                "deafult",
                                1,
                                1,
                                "",
                                "",
                                mission);
                        if (fg != NULL) {
                            fg->target.SetUnit(caller->Target());
                            fg->directive = "a";
                            fg->name =
                                    "Base_Patrol";                               //this fixes base-spawned fighters becoming navpoints, which happens sometimes

                            fgsnumber = fg->nr_ships;
                            fg->nr_ships = 1;
                            fg->nr_ships_left = 1;
                        }
                        temp = new Unit(type->file.c_str(), false, caller->faction, "", fg, fgsnumber);
                    } else {
                        Flightgroup *fg = caller->getFlightgroup();
                        int fgsnumber = 0;
                        if (fg != NULL) {
                            fgsnumber = fg->nr_ships;
                            fg->nr_ships++;
                            fg->nr_ships_left++;
                        }
                        temp = new Unit(type->file.c_str(), false, caller->faction, "", fg, fgsnumber);
                    }
                }
                Vector adder = Vector(mat.r[6], mat.r[7], mat.r[8]) * type->speed;
                temp->SetVelocity(caller->GetVelocity() + adder);

                if (target && target != owner) {
                    temp->Target(target);
                    temp->TargetTurret(target);
                    if (err <= Ok) {
                        temp->EnqueueAI(new AIScript((type->file + ".xai").c_str()));
                        temp->EnqueueAI(new Orders::FireAllYouGot);
                        if (match_speed_with_target) {
                            temp->GetComputerData().velocity_ref.SetUnit(target);
                        }
                    } else {
                        temp->EnqueueAI(new Orders::AggressiveAI("default.agg.xml"));
                        temp->SetTurretAI();
                        temp->TurretFAW();                         //turrets are for DEFENSE damnit!
                        temp->owner =
                                caller;                         //spawned wingmen act as cargo (owned) wingmen, not as hired wingmen
                        float relat;
                        relat = caller->getRelation(target);
                        if (caller->isSubUnit() && relat >= 0) {
                            relat = -1;
                            temp->owner = caller->owner;
                        }
                        if (relat < 0) {
                            int i = 0;
                            while (relat < temp->getRelation(target) && i++ < 100) {
                                GetMadAt(target, temp, 2);
                            }
                        }
                        //pissed off					getMadAt(target, 10); // how do I cause an attack here?
                    }
                } else {
                    temp->EnqueueAI(new Orders::MatchLinearVelocity(Vector(0, 0, 100000), true, false));
                    temp->EnqueueAI(new Orders::FireAllYouGot);
                }
                temp->SetOwner((Unit *) owner);
                temp->Velocity = velocity + adder;
                temp->curr_physical_state = temp->prev_physical_state = temp->cumulative_transformation = tmp;
                CopyMatrix(temp->cumulative_transformation_matrix, m);
                _Universe->activeStarSystem()->AddUnit(temp);
                temp->UpdateCollideQueue(_Universe->activeStarSystem(), hint);
                for (unsigned int locind = 0; locind < Unit::NUM_COLLIDE_MAPS; ++locind) {
                    if (!is_null(temp->location[locind])) {
                        hint[locind] = temp->location[locind];
                    }
                }

                break;
        }
        static bool use_separate_sound =
                XMLSupport::parse_bool(vs_config->getVariable("audio", "high_quality_weapon", "true"));
        static bool ai_use_separate_sound =
                XMLSupport::parse_bool(vs_config->getVariable("audio", "ai_high_quality_weapon", "false"));
        static bool ai_sound = XMLSupport::parse_bool(vs_config->getVariable("audio", "ai_sound", "true"));
        Cockpit *cp;
        bool ips = ((cp = _Universe->isPlayerStarshipVoid(owner)) != NULL);
        double distancesqr = (tmp.position - AUDListenerLocation()).MagnitudeSquared();
        static double maxdistancesqr =
                XMLSupport::parse_float(vs_config->getVariable("audio", "max_range_to_hear_weapon_fire",
                        "100000"))
                        * XMLSupport::parse_float(vs_config->getVariable("audio",
                                "max_range_to_hear_weapon_fire",
                                "100000"));
        static float weapon_gain =
                XMLSupport::parse_float(vs_config->getVariable("audio", "weapon_gain", ".25"));
        static float exterior_weapon_gain =
                XMLSupport::parse_float(vs_config->getVariable("audio", "exterior_weapon_gain", ".35"));
        static float min_weapon_sound_refire =
                XMLSupport::parse_float(vs_config->getVariable("audio", "min_weapon_sound_refire", ".2"));
        float curtime = realTime();
        bool tooquick = ((curtime - last_sound_refire_time) < min_weapon_sound_refire);
        if (!tooquick) {
            last_sound_refire_time = curtime;

            QVector sound_pos;
            Vector sound_vel;
            float sound_gain;

            if (ips && cp != NULL && cp->GetView() <= CP_RIGHT) {
                sound_pos = QVector(0, 0, 0);
                sound_vel = Vector(0, 0, 0);
                sound_gain = weapon_gain;
            } else {
                sound_pos = tmp.position;
                sound_vel = velocity;
                sound_gain = exterior_weapon_gain;
            }

            if ((((!use_separate_sound)
                    || type->type == WEAPON_TYPE::BEAM)
                    || ((!ai_use_separate_sound) && !ips)) && !type->isMissile()) {
                if (ai_sound || (ips && type->type == WEAPON_TYPE::BEAM)) {
                    if (!AUDIsPlaying(sound)) {
                        AUDPlay(sound, sound_pos, sound_vel, sound_gain);
                    } else {
                        if (distancesqr < maxdistancesqr) {
                            if (type->type == WEAPON_TYPE::BEAM) {
                                // Beams don't re-start, they keep playing
                                AUDAdjustSound(sound, sound_pos, sound_vel);
                                AUDSoundGain(sound, sound_gain);
                            } else {
                                // Other kinds of weapons just re-play the sound
                                AUDStopPlaying(sound);
                                AUDPlay(sound, sound_pos, sound_vel, sound_gain);
                            }
                        } else {
                            AUDStopPlaying(sound);
                        }
                    }
                }
            } else if ((ai_sound || ips) && distancesqr < maxdistancesqr) {
                int snd = AUDCreateSound(sound, false);
                AUDPlay(sound, sound_pos, sound_vel, sound_gain);
                AUDDeleteSound(snd);
            }
        }
        if (type->isMissile() && ammo == 0) {
            status = UNCHOSEN;
        }
    }
    return true;
}

bool Mount::NextMountCloser(Mount *nextmount, Unit *firer) {
    Unit *target;
    if (nextmount && (target = firer->Target())) {
        Matrix mat;
        nextmount->orient.to_matrix(mat);
        Vector nextR = mat.getR();
        this->orient.to_matrix(mat);
        Vector diff = firer->LocalCoordinates(target);
        Vector nextmountnorm = diff - nextmount->pos;
        nextmountnorm.Normalize();
        Vector thismountnorm = diff - this->pos;
        thismountnorm.Normalize();
        return nextR.Dot(nextmountnorm) > mat.getR().Dot(thismountnorm);
    }
    return false;
}

bool Mount::Fire(Unit *firer, void *owner, bool Missile, bool listen_to_owner) {
    if (ammo == 0) {
        processed = UNFIRED;
    }
    if (processed == FIRED || status != ACTIVE || (type->isMissile() != Missile) || ammo == 0) {
        return false;
    }
    if (type->type == WEAPON_TYPE::BEAM) {
        bool fireit = ref.gun == NULL;
        if (!fireit) {
            fireit = ref.gun->Ready();
        } else {
            ref.gun = new Beam(Transformation(orient, pos.Cast()), *type, owner, firer, sound);
        }
        if (fireit) {
            ref.gun->ListenToOwner(listen_to_owner);
            processed = FIRED;
        }
        return true;
    } else if (ref.refire >= type->Refire()) {
        processed = FIRED;
        if (owner == _Universe->AccessCockpit()->GetParent()) {
            forcefeedback->playLaser();
        };

        return true;
    }
    return false;
}

void Mount::PhysicsAlignedUnfire() {
    //Stop Playing SOund?? No, that's done in the beam, must not be aligned
    if (processed == UNFIRED) {
        if (AUDIsPlaying(sound)) {
            AUDStopPlaying(sound);
        }
    }
}

void Mount::ReplaceSound() {
    sound = AUDCreateSound(sound, false);     //copy constructor basically
}

void Mount::Activate(bool Missile) {
    if (type->isMissile() == Missile) {
        if (status == INACTIVE) {
            status = ACTIVE;
        }
    }
}

///Sets this gun to inactive, unless unchosen or destroyed
void Mount::DeActive(bool Missile) {
    if (type->isMissile() == Missile) {
        if (status == ACTIVE) {
            status = INACTIVE;
        }
    }
}

void Mount::SetMountPosition(const Vector &v) {
    pos = v;
}

void Mount::SetMountOrientation(const Quaternion &t) {
    orient = t;
}

bool Mount::AddAmmo(const Mount *upgrading_mount, const Unit* weapon, bool perform_action) {
    // Weapons don't match so ammo doesn't either
    if(upgrading_mount->type->name != type->name) {
        return false;
    }

    // If either is unlimited?
    if(upgrading_mount->ammo == -1 || ammo == -1) {
        return false;
    }

    int tmp_ammo = this->ammo;
    tmp_ammo += upgrading_mount->ammo;

    if(type->isMissile()) {
        if (tmp_ammo * this->type->volume > this->volume) {
            tmp_ammo = (int) floor(0.125 + ((0 + this->volume) / this->type->volume));
        }
    } else {
        // TODO: make load_failed a single variable throughout
        if (weapon == nullptr || weapon->name == "LOAD_FAILED") {
            // this should not happen
            VS_LOG(info,(boost::format("UpgradeMount(): FAILED to obtain weapon: %1%")
                    % weapon->name));
            return false;
        }

        int max_ammo = weapon->mounts[0].ammo;

        if (tmp_ammo > max_ammo) {
            tmp_ammo = max_ammo;
        }
    }

    if (tmp_ammo <= this->ammo) {
        return false;
    }

    if (perform_action) {
            this->ammo = tmp_ammo;
    }

    return true;
}


// Almost certain the parameter is not required
// A beam weapon has ammo=-1, everything else is >=0
void Mount::Deactivate(bool is_ammo, bool is_missile_mount) {
    status = Mount::UNCHOSEN;
    if(!is_missile_mount) {
        ammo = -1;
    }
}


bool Mount::IsMissileMount() const {
    int mount_size = as_integer(this->type->size);
    return (mount_size & (
            as_integer(MOUNT_SIZE::LIGHTMISSILE) |
                    as_integer(MOUNT_SIZE::MEDIUMMISSILE) |
                    as_integer(MOUNT_SIZE::HEAVYMISSILE) |
                    as_integer(MOUNT_SIZE::CAPSHIPLIGHTMISSILE) |
                    as_integer(MOUNT_SIZE::CAPSHIPHEAVYMISSILE) |
                    as_integer(MOUNT_SIZE::SPECIALMISSILE))) != 0;
}

bool Mount::MountFits(const Mount upgrading_mount) const {
    return ((unsigned int) (as_integer(upgrading_mount.type->size))
                           == (as_integer(upgrading_mount.type->size) & this->size));
}

bool Mount::SameWeapon(const Mount* other_mount) const {
    return other_mount->type->name == this->type->name;
}

bool Mount::CanUpgradeMount(const Mount* other_mount,
                            Unit* unit,
                            const bool is_ammo,
                            int &numave,        // number of used parts?
                            double &percentage,
                            bool perform_upgrade) {
    //If missile, can upgrade directly, if other type of ammo, needs actual gun to be present.
    if (is_ammo && !other_mount->IsMissileMount()) {
        return false;
    }

    ++numave;                                 //ok now we can compute percentage of used parts
    Mount upmount(*other_mount);

    //compute here
    percentage += Percentage(&upmount);

    //if we wish to modify the mounts
    if (perform_upgrade) {
        //switch this mount with the upgrador mount
        ReplaceMounts(unit, &upmount);
    }

    return true;
}
