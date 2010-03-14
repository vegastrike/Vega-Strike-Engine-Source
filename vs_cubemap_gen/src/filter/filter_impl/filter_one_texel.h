#ifndef __FILTER_ONE_TEX_H__
#define __FILTER_ONE_TEX_H__

/*
* This class is a functor that is created for each texel of the target texture. It's operator()() takes a ray
* as input and returns a color. Computing this color is not trivial, however: It must iterate trough the source
* texels within a given angular radius of the ray, and accumulate light and "weights". The weight for each
* source texel is a product of its filter angle based weight --Phong: weight = pow( cos(angle), shininess )--,
* and the solid angle of that source texel, as perceived from the center of the cube.
*/

class mem_cubemap;
#include "../../units/radians.h"
#include "../../units/steradians.h"


class filter_one_texel
{
    mem_cubemap const & source_;
    float               shininess_;
    void                init_constants();
    radians             radius_;
    steradians          solid_angle_;
    //etceteras...
    dRGBAcol            accumulator; //weight in alpha channel
public:
    virtual ~filter_one_texel();
    filter_one_texel
    (
        mem_cubemap const & source
      , mem_cubemap const & target
      , Shininess const & shininess
    )
    : source_(source)
    , target_(target)
    , shininess_(shininess)
    {
        init_constants();
    }
    fRGBAcol operator()( fvector const * ray );
};


#endif


