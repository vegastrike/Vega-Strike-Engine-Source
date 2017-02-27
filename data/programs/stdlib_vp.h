#include "config.h"

uniform vec4 light_size[gl_MaxLights];

vec4 lightPosAndSize0(in vec4 vertex)
{
  vec4 lpos = gl_LightSource[0].position;
  vec4 rv;
  rv.xyz    = lpos.xyz - vertex.xyz*lpos.w;
  rv.w      = light_size[0].z;
  return rv;
}

float lightAtt0(in float lightdistance, in float lightdistancesq)
{
  float d = lightdistance;
  float d2 = lightdistancesq;
  return 1.0 / dot( vec3(1.0,d,d2),
                    vec3(gl_LightSource[0].constantAttenuation,
                         gl_LightSource[0].linearAttenuation,
                         gl_LightSource[0].quadraticAttenuation) );
}

float lightAtt0(in vec4 lightpos)
{
    float d2 = dot(lightpos.xyz, lightpos.xyz);
    float d = sqrt(d2);
    return lightAtt0(d, d2);
}

void lightPosSizeAndAtt0(in vec4 vertex, out vec4 lightposnsize, out float lightatt)
{
   vec4 rlightpos = lightPosAndSize0(vertex);
   float lightd2 = dot(rlightpos.xyz, rlightpos.xyz);
   float lightd = sqrt(lightd2);
   vec4 lightpos = rlightpos / lightd;
   lightatt = lightAtt0(lightd, lightd2);
   lightposnsize = lightpos;
}

