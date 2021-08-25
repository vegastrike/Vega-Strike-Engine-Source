#ifndef __UNIT_DAMAGE_CPP__
#define __UNIT_DAMAGE_CPP__
#include <string>
#include <vector>
#include "unit.h"
#include "ai/order.h"
#include "gfx/animation.h"
#include "gfx/mesh.h"
#include "gfx/halo.h"
#include "vegastrike.h"
#include "unit_collide.h"
#include <float.h>
#include "audiolib.h"
#include "images.h"
#include "beam.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "savegame.h"
#include "gfx/cockpit.h"
#include "cmd/script/mission.h"
#include "missile.h"
#include "cmd/ai/communication.h"
#include "cmd/script/flightgroup.h"
#include "music.h"
#include "faction_generic.h"
#include "universe_util.h"
#include "csv.h"
#include "unit_csv.h"
#include "base.h"
#include "options.h"
#include "star_system.h"
#include "universe.h"






extern Animation * GetVolatileAni( unsigned int );
extern unsigned int AddAnimation( const QVector&, const float, bool, const string&, float percentgrow );

extern Animation * getRandomCachedAni();
extern string getRandomCachedAniString();
extern void disableSubUnits( Unit *un );

// TODO: move this from here
bool GameUnit::Explode( bool drawit, float timeit )
{
    if (this->pImage->pExplosion == NULL && this->pImage->timeexplode == 0) {
        //no explosion in unit data file && explosions haven't started yet

        //notify the director that a ship got destroyed
        mission->DirectorShipDestroyed( this );
        disableSubUnits( this );
        this->pImage->timeexplode = 0;

        string bleh = this->pImage->explosion_type;
        if ( bleh.empty() )
            FactionUtil::GetRandExplosionAnimation( this->faction, bleh );
        if ( bleh.empty() ) {
            static Animation cache( game_options.explosion_animation.c_str(), false, .1, BILINEAR, false );
            bleh = getRandomCachedAniString();
            if (bleh.size() == 0)
                bleh = game_options.explosion_animation;
        }
        this->pImage->pExplosion = new Animation( bleh.c_str(), game_options.explosion_face_player, .1, BILINEAR, true );
        this->pImage->pExplosion->SetDimensions( this->ExplosionRadius(), this->ExplosionRadius() );
        Vector p, q, r;
        this->GetOrientation( p, q, r );
        this->pImage->pExplosion->SetOrientation( p, q, r );
        if (this->isUnit() != _UnitType::missile) {
            _Universe->activeStarSystem()->AddMissileToQueue( new MissileEffect( this->Position(), this->MaxShieldVal(),
                                                                                 0, this->ExplosionRadius()*game_options.explosion_damage_center,
                                                                                 this->ExplosionRadius()*game_options.explosion_damage_center
                                                                                 *game_options.explosion_damage_edge, NULL ) );
        }
        QVector exploc = this->cumulative_transformation.position;
        bool    sub    = this->isSubUnit();
        Unit   *un     = NULL;
        if (!sub)
            if (( un = _Universe->AccessCockpit( 0 )->GetParent() )) {
                exploc = un->Position()*game_options.explosion_closeness+exploc*(1-game_options.explosion_closeness);
            }
        //AUDPlay( this->sound->explode, exploc, this->Velocity, 1 );
        playExplosionDamageSound();

        if (!sub) {
            un = _Universe->AccessCockpit()->GetParent();
            if (this->isUnit() == _UnitType::unit) {
                if ( rand() < RAND_MAX*game_options.percent_shockwave && ( !this->isSubUnit() ) ) {
                    static string     shockani( game_options.shockwave_animation);
                    static Animation *__shock__ani = new Animation( shockani.c_str(), true, .1, MIPMAP, false );

                    __shock__ani->SetFaceCam( false );
                    unsigned int      which = AddAnimation( this->Position(),
                                                            this->ExplosionRadius(), true, shockani, game_options.shockwave_growth );
                    Animation *ani = GetVolatileAni( which );
                    if (ani) {
                        ani->SetFaceCam( false );
                        Vector p, q, r;
                        this->GetOrientation( p, q, r );
                        int    tmp = rand();
                        if (tmp < RAND_MAX/24)
                            ani->SetOrientation( Vector( 0, 0, 1 ), Vector( 1, 0, 0 ), Vector( 0, 1, 0 ) );
                        else if (tmp < RAND_MAX/16)
                            ani->SetOrientation( Vector( 0, 1, 0 ), Vector( 0, 0, 1 ), Vector( 1, 0, 0 ) );
                        else if (tmp < RAND_MAX/8)
                            ani->SetOrientation( Vector( 1, 0, 0 ), Vector( 0, 1, 0 ), Vector( 0, 0, 1 ) );
                        else
                            ani->SetOrientation( p, q, r );
                    }
                }
                if (un) {
                    int upgradesfaction    = FactionUtil::GetUpgradeFaction();
                    float rel = un->getRelation( this );
                    if (!BaseInterface::CurrentBase) {
                        static float lasttime = 0;
                        float newtime = getNewTime();
                        if ( newtime-lasttime > game_options.time_between_music
                            || (_Universe->isPlayerStarship( this ) && this->isUnit() != _UnitType::missile && this->faction
                                != upgradesfaction) ) {
                            //No victory for missiles or spawned explosions
                            if (rel > game_options.victory_relationship) {
                                lasttime = newtime;
                                muzak->SkipRandSong( Music::LOSSLIST );
                            } else if (rel < game_options.loss_relationship) {
                                lasttime = newtime;
                                muzak->SkipRandSong( Music::VICTORYLIST );
                            }
                        }
                    }
                }
            }
        }
    }
    bool timealldone =
        ( this->pImage->timeexplode > game_options.debris_time || this->isUnit() == _UnitType::missile
         || _Universe->AccessCockpit()->GetParent() == this || this->SubUnits.empty() );
    if (this->pImage->pExplosion) {
        this->pImage->timeexplode += timeit;
        this->pImage->pExplosion->SetPosition( this->Position() );
        Vector p, q, r;
        this->GetOrientation( p, q, r );
        this->pImage->pExplosion->SetOrientation( p, q, r );
        if (this->pImage->pExplosion->Done() && timealldone) {
            delete this->pImage->pExplosion;
            this->pImage->pExplosion = NULL;
        }
        if (drawit && this->pImage->pExplosion)
            this->pImage->pExplosion->Draw();              //puts on draw queue... please don't delete
    }
    bool alldone = this->pImage->pExplosion ? !this->pImage->pExplosion->Done() : false;
    if ( !this->SubUnits.empty() ) {
        Unit *su;
        for (un_iter ui = this->getSubUnits(); (su = *ui); ++ui) {
            bool temp = su->Explode( drawit, timeit );
            if (su->GetImageInformation().pExplosion)
                alldone |= temp;
        }
    }
    if ( (game_options.eject_cargo_on_blowup > 0) && (this->numCargo() > 0) ) {
        unsigned int dropcount =  floorf( this->numCargo()/game_options.eject_cargo_on_blowup )+1;
        if ( dropcount > this->numCargo() ) dropcount = this->numCargo();
        for (unsigned int i = 0; i < dropcount; i++)
            this->EjectCargo( this->numCargo()-1 );              //Ejecting the last one is somewhat faster
    }
    return alldone || (!timealldone);
}

#endif

