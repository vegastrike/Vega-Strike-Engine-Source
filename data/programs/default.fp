#include "config.h"
#include "stdlib.h"
#include "normalmap.h"

uniform int light_enabled[gl_MaxLights];
uniform int max_light_enabled;
uniform sampler2D diffuseMap;
uniform samplerCube envMap;
uniform sampler2D specMap;
uniform sampler2D glowMap;
uniform sampler2D normalMap;
uniform sampler2D damageMap;
uniform sampler2D detail0Map;
uniform sampler2D detail1Map;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;

vec3 matmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return normalize(vec3(dot(lightVec,tangent),dot(lightVec,binormal),dot(lightVec,normal)));
}
vec3 imatmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return normalize(lightVec.xxx*tangent+lightVec.yyy*binormal+lightVec.zzz*normal);
}


//GLOSS class
void GLOSS_init( inout vec4 mat_gloss, in float gloss_in  )
{
  //decrease resolution at high end of input range; --LOD 0 to LOD 1 difference is hardly noticeable
  //except on glossy AND very flat surfaces; whereas at the low end the resolution is more critical
  //See the forum posts indicate at the top of this file for the math derivation
  mat_gloss.x = 0.5 * ( gloss_in + gloss_in*gloss_in ); //relinearized input
  mat_gloss.y  = 7.288828847 * ( 1.0 - mat_gloss.x ); //mip bias
  mat_gloss.z = PI_OVER_3 * pow( 2.5, (mat_gloss.y-8.0) ); //blur radius angle
  mat_gloss.w = TWO_PI * ( 1.0 - cos(mat_gloss.z) ); //blur solid angle
}
//private:
float GLOSS_power( in float mat_gloss_sa, in float light_solid_angle ) //const
{
  // shininess = ( 0.5 * pi / SolidAngle ) - 0.810660172
  const float MAGIC_TERM = 0.810660172;
  return ( HALF_PI / (mat_gloss_sa + light_solid_angle) ) - MAGIC_TERM;
}
//public:
#if (SUPRESS_ENVIRONMENT == 0)
//called GLOSS_... because it's part of the GLOSS class; but with no gloss for default
vec3 GLOSS_env_reflection( in vec3 direction ) //const
{
  //ENV MAP FETCH:
  vec3 result = textureCube( envMap, direction ).rgb;
  return degamma_env(result);
}
#endif
float GLOSS_phong_reflection( in float mat_gloss_sa, in float RdotL, in float light_solid_angle ) //const
{
  float shininess = GLOSS_power( mat_gloss_sa, light_solid_angle );
  //Below, multiplying by 3.621 would be correct; but the brightness is ridiculous in practice...
  //Well, no; correct only if we assume a chalk sphere and a chrome sphere throw the same total
  //ammount of light into your eye...
  return max( 0.0, pow( RdotL, shininess ) * shininess );
}

float diffuse_soft_dot(in vec3 normal, in vec3 light, in float light_sa)
{
    float NdotL = dot(normal, light);
    float normalized_sa = light_sa / TWO_PI;
    return (NdotL + normalized_sa) / (1.0 + normalized_sa);
}

//PER-LIGHT STUFF
void lightingLight(
   in vec4 lightinfo, in vec3 normal, in vec3 vnormal, in vec3 reflection, 
   in vec3 raw_light_col,
   in float mat_gloss_sa,
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc)
{
   vec3  light_pos = normalize(lightinfo.xyz);
   float light_sa = lightinfo.w;
   vec3  light_col = degamma_light(raw_light_col);

   float VNdotLx4= saturatef( 4.0 * diffuse_soft_dot(vnormal,light_pos,light_sa) );
   float NdotL = clamp( diffuse_soft_dot(normal,light_pos,light_sa), 0.0, VNdotLx4 );
   float RdotL = clamp( dot(reflection,light_pos), 0.0, VNdotLx4 );
   light_acc += light_col;
   float phong  = GLOSS_phong_reflection(mat_gloss_sa,RdotL,light_sa);
   specular_acc += ( phong * light_col );
   diffuse_acc  += ( NdotL * light_col );
}

#define lighting(name, lightno_gl, lightno_tex) \
void name( \
   in vec3 normal, in vec3 vnormal, in  vec3 reflection, \
   in float mat_gloss_sa, \
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc) \
{ \
   lightingLight( \
      gl_TexCoord[lightno_tex], normal, vnormal, reflection, \
      gl_LightSource[lightno_gl].diffuse.rgb, \
      mat_gloss_sa, \
      light_acc, diffuse_acc, specular_acc); \
}

lighting(lighting0, 0, 5)
lighting(lighting1, 1, 6)


void main() 
{
  // Retreive texture coordinates
  vec2 tex_coord = gl_TexCoord[0].xy;
  
  // Retrieve vectors
  vec3 iNormal=gl_TexCoord[1].xyz;
  vec3 iTangent=gl_TexCoord[2].xyz;
  vec3 iBinormal=gl_TexCoord[3].xyz;
  vec3 vnormal=normalize(iNormal);
  //vec3 normal=imatmul(iTangent,iBinormal,iNormal,expand(texture2D(normalMap,tex_coord).yxz)*vec3(-1.0,1.0,1.0));
  vec3 normal=imatmul(iTangent,iBinormal,iNormal,normalmap_decode(texture2D(normalMap,tex_coord)));
  
  // Other vectors
  vec3 eye = gl_TexCoord[4].xyz;
  
  // Sample textures
  vec4 damagecolor = texture2D(damageMap , tex_coord);
  vec4 diffcolor   = texture2D(diffuseMap, tex_coord);
  vec4 speccolor   = texture2D(specMap   , tex_coord);
  vec4 glowcolor   = texture2D(glowMap   , tex_coord);
  
  // De-gamma input textures
  damagecolor.rgb  = degamma_tex(damagecolor.rgb);
  diffcolor.rgb    = degamma_tex(diffcolor.rgb);
  speccolor.rgb    = degamma_spec(speccolor.rgb);
  glowcolor.rgb    = degamma_glow(glowcolor.rgb);

  //better apply damage lerps after de-gamma-ing
  diffcolor.rgb  = lerp(diffcolor, damagecolor, damage.x).rgb;
  speccolor     *= (1.0-damage.x);
  glowcolor.rgb *= (1.0-damage.x);
  
  //materials
  vec4 mtl_gloss;
  vec3 diff_col, mspec_col, glow_col;
  float alpha, UAO;
  //grab alpha channels  
  alpha = diffcolor.a;
  UAO = glowcolor.a;

  diff_col = diffcolor.rgb;
  mspec_col = speccolor.rgb;
  glow_col = glowcolor.rgb;
  
  //compute gloss-related stuff
#if (SHININESS_FROM == AD_HOC_SHININESS)
  float crapgloss = saturatef(0.5*dot(mspec_col,vec3(0.3,1.0,0.7)));
  GLOSS_init( mtl_gloss, 0.1 + 0.4 * sqr(crapgloss) );
#endif
#if (SHININESS_FROM == GLOSS_IN_SPEC_ALPHA)
  GLOSS_init( mtl_gloss, speccolor.a );
#endif
  
  //reflection
  vec3 reflection = -reflect(eye,normal);

  // Init lighting accumulators
  vec3 light_acc    = vec3(0.0);
  vec3 diffuse_acc  = vec3(0.0);
  vec3 specular_acc = vec3(0.0);
  // Do lighting for every active light
  float mtl_gloss_sa = mtl_gloss.w;
  if (light_enabled[0] != 0)
     lighting0(normal, vnormal, reflection, mtl_gloss_sa, light_acc, diffuse_acc, specular_acc);
  if (light_enabled[1] != 0)
     lighting1(normal, vnormal, reflection, mtl_gloss_sa, light_acc, diffuse_acc, specular_acc);

  //Light in a lot of systems is just too dark.
  //Until the universe generator gets fixed, this hack fixes that:
  vec3 crank_factor = 3.0*normalize(light_acc)/light_acc;
  diffuse_acc *= crank_factor;
  specular_acc *= crank_factor;

  //Gather all incoming light:
  //NOTE: "Incoming" is a misnomer, really; what it means is that of all incoming light, these are the
  //portions expected to reflect specularly and/or diffusely, as per angle and shininess; --but not yet
  //modulated as per fresnel reflectivity or material colors. So I put them in quotes in the comments.
  //"Incoming specular":
#if (SUPRESS_ENVIRONMENT == 0)
  vec3 incoming_specular_light = GLOSS_env_reflection(reflection);
#else
  vec3 incoming_specular_light = vec3(0.0);
#endif
#if (SUPRESS_LIGHTS == 0)
  incoming_specular_light += specular_acc;
#endif
  //"Incoming diffuse":
#if (SUPRESS_GLOWMAP == 0)
  vec3 incoming_diffuse_light = glow_col;
#else
  vec3 incoming_diffuse_light = vec3(0.0);
#endif
#if (SUPRESS_LIGHTS == 0)
  incoming_diffuse_light += diffuse_acc;
#endif
  
  //Multiply and Add:
  vec3 final_reflected = incoming_specular_light * mspec_col * UAO;
#if (SUPRESS_DIFFUSE == 0)
  final_reflected += ( incoming_diffuse_light * diff_col );
#endif
  final_reflected *= UAO;

#if (1 == 1)
  //temporarily
  final_reflected += glow_col;
#endif

  gl_FragColor.rgb = regamma(final_reflected * cloaking.rrr);
  gl_FragColor.a   = alpha * cloaking.g;
  //Finitto!
}
