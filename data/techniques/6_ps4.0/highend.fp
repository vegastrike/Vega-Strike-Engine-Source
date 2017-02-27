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

//      Normalmap types:
#define CINEMUT_NM           1
#define RED_IN_ALPHA_NM      2
#define TRADITIONAL_NM       3

//      Shininess sources
#define AD_HOC_SHININESS     1
#define GLOSS_IN_SPEC_ALPHA  2

/**********************************/
//  CUSTOMIZATION  (EDITABLE)
/**********************************/
#define SHININESS_FROM       GLOSS_IN_SPEC_ALPHA
#define NORMALMAP_TYPE       CINEMUT_NM
#define DEGAMMA_SPECULAR     1
#define DEGAMMA_GLOW_MAP     1
#define DEGAMMA_LIGHTS       1
#define DEGAMMA_ENVIRONMENT  1
#define SANITIZE             0
/**********************************/

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE) (all should be zero for normal operation)
/**********************************/
// Light source switches:
#define SUPRESS_LIGHTS       0
#define SUPRESS_ENVIRONMENT  0
#define SUPRESS_GLOWMAP      0
// Material switches:
#define SUPRESS_DIFFUSE      0
#define USE_DIFF_AS_METAL    0
#define SUPRESS_SPECULAR     0
#define SUPRESS_DIELECTRIC   0
#define FORCE_FULL_REFLECT   0
#define SUPRESS_NORMALMAP    0
// Hack switches:
#define SUPRESS_CORNER_TRIM  0
#define SHOW_FLAT_SHADED     0
#define SUPRESS_HI_Q_VNORM   0
#define SUPRESS_GAREFLECT    0
#define SUPRESS_NMRELEV      1
/**********************************/

//  SPECIALS:
//      Special showings for debugging (do not edit)
#define SHOW_NO_SPECIAL      0
#define SHOW_MAT             1
#define SHOW_NORMAL          2
#define SHOW_TANGENTX        3
#define SHOW_TANGENTY        4
#define SHOW_TANGENTZ        5
#define SHOW_BINORMX         6
#define SHOW_BINORMY         7
#define SHOW_BINORMZ         8
#define SHOW_NOR_DOT_VIEW    9
#define SHOW_TAN_DOT_VIEW   10
#define SHOW_BIN_DOT_VIEW   11
#define SHOW_NOR_DOT_LIGHT0 12
#define SHOW_TAN_DOT_LIGHT0 13
#define SHOW_BIN_DOT_LIGHT0 14
#define SHOW_NOR_DOT_LIGHT1 15
#define SHOW_TAN_DOT_LIGHT1 16
#define SHOW_BIN_DOT_LIGHT1 17
#define SHOW_NOR_DOT_VNORM  18
#define SHOW_IS_PERIPHERY   19
#define SHOW_IS_NEAR_VERT   20
#define SHOW_IS_UGLY_CORNER 21
#define SHOW_MA_NO_CORNERS  22
#define SHOW_VNOR_DOT_FNOR  23

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE)
// (set to SHOW_NO_SPECIAL for normal operation)
/**********************************/
#define SHOW_SPECIAL SHOW_NO_SPECIAL
/**********************************/

//      CONSTANTS
#define TWO_PI     (6.2831853071795862)
#define HALF_PI    (1.5707963267948966)
#define PI_OVER_3  (1.0471975511965976)

vec3 matmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return normalize(vec3(dot(lightVec,tangent),dot(lightVec,binormal),dot(lightVec,normal)));
}
vec3 imatmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return normalize(lightVec.xxx*tangent+lightVec.yyy*binormal+lightVec.zzz*normal);
}

float bias(float f){ return f*0.5+0.5; }
vec2  bias(vec2 f) { return f*0.5+vec2(0.5); }
vec3  bias(vec3 f) { return f*0.5+vec3(0.5); }
vec4  bias(vec4 f) { return f*0.5+vec4(0.5); }
float expand(float f){ return f*2.0-1.0; }
vec2  expand(vec2 f) { return f*2.0-vec2(1.0); }
vec3  expand(vec3 f) { return f*2.0-vec3(1.0); }
vec4  expand(vec4 f) { return f*2.0-vec4(1.0); }
float lerp(float f, float a, float b){return (1.0-f)*a+f*b; }
vec2  lerp(float f, vec2 a, vec2 b) { return (1.0-f)*a+f*b; }
vec3  lerp(float f, vec3 a, vec3 b) { return (1.0-f)*a+f*b; }
vec4  lerp(float f, vec4 a, vec4 b) { return (1.0-f)*a+f*b; }
float saturate( in float a ){ return clamp( a, 0.0, 1.0 ); }
vec2  saturate( in vec2  a ){ return clamp( a, vec2(0.0), vec2(1.0) ); }
vec3  saturate( in vec3  a ){ return clamp( a, vec3(0.0), vec3(1.0) ); }
vec4  saturate( in vec4  a ){ return clamp( a, vec4(0.0), vec4(1.0) ); }

#if NORMALMAP_TYPE == CINEMUT_NM
vec2 dUdV_first_decode( in vec4 nmfetch )
{
  return vec2( 0.3333*(nmfetch.r+nmfetch.g+nmfetch.b)-0.5, nmfetch.a-0.5 );
}
vec3 dUdV_final_decode( vec2 blend_of_first_decode )
{
    return normalize( vec3( blend_of_first_decode, 0.25 ) );
}
vec3 normalmap_decode(vec4 nm)
{
  return dUdV_final_decode( dUdV_first_decode( nm ) );
}
#endif
#if NORMALMAP_TYPE == RED_IN_ALPHA_NM
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(nm.wy*vec2(2.0,2.0)-vec2(1.0,1.0),nm.z) );
}
#endif
#if NORMALMAP_TYPE == TRADITIONAL_NM
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(nm.xy*vec2(2.0,2.0)-vec2(1.0,1.0),nm.z) );
}
#endif

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
#if SUPRESS_ENVIRONMENT == 0
vec3 GLOSS_env_reflection( in vec4 mat_gloss, in vec3 direction ) //const
{
  //ENV MAP FETCH:
  vec3 result = textureCubeLod( envMap, direction, mat_gloss.y ).rgb;
#if DEGAMMA_ENVIRONMENT
  return result * result;
#else
  return result;
#endif
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


//AMBIENT
#if SUPRESS_AMBIENT==0
vec3 ambMapping(in vec3 bent_normal, in float ao )
{
  //compute lod bias from ao. The full math would go:
  //  solid_angle = pi * ao;
  //  radial_angle = acos( 1 - solid_angle/2pi )
  //  mipbias = 8.0 - log2.5((pi/3)/radial_angle)
  //  the formula used for bias is a rough approximation
  //  see this post for the creative process:
  //  http://wcjunction.com/phpBB2/viewtopic.php?p=22484#22484
  float bias = clamp( 8.223776 * ao / ( 0.027972 + ao ), 5.5, 7.7 );
  //but I'm limiting the bias to 7.7 because the 4x4 mip (lod 8)
  //is one solid color due to bugs with CubeMapGen's edge fix-up; I'll
  //put it to 8.0 after I hack CMG and make it make better cubemaps
  vec3 amb = textureCubeLod(envMap, bent_normal, bias).rgb;
#if DEGAMMA_ENVIRONMENT
  return amb * amb;
#else
  return amb;
#endif
}
#endif

//PER-LIGHT STUFF
void lightingLight(
   in vec4 lightinfo, in vec3 normal, in vec3 vnormal, in vec3 reflection, in vec3 vreflection,
   in vec3 raw_light_col,
   in float relevance, in float mat_gloss_sa,
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc)
{
   vec3  light_pos = normalize(lightinfo.xyz);
   float light_sa = lightinfo.w;
#if DEGAMMA_LIGHTS
   vec3 light_col = raw_light_col * raw_light_col;
#else
   vec3 light_col = raw_light_col;
#endif
   float VNdotL= saturate( dot(vnormal,light_pos) );
   float NdotL = clamp( dot(normal,light_pos), 0.0, VNdotL*4.0 );
   float RdotL = clamp( dot(reflection,light_pos), 0.0, VNdotL*4.0 );
   float VRdotL= clamp( dot(vreflection,light_pos), 0.0, VNdotL*4.0 );
   light_acc += light_col;
   float phong  = GLOSS_phong_reflection(mat_gloss_sa,RdotL,light_sa);
#if SUPRESS_NMRELEV == 0
   float vphong = GLOSS_phong_reflection(mat_gloss_sa,VRdotL,light_sa);
   specular_acc += ( lerp( relevance, vphong, phong ) * light_col );
   diffuse_acc  += ( lerp( relevance, VNdotL, NdotL ) * light_col );
#else
   specular_acc += ( phong * light_col );
   diffuse_acc += ( NdotL * light_col );
#endif
}

#define lighting(name, lightno_gl, lightno_tex) \
void name( \
   in vec3 normal, in vec3 vnormal, in vec3 reflection, in vec3 vreflection, \
   in float relevance, in float mat_gloss_sa, \
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc) \
{ \
   lightingLight( \
      gl_TexCoord[lightno_tex], normal, vnormal, reflection, vreflection, \
      gl_LightSource[lightno_gl].diffuse.rgb, \
      relevance, mat_gloss_sa, \
      light_acc, diffuse_acc, specular_acc); \
}

lighting(lighting0, 0, 5)
lighting(lighting1, 1, 6)


//REFLECTION
/*
 GAR stands for Gloss-Adjusted Reflection. It addresses the problem that a reflection vector off
  a finite gloss material represents a blur cone. But at very shallow angles, near the periphery
  of objects, when the reflection vector approaches colinearity with the view vector, that cone
  would extend to areas of the sky behind the object!, which makes no sense. The reflection cone
  must span up to 1 blur radius less than 180 degrees from the view vector; and it must dim also
  as it approaches maximum reflection angle.
*/
void GAR( in vec3 eye, in vec3 normal, in float blur_radius, out vec3 GAreflection )
{
  vec3 adjnormal = normalize( normal + 0.5 * blur_radius * eye );
  GAreflection = normalize(-reflect(eye,adjnormal));
}


void main() 
{
  // Retreive texture coordinates
  vec2 tex_coord = gl_TexCoord[0].xy;
  
  // Retrieve vectors
  vec3 iNormal=gl_TexCoord[1].xyz;
  vec3 iTangent=gl_TexCoord[2].xyz;
  vec3 iBinormal=gl_TexCoord[3].xyz;
  vec3 position = gl_TexCoord[7].xyz;
  vec3 fnormal = normalize( cross( dFdx(position), dFdy(position) ) );
  fnormal *= sign(dot(fnormal,iNormal)+0.1);
#if SUPRESS_HI_Q_VNORM == 0
  //supplement the vnormal with face normal before normalizing
  float supplemental_fraction=(1.0-length(iNormal));
  vec3 vnormal = normalize( iNormal + supplemental_fraction*fnormal );
#else
  vec3 vnormal = normalize( iNormal );
#endif
  vec3 normal=imatmul(iTangent,iBinormal,iNormal,normalmap_decode(texture2D(normalMap,tex_coord)));
#if SUPRESS_NORMALMAP
  normal = vnormal;
#endif
  
  // Other vectors
  vec3 eye = gl_TexCoord[4].xyz;
  
  // Sample textures
  vec4 damagecolor = texture2D(damageMap , tex_coord);
  vec4 diffcolor   = texture2D(diffuseMap, tex_coord);
  vec4 speccolor   = texture2D(specMap   , tex_coord);
  vec4 glowcolor   = texture2D(glowMap   , tex_coord);

  //better apply damage lerps before de-gamma-ing
  diffcolor.rgb  = lerp(damage.x, diffcolor, damagecolor).rgb;
  speccolor     *= (1.0-damage.x);
  glowcolor.rgb *= (1.0-damage.x);
  
  //materials
  vec4 mtl_gloss;
  vec3 diff_col, mspec_col, glow_col;
  float alpha, UAO;
  //grab alpha channels  
  alpha = diffcolor.a;
  UAO = glowcolor.a;
  float rootUAO = sqrt(UAO);
  //de-gamma diff and spec
  diff_col = (diffcolor*diffcolor).rgb;
#if DEGAMMA_SPECULAR
  mspec_col = (speccolor*speccolor).rgb;
#else
  mspec_col = speccolor.rgb;
#endif
#if DEGAMMA_GLOW_MAP
  glow_col = (glowcolor*glowcolor).rgb;
#else
  glow_col = glowcolor.rgb;
#endif

#if SANITIZE
  //sanity
  vec3 inverse_insanity = vec3(1.0) / clamp( mspec_col+diff_col, vec3(1.0), vec3(2.0) );
  diff_col *= inverse_insanity;
  mspec_col *= inverse_insanity;
#endif
  
  //compute gloss-related stuff
#if SHININESS_FROM == AD_HOC_SHININESS
  float crapgloss = 0.3333*dot(mspec_col,mspec_col);
  GLOSS_init( mtl_gloss, crapgloss*crapgloss );
#endif
#if SHININESS_FROM == GLOSS_IN_SPEC_ALPHA
  GLOSS_init( mtl_gloss, speccolor.a );
#endif

  //reflection
  vec3 reflection;
#if SUPRESS_GAREFLECT == 0
  GAR( eye, normal, mtl_gloss.z, reflection );
#else
  reflection = normalize(-reflect(eye,normal));
#endif
  vec3 vreflection = normalize(-reflect(eye,vnormal));
  
#if SUPRESS_NMRELEV == 0
  //normalmap feature relevance adjustments
  //(Just a way of getting a hint of paralax on the very cheap. Basic idea is to make shading effects on bump
  //features facing you a bit exaggerated in the shading; and make their shading fade away as they face away.)
  float r =  saturate(dot(  normal, eye )); //normalmap modulated: 0 when view is tangential; 1 when viewing surface face on
  float vr = saturate(dot( vnormal, eye )); //raw face normal:  0 when view is tangential; 1 when viewing surface face on
  float rel = clamp( (r+0.5)/(vr+0.5), 0.125, 1.875 );
#else
  float rel = 1.0;
#endif

  // Init lighting accumulators
  vec3 light_acc    = vec3(0.0);
  vec3 diffuse_acc  = vec3(0.0);
  vec3 specular_acc = vec3(0.0);
  // Do lighting for every active light
  float mtl_gloss_sa = mtl_gloss.w;
  if (light_enabled[0] != 0)
     lighting0(normal, vnormal, reflection, vreflection, rel, mtl_gloss_sa, light_acc, diffuse_acc, specular_acc);
  if (light_enabled[1] != 0)
     lighting1(normal, vnormal, reflection, vreflection, rel, mtl_gloss_sa, light_acc, diffuse_acc, specular_acc);
  
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
#if SUPRESS_ENVIRONMENT == 0
  vec3 incoming_specular_light = GLOSS_env_reflection(mtl_gloss,reflection);
#else
  vec3 incoming_specular_light = vec3(0.0);
#endif
#if SUPRESS_LIGHTS == 0
  incoming_specular_light += specular_acc;
#endif
  //"Incoming diffuse":
#if SUPRESS_GLOWMAP == 0
  vec3 incoming_diffuse_light = glow_col;
#else
  vec3 incoming_diffuse_light = vec3(0.0);
#endif
#if SUPRESS_LIGHTS == 0
  incoming_diffuse_light += diffuse_acc;
#endif
#if SUPRESS_AMBIENT == 0
  incoming_diffuse_light += ( ambMapping( normal, UAO ) * UAO );
#endif
  
  //Multiply and Add:
  vec3 final_reflected = incoming_specular_light * mspec_col * UAO * rootUAO;
#if SUPRESS_DIFFUSE == 0
  final_reflected += ( incoming_diffuse_light * diff_col * rootUAO );
#endif

#if 1
  //temporarily
  final_reflected += glow_col;
#endif

  gl_FragColor = vec4( sqrt(final_reflected), alpha ) * cloaking.rrrg;
  //Finitto!
}
