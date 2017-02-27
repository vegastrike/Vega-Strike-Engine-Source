#version 130

#include "../fplod.h"

// Default precision qualifier - make pedantic drivers happy,
// since OpenGL specs clearly specify they have no meaning except in OpenGL ES
precision highp float;

#include "earth_params.h"
#include "../config.h"
#include "../stdlib.h"

uniform sampler2D specularMap_20;
uniform sampler2D baseMap_20;
uniform sampler2D cityLights_20;
uniform sampler2D cosAngleToDepth_20;
uniform sampler2D cloudMap_20;
uniform sampler2D noiseMap_20;
uniform sampler2D normalMap_20;

varying vec3 varTSLight;
varying vec3 varTSView;

float  cityLightTrigger(float fNDotLB) { return saturatef(4.0*fNDotLB); }

float fresnel(float fNDotV)
{
   return fresnel(fNDotV, fFresnelEffect.x);
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
   return expandPrecision(textureGrad(cosAngleToDepth_20,clamp(vec2(fNDotV,fAtmosphereType),mn,mx),vec2(0.0),vec2(0.0))) * fAtmosphereThickness;
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

float twilight(float fNDotL)
{
   return saturatef(abs(fNDotL*10.0));
}

vec3 rayleighAmbient(float fNDotL, float fVDotL)
{
   float  ldepth     = cosAngleToDepth(abs(fNDotL));
   return reyleigh(fVDotL,ldepth);
}

vec4 atmosphericScatter(vec4 dif, float fNDotV, float fNDotL, float fVDotL, vec3 fvShadow, vec3 fvAtmShadow)
{
   float  vdepth     = cosAngleToDepth(fNDotV);
   float  ldepth     = cosAngleToDepth(fNDotL+fAtmosphereAbsorptionOffset);
   float  alpha      = saturatef(2.0 * (cosAngleToAlpha(fNDotV) - 0.5));
   
   #if SCATTER
   vec3  labsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*ldepth));
   vec3  vabsorption = pow(fAtmosphereAbsorptionColor.rgb,vec3(fAtmosphereAbsorptionColor.a*vdepth));
   vec3  lscatter    = gl_LightSource[0].diffuse.rgb 
                       * fAtmosphereScatterColor.rgb 
                       * pow(labsorption,vec3(fSelfShadowFactor)) 
                       * (fMinScatterFactor+soft_min(fMaxScatterFactor-fMinScatterFactor,vdepth))
                     + reyleigh(fVDotL, ldepth);
   #else
   const vec3 labsorption = vec3(1.0);
   const vec3 vabsorption = vec3(1.0);
   const vec3 lscatter    = vec3(0.0);
   #endif
   
   vec4 rv;
   rv.rgb = regamma( (dif.rgb*labsorption*fvShadow)*vabsorption 
                  + atmosphereLighting(fNDotL)
                    *lscatter
                    *lerp(vec3(1.0),fvAtmShadow,fAtmosphereShadowInfluence.x) );
   rv.a = dif.a * alpha;
   return rv;
}

void main()
{      
   vec2 texcoord = gl_TexCoord[0].xy;
   vec4 shadowcoord = gl_TexCoord[1];
   
   vec3 L = normalize(varTSLight);
   vec3 V = normalize(varTSView);
   float  HG  = texture2D( normalMap_20, texcoord ).a;
   float  HG0 = HG * fBumpScale.x;
   
   vec2 offs             = (HG0 / V.z) * V.xy;
   texcoord             += offs;
   shadowcoord.xy       += offs;
   vec3 tN = expand( texture2D( normalMap_20, texcoord ).rgb ) * vec3(-1.0,1.0,1.0); // Do not normalize, to avoid aliasing
   
   vec4 rnoise = expand( texture2D( noiseMap_20, texcoord * fBumpScale.y * 5.0 ) );
   vec2 noise = (rnoise.xy * 0.25 + 0.75 * (rnoise.yz * rnoise.w)) * (0.2 + HG);
   vec3 dN = normalize( vec3( noise * fBumpScale.z, 1 ) ) - vec3(0.0,0.0,1.0);
   
   vec3 N = dN + tN;
   
   vec3 R = -reflect(L,N);
   
   float  fNDotL           = saturatef( dot(N,L) ); 
   float  fNDotLs          = saturatef( L.z ); 
   float  fNDotLf          = L.z; 
   float  fNDotLB          = saturatef(-L.z + fCityLightTriggerBias.x);
   float  fRDotV           = saturatef( dot(R,V) );
   float  fNDotV           = saturatef( dot(N,V) );
   float  fNDotVs          = saturatef( V.z );
   float  fVDotL           = dot(L,V);
   
   vec4 fvTexColor         = texture2D( baseMap_20, texcoord );
   fvTexColor.rgb          = degamma_tex(fvTexColor.rgb);
   
   vec4 cnoise             = texture2D(noiseMap_20,gl_TexCoord[2].xy);
   vec3 fvDrift            = fvCloudLayerDrift_ShadowRelHeight.zzw*(cnoise.xyw * 0.25 + cnoise.aaa * 0.75 - vec3(0.0,0.0,0.5)) + vec3(0.0,0.0,1.0);
   shadowcoord            += fvDrift.xyxy;
   
   vec2 scddx              = (shadowcoord.xy-shadowcoord.zw)*0.25;
   vec2 sc1                = shadowcoord.zw + 1.0*scddx;
   vec2 sc2                = shadowcoord.zw + 2.0*scddx;
   vec2 sc3                = shadowcoord.zw + 3.0*scddx;
   float  fGShadow         = texture2D( cloudMap_20, shadowcoord.xy ).a;
   float  fCReflection     = texture2D( cloudMap_20, shadowcoord.xy, 2.0 ).a;
   float  fAShadow         = textureGrad( cloudMap_20, sc1, dFdx(sc1)+2.0*scddx, dFdy(sc1) ).a;
   fAShadow               += textureGrad( cloudMap_20, sc2, dFdx(sc2)+2.0*scddx, dFdy(sc2) ).a;
   fAShadow               += textureGrad( cloudMap_20, sc3, dFdx(sc3)+2.0*scddx, dFdy(sc3) ).a;
   fAShadow               *= 0.33*fvDrift.z*fCloudLayerDensity;
   fGShadow               *= fvDrift.z*fCloudLayerDensity;
   
   vec3 fvGShadow          = lerp( vec3(1.0), fvShadowColor.rgb, saturatef(fGShadow) );
   vec3 fvAShadow          = lerp( vec3(1.0), fvShadowColor.rgb, saturatef(fAShadow) );
   
   vec4 fvSpecular         = degamma_tex(texture2D( specularMap_20, texcoord ));
   fvSpecular.rgb         *= fresnel(fNDotV);
   fRDotV                  = pow( fRDotV, fShininess.x*(0.01+0.99*fvSpecular.a)*256.0 );
   fvSpecular              = fvSpecular * gl_SecondaryColor * fRDotV;

   vec4 fvBaseColor;
   fvBaseColor.rgb         = gl_Color.rgb * groundLighting(fNDotL) * self_shadow(fNDotLs);
   fvBaseColor.a           = gl_Color.a;
   
   vec4 dif                = fvBaseColor * fvTexColor;
   vec4 spec               = 4.0*fvSpecular*self_shadow_smooth_ex(fNDotLs);

   gl_FragColor = atmosphericScatter( dif+spec, fNDotVs, fNDotLs, fVDotL, fvGShadow, fvAShadow );
}


