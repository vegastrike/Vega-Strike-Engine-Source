
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
  PYTHON_DEFINE_METHOD(Class,&UnitWrapper::queryBSP,"queryBSP");
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
