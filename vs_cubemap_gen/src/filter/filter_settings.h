#ifndef __FILTER_MATH_H__
#define __FILTER_MATH_H__


class filter_settings
{
    //input params:
    size_t   top_lod_size_;
    size_t   ambient_lod_size_;
    //computed params:
    double   lod_ratio_;
    size_t   lod_steps_;
    radians  min_flt_radius_;
    radians  amb_flt_radius_;
    float    flt_radius_ratio_;
    double   angle_scaling_;
public:
    filter_settings(
                     size_t top_lod_size        //e.g. 1024
                   , size_t ambient_lod_size    //e.g.    8
                   )
      : top_lod_size_(top_lod_size)
      , ambient_lod_size_(ambient_lod_size)
    {
    }
    void init();
    void print();
};


#endif


