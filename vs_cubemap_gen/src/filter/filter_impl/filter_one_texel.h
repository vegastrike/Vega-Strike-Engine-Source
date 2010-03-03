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
    radians             radius_;
    void init_constants();
    steradians          solid_angle_;
    float               shininess_;
    //etceteras...
    double acc_red_;
    double acc_grn_;
    double acc_blu_;
    double acc_weight_;
    size_t count_;
public:
    virtual ~filter_one_texel();
    filter_one_texel
    (
        mem_cubemap const & source
      , radians const & radius
    )
    : source_(source)
    , target_(target)
    , radius_(radius)
    {
        init_constants();
    }
    fcolor operator()( fvector const * ray );
};


#endif


