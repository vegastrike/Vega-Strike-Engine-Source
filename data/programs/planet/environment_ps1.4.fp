#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

uniform sampler2D diffuseMap;
uniform samplerCube envMap;
uniform sampler2D specMap;
uniform sampler2D cloudMap;
uniform sampler2D cityMap;
uniform sampler2D cosAngleToDepth_20;

vec3 ambientMapping( in vec3 direction, in float cloudmap )
{
   return gl_LightSource[0].ambient.rgb * (1.0 - cloudmap);
}

vec3 specEnvMapping( in float shininess, in vec3 direction, in float cloudmap ) //const
{
  float mipbias = max(0.0, 8.0 - shininess * shininess * 16.0);
  vec4 result = textureCube( envMap, direction, mipbias );
  result = degamma_env(result);
  
  return result.rgb * (1.0 - cloudmap);
}

float fresnel(float fNDotV)
{
   return degamma(1.0-lerp(0.0,fNDotV,fFresnelEffect.x));
}

float  cityLightTrigger(float fNDotL) { return clamp(4.0*(-fNDotL + fCityLightTriggerBias.x), 0.0, 1.0); }

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx)).a;
}

void main() 
{
  // Compute relevant vectors
  vec2 texcoord    = gl_TexCoord[0].xy;
  vec2 shadowcoord = gl_TexCoord[4].xy;
  vec2 refgndcoord = gl_TexCoord[5].xy;
  vec3 eye         = normalize(gl_TexCoord[3].xyz);
  vec3 diffusecol  = gl_Color.rgb;
  vec3 speccol     = gl_SecondaryColor.rgb;
  vec3 normal      = normalize(gl_TexCoord[1].xyz);
  vec3 lightpos    = normalize(gl_TexCoord[2].xyz);
  vec3 reflection  = normalize(-reflect(eye,normal));
  float cityLightFactor = fCloud_Dens_Thick_CLF_SSF.z;

  // Sample textures
  vec4 specmap     = texture2D(specMap, texcoord);
  vec4 diffusemap  = texture2D(diffuseMap, texcoord);
  vec4 gcitymap    = texture2D(cityMap, texcoord);
  float cloudmap   = texture2D(cloudMap, shadowcoord).a;
  
  diffusemap.rgb   = degamma_tex(diffusemap.rgb);
  specmap.rgb      = degamma_tex(specmap.rgb);
  gcitymap.rgb     = degamma_tex(gcitymap.rgb);
  gcitymap.rgb     = degamma_tex(gcitymap.rgb); // degamma twice, it's  glowmap and we need a lot of precision near darkness
  
  // Compute specular factor
  float shininess  = fShininess.r * specmap.a;
  float fNDotV     = dot(normal, eye);
  float fNDotL     = dot(normal, lightpos);
  vec3 specular    = fresnel(fNDotV) * speccol * specmap.rgb;
  
  // Make citymap night-only
  vec3 trigger     = (cityLightTrigger(fNDotL) * cityLightFactor) * fvCityLightColor.rgb;
  gcitymap.rgb    *= trigger;

  // Do lighting
  vec3 result;
  result = diffusecol * diffusemap.rgb * ambientMapping(normal, cloudmap) 
         + specEnvMapping(shininess, reflection, cloudmap) * specular
         + gcitymap.rgb;
  
  // Do silhouette alpha
  float  alpha     = saturatef(2.0 * (cosAngleToAlpha(fNDotV) - 0.5));
  
  // re-gamma and return
  gl_FragColor.a = diffusemap.a * alpha;
  gl_FragColor.rgb = regamma(result * alpha);
}
