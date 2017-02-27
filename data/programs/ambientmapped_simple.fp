#include "fplod.h"

uniform sampler2D diffuseMap;
uniform samplerCube envMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;

vec3 ambientMapping()
{
   return textureCubeLod(envMap, gl_TexCoord[2].xyz, 8.0).rgb;
}

void main() 
{
  // Sample textures
  vec4 diffusemap  = texture2D(diffuseMap, gl_TexCoord[0].xy);
  vec3 diffusecol = gl_Color.rgb;
  diffusecol += ambientMapping();
  vec4 result;
  result.rgb = diffusemap.rgb * diffusecol;
  result.rgb += gl_SecondaryColor.rgb;
  result.a = diffusemap.a;
  result *= cloaking.rrrg;
  gl_FragColor = result;
}
