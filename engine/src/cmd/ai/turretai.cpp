#include "config_xml.h"
#include "vs_globals.h"
#include "turretai.h"
#include "cmd/unit_generic.h"
#include "cmd/role_bitmask.h"
#include "universe.h"

using namespace Orders;
TurretAI::TurretAI() : FaceTargetITTS( false )
{
    type  |= WEAPON;
    range  = -1;
    speed  = 1;
    mrange = 1;
    hadFired = false;
}
void TurretAI::getAverageGunSpeed( float &speed, float &range, float &mrange ) const
{
    speed  = this->speed;
    range  = this->range;
    mrange = this->mrange;
}
extern unsigned int FireBitmask( Unit *parent, bool shouldfire, bool firemissile );
void TurretAI::Execute()
{
    Unit *targ = parent->Target();
    if (range == -1) {
        range = mrange = speed = 0;
        parent->getAverageGunSpeed( speed, range, mrange );
        float tspeed, trange, tmrange;
        Unit *gun;
        if (parent->getNumMounts() == 0) {
            speed  = 1;
            range  = 1;
            mrange = 1;                             //not much
        }
        for (un_iter i = parent->getSubUnits(); (gun = *i) != NULL; ++i) {
            (*i)->getAverageGunSpeed( tspeed, trange, tmrange );
            if (trange > range) {
                speed  = tspeed;
                range  = trange;
                mrange = tmrange;
            }
        }
        if (range == 0) {
            range = mrange;
            speed = FLT_MAX;
        }
    }
    if (targ) {
        static float dot_cutoff   = XMLSupport::parse_float( vs_config->getVariable( "AI", "Firing", "TurretDotCutoff", ".4" ) );
        static float missile_prob =
            XMLSupport::parse_float( vs_config->getVariable( "AI", "Firing", "TurretMissileProbability", ".05" ) );
        FaceTargetITTS::Execute();
        if (parent->getNumMounts() > 0) {
            Vector  R( parent->GetTransformation().getR() );
            QVector Pos( targ->Position()-parent->Position() );
            double  mag = Pos.Magnitude();
            Pos = Pos/mag;
            float   dot = R.Dot( Pos.Cast() );
            int     neu = FactionUtil::GetNeutralFaction();
            int     upg = FactionUtil::GetUpgradeFaction();
            bool    isplayerstarship = _Universe->isPlayerStarshipVoid( parent->owner ) != NULL;

            bool    shouldfire =
                ( (mag-targ->rSize()-parent->rSize() < range
                   && dot > dot_cutoff)
                 && ( isplayerstarship == false || targ->faction == upg
                     || ( isplayerstarship
                         && (targ->getRelation( (Unit*) parent->owner ) < 0 /*now that it is a player, we know it's dereferencable*/
                             || targ->Target() == (Unit*) parent->owner) ) )
                 && targ->faction != neu );

            //FIXME - rand() is not going to be in the expected range here - stephengtuggy 2020-07-25
            parent->Fire( FireBitmask( parent, shouldfire, rand() < missile_prob*RAND_MAX*SIMULATION_ATOM ), true );
            if (!shouldfire)
                parent->UnFire();
            hadFired = shouldfire;
        }
        if (targ->Destroyed()) {
            parent->Target( nullptr );
        }
    } else if (hadFired && parent->getNumMounts() > 0) {
        // When we get a kill, we must stop firing
        parent->UnFire();
        hadFired = false;
    }
}

