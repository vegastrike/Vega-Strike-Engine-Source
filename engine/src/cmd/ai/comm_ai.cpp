/*
 * comm_ai.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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


#include "comm_ai.h"
#include "faction_generic.h"
#include "communication.h"
#include "cmd/collection.h"
#include "gfx/cockpit_generic.h"
#include "cmd/images.h"
#include "configxml.h"
#include "vs_globals.h"
#include "cmd/script/flightgroup.h"
#include "cmd/unit_util.h"
#include "vs_random.h"
#include "cmd/unit_find.h"
#include "cmd/pilot.h"
#include "universe_util.h"
#include "universe.h"

CommunicatingAI::CommunicatingAI(int ttype,
        int stype,
        float mood,
        float anger,
        float appeas,
        float moodswingyness,
        float randomresp) : Order(ttype, stype),
        anger(anger),
        appease(appeas),
        moodswingyness(moodswingyness),
        randomresponse(randomresp),
        mood(mood) {
    if (appease > 665 && appease < 667) {
        this->appease = configuration()->ai.ease_to_appease;
    }
    if ((anger > 665 && anger < 667) || (anger > -667 && anger < -665)) {
        this->anger = configuration()->ai.ease_to_anger;
    }
    if (moodswingyness > 665 && moodswingyness < 667) {
        this->moodswingyness = configuration()->ai.mood_swing_level;
    }
    if (randomresp > 665 && moodswingyness < 667) {
        this->randomresponse = configuration()->ai.random_response_range;
    }
}

bool MatchingMood(const CommunicationMessage &c, float mood, float randomresponse, float relationship) {
    const FSM::Node *n = (unsigned int) c.curstate
            < c.fsm->nodes.size() ? (&c.fsm->nodes[c.curstate]) : (&c.fsm->nodes[c.fsm
            ->getDefaultState(
                    relationship)]);
    auto iend = n->edges.cend();
    for (auto i = n->edges.cbegin(); i != iend; ++i) {
        if (c.fsm->nodes[*i].messagedelta >= configuration()->ai.lowest_positive_comm_choice && relationship >= 0) {
            return true;
        }
        if (c.fsm->nodes[*i].messagedelta <= configuration()->ai.lowest_negative_comm_choice && relationship < 0) {
            return true;
        }
    }
    return false;
}

int CommunicatingAI::selectCommunicationMessageMood(CommunicationMessage &c, float mood) {
    Unit *targ = c.sender.GetUnit();
    float relationship = 0;
    if (targ) {
        relationship = parent->pilot->GetEffectiveRelationship(parent, targ);
        if (targ == parent->Target() && relationship > -1.0) {
            relationship = -1.0;
        }
        mood += (1 - randomresponse) * relationship;
    }
    //breaks stuff
    if ((c.curstate >= c.fsm->GetUnDockNode()) || !MatchingMood(c, mood, randomresponse, relationship)) {
        c.curstate = c.fsm->getDefaultState(relationship);
    }          //hijack the current state
    return c.fsm->getCommMessageMood(c.curstate, mood, randomresponse, relationship);
}

using std::pair;

void GetMadAt(Unit *un, Unit *parent, int numhits = 0) {
    if (numhits == 0) {
        static int snumhits = XMLSupport::parse_int(vs_config->getVariable("AI", "ContrabandMadness", "5"));
        numhits = snumhits;
    }
    CommunicationMessage hit(un, parent, nullptr, 0);
    hit.SetCurrentState(hit.fsm->GetHitNode(), nullptr, 0);
    for (int i = 0; i < numhits; i++) {
        parent->getAIState()->Communicate(hit);
    }
}

void AllUnitsCloseAndEngage(Unit *un, int faction) {
    Unit *ally;
    static float contraband_assist_range =
            XMLSupport::parse_floatf(vs_config->getVariable("physics", "contraband_assist_range", "50000"));
    float relation = 0;
    static float
            minrel = XMLSupport::parse_floatf(vs_config->getVariable("AI", "max_faction_contraband_relation", "-.05"));
    static float
            adj = XMLSupport::parse_floatf(vs_config->getVariable("AI", "faction_contraband_relation_adjust", "-.025"));
    float delta;
    int cp = _Universe->whichPlayerStarship(un);
    if (cp != -1) {
        if ((relation = UnitUtil::getRelationFromFaction(un, faction)) > minrel) {
            delta = minrel - relation;
        } else {
            delta = adj;
        }
        UniverseUtil::adjustRelationModifierInt(cp, faction, delta);
    }
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
            (ally = *i) != NULL;
            ++i) {
        //Vector loc;
        if (ally->faction == faction) {
            if ((ally->Position() - un->Position()).Magnitude() < contraband_assist_range) {
                GetMadAt(un, ally);
                Flightgroup *fg = ally->getFlightgroup();
                if (fg) {
                    if (fg->directive.empty() ? true : toupper(*fg->directive.begin()) != *fg->directive.begin()) {
                        ally->Target(un);
                        ally->TargetTurret(un);
                    } else {
                        ally->Target(un);
                        ally->TargetTurret(un);
                    }
                }
            }
        }
    }
    //}
}

void CommunicatingAI::TerminateContrabandSearch(bool contraband_detected) {
    //reports success or failure
    Unit *un;
    unsigned char gender;
    vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face = parent->pilot->getCommFaces(gender);
    if ((un = contraband_searchee.GetUnit())) {
        CommunicationMessage c(parent, un, comm_face, gender);
        if (contraband_detected) {
            c.SetCurrentState(c.fsm->GetContrabandDetectedNode(), comm_face, gender);
            GetMadAt(un, 0);
            AllUnitsCloseAndEngage(un, parent->faction);
        } else {
            c.SetCurrentState(c.fsm->GetContrabandUnDetectedNode(), comm_face, gender);
        }
        Order *o = un->getAIState();
        if (o) {
            o->Communicate(c);
        }
    }
    contraband_searchee.SetUnit(nullptr);
}

void CommunicatingAI::GetMadAt(Unit *un, int numHitsPerContrabandFail) {
    ::GetMadAt(un, parent, numHitsPerContrabandFail);
}

static int InList(std::string item, Unit *un) {
    float numcontr = 0;
    if (un) {
        for (unsigned int i = 0; i < un->numCargo(); i++) {
            if (un->GetCargo(i).GetName() == item) {
                if (un->GetCargo(i).GetQuantity() > 0) {
                    numcontr++;
                }
            }
        }
    }
    return float_to_int(numcontr);
}

void CommunicatingAI::UpdateContrabandSearch() {
    static unsigned int contraband_search_batch_update =
            XMLSupport::parse_int(vs_config->getVariable("AI", "num_contraband_scans_per_search", "10"));
    for (unsigned int rep = 0; rep < contraband_search_batch_update; ++rep) {
        Unit *u = contraband_searchee.GetUnit();
        if (u && (u->faction != parent->faction)) {
            //don't scan your buddies
            if (which_cargo_item < (int) u->numCargo()) {
                if (u->GetCargo(which_cargo_item).GetQuantity() > 0) {
                    int which_carg_item_bak = which_cargo_item;
                    std::string item = u->GetManifest(which_cargo_item++, parent, SpeedAndCourse);
                    static bool use_hidden_cargo_space =
                            XMLSupport::parse_bool(vs_config->getVariable("physics", "use_hidden_cargo_space", "true"));
                    static float speed_course_change =
                            XMLSupport::parse_floatf(vs_config->getVariable("AI",
                                    "PercentageSpeedChangeToStopSearch",
                                    "1"));
                    if (u->CourseDeviation(SpeedAndCourse, u->GetVelocity()) > speed_course_change) {
                        unsigned char gender;
                        vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face = parent->pilot->getCommFaces(gender);
                        CommunicationMessage c(parent, u, comm_face, gender);
                        c.SetCurrentState(c.fsm->GetContrabandWobblyNode(), comm_face, gender);
                        Order *o;
                        if ((o = u->getAIState())) {
                            o->Communicate(c);
                        }
                        GetMadAt(u, 1);
                        SpeedAndCourse = u->GetVelocity();
                    }
                    float HiddenTotal = use_hidden_cargo_space ? (u->getHiddenCargoVolume()) : (0);
                    Unit *contrabandlist = FactionUtil::GetContraband(parent->faction);
                    if (InList(item, contrabandlist) > 0) {
                        //inlist now returns an integer so that we can do this at all...
                        if (HiddenTotal == 0 || u->GetCargo(which_carg_item_bak).GetQuantity() > HiddenTotal) {
                            TerminateContrabandSearch(true);                             //BUCO this is where we want to check against free hidden cargo space.
                        } else {
                            unsigned int max = u->numCargo();
                            unsigned int quantity = 0;
                            for (unsigned int i = 0; i < max; ++i) {
                                if (InList(u->GetCargo(i).GetName(), contrabandlist) > 0) {
                                    quantity += u->GetCargo(i).GetQuantity();
                                    if (quantity > HiddenTotal) {
                                        TerminateContrabandSearch(true);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    TerminateContrabandSearch(false);
                }
            }
        }
    }
}

static bool isDockedAtAll(Unit *un) {
    return (un->docked & (Unit::DOCKED_INSIDE | Unit::DOCKED)) != 0;
}

void CommunicatingAI::Destroy() {
    for (unsigned int i = 0; i < _Universe->numPlayers(); ++i) {
        Unit *target = _Universe->AccessCockpit(i)->GetParent();
        if (target) {
            FSM *fsm = FactionUtil::GetConversation(this->parent->faction, target->faction);
            if (fsm->StopAllSounds((unsigned char) (parent->pilot->getGender()))) {
                _Universe->AccessCockpit(i)->SetStaticAnimation();
                _Universe->AccessCockpit(i)->SetStaticAnimation();
            }
        }
    }
    this->Order::Destroy();
}

void CommunicatingAI::InitiateContrabandSearch(float playaprob, float targprob) {
    Unit *u = GetRandomUnit(playaprob, targprob);
    if (u) {
        Unit *un = FactionUtil::GetContraband(parent->faction);
        if (un) {
            if (un->numCargo() > 0 && UnitUtil::getUnitSystemFile(u) == UnitUtil::getUnitSystemFile(parent)
                    && !UnitUtil::isDockableUnit(parent)) {
                Unit *v;
                if ((v = contraband_searchee.GetUnit())) {
                    if (v == u) {
                        return;
                    }
                    TerminateContrabandSearch(false);
                }
                contraband_searchee.SetUnit(u);
                SpeedAndCourse = u->GetVelocity();
                unsigned char gender;
                vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face = parent->pilot->getCommFaces(gender);
                CommunicationMessage c(parent, u, comm_face, gender);
                c.SetCurrentState(c.fsm->GetContrabandInitiateNode(), comm_face, gender);
                if (u->getAIState()) {
                    u->getAIState()->Communicate(c);
                }
                which_cargo_item = 0;
            }
        }
    }
}

void CommunicatingAI::AdjustRelationTo(Unit *un, float factor) {
    Order::AdjustRelationTo(un, factor);
    float newrel = parent->pilot->adjustSpecificRelationship(parent, un, factor, un->faction);

    {
        int whichCp = _Universe->whichPlayerStarship(parent);
        Flightgroup *toFg;
        int toFaction;
        float oRlyFactor = factor;
        if (whichCp != -1) {
            toFg = un->getFlightgroup();
            toFaction = un->faction;
        } else {
            /* Instead use the Aggressor's cockpit? */
            whichCp = _Universe->whichPlayerStarship(un);
            toFg = parent->getFlightgroup();
            toFaction = parent->faction;
        }
        if (whichCp != -1) {
            if (toFg && un->faction != parent->faction) {
                oRlyFactor = factor * 0.5;
            }
            if (toFg) {
                UniverseUtil::adjustFGRelationModifier(whichCp, toFg->name, oRlyFactor * parent->pilot->getRank());
            }
            if (un->faction != parent->faction) {
                UniverseUtil::adjustRelationModifierInt(whichCp, toFaction, oRlyFactor * parent->pilot->getRank());
            }
        }
    }
    if (newrel < anger || (parent->Target() == NULL && newrel + UnitUtil::getFactionRelation(parent, un) < 0)) {
        if (parent->Target() == NULL
                || (parent->getFlightgroup() == NULL
                        || parent->getFlightgroup()->directive.find(".") == string::npos)) {
            parent->Target(un);             //he'll target you--even if he's friendly
            parent->TargetTurret(un);             //he'll target you--even if he's friendly
        } else if (newrel > appease) {
            if (parent->Target() == un) {
                if (parent->getFlightgroup() == NULL || parent->getFlightgroup()->directive.find(".") == string::npos) {
                    parent->Target(NULL);
                    parent->TargetTurret(NULL);                     //he'll target you--even if he's friendly
                }
            }
        }
    }
    mood += factor * moodswingyness;
}

//modified not to check player when hostiles are around--unless player IS the hostile
Unit *CommunicatingAI::GetRandomUnit(float playaprob, float targprob) {
    if (vsrandom.uniformInc(0, 1) < playaprob) {
        Unit *playa = _Universe->AccessCockpit(rand() % _Universe->numPlayers())->GetParent();
        if (playa) {
            if ((playa->Position() - parent->Position()).Magnitude() - parent->rSize() - playa->rSize()) {
                return playa;
            }
        }
    }
    if (vsrandom.uniformInc(0, 1) < targprob && parent->Target()) {
        return parent->Target();
    }
    //FIXME FOR TESTING ONLY
    //return parent->Target();
    QVector where = parent->Position() + parent->GetComputerData().radar.maxrange * QVector(vsrandom.uniformInc(-1, 1),
            vsrandom.uniformInc(-1, 1),
            vsrandom.uniformInc(-1, 1));
    Collidable wherewrapper(0, 0, where);

    NearestUnitLocator unitLocator;
#ifdef VS_ENABLE_COLLIDE_KEY
    CollideMap  *cm = _Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY];
    const float unitRad = configuration()->graphics_config.hud.radar_search_extra_radius;
    CollideMap::iterator iter = cm->lower_bound( wherewrapper );
    if (iter != cm->end() && (*iter)->radius > 0)
        if ( (*iter)->ref.unit != parent )
            return (*iter)->ref.unit;
    findObjects( _Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY], iter, &unitLocator );

    Unit *target = unitLocator.retval.unit;
    if (target == NULL || target == parent)
        target = parent->Target();
#else
    Unit *target = parent->Target();
#endif
    return target;
}

void CommunicatingAI::RandomInitiateCommunication(float playaprob, float targprob) {
    Unit *target = GetRandomUnit(playaprob, targprob);
    if (target != NULL) {
        if (UnitUtil::getUnitSystemFile(target) == UnitUtil::getUnitSystemFile(parent)
                && UnitUtil::getFlightgroupName(parent) != "Base" && !isDockedAtAll(target)
                && UnitUtil::getDistance(parent, target) <= target->GetComputerData().radar.maxrange) {
            //warning--odd hack they can talk to you if you can sense them--it's like SETI@home
            for (std::list<CommunicationMessage *>::iterator i = messagequeue.begin(); i != messagequeue.end(); i++) {
                Unit *un = (*i)->sender.GetUnit();
                if (un == target) {
                    return;
                }
            }
            //ok we're good to put a default msg in the queue as a fake message;
            FSM *fsm = FactionUtil::GetConversation(target->faction, this->parent->faction);
            int state = fsm->getDefaultState(parent->getRelation(target));
            unsigned char gender;
            vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face = parent->pilot->getCommFaces(gender);
            messagequeue.push_back(new CommunicationMessage(target, this->parent, state, state, comm_face, gender));
        }
    }
}

int CommunicatingAI::selectCommunicationMessage(CommunicationMessage &c, Unit *un) {
    if (0 && mood == 0) {
        FSM::Node *n = c.getCurrentState();
        if (n) {
            return rand() % n->edges.size();
        } else {
            return 0;
        }
    } else {
        static float moodmul = XMLSupport::parse_float(vs_config->getVariable("AI", "MoodAffectsRespose", "0"));
        static float angermul = XMLSupport::parse_float(vs_config->getVariable("AI", "AngerAffectsRespose", "1"));
        static float staticrelmul =
                XMLSupport::parse_float(vs_config->getVariable("AI", "StaticRelationshipAffectsResponse", "1"));
        return selectCommunicationMessageMood(c, moodmul * mood + angermul * parent->pilot->getAnger(parent,
                un) + staticrelmul
                * UnitUtil::getFactionRelation(parent, un));
    }
}

void CommunicatingAI::ProcessCommMessage(CommunicationMessage &c) {
    if (messagequeue.back()->curstate < messagequeue.back()->fsm->GetUnDockNode()) {
        Order::ProcessCommMessage(c);
        FSM *tmpfsm = c.fsm;
        Unit *targ = c.sender.GetUnit();
        if (targ && UnitUtil::getUnitSystemFile(targ) == UnitUtil::getUnitSystemFile(parent) && !isDockedAtAll(targ)) {
            c.fsm = FactionUtil::GetConversation(parent->faction, targ->faction);
            FSM::Node *n = c.getCurrentState();
            if (n) {
                if (n->edges.size()) {
                    Unit *un = c.sender.GetUnit();
                    if (un) {
                        int b = selectCommunicationMessage(c, un);
                        Order *o = un->getAIState();
                        unsigned char gender;
                        vega_types::SharedPtr<std::vector<vega_types::SharedPtr<Animation>>> comm_face = parent->pilot->getCommFaces(gender);
                        if (o) {
                            o->Communicate(CommunicationMessage(parent, un, c, b, comm_face, gender));
                        }
                    }
                }
            }
            c.fsm = tmpfsm;
        }
    }
}

CommunicatingAI::~CommunicatingAI() {
}

