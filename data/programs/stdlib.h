#include "config.h"

#define GAMMA_OFFSET 0.0
#define GAMMA_OFFSET2 (GAMMA_OFFSET*GAMMA_OFFSET)

#if (DEGAMMA != 0)
vec4  degammac( in vec4 a ) { a.rgb *= a.rgb; return a; }
vec3  degammac( in vec3 a ) { a.rgb *= a.rgb; return a; }
vec4  degamma( in vec4 a ) { return a*a; }
vec3  degamma( in vec3 a ) { return a*a; }
vec2  degamma( in vec2 a ) { return a*a; }
float degamma( in float a) { return a*a; }
#else
vec4  degammac( in vec4 a ) { return a; }
vec3  degammac( in vec3 a ) { return a; }
vec4  degamma( in vec4 a ) { return a; }
vec3  degamma( in vec3 a ) { return a; }
vec2  degamma( in vec2 a ) { return a; }
float degamma( in float a) { return a; }
#endif

#if (REGAMMA != 0)
vec4  regammac( in vec4 a ) { a.rgb = sqrt(a.rgb+vec3(GAMMA_OFFSET2))-vec3(GAMMA_OFFSET); return a; }
vec3  regammac( in vec3 a ) { a.rgb = sqrt(a.rgb+vec3(GAMMA_OFFSET2))-vec3(GAMMA_OFFSET); return a; }
vec4  regamma( in vec4 a ) { return sqrt(a+vec4(GAMMA_OFFSET2))-vec4(GAMMA_OFFSET); }
vec3  regamma( in vec3 a ) { return sqrt(a+vec3(GAMMA_OFFSET2))-vec3(GAMMA_OFFSET); }
vec2  regamma( in vec2 a ) { return sqrt(a+vec2(GAMMA_OFFSET2))-vec2(GAMMA_OFFSET); }
float regamma( in float a) { return sqrt(a+GAMMA_OFFSET2)-GAMMA_OFFSET; }
#else
vec4  regammac( in vec4 a ) { return a; }
vec3  regammac( in vec3 a ) { return a; }
vec4  regamma( in vec4 a ) { return a; }
vec3  regamma( in vec3 a ) { return a; }
vec2  regamma( in vec2 a ) { return a; }
float regamma( in float a) { return a; }
#endif

#if (DEGAMMA_ENVIRONMENT != 0)
    #define degamma_env degammac
#else
    #define degamma_env 
#endif

#if (DEGAMMA_SPECULAR != 0)
    #define degamma_spec degamma
#else
    #define degamma_spec
#endif

#if (DEGAMMA_GLOW_MAP != 0)
    #define degamma_glow degammac
#else
    #define degamma_glow
#endif

#if (DEGAMMA_LIGHTS != 0)
    #define degamma_light degammac
#else
    #define degamma_light
#endif

#if (DEGAMMA_TEXTURES != 0)
    #define degamma_tex degamma
#else
    #define degamma_tex
#endif

vec4  sqr( in vec4 a )     { return a*a; }
vec3  sqr( in vec3 a )     { return a*a; }
vec2  sqr( in vec2 a )     { return a*a; }
float sqr( in float a )    { return a*a; }


float lerp(float a, float b, float t) { return a+t*(b-a); }
vec2 lerp(vec2 a, vec2 b, float t) { return a+t*(b-a); }
vec3 lerp(vec3 a, vec3 b, float t) { return a+t*(b-a); }
vec4 lerp(vec4 a, vec4 b, float t) { return a+t*(b-a); }

float  saturatef(float x) { return clamp(x,0.0,1.0); }
vec2   saturate(vec2 x) { return clamp(x,0.0,1.0); }
vec3   saturate(vec3 x) { return clamp(x,0.0,1.0); }
vec4   saturate(vec4 x) { return clamp(x,0.0,1.0); }

float fresnel(float fNDotV, float fresnelEffect)
{
   return sqr(1.0-lerp(0.0,fNDotV,fresnelEffect));
}

float twosided_fresnel( in float cosa, in float k, in float two_sided )
{
   float tmp1 = sqrt(1.0-(1.0-cosa*cosa)/(k*k));
   float tmp2 = k*cosa;
   float tmp3 = k*tmp1;
   float tmp4 = (tmp1-tmp2)/(tmp1+tmp2+0.0001);
   tmp1 = (cosa-tmp3)/(cosa+tmp3+0.0001);
   tmp2 = 0.5*(tmp1*tmp1+tmp4*tmp4);
   //That'd ne the final Fresnel value, in tmp2. But we got two surfaces to
   //a glass pane: outer and inner. And the inner reflection is equally as
   //strong as the outer. I'll look for a multi-bounce solution; but for now
   //we'll square the refractivity, and convert back to reflectivity; then
   //average the two for a rough multi-bounce approximation.
   tmp3 = 1.0 - tmp2;
   tmp4 = 1.0 - tmp3*tmp3;
   return lerp(tmp2, tmp4, 0.5 * two_sided);
}

float full_fresnel( in float cosa, in float k )
{
   return twosided_fresnel( cosa, k, 1.0 );
}


float  luma(vec3 color) { return dot( color, vec3(1.0/3.0, 1.0/3.0, 1.0/3.0) ); }

vec4 expand(vec4 x)   { return x*2.0-1.0; }
vec3 expand(vec3 x)   { return x*2.0-1.0; }
vec2 expand(vec2 x)   { return x*2.0-1.0; }
float  expand(float  x)   { return x*2.0-1.0; }

vec4 bias(vec4 x)     { return x*0.5+0.5; }
vec3 bias(vec3 x)     { return x*0.5+0.5; }
vec2 bias(vec2 x)     { return x*0.5+0.5; }
float  bias(float  x)     { return x*0.5+0.5; }

float  self_shadow(float x) { return (x>0.0)?1.0:0.0; }
float  self_shadow_smooth(float x) { return saturatef(2.0*x); }
float  self_shadow_smooth_ex(float x) { return saturatef(4.0*x); }

float soft_min(float m, float x)
{
   const float hpi_i = 0.63661977236758134307553505349006;
   float xm = x/m;
   float softx = min(m,m*1.25*hpi_i*atan(xm));
   return lerp(x, softx, saturatef(xm));
}

float maxof(vec2 x) { return max(x.x, x.y); }
float maxof(vec3 x) { return max(x.x, max(x.y, x.z)); }
float maxof(vec4 x) { return maxof(max(x.xy, x.zw)); }

float minof(vec2 x) { return min(x.x, x.y); }
float minof(vec3 x) { return min(x.x, min(x.y, x.z)); }
float minof(vec4 x) { return minof(min(x.xy, x.zw)); }

