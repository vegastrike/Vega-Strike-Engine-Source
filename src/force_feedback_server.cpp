/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Force Feedback support by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include "force_feedback.h"

#include "vegastrike.h"
#include "vsfilesystem.h"
#include "vs_globals.h"

#define FF_DOIT 1


ForceFeedback::ForceFeedback(){
};
ForceFeedback::~ForceFeedback(){
}

bool ForceFeedback::haveFF(){
	return false;
}
void ForceFeedback::updateForce(float angle,float strength){
}
void ForceFeedback::updateSpeedEffect(float strength){
}
void ForceFeedback::playHit(float angle,float strength){
}
void ForceFeedback::playAfterburner(bool activate){
}
void ForceFeedback::playLaser(){
}

void ForceFeedback::playDurationEffect(unsigned int eff_nr,bool activate){
}

void ForceFeedback::playShortEffect(unsigned int eff_nr){
}

#if HAVE_FORCE_FEEDBACK

void ForceFeedback::playEffect(unsigned int eff_nr){
}
void ForceFeedback::stopEffect(unsigned int eff_nr){
}
void ForceFeedback::init(){
}

#endif
