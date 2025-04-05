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


#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet.h"
#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "src/vs_logging.h"
#include "cmd/unit_util.h"
#include "cmd/script/flightgroup.h"
#include "cmd/role_bitmask.h"
#include "cmd/ai/communication.h"
#include "src/universe_util.h"
#include <algorithm>
#include "cmd/unit_find.h"
#include "src/vs_random.h"
#include "root_generic/lin_time.h" //DEBUG ONLY
#include "cmd/pilot.h"
#include "src/universe.h"
#include "cmd/unit_util.h"

extern int numprocessed;
extern double targetpick;

static bool NoDockWithClear() {
    static bool nodockwithclear =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "dock_with_clear_planets", "true"));
    return nodockwithclear;
}

VSRandom targrand(time(NULL));

Unit *getAtmospheric(Unit *targ) {
    if (targ) {
        Unit *un;
        for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
                (un = *i) != NULL;
                ++i) {
            if (un->isUnit() == Vega_UnitType::planet) {
                if ((targ->Position() - un->Position()).Magnitude() < targ->rSize() * .5) {
                    if (!(((Planet *) un)->isAtmospheric())) {
                        return un;
                    }
                }
            }
        }
    }
    return NULL;
}

bool RequestClearence(Unit *parent, Unit *targ, unsigned char sex) {
    if (!targ->DockingPortLocations().size()) {
        return false;
    }

    if (targ->isUnit() == Vega_UnitType::planet) {
        if (((Planet *) targ)->isAtmospheric() && NoDockWithClear()) {
            targ = getAtmospheric(targ);
            if (!targ) {
                return false;
            }
            parent->Target(targ);
        }
    }
    CommunicationMessage c(parent, targ, NULL, sex);
    c.SetCurrentState(c.fsm->GetRequestLandNode(), NULL, sex);
    Order *o = targ->getAIState();
    if (o) {
        o->Communicate(c);
    }
    return true;
}

using Orders::FireAt;

bool FireAt::PursueTarget(Unit *un, bool leader) {
    if (leader) {
        return true;
    }
    if (un == parent->Target()) {
        return rand() < .9 * RAND_MAX;
    }
    if (parent->getRelation(un) < 0) {
        return rand() < .2 * RAND_MAX;
    }
    return false;
}

bool CanFaceTarget(Unit *su, Unit *targ, const Matrix &matrix) {
    return true;

    float limitmin = su->limit_min;
    if (limitmin > -.99) {
        QVector pos = (targ->Position() - su->Position()).Normalize();
        QVector pnorm = pos.Cast();
        Vector structurelimits = su->structure_limits;
        Vector worldlimit = TransformNormal(matrix, structurelimits);
        if (pnorm.Dot(worldlimit) < limitmin) {
            return false;
        }
    }
    Unit *ssu;
    for (un_iter i = su->getSubUnits();
            (ssu = *i) != NULL;
            ++i) {
        if (!CanFaceTarget(ssu, targ, su->cumulative_transformation_matrix)) {
            return false;
        }
    }
    return true;
}

void FireAt::ReInit(float aggressivitylevel) {
    lastmissiletime = UniverseUtil::GetGameTime() - 65536.;
    missileprobability = vega_config::config->ai.firing.missile_probability;
    delay = 0;
    agg = aggressivitylevel;
    distance = 1;
    //JS --- spreading target switch times
    lastchangedtarg = 0.0 - targrand.uniformInc(0, 1) * vega_config::config->ai.targeting.min_time_to_switch_targets;
    had_target = false;
}

FireAt::FireAt(float aggressivitylevel) : CommunicatingAI(WEAPON, STARGET) {
    ReInit(aggressivitylevel);
}

FireAt::FireAt() : CommunicatingAI(WEAPON, STARGET) {
    ReInit(vega_config::config->ai.firing.aggressivity);
}

void FireAt::SignalChosenTarget() {
}

//temporary way of choosing
struct TargetAndRange {
    Unit *t;
    float range;
    float relation;

    TargetAndRange(Unit *tt, float r, float rel) {
        t = tt;
        range = r;
        this->relation = rel;
    }
};

struct RangeSortedTurrets {
    Unit *tur;
    float gunrange;

    RangeSortedTurrets(Unit *t, float r) {
        tur = t;
        gunrange = r;
    }

    bool operator<(const RangeSortedTurrets &o) const {
        return gunrange < o.gunrange;
    }
};

struct TurretBin {
    float maxrange;
    vector<RangeSortedTurrets> turret;
    vector<TargetAndRange> listOfTargets[2];     //we have the over (and eq) 16 crowd and the under 16
    TurretBin() {
        maxrange = 0;
    }

    bool operator<(const TurretBin &o) const {
        return maxrange > o.maxrange;
    }

    void AssignTargets(const TargetAndRange &finalChoice, const Matrix &pos) {
        //go through modularly assigning as you go;
        int count = 0;
        size_t lotsize[2] = {listOfTargets[0].size(), listOfTargets[1].size()};
        for (vector<RangeSortedTurrets>::iterator uniter = turret.begin(); uniter != turret.end(); ++uniter) {
            bool foundfinal = false;
            uniter->tur->Target(NULL);
            uniter->tur->TargetTurret(NULL);
            if (finalChoice.t) {
                if (finalChoice.range < uniter->gunrange
                        && ROLES::getPriority(uniter->tur->getAttackPreferenceChar())[finalChoice.t->getUnitRoleChar()]
                                < 31) {
                    if (CanFaceTarget(uniter->tur, finalChoice.t, pos)) {
                        uniter->tur->Target(finalChoice.t);
                        uniter->tur->TargetTurret(finalChoice.t);
                        foundfinal = true;
                    }
                }
            }
            if (!foundfinal) {
                for (unsigned int f = 0; f < 2 && !foundfinal; f++) {
                    for (size_t i = 0; i < lotsize[f]; i++) {
                        const size_t index = (count + i) % lotsize[f];
                        if (listOfTargets[f][index].range < uniter->gunrange) {
                            if (CanFaceTarget(uniter->tur, finalChoice.t, pos)) {
                                uniter->tur->Target(listOfTargets[f][index].t);
                                uniter->tur->TargetTurret(listOfTargets[f][index].t);
                                count++;
                                foundfinal = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
};

void AssignTBin(Unit *su, vector<TurretBin> &tbin) {
    unsigned int bnum = 0;
    for (; bnum < tbin.size(); bnum++) {
        if (su->getAttackPreferenceChar() == tbin[bnum].turret[0].tur->getAttackPreferenceChar()) {
            break;
        }
    }
    if (bnum >= tbin.size()) {
        tbin.push_back(TurretBin());
    }
    float gspeed, grange, mrange;
    grange = FLT_MAX;
    su->getAverageGunSpeed(gspeed, grange, mrange);
    {
        float ggspeed, ggrange, mmrange;
        Unit *ssu;
        for (un_iter i = su->getSubUnits(); (ssu = *i) != NULL; ++i) {
            ssu->getAverageGunSpeed(ggspeed, ggrange, mmrange);
            if (ggspeed > gspeed) {
                gspeed = ggspeed;
            }
            if (ggrange > grange) {
                grange = ggrange;
            }
            if (mmrange > mrange) {
                mrange = mmrange;
            }
        }
    }
    if (tbin[bnum].maxrange < grange) {
        tbin[bnum].maxrange = grange;
    }
    tbin[bnum].turret.push_back(RangeSortedTurrets(su, grange));
}

float Priority(Unit *me, Unit *targ, float gunrange, float rangetotarget, float relationship, char *rolepriority) {
    if (relationship >= 0) {
        return -1;
    }
    if (targ->Destroyed()) {
        return -1;
    }
    *rolepriority =
            ROLES::getPriority(me->getAttackPreferenceChar())[targ->getUnitRoleChar()];     //number btw 0 and 31 higher better
    char invrolepriority = 31 - *rolepriority;
    if (invrolepriority <= 0) {
        return -1;
    }
    if (rangetotarget < 1 && rangetotarget > -1000) {
        rangetotarget = 1;
    } else {
        rangetotarget = fabs(rangetotarget);
    }
    if (rangetotarget < .5 * gunrange) {
        rangetotarget = .5 * gunrange;
    }
    if (gunrange <= 0) {
        static float mountless_gunrange =
                XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MountlessGunRange", "300000000"));
        gunrange = mountless_gunrange;
    }     //probably a mountless capship. 50000 is chosen arbitrarily
    float inertial_priority = 0;
    {
        static float mass_inertial_priority_cutoff =
                XMLSupport::parse_float(vs_config->getVariable("AI",
                        "Targetting",
                        "MassInertialPriorityCutoff",
                        "5000"));
        if (me->getMass() > mass_inertial_priority_cutoff) {
            static float mass_inertial_priority_scale =
                    XMLSupport::parse_float(vs_config->getVariable("AI",
                            "Targetting",
                            "MassInertialPriorityScale",
                            ".0000001"));
            Vector normv(me->GetVelocity());
            float Speed = me->GetVelocity().Magnitude();
            normv *= Speed ? 1.0f / Speed : 1.0f;
            Vector ourToThem = targ->Position() - me->Position();
            ourToThem.Normalize();
            inertial_priority =
                    mass_inertial_priority_scale * (.5 + .5 * (normv.Dot(ourToThem))) * me->getMass() * Speed;
        }
    }
    static float
            threat_weight = XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "ThreatWeight", ".5"));
    float threat_priority = (me->Threat() == targ) ? threat_weight : 0;
    threat_priority += (targ->Target() == me) ? threat_weight : 0;
    float role_priority01 = ((float) *rolepriority) / 31.;
    float range_priority01 = .5 * gunrange / rangetotarget;     //number between 0 and 1 for most ships 1 is best
    return range_priority01 * role_priority01 + inertial_priority + threat_priority;
}

float Priority(Unit *me, Unit *targ, float gunrange, float rangetotarget, float relationship) {
    char rolepriority = 0;
    return Priority(me, targ, gunrange, rangetotarget, relationship, &rolepriority);
}

template<class T, size_t n>
class StaticTuple {
public:
    T vec[n];

    size_t size() {
        return n;
    }

    T &operator[](size_t index) {
        return vec[index];
    }

    const T &operator[](size_t index) const {
        return vec[index];
    }
};

template<size_t numTuple>
class ChooseTargetClass {
    Unit *parent{};
    Unit *parentparent{};
    vector<TurretBin> *tbin{};
    StaticTuple<float, numTuple> maxinnerrangeless;
    StaticTuple<float, numTuple> maxinnerrangemore;
    float priority{};
    char rolepriority{};
    char maxrolepriority{};
    bool reachedMore{};
    bool reachedLess{};
    FireAt *fireat{};
    float gunrange{};
    int numtargets{};
    int maxtargets{};
public:
    Unit *mytarg{};

    ChooseTargetClass() {
    }

    void init(FireAt *fireat,
            Unit *un,
            float gunrange,
            vector<TurretBin> *tbin,
            const StaticTuple<float, numTuple> &innermaxrange,
            char maxrolepriority,
            int maxtargets) {
        this->fireat = fireat;
        this->tbin = tbin;
        this->parent = un;
        this->parentparent = un->owner ? UniverseUtil::getUnitByPtr(un->owner, un, false) : 0;
        mytarg = NULL;
        double currad = 0;
        if (!is_null(un->location[Unit::UNIT_ONLY])) {
            currad = un->location[Unit::UNIT_ONLY]->getKey();
        }
        for (size_t i = 0; i < numTuple; ++i) {
            double tmpless = currad - innermaxrange[i];
            double tmpmore = currad + innermaxrange[i];
            this->maxinnerrangeless[i] = tmpless;
            this->maxinnerrangemore[i] = tmpmore;
        }
        this->maxrolepriority = maxrolepriority; //max priority that will allow gun range to be ok
        reachedMore = false;
        reachedLess = false;
        this->priority = -1;
        this->rolepriority = 31;
        this->gunrange = gunrange;
        this->numtargets = 0;
        this->maxtargets = maxtargets;
    }

    bool acquire(Unit *un, float distance) {
        double unkey = un->location[Unit::UNIT_ONLY]->getKey();
        bool lesscheck = unkey < maxinnerrangeless[0];
        bool morecheck = unkey > maxinnerrangemore[0];
        if (reachedMore == false || reachedLess == false) {
            if (lesscheck || morecheck) {
                if (lesscheck) {
                    reachedLess = true;
                }
                if (morecheck) {
                    reachedMore = true;
                }
                if (mytarg && rolepriority < maxrolepriority) {
                    return false;
                } else if (reachedLess == true && reachedMore == true) {
                    for (size_t i = 1; i < numTuple; ++i) {
                        if (unkey > maxinnerrangeless[i] && unkey < maxinnerrangemore[i]) {
                            maxinnerrangeless[0] = maxinnerrangeless[i];
                            maxinnerrangemore[0] = maxinnerrangemore[i];
                            reachedLess = false;
                            reachedMore = false;
                        }
                    }
                }
            }
        }
        return ShouldTargetUnit(un, distance);
    }

    bool ShouldTargetUnit(Unit *un, float distance) {
        if (un->cloak.Visible()) {
            float rangetotarget = distance;
            float rel0 = parent->getRelation(un);
            float rel[] = {
                    rel0         //not initialized until after array
                    , (parentparent ? parentparent->getRelation(un) : rel0)           //not initialized till after array
            };
            float relationship = rel0;
            for (unsigned int i = 1; i < sizeof(rel) / sizeof(*rel); i++) {
                if (rel[i] < relationship) {
                    relationship = rel[i];
                }
            }
            char rp = 31;
            float tmp = Priority(parent, un, gunrange, rangetotarget, relationship, &rp);
            if (tmp > priority) {
                mytarg = un;
                priority = tmp;
                rolepriority = rp;
            }
            for (vector<TurretBin>::iterator k = tbin->begin(); k != tbin->end(); ++k) {
                if (rangetotarget > k->maxrange) {
                    break;
                }
                const char
                        tprior = ROLES::getPriority(k->turret[0].tur->getAttackPreferenceChar())[un->getUnitRoleChar()];
                if (relationship < 0) {
                    if (tprior < 16) {
                        k->listOfTargets[0].push_back(TargetAndRange(un, rangetotarget, relationship));
                        numtargets++;
                    } else if (tprior < 31) {
                        k->listOfTargets[1].push_back(TargetAndRange(un, rangetotarget, relationship));
                        numtargets++;
                    }
                }
            }
        }
        return (maxtargets == 0) || (numtargets < maxtargets);
    }
};

int numpolled[2] = {0, 0};   //number of units that searched for a target

int prevpollindex[2] = {10000, 10000};   //previous number of units touched (doesn't need to be precise)

int pollindex[2] = {1,
        1};   //current count of number of units touched (doesn't need to be precise)  -- used for "fairness" heuristic

void FireAt::ChooseTargets(int numtargs, bool force) {
    float gunspeed, gunrange, missilerange;
    parent->getAverageGunSpeed(gunspeed, gunrange, missilerange);
    static float targettimer = UniverseUtil::GetGameTime();    //timer used to determine passage of physics frames
    static float mintimetoswitch =
            XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinTimeToSwitchTargets", "3"));
    static float minnulltimetoswitch =
            XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "MinNullTimeToSwitchTargets", "5"));
    static int minnumpollers =
            float_to_int(XMLSupport::parse_float(vs_config->getVariable("AI",
                    "Targetting",
                    "MinNumberofpollersperframe",
                    "5")));                    //maximum number of vessels allowed to search for a target in a given physics frame
    static int maxnumpollers =
            float_to_int(XMLSupport::parse_float(vs_config->getVariable("AI",
                    "Targetting",
                    "MaxNumberofpollersperframe",
                    "49")));                    //maximum number of vessels allowed to search for a target in a given physics frame
    static int numpollers[2] = {maxnumpollers, maxnumpollers};

    static int nextframenumpollers[2] = {maxnumpollers, maxnumpollers};
    if (lastchangedtarg + mintimetoswitch > 0) {
        return;
    }          //don't switch if switching too soon

    Unit *curtarg = parent->Target();
    int hastarg = (curtarg == NULL) ? 0 : 1;
    //Following code exists to limit the number of craft polling for a target in a given frame - this is an expensive operation, and needs to be spread out, or there will be pauses.
    if ((UniverseUtil::GetGameTime()) - targettimer >= SIMULATION_ATOM * .99) {
        //Check if one or more physics frames have passed
        numpolled[0] = numpolled[1] = 0;         //reset counters
        prevpollindex[0] = pollindex[0];
        prevpollindex[1] = pollindex[1];
        pollindex[hastarg] = 0;
        targettimer = UniverseUtil::GetGameTime();
        numpollers[0] = float_to_int(nextframenumpollers[0]);
        numpollers[1] = float_to_int(nextframenumpollers[1]);
    }
    pollindex[hastarg]++;     //count number of craft touched - will use in the next physics frame to spread out the vessels actually chosen to be processed among all of the vessels being touched
    if (numpolled[hastarg] > numpollers[hastarg]) {      //over quota, wait until next physics frame
        return;
    }
    if (!(pollindex[hastarg] % ((prevpollindex[hastarg] / numpollers[hastarg])
            + 1))) {      //spread out, in modulo fashion, the possibility of changing one's target. Use previous physics frame count of craft to estimate current number of craft
        numpolled[hastarg]++;          //if a more likely candidate, we're going to search for a target.
    } else {
        return;
    }          //skipped to achieve better fairness - see comment on modulo distribution above
    if (curtarg) {
        if (isJumpablePlanet(curtarg)) {
            return;
        }
    }
    bool wasnull = (curtarg == NULL);
    Flightgroup *fg = parent->getFlightgroup();
    lastchangedtarg = 0 + targrand.uniformInc(0, 1)
            * mintimetoswitch;     //spread out next valid time to switch targets - helps to ease per-frame loads.
    if (fg) {
        if (!fg->directive.empty()) {
            if (curtarg != NULL && (*fg->directive.begin()) == toupper(*fg->directive.begin())) {
                return;
            }
        }
    }
    //not   allowed to switch targets
    numprocessed++;
    vector<TurretBin> tbin;
    Unit *su = NULL;
    un_iter subun = parent->getSubUnits();
    for (; (su = *subun) != NULL; ++subun) {
        static unsigned int inert = ROLES::getRole("INERT");
        static unsigned int pointdef = ROLES::getRole("POINTDEF");
        static bool assignpointdef =
                XMLSupport::parse_bool(vs_config->getVariable("AI", "Targetting", "AssignPointDef", "true"));
        if ((su->getAttackPreferenceChar() != pointdef) || assignpointdef) {
            if (su->getAttackPreferenceChar() != inert) {
                AssignTBin(su, tbin);
            } else {
                Unit *ssu = NULL;
                for (un_iter subturret = su->getSubUnits(); (ssu = (*subturret)); ++subturret) {
                    AssignTBin(ssu, tbin);
                }
            }
        }
    }
    std::sort(tbin.begin(), tbin.end());
    float efrel = 0;
    float mytargrange = FLT_MAX;
    static float unitRad =
            XMLSupport::parse_float(vs_config->getVariable("AI",
                    "Targetting",
                    "search_extra_radius",
                    "1000"));                 //Maximum target radius that is guaranteed to be detected
    static char maxrolepriority =
            XMLSupport::parse_int(vs_config->getVariable("AI", "Targetting", "search_max_role_priority", "16"));
    static int maxtargets = XMLSupport::parse_int(vs_config->getVariable("AI",
            "Targetting",
            "search_max_candidates",
            "64"));   //Cutoff candidate count (if that many hostiles found, stop search - performance/quality tradeoff, 0=no cutoff)
    UnitWithinRangeLocator<ChooseTargetClass<2> > unitLocator(parent->radar.GetMaxRange(), unitRad);
    StaticTuple<float, 2> maxranges{};

    maxranges[0] = gunrange;
    maxranges[1] = missilerange;
    if (tbin.size()) {
        maxranges[0] = (tbin[0].maxrange > gunrange ? tbin[0].maxrange : gunrange);
    }
    double pretable = queryTime();
    unitLocator.action.init(this, parent, gunrange, &tbin, maxranges, maxrolepriority, maxtargets);
    static int gcounter = 0;
    static int
            min_rechoose_interval = XMLSupport::parse_int(vs_config->getVariable("AI", "min_rechoose_interval", "128"));
    if (curtarg) {
        if (gcounter++ < min_rechoose_interval || rand() / 8 < RAND_MAX / 9) {
            //in this case only look at potentially *interesting* units rather than huge swaths of nearby units...including target, threat, players, and leader's target
            unitLocator.action.ShouldTargetUnit(curtarg, UnitUtil::getDistance(parent, curtarg));
            unsigned int np = _Universe->numPlayers();
            for (unsigned int i = 0; i < np; ++i) {
                Unit *playa = _Universe->AccessCockpit(i)->GetParent();
                if (playa) {
                    unitLocator.action.ShouldTargetUnit(playa, UnitUtil::getDistance(parent, playa));
                }
            }
            Unit *lead = UnitUtil::getFlightgroupLeader(parent);
            if (lead != NULL && lead != parent && (lead = lead->Target()) != NULL) {
                unitLocator.action.ShouldTargetUnit(lead, UnitUtil::getDistance(parent, lead));
            }
            Unit *threat = parent->Threat();
            if (threat) {
                unitLocator.action.ShouldTargetUnit(threat, UnitUtil::getDistance(parent, threat));
            }
        } else {
            gcounter = 0;
        }
    }
    if (unitLocator.action.mytarg == NULL) {      //decided to rechoose or did not have initial target
        findObjects(
                _Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY],
                parent->location[Unit::UNIT_ONLY],
                &unitLocator);
    }
    Unit *mytarg = unitLocator.action.mytarg;
    targetpick += queryTime() - pretable;
    if (mytarg) {
        efrel = parent->getRelation(mytarg);
        mytargrange = UnitUtil::getDistance(parent, mytarg);
    }
    TargetAndRange my_target(mytarg, mytargrange, efrel);
    for (vector<TurretBin>::iterator k = tbin.begin(); k != tbin.end(); ++k) {
        k->AssignTargets(my_target, parent->cumulative_transformation_matrix);
    }
    parent->radar.Unlock();
    if (wasnull) {
        if (mytarg) {
            nextframenumpollers[hastarg] += 2;
            if (nextframenumpollers[hastarg] > maxnumpollers) {
                nextframenumpollers[hastarg] = maxnumpollers;
            }
        } else {
            lastchangedtarg += targrand.uniformInc(0, 1) * minnulltimetoswitch;
            nextframenumpollers[hastarg] -= .05;
            if (nextframenumpollers[hastarg] < minnumpollers) {
                nextframenumpollers[hastarg] = minnumpollers;
            }
        }
    } else {
        if (parent->Target() != mytarg) {
            nextframenumpollers[hastarg] += 2;
            if (nextframenumpollers[hastarg] > maxnumpollers) {
                nextframenumpollers[hastarg] = maxnumpollers;
            }
        } else {
            nextframenumpollers[hastarg] -= .01;
            if (nextframenumpollers[hastarg] < minnumpollers) {
                nextframenumpollers[hastarg] = minnumpollers;
            }
        }
    }
    parent->Target(mytarg);
    parent->radar.Lock(UnitUtil::isSignificant(parent));
    SignalChosenTarget();
}

bool FireAt::ShouldFire(Unit *targ, bool &missilelock) {
    float dist;
    if (!targ) {
        return false;

        static int test = 0;
        if (test++ % 1000 == 1) {
            VS_LOG(warning, "lost target");
        }
    }
    float gunspeed, gunrange, missilerange;
    parent->getAverageGunSpeed(gunspeed, gunrange, missilerange);
    float angle = parent->cosAngleTo(targ, dist, parent->GetComputerData().itts ? gunspeed : FLT_MAX, gunrange, false);
    missilelock = false;
    targ->Threaten(parent, angle / (dist < .8 ? .8 : dist));
    if (targ == parent->Target()) {
        distance = dist;
    }
    static float firewhen = XMLSupport::parse_float(vs_config->getVariable("AI", "Firing", "InWeaponRange", "1.2"));
    static float fireangle_minagg =
            (float) cos(M_PI * XMLSupport::parse_float(vs_config->getVariable("AI",
                    "Firing",
                    "MaximumFiringAngle.minagg",
                    "10"))
                    / 180.);                                                                      //Roughly 10 degrees
    static float fireangle_maxagg =
            (float) cos(M_PI * XMLSupport::parse_float(vs_config->getVariable("AI",
                    "Firing",
                    "MaximumFiringAngle.maxagg",
                    "18"))
                    / 180.);                                                                      //Roughly 18 degrees
    float temp = parent->TrackingGuns(missilelock);
    bool isjumppoint = targ->isUnit() == Vega_UnitType::planet && ((Planet *) targ)->GetDestinations().empty() == false;
    float fangle = (fireangle_minagg + fireangle_maxagg * agg) / (1.0f + agg);
    bool retval =
            ((dist < firewhen)
                    && ((angle > fangle) || (temp && (angle > temp)) || (missilelock && (angle > 0)))) && !isjumppoint;
    if (retval) {
        if (Cockpit::tooManyAttackers()) {
            Cockpit *player = _Universe->isPlayerStarship(targ);
            if (player) {
                static int max_attackers =
                        XMLSupport::parse_int(vs_config->getVariable("AI", "max_player_attackers", "0"));
                int attackers = player->number_of_attackers;
                if (attackers > max_attackers && max_attackers > 0) {
                    static float attacker_switch_time =
                            XMLSupport::parse_float(vs_config->getVariable("AI", "attacker_switch_time", "15"));
                    int curtime =
                            (int) fmod(floor(UniverseUtil::GetGameTime() / attacker_switch_time), (float) (1 << 24));
                    int seed = ((((size_t) parent) & 0xffffffff) ^ curtime);
                    static VSRandom decide(seed);
                    decide.init_genrand(seed);
                    if (decide.genrand_int31() % attackers >= max_attackers) {
                        return false;
                    }
                }
            }
        }
    }
    return retval;
}

FireAt::~FireAt() {
#ifdef ORDERDEBUG
    VS_LOG_AND_FLUSH(trace, (boost::format("fire%1$x") % this));
#endif
}

unsigned int FireBitmask(Unit *parent, bool shouldfire, bool firemissile) {
    unsigned int firebitm = ROLES::EVERYTHING_ELSE;
    Unit *un = parent->Target();
    if (un) {
        firebitm = (1 << un->getUnitRoleChar());

        static bool AlwaysFireAutotrackers =
                XMLSupport::parse_bool(vs_config->getVariable("AI", "AlwaysFireAutotrackers", "true"));
        if (shouldfire) {
            firebitm |= ROLES::FIRE_GUNS;
        }
        if (AlwaysFireAutotrackers && !shouldfire) {
            firebitm |= ROLES::FIRE_GUNS;
            firebitm |= ROLES::FIRE_ONLY_AUTOTRACKERS;
        }
        if (firemissile) {
            firebitm = ROLES::FIRE_MISSILES;
        }              //stops guns
    }
    return firebitm;
}

void FireAt::FireWeapons(bool shouldfire, bool lockmissile) {
    static float missiledelay = XMLSupport::parse_float(vs_config->getVariable("AI", "MissileGunDelay", "4"));
    //Will rand() be in the expected range here? -- stephengtuggy 2020-07-25
    bool fire_missile = lockmissile && rand() < RAND_MAX * missileprobability * SIMULATION_ATOM;
    delay += SIMULATION_ATOM; //simulation_atom_var?
    if (shouldfire && delay < parent->pilot->getReactionTime()) {
        return;
    } else if (!shouldfire) {
        delay = 0;
    }
    if (fire_missile) {
        lastmissiletime = UniverseUtil::GetGameTime();
    } else if (UniverseUtil::GetGameTime() - lastmissiletime < missiledelay && !fire_missile) {
        return;
    }
    parent->Fire(FireBitmask(parent, shouldfire, fire_missile), true);
}

bool FireAt::isJumpablePlanet(Unit *targ) {
    bool istargetjumpableplanet = targ->isUnit() == Vega_UnitType::planet;
    if (istargetjumpableplanet) {
        istargetjumpableplanet =
                (!((Planet *) targ)->GetDestinations().empty()) && (parent->jump_drive.IsDestinationSet());
    }
    return istargetjumpableplanet;
}

using std::string;

void FireAt::PossiblySwitchTarget(bool unused) {
    static float
            targettime = XMLSupport::parse_float(vs_config->getVariable("AI", "Targetting", "TimeUntilSwitch", "20"));
    if ((targettime <= 0) || (vsrandom.uniformInc(0, 1) < simulation_atom_var / targettime)) {
        bool ct = true;
        Flightgroup *fg;
        if ((fg = parent->getFlightgroup())) {
            if (fg->directive.find(".") != string::npos) {
                ct = (parent->Target() == NULL);
            }
        }
        if (ct) {
            ChooseTarget();
        }
    }
}

void FireAt::Execute() {
    lastchangedtarg -= SIMULATION_ATOM;
    bool missilelock = false;
    bool tmp = done;
    Order::Execute();
    done = tmp;
    Unit *targ;
    if (parent->isUnit() == Vega_UnitType::unit) {
        static float
                cont_update_time = XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandUpdateTime", "1"));
        //Will rand() be in the expected range here? -- stephengtuggy 2020-07-25
        if (rand() < RAND_MAX * SIMULATION_ATOM / cont_update_time) {
            UpdateContrabandSearch();
        }
        static float
                cont_initiate_time = XMLSupport::parse_float(vs_config->getVariable("AI", "CommInitiateTime", "300"));
        //Or here?
        if ((float) rand() < ((float) RAND_MAX * (SIMULATION_ATOM / cont_initiate_time))) {
            static float contraband_initiate_time =
                    XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandInitiateTime", "3000"));
            static float comm_to_player =
                    XMLSupport::parse_float(vs_config->getVariable("AI", "CommToPlayerPercent", ".05"));
            static float comm_to_target =
                    XMLSupport::parse_float(vs_config->getVariable("AI", "CommToTargetPercent", ".25"));
            static float contraband_to_player =
                    XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandToPlayerPercent", ".98"));
            static float contraband_to_target =
                    XMLSupport::parse_float(vs_config->getVariable("AI", "ContrabandToTargetPercent", "0.001"));

            unsigned int modulo = ((unsigned int) (contraband_initiate_time / cont_initiate_time));
            if (modulo < 1) {
                modulo = 1;
            }
            if (rand() % modulo) {
                RandomInitiateCommunication(comm_to_player, comm_to_target);
            } else {
                InitiateContrabandSearch(contraband_to_player, contraband_to_target);
            }
        }
    }
    bool shouldfire = false;
    bool istargetjumpableplanet = false;
    if ((targ = parent->Target())) {
        istargetjumpableplanet = isJumpablePlanet(targ);
        if (targ->cloak.Visible() && !targ->Destroyed()) {
            had_target = true;
            if (parent->getNumMounts() > 0) {
                if (!istargetjumpableplanet) {
                    shouldfire |= ShouldFire(targ, missilelock);
                }
            }
        } else {
            if (had_target) {
                had_target = false;
                lastchangedtarg = -100000;
            }
            ChooseTarget();
        }
    } else if (had_target) {
        had_target = false;
        lastchangedtarg = -100000;
    }
    PossiblySwitchTarget(istargetjumpableplanet);
    if ((!istargetjumpableplanet) && parent->getNumMounts() > 0) {
        FireWeapons(shouldfire, missilelock);
    }
}

