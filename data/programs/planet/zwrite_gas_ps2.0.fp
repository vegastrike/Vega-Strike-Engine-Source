#include "../fplod.h"

#include "gas_giants_params.h"
#include "../config.h"
#include "../stdlib.h"

#define inCloudCoord gl_TexCoord[0]
#define inGroundCoord gl_TexCoord[1]
#define inNoiseCoord gl_TexCoord[2]

#define varTSView gl_TexCoord[3]

uniform sampler2D cosAngleToDepth_20;
uniform sampler2D cloudMap_20;
uniform sampler2D noiseMap_20;

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

vec4 atmosphericScatter(float dif, float fNDotV)
{
   float  vdepth     = cosAngleToDepth(fNDotV) * sqr(saturatef(1.0-fShadowRelHeight.x));
   float  alpha      = cosAngleToAlpha(saturatef(fNDotV * 0.95 - 0.05));
   
   vec4 rv;
   rv.a = saturatef(dif) * alpha;
   rv.rgb = vec3(0.0);
   
   if (rv.a < 0.99) discard;
   
   return rv;
}


void main()
{    
   vec2 CloudCoord = inCloudCoord.xy;
   vec2 GroundCoord = inGroundCoord.xy;
   vec2 NoiseCoord = inNoiseCoord.xy;

   vec3 V = normalize(varTSView.xyz);
   
   float  fNDotV           = saturatef( V.z );

   // Drift noise
   vec4 cnoise       = texture2D(noiseMap_20,NoiseCoord);
   vec4 hcnoise      = texture2D(noiseMap_20,NoiseCoord*7.0);
   vec3 noise        = /*hcnoise.xyz * vec3(0.025,0.025,0.20)
                     + */cnoise.xyz * 0.25 
                     + cnoise.aaa * 0.75;
   vec3 fvDrift      = fvCloudLayerDrift.zzw*(noise - vec3(0.0,0.0,0.5)) + vec3(0.0,0.0,1.0);
   
   CloudCoord       += fvDrift.xy;
   GroundCoord      += fvDrift.xy;
   
   // Sample cloudmap
   vec2 gc1              =      CloudCoord                                         ;
   vec2 gc2              = lerp(CloudCoord,GroundCoord,0.25 * fCloudLayerThickness);
   vec2 gc3              = lerp(CloudCoord,GroundCoord,0.50 * fCloudLayerThickness);
   vec2 gc4              = lerp(CloudCoord,GroundCoord,       fCloudLayerThickness);
   vec3 fvCloudA;
   fvCloudA.x            = texture2D( cloudMap_20, gc1 ).a;
   fvCloudA.y            = texture2D( cloudMap_20, gc2 ).a;
   fvCloudA.z            = texture2D( cloudMap_20, gc3 ).a;
   
   // Mask heights
   fvCloudA              = saturate((fvCloudA*fvDrift.z-fvCloudLayers.xyz)*fvCloudLayerScales.xyz);
   
   // Parallax - offset coords by relative displacement and resample
   #if (PARALLAX != 0)
   gc1                   = lerp(gc2,gc1,fvCloudA.x);
   gc2                   = lerp(gc3,gc2,fvCloudA.y);
   gc3                   = lerp(gc4,gc3,fvCloudA.z);
   fvCloudA.x            = texture2D( cloudMap_20, gc1 ).a;
   fvCloudA.y            = texture2D( cloudMap_20, gc2 ).a;
   fvCloudA.z            = texture2D( cloudMap_20, gc3 ).a;
   
   // Re-Mask heights
   fvCloudA              = saturate((fvCloudA*fvDrift.z-fvCloudLayers.xyz)*fvCloudLayerScales.xyz);
   #endif
   
   if (fvCloudA.z < 0.01) discard;
   
   // Compute self-shadowed cloud color
   float fvCloud         = dot(fvCloudLayerMix.xyz,fvCloudA);
   gl_FragColor = atmosphericScatter( fvCloud, fNDotV );
}

