#include "cmd/asteroid.h"
GameAsteroid::GameAsteroid(const char * filename, int faction, Flightgroup* fg, int fg_snumber, float difficulty):GameUnit<Asteroid> (filename,false, faction,string(""),fg,fg_snumber) {
	this->Asteroid::Init( difficulty);
}
void GameAsteroid::UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  GameUnit<Asteroid>::UpdatePhysics (trans,transmat, CumulativeVelocity,ResolveLast,uc);
}
