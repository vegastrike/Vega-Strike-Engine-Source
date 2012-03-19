#ifndef _ASTEROIDGENERIC_H_
#define _ASTEROIDGENERIC_H_
#include "cmd/script/flightgroup.h"
#include "gfx/vec.h"
#include "cmd/unit_generic.h"

class Asteroid: public Unit
{
	public:
		void Init( float difficulty);
		virtual enum clsptr isUnit() const { return(ASTEROIDPTR);}
		virtual void reactToCollision(Unit * smaller, const QVector& biglocation, const Vector& bignormal, const QVector& smalllocation, const Vector& smallnormal, float dist);

		protected:
		/** Constructor that can only be called by the UnitFactory.
		 */
		Asteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber=0, float difficulty=.01);

		friend class UnitFactory;

		// default constructor forbidden
		Asteroid( ) {}
		Asteroid (std::vector <Mesh *> m,bool b,int i): Unit (m,b,i){}
		// copy constructor forbidden
		
		// assignment operator forbidden
		
	private:
		unsigned int asteroid_physics_offset;
};
#endif
