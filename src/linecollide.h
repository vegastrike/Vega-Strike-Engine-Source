#ifndef _LINECOLLIDE_H_
#define _LINECOLLIDE_H_

#include <stdlib.h>
#include "gfx/vec.h"

/**
 * Line Collide provides a complete container for a unit that is put in a collide hashtable
 * For collisions. The mini, maxi vectors of the line collide are taken
 */
class Unit;
class Beam;
class Bolt;

struct LineCollide
{
private:
  union OBJECT {
    Unit *u;
    Beam *b;
    Bolt *blt;
    int i;
  };

public:
    enum collidables {UNIT, BEAM,BALL,BOLT,PROJECTILE};

    ///The object that this LineCollide approximates
    OBJECT object;
    ///The minimum x,y,z that this object has
    QVector Mini;
    ///The maximum x,y,z that this object has
    QVector Maxi;
    /**
     * The last item that checked this for collisions
     * to prevent duplicate selection 
     */
    void * lastchecked;

    ///Which type of unit it is. Used for subsequently calling object's Accurate collide func
    collidables type;

    ///If this object was saved as a huge object (hhuge for dos oddities)
    bool hhuge;

    LineCollide()
        : Mini(0,0,0)
        , Maxi(0,0,0)
        , lastchecked( NULL )
        , type(UNIT)
        , hhuge( false )
    {
        object.u=NULL;
    }

    LineCollide( void * objec, collidables typ,const QVector &st, const QVector &en)
        : Mini( st )
        , Maxi( en )
        , lastchecked( NULL )
        , type( typ )
        , hhuge( false )
    {
        this->object.u=(Unit *)objec;
    }

    LineCollide( const LineCollide& l )
        : Mini( l.Mini )
        , Maxi( l.Maxi )
        , lastchecked( NULL )
        , type( l.type )
        , hhuge( l.hhuge )
    {
        object=l.object;
    }      

    LineCollide& operator=( const LineCollide& l )
    {
        object=l.object;
        type=l.type;
        Mini=l.Mini;
        Maxi=l.Maxi;
        hhuge=l.hhuge;
        lastchecked=NULL;
        return *this;
    }
};

#endif
