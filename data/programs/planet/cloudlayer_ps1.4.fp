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
   return expandPrecision(texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx),-8.0)) * fAtmosphereThickness;
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   vec2 tc = clamp(vec2(fNDotV,fAtmosphereType),mn,mx);
   return texture2D(cosAngleToDepth_20,tc,-8.0).a;
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
   rv.a = saturatef(dif.a) * alpha;
   return rv;
}

vec3 ambientMapping( in vec3 direction )
{
   return degamma_env(textureCube(envMap, direction, 8.0)).rgb;
}


void main()
{    
   vec2 CloudCoord = inCloudCoord;
   vec2 GroundCoord = inGroundCoord;
   vec2 ShadowCoord = inShadowCoord;
   vec2 NoiseCoord = inNoiseCoord;

   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   vec3 N = varWSNormal;
   
   float  fNDotL           = saturatef( L.z ); 
   float  fNDotV           = saturatef( V.z );
   float  fVDotL           = dot(L, V);
   
   // Sample cloudmap
   vec2 gc1              =      CloudCoord                                         ;
   vec2 gc2              = lerp(CloudCoord,GroundCoord,0.75 * fCloudLayerThickness);
   vec4 fvCloud1         = texture2D( cloudMap_20, gc1 );
   vec4 fvCloud2         = texture2D( cloudMap_20, gc2 );
   vec2 H                = vec2(fvCloud1.a, fvCloud2.a);
   
   // Mask heights
   H                     = (H - fvCloudLayers.xy)*fvCloudLayerScales.xy;
   
   // 1.4 doesn't have multiple layer passes, so useless
   // if (H.y < 0.01) discard;
   
   // degamma cloud colors
   fvCloud1.rgb          = degamma_tex(fvCloud1.rgb);
   fvCloud2.rgb          = degamma_tex(fvCloud2.rgb);
   
   vec2 sc1              =      gc1;
   vec2 sc2              = lerp(gc2,ShadowCoord,0.75 * fCloudLayerThickness);
   vec2 scbias           = (vec2(1.0) - fvCloudLayers.xy) * 2.0;
   
   float  fCloudShadow1  = texture2D( cloudMap_20, sc1, scbias.y+0.5).a;
   float  fCloudShadow2  = texture2D( cloudMap_20, sc2, scbias.x+0.5).a;
   
   
   // Simplified for ps2.a
   vec2 shadowStep1 = vec2(fvCloudLayers.x) + vec2(0.00, 0.70) * vec2(1.0 - fvCloudLayers.x);
   vec2 shadowStep2 = vec2(fvCloudLayers.y) + vec2(0.00, 0.70) * vec2(1.0 - fvCloudLayers.y);
   vec2 fvCloudShadow    = vec2(fCloudShadow1,fCloudShadow2);
   fCloudShadow1         = dot(saturate(fvCloudShadow - shadowStep1), vec2(0.5));
   fCloudShadow2         = dot(saturate(fvCloudShadow - shadowStep2), vec2(0.5));
   fvCloudShadow         = vec2(fCloudShadow1,fCloudShadow2);
   
   // Attack angle density adjustment   
   vec2 CloudLayerDensitySVC;
   float  fCloudLayerDensityL = fCloudLayerDensity / (abs(L.z)+0.01);
   float  fCloudLayerDensityV = fCloudLayerDensity / (abs(V.z)+0.01);
   CloudLayerDensitySVC.x     = fCloudLayerDensityL * fCloudSelfShadowFactor;
   CloudLayerDensitySVC.y     = fCloudLayerDensityV;
  
   // Compute self-shadowed cloud color
   vec3 fvAmbient         = gl_Color.rgb * fvCloud1.rgb * ambientMapping(varWSNormal) * 0.5;
   vec4 fvBaseColor       = vec4(gl_Color.rgb * atmosphereLighting(fNDotL), gl_Color.a);
   vec3 fvCloud1s,fvCloud2s;
   vec4 fvCloud, fvCloudE;
   fvCloudShadow           = saturate(fvCloudShadow * CloudLayerDensitySVC.xx);
   fvCloud1s               = fvCloud1.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fvCloudShadow.x);
   fvCloud2s               = fvCloud2.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fvCloudShadow.y);
   fvCloud.a               = dot(fvCloudLayerMix.xy,H);
   fvCloud.rgb             = fvCloud2s;
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud1s,saturatef(H.x*CloudLayerDensitySVC.y));
   fvCloud.rgb            *= fvBaseColor.rgb;

   gl_FragColor = atmosphericScatter( fvCloud1.rgb, fvAmbient, fvCloud, fNDotV, fNDotL, fVDotL );
}

