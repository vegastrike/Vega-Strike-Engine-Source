#ifndef __CUBE_H__
#define __CUBE_H__

//this file presents:
class cube;

//this file references:
struct side_and_coords;
class  ccoords;
class  fvector;

/*
* This file/class computes the face and coordinates where a ray intersects a surrounding cube; and
* viceversa: having a face and face coordinates, it gives us a 3d fvector, or "ray".
* But it will probably do more than that, namely, give us the rectangular sub-areas intersected by
* a cone, so that we can set up our iterations accordingly, rather than iterate the entire source
* cubemap for each target texel.
*/

#include "../fvector"

/*
* First some definitions:
* -----------------------
* Left means positive X
* Right means negative X
* Up means positive Y
* Down means negative Y
* Front means positive Z
* Back means negative Z
*/

//class cube is being used like a namespace here; but I think it will eventually be a real class :)
class cube
{
public:
    //get cube coords (3d) from side and signed side uv's:
    static void compute_ccoords_from_side_and_scoords( side_and_coords const & snc, ccoords & cc );
    //conversely, get side and signed uv's from cube coords:
    static void compute_side_and_scoords_from_ccoords( ccoords const & cc, side_and_coords & snc );
};


#endif


