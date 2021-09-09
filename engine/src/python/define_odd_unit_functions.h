/**
* define_odd_unit_functions.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/


void DefineOddUnitFunctions (boost::python::class_builder <UnitWrapper> &Class) {
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::setNull,"setNull");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::isNull,"isNull");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::notNull,"__nonzero__");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::Kill,"Kill");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::SetTarget,"SetTarget");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetTarget,"GetTarget");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::SetVelocityReference,"SetVelocityReference");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetVelocityReference,"GetVelocityReference");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetOrientation,"GetOrientation");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::rayCollide,"rayCollide");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::cosAngleTo,"cosAngleTo");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::cosAngleToITTS,"cosAngleToITTS");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::cosAngleFromMountTo,"cosAngleFromMountTo");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::getAverageGunSpeed,"getAverageGunSpeed");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::InsideCollideTree,"InsideCollideTree");
//  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::getFlightgroupLeader,"getFlightgroupLeader");
//  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::setFlightgroupLeader,"setFlightgroupLeader");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetVelocityDifficultyMult,"GetVelocityDifficultyMult");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetJumpStatus,"GetJumpStatus");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::ApplyDamage,"ApplyDamage");
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::GetMountInfo,"GetMountInfo");
}
