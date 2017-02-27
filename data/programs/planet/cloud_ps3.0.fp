#version 130

#include "../fplod.h"

// Default precision qualifier - make pedantic drivers happy,
// since OpenGL specs clearly specify they have no meaning except in OpenGL ES
precision highp float;

#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

#define inCloudCoord gl_TexCoord[0]
#define inGroundCoord gl_TexCoord[1]
#define inShadowCoord gl_TexCoord[2]
#define inNoiseCoord gl_TexCoord[3]
#define inCityCoord gl_TexCoord[4]

varying vec3 varTSLight;
varying vec3 varTSView;
varying vec3 varWSNormal;

uniform sampler2D cosAngleToDepth_20;
uniform sampler2D cloudMap_20;
uniform sampler2D noiseMap_20;
uniform sampler2D cityLights_20;
uniform samplerCube envMap;

float  cityLightTrigger(float fNDotLB) { return saturatef(4.0*fNDotLB); }

float fresnel(float fNDotV)
{
   return fresnel(fNDotV,fFresnelEffect.x);
}

float expandPrecision(vec4 src)
{
   return dot(src,(vec4(1.0,256.0,65536.0,0.0)/131072.0));
}

float cosAngleToDepth(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return expandPrecision(texture2DLod(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx),0.0)) * fAtmosphereThickness;
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   vec2 tc = clamp(vec2(fNDotV,fAtmosphereType),mn,mx);
   return textureGrad(cosAngleToDepth_20,tc,dFdx(vec2(0.0,tc.y)),dFdy(vec2(0.0,tc.y))).a;
}

float  atmosphereLighting(float fNDotL) { return saturatef(soft_min(1.0,2.0*fAtmosphereContrast*fNDotL)); }
float  groundLighting(float fNDotL) { return saturatef(soft_min(1.0,2.0*fGroundContrast*fNDotL)); }

vec3 reyleigh(float fVDotL, float ldepth)
{
    if (ldepth > 0.0 && fVDotL < 0.0) {
        vec3 scatter = pow(vec3(1.0) - fAtmosphereScatterColor.a*fAtmosphereScatterColor.rgb, vec3(fReyleighRate*ldepth));
        float rfactor = pow(saturatef(-fVDotL),64.0/(fReyleighAmount*fReyleighRate*ldepth));
        return degamma(fReyleighAmount*rfactor*scatter);
    } else {
        return vec3(0.0);
    }
}

vec4 atmosphericScatter(vec3 amb, vec4 dif, float fNDotV, float fNDotL, float fVDotL)
{
   float  vdepth     = cosAngleToDepth(fNDotV) * sqr(saturatef(1.0-fShadowRelHeight.x));
   float  ldepth     = cosAngleToDepth(fNDotL+fAtmosphereAbsorptionOffset) * sqr(saturatef(1.0-fShadowRelHeight.x));
   float  alpha      = cosAngleToAlpha(fNDotV);
   
   #if SCATTER
   vec3  labsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*ldepth));
   vec3  vabsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*vdepth*2.0));
   vec3  lscatter    = gl_LightSource[0].diffuse.rgb 
                       * fAtmosphereScatterColor.rgb 
                       * pow(labsorption,vec3(fSelfShadowFactor)) 
                       * (fMinScatterFactor+soft_min(fMaxScatterFactor-fMinScatterFactor,vdepth*2.0))
                     + reyleigh(fVDotL,ldepth*alpha);
   
   #else
   const vec3 labsorption = vec3(1.0);
   const vec3 vabsorption = vec3(1.0);
   const vec3 lscatter    = vec3(0.0);
   #endif
   
   
   vec4 rv;
   rv.rgb = regamma( amb + dif.rgb*(labsorption*vabsorption)
                  + atmosphereLighting(fNDotL)
                    *lscatter );
   rv.a = dif.a * alpha;
   return rv;
}

vec3 ambientMapping( in vec3 direction )
{
   return degamma_env(textureCubeLod(envMap, direction, 8.0)).rgb;
}


void main()
{    
   vec2 CloudCoord = inCloudCoord.xy;
   vec2 GroundCoord = inGroundCoord.xy;
   vec2 ShadowCoord = inShadowCoord.xy;
   vec2 NoiseCoord = inNoiseCoord.xy;
   vec2 CityCoord = inCityCoord.xy;
   
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   vec3 N = varWSNormal;
   
   float  fNDotL           = saturatef( L.z ); 
   float  fNDotLB          = saturatef(-L.z + fCityLightTriggerBias.x);
   float  fNDotV           = saturatef( V.z );
   float  fVDotL           = dot(L, V);

   // Attack angle density adjustment   
   vec3 CloudLayerDensitySVC;
   float  fCloudLayerDensityL = fCloudLayerDensity / (abs(L.z)+0.01);
   float  fCloudLayerDensityV = fCloudLayerDensity / (abs(V.z)+0.01);
   CloudLayerDensitySVC.x     = fCloudLayerDensityL * fCloudSelfShadowFactor;
   CloudLayerDensitySVC.y     = fCloudLayerDensityV;
   CloudLayerDensitySVC.z     = fCloudLayerDensity * fCloudSelfShadowFactor;
  
   // Drift noise
   vec4 cnoise       = texture2D(noiseMap_20,NoiseCoord);
   vec4 hcnoise      = texture2D(noiseMap_20,NoiseCoord*7.0);
   vec3 noise        = /*hcnoise.xyz * vec3(0.025,0.025,0.20)
                     + */cnoise.xyz * 0.25 
                     + cnoise.aaa * 0.75;
   vec3 fvDrift      = fvCloudLayerDrift.zzw*(noise - vec3(0.0,0.0,0.5)) + vec3(0.0,0.0,1.0);
   
   CloudCoord       += fvDrift.xy;
   GroundCoord      += fvDrift.xy;
   ShadowCoord      += fvDrift.xy;
   
   vec4 fvCityLightBase   = cityLightTrigger(fNDotLB) * fvCityLightColor * fCityLightFactor;
   vec4 fvCityLights1     = texture2D( cityLights_20, CityCoord, fvCityLightCloudDiffusion.x ) * fvCityLightBase;
   vec4 fvCityLights4     = texture2D( cityLights_20, CityCoord, fvCityLightCloudDiffusion.y ) * fvCityLightBase;

   // Sample cloudmap
   vec2 gc1              =      CloudCoord                                         ;
   vec2 gc2              = lerp(CloudCoord,GroundCoord,0.25 * fCloudLayerThickness);
   vec2 gc3              = lerp(CloudCoord,GroundCoord,0.50 * fCloudLayerThickness);
   vec2 gc4              = lerp(CloudCoord,GroundCoord,0.75 * fCloudLayerThickness);
   vec2 gc5              = lerp(CloudCoord,GroundCoord,       fCloudLayerThickness);
   vec4 fvCloud1         = texture2D( cloudMap_20, gc1 );
   vec4 fvCloud2         = texture2D( cloudMap_20, gc2 );
   vec4 fvCloud3         = texture2D( cloudMap_20, gc3 );
   vec4 fvCloud4         = texture2D( cloudMap_20, gc4 );
   
   // Mask heights
   fvCloud1.a            = saturatef((fvCloud1.a*fvDrift.z-0.5000)*1.0); // 0.5000 - 1.0000
   fvCloud2.a            = saturatef((fvCloud2.a*fvDrift.z-0.2500)*4.0); // 0.2500 - 0.5000
   fvCloud3.a            = saturatef((fvCloud3.a*fvDrift.z-0.1250)*8.0); // 0.1250 - 0.2500
   fvCloud4.a            = saturatef((fvCloud4.a*fvDrift.z       )*8.0); // 0.0000 - 0.1250
   
   // Parallax - offset coords by relative displacement and resample
   #if (PARALLAX != 0)
   gc1                   = lerp(gc2,gc1,fvCloud1.a);
   gc2                   = lerp(gc3,gc2,fvCloud2.a);
   gc3                   = lerp(gc4,gc3,fvCloud3.a);
   gc4                   = lerp(gc5,gc4,fvCloud4.a);
   fvCloud1              = texture2D( cloudMap_20, gc1 );
   fvCloud2              = texture2D( cloudMap_20, gc2 );
   fvCloud3              = texture2D( cloudMap_20, gc3 );
   fvCloud4              = texture2D( cloudMap_20, gc4 );
   
   // Re-Mask heights
   fvCloud1.a            = saturatef((fvCloud1.a*fvDrift.z-0.5000)*1.0); // 0.5000 - 1.0000
   fvCloud2.a            = saturatef((fvCloud2.a*fvDrift.z-0.2500)*4.0); // 0.2500 - 0.5000
   fvCloud3.a            = saturatef((fvCloud3.a*fvDrift.z-0.1250)*8.0); // 0.1250 - 0.2500
   fvCloud4.a            = saturatef((fvCloud4.a*fvDrift.z       )*8.0); // 0.0000 - 0.1250
   #endif
   
   if (fvCloud4.a < 0.01) discard;
   
   // degamma cloud colors
   fvCloud1.rgb          = degamma_tex(fvCloud1.rgb);
   fvCloud2.rgb          = degamma_tex(fvCloud2.rgb);
   fvCloud3.rgb          = degamma_tex(fvCloud3.rgb);
   fvCloud4.rgb          = degamma_tex(fvCloud4.rgb);
   
   vec2 sc2              = lerp(gc2,ShadowCoord,0.25 * fCloudLayerThickness);
   vec2 sc3              = lerp(gc3,ShadowCoord,0.50 * fCloudLayerThickness);
   vec2 sc4              = lerp(gc4,ShadowCoord,0.75 * fCloudLayerThickness);
   float  fCloudShadow2  = texture2D( cloudMap_20, sc2, 0.5 ).a;
   float  fCloudShadow3  = texture2D( cloudMap_20, sc3, 1.0 ).a;
   float  fCloudShadow4  = texture2D( cloudMap_20, sc4, 1.5 ).a;
   
   
   // Simplified for ps2.a
   const vec3 shadowStep2 = vec3(0.500, 0.650, 0.800);
   const vec3 shadowStep3 = vec3(0.250, 0.500, 0.750);
   const vec3 shadowStep4 = vec3(0.125, 0.416, 0.708);
   vec3 fvCloudShadow    = vec3(fCloudShadow2,fCloudShadow3,fCloudShadow4) * fvDrift.zzz;
   fCloudShadow2         = saturatef( dot(fvCloudShadow - shadowStep2, vec3(0.5)) );
   fCloudShadow3         = saturatef( dot(fvCloudShadow - shadowStep3, vec3(0.5)) );
   fCloudShadow4         = saturatef( dot(fvCloudShadow - shadowStep4, vec3(0.5)) );
   
   // Compute self-shadowed cloud color
   vec3 fvAmbient         = gl_Color.rgb * ambientMapping(varWSNormal) * 0.5;
   vec4 fvBaseColor       = vec4(gl_Color.rgb * atmosphereLighting(fNDotL), gl_Color.a);
   vec3 fvCloud1s,fvCloud2s,fvCloud3s,fvCloud4s;
   vec3 fvCloud1c,fvCloud2c,fvCloud3c,fvCloud4c;
   vec4 fvCloud, fvCloudE;
   fvCloud1s               = fvCloud1.rgb;
   fvCloud2s               = fvCloud2.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(fCloudShadow2*CloudLayerDensitySVC.x));
   fvCloud3s               = fvCloud3.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(fCloudShadow3*CloudLayerDensitySVC.x));
   fvCloud4s               = fvCloud4.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(fCloudShadow4*CloudLayerDensitySVC.x));
   fvCloud1c               = fvCloud1.rgb*lerp(fvCityLights1,fvCityLights4,1.00).rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(3.0*CloudLayerDensitySVC.z));
   fvCloud2c               = fvCloud2.rgb*lerp(fvCityLights1,fvCityLights4,0.50).rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(2.0*CloudLayerDensitySVC.z));
   fvCloud3c               = fvCloud3.rgb*lerp(fvCityLights1,fvCityLights4,0.25).rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(1.0*CloudLayerDensitySVC.z));
   fvCloud4c               = fvCloud4.rgb*     fvCityLights1                    .rgb;
   fvCloud.a               = fvBaseColor.a*dot(fvCloudLayerMix,vec4(fvCloud1.a,fvCloud2.a,fvCloud3.a,fvCloud4.a));
   fvCloud.rgb             = fvCloud4s*fvBaseColor.rgb+fvCloud4c;
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud3s*fvBaseColor.rgb+fvCloud3c,saturatef(fvCloud3.a*CloudLayerDensitySVC.y));
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud2s*fvBaseColor.rgb+fvCloud2c,saturatef(fvCloud2.a*CloudLayerDensitySVC.y));
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud1s*fvBaseColor.rgb+fvCloud1c,saturatef(fvCloud1.a*CloudLayerDensitySVC.y));

   gl_FragColor = atmosphericScatter( fvAmbient, fvCloud, fNDotV, fNDotL, fVDotL );
}

