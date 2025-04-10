/*
 * hard_coded_scripts.cpp
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
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/version.hpp>
#include <boost/python.hpp>
#include "src/python/python_class.h"
#include "script.h"
#include "cmd/unit_generic.h"
#include "cmd/mount_size.h"
#include "hard_coded_scripts.h"
#include "flybywire.h"
#include "navigation.h"
#include "tactics.h"
#include "fire.h"
#include "order.h"
#include "src/vs_random.h"
#include "cmd/unit_util.h"
#include "root_generic/configxml.h"
using Orders::FireAt;

BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE

BOOST_PYTHON_END_CONVERSION_NAMESPACE

bool useAfterburner() {
    static bool useafterburner = XMLSupport::parse_bool(vs_config->getVariable("AI", "use_afterburner", "true"));
    return useafterburner;
}

bool useAfterburnerToRun() {
    static bool useafterburner = XMLSupport::parse_bool(vs_config->getVariable("AI", "use_afterburner_to_run", "true"));
    return useafterburner;
}

bool useAfterburnerToFollow() {
    static bool
            useafterburner = XMLSupport::parse_bool(vs_config->getVariable("AI", "use_afterburner_to_follow", "true"));
    return useafterburner;
}

void AddOrd(Order *aisc, Unit *un, Order *ord) {
    ord->SetParent(un);
    aisc->EnqueueOrder(ord);
}

void ReplaceOrd(Order *aisc, Unit *un, Order *ord) {
    ord->SetParent(un);
    aisc->ReplaceOrder(ord);
}

static Order *lastOrder = NULL;

void FireAt::AddReplaceLastOrder(bool replace) {
    if (lastOrder) {
        if (replace) {
            ReplaceOrd(this, parent, lastOrder);
        } else {
            AddOrd(this, parent, lastOrder);
        }
        lastOrder = NULL;
    }
}

void FireAt::ExecuteLastScriptFor(float time) {
    if (lastOrder) {
        lastOrder = new ExecuteFor(lastOrder, time);
    }
}

void FireAt::FaceTarget(bool end) {
    lastOrder = new Orders::FaceTarget(end, 4);
}

void FireAt::FaceTargetITTS(bool end) {
    lastOrder = new Orders::FaceTargetITTS(end, 4);
}

void FireAt::MatchLinearVelocity(bool terminate, Vector vec, bool afterburn, bool local) {
    afterburn = afterburn && useAfterburner();
    lastOrder = new Orders::MatchLinearVelocity(parent->ClampVelocity(vec, afterburn), local, afterburn, terminate);
}

void FireAt::MatchAngularVelocity(bool terminate, Vector vec, bool local) {
    lastOrder = new Orders::MatchAngularVelocity(parent->ClampAngVel(vec), local, terminate);
}

void FireAt::ChangeHeading(QVector vec) {
    lastOrder = new Orders::ChangeHeading(vec, 3);
}

void FireAt::ChangeLocalDirection(Vector vec) {
    lastOrder = new Orders::ChangeHeading(((parent->Position().Cast()) + parent->ToWorldCoordinates(vec)).Cast(), 3);
}

void FireAt::MoveTo(QVector vec, bool afterburn) {
    afterburn = afterburn && useAfterburner();
    lastOrder = new Orders::MoveTo(vec, afterburn, 3);
}

void FireAt::MatchVelocity(bool terminate, Vector vec, Vector angvel, bool afterburn, bool local) {
    afterburn = afterburn && useAfterburner();
    lastOrder = new Orders::MatchVelocity(parent->ClampVelocity(vec, afterburn), parent->ClampAngVel(
            angvel), local, afterburn, terminate);
}

void FireAt::Cloak(bool enable, float seconds) {
    lastOrder = new CloakFor(enable, seconds);
}

void FireAt::FormUp(QVector pos) {
    lastOrder = new Orders::FormUp(pos);
}

void FireAt::FormUpToOwner(QVector pos) {
    lastOrder = new Orders::FormUpToOwner(pos);
}

void FireAt::FaceDirection(float distToMatchFacing, bool finish) {
    lastOrder = new Orders::FaceDirection(distToMatchFacing, finish, 3);
}

void FireAt::XMLScript(string script) {
    lastOrder = new AIScript(script.c_str());
}

void FireAt::LastPythonScript() {
    lastOrder = PythonAI<Orders::FireAt>::LastPythonClass();
}

class EvadeLeftRightC : public FlyByWire {
    bool updown;
    Vector facing;
    bool dir;
public:
    EvadeLeftRightC(bool updown) {
        this->updown = updown;
        facing = Vector(0, 0, 0);
        desired_ang_velocity = Vector(0, 0, 0);
        dir = (rand() < RAND_MAX / 2);
    }

    void SetOppositeDir() {
        dir = !dir;
        SetDesiredAngularVelocity(Vector(0.0f, 0.0f, 0.0f), true);
        if (updown) {
            Up(dir ? 1.0f : -1.0f);
        } else {
            Right(dir ? 1.0f : -1.0f);
        }
        Vector P = Vector(0, 0, 0), Q = Vector(0, 0, 0);
        parent->GetOrientation(P, Q, facing);
    }

    virtual void SetParent(Unit *parent1) {
        FlyByWire::SetParent(parent1);
        SetOppositeDir();
    }

    void Execute() {
        FlyByWire::Execute();
        Vector P, Q, R;
        parent->GetOrientation(P, Q, R);
        static float ang = cos(XMLSupport::parse_float(vs_config->getVariable("AI", "evasion_angle", "45")));
        if (R.Dot(facing) < ang
                || (desired_ang_velocity.i == 0 && desired_ang_velocity.j == 0 && desired_ang_velocity.k == 0)) {
            SetOppositeDir();
        }
    }
};

//these can be used in the XML scripts if they are allowed to be called

//these can be used in the XML scripts if they are allowed to be called

void AfterburnTurnTowards(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    bool afterburn = useAfterburnerToFollow();
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, afterburn), true, afterburn, false);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTarget(false, 3));
    AddOrd(aisc, un, ord);
}

void AfterburnTurnTowardsITTS(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);

    bool afterburn = useAfterburnerToFollow();
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, afterburn), true, afterburn, false);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void BarrelRoll(Order *aisc, Unit *un) {
    FlyByWire *broll = new FlyByWire;
    AddOrd(aisc, un, broll);
    broll->RollRight(rand() > RAND_MAX / 2 ? 1 : -1);
    float per;
    if (rand() < RAND_MAX / 2) {
        per = ((float) rand()) / RAND_MAX;
        if (per < .5) {
            per -= 1;
        }
        broll->Up(per);
    } else {
        per = ((float) rand()) / RAND_MAX;
        if (per < .5) {
            per -= 1;
        }
        broll->Right(per);
    }
    bool afterburn = useAfterburner();
    broll->MatchSpeed(Vector(0,
            0,
            afterburn ? un->MaxAfterburnerSpeed() : un->MaxSpeed()));
    broll->Afterburn(afterburn);
}

static void EvadeWavy(Order *aisc, Unit *un, bool updown, bool ab) {
    EvadeLeftRightC *broll = NULL;
    broll = new EvadeLeftRightC(updown);
    AddOrd(aisc, un, broll);
    bool afterburn = ab && useAfterburner();
    broll->MatchSpeed(Vector(0,
            0,
            afterburn ? un->MaxAfterburnerSpeed() : un->MaxSpeed()));
    broll->Afterburn(afterburn);
}

void AfterburnEvadeLeftRight(Order *aisc, Unit *un) {
    EvadeWavy(aisc, un, false, true);
}

void AfterburnEvadeUpDown(Order *aisc, Unit *un) {
    EvadeWavy(aisc, un, true, true);
}

void EvadeLeftRight(Order *aisc, Unit *un) {
    EvadeWavy(aisc, un, false, false);
}

void EvadeUpDown(Order *aisc, Unit *un) {
    EvadeWavy(aisc, un, true, false);
}

namespace Orders {
class LoopAround : public Orders::FaceTargetITTS {
    Orders::MatchLinearVelocity m;
    float qq;
    float pp;
    Vector rr;    //place to go for @ end1111
    bool afterburn;
    bool aggressive;
    bool force_afterburn;
public:
    void SetParent(Unit *parent1) {
        FaceTargetITTS::SetParent(parent1);
        m.SetParent(parent1);
    }

    LoopAround(bool aggressive, bool afterburn, bool force_afterburn, int seed) : FaceTargetITTS(false, 3),
            m(Vector(0, 0, 1000),
                    true,
                    afterburn,
                    false) {
        VSRandom vsr(seed);
        this->aggressive = aggressive;
        this->afterburn = afterburn;
        this->force_afterburn = force_afterburn;

        static float loopdis = XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_distance", "1"));
        qq = pp = 0;
        static float loopdisd =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_distance", "20.0"));
        static float loopdisv =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_vertical", "4.0"));
        static float loopdisl =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_lateral", "4.0"));
        rr.Set(loopdisl * vsr.uniformInc(-1, 1),
                loopdisv * vsr.uniformInc(-1, 1),
                1.0 + loopdisd * vsr.uniformInc(0, 1));
        if (vsr.rand() < VS_RAND_MAX / 2) {
            qq = vsr.uniformInc(-1, 1);
            rr.j = qq;
            if (qq > 0) {
                qq += loopdis;
            }
            if (qq < 0) {
                qq -= loopdis;
            }
        } else {
            pp = vsr.uniformInc(-1, 1);
            rr.i = pp;
            if (pp > 0) {
                pp += loopdis;
            }
            if (pp < 0) {
                pp -= loopdis;
            }
        }
    }

    void Execute() {
        Unit *targ = parent->Target();
        if (targ) {
            Vector relloc = parent->Position() - targ->Position();
            Vector r = targ->cumulative_transformation_matrix.getR();
            bool afterburn = useAfterburner() && this->afterburn;
            bool ab_needed =
                    force_afterburn || targ->GetVelocity().MagnitudeSquared() > parent->MaxSpeed();
            m.SetDesiredVelocity(Vector(0, 0, afterburn
                    && ab_needed ? parent->MaxAfterburnerSpeed()
                    : parent->MaxSpeed()), true);
            float spseed, grange = 0, mrange = 0;
            parent->getAverageGunSpeed(spseed, grange, mrange);
            if (r.Dot(relloc) < 0) {
                static float gun_range_pct =
                        XMLSupport::parse_float(vs_config->getVariable("AI", "gun_range_percent_ok", ".66"));
                FaceTargetITTS::Execute();
                float dist = UnitUtil::getDistance(parent, targ);
                if (dist < grange * gun_range_pct || (grange == 0 && dist < mrange)) {
                    static float velocity_adjustment_pct =
                            XMLSupport::parse_float(vs_config->getVariable("AI",
                                    "loop_around_pursuit_velocity_percent",
                                    ".9"));
                    m.SetDesiredVelocity(Vector(0, 0, targ->cumulative_velocity.Magnitude() * velocity_adjustment_pct),
                            true);
                }
                m.SetAfterburn(afterburn && ab_needed);
                m.Execute();
            } else {
                done = false;
                if (afterburn) {
                    m.SetAfterburn(ab_needed);
                } else {
                    m.SetAfterburn(0);
                }
                Vector scala =
                        targ->cumulative_transformation_matrix.getQ().Scale(qq
                                * (parent->rSize()
                                        + targ->rSize()))
                                + targ->cumulative_transformation_matrix
                                        .getP()
                                        .Scale(pp * (parent->rSize() + targ->rSize()));
                QVector dest = targ->Position() + scala;
                SetDest(dest);
                ChangeHeading::Execute();
                m.Execute();
            }
        }
    }
};

class LoopAroundAgro : public Orders::FaceTargetITTS {
    Orders::MoveToParent m;
    float qq;
    float pp;
    Vector rr;    //place to go for @ end
    bool aggressive;
    bool afterburn;
    bool force_afterburn;
public:
    LoopAroundAgro(bool aggressive, bool afterburn, bool force_afterburn, int seed) : FaceTargetITTS(false, 3),
            m(false, 2, false) {
        VSRandom vsr(seed);
        this->afterburn = afterburn;
        this->force_afterburn = force_afterburn;
        this->aggressive = aggressive;
        static float loopdis = XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_distance", "1"));
        qq = pp = 0;
        static float loopdisd =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_distance", "20.0"));
        static float loopdisv =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_vertical", "4.0"));
        static float loopdisl =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_lateral", "4.0"));
        rr.Set(loopdisl * vsr.uniformInc(-1, 1),
                loopdisv * vsr.uniformInc(-1, 1),
                1.0 + loopdisd * vsr.uniformInc(0, 1));
        if (vsr.rand() < VS_RAND_MAX / 2) {
            qq = vsr.uniformInc(-1, 1);
            rr.j = qq;
            if (qq > 0) {
                qq += loopdis;
            }
            if (qq < 0) {
                qq -= loopdis;
            }
        } else {
            pp = vsr.uniformInc(-1, 1);
            rr.i = pp;
            if (pp > 0) {
                pp += loopdis;
            }
            if (pp < 0) {
                pp -= loopdis;
            }
        }
    }

    void Execute() {
        Unit *targ = parent->Target();
        if (targ) {
            Vector relloc = parent->Position() - targ->Position();
            Vector r = targ->cumulative_transformation_matrix.getR();
            bool afterburn = useAfterburner() && this->afterburn;
            bool ab_needed =
                    force_afterburn || targ->GetVelocity().MagnitudeSquared() > parent->MaxSpeed();
            if (r.Dot(relloc) < 0) {
                FaceTargetITTS::Execute();
                m.SetAfterburn(afterburn && ab_needed);
                m.Execute(parent, targ->Position() - r.Scale(
                        rr.k * parent->rSize()
                                + targ->rSize()) + targ->cumulative_transformation_matrix.getP()
                        * (rr.i * parent->rSize())
                        + targ->cumulative_transformation_matrix.getQ() * (rr.j * parent->rSize()));
            } else {
                done = false;
                if (afterburn) {
                    m.SetAfterburn(ab_needed);
                } else {
                    m.SetAfterburn(0);
                }
                Vector scala =
                        targ->cumulative_transformation_matrix.getQ().Scale(qq
                                * (parent->rSize()
                                        + targ->rSize()))
                                + targ->cumulative_transformation_matrix
                                        .getP()
                                        .Scale(pp * (parent->rSize() + targ->rSize()));
                QVector dest = targ->Position() + scala;
                if (aggressive) {
                    FaceTargetITTS::Execute();
                } else {
                    SetDest(dest);
                    ChangeHeading::Execute();
                }
                m.Execute(parent, dest + scala);
            }
        }
    }
};

class FacePerpendicular : public Orders::FaceTargetITTS {
    Orders::MatchLinearVelocity m;
    float qq;
    float pp;
    Vector rr;    //place to go for @ end1111
    bool afterburn;
    bool aggressive;
    bool force_afterburn;
public:
    void SetParent(Unit *parent1) {
        FaceTargetITTS::SetParent(parent1);
        m.SetParent(parent1);
    }

    FacePerpendicular(bool aggressive, bool afterburn, bool force_afterburn, int seed) : FaceTargetITTS(false, 3),
            m(Vector(0, 0, 1000),
                    true,
                    afterburn,
                    false) {
        this->aggressive = aggressive;
        VSRandom vsr(seed);
        this->afterburn = afterburn;
        this->force_afterburn = force_afterburn;

        static float loopdis = XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_distance", "1"));
        qq = pp = 0;
        static float loopdisd =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_distance", "20.0"));
        static float loopdisv =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_vertical", "4.0"));
        static float loopdisl =
                XMLSupport::parse_float(vs_config->getVariable("AI", "loop_around_destination_lateral", "4.0"));
        rr.Set(loopdisl * vsr.uniformInc(-1, 1),
                loopdisv * vsr.uniformInc(-1, 1),
                1.0 + loopdisd * vsr.uniformInc(0, 1));
        if (vsr.rand() < VS_RAND_MAX / 2) {
            qq = vsr.uniformInc(-1, 1);
            rr.j = qq;
            if (qq > 0) {
                qq += loopdis;
            }
            if (qq < 0) {
                qq -= loopdis;
            }
        } else {
            pp = vsr.uniformInc(-1, 1);
            rr.i = pp;
            if (pp > 0) {
                pp += loopdis;
            }
            if (pp < 0) {
                pp -= loopdis;
            }
        }
    }

    void Execute() {
        static float
                gun_range_pct = XMLSupport::parse_float(vs_config->getVariable("AI", "gun_range_percent_ok", ".66"));
        Unit *targ = parent->Target();
        if (targ) {
            Vector relloc = parent->Position() - targ->Position();
            Vector r = targ->cumulative_transformation_matrix.getR();
            bool afterburn = useAfterburner() && this->afterburn;
            bool ab_needed =
                    force_afterburn || targ->GetVelocity().MagnitudeSquared() > parent->MaxSpeed();
            m.SetDesiredVelocity(Vector(0, 0, afterburn
                    && ab_needed ? parent->MaxAfterburnerSpeed()
                    : parent->MaxSpeed()), true);
            float speed, grange = 0, mrange = 0;
            parent->getAverageGunSpeed(speed, grange, mrange);
            if (r.Dot(relloc) < 0) {
                FaceTargetITTS::Execute();
                float dist = UnitUtil::getDistance(parent, targ);
                if (dist < grange * gun_range_pct || (grange == 0 && dist < mrange)) {
                    static float velocity_adjustment_pct =
                            XMLSupport::parse_float(vs_config->getVariable("AI",
                                    "loop_around_pursuit_velocity_percent",
                                    ".9"));
                    m.SetDesiredVelocity(Vector(0, 0, targ->cumulative_velocity.Magnitude() * velocity_adjustment_pct),
                            true);
                }
                m.SetAfterburn(afterburn && ab_needed);
                m.Execute();
            } else {
                done = false;
                if (afterburn) {
                    m.SetAfterburn(ab_needed);
                } else {
                    m.SetAfterburn(0);
                }
                Vector scala = targ->cumulative_transformation_matrix.getR().Cross(
                        aggressive ? parent->cumulative_transformation_matrix.getQ() : Vector(.01, .99,
                                -.001)) * parent->rSize()
                        * 100.;
                QVector dest = parent->Position() + scala;
                SetDest(dest);
                ChangeHeading::Execute();
                m.Execute();
            }
        }
    }
};
}

void LoopAround(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAround(false, true, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void AggressiveLoopAround(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAroundAgro(true, true, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void RollLeft(Order *aisc, Unit *un) {
    if (un->aistate) {
        AddOrd(un->aistate,
                un,
                new Orders::ExecuteFor(new Orders::MatchRoll(un->drive.max_roll_right, false), 1.0f));
    }
}

void RollRight(Order *aisc, Unit *un) {
    if (un->aistate) {
        AddOrd(un->aistate,
                un,
                new Orders::ExecuteFor(new Orders::MatchRoll(-un->drive.max_roll_left, false), 1.0f));
    }
}

void RollLeftHard(Order *aisc, Unit *un) {
    static float durvar = XMLSupport::parse_float(vs_config->getVariable("AI", "roll_order_duration", "5.0"));
    if (un->aistate) {
        AddOrd(un->aistate,
                un,
                new Orders::ExecuteFor(new Orders::MatchRoll(un->drive.max_roll_right, false), durvar));
    }
}

void RollRightHard(Order *aisc, Unit *un) {
    static float durvar = XMLSupport::parse_float(vs_config->getVariable("AI", "roll_order_duration", "5.0"));
    if (un->aistate) {
        AddOrd(un->aistate,
                un,
                new Orders::ExecuteFor(new Orders::MatchRoll(-un->drive.max_roll_left, false), durvar));
    }
}

void LoopAroundFast(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAround(false, true, true, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void FacePerpendicularFast(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(false, true, true, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void FacePerpendicular(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(false, true, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void FacePerpendicularSlow(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(false, false, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void RollFacePerpendicularFast(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(true, true, true, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void RollFacePerpendicular(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(true, true, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void RollFacePerpendicularSlow(Order *aisc, Unit *un) {
    Order *broll = new Orders::FacePerpendicular(true, false, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void AggressiveLoopAroundFast(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAroundAgro(true, true, true, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void LoopAroundSlow(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAround(false, false, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

void SelfDestruct(Order *aisc, Unit *un) {
    VS_LOG_AND_FLUSH(trace, "hard_coded_scripts::SelfDestruct " + un->name);
    un->Destroy();
    un->Split(rand() % 3 + 1);
    un->Explode(true, 0);     //displays explosion, unit continues
    un->RemoveFromSystem();      //has no effect
}

void AggressiveLoopAroundSlow(Order *aisc, Unit *un) {
    Order *broll = new Orders::LoopAroundAgro(true, false, false, (int) (size_t) un);
    AddOrd(aisc, un, broll);
}

#if 0
void Evade( Order *aisc, Unit *un )
{
    QVector v( un->Position() );
    QVector u( v );
    Unit   *targ = un->Target();
    if (targ)
        u = targ->Position();
    Order  *ord  = new Orders::ChangeHeading( ( 200*(v-u) )+v, 3 );
    AddOrd( aisc, un, ord );
    bool    afterburn = useAfterburner();
    ord = new Orders::MatchLinearVelocity( un->ClampVelocity( Vector( -10000, 0, 10000 ), afterburn ), false, afterburn, true );
    AddOrd( aisc, un, ord );
    ord = new Orders::FaceTargetITTS( false, 3 );
    AddOrd( aisc, un, ord );
    ord = new Orders::MatchLinearVelocity( un->ClampVelocity( Vector( 10000, 0, 10000 ), afterburn ), false, afterburn, true );
    AddOrd( aisc, un, ord );
}
#endif

void MoveTo(Order *aisc, Unit *un) {
    QVector Targ(un->Position());
    Unit *untarg = un->Target();
    if (untarg) {
        Targ = untarg->Position();
    }
    Order *ord = new Orders::MoveTo(Targ, false, 3);
    AddOrd(aisc, un, ord);
}

void KickstopBase(Order *aisc, Unit *un, bool match) {
    Vector vec(0, 0, 0);
    if (match && un->Target()) {
        vec = un->Target()->GetVelocity();
    }
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, true);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void Kickstop(Order *aisc, Unit *un) {
    KickstopBase(aisc, un, false);
}

void CoastToStop(Order *aisc, Unit *un) {
    Vector vec(0, 0, 0);
    vec = un->GetVelocity();

    vec.i = vec.i * 0.1;
    vec.j = vec.j * 0.1;
    vec.k = vec.k * 0.1;

    Order *ord =
            new Orders::ExecuteFor(new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, true),
                    1);
    AddOrd(aisc, un, ord);
}

void DoNothing(Order *aisc, Unit *un) {
    Vector vec(0, 0, 0);
    vec = un->GetVelocity();

    Order *ord = new Orders::ExecuteFor(new Orders::MatchLinearVelocity(vec, true, false, true), 1);
    AddOrd(aisc, un, ord);
}

void MatchVelocity(Order *aisc, Unit *un) {
    KickstopBase(aisc, un, true);
}

static Vector VectorThrustHelper(Order *aisc, Unit *un, bool ab = false) {
    Vector vec(0, 0, 0);
    Vector retval(0, 0, 0);
    if (un->Target()) {
        Vector tpos = un->Target()->Position().Cast();
        Vector relpos = tpos - un->Position().Cast();
        CrossProduct(relpos, Vector(1, 0, 0), vec);
        retval += tpos;
    }
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, ab), false, ab, true);
    AddOrd(aisc, un, ord);
    return retval;
}

void VeerAway(Order *aisc, Unit *un) {
    VectorThrustHelper(aisc, un);
    Order *ord = (new Orders::FaceTarget(false, 3));
    AddOrd(aisc, un, ord);
}

void VeerAwayITTS(Order *aisc, Unit *un) {
    VectorThrustHelper(aisc, un);
    Order *ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void VeerAndVectorAway(Order *aisc, Unit *un) {
    Vector retval = VectorThrustHelper(aisc, un);
    Order *ord = new Orders::ChangeHeading(retval, 3, 1);
    AddOrd(aisc, un, ord);
}

void AfterburnVeerAndVectorAway(Order *aisc, Unit *un) {
    Vector retval = VectorThrustHelper(aisc, un, true);
    Order *ord = new Orders::ChangeHeading(retval, 3, 1);
    AddOrd(aisc, un, ord);
}

void AfterburnVeerAndTurnAway(Order *aisc, Unit *un) {
    Vector vec = Vector(0, 0, 1);
    bool ab = true;
    Vector tpos = un->Position().Cast();
    if (un->Target()) {
        tpos = un->Target()->Position().Cast();
        Vector relpos = tpos - un->Position().Cast();
        CrossProduct(relpos, Vector(1, 0, 0), vec);
    }
    Order *ord = new Orders::ExecuteFor(new Orders::MatchLinearVelocity(Vector(0, 0, 0), true, ab, true), .5);

    AddOrd(aisc, un, ord);
    ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Vector(0, 0, 100000), ab), true, ab, true);
    aisc->EnqueueOrder(ord);
    ord = new Orders::ChangeHeading(tpos + vec, 3, 1);
    AddOrd(aisc, un, ord);
}

static void SetupVAndTargetV(QVector &targetv, QVector &targetpos, Unit *un) {
    Unit *targ;
    if ((targ = un->Target())) {
        targetv = targ->GetVelocity().Cast();
        targetpos = targ->Position();
    }
}

void SheltonSlide(Order *aisc, Unit *un) {
    QVector def(un->Position() + QVector(1, 0, 0));
    QVector targetv(def);
    QVector targetpos(def);
    SetupVAndTargetV(targetpos, targetv, un);
    QVector difference = targetpos - un->Position();
    QVector perp = targetv.Cross(-difference);
    perp.Normalize();
    perp = perp * (targetv.Dot(difference * -1. / (difference.Magnitude())));
    perp = (perp + difference) * 10000.;

    bool afterburn = useAfterburner();
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp.Cast(), afterburn), false, afterburn, true);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void AfterburnerSlide(Order *aisc, Unit *un) {
    QVector def = un->Position() + QVector(1, 0, 0);
    QVector targetv(def);
    QVector targetpos(def);
    SetupVAndTargetV(targetpos, targetv, un);

    QVector difference = targetpos - un->Position();
    QVector perp = targetv.Cross(-difference);
    perp.Normalize();
    perp = perp * (targetv.Dot(difference * -1. / (difference.Magnitude())));
    perp = (perp + difference) * 1000;
    bool afterburn = useAfterburner();
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(perp.Cast(), afterburn), false, afterburn, true);
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new Orders::ChangeHeading(perp + un->Position(), 3), 1.5);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void SkilledABSlide(Order *aisc, Unit *un) {
    QVector def = un->Position() + QVector(1, 0, 0);
    QVector targetv(def);
    QVector targetpos(def);
    SetupVAndTargetV(targetpos, targetv, un);

    QVector difference = targetpos - un->Position();
    QVector ndifference = difference;
    ndifference.Normalize();
    QVector Perp;
    ScaledCrossProduct(ndifference, targetv, Perp);
    Perp = Perp + .5 * ndifference;
    Perp = Perp * 10000;

    bool afterburn = useAfterburner();

    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(Perp.Cast(), afterburn), false, afterburn, true);
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new Orders::ChangeHeading(Perp + un->Position(), 3), .5);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(false, 3));
    AddOrd(aisc, un, ord);
}

void Stop(Order *aisc, Unit *un) {
    Vector vec(0, 0, 0000);
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
    AddOrd(aisc, un, ord);     //<!-- should we fini? -->
}

void AfterburnTurnAway(Order *aisc, Unit *un) {
    QVector v(un->Position());
    QVector u(v);
    Unit *targ = un->Target();
    if (targ) {
        u = targ->Position();
    }
    bool afterburn = useAfterburner() || useAfterburnerToRun();
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(200 * (v - u).Cast(), afterburn),
            false,
            afterburn,
            false);
    AddOrd(aisc, un, ord);
    ord = new Orders::ChangeHeading((200 * (v - u)) + v, 3);
    AddOrd(aisc, un, ord);
}

void TurnAway(Order *aisc, Unit *un) {
    QVector v(un->Position());
    QVector u(v);
    Unit *targ = un->Target();
    if (targ) {
        u = targ->Position();
    }
    bool afterburn = false;
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(200 * (v - u).Cast(), afterburn),
            false,
            afterburn,
            false);
    AddOrd(aisc, un, ord);
    ord = new Orders::ChangeHeading((200 * (v - u)) + v, 3);
    AddOrd(aisc, un, ord);
}

void TurnTowards(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
    AddOrd(aisc, un, ord);

    ord = new Orders::FaceTarget(0, 3);
    AddOrd(aisc, un, ord);
}

void FlyStraight(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    Order *ord = new Orders::MatchVelocity(un->ClampVelocity(vec, false), Vector(0, 0, 0), true, false, false);
    AddOrd(aisc, un, ord);
    ord = new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false);
    AddOrd(aisc, un, ord);
}

void FlyStraightAfterburner(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    bool afterburn = useAfterburner();
    Order *ord = new Orders::MatchVelocity(un->ClampVelocity(vec, afterburn), Vector(0, 0, 0), true, afterburn, false);
    AddOrd(aisc, un, ord);
    ord = new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false);
    AddOrd(aisc, un, ord);
}

//spiritplumber was here and added some orders, mostly for carriers and their spawn (eep, can carriers have kids?)

void Takeoff(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    static bool firsttime = true;
    Order *ord;
    if (firsttime) {
        un->UnFire();
        ord = new Orders::MatchVelocity(un->ClampVelocity(vec, true), Vector(0, 0, 0), true, true, false);
        AddOrd(aisc, un, ord);
        ord = new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false);
        AddOrd(aisc, un, ord);
        ord = new Orders::ExecuteFor(new Orders::MatchVelocity(un->ClampVelocity(vec, true),
                Vector(0,
                        0,
                        0),
                true,
                true,
                false), 1.5f);
        AddOrd(aisc, un, ord);
        ord = new Orders::ExecuteFor(new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false), 1.5f);
        AddOrd(aisc, un, ord);
        ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
        AddOrd(aisc, un, ord);
        ord = (new Orders::FaceTargetITTS(0, 3));
        AddOrd(aisc, un, ord);
        un->UnFire();
        firsttime = false;
    } else {
        firsttime = false;
        ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
        AddOrd(aisc, un, ord);
        ord = (new Orders::FaceTargetITTS(0, 3));
        AddOrd(aisc, un, ord);
    }
    ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(0, 3));
    AddOrd(aisc, un, ord);
    un->SelectAllWeapon(false);
    un->Fire(as_integer(MOUNT_SIZE::LIGHT) | as_integer(MOUNT_SIZE::MEDIUM), false);
    TurnTowards(aisc, un);
}

void TakeoffEveryZig(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);

    Order *ord;
    un->UnFire();
    un->Fire(as_integer(MOUNT_SIZE::CAPSHIPHEAVY), false);
    un->UnFire();
    ord = new Orders::MatchVelocity(un->ClampVelocity(vec, true), Vector(0, 0, 0), true, true, false);
    AddOrd(aisc, un, ord);
    ord = new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false);
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false), 3.4f);
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new Orders::MatchVelocity(un->ClampVelocity(vec, true), Vector(0,
                    0,
                    0), true, true, false),
            0.1f);
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new Orders::FaceTarget(0, 3), 1.0f);
    AddOrd(aisc, un, ord);
    ord = new Orders::MatchVelocity(un->ClampVelocity(vec, true), Vector(0, 0, 0), true, true, false);
    AddOrd(aisc, un, ord);
    ord = new Orders::MatchAngularVelocity(Vector(0, 0, 0), 1, false);
    AddOrd(aisc, un, ord);
    un->Fire(as_integer(MOUNT_SIZE::CAPSHIPHEAVY), false);     //doesn't have time to happen anyway
    un->SelectAllWeapon(false);
}

void CloakForScript(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(0, 3));
    AddOrd(aisc, un, ord);
    ord = new Orders::ExecuteFor(new CloakFor(1, 8), 32);
    AddOrd(aisc, un, ord);
}

void TurnTowardsITTS(Order *aisc, Unit *un) {
    Vector vec(0, 0, 10000);
    Order *ord = new Orders::MatchLinearVelocity(un->ClampVelocity(vec, false), true, false, false);
    AddOrd(aisc, un, ord);
    ord = (new Orders::FaceTargetITTS(0, 3));
    AddOrd(aisc, un, ord);
}

void DropCargo(Order *aisc, Unit *un) {
    if (un->numCargo() > 0) {
        int dropcount = un->numCargo();

        {
            for (int i = 0; i < dropcount; i++) {
                un->EjectCargo(0);
            }
        }
        Stop(aisc, un);
    } else {
        TurnAway(aisc, un);
    }
}

void DropHalfCargo(Order *aisc, Unit *un) {
    if (un->numCargo() > 0) {
        int dropcount = (un->numCargo() / 2) + 1;

        {
            for (int i = 0; i < dropcount; i++) {
                un->EjectCargo(0);
            }
        }
        Stop(aisc, un);
    } else {
        TurnAway(aisc, un);
    }
}

void DropOneCargo(Order *aisc, Unit *un) {
    if (un->numCargo() > 0) {
        un->EjectCargo(0);
        Stop(aisc, un);
    } else {
        TurnAway(aisc, un);
    }
}

