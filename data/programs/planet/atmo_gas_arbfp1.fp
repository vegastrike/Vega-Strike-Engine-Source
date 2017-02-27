#include "../fplod.h"

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
   return (1.0-cosAngleToAlpha(fNDotV))*2.0*saturatef(fNDotV)*fAtmosphereExtrusionThickness;
}

float cosAngleToADepth(float fNDotV)
{
   return cosAngleToAlpha(fNDotV)*2.0*saturatef(fNDotV)*fAtmosphereExtrusionThickness;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,fAtmosphereHaloContrast*sqr(fNDotL))); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,fGroundContrast*fNDotL)); }

vec3 reyleigh(float fVDotL, float ldepth, vec3 fAtmosphereScatterColor, float saturation)
{
    if (ldepth > 0.0 && fVDotL < 0.0) {
        vec3 scatter = lerp(gl_LightSource[0].diffuse.rgb * luma(fAtmosphereScatterColor.rgb), fAtmosphereScatterColor.rgb, saturation);
        float rfactor = fVDotL*pow(saturatef(-fVDotL),64.0/(fReyleighAmount*fReyleighRate*ldepth));
        return degamma(fReyleighAmount*rfactor*scatter);
    } else {
        return vec3(0.0);
    }
}

float reyleighShadow(float fNDotV) {
    return pow(1.0-fNDotV, 6.0);
}

float scaleAndOffset(float v)
{
   return saturatef( dot(vec2(v,1.0), fAtmosphereExtrusionNDLScaleOffs) );
}

vec4 atmosphericScatter(vec3 ambient, vec3 dif, float fNDotV, float fNDotL, float fLDotV)
{
   float vadepth    = cosAngleToDepth(fNDotV)*2.0;
   float vdepth     = cosAngleToADepth(fNDotV);
   float rdepth     = vdepth;
   float ldepth     = cosAngleToLDepth(fNDotL);
   float ralpha     = cosAngleToAlpha(fNDotV);
   ralpha           = saturatef(pow(ralpha,fAtmosphereExtrusionSteepness));
   
   vec3 labsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*ldepth*0.5*fSelfShadowFactor));
   
   vec3 lscatter    = gl_LightSource[0].diffuse.rgb 
                       * dif * labsorption 
                       * (fMinScatterFactor+min(fMaxScatterFactor*2.0-fMinScatterFactor,2.0*vdepth*ralpha));
   
   vec4 rv;
   rv.rgb = regamma( ambient * dif * 0.5
                  + atmosphereLighting(scaleAndOffset(fNDotL))
                    *reyleighShadow(fNDotV) // out of reyleigh since it must apply to lscatter too
                    *(lscatter+reyleigh(fLDotV,rdepth*ralpha,dif,0.4)) );
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
   
   vec3 dif = texture2D(baseMap, gl_TexCoord[0].xy, 4.0).rgb;
   
   gl_FragColor = atmosphericScatter( ambientMapping(varWSNormal), dif, V.z, L.z, dot(L,V) );
}


