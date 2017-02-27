#include "../fplod.h"

#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

#define varTSView (gl_TexCoord[1].xyz)
#define varTSLight (gl_TexCoord[2].xyz)
#define varWSNormal (gl_TexCoord[3].xyz)

uniform sampler2D cosAngleToDepth_20;
uniform samplerCube envMap;

float  cityLightTrigger(float fNDotLB) { return saturatef(4.0*fNDotLB); }

float expandPrecision(vec4 src)
{
   return dot(src,(vec4(1.0,256.0,65536.0,0.0)/131072.0));
}

float cosAngleToDepth(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return expandPrecision(texture2DLod(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereHaloType),mn,mx),0.0)) * fAtmosphereHaloThickness;
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereHaloType),mn,mx)).a;
}

float cosAngleToLDepth(float fNDotV)
{
   return (1.0-cosAngleToAlpha(fNDotV))*2.0*saturatef(fNDotV)*fAtmosphereHaloThickness;
}

float cosAngleToADepth(float fNDotV)
{
   return cosAngleToAlpha(fNDotV)*2.0*saturatef(fNDotV)*fAtmosphereHaloThickness;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,2.0*fAtmosphereHaloContrast*fNDotL)); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,2.0*fGroundContrast*fNDotL)); }

vec3 reyleigh(float fVDotL, float ldepth)
{
   vec3 scatter = pow(vec3(1.0) - fAtmosphereScatterColor.a*fAtmosphereScatterColor.rgb, vec3(fReyleighRate*ldepth));
   float rfactor = ((fReyleighRate*ldepth > 0.0)?pow(saturatef(-fVDotL),64.0/(fReyleighAmount*fReyleighRate*ldepth)):0.0);
   return degamma(fReyleighAmount*rfactor*scatter);
}

float scaleAndOffset(float v)
{
   return saturatef( dot(vec2(v,1.0), fAtmosphereExtrusionNDLScaleOffs) );
}

vec4 atmosphericScatter(vec3 ambient, float fNDotV, float fNDotL, float fLDotV)
{
   float vadepth    = cosAngleToDepth(fNDotV)*2.0;
   float vdepth     = cosAngleToADepth(fNDotV);
   float rdepth     = vdepth;
   float ldepth     = cosAngleToLDepth(scaleAndOffset(fNDotL));
   float ralpha     = cosAngleToAlpha(fNDotV);
   ralpha           = saturatef(pow(ralpha,fAtmosphereExtrusionSteepness));
   
   vec3 labsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*ldepth*0.5*fSelfShadowFactor));
   
   vec3 lscatter    = gl_LightSource[0].diffuse.rgb 
                       * fAtmosphereScatterColor.rgb 
                       * labsorption
                       * (fMinScatterFactor+min(fMaxScatterFactor*4.0-fMinScatterFactor,4.0*vdepth*ralpha));
   
   vec4 rv;
   rv.rgb = regamma( ambient + atmosphereLighting(scaleAndOffset(fNDotL))*lscatter );
   rv.a = ralpha;
   return rv;
}

vec3 ambientMapping( in vec3 direction )
{
   return degamma_env(textureCubeLod(envMap, direction, 8.0)).rgb;
}

void main()
{      
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   
   vec4 rv = atmosphericScatter( ambientMapping(varWSNormal), V.z, L.z, dot(L,V) );
   gl_FragColor.rgb = (rv.rgb);
   gl_FragColor.a = rv.a;
}


