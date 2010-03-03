#ifndef __COORDS_H__
#define __COORDS_H__


//this file presents:
struct ucoords;
struct scoords;
struct side_and_coords;


//this file references
//from "side.h":
enum eSides;
template < eSides > struct side;
template <> side< eLeft > left_side;
template <> side< eRight > left_side;
template <> side< eUp > left_side;
template <> side< eDown > left_side;
template <> side< eFront > left_side;
template <> side< eBack > left_side;


//coordinates stuff:
/*
* Coordinates for textures usually start at 0,0 at one corner, and grow to 1,1 at the opposite corner.
* We call these coordinates "ucoord", because of their "unsigned range".
* For our computations, it is more convenient to have coordinates that span -0.5 to +0.5 range. So, we
* call these "scoords".
* Of course, there's also "integer" type coordinates, used to identify texels like an array index. And
* we shall call these "icoords".
*/
struct icoords
{
    size_t u_, v_;
};
struct ucoords
{
    float u_, v_;
    explicit ucoords( scoords const & sc );
    ucoords( icoords const & ic, size_t texture_size );
};
struct scoords
{
    float u_, v_;
    explicit scoords( ucoords const & uc );
};

//side_and_coords specify a side in the cube and its (signed) texture coordinates
struct side_and_coords
{
    side const &  side_;
    scoords       scoords_;
    side_and_coords( side const & s, scoords const & c ): side_(s), scoords_(c) {}
};

//cube coords, called "ccoords" here, span from -0.5 to +0.5 in x, y AND z axes;
//they are interconvertible with side_and_coords, as well as with fvector (ray),
//thus acting like a bridge
class ccoords
{
    float x_, y_, z_;
    void check_invariants(); //throw() ... but only for debugging
public:
    ccoords( float x, foat y, float z ): x_(x), y_(y), z(_z)
    {
        check_invariants();
    }
    explicit ccoords( fvector const & v );
};


#endif


