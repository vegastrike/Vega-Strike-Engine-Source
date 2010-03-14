#ifndef __CUBE_H__
#define __CUBE_H__

//this file presents:
class cube;

//this file references:
struct side_and_coords;
class  ccoords;
class  fvector;

/*
NOT SURE ABOUT THIS ANYMORE:
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

#include <memory>

struct cube_impl;
enum eSides;

//class cube is being used like a namespace here; but I think it will eventually be a real class :)
class cube
{
    std::auto_ptr<cube_impl> impl_;
    cube( cube const & ); //no copying
    cube & operator=( cube const & ); //no copying
public:
    cube();
    mem_texture const & get_buffer() const;
    mem_texture       & get_buffer();
};


#endif


