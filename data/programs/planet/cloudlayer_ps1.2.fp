#include "gas_giants_params.h"
#include "../config.h"
#include "../stdlib.h"

#define inCloudCoord gl_TexCoord[0].xy
#define inGroundCoord gl_TexCoord[0].zw
#define inShadowCoord gl_TexCoord[1].xy
#define inNoiseCoord gl_TexCoord[1].zw

#define varTSLight (gl_TexCoord[3].xyz)
#define varTSView (gl_TexCoord[4].xyz)
#define varWSNormal (gl_TexCoord[5].xyz)
#define varCloudLayerDensitySVC (gl_TexCoord[6].xyz)

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

vec4 atmosphericScatter(vec4 dif, vec3 ambient, float fNDotV, float fNDotL, float fVDotL)
{
   float alpha = cosAngleToAlpha(saturatef(fNDotV * 0.95 - 0.05));
   vec4 rv;
   rv.rgb = regamma( dif.rgb + ambient );
   rv.a = saturatef(dif.a) * alpha;
   return rv;
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
   H                     = saturate((H - fvCloudLayers.xy)*fvCloudLayerScales.xy);
   
   // 1.2 doesn't have multiple layer passes... so useless
   //if (H.y < 0.01) discard;
   
   // degamma cloud colors
   fvCloud1.rgb          = degamma_tex(fvCloud1.rgb);
   fvCloud2.rgb          = degamma_tex(fvCloud2.rgb);
   
   vec2 sc2              = lerp(gc2,ShadowCoord,0.75 * fCloudLayerThickness);
   float scbias         = saturatef(1.0 - H.y) * 1.5;
   
   float  fCloudShadow2  = texture2D( cloudMap_20, sc2, scbias+0.5).a;
   
   
   // Simplified for ps2.a
   float shadowStep2    = fvCloud2.a;
   fCloudShadow2         = saturatef(fCloudShadow2 - shadowStep2) * 1.5;
   
   // Attack angle density adjustment   
   vec3 CloudLayerDensitySVC = varCloudLayerDensitySVC;
  
   // Compute self-shadowed cloud color
   vec3 fvAmbient         = gl_SecondaryColor.rgb * fvCloud1.rgb;
   vec4 fvBaseColor       = degamma(gl_Color);
   vec3 fvCloud1s,fvCloud2s;
   vec4 fvCloud;
   fCloudShadow2           = saturatef(fCloudShadow2 * CloudLayerDensitySVC.x);
   fvCloud1s               = fvCloud1.rgb;
   fvCloud2s               = fvCloud2.rgb*lerp(vec3(1.0),fvCloudSelfShadowColor.rgb,fCloudShadow2);
   fvCloud.a               = dot(fvCloudLayerMix.xy,H);
   fvCloud.rgb             = fvCloud2s;
   fvCloud.rgb             = lerp(fvCloud.rgb,fvCloud1s,saturatef(H.x*CloudLayerDensitySVC.y));
   fvCloud.rgb            *= fvBaseColor.rgb;

   gl_FragColor = atmosphericScatter( fvCloud, fvAmbient, fNDotV, fNDotL, fVDotL );
}

