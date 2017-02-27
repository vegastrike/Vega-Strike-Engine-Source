#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

uniform sampler2D specularMap_20;
uniform sampler2D baseMap_20;
uniform sampler2D cityLights_20;
uniform sampler2D cosAngleToDepth_20;
uniform sampler2D cloudMap_20;
uniform sampler2D noiseMap_20;
uniform sampler2D normalMap_20;
uniform samplerCube envMap;

#define varTSView (gl_TexCoord[3].xyz)
#define varTSLight (gl_TexCoord[4].xyz)
#define varScatter (gl_TexCoord[5].rgb)

float  cityLightTrigger(float fNDotLB) { return clamp(4.0*fNDotLB, 0.0, 1.0); }
float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,2.0*fAtmosphereContrast*fNDotL)); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,2.0*fGroundContrast*fNDotL)); }

vec4 atmosphericScatter(vec4 dif, float fNDotV, float fNDotL, float fVDotL, vec3 fvShadow, vec3 fvAShadow)
{
   vec4 rv;
   rv.rgb = regamma( dif.rgb*fvShadow + varScatter*fvAShadow );
   rv.a = dif.a;
   return rv;
}

void main()
{      
   vec2 texcoord = gl_TexCoord[0].xy;
   vec4 shadowcoord = gl_TexCoord[1];
   
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   vec3 N = expand( texture2D( normalMap_20, texcoord ).rgb ); // Do not normalize, to avoid aliasing
   N.x = -N.x;
   vec3 R = -reflect(L,N);
   
   float  fNDotL           = saturatef( dot(N,L) ); 
   float  fNDotLs          = saturatef( L.z ); 
   float  fNDotLf          = L.z; 
   float  fNDotLB          = saturatef(-L.z + fCityLightTriggerBias.x);
   float  fRDotV           = saturatef( dot(R,V) );
   float  fNDotV           = saturatef( dot(N,V) );
   float  fNDotVs          = saturatef( V.z );
   float  fVDotL           = dot(L,V);
   
   vec4 fvTexColor         = texture2D( baseMap_20, texcoord );
   fvTexColor.rgb          = degamma_tex(fvTexColor.rgb);
   
   vec4 gcitymap            = texture2D(cityLights_20, texcoord);
   gcitymap.rgb             = degamma_tex(gcitymap.rgb);
   gcitymap.rgb             = degamma_tex(gcitymap.rgb); // degamma twice, it's  glowmap and we need a lot of precision near darkness
   
   // Make citymap night-only
   float cityLightFactor   = fCloud_Dens_Thick_CLF_SSF.z;
   vec3 trigger            = (cityLightTrigger(fNDotLB) * cityLightFactor) * fvCityLightColor.rgb;
   gcitymap.rgb           *= trigger;
   
   float  fGShadow         = texture2D( cloudMap_20, shadowcoord.xy ).a;
   fGShadow                = saturatef(fGShadow*fCloudLayerDensity);
   
   vec3 fvGShadow          = lerp( vec3(1.0), fvShadowColor.rgb, fGShadow );
   vec3 fvAShadow          = lerp( vec3(1.0), fvShadowColor.rgb, fGShadow*0.1 );
   
   vec4 fvSpecular         = degamma_tex(texture2D( specularMap_20, texcoord ));
   fRDotV                  = pow( fRDotV, fShininess.x*(2.56+253.5*fvSpecular.a) );
   fvSpecular.rgb          = fvSpecular.rgb * gl_SecondaryColor.rgb * fRDotV;

   vec4 fvBaseColor;
   // ambient
   fvBaseColor.rgb         = gl_Color.rgb * groundLighting(fNDotL) + gl_LightSource[0].ambient.rgb; // per-pixel for normal mapping
   fvBaseColor.a           = gl_Color.a;
   fvBaseColor            *= fvTexColor;
   fvBaseColor.rgb         = fvBaseColor.rgb + fvSpecular.rgb + gcitymap.rgb;

   gl_FragColor = atmosphericScatter( fvBaseColor, fNDotVs, fNDotLs, fVDotL, fvGShadow, fvAShadow );
}


