#include "gas_giants_params.h"
#include "../config.h"
#include "../stdlib.h"

#define varTSView (gl_TexCoord[1].xyz)
#define varTSLight (gl_TexCoord[2].xyz)
#define varWSNormal (gl_TexCoord[3].xyz)

uniform sampler2D baseMap;
uniform sampler2D cosAngleToDepth_20;
uniform samplerCube envMap;

float expandPrecision(vec4 src)
{
   return dot(src,(vec4(1.0,256.0,65536.0,0.0)/131072.0));
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereHaloType),mn,mx)).a;
}

float cosAngleToADepth(float fNDotV)
{
   return cosAngleToAlpha(fNDotV)*2.0*saturatef(fNDotV)*fAtmosphereExtrusionThickness;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,fAtmosphereHaloContrast*sqr(fNDotL))); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,fGroundContrast*fNDotL)); }

vec3 reyleigh(float fVDotL, float ldepth, vec3 fAtmosphereScatterColor)
{
    vec3 scatter = gl_LightSource[0].diffuse.rgb * luma(fAtmosphereScatterColor.rgb);
    float rfactor = fVDotL*pow(saturatef(-fVDotL),8.0);
    return degamma(fReyleighAmount*rfactor*scatter);
}

float reyleighShadow(float fNDotV) {
    return sqr(sqr(1.0-fNDotV));
}

float scaleAndOffset(float v)
{
   return saturatef( dot(vec2(v,1.0), fAtmosphereExtrusionNDLScaleOffs) );
}

vec4 atmosphericScatter(vec3 ambient, vec3 dif, float fNDotV, float fNDotL, float fLDotV)
{
   float rdepth = cosAngleToADepth(fNDotV);
   float ralpha = cosAngleToAlpha(fNDotV);
   ralpha        = saturatef(pow(ralpha,fAtmosphereExtrusionSteepness));
   
   vec4 rv;
   rv.rgb = regamma( ambient * dif * 0.5
                  + atmosphereLighting(scaleAndOffset(fNDotL))
                    *reyleighShadow(fNDotV)
                    *(dif+reyleigh(fLDotV,rdepth*ralpha,dif)) );
   rv.a = ralpha;
   return rv;
}

vec3 ambientMapping( in vec3 direction )
{
   return degamma_env(textureCube(envMap, direction, 8.0)).rgb;
}

void main()
{      
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   
   vec3 dif = texture2D(baseMap, gl_TexCoord[0].xy, 4.0).rgb;
   
   gl_FragColor = atmosphericScatter( ambientMapping(varWSNormal), dif, V.z, L.z, dot(L,V) );
}


