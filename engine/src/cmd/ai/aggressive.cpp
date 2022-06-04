/*
 * aggressive.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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


#include <list>
#include <vector>
#include "aggressive.h"
#include "event_xml.h"
#include "script.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "config_xml.h"
#include "xml_support.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "cmd/script/flightgroup.h"
#include "flybywire.h"
#include "hard_coded_scripts.h"
#include "cmd/script/mission.h"
#include "gfx/cockpit_generic.h"
#include "lin_time.h"
#include "faction_generic.h"
#include "cmd/role_bitmask.h"
#include "cmd/unit_util.h"
#include "warpto.h"
#include "cmd/csv.h"
#include "universe_util.h"
#include "vs_random.h"
#include "python/python_compile.h"
#include "cmd/unit_find.h"
#include "faction_generic.h"
#include "docking.h"
#include "star_system.h"
#include "universe.h"

extern double aggfire;

using namespace Orders;
using namespace XMLSupport;

const EnumMap::Pair element_names[] = {
        EnumMap::Pair("AggressiveAI", AggressiveAI::AGGAI),
        EnumMap::Pair("UNKNOWN", AggressiveAI::UNKNOWN),
        EnumMap::Pair("Distance", AggressiveAI::DISTANCE),
        EnumMap::Pair("MeterDistance", AggressiveAI::METERDISTANCE),
        EnumMap::Pair("Threat", AggressiveAI::THREAT),
        EnumMap::Pair("FShield", AggressiveAI::FSHIELD),
        EnumMap::Pair("LShield", AggressiveAI::LSHIELD),
        EnumMap::Pair("RShield", AggressiveAI::RSHIELD),
        EnumMap::Pair("BShield", AggressiveAI::BSHIELD),
        EnumMap::Pair("Hull", AggressiveAI::HULL),
        EnumMap::Pair("Facing", AggressiveAI::FACING),
        EnumMap::Pair("Movement", AggressiveAI::MOVEMENT),
        EnumMap::Pair("FShield_Heal_Rate", AggressiveAI::FSHIELD_HEAL_RATE),
        EnumMap::Pair("BShield_Heal_Rate", AggressiveAI::BSHIELD_HEAL_RATE),
        EnumMap::Pair("LShield_Heal_Rate", AggressiveAI::LSHIELD_HEAL_RATE),
        EnumMap::Pair("RShield_Heal_Rate", AggressiveAI::RSHIELD_HEAL_RATE),
        EnumMap::Pair("FArmor_Heal_Rate", AggressiveAI::FARMOR_HEAL_RATE),
        EnumMap::Pair("BArmor_Heal_Rate", AggressiveAI::BARMOR_HEAL_RATE),
        EnumMap::Pair("LArmor_Heal_Rate", AggressiveAI::LARMOR_HEAL_RATE),
        EnumMap::Pair("RArmor_Heal_Rate", AggressiveAI::RARMOR_HEAL_RATE),
        EnumMap::Pair("Hull_Heal_Rate", AggressiveAI::HULL_HEAL_RATE),
        EnumMap::Pair("Target_Faces_You", AggressiveAI::TARGET_FACES_YOU),
        EnumMap::Pair("Target_In_Front_Of_You", AggressiveAI::TARGET_IN_FRONT_OF_YOU),
        EnumMap::Pair("Rand", AggressiveAI::RANDOMIZ),
        EnumMap::Pair("Target_Going_Your_Direction", AggressiveAI::TARGET_GOING_YOUR_DIRECTION)
};
const EnumMap AggressiveAIel_map(element_names, 25);

using std::pair;

vsUMap<string, AIEvents::ElemAttrMap *> logic;

extern bool CheckAccessory(Unit *tur);

static void TurretFAW(Unit *parent) {
    un_iter iter = parent->getSubUnits();
    Unit *un;
    while (NULL != (un = *iter)) {
        if (!CheckAccessory(un)) {
            un->EnqueueAIFirst(new Orders::FireAt(configuration()->ai.firing_config.aggressivity));
            un->EnqueueAIFirst(new Orders::FaceTarget(false, 3));
        }
        TurretFAW(un);
        ++iter;
    }
}

static vsUMap<string, string> getAITypes() {
    vsUMap<string, string> ret;
    VSFileSystem::VSFile f;
    VSFileSystem::VSError err = f.OpenReadOnly("VegaPersonalities.csv", VSFileSystem::AiFile);
    if (err <= VSFileSystem::Ok) {
        CSVTable table(f, f.GetRoot());
        vsUMap<std::string, int>::iterator browser = table.rows.begin();
        for (; browser != table.rows.end(); ++browser) {
            string rowname = (*browser).first;
            CSVRow row(&table, rowname);
            for (unsigned int i = 1; i < table.key.size(); ++i) {
                string hasher = rowname;
                if (i != 1) {
                    hasher = rowname + "%" + table.key[i];
                }
                string rawrow = row[i];
                if (rawrow.length() > 0) {
                    ret[hasher] = rawrow;
                }
            }
        }
        f.Close();
    }
    return ret;
}

static string select_from_space_list(string inp, unsigned int seed) {
    if (inp.length() == 0) {
        return "";
    }
    int count = 1;
    string::size_type len = inp.length();
    for (unsigned int i = 0; i < len; ++i) {
        if (inp[i] == ' ') {
            count++;
        }
    }
    count = seed % count;
    int ncount = 0;
    unsigned int j;
    for (j = 0; j < len; ++j) {
        if (inp[j] == ' ') {
            ncount++;
        }
        if (ncount >= count) {
            break;
        }
    }
    if (inp[j] == ' ') {
        j++;
    }
    inp = inp.substr(j);
    if ((len = inp.find(" ")) != string::npos) {
        inp = inp.substr(0, len);
    }
    return inp;
}

static AIEvents::ElemAttrMap *getLogicOrInterrupt(string name,
        int faction,
        string unittype,
        vsUMap<string, AIEvents::ElemAttrMap *> &mymap,
        int personalityseed) {
    string append = "agg";
    static vsUMap<string, string> myappend = getAITypes();
    vsUMap<string, string>::iterator iter;
    string factionname = FactionUtil::GetFaction(faction);
    if ((iter = myappend.find(factionname + "%" + unittype)) != myappend.end()) {
        append = select_from_space_list((*iter).second, personalityseed);
    } else if ((iter = myappend.find(factionname)) != myappend.end()) {
        append = select_from_space_list((*iter).second, personalityseed);
    }
    if (append.length() == 0) {
        append = "agg";
    }
    string hashname = name + "." + append;
    vsUMap<string, AIEvents::ElemAttrMap *>::iterator i = mymap.find(hashname);
    if (i == mymap.end()) {
        AIEvents::ElemAttrMap *attr = new AIEvents::ElemAttrMap(AggressiveAIel_map);
        string filename(name + "." + append + ".xml");
        AIEvents::LoadAI(filename.c_str(), *attr, FactionUtil::GetFaction(faction));
        mymap.insert(pair<string, AIEvents::ElemAttrMap *>(hashname, attr));
        return attr;
    }
    return i->second;
}

static AIEvents::ElemAttrMap *getProperLogicOrInterruptScript(string name,
        int faction,
        string unittype,
        bool interrupt,
        int personalityseed) {
    return getLogicOrInterrupt(name, faction, unittype, logic, personalityseed);
}

static AIEvents::ElemAttrMap *getProperScript(Unit *me, Unit *targ, bool interrupt, int personalityseed) {
    if (!me || !targ) {
        string nam = "eject";
        int fac = 0;
        if (me) {
            fac = me->faction;
            nam = me->name;
        }
        return getProperLogicOrInterruptScript("default", fac, nam, interrupt, personalityseed);
    }
    return getProperLogicOrInterruptScript(ROLES::getRoleEvents(me->getAttackPreferenceChar(),
                    targ->getUnitRoleChar()),
            me->faction,
            me->name,
            interrupt,
            personalityseed);
}

static float aggressivity = 2.01F;
static int randomtemp;

AggressiveAI::AggressiveAI(const char *filename, Unit *target)
        : FireAt(), logic(getProperScript(nullptr, nullptr, "default", randomtemp = rand())) {
    currentpriority = 0;
    last_jump_time = 0;
    nav = QVector(0, 0, 0);
    personalityseed = randomtemp;
    last_jump_distance = FLT_MAX;
    interruptcurtime = 0;
    creationtime = getNewTime();
    jump_time_check = 1;
    last_time_insys = true;
    logiccurtime = logic->maxtime;     //set it to the time allotted
    obedient = true;
    if (aggressivity == 2.01F) {
        aggressivity = configuration()->unit_config_.default_aggressivity;
    }
    if (target != nullptr) {
        AttachOrder(target);
    }
    last_directive = filename;
}

void AggressiveAI::SetParent(Unit *parent1) {
    FireAt::SetParent(parent1);
    string::size_type which = last_directive.find("|");
    string filename(string("default.agg.xml"));
    string interruptname(string("default.int.xml"));
    if (which != string::npos) {
        filename = last_directive.substr(0, which);
        interruptname = last_directive.substr(which + 1);
    }
    last_directive = "b";     //prevent escort race condition

    //INIT stored stuff
    Fshield_prev = parent->graphicOptions.InWarp ? 1 : parent->FShieldData();
    Fshield_rate_old = 0.0;
    Fshield_prev_time = UniverseUtil::GetGameTime();
    Bshield_prev = parent->graphicOptions.InWarp ? 1 : parent->BShieldData();
    Bshield_rate_old = 0.0;
    Bshield_prev_time = UniverseUtil::GetGameTime();
    Lshield_prev = parent->graphicOptions.InWarp ? 1 : parent->LShieldData();
    Lshield_rate_old = 0.0;
    Lshield_prev_time = UniverseUtil::GetGameTime();
    Rshield_prev = parent->graphicOptions.InWarp ? 1 : parent->RShieldData();
    Rshield_rate_old = 0.0;
    Rshield_prev_time = UniverseUtil::GetGameTime();
    Farmour_prev = 1.0;
    Farmour_rate_old = 0.0;
    Farmour_prev_time = UniverseUtil::GetGameTime();
    Barmour_prev = 1.0;
    Barmour_rate_old = 0.0;
    Barmour_prev_time = UniverseUtil::GetGameTime();
    Larmour_prev = 1.0;
    Larmour_rate_old = 0.0;
    Larmour_prev_time = UniverseUtil::GetGameTime();
    Rarmour_prev = 1.0;
    Rarmour_rate_old = 0.0;
    Rarmour_prev_time = UniverseUtil::GetGameTime();
    Hull_prev = parent->GetHullPercent();
    Hull_rate_old = 0.0;
    Hull_prev_time = UniverseUtil::GetGameTime();
}

void AggressiveAI::SignalChosenTarget() {
    if (parent) {
        logic = getProperScript(parent, parent->Target(), false, personalityseed);
    }
    FireAt::SignalChosenTarget();
}

bool AggressiveAI::ExecuteLogicItem(const AIEvents::AIEvresult &item) {
    if (item.script.length() != 0) {
        Order *tmp = new ExecuteFor(new AIScript(item.script.c_str()), item.timetofinish);
        EnqueueOrder(tmp);
        return true;
    } else {
        return false;
    }
}

bool AggressiveAI::ProcessLogicItem(const AIEvents::AIEvresult &item) {
    float value = 0.0;

    switch (abs(item.type)) {
        case DISTANCE:
            value = distance;
            break;
        case METERDISTANCE: {
            Unit *targ = parent->Target();
            if (targ) {
                Vector PosDifference = targ->Position().Cast() - parent->Position().Cast();
                float pdmag = PosDifference.Magnitude();
                value = (pdmag - parent->rSize() - targ->rSize());
                float myvel = PosDifference.Dot(parent->GetVelocity() - targ->GetVelocity()) / value;        ///pdmag;
                if (myvel > 0) {
                    value -= myvel * myvel / (2 * (parent->limits.retro / parent->getMass()));
                }
            } else {
                value = 10000;
            }
            value /= configuration()->physics_config_.game_speed;     /*game_accel*/
            break;
        }
        case THREAT:
            value = parent->GetComputerData().threatlevel;
            break;
        case FSHIELD:
            value = parent->graphicOptions.InWarp ? 1 : parent->FShieldData();
            break;
        case BSHIELD:
            value = parent->graphicOptions.InWarp ? 1 : parent->BShieldData();
            break;
        case HULL: {
            value = parent->GetHullPercent();
            break;
        }
        case LSHIELD:
            value = parent->graphicOptions.InWarp ? 1 : parent->LShieldData();
            break;
        case RSHIELD:
            value = parent->graphicOptions.InWarp ? 1 : parent->RShieldData();
            break;
        case FSHIELD_HEAL_RATE: {
            double delta_t = UniverseUtil::GetGameTime() - Fshield_prev_time;
            if (delta_t > 0.5) {
                //0.5 = reaction time limit for hit rate
                double delta_v = parent->graphicOptions.InWarp ? 1 : parent->FShieldData() - Fshield_prev;
                value = delta_v / delta_t;
                Fshield_rate_old = value;
                Fshield_prev_time = UniverseUtil::GetGameTime();
                Fshield_prev = parent->graphicOptions.InWarp ? 1 : parent->FShieldData();
            } else {
                value = Fshield_rate_old;
            }
            break;
        }
        case BSHIELD_HEAL_RATE: {
            double delta_t = UniverseUtil::GetGameTime() - Bshield_prev_time;
            if (delta_t > 0.5) {
                //0.5 = reaction time limit for hit rate
                double delta_v = parent->graphicOptions.InWarp ? 1 : parent->BShieldData() - Bshield_prev;
                value = delta_v / delta_t;
                Bshield_rate_old = value;
                Bshield_prev_time = UniverseUtil::GetGameTime();
                Bshield_prev = parent->graphicOptions.InWarp ? 1 : parent->BShieldData();
            } else {
                value = Bshield_rate_old;
            }
            break;
        }
        case LSHIELD_HEAL_RATE: {
            double delta_t = UniverseUtil::GetGameTime() - Lshield_prev_time;
            if (delta_t > 0.5) {
                //0.5 = reaction time limit for hit rate
                double delta_v = parent->graphicOptions.InWarp ? 1 : parent->LShieldData() - Lshield_prev;
                value = delta_v / delta_t;
                Lshield_rate_old = value;
                Lshield_prev_time = UniverseUtil::GetGameTime();
                Lshield_prev = parent->graphicOptions.InWarp ? 1 : parent->LShieldData();
            } else {
                value = Lshield_rate_old;
            }
            break;
        }
        case RSHIELD_HEAL_RATE: {
            double delta_t = UniverseUtil::GetGameTime() - Rshield_prev_time;
            if (delta_t > 0.5) {
                //0.5 = reaction time limit for hit rate
                double delta_v = parent->graphicOptions.InWarp ? 1 : parent->RShieldData() - Rshield_prev;
                value = delta_v / delta_t;
                Rshield_rate_old = value;
                Rshield_prev_time = UniverseUtil::GetGameTime();
                Rshield_prev = parent->graphicOptions.InWarp ? 1 : parent->RShieldData();
            } else {
                value = Rshield_rate_old;
            }
            break;
        }
        case FARMOR_HEAL_RATE:
            value = 0.0;
            break;
        case BARMOR_HEAL_RATE:
            value = 0.0;
            break;
        case LARMOR_HEAL_RATE:
            value = 0.0;
            break;
        case RARMOR_HEAL_RATE:
            value = 0.5;
            break;
        case HULL_HEAL_RATE: {
            double delta_t = UniverseUtil::GetGameTime() - Hull_prev_time;
            if (delta_t > 0.5) {
                //0.5 = reaction time limit for hit rate
                double delta_v = parent->GetHullPercent() - Hull_prev;
                value = delta_v / delta_t;
                Hull_rate_old = value;
                Hull_prev_time = UniverseUtil::GetGameTime();
                Hull_prev = parent->GetHullPercent();
            } else {
                value = Hull_rate_old;
            }
            break;
        }
        case TARGET_FACES_YOU: {
            value = 0.0;
            Unit *targ = parent->Target();
            if (targ) {
                Vector Q;
                Vector P;
                Vector R;

                Vector PosDelta = (parent->Position()) - (targ->Position());
                PosDelta = PosDelta / PosDelta.Magnitude();
                targ->GetOrientation(Q, P, R);
                value = PosDelta.Dot(R);
            }
            break;
        }
        case TARGET_IN_FRONT_OF_YOU: {
            value = 0.0;
            Unit *targ = parent->Target();
            if (targ) {
                Vector Q;
                Vector P;
                Vector S;

                Vector PosDelta = (targ->Position()) - (parent->Position());
                PosDelta = PosDelta / PosDelta.Magnitude();
                parent->GetOrientation(Q, P, S);
                value = PosDelta.Dot(S);
            }
            break;
        }
        case TARGET_GOING_YOUR_DIRECTION: {
            value = 0.0;
            Unit *targ = parent->Target();
            if (targ) {
                Vector Q;
                Vector P;
                Vector R;
                Vector S;

                parent->GetOrientation(Q, P, S);
                targ->GetOrientation(Q, P, R);
                value = S.Dot(R);
            }
            break;
        }
        case FACING:
            return queryType(Order::FACING) == NULL;

        case MOVEMENT:
            return queryType(Order::MOVEMENT) == NULL;

        case RANDOMIZ:
            value = ((float) rand()) / RAND_MAX;
            break;
        default:
            return false;
    }
    return item.Eval(value);
}

bool AggressiveAI::ProcessLogic(AIEvents::ElemAttrMap &logi, bool inter) {
    //go through the logic.
    bool retval = false;
    std::vector<std::list<AIEvents::AIEvresult> >::iterator i = logi.result.begin();
    for (; i != logi.result.end(); i++) {
        std::list<AIEvents::AIEvresult>::iterator j;
        bool trueit = true;
        for (j = i->begin(); j != i->end(); j++) {
            if (!ProcessLogicItem(*j)) {
                trueit = false;
                break;
            }
        }
        if (trueit && j == i->end()) {
            //do it
            if (j != i->begin()) {
                j--;
            }
            if (j != i->end()) {
                float priority = (*j).priority;
                if (priority > this->currentpriority || !inter) {
                    if (inter) {
                        eraseType(Order::FACING);
                        eraseType(Order::MOVEMENT);
                    }
                    j = i->begin();
                    logiccurtime = 0;
                    interruptcurtime = 0;
                    if (j != i->end()) {
                        while (j != i->end()) {
                            if (ExecuteLogicItem(*j)) {
                                this->currentpriority = priority;
                                logiccurtime += (*j).timetofinish;
                                interruptcurtime += (*j).timetointerrupt;
                                retval = true;
                            }
                            j++;
                        }
                        if (retval) {
                            break;
                        }
                    }
                }
            }
        }
    }
    return retval;
}

Unit *GetThreat(Unit *parent, Unit *leader) {
    Unit *th = NULL;
    Unit *un = NULL;
    bool targetted = false;
    float mindist = FLT_MAX;
    for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator();
            (un = *ui);
            ++ui) {
        if (parent->getRelation(un) < 0) {
            float d = (un->Position() - leader->Position()).Magnitude();
            bool thistargetted = (un->Target() == leader);
            if (!th || (thistargetted && !targetted) || ((thistargetted || (!targetted)) && d < mindist)) {
                th = un;
                targetted = thistargetted;
                mindist = d;
            }
        }
    }
    return th;
}

bool AggressiveAI::ProcessCurrentFgDirective(Flightgroup *fg) {
    bool retval = false;
    if (fg != NULL) {
        Unit *leader = fg->leader.GetUnit();
        if (last_directive.empty()) {
            last_directive = fg->directive;
        }
        if (fg->directive != last_directive) {
            if (configuration()->ai.always_obedient) {
                obedient = true;
            } else if (float ( rand())/RAND_MAX < (obedient ? (1 - logic->obedience) : logic->obedience)) {
                obedient = !obedient;
            }
            if (obedient) {
                eraseType(Order::FACING);
                eraseType(Order::MOVEMENT);
                Unit *targ = parent->Target();
                if (targ) {
                    bool attacking = fg->directive.length() > 0;
                    if (attacking) {
                        attacking = tolower(fg->directive[0]) == 'a';
                    }
                    if ((!isJumpablePlanet(targ)) && attacking == false) {
                        parent->Target(NULL);
                    }
                }
            } else {
                CommunicationMessage c(parent, leader, NULL, 0);
                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                Order *lo = leader->getAIState();
                if (lo) {
                    lo->Communicate(c);
                }
            }
        }
        if (obedient) {
            void *parentowner;
            void *leaderowner = leader;
            parentowner = parent->owner ? parent->owner : parent;
            leaderowner = leader->owner ? leader->owner : leader;
            if (fg->directive.find("k") != string::npos || fg->directive.find("K") != string::npos) {
                Unit *targ = fg->target.GetUnit();
                bool callme = false;
                if (targ && (targ->faction != parent->faction)) {
                    if (targ->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        CommunicationMessage c(parent, leader, NULL, 0);
                        c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                        if (parent->InRange(targ, true, false)) {
                            if (targ != parent->Target()) {
                                callme = true;
                            }
                            parent->Target(targ);
                            parent->SetTurretAI();
                            parent->TargetTurret(targ);
                            //if I am the capship, go into defensive mode.
                            if (parent == leaderowner) {
                                //get in front of me
                                parent->TurretFAW();

                                c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                                Order *ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,
                                                        0,
                                                        0),
                                                true),
                                        true,
                                        false,
                                        true);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                //facing forward
                                ord = new Orders::FaceTarget(false, 3);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                            } else {
                                Order *ord = new Orders::FaceTarget(false, 3);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                            }
                        } else {
                            c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                        }
                        if (fg->directive != last_directive) {
                            Order *lo = leader->getAIState();
                            if (lo || callme) {
                                lo->Communicate(c);
                            }
                        }
                    }
                }
                //a is now used for AI, for backward compatibility. do not use for player
            } else if (fg->directive.find("a") != string::npos || fg->directive.find("A") != string::npos) {
                Unit *targ = fg->leader.GetUnit();
                targ = targ != NULL ? targ->Target() : NULL;
                if (targ) {
                    if (targ->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        CommunicationMessage c(parent, leader, NULL, 0);
                        if (parent->InRange(targ, true, false)) {
                            parent->Target(targ);
                            parent->TargetTurret(targ);
                            c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                        } else {
                            c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                        }
                        if (fg->directive != last_directive) {
                            Order *lo = leader->getAIState();
                            if (lo) {
                                lo->Communicate(c);
                            }
                        }
                    }
                }
            } else if (fg->directive.find("f") != string::npos || fg->directive.find("F") != string::npos) {
                if (leader != NULL) {
                    if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        retval = true;
                        if (fg->directive != last_directive || (!last_time_insys)) {
                            last_time_insys = true;
                            CommunicationMessage c(parent, leader, NULL, 0);
                            c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                            Order *o = leader->getAIState();
                            if (o) {
                                o->Communicate(c);
                            }
                            const float esc_percent = configuration()->ai.targeting_config.escort_distance;
                            const float turn_leader = configuration()->ai.targeting_config.turn_leader_distance;
                            int fgnum = parent->getFgSubnumber();
                            if (parent->getFlightgroup()) {
                                int tempnum = 0;
                                string nam = parent->getFlightgroup()->name;
                                int i = nam.length() - 1;
                                for (; i >= 0; --i) {
                                    char digit = nam[i];
                                    if (digit >= '0' && digit <= '9') {
                                        tempnum *= 10;
                                        tempnum += digit - '0';
                                    } else {
                                        break;
                                    }
                                }
                                fgnum += tempnum;
                            }
                            float left = fgnum % 2 ? 1 : -1;

                            double dist =
                                    esc_percent * (1 + abs(fgnum - 1) / 2) * left * (parent->rSize() + leader->rSize());
                            Order *ord = new Orders::FormUp(QVector(dist, 0, -fabs(dist)));
                            ord->SetParent(parent);
                            ReplaceOrder(ord);
                            ord = new Orders::FaceDirection(dist * turn_leader);
                            ord->SetParent(parent);
                            ReplaceOrder(ord);
                        }
                    } else {
                        last_time_insys = false;
                    }
                    for (unsigned int i = 0; i < suborders.size(); i++) {
                        suborders[i]->AttachSelfOrder(leader);
                    }
                }
            }
                //IAmDave - hold position command
            else if (fg->directive.find("s") != string::npos || fg->directive.find("S") != string::npos) {
                Order *ord = new Orders::MatchVelocity(Vector(0, 0, 0), Vector(0, 0, 0), true, false);
                ord->SetParent(parent);
                ReplaceOrder(ord);
            }
                //IAmDave - dock at target command start...
            else if (fg->directive.find("t") != string::npos || fg->directive.find("T") != string::npos) {
                Unit *targ = fg->target.GetUnit();
                if (targ->InCorrectStarSystem(_Universe->activeStarSystem())) {
                    Order *ord;
                    if (targ->IsBase()) {
                        ord = new Orders::DockingOps(targ,
                                new Orders::MatchVelocity(Vector(0, 0, 0),
                                        Vector(0, 0, 0),
                                        true,
                                        false),
                                true,
                                true);
                    } else {
                        ord = new Orders::MoveTo(targ->Position(), true, 4);
                    }
                    ord->SetParent(parent);
                    ReplaceOrder(ord);
                }

            }
                //IAmDave - ...dock at target command end.
            else if (fg->directive.find("l") != string::npos || fg->directive.find("L") != string::npos) {
                if (leader != NULL) {
                    if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        retval = true;
                        if (fg->directive != last_directive || (!last_time_insys)) {
                            last_time_insys = true;
                            CommunicationMessage c(parent, leader, NULL, 0);
//this order is only valid for cargo wingmen, other wingmen will not comply
                            c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                            Order *o = leader->getAIState();
                            if (o) {
                                o->Communicate(c);
                            }
                            const float esc_percent = configuration()->ai.targeting_config.escort_distance;
                            const float turn_leader = configuration()->ai.targeting_config.turn_leader_distance;
                            int fgnum = parent->getFgSubnumber();
                            if (parent->getFlightgroup()) {
                                int tempnum = 0;
                                string nam = parent->getFlightgroup()->name;
                                int i = nam.length() - 1;
                                for (; i >= 0; --i) {
                                    char digit = nam[i];
                                    if (digit >= '0' && digit <= '9') {
                                        tempnum *= 10;
                                        tempnum += digit - '0';
                                    } else {
                                        break;
                                    }
                                }
                                fgnum += tempnum;
                            }
/*
 *  // this does the job for real! "parent" is executor, "leader" is commander
 *
 *  // moves where you want it to
 *  // moves flat out in front of parent unit (to allow for tractoring)
 *                 Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
 *             ord->SetParent (parent);
 *             ReplaceOrder (ord);
 *  // faces same direction as leader
 *  //		  ord = new Orders::FaceDirection(dist*turn_leader);
 *  // faces opposite direction as leader, as in, stare at me in the face please
 *                 ord = new Orders::FaceDirection(-dist*turn_leader);
 *             ord->SetParent (parent);
 *             ReplaceOrder (ord);
 */

                            int alternate = fgnum % 2 ? 1 : -1;
                            float psize = parent->radial_size;
                            int Ypos = 0;
                            int Xpos = 0;
//nice square formation, how many of these are you going to have anyway? Max 9, then go back. Should be enough.
                            switch (fgnum % 9) {
                                case 0:
                                    Xpos = 0;
                                    Ypos = 0;
                                    break;
                                case 1:
                                    Xpos = -1;
                                    Ypos = 0;
                                    break;
                                case 2:
                                    Xpos = 1;
                                    Ypos = 0;
                                    break;
                                case 3:
                                    Xpos = 0;
                                    Ypos = -1;
                                    break;
                                case 4:
                                    Xpos = -1;
                                    Ypos = -1;
                                    break;
                                case 5:
                                    Xpos = 1;
                                    Ypos = -1;
                                    break;
                                case 6:
                                    Xpos = 0;
                                    Ypos = 1;
                                    break;
                                case 7:
                                    Xpos = -1;
                                    Ypos = 1;
                                    break;
                                case 8:
                                    Xpos = 1;
                                    Ypos = 1;
                                    break;
                                default:
                                    Xpos = 0;
                                    Ypos = 0;
                            }
                            float dist = (leader->radial_size + parent->radial_size * 2);
                            float formdist = esc_percent * (1 + fgnum * 2) * alternate * (dist);
                            //if i am a cargo wingman, get into a dockable position
                            if (parentowner == leader) {
                                // move in front
                                Order *ord = new Orders::FormUp(QVector(0, 0, fabs(dist)));
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                //facing me
                                ord = new Orders::FaceDirection(-dist * turn_leader);
                                EnqueueOrderFirst(ord);
                            }
                                //if i am a cargo wingman and so is the player, get into a dockable position with the leader
                            else if (parentowner && leaderowner && (parentowner == leaderowner)) {
                                const Unit *leaderownerun =
                                        (leaderowner
                                                == leader ? leader : (leaderowner == parent ? parent
                                                : findUnitInStarsystem(
                                                        leaderowner)));
                                float qdist = (parent->rSize() + leaderownerun->rSize());
                                Order *ord =
                                        new Orders::MoveTo(leaderownerun->Position() + Vector(0.5 * Xpos * psize,
                                                0.5 * Ypos * psize,
                                                0.5 * qdist), true, 4);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                //facing it
                                ord = new Orders::FaceDirection(-qdist * turn_leader);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                            }
                                //if i am the capship, go into defensive mode
                            else if (parent == leaderowner) {
                                parent->SetTurretAI();
                                TurretFAW(parent);
                                Order *ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,
                                                        0,
                                                        0),
                                                true),
                                        true,
                                        false,
                                        true);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                if (parent->Target() != NULL) {
                                    ord = new Orders::FaceTarget(false, 3);
                                } else {
                                    ord = new Orders::FaceDirection(-dist * turn_leader);
                                }
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                            } else {
                                //if i'm not a cargo wingman, just form up somewhat loosely.
                                parentowner = parent;
                                Order *ord =
                                        new Orders::FormUp(QVector(5 * Xpos * psize,
                                                5 * Ypos * psize,
                                                -fabs(formdist) + Ypos * psize + Xpos
                                                        * psize));
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                ord = new Orders::FaceDirection(dist * turn_leader);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                            }
                        }
                    } else {
                        last_time_insys = false;
                    }
                    for (unsigned int i = 0; i < suborders.size(); i++) {
                        suborders[i]->AttachSelfOrder(leader);
                    }
                }
            } else if (fg->directive.find("e") != string::npos || fg->directive.find("E") != string::npos) {
                static QVector LeaderPosition = QVector(0, 0, 0);
                if (LeaderPosition.Magnitude() > 0 || leader != NULL) {
                    if (LeaderPosition.Magnitude() > 0 || leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        retval = true;
                        if (LeaderPosition.Magnitude()
                                == 0) {                          //only read the position the first time
                            LeaderPosition = leader->Position();
                        }
                        if (fg->directive != last_directive || (!last_time_insys)) {
                            last_time_insys = true;
                            CommunicationMessage c(parent, leader, NULL, 0);
//this order is only valid for cargo wingmen, other wingmen will not comply
                            c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                            const float turn_leader = configuration()->ai.targeting_config.turn_leader_distance;
                            int fgnum = parent->getFgSubnumber();
                            if (parent->getFlightgroup()) {
                                int tempnum = 0;
                                string nam = parent->getFlightgroup()->name;
                                int i = nam.length() - 1;
                                for (; i >= 0; --i) {
                                    char digit = nam[i];
                                    if (digit >= '0' && digit <= '9') {
                                        tempnum *= 10;
                                        tempnum += digit - '0';
                                    } else {
                                        break;
                                    }
                                }
                                fgnum += tempnum;
                            }
/*
 *  // this does the job for real! "parent" is executor, "leader" is commander
 *
 *  // moves where you want it to
 *  // moves flat out in front of parent unit (to allow for tractoring)
 *                 Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
 *             ord->SetParent (parent);
 *             ReplaceOrder (ord);
 *  // faces same direction as leader
 *  //		  ord = new Orders::FaceDirection(dist*turn_leader);
 *  // faces opposite direction as leader, as in, stare at me in the face please
 *                 ord = new Orders::FaceDirection(-dist*turn_leader);
 *             ord->SetParent (parent);
 *             ReplaceOrder (ord);
 */

                            float psize = parent->radial_size;
                            int Ypos = 0;
                            int Xpos = 0;
//nice square formation, how many of these are you going to have anyway? Max 9, then go back. Should be enough.
                            switch (fgnum % 9) {
                                case 0:
                                    Xpos = 0;
                                    Ypos = 0;
                                    break;
                                case 1:
                                    Xpos = -1;
                                    Ypos = 0;
                                    break;
                                case 2:
                                    Xpos = 1;
                                    Ypos = 0;
                                    break;
                                case 3:
                                    Xpos = 0;
                                    Ypos = -1;
                                    break;
                                case 4:
                                    Xpos = -1;
                                    Ypos = -1;
                                    break;
                                case 5:
                                    Xpos = 1;
                                    Ypos = -1;
                                    break;
                                case 6:
                                    Xpos = 0;
                                    Ypos = 1;
                                    break;
                                case 7:
                                    Xpos = -1;
                                    Ypos = 1;
                                    break;
                                case 8:
                                    Xpos = 1;
                                    Ypos = 1;
                                    break;
                                default:
                                    Xpos = 0;
                                    Ypos = 0;
                            }
                            //if i am a cargo wingman go close for pickup
                            //if i am the capship, go close for pickup
                            if ((parent->owner == leader->owner) || parent->owner == leader) {
//float left= fgnum%2?1:-1;
                                float qdist = (1.5 * parent->rSize() + 1.5 * leader->rSize());
                                Order *ord =
                                        new Orders::MoveTo(LeaderPosition + Vector(0.5 * Xpos * psize,
                                                0.5 * Ypos * psize,
                                                0.5 * qdist), true, 4);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                                //facing it
                                ord = new Orders::FaceDirection(-qdist * turn_leader);
                                ord->SetParent(parent);
                                ReplaceOrder(ord);
                            }
                                //if i'm not a cargo wingman, IT'S NOT MY PROBLEM.
                            else {
                                parent->owner = parent;
                                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                                Flightgroup *leave = new Flightgroup();
                                leave->directive = "b";
                                parent->SetFg(leave, 1);
                            }
                            Order *o = leader->getAIState();
                            if (o) {
                                o->Communicate(c);
                            }
                        }
                    } else {
                        last_time_insys = false;
                    }
                    for (unsigned int i = 0; i < suborders.size(); i++) {
                        suborders[i]->AttachSelfOrder(leader);
                    }
                }
            } else if (fg->directive.find("h") != string::npos || fg->directive.find("H") != string::npos) {
                if (fg->directive != last_directive && leader) {
                    if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        Unit *th = NULL;
                        if ((th = leader->Threat())) {
                            CommunicationMessage c(parent, leader, NULL, 0);
                            if (parent->InRange(th, true, false)) {
                                parent->Target(th);
                                parent->TargetTurret(th);
                                c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                            } else {
                                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                            }
                            Order *oo = leader->getAIState();
                            if (oo) {
                                oo->Communicate(c);
                            }
                        } else {
                            th = GetThreat(parent, leader);
                            CommunicationMessage c(parent, leader, NULL, 0);
                            if (th) {
                                if (parent->InRange(th, true, false)) {
                                    c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                                    parent->Target(th);
                                    parent->TargetTurret(th);
//if I am the capship, go into defensive mode.
                                    if (parent == leader->owner) {
                                        //parent->Target(parent);
                                        parent->SetTurretAI();
                                        TurretFAW(parent);
                                        Order *ord =
                                                new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,
                                                                        0,
                                                                        0),
                                                                true),
                                                        true,
                                                        false,
                                                        true);
                                        ord->SetParent(parent);
                                        ReplaceOrder(ord);
                                        if (parent->Target() != NULL) {
                                            ord = new Orders::FaceTarget(false, 3);
                                            ord->SetParent(parent);
                                            ReplaceOrder(ord);
                                        }
                                    }
                                } else {
                                }
                            } else {
                                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                            }
                            Order *loo = leader->getAIState();
                            if (loo) {
                                loo->Communicate(c);
                            }
                        }
                    }
                }
            } else if (fg->directive.find("p") != string::npos || fg->directive.find("P") != string::npos) {
                bool callme = false;
                if (fg->directive != last_directive && leader) {
                    if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
                        Unit *th = NULL;
                        Unit *targ = fg->target.GetUnit();
                        if (targ && (th = targ->Threat())) {
                            CommunicationMessage c(parent, leader, NULL, 0);
                            if (parent->InRange(th, true, false)) {
                                parent->Target(th);
                                parent->TargetTurret(th);
                                c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                                fg->directive = "";
                            } else {
                                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                            }
                            Order *oo = leader->getAIState();
                            if (oo) {
                                oo->Communicate(c);
                            }
                        } else {
                            th = GetThreat(parent, targ);
                            CommunicationMessage c(parent, leader, NULL, 0);
                            if (th) {
                                if (parent->InRange(th, true, false)) {
                                    c.SetCurrentState(c.fsm->GetYesNode(), NULL, 0);
                                    if (th != parent->Target()) {
                                        callme = true;
                                    }
                                    parent->Target(th);
                                    parent->TargetTurret(th);
//if I am the capship, go into defensive mode.
                                    if (parent == leaderowner) {
                                        parent->SetTurretAI();
                                        parent->TurretFAW();
                                        Order *ord =
                                                new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,
                                                                        0,
                                                                        0),
                                                                true),
                                                        true,
                                                        false,
                                                        true);
                                        ord->SetParent(parent);
                                        ReplaceOrder(ord);
                                        if (parent->Target() != NULL) {
                                            ord = new Orders::FaceTarget(false, 3);
                                            ord->SetParent(parent);
                                            ReplaceOrder(ord);
                                        }
                                    }
                                } else {
                                    c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                                }
                            } else {
                                c.SetCurrentState(c.fsm->GetNoNode(), NULL, 0);
                            }
                            Order *loo = leader->getAIState();
                            if (loo || callme) {
                                loo->Communicate(c);
                            }
                        }
                    }
                }
            }
        }
        last_directive = fg->directive;
    }
    return retval;
}

static bool overridable(const std::string &s) {
    if (s.empty()) {
        return true;
    }
    return (*s.begin()) != toupper(*s.begin());
}

extern void LeadMe(Unit *un, string directive, string speech, bool changetarget);

void AggressiveAI::ReCommandWing(Flightgroup *fg) {
    const float time_to_recommand_wing = configuration()->ai.targeting_config.time_to_recommand_wing;
    const bool verbose_debug = configuration()->logging.verbose_debug;
    if (fg != nullptr) {
        Unit *lead;
        if (overridable(fg->directive)) {
            //computer won't override capital orders
            if (nullptr != (lead = fg->leader.GetUnit())) {
                if (float ( rand())/RAND_MAX < simulation_atom_var / time_to_recommand_wing) {
                    if (parent->Threat() && (parent->FShieldData() < .2 || parent->RShieldData() < .2)) {
                        fg->directive = string("h");
                        LeadMe(parent, "h", "I need help here!", false);
                        if (verbose_debug) {
                            VS_LOG(trace, (boost::format("he needs help %1%") % parent->name.get().c_str()));
                        }
                    } else if (lead->getFgSubnumber() >= parent->getFgSubnumber()) {
                        fg->directive = string("b");
                        LeadMe(parent, "b", "I'm taking over this wing. Break and attack", false);
                    }
                }
            }
        }
    }
}

static Unit *GetRandomNav(vector<UnitContainer> navs[3], unsigned int randnum) {
    size_t total_size = navs[0].size() + navs[1].size() + navs[2].size();
    if (total_size == 0) {
        return NULL;
    }
    randnum %= total_size;
    if (randnum >= navs[0].size()) {
        randnum -= navs[0].size();
        if (randnum >= navs[1].size()) {
            randnum -= navs[1].size();
            return navs[2][randnum].GetUnit();
        }
        return navs[1][randnum].GetUnit();
    }
    return navs[0][randnum].GetUnit();
}

static std::string insysString("Insys");

static std::string arrowString("->");

static Unit *ChooseNavPoint(Unit *parent, Unit **otherdest, float *lurk_on_arrival) {
    static string script = vs_config->getVariable("AI", "ChooseDestinationScript", "");
    *lurk_on_arrival = 0;
    if (script.length() > 0) {
        Unit *ret = NULL;
        UniverseUtil::setScratchUnit(parent);
        CompileRunPython(script);
        ret = UniverseUtil::getScratchUnit();
        UniverseUtil::setScratchUnit(NULL);
        if (ret != NULL && ret != parent) {
            return ret;
        }
    }
    StarSystem *ss = _Universe->activeStarSystem();
    Statistics *stats = &ss->stats;

    float sysrel = UnitUtil::getRelationFromFaction(parent, stats->system_faction);
    static float lurk_time = XMLSupport::parse_float(vs_config->getVariable("AI", "lurk_time", "600"));
    static float select_time = XMLSupport::parse_float(vs_config->getVariable("AI", "fg_nav_select_time", "120"));
    static float hostile_select_time =
            XMLSupport::parse_float(vs_config->getVariable("AI", "pirate_nav_select_time", "400"));
    static int num_ships_per_roid =
            XMLSupport::parse_int(vs_config->getVariable("AI", "num_pirates_per_asteroid_field", "12"));
    bool civilian = FactionUtil::isCitizenInt(parent->faction);

    bool hostile = sysrel < 0;
    bool anarchy = stats->enemycount > stats->friendlycount;
    float timehash = select_time;
    if (hostile && !anarchy) {
        timehash = hostile_select_time;
    }
    unsigned int firstRand, thirdRand;
    float secondRand;
    const unsigned int maxrand = 5;
    unsigned int additionalrand[maxrand];
    if (civilian) {
        firstRand = vsrandom.genrand_int31();
        secondRand = vsrandom.uniformExc(0, 1);
        thirdRand = vsrandom.genrand_int31();
        for (unsigned int i = 0; i < maxrand; ++i) {
            additionalrand[i] = thirdRand + i;
        }
    } else {
        int k = (int) (getNewTime() / timehash);        //two minutes
        string key = UnitUtil::getFlightgroupName(parent);
        std::string::const_iterator start = key.begin();
        for (; start != key.end(); start++) {
            k += (k * 128) + *start;
        }
        VSRandom choosePlace(k);
        firstRand = choosePlace.genrand_int31();
        secondRand = choosePlace.uniformExc(0, 1);
        thirdRand = choosePlace.genrand_int31();
        for (unsigned int i = 0; i < maxrand; ++i) {
            additionalrand[i] = choosePlace.genrand_int31();
        }
    }
    bool asteroidhide = false;
    if (stats->friendlycount > 0 && stats->enemycount > 0) {
        asteroidhide = (secondRand < stats->enemycount / (float) stats->friendlycount)
                && (secondRand < num_ships_per_roid * stats->navs[2].size() / (float) stats->enemycount);
    }
    bool siege = stats->enemycount > 2 * stats->friendlycount;       //rough approx
    int whichlist = 1;  //friendly
    std::string fgname = UnitUtil::getFlightgroupName(parent);

    bool insys = (parent->GetJumpStatus().drive == -2) || fgname.find(insysString) != std::string::npos;
    std::string::size_type whereconvoy = fgname.find(arrowString);
    bool convoy = (whereconvoy != std::string::npos);
    size_t total_size = stats->navs[0].size() + stats->navs[whichlist].size();     //friendly and neutral
    static bool bad_units_lurk = XMLSupport::parse_bool(vs_config->getVariable("AI", "hostile_lurk", "true"));
    if (hostile && bad_units_lurk) {
        if (anarchy && !siege) {
            whichlist = 2;
            total_size = stats->navs[0].size() + stats->navs[whichlist].size();             //asteroids and neutrals
        } else {
            whichlist = 2;
            total_size = stats->navs[whichlist].size();             //just asteroids
        }
    } else if (civilian) {
        if (anarchy || siege) {
            whichlist = 0;
            total_size = stats->navs[0].size();
        } else if (insys || convoy) {
            whichlist = 1;
            total_size = stats->navs[1].size();             //don't go to jump point
        }
    }
    if (hostile && ((anarchy == false && asteroidhide == false) || total_size == 0) && civilian == false
            && bad_units_lurk) {
        //hit and run
        Unit *a = GetRandomNav(stats->navs, firstRand);
        Unit *b = GetRandomNav(stats->navs, thirdRand);
        if (a == b) {
            b = GetRandomNav(stats->navs, thirdRand + 1);
        }
        if (a != b) {
            int retrycount = maxrand;
            while (--retrycount > 0
                    && (UnitUtil::getDistance(a, b) < parent->GetComputerData().radar.maxrange * 4 || a == b)) {
                b = GetRandomNav(stats->navs, additionalrand[retrycount]);
            }
            if (retrycount != 0) {
                *otherdest = b;
                *lurk_on_arrival = lurk_time;
            }
        }
        return a;
    } else {
        if (convoy) {
            std::string srcdst[2] = {fgname.substr(0, whereconvoy), fgname.substr(whereconvoy + 2)};
            if (srcdst[0] == ss->getFileName()) {
                srcdst[0] = srcdst[1];
            }
            if (srcdst[1] == ss->getFileName()) {
                srcdst[1] = srcdst[0];
            }
            if (thirdRand < 2) {
                vsUMap<std::string, UnitContainer>::iterator i = stats->jumpPoints.find(srcdst[thirdRand]);
                if (i != stats->jumpPoints.end()) {
                    Unit *un = i->second.GetUnit();
                    if (un) {
                        return un;
                    }
                } else {
                    total_size = stats->navs[whichlist].size()
                            + stats->navs[0].size();                     //no such jump point--have to random-walk it
                    //maybe one day we can incorporate some sort of route planning
                }
            }
        }
        if (total_size > 0) {
            firstRand %= total_size;
            if (firstRand >= stats->navs[whichlist].size()) {
                firstRand -= stats->navs[whichlist].size();
                whichlist = 0;                 //allows you to look for both neutral and ally lists
            }
            return stats->navs[whichlist][firstRand].GetUnit();
        }
    }
    return NULL;
}

static Unit *ChooseNearNavPoint(Unit *parent, Unit *suggestion, QVector location, float locradius) {
    if (suggestion) {
        return suggestion;
    }
    Unit *candidate = NULL;
    float dist = FLT_MAX;
    Unit *un;
    NearestNavOrCapshipLocator nnl;
    findObjects(_Universe->activeStarSystem()->collide_map[Unit::UNIT_ONLY],
            parent->location[Unit::UNIT_ONLY],
            &nnl);
    return nnl.retval.unit;
    //DEAD CODE
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
            (un = *i) != NULL;
            ++i) {
        if (UnitUtil::isSignificant(un) && un != parent) {
            float newdist = (location - un->Position()).Magnitude() - un->rSize() - locradius;
            if (candidate == NULL || newdist <= dist) {
                candidate = un;
                dist = newdist;
            }
        }
    }
    return candidate;
    //END DEAD CODE
}

bool CloseEnoughToNavOrDest(Unit *parent, Unit *navUnit, QVector nav) {
    static float how_far_to_stop_moving =
            XMLSupport::parse_float(vs_config->getVariable("AI", "how_far_to_stop_navigating", "100"));
    if (navUnit && navUnit->isUnit() != Vega_UnitType::planet) {
        float dist = UnitUtil::getDistance(navUnit, parent);
        if (dist < SIMULATION_ATOM /*simulation_atom_var?*/ * parent->Velocity.Magnitude() * parent->predicted_priority
                * how_far_to_stop_moving) {
            return true;
        }
    }
    return (nav - parent->Position()).MagnitudeSquared() < 4 * parent->rSize() * parent->rSize();
}

volatile Unit *uoif;

class FlyTo : public Orders::MoveTo {
    float creationtime;
    UnitContainer destUnit;
public:
    FlyTo(const QVector &target,
            bool aft,
            bool terminating = true,
            float creationtime = 0,
            int leniency = 6,
            Unit *destUnit = NULL) : MoveTo(target, aft, leniency, terminating) {
        this->creationtime = creationtime;
        this->destUnit = destUnit;
    }

    virtual void Execute() {
        if (parent == uoif) {
            VS_LOG(info, "kewl");
        }
        MoveTo::Execute();
        Unit *un = destUnit.GetUnit();
        if (CloseEnoughToNavOrDest(parent, un, targetlocation)) {
            done = true;
        }
        un = NULL;
        static float mintime = XMLSupport::parse_float(vs_config->getVariable("AI", "min_time_to_auto", "25"));
        if (getNewTime() - creationtime > mintime) {
            if (_Universe->AccessCockpit()->autoInProgress() && (!_Universe->AccessCockpit()->unitInAutoRegion(parent))
                    && (un = ChooseNearNavPoint(parent, destUnit.GetUnit(), targetlocation, 0)) != NULL) {
                WarpToP(parent, un, true);
            } else {
                Unit *playa = _Universe->AccessCockpit()->GetParent();
                if (playa == NULL || playa->Target() != parent || 1) {
                    WarpToP(parent, targetlocation, 0, true);
                }
            }
        }
    }
};

static Vector randVector() {
    return Vector((rand() / (float) RAND_MAX) * 2 - 1,
            (rand() / (float) RAND_MAX) * 2 - 1,
            (rand() / (float) RAND_MAX) * 2 - 1);
}

static void GoTo(AggressiveAI *ai,
        Unit *parent,
        const QVector &nav,
        float creationtime,
        bool boonies = false,
        Unit *destUnit = NULL) {
    static bool can_afterburn = XMLSupport::parse_bool(vs_config->getVariable("AI", "afterburn_to_no_enemies", "true"));
    Order *mt = new FlyTo(nav, can_afterburn, true, creationtime, boonies ? 16 : 6, destUnit);
    Order *ch = new Orders::ChangeHeading(nav, 32, .25f, false);
    ai->eraseType(Order::FACING);
    ai->eraseType(Order::MOVEMENT);
    mt->SetParent(parent);
    ch->SetParent(parent);
    ai->ReplaceOrder(mt);
    ai->EnqueueOrder(ch);
}

void AggressiveAI::ExecuteNoEnemies() {
    static float safetyspacing = XMLSupport::parse_float(vs_config->getVariable("AI", "safetyspacing", "2500"));
    static float randspacingfactor = XMLSupport::parse_float(vs_config->getVariable("AI", "randomspacingfactor", "4"));
    if (nav.i == 0 && nav.j == 0 && nav.k == 0) {
        Unit *otherdest = NULL;
        Unit *dest = ChooseNavPoint(parent, &otherdest, &this->lurk_on_arrival);
        if (dest) {
            static bool
                    can_warp_to = XMLSupport::parse_bool(vs_config->getVariable("AI", "warp_to_no_enemies", "true"));
            static float mintime = XMLSupport::parse_float(vs_config->getVariable("AI", "min_time_to_auto", "25"));
            if (getNewTime() - creationtime > mintime) {
                if (can_warp_to) {
                    WarpToP(parent, dest, true);
                } else if (_Universe->AccessCockpit()->autoInProgress()
                        && !_Universe->AccessCockpit()->unitInAutoRegion(parent)) {
                    WarpToP(parent, dest, true);
                }
            }
            Vector dir = parent->Position() - dest->Position();
            Vector unitdir = dir.Normalize();
            if (!otherdest) {
                navDestination = dest;
                dir = unitdir * (dest->rSize() + parent->rSize());
                if (dest->isUnit() == Vega_UnitType::planet) {
                    float planetpct = UniverseUtil::getPlanetRadiusPercent();
                    dir *= (planetpct + 1.0f);
                    dir += randVector() * parent->rSize() * 2 * randspacingfactor;
                } else {
                    dir *= 2;
                    dir += (unitdir * safetyspacing);
                    dir +=
                            ((randVector() * randspacingfactor
                                    / 4.0f)
                                    + (unitdir
                                            * randspacingfactor))
                                    * ((parent->rSize() > (safetyspacing / 5)) ? (safetyspacing / 5)
                                            : (parent->rSize()));
                }
            }
            nav = dest->Position() + dir;
            if (otherdest) {
                nav += otherdest->Position();
                nav = nav * .5;
            }
#ifdef AGGDEBUG
            std::string fgname    = UnitUtil::getFlightgroupName( parent );
            std::string pfullname = parent->getFullname();
            std::string dfullname = dest->getFullname();
            VS_LOG(debug, (boost::format("%1%:%2% %3% going to %4%:%5%")
                    % parent->name.get().c_str() % pfullname.c_str() % fgname.c_str() % dest->name.get().c_str() % dfullname.c_str()));
            if (otherdest) {
                std::string ofullname = otherdest->getFullname();
                VS_LOG(debug, (boost::format(" between %1%:%2%\n") % otherdest->name.get().c_str() % ofullname.c_str()));
            } else {
                VS_LOG(debug, "\n");
            }
#endif
            GoTo(this, parent, nav, creationtime, otherdest != NULL, otherdest == NULL ? dest : NULL);
        }
    } else {
        if (CloseEnoughToNavOrDest(parent, navDestination.GetUnit(), nav) && lurk_on_arrival == 0) {
            std::string fgname = UnitUtil::getFlightgroupName(parent);

            nav = QVector(0, 0, 0);
            Unit *dest = ChooseNearNavPoint(parent, navDestination.GetUnit(), parent->Position(), parent->rSize());
            if (dest) {
                if (fgname.find(insysString) == string::npos && dest->GetDestinations().size() > 0
                        && UniverseUtil::systemInMemory(dest->GetDestinations()[0])) {
                    parent->ActivateJumpDrive(0);
                    parent->Target(dest);                     //fly there, baby!
                } else if (dest->GetDestinations().size() == 0 && false == UnitUtil::isCapitalShip(parent)
                        && UnitUtil::isDockableUnit(dest)) {
                    Order *ai = parent->aistate;
                    parent->aistate = NULL;
                    parent->PrimeOrders(new Orders::DockingOps(dest, ai, true, false));
                } else {
                    ExecuteNoEnemies();                     //find a new place to go
                }
            } else {
                ExecuteNoEnemies();                 //no suitable docking point found, recursive call which will take door1
                //go dock to the nav point
            }
        } else if (lurk_on_arrival > 0) {
            lurk_on_arrival -= simulation_atom_var;
            //slowdown
            parent->Thrust(-parent->getMass() * parent->UpCoordinateLevel(parent->GetVelocity()) / simulation_atom_var,
                    false);
            parent->graphicOptions.InWarp = 0;
            if (lurk_on_arrival <= 0) {
                nav = QVector(0, 0, 0);
                ExecuteNoEnemies();                 //select new place to go
            }
            //have to do something while here.
        } else {
            GoTo(this, parent, nav, creationtime, false, navDestination.GetUnit());
        }
    }
}

void AggressiveAI::AfterburnerJumpTurnTowards(Unit *target) {
    AfterburnTurnTowards(this, parent);
    static float
            jump_time_limit = XMLSupport::parse_float(vs_config->getVariable("AI", "force_jump_after_time", "120"));
    if (jump_time_check == 0) {
        float dist = (target->Position() - parent->Position()).MagnitudeSquared();
        if (last_jump_distance < dist || last_jump_time > jump_time_limit) {
            //force jump
            last_jump_time = 0;
            if (target->GetDestinations().size()) {
                string dest = target->GetDestinations()[0];
                UnitUtil::JumpTo(parent, dest);
            }
        } else {
            last_jump_distance = dist;
        }
    }
}

volatile Unit *uoi;

void AggressiveAI::Execute() {
    if (parent == uoi) {
        VS_LOG(info, "kewl");
    }
    jump_time_check++;     //just so we get a nicely often wrapping var;
    jump_time_check %= 5;
    Flightgroup *fg = parent->getFlightgroup();
    double firetime = queryTime();
    static int pir = FactionUtil::GetFactionIndex("pirates");
    if (parent->faction == pir) {
        if (rand() == 0) {
            VS_LOG(info, "ahoy, a pirates!");
        }
    }
    FireAt::Execute();
    aggfire += queryTime() - firetime;
    static bool resistance_to_side_movement =
            XMLSupport::parse_bool(vs_config->getVariable("AI", "resistance_to_side_movement", "false"));
    if (resistance_to_side_movement) {
        Vector p, q, r;
        parent->GetOrientation(p, q, r);
        float forwardness = parent->Velocity.Dot(r);
        Vector countervelocity = -parent->Velocity;
        Vector counterforce = -parent->NetForce;
        float forceforwardness = parent->NetForce.Dot(r);
        if (forceforwardness > 0) {
            counterforce = forceforwardness * r - parent->NetForce;
        }
        if (forwardness > 0) {
            countervelocity = forwardness * r - parent->Velocity;
        }
        static float resistance_percent =
                XMLSupport::parse_float(vs_config->getVariable("AI", "resistance_to_side_movement_percent", ".01"));
        static float force_resistance_percent =
                XMLSupport::parse_float(vs_config->getVariable("AI", "resistance_to_side_force_percent", "1"));
        parent->Velocity += countervelocity * resistance_percent;
        parent->NetForce += counterforce * force_resistance_percent;
        counterforce = -parent->NetLocalForce;
        counterforce.k = 0;
        parent->NetLocalForce += counterforce * force_resistance_percent;
    }
    Unit *target = parent->Target();

    bool isjumpable = target ? (!target->GetDestinations().empty()) : false;
    if (!ProcessCurrentFgDirective(fg)) {
        if (isjumpable) {
            if (parent->GetJumpStatus().drive < 0) {
                parent->ActivateJumpDrive(0);
                if (parent->GetJumpStatus().drive == -2) {
                    static bool AIjumpCheat =
                            XMLSupport::parse_bool(vs_config->getVariable("AI",
                                    "always_have_jumpdrive_cheat",
                                    "false"));
                    if (AIjumpCheat) {
                        static int i = 0;
                        if (!i) {
                            VS_LOG(warning, "FIXME: warning ship not equipped to jump");
                            i = 1;
                        }
                        parent->jump.drive = -1;
                    } else {
                        parent->Target(NULL);
                    }
                } else if (parent->GetJumpStatus().drive < 0) {
                    static bool
                            AIjumpCheat = XMLSupport::parse_bool(vs_config->getVariable("AI", "jump_cheat", "true"));
                    if (AIjumpCheat) {
                        parent->jump.drive = 0;
                    }
                }
            }
            last_jump_time += simulation_atom_var;
        } else {
            last_jump_time = 0;
        }
        if ((!isjumpable) && interruptcurtime <= 0 && target) {
            ProcessLogic(*logic, true);
        }
        if (!target) {
            logiccurtime -= simulation_atom_var;
            if (logiccurtime < 0) {
                logiccurtime = 20;
                currentpriority = -FLT_MAX;
            }
        }
        if (queryAny(Order::FACING | Order::MOVEMENT) == NULL) {
            if (isjumpable) {
                AfterburnerJumpTurnTowards(target);
            } else {
                last_jump_distance = FLT_MAX;
                if (target) {
                    ProcessLogic(*logic, false);
                } else {
                }
            }
            if (!isjumpable) {
                ExecuteNoEnemies();
            }
        } else {
            if (target) {
                static bool
                        can_warp_to = XMLSupport::parse_bool(vs_config->getVariable("AI", "warp_to_enemies", "true"));
                if (can_warp_to || _Universe->AccessCockpit()->autoInProgress()) {
                    WarpToP(parent, target, false);
                }
                logiccurtime -= simulation_atom_var;
                interruptcurtime -= simulation_atom_var;
                if (logiccurtime <= 0) {
                    eraseType(Order::FACING);
                    eraseType(Order::MOVEMENT);
                    if (isjumpable) {
                        AfterburnerJumpTurnTowards(target);
                        logiccurtime = logic->maxtime;
                    } else {
                        last_jump_distance = FLT_MAX;
                        ProcessLogic(*logic, false);
                    }
                }
            } else if (queryAny(Order::MOVEMENT) == NULL) {
                ExecuteNoEnemies();
            }
        }
    }
#ifdef AGGDEBUG
    VS_LOG_AND_FLUSH(debug, "endagg");
#endif
    if (getTimeCompression() > 3) {
        float mag = parent->GetVelocity().Magnitude();
        if (mag > .01) {
            mag = 1 / mag;
        }
        parent->SetVelocity(
                parent->GetVelocity() * (mag * parent->GetComputerData().max_speed() / getTimeCompression()));
        parent->NetLocalForce = parent->NetForce = Vector(0, 0, 0);
    }
    target = parent->Target();

    isjumpable = target ? (!target->GetDestinations().empty()) : false;
    if (!isjumpable) {
        if (parent->GetJumpStatus().drive >= 0) {
            parent->ActivateJumpDrive(-1);
        }
    }
}

AggressiveAI *DONOTUSEAG = NULL;

