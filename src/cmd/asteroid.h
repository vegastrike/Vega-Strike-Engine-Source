#ifndef _ASTEROID_H_
#define _ASTEROID_H_
#include "gfx/quaternion.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "cmd/script/flightgroup.h"
#include "cmd/collection.h"
#include "cmd/asteroid_generic.h"
#include "cmd/unit_factory.h"
#include "cmd/unit.h"

class GameAsteroid: public GameUnit<Asteroid>
{
	public:
		//void Init( float difficulty);
		//virtual enum clsptr isUnit() {return ASTEROIDPTR;}
		//virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
		virtual void UpdatePhysics2 (const Transformation& trans, const Transformation& old_physical_state, const Vector& accel, float difficulty, const Matrix& transmat, const Vector& CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
	protected:
		/** Constructor that can only be called by the UnitFactory.
		 */
		GameAsteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber=0, float difficulty=.01);

		friend class UnitFactory;

	private:
		/// default constructor forbidden
		GameAsteroid( );

		/// copy constructor forbidden
		GameAsteroid( const Asteroid& );

		/// assignment operator forbidden
		GameAsteroid& operator=( const Asteroid& );
};
#endif
