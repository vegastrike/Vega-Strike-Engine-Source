#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

#define inCloudCoord gl_TexCoord[0]
#define inGroundCoord gl_TexCoord[1]
#define inShadowCoord gl_TexCoord[2]
#define inNoiseCoord gl_TexCoord[3]

#define varTSView (gl_TexCoord[5].xyz)
#define varTSLight (gl_TexCoord[6].xyz)
#define varWSNormal (gl_TexCoord[7].xyz)

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
   return expandPrecision(texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx),-8.0)) * fAtmosphereThickness;
}

float cosAngleToAlpha(float fNDotV)
{
   vec2 res = vec2(1.0) / vec2(1024.0,128.0);
   vec2 mn = res * 0.5;
   vec2 mx = vec2(1.0)-res * 0.5;
   return texture2D(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx)).a;
}

float  atmosphereLighting(float fNDotL) { return saturatef(min(1.0,2.0*fAtmosphereContrast*fNDotL)); }
float  groundLighting(float fNDotL) { return saturatef(min(1.0,2.0*fGroundContrast*fNDotL)); }

vec4 atmosphericScatter(vec3 amb, vec4 dif, float fNDotV, float fNDotL, float fVDotL)
{
   float  alpha      = cosAngleToAlpha(fNDotV);
   
   vec4 rv;
   
   vec3 absorption = lerp(fAtmosphereAbsorptionColor.rgb,vec3(1.0),saturatef(sqr(fNDotV*fNDotL*4.0)));
   float scattermuch = sqr(sqr(saturatef(1.0-fNDotV)));
   
   rv.rgb = regamma( amb + dif.rgb*absorption 
                  + atmosphereLighting(fNDotL)
                    *lerp(fMinScatterFactor, fMaxScatterFactor, scattermuch)
                    *fAtmosphereScatterColor.rgb );
   rv.a = dif.a * alpha;
   return rv;
}

vec3 ambientMapping( in vec3 direction )
{
   return gl_LightSource[0].ambient.rgb;
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

   // Attack angle density adjustment   
   vec3 CloudLayerDensitySVC;
   float  fCloudLayerDensityL = fCloudLayerDensity / (abs(L.z)+0.01);
   float  fCloudLayerDensityV = fCloudLayerDensity / (abs(V.z)+0.01);
   CloudLayerDensitySVC.x     = fCloudLayerDensityL * fCloudSelfShadowFactor;
   CloudLayerDensitySVC.y     = fCloudLayerDensityV;
   CloudLayerDensitySVC.z     = fCloudLayerDensity * fCloudSelfShadowFactor;
  
   // Sample cloudmap
   vec2 gc1              =      CloudCoord                                         ;
   vec2 gc4              = lerp(CloudCoord,GroundCoord,0.75 * fCloudLayerThickness);
   vec4 fvCloud1         = texture2D( cloudMap_20, gc1 );
   vec4 fvCloud4         = texture2D( cloudMap_20, gc4 );
   
   vec2 sc4              = lerp(gc4,ShadowCoord,0.75 * fCloudLayerThickness);
   float  fCloudShadow4  = texture2D( cloudMap_20, sc4, 1.5 ).a;
   
   // Simplified for ps2.a
   fCloudShadow4         = saturatef( (fCloudShadow4 - fvCloud1.a) * 0.75 );
   
   // Mask heights
   fvCloud1.a            = saturatef((fvCloud1.a-0.5000)*1.0); // 0.5000 - 1.0000
   fvCloud4.a            = saturatef((fvCloud4.a       )*2.0); // 0.0000 - 0.5000
   
   // Compute self-shadowed cloud color
   vec3 fvAmbient         = gl_Color.rgb * ambientMapping(N) * 0.5;
   vec4 fvBaseColor       = vec4(gl_Color.rgb * atmosphereLighting(fNDotL), gl_Color.a);
   vec3 fvCloud1s,fvCloud4s;
   vec4 fvCloud;
   fvCloud1s               = fvCloud1.rgb;
   fvCloud4s               = fvCloud4.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,saturatef(fCloudShadow4*CloudLayerDensitySVC.x));
   fvCloud.a               = fvBaseColor.a*dot(fvCloudLayerMix,vec4(fvCloud1.a,fvCloud1.a,fvCloud4.a,fvCloud4.a));
   fvCloud.rgb             = fvCloud4s*fvBaseColor.rgb;
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud1s*fvBaseColor.rgb,saturatef(fvCloud1.a*CloudLayerDensitySVC.y));

   gl_FragColor = atmosphericScatter( fvAmbient, fvCloud, fNDotV, fNDotL, fVDotL );
}

