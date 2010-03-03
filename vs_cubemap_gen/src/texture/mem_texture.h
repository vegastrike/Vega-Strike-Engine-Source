#ifndef __MEM_TEXTURE_H__
#define __MEM_TEXTURE_H__

//this file presents:
class mem_texture;

//this file references:
class fcolor;
class steradians;

//it also defines:
typedef std::pair< fcolor, steradians > tex_fetch_t;


template < size_t SIZE >
class mem_texture
{
    //float front [SIZE*SIZE*3];
    //float back  [SIZE*SIZE*3];
    //float left  [SIZE*SIZE*3];
    //float right [SIZE*SIZE*3];
    //float up    [SIZE*SIZE*3];
    //float down  [SIZE*SIZE*3];
    float buff[6*SIZE*SIZE*3];
    size_t tex_index( side const & s, icoords const & ic ) const;
    radians angles_from_center_[SIZE>>1];
public:
    mem_texture();
    size_t size() const { return SIZE; }
    radians angle_from_center( size_t single_coord ) const;
    radians texel_angle_width( size_t single_coord ) const;
    steradians texel_solid_angle( icoords const & ic ) const;
    tex_fetch_t fetch_texel( side const & s, icoords const & ic ) const;
}


#endif


