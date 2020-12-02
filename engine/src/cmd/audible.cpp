/**
 * audible.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
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

#include "audible.h"

#include "gfx/vec.h"
#include "vs_globals.h"
#include "universe.h"
#include "audiolib.h"
#include "options.h"

void Audible::ArmorDamageSound( const Vector &pnt )
{
    Unit *unit = static_cast<Unit*>(this);

    if ( !_Universe->isPlayerStarship( unit ) ) {
        if ( AUDIsPlaying( this->sound->armor ) )
            AUDStopPlaying( this->sound->armor );
        if (game_options.ai_sound)
            AUDPlay( this->sound->armor, unit->ToWorldCoordinates(
                         pnt ).Cast()+unit->cumulative_transformation.position, unit->Velocity, 1 );
    } else {
        static int playerarmorsound =
            AUDCreateSoundWAV( game_options.player_armor_hit );
        int sound = playerarmorsound != -1 ? playerarmorsound : this->sound->armor;
        if ( AUDIsPlaying( sound ) )
            AUDStopPlaying( sound );
        AUDPlay( sound, unit->ToWorldCoordinates(
            pnt ).Cast()+unit->cumulative_transformation.position, unit->Velocity, 1 );
    }
}

void Audible::HullDamageSound( const Vector &pnt )
{
    Unit *unit = static_cast<Unit*>(this);

    if ( !_Universe->isPlayerStarship( unit ) ) {
        if ( AUDIsPlaying( this->sound->hull ) )
            AUDStopPlaying( this->sound->hull );
        if (game_options.ai_sound)
            AUDPlay( this->sound->hull, unit->ToWorldCoordinates(
                         pnt ).Cast()+unit->cumulative_transformation.position, unit->Velocity, 1 );
    } else {
        static int playerhullsound = AUDCreateSoundWAV( game_options.player_hull_hit );
        int sound = playerhullsound != -1 ? playerhullsound : this->sound->hull;
        if ( AUDIsPlaying( sound ) )
            AUDStopPlaying( sound );
        AUDPlay( sound, unit->ToWorldCoordinates(
            pnt ).Cast()+unit->cumulative_transformation.position, unit->Velocity, 1 );
    }
}
