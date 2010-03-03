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
    void init()
    {
        vs_assert( top_lod_size > ambient_lod_size, "lod sizes make no sense" );
        vs_assert( is_POT(top_lod_size), "target texture size must be a power of two" );
        vs_assert( is_POT(ambient_lod_size), "target ambient lod size must be a power of two" );
        lod_ratio_ = double(top_lod_size) / double(ambient_lod_size); //128
        lod_steps_ = size_t( log2( lod_ratio )+0.0001 ); //7
        min_flt_radius_ = float( atan( sqrt(0.5) / double(top_lod_size_) ) ); //0.000690534 (0.039564676 deg radius)
        amb_flt_radius_ = float( 0.531734722 ) //see http://wcjunction.com/phpBB2/viewtopic.php?p=22193#22193
        flt_radius_ratio_ = amb_flt_radius_ / min_flt_radius_; // 0.531734722 / 0.000690534 = 770.034092456
        angle_scaling_ = pow( flt_radius_ratio_, 1.0 / double(lod_steps_) ); // 770.034092456 ^ (1/7) = 2.584393052
    }
    void print();
};

steradians<double> solid_angle_from_radius( radians<double> const & radius )
{
    return 6.283185307 * (1.0-cos(radius));
}

double shininess_from_solid_angle( steradians<double> const & sa )
{
    return ( 1.570796327 / sa ) - 0.810660172;
}

inline float flt_decay( float shininess, fvector v1, fvector v2 )
{
    return pow( max( 0.0, dot( v1, v2 ) ), shininess );
}

#endif


