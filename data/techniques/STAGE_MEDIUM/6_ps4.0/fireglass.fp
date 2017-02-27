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
uniform vec4 pass_num;

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
#define SHININESS_FROM       AD_HOC_SHININESS
#define NORMALMAP_TYPE       CINEMUT_NM
#define NM_FREQ_SCALING      5.777
#define NM_Z_SCALING         0.015625
#define CORNER_TRIMMING_POW  177.77
#define DEGAMMA_GLOW_MAP     1
#define DEGAMMA_LIGHTS       0
#define DEGAMMA_ENVIRONMENT  1
#define DIM_SHALLOW_REFLECT  0
/**********************************/

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE) (all should be zero for normal operation)
/**********************************/
// Light source switches:
#define SUPRESS_LIGHTS       0
#define SUPRESS_ENVIRONMENT  0
#define SUPRESS_GLOWMAP      1
// Material switches:
#define SUPRESS_DIFFUSE      1
#define USE_DIFF_AS_METAL    0
#define SUPRESS_SPECULAR     0
#define SUPRESS_DIELECTRIC   0
#define FORCE_FULL_REFLECT   0
#define SUPRESS_NORMALMAP    0
// Hack switches:
#define SUPRESS_CORNER_TRIM  0
#define SHOW_FLAT_SHADED     0
#define SUPRESS_HI_Q_VNORM   0
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

//UTILS
vec3 matmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return vec3(dot(lightVec,tangent),dot(lightVec,binormal),dot(lightVec,normal));
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

#if NORMALMAP_TYPE == CINEMUT_NM
vec2 dUdV_first_decode( in vec4 nmfetch )
{
  return vec2( NM_Z_SCALING*(0.3333*(nmfetch.r+nmfetch.g+nmfetch.b)-0.5), NM_Z_SCALING*(nmfetch.a-0.5) );
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
  return normalize( vec3(NM_Z_SCALING*(nm.wy*vec2(2.0,2.0)-vec2(1.0,1.0)),nm.z) );
}
#endif
#if NORMALMAP_TYPE == TRADITIONAL_NM
vec3 normalmap_decode(vec4 nm)
{
  return normalize( vec3(NM_Z_SCALING*(nm.xy*vec2(2.0,2.0)-vec2(1.0,1.0)),nm.z) );
}
#endif

//OUTLINE SMOOTHING (by lowering alpha at corners on the periphery)
float is_periphery( in vec3 view, in vec3 rawvnorm, in float is_perimeter )
{
  /*float temp1 = dot( view, rawvnorm );
  return clamp( 1.0-(temp1)*(1.0-is_perimeter), 0.0, 1.0 );*/
  return is_perimeter;
}
float is_near_vert( in vec3 rawvnorm )
{
  return clamp(length(rawvnorm),0.8,1.0);
}
float is_near_vert_on_periphery( in vec3 view, in vec3 rawvnorm, in float is_perimeter )
{
  /*float temp1 = dot( view, rawvnorm );
  float temp2 = 1.0 - temp1*temp1;
  temp1 = temp2 * is_perimeter;*/
  //float temp = (1.0-is_perimeter(view,rawvnorm)) * (1.0-is_near_vert(vnorm));
  //return 1.0 - temp * temp;
  return pow( is_periphery(view,rawvnorm,is_perimeter) * is_near_vert(rawvnorm), CORNER_TRIMMING_POW );
}
float outline_smoothing_alpha( in vec3 view, in vec3 rawvnorm, in float is_perimeter )
{
  float temp = 1.0 - is_near_vert_on_periphery( view, rawvnorm, is_perimeter );
  return temp * temp * temp;
}

//FRESNEL
float fresnel( in float cosa, in float k )
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
   return 0.5 * (tmp4+tmp2);
}


//GLOSS class
void GLOSS_init( inout vec4 mat_gloss, in float gloss_in  )
{
  //decrease resolution at high end of input range; --LOD 0 to LOD 1 difference is hardly noticeable
  //except on glossy AND very flat surfaces; whereas at the low end the resolution is more critical
  //See the forum posts indicate at the top of this file for the math derivation
  mat_gloss.x = 0.5 * ( gloss_in + gloss_in*gloss_in ); //relinearized input
  mat_gloss.y  = max( 0.0, 7.288828847 * ( 1.0 - mat_gloss.x ) ); //mip bias
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


#if SUPRESS_LIGHTS == 0
//PER-LIGHT STUFF
void lightingLight(
   in vec4 lightinfo, in vec3 normal, in vec3 vnormal, in vec3 reflection, 
   in vec4 raw_light_col,
   in float mat_gloss_sa, in float nD,
   inout vec3 light_acc, inout vec3 specular_acc)
{
   vec3  light_pos = normalize(lightinfo.xyz);
   float light_size = lightinfo.w;
#if DEGAMMA_LIGHTS
   vec3 light_col = raw_light_col.rgb * raw_light_col.rgb;
#else
   vec3 light_col = raw_light_col.rgb;
#endif
   float VNdotLx4= clamp( 400.0 * dot(vnormal,light_pos), 0.0, 1.0 );
   float RdotL = clamp( dot(reflection,light_pos), 0.0, VNdotLx4 );
   light_acc += light_col;
   specular_acc += ( GLOSS_phong_reflection(mat_gloss_sa,RdotL,light_size) * light_col );
}
#define lighting(name, lightno_gl, lightno_tex) \
void name( \
   in vec3 normal, in vec3 vnormal, in  vec3 reflection, \
   in float mat_gloss_sa, in float nD, \
   inout vec3 light_acc, inout vec3 specular_acc) \
{ \
   lightingLight( \
      gl_TexCoord[lightno_tex], normal, vnormal, reflection, \
      gl_LightSource[lightno_gl].diffuse, \
      mat_gloss_sa, nD, \
      light_acc, specular_acc); \
}
lighting(lighting0, 0, 5)
lighting(lighting1, 1, 6)
#endif


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


//FINALLY... MAIN()
void main() 
{
  // Retreive texture coordinates
  vec2 tex_coord = gl_TexCoord[0].xy;
  vec2 nm_tex_coord = NM_FREQ_SCALING * tex_coord;
  
  // Retrieve vectors
  vec3 iNormal=gl_TexCoord[1].xyz;
  vec3 iTangent=gl_TexCoord[2].xyz;
  vec3 iBinormal=gl_TexCoord[3].xyz;
  vec3 position = gl_TexCoord[7].xyz;
  vec3 face_normal = normalize( cross( dFdx(position), dFdy(position) ) );
#if SUPRESS_HI_Q_VNORM == 0
  //supplement the vnormal with face normal before normalizing
  float supplemental_fraction=(1.0-length(iNormal));
  vec3 vnormal = normalize( iNormal + supplemental_fraction*face_normal );
#else
  vec3 vnormal = normalize( iNormal );
#endif
  vec3 normal=imatmul(iTangent,iBinormal,iNormal,normalmap_decode(texture2D(normalMap,nm_tex_coord)));
#if SUPRESS_NORMALMAP
  normal = vnormal;
#endif

  // Other vectors
  vec3 eye = gl_TexCoord[4].xyz;
  float is_perimeter = gl_TexCoord[4].w;
  
  //compute a periphery smoothing alpha
  float PSalpha = outline_smoothing_alpha( eye, iNormal, is_perimeter );
  
  // Sample textures
  vec4 damagecolor = texture2D(damageMap , tex_coord);
  vec4 diffcolor   = texture2D(diffuseMap, tex_coord);
  vec4 speccolor   = texture2D(specMap   , tex_coord);
  vec4 glowcolor   = texture2D(glowMap   , tex_coord);
  
  //better apply damage lerps before de-gamma-ing
  //COMMENTED OUT because I don't think anyone would bother to create a damage texture
  //for transparent parts, and it would look like crap, anyways.
//  diffcolor.rgb  = lerp(damage.x, diffcolor, damagecolor).rgb;
//  speccolor  *= (1.0-damage.x);
//  glowcolor.rgb  *= (1.0-damage.x);
  
  //materials
  vec4 mtl_gloss;
//  vec3 diff_col, glow_col;
  float alpha, nD, UAO;
#if SHOW_SPECIAL == SHOW_NO_SPECIAL
  const float GLASS_REFRACTIVE_CONSTANT = 1.48567;
  nD = GLASS_REFRACTIVE_CONSTANT;
  //grab alpha channels  
  alpha = diffcolor.a;
  UAO = glowcolor.a;
  GLOSS_init( mtl_gloss, speccolor.a );
//#if DEGAMMA_GLOW_MAP
//  glow_col = (glowcolor*glowcolor).rgb;
//#else
// glow_col = glowcolor.rgb;
//#endif

  vec3 reflection;
#if SHOW_FLAT_SHADED
  normal = face_normal;
#endif
  //GAR( eye, normal, mtl_gloss.z, reflection );
  reflection = -reflect( eye, normal );
  
  //DIELECTRIC REFLECTION
#if SUPRESS_DIELECTRIC == 0
  float fresnel_refl = fresnel( dot( reflection, normal), nD );
#else
  float fresnel_refl = 0.0;
#endif
  float fresnel_refr = 1.0 - fresnel_refl;

  // Init lighting accumulators
  vec3 light_acc    = vec3(0.0);
  vec3 specular_acc = vec3(0.0);
#if SUPRESS_LIGHTS == 0
  // Do lighting for every active light
  float mtl_gloss_sa = mtl_gloss.w;
  if (light_enabled[0] != 0)
     lighting0(normal, vnormal, reflection, mtl_gloss_sa, nD, light_acc, specular_acc);
  if (light_enabled[1] != 0)
     lighting1(normal, vnormal, reflection, mtl_gloss_sa, nD, light_acc, specular_acc);
#endif
   
  //Light in a lot of systems is just too dark.
  //Until the universe generator gets fixed, this hack fixes that:
  vec3 crank_factor = 3.0*normalize(light_acc)/light_acc;
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
  
  //Gather the reflectivities:
  vec3 dielectric_specularity = vec3(1.0);
  vec3 total_specularity = dielectric_specularity; // + metallic_specularity;
 
  //Multiply and Add:
  //we multiply by pass_num so that in pass 0 there's no env mapping or specular lights; but in
  // pass 1 there are; so that the far side appears to reflect only the (presumably) darker interior
  vec3 final_reflected = UAO * UAO * pass_num.x * incoming_specular_light * total_specularity;
  
  //FINAL PIXEL WRITE:
  //Restore gamma, add alpha, and Commit:
  float final_alpha = sqrt(fresnel_refl);
#if FORCE_FULL_REFLECT
  final_alpha = 1.0;
#endif
  //trim the corners around the outline
  final_alpha *= PSalpha;
  //final_reflected = vec3(0.0,1.5,0.0);
  gl_FragColor = vec4( sqrt(final_reflected)*final_alpha, final_alpha );// * cloaking.rrrg;
  //Finitto!
#endif
#if SHOW_SPECIAL == SHOW_MAT
  //  * material AI detections (red = matte; green = metal; blue = dielectric )
  gl_FragColor = vec4( mattype.rgb, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_NORMAL
  //  * RGB = normal.xyz * 0.5 + 0.5
  gl_FragColor = vec4( vnormal.xyz * 0.5 + vec3(0.5), 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TANGENTX
  //  * RGB = tangent.xyz * 0.5 + 0.5
  gl_FragColor = vec4( normalize(iTangent.x) * 0.5 + 0.5, 0.5, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TANGENTY
  //  * RGB = tangent.xyz * 0.5 + 0.5
  gl_FragColor = vec4( 0.5, normalize(iTangent.y) * 0.5 + 0.5, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TANGENTZ
  //  * RGB = tangent.xyz * 0.5 + 0.5
  gl_FragColor = vec4( 0.5, 0.5, normalize(iTangent.z) * 0.5 + 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BINORMX
  //  * RGB = binormal.xyz * 0.5 + 0.5
  gl_FragColor = vec4( normalize(iBinormal.x) * 0.5 + 0.5, 0.5, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BINORMY
  //  * RGB = binormal.xyz * 0.5 + 0.5
  gl_FragColor = vec4( 0.5, normalize(iBinormal.y) * 0.5 + 0.5, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BINORMZ
  //  * RGB = binormal.xyz * 0.5 + 0.5
  gl_FragColor = vec4( 0.5, 0.5, normalize(iBinormal.z) * 0.5 + 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_NOR_DOT_VIEW
  //  * R = dot(normal, view)
  vec3 eyevec = normalize( eye );
  vec3 temp;
  temp.r = dot( vnormal, eyevec ) * 0.5 + 0.5;
  temp.g = 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TAN_DOT_VIEW
  //  * G = dot(tangent, view)
  vec3 eyevec = normalize( eye );
  vec3 temp;
  temp.r = 0.5;
  temp.g = dot( normalize(iTangent.xyz), eyevec ) * 0.5 + 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BIN_DOT_VIEW
  //  * B = dot(binormal, view)
  vec3 eyevec = normalize( eye );
  vec3 temp;
  temp.r = 0.5;
  temp.g = 0.5;
  temp.b = dot( normalize(iBinormal.xyz), eyevec ) * 0.5 + 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_NOR_DOT_LIGHT0
  //  * R = dot(normal, light)
  vec3 lightvec = normalize( gl_TexCoord[5].xyz );
  vec3 temp;
  temp.r = dot( vnormal, lightvec ) * 0.5 + 0.5;
  temp.g = 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TAN_DOT_LIGHT0
  //  * G = dot(tangent, light)
  vec3 lightvec = normalize( gl_TexCoord[5].xyz );
  vec3 temp;
  temp.r = 0.5;
  temp.g = dot( normalize(iTangent.xyz), lightvec ) * 0.5 + 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BIN_DOT_LIGHT0
  //  * B = dot(binormal, light)
  vec3 lightvec = normalize( gl_TexCoord[5].xyz );
  vec3 temp;
  temp.r = 0.5;
  temp.g = 0.5;
  temp.b = dot( normalize(iBinormal.xyz), lightvec ) * 0.5 + 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_NOR_DOT_LIGHT1
  //  * R = dot(normal, light)
  vec3 lightvec = normalize( gl_TexCoord[6].xyz );
  vec3 temp;
  temp.r = dot( vnormal, lightvec ) * 0.5 + 0.5;
  temp.g = 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_TAN_DOT_LIGHT1
  //  * G = dot(tangent, light)
  vec3 lightvec = normalize( gl_TexCoord[6].xyz );
  vec3 temp;
  temp.r = 0.5;
  temp.g = dot( normalize(iTangent.xyz), lightvec ) * 0.5 + 0.5;
  temp.b = 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_BIN_DOT_LIGHT1
  //  * B = dot(binormal, light)
  vec3 lightvec = normalize( gl_TexCoord[6].xyz );
  vec3 temp;
  temp.r = 0.5;
  temp.g = 0.5;
  temp.b = dot( normalize(iBinormal.xyz), lightvec ) * 0.5 + 0.5;
  gl_FragColor = vec4( temp, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_NOR_DOT_VNORM
  float shade = dot(normal,vnormal) * 0.5 + 0.5;
  gl_FragColor = vec4( shade, shade, shade, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_IS_PERIPHERY
//float is_periphery( in vec3 view, in vec3 rawvnorm, in float is_perimeter )
  float temp = pow( is_periphery( eye, iNormal, is_perimeter ), CORNER_TRIMMING_POW );
  gl_FragColor = vec4( temp, temp, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_IS_NEAR_VERT
  float temp = pow( is_near_vert( iNormal ), CORNER_TRIMMING_POW );
  gl_FragColor = vec4( temp, temp, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_IS_UGLY_CORNER
  float temp = is_near_vert_on_periphery( eye, iNormal, is_perimeter );
  gl_FragColor = vec4( temp, temp, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_MA_NO_CORNERS
  float temp = outline_smoothing_alpha( eye, iNormal, is_perimeter );
  gl_FragColor = vec4( temp, temp, 0.5, 1.0 );
#endif
#if SHOW_SPECIAL == SHOW_VNOR_DOT_FNOR
  float temp = dot(face_normal,normalize(iNormal.xyz) * 0.5 + 0.5;
  //must exaggerate it greatly to be able to see...
  temp = 1.0-temp;
  temp = 1.0-177.7*temp;
  gl_FragColor = vec4( temp, temp, 0.5, 1.0 );
#endif
}


