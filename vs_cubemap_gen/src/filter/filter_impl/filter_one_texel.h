#ifndef __FILTER_ONE_TEX_H__
#define __FILTER_ONE_TEX_H__

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


