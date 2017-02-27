#include "config.h"

#if (NORMALMAP_TYPE == CINEMUT_NM)
vec2 dUdV_first_decode( in vec4 nmfetch )
{
  return vec2( 0.3333*(nmfetch.r+nmfetch.g+nmfetch.b)-0.5, nmfetch.a-0.5 );
}
vec3 dUdV_final_decode( vec2 blend_of_first_decode )
{
    return normalize( vec3( blend_of_first_decode, 0.25 ) );
}
vec3 normalmap_decode(vec4 nm)
{
  return dUdV_final_decode( dUdV_first_decode( nm ) );
}
#endif
#if (NORMALMAP_TYPE == RED_IN_ALPHA_NM)
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(nm.wy*vec2(2.0,2.0)-vec2(1.0,1.0),nm.z) );
}
#endif
#if (NORMALMAP_TYPE == TRADITIONAL_NM)
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(nm.xy*vec2(2.0,2.0)-vec2(1.0,1.0),nm.z) );
}
#endif
#if (NORMALMAP_TYPE == DXT5_NM)
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(nm.ag*vec2(2.0,2.0)-vec2(1.0,1.0),sqrt(1.0-dot(nm.ag*nm.ag,vec2(1.0,1.0))) ) );
}
#endif
