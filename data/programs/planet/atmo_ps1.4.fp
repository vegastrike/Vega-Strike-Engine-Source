#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

#define varTSView (gl_TexCoord[1].xyz)
#define varTSLight (gl_TexCoord[2].xyz)
#define varWSNormal (gl_TexCoord[3].xyz)

uniform sampler2D cosAngleToDepth_20;
uniform samplerCube envMap;

float  cityLightTrigger(float fNDotLB) { return saturatef(4.0*fNDotLB); }

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereHaloType),mn,mx)).a;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,2.0*fAtmosphereHaloContrast*fNDotL)); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,2.0*fGroundContrast*fNDotL)); }

float scaleAndOffset(float v)
{
   return saturatef( dot(vec2(v,1.0), fAtmosphereExtrusionNDLScaleOffs) );
}

vec4 atmosphericScatter(vec4 ambient, float fNDotV, float fNDotL, float fLDotV)
{
   float ralpha = cosAngleToAlpha(fNDotV);
   vec4 rv;
   rv.rgb = regamma(ambient.rgb + atmosphereLighting(scaleAndOffset(fNDotL))*1.414*fAtmosphereScatterColor.rgb );
   rv.a = sqr(ralpha);
   return rv;
}

void main()
{      
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   
   vec4 rv = atmosphericScatter( gl_LightSource[0].ambient, V.z, L.z, dot(L,V) );
   gl_FragColor.rgb = (rv.rgb);
   gl_FragColor.a = rv.a;
}


