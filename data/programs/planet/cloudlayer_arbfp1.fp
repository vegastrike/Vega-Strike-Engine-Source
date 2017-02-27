#include "../fplod.h"

#include "gas_giants_params.h"
#include "../config.h"
#include "../stdlib.h"

#define inCloudCoord gl_TexCoord[0].xy
#define inGroundCoord gl_TexCoord[0].zw
#define inShadowCoord gl_TexCoord[1].xy
#define inNoiseCoord gl_TexCoord[1].zw

#define varTSLight (gl_TexCoord[4].xyz)
#define varTSView (gl_TexCoord[5].xyz)
#define varWSNormal (gl_TexCoord[6].xyz)

uniform sampler2D cosAngleToDepth_20;
uniform sampler2D cloudMap_20;
uniform sampler2D noiseMap_20;
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
   return expandPrecision(texture2DLod(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx),0.0)) * fAtmosphereThickness;
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   vec2 tc = clamp(vec2(fNDotV,fAtmosphereType),mn,mx);
   return texture2DLod(cosAngleToDepth_20,tc,0.0).a;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,fAtmosphereContrast*sqr(fNDotL))); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,fGroundContrast*fNDotL)); }

vec4 atmosphericScatter(vec3 atmo, vec3 amb, vec4 dif, float fNDotV, float fNDotL, float fVDotL)
{
   float  vdepth     = cosAngleToDepth(fNDotV) * sqr(saturatef(1.0-fShadowRelHeight.x));
   float  ldepth     = cosAngleToDepth(fNDotL+fAtmosphereAbsorptionOffset*6.0) * sqr(saturatef(1.0-fShadowRelHeight.x));
   float  alpha      = cosAngleToAlpha(saturatef(fNDotV * 0.95 - 0.05));
   
   vec3  lvabsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*(ldepth+vdepth*2.0)));
   
   vec4 rv;
   rv.rgb = regamma( amb + dif.rgb*lvabsorption );
   rv.a = saturatef(clamp(dif.a,0.0,2.0) * alpha);
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

   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   vec3 N = varWSNormal;
   
   float  fNDotL           = saturatef( L.z ); 
   float  fNDotV           = saturatef( V.z );
   float  fVDotL           = dot(L, V);
   
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
   
   // Sample cloudmap
   vec2 gc1              =      CloudCoord                                         ;
   vec2 gc2              = lerp(CloudCoord,GroundCoord,0.25 * fCloudLayerThickness);
   vec2 gc3              = lerp(CloudCoord,GroundCoord,0.50 * fCloudLayerThickness);
   vec2 gc4              = lerp(CloudCoord,GroundCoord,       fCloudLayerThickness);
   vec4 fvCloud1         = texture2D( cloudMap_20, gc1 );
   vec4 fvCloud2         = texture2D( cloudMap_20, gc2 );
   vec4 fvCloud3         = texture2D( cloudMap_20, gc3 );
   float H1 = fvCloud1.a;
   float H2 = fvCloud2.a;
   float H3 = fvCloud3.a;
   
   // Mask heights
   fvCloud1.a            = saturatef((fvCloud1.a*fvDrift.z-fvCloudLayers.x)*fvCloudLayerScales.x); // 0.5000 - 1.0000 (default)
   fvCloud2.a            = saturatef((fvCloud2.a*fvDrift.z-fvCloudLayers.y)*fvCloudLayerScales.y); // 0.2500 - 0.5000 (default)
   fvCloud3.a            = saturatef((fvCloud3.a*fvDrift.z-fvCloudLayers.z)*fvCloudLayerScales.z); // 0.1250 - 0.2500 (default)
   
   // Parallax - offset coords by relative displacement and resample
   #if (PARALLAX != 0)
   gc1                   = lerp(gc2,gc1,fvCloud1.a);
   gc2                   = lerp(gc3,gc2,fvCloud2.a);
   gc3                   = lerp(gc4,gc3,fvCloud3.a);
   fvCloud1              = texture2D( cloudMap_20, gc1 );
   fvCloud2              = texture2D( cloudMap_20, gc2 );
   fvCloud3              = texture2D( cloudMap_20, gc3 );
   
   // Re-Mask heights
   fvCloud1.a            = saturatef((fvCloud1.a*fvDrift.z-fvCloudLayers.x)*fvCloudLayerScales.x); // 0.5000 - 1.0000 (default)
   fvCloud2.a            = saturatef((fvCloud2.a*fvDrift.z-fvCloudLayers.y)*fvCloudLayerScales.y); // 0.2500 - 0.5000 (default)
   fvCloud3.a            = saturatef((fvCloud3.a*fvDrift.z-fvCloudLayers.z)*fvCloudLayerScales.z); // 0.1250 - 0.2500 (default)
   #endif
   
   if (fvCloud3.a < 0.01) discard;
   
   // degamma cloud colors
   fvCloud1.rgb          = degamma_tex(fvCloud1.rgb);
   fvCloud2.rgb          = degamma_tex(fvCloud2.rgb);
   fvCloud3.rgb          = degamma_tex(fvCloud3.rgb);
   
   vec2 sc1              =      gc1;
   vec2 sc2              = lerp(gc2,ShadowCoord,0.25 * fCloudLayerThickness);
   vec2 sc3              = lerp(gc3,ShadowCoord,       fCloudLayerThickness);
   vec3 scbias           = (vec3(1.0) - fvCloudLayers.xyz) * 2.0;
   
   float  fCloudShadow1  = texture2D( cloudMap_20, sc1, scbias.z+0.5).a;
   float  fCloudShadow2  = texture2D( cloudMap_20, sc2, scbias.y+0.5).a;
   float  fCloudShadow3  = texture2D( cloudMap_20, sc3, scbias.x+0.5).a;
   
   
   // Simplified for ps2.a
   const vec3 shadowStairs = vec3(0.000, 0.25, 0.70);
   vec3 shadowStep1 = vec3(fvCloudLayers.x) + shadowStairs * vec3(1.0 - fvCloudLayers.x);
   vec3 shadowStep2 = vec3(fvCloudLayers.y) + shadowStairs * vec3(1.0 - fvCloudLayers.y);
   vec3 shadowStep3 = vec3(fvCloudLayers.z) + shadowStairs * vec3(1.0 - fvCloudLayers.z);
   vec3 fvCloudShadow    = vec3(fCloudShadow1,fCloudShadow2,fCloudShadow3) * fvDrift.zzz;
   fCloudShadow1         = dot(saturate(fvCloudShadow - shadowStep1), vec3(0.33));
   fCloudShadow2         = dot(saturate(fvCloudShadow - shadowStep2), vec3(0.33));
   fCloudShadow3         = dot(saturate(fvCloudShadow - shadowStep3), vec3(0.33));
   fvCloudShadow         = vec3(fCloudShadow1,fCloudShadow2,fCloudShadow3);
   
   // Attack angle density adjustment   
   vec2 CloudLayerDensitySVC;
   float  fCloudLayerDensityL = fCloudLayerDensity / (abs(L.z)+fShadowRelHeight.x);
   float  fCloudLayerDensityV = fCloudLayerDensity / (abs(V.z)+fShadowRelHeight.x);
   CloudLayerDensitySVC.x     = fCloudLayerDensityL * fCloudSelfShadowFactor;
   CloudLayerDensitySVC.y     = fCloudLayerDensityV;
  
   // Compute self-shadowed cloud color
   vec3 fvAmbient         = gl_Color.rgb * fvCloud1.rgb * ambientMapping(varWSNormal) * 0.5;
   vec4 fvBaseColor       = vec4(gl_Color.rgb * atmosphereLighting(fNDotL), gl_Color.a);
   vec3 fvCloud1s,fvCloud2s,fvCloud3s,fvCloud4s;
   vec4 fvCloud, fvCloudE;
   fvCloudShadow           = saturate(fvCloudShadow * CloudLayerDensitySVC.xxx);
   fvCloud1s               = fvCloud1.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fvCloudShadow.x);
   fvCloud2s               = fvCloud2.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fvCloudShadow.y);
   fvCloud3s               = fvCloud3.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fvCloudShadow.z);
   fvCloud.a               = dot(fvCloudLayerMix.xyz,vec3(fvCloud1.a,fvCloud2.a,fvCloud3.a));
   fvCloud.rgb             = fvCloud3s;
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud2s,saturatef(fvCloud2.a*CloudLayerDensitySVC.y));
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud1s,saturatef(fvCloud1.a*CloudLayerDensitySVC.y));
   fvCloud.rgb            *= fvBaseColor.rgb;

   gl_FragColor = atmosphericScatter( fvCloud1.rgb, fvAmbient, fvCloud, fNDotV, fNDotL, fVDotL );
}

