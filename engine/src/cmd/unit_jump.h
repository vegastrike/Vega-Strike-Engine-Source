/*
 * unit_jump.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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


#ifndef __UNIT_JUMP_CPP__
#define __UNIT_JUMP_CPP__

#include "unit.h"
#include "audiolib.h"
#include "star_system_generic.h"
#include "cmd/images.h"
#include "vs_logging.h"


//From star_system_jump.cpp
extern Hashtable< std::string, StarSystem, 127 >star_system_table;
extern std::vector< unorigdest* >pendingjump;

//From star_system_jump.cpp
inline bool CompareDest( Unit *un, StarSystem *origin )
{
    for (unsigned int i = 0; i < un->GetDestinations().size(); i++)
        if ( std::string( origin->getFileName() ) == std::string( un->GetDestinations()[i] ) )
            return true;
    return false;
}

inline std::vector< Unit* >ComparePrimaries( Unit *primary, StarSystem *origin )
{
    std::vector< Unit* >myvec;
    if ( CompareDest( primary, origin ) )
        myvec.push_back( primary );
    return myvec;
}

extern void DealPossibleJumpDamage( Unit *un );
extern void ActivateAnimation( Unit* );
void WarpPursuit( Unit *un, StarSystem *sourcess, std::string destination );


bool GameUnit::TransferUnitToSystem( unsigned int kk, StarSystem* &savedStarSystem, bool dosightandsound )
{
    bool ret = false;
    if (pendingjump[kk]->orig == this->activeStarSystem || this->activeStarSystem == NULL) {
        if ( Unit::TransferUnitToSystem( pendingjump[kk]->dest ) ) {
            ///eradicating from system, leaving no trace
            ret = true;

            Unit *unit;
            for (un_iter iter = pendingjump[kk]->orig->getUnitList().createIterator(); (unit = *iter); ++iter) {
                if (unit->Threat() == this)
                    unit->Threaten( NULL, 0 );
                if (unit->VelocityReference() == this)
                    unit->VelocityReference( NULL );
                if (unit->Target() == this) {
                    if ( pendingjump[kk]->jumppoint.GetUnit() ) {
                        unit->Target( pendingjump[kk]->jumppoint.GetUnit() );
                        unit->ActivateJumpDrive( 0 );
                    } else {
                        WarpPursuit( unit, pendingjump[kk]->orig, pendingjump[kk]->dest->getFileName() );
                    }
                } else {
                    Flightgroup *ff = unit->getFlightgroup();
                    if (ff)
                        if ( this == ff->leader.GetUnit() && (ff->directive == "f" || ff->directive == "F") ) {
                            unit->Target( pendingjump[kk]->jumppoint.GetUnit() );
                            unit->getFlightgroup()->directive = "F";
                            unit->ActivateJumpDrive( 0 );
                        }
                }
            }
            if ( this == _Universe->AccessCockpit()->GetParent() ) {
                VS_LOG(info, "Unit is the active player character...changing scene graph\n");
                savedStarSystem->SwapOut();
                AUDStopAllSounds();
                savedStarSystem = pendingjump[kk]->dest;
                pendingjump[kk]->dest->SwapIn();
            }
            _Universe->setActiveStarSystem( pendingjump[kk]->dest );
            vector< Unit* >possibilities;
            Unit *primary;
            if (pendingjump[kk]->final_location.i == 0
                && pendingjump[kk]->final_location.j == 0
                && pendingjump[kk]->final_location.k == 0)
                for (un_iter iter = pendingjump[kk]->dest->getUnitList().createIterator(); (primary = *iter); ++iter) {
                    vector< Unit* >tmp;
                    tmp = ComparePrimaries( primary, pendingjump[kk]->orig );
                    if ( !tmp.empty() )
                        possibilities.insert( possibilities.end(), tmp.begin(), tmp.end() );
                }
            else
                this->SetCurPosition( pendingjump[kk]->final_location );
            if ( !possibilities.empty() ) {
                static int jumpdest = 235034;
                Unit *jumpnode = possibilities[jumpdest%possibilities.size()];
                QVector    pos = jumpnode->Position();

                this->SetCurPosition( pos );
                ActivateAnimation( jumpnode );
                if (jumpnode->isUnit() == _UnitType::unit) {
                    QVector Offset( pos.i < 0 ? 1 : -1,
                                    pos.j < 0 ? 1 : -1,
                                    pos.k < 0 ? 1 : -1 );
                    Offset *= jumpnode->rSize()*2+this->rSize()*2;
                    this->SetPosAndCumPos( pos+Offset );
                    if (is_null( jumpnode->location[Unit::UNIT_ONLY] ) == false
                        && is_null( jumpnode->location[Unit::UNIT_BOLT] ) == false)
                        this->UpdateCollideQueue( pendingjump[kk]->dest, jumpnode->location );
                }
                jumpdest += 23231;
            }
            Unit *tester;
            for (unsigned int jjj = 0; jjj < 2; ++jjj)
                for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator();
                     (tester = *i) != NULL; ++i)
                    if (tester->isUnit() == _UnitType::unit && tester != this)
                        if ( ( this->LocalPosition()-tester->LocalPosition() ).Magnitude() < this->rSize()+tester->rSize() )
                            this->SetCurPosition( this->LocalPosition()+this->cumulative_transformation_matrix.getR()
                                           *( 4*( this->rSize()+tester->rSize() ) ) );
            DealPossibleJumpDamage( this );
            static int jumparrive = AUDCreateSound( vs_config->getVariable( "unitaudio", "jumparrive", "sfx43.wav" ), false );
            if (dosightandsound)
                AUDPlay( jumparrive, this->LocalPosition(), this->GetVelocity(), 1 );
        } else {
#ifdef JUMP_DEBUG
            VS_LOG(debug, "Unit FAILED remove from star system\n");
#endif
        }
        if (this->docked&DOCKING_UNITS)
            for (unsigned int i = 0; i < this->pImage->dockedunits.size(); i++) {
                Unit *unut;
                if ( NULL != ( unut = this->pImage->dockedunits[i]->uc.GetUnit() ) )
                    unut->TransferUnitToSystem( kk, savedStarSystem, dosightandsound );
            }
        if ( this->docked&(DOCKED|DOCKED_INSIDE) ) {
            Unit *un = this->pImage->DockedTo.GetUnit();
            if (!un) {
                this->docked &= ( ~(DOCKED|DOCKED_INSIDE) );
            } else {
                Unit *targ = NULL;
                for (un_iter i = pendingjump[kk]->dest->getUnitList().createIterator();
                     ( targ = (*i) );
                     ++i)
                    if (targ == un)
                        break;
                if (targ != un)
                    this->UnDock( un );
            }
        }
    } else {
        VS_LOG(warning, "Already jumped\n");
    }
    return ret;
}

#endif

