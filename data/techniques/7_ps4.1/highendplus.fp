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

//      Shininess comes from...
#define AD_HOC_SHININESS     1
#define GLOSS_IN_SPEC_ALPHA  1

/**********************************/
//  CUSTOMIZATION  (EDITABLE)
/**********************************/
#define SHININESS_FROM       AD_HOC_SHININESS
#define NORMALMAP_TYPE       CINEMUT_NM
#define DEGAMMA_GLOW_MAP     0
#define DEGAMMA_LIGHTS       0
#define DEGAMMA_ENVIRONMENT  0
#define ALLOW_GRAY_PAINT     0
/**********************************/

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE) (all should be zero for normal operation)
/**********************************/
// Light source switches:
#define SUPRESS_AMBIENT      0
#define SUPRESS_LIGHTS       0
#define SUPRESS_ENVIRONMENT  0
#define SUPRESS_GLOWMAP      0
// Material switches:
#define SUPRESS_DIFFUSE      0
#define SUPRESS_SPECULAR     0
#define SUPRESS_DIELECTRIC   0
/**********************************/

//      Special showings for debugging
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

/**********************************/
//  DEBUGGING SWITCHES (EDITABLE)
/**********************************/
#define FORCE_GLASS          0
#define SHOW_SPECIAL         SHOW_NO_SPECIAL
/**********************************/

//      CONSTANTS
#define TWO_PI     (6.2831853071795862)
#define HALF_PI    (1.5707963267948966)
#define PI_OVER_3  (1.0471975511965976)

//UTILS
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

//FRESNEL
float fresnel( in float cosa, in float k, in float two_sided )
{
   float tmp1 = sqrt(1.0-(1.0-cosa*cosa)/(k*k));
   float tmp2 = k*cosa;
   float tmp3 = k*tmp1;
   float tmp4 = (tmp1-tmp2)/(tmp1+tmp2+0.0001);
   tmp1 = (cosa-tmp3)/(cosa+tmp3+0.0001);
   tmp2 = 0.5*(tmp1*tmp1+tmp4*tmp4);
   //that's the final Fresnel value, in tmp2. For glass, we got two surfaces to
   //a glass pane: outer and inner. And the inner reflection is is equally as
   //strong as the outer. I'll look for a multi-bounce solution; but for now
   //we'll square the refractivity, and convert back to reflectivity; then
   //average the two for a rough multi-bounce approximation.
   tmp3 = 1.0 - tmp2;
   tmp4 = 1.0 - tmp3*tmp3;
   //So, for glass, we would return 0.5 * (tmp4+tmp2); but in the general case,
   return lerp( 0.5 * two_sided, tmp2, tmp4 );
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
#if SUPRESS_ENVIRONMENT == 0
vec3 GLOSS_env_reflection( in vec4 mat_gloss, in vec3 direction ) //const
{
  //ENV MAP FETCH:
  float temp = mat_gloss.x;
  vec3 col_lod = textureLod( envMap, direction, mat_gloss.y ).rgb;
  temp *= temp;
  temp *= temp;
  //I multiply the dFd's below, to account for multiple inner reflections causing linear blur.
  //Should really use dot(eye,normal) to max out at like 45 degrees... Later.
  vec3 col_gra = textureGrad( envMap, direction, dFdx(direction), dFdy(direction) ).rgb;
  temp *= temp;
  temp *= temp;
  vec3 result = lerp( temp*temp, col_lod, col_gra );
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


//MATERIAL_AI CLASS
//  Note: this is a very complex routine, but by the same token it sucks a lot of crap
//  out from the rest of the poor shader :D
#define is_matte mattype.r
#define is_metal mattype.g
#define is_dielk mattype.b
#define is_glass mattype.a
//public
void mattype_init
(
  in vec4 diff, in vec4 spec, in vec4 glow,
  inout vec4 mat_gloss, inout vec4 mattype,
  out vec3 diff_col, out vec3 mspec_col, out vec3 glow_col,
  out float alpha, out float nD, out float AO
)
{
  //Note: This routine de-gamma-es and fixes diff and spec colors as a side-effect; --it
  //was just too convenient to avoid...
  vec3  tempv;
  float tempf;
  //Okay, first let's un-pack a few things:
  AO = glow.a;
  //*********************************************
  alpha = clamp( diff.a * 1.2 - 0.1, 0.0, 1.0 );
#if FORCE_GLASS
  alpha = 0.0;
#endif
  //*********************************************
  //MATERIAL AI'S BASIC INITIALIZATIONS:
  //add a fraction of diff to spec, & viceversa, to avoid chroma inprecision when either is black
  vec3 diff_ = ( diff + 0.05 * spec ).rgb;
  vec3 spec_ = ( spec + 0.05 * diff ).rgb;
#if FORCE_GLASS
  spec_ = vec3(1.0);
#endif
  //extract lumas (lumas-squared, rather; but you'll see...)
  float diffluma = dot( diff_, diff_ );
  float specluma = dot( spec_, spec_ );
  //de-gamma now (more convenient to de-gamma now, than to sqrt() the lumas :))
  diff_ *= diff_;
  spec_ *= spec_;
  //set metallic albedo for later use (note that we don't know yet whether it's a metal...)
  vec3 metalbedo = diff_ + spec_;
  //get normalized chromas
  vec3 ndiff = diff_ / diffluma;
  vec3 nspec = spec_ / specluma;
  //get saturations
  tempv = ndiff - vec3(0.333);
  float satdiff = sqrt( dot( tempv, tempv ) );
  tempv = nspec - vec3(0.333);
  float satspec = sqrt( dot( tempv, tempv ) );
  //and our lumas, so far, range 0-3; so scale them down to 0-1, for boolean convenience
  diffluma *= 0.333;
  specluma *= 0.333;
  float sum_lumas = diffluma + specluma;
  //THE CORE MATERIAL AI BEGINS:
  //if specular is white and alpha is zero; this is intended to represent glass
  is_glass = clamp( specluma - alpha, 0.0, 1.0 );
  //Metals are represented by equal diff and spec chromas
  tempv = ndiff - nspec;
  tempf = clamp( 1.0 - sqrt( dot( tempv, tempv ) ), 0.0, 1.0 );
  is_metal = tempf * tempf;
  //Matte materials are those with much higher diff luma than spec luma
  tempf = diffluma - specluma;
  is_matte = clamp( tempf/sum_lumas, 0.0, 1.0 );
  is_metal *= (1.0-is_matte); //matte strongly overrides metal detection
  //Dielectrics are represented by unsaturated spec versus colorful diff
  const float MINSATURATION = 0.05;
  is_dielk = clamp( (satdiff-satspec) / (satdiff+satspec+MINSATURATION), 0.0, 1.0 );
  //SPECIAL CASES:
  //If it so far appears to be a gray metal, but specluma == diffluma, make it gray paint.
  //Also, if alpha is not 1.0, it's glass or plexiglass --i.e. a dielectric.
#if ALLOW_GRAY_PAINT
  float isreallydielectric = clamp( 1.0 - 2.0*( satdiff + abs(tempf) ), 2.0*sqrt(1.0-alpha), 1.0 );
  isreallydielectric *= isreallydielectric;
  isreallydielectric *= isreallydielectric;
  isreallydielectric *= isreallydielectric;
  is_metal *= (1.0-isreallydielectric);
  is_dielk += ( (1.0-is_dielk) * isreallydielectric );
#endif
  //and if alpha is low; this is either an alpha-testing hole, or glass; and is therefore neither
  //a metal nor a matte material
  is_metal *= alpha;
  is_matte *= alpha;
  //and if it's glass, it's a dielectric for sure
  is_dielk = max( is_dielk, is_glass );
  //finally, normalize our conclusions so that they add to one :)
  tempv = (mattype*mattype).rgb;
  tempf = dot( mattype.rgb, mattype.rgb );
  mattype.rgb = tempv/tempf;
  //AI FINISHED!!
  //Now we finish de-gammaing and fixing the diff and spec colors:
#if DEGAMMA_GLOW_MAP
  glow_col = (glow*glow).rgb;
#else
  glow_col = glow.rgb;
#endif
  //the non-metal diffuse color for matte materials and dielectrics is trivial:
  diff_col = diff_;
  //the metallic diff and spec are a bit more complicated.. First, they must be
  //guaranteed to derive from a specular albedo, which for diffuse is squared.
  //Second, we must adjust the lumas so that they match the original lumas;
  vec3 mdiff_col;
  tempf = 1.0 / sum_lumas;
  mspec_col = metalbedo * specluma * tempf;
  mdiff_col = metalbedo * metalbedo * ( diffluma * tempf );
  //but they must not exceed 1.0 for any rgb channel... and we also don't want
  //reflectivity for emissive materials; a common problem in Vegastrike art
  tempv = mspec_col + mdiff_col;
  tempf = max( max( tempv.r, tempv.g ), tempv.b );
  tempf = 1.0 / ( max( tempf, 1.0 ) + dot( glow_col, glow_col ) );
  mspec_col *= tempf;
  mdiff_col *= tempf;
  //finally, for dielectrics (except glass), spec luma represents refractive nD;
  //but metal oxides have refractive constants too; so default to chromium
  const float CHROMIUM_OXIDE_REFRACTIVE_CONSTANT = 2.5;
  const float RCND = sqrt( CHROMIUM_OXIDE_REFRACTIVE_CONSTANT );
  tempf = lerp( is_dielk, RCND, 1.0+specluma );
  //but in the special case of glass, we go to the average nD for pyrex, lucite and plexiglass
  const float GLASS_REFRACTIVE_CONSTANT = 1.48567;
  nD = lerp( is_glass, tempf*tempf, GLASS_REFRACTIVE_CONSTANT );
  //and combine the diffuse colors
  diff_col = lerp( is_metal, diff_col, mdiff_col );
  //*********************************************
  //Temporary hack to get shininess
  //Eventually it should read shininess from spec alpha
#if SHININESS_FROM == AD_HOC_SHININESS
  tempf = 0.333*dot(spec.rgb,spec.rgb);// + 0.2*is_dielk;
  GLOSS_init( mat_gloss, tempf*sqrt(tempf) );
#else
  GLOSS_init( mat_gloss, spec.a );
#endif
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
  float bias = clamp( 8.223776 * ao / ( 0.027972 + ao ), 3.0, 8.0 );
  //but I'm subtracting 0.5 from the bias because the 4x4 mip (lod 8)
  //is one solid color due to bugs with CubeMapGen's edge fix-up; I'll
  //remove it after I hack CMG and make better cubemaps
  vec3 amb = textureLod(envMap, bent_normal, bias-0.5).rgb;
#if DEGAMMA_ENVIRONMENT
  return amb * amb;
#else
  return amb;
#endif
}
#endif


//PER-LIGHT STUFF
#if SUPRESS_LIGHTS == 0
void lightingLight(
   in vec4 lightinfo, in vec3 normal, in vec3 vnormal, in vec3 eye, in vec3 reflection, 
   in vec3 raw_light_col,
   in float mat_gloss_sa, in float nD,
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc)
{
   vec3  light_pos = normalize(lightinfo.xyz);
   float light_size = lightinfo.w;
#if DEGAMMA_LIGHTS
   vec3 light_col = raw_light_col * raw_light_col;
#else
   vec3 light_col = raw_light_col;
#endif
   float VNdotLx4= clamp( 4.0 * dot(vnormal,light), 0.0, 1.0 );
   float NdotL = clamp( dot(normal,light_pos), 0.0, VNdotLx4 );
   float RdotL = clamp( dot(reflection,light_pos), 0.0, VNdotLx4 );
   light_acc += light_col;
   specular_acc += ( GLOSS_phong_reflection(mat_gloss_sa,RdotL,light_size) * light_col );
   diffuse_acc  += ( NdotL * light_col );
}
#define lighting(name, lightno_gl, lightno_tex) \
void name( \
   in vec3 normal, in vec3 vnormal, in vec3 eye, in  vec3 reflection, \
   in float mat_gloss_sa, in float nD, \
   inout vec3 light_acc, inout vec3 diffuse_acc, inout vec3 specular_acc) \
{ \
   lightingLight( \
      gl_TexCoord[lightno_tex], normal, vnormal, eye, reflection, \
      gl_LightSource[lightno_gl].diffuse.rgb, \
      mat_gloss_sa, nD, \
      light_acc, diffuse_acc, specular_acc); \
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
void GAR( in vec3 eye, in vec3 normal, in vec3 vnormal, in float blur_radius, out vec3 GAreflection, out float GAdim )
{
  vec3 adjnormal = normalize( normal + 0.5 * blur_radius * eye );
  GAreflection = normalize(-reflect(eye,adjnormal));
  float temp = dot( eye, normal ) / blur_radius;
  //edgeartifactclamp is for use in specularities; avoids excessively bright outlines
  float edgeartifactclamp = max( 32.0 * dot( eye, vnormal ), 0.0 );
  edgeartifactclamp /= ( edgeartifactclamp + 1.0 );
  edgeartifactclamp *= edgeartifactclamp;
  GAdim = edgeartifactclamp * dot(normal,vnormal) * (temp+0.5) / (temp+1.0);
}


//FINALLY... MAIN()
void main() 
{
  // Retreive texture coordinates
  vec2 tex_coord = gl_TexCoord[0].xy;
  
  // Retrieve vectors
  vec3 iNormal=gl_TexCoord[1].xyz;
  vec3 iTangent=gl_TexCoord[2].xyz;
  vec3 iBinormal=gl_TexCoord[3].xyz;
  vec3 position = gl_TexCoord[7].xyz;
  vec3 face_normal = normalize( cross( dFdx(position), dFdy(position) ) );
#if SUPRESS_HI_Q_VNORM == 0
  //supplement the vnormal with face normal before normalizing
  float supplemental_fraction=(1.0-length(iNormal));///dot(face_normal,iNormal);
  vec3 vnormal = normalize( iNormal + supplemental_fraction*face_normal );
#else
  vec3 vnormal = normalize( iNormal );
#endif
#if NORMALMAP_TYPE == CINEMUT_NM
  vec2 raw_dUdV = dUdV_first_decode( texture2D(normalMap,tex_coord) );
  //mix-in detail normals, then...
  vec3 normal = normalize(dUdV_final_decode( raw_dUdV ));
  normal = imatmul(iTangent,iBinormal,iNormal,normal);
#else
  vec3 normal=imatmul(iTangent,iBinormal,iNormal,normalmap_decode(texture2D(normalMap,tex_coord)));
#endif
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
  speccolor  *= (1.0-damage.x);
  glowcolor  *= (1.0-damage.x);
  
  //materials
  vec4 mattype, mtl_gloss;
  vec3 diff_col, mspec_col, glow_col;
  float alpha, nD, UAO;
  mattype_init
  (
    diffcolor, speccolor, glowcolor,
    mtl_gloss, mattype,
    diff_col, mspec_col, glow_col,
    alpha, nD, UAO
  );
  
#if SHOW_SPECIAL == SHOW_NO_SPECIAL
  float rdim;
  vec3 reflection;
  GAR( eye, normal, vnormal, mtl_gloss.z, reflection, rdim );
  
  //DIELECTRIC REFLECTION
#if SUPRESS_DIELECTRIC == 0
  float fresnel_refl = is_dielk * fresnel( dot( reflection, normal), nD, is_glass );
#else
  float fresnel_refl = 0.0;
#endif
  float fresnel_refr = 1.0 - fresnel_refl;

#if SUPRESS_LIGHTS == 0
  // Init lighting accumulators
  vec3 light_acc    = vec3(0.0);
  vec3 diffuse_acc  = vec3(0.0);
  vec3 specular_acc = vec3(0.0);
  // Do lighting for every active light
  float mtl_gloss_sa = mtl_gloss.w;
  if (light_enabled[0] != 0)
     lighting0(normal, vnormal, eye, reflection, mtl_gloss_sa, nD, light_acc, diffuse_acc, specular_acc);
  if (light_enabled[1] != 0)
     lighting1(normal, vnormal, eye, reflection, mtl_gloss_sa, nD, light_acc, diffuse_acc, specular_acc);
#endif
     
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
  
  //Gather the reflectivities:
  vec3 dielectric_specularity = vec3( fresnel_refl );
#if SUPRESS_SPECULAR == 0
  vec3 metallic_specularity = mspec_col * ( is_metal * fresnel_refr * alpha );
#else
  vec3 metallic_specularity = vec3(0.0);
#endif
  vec3 total_specularity = dielectric_specularity + metallic_specularity;
#if SUPRESS_DIFFUSE == 0
  vec3 diffuse_reflectivity = diff_col * ( fresnel_refr * alpha );
#endif
  
  //Multiply and Add:
  vec3 final_reflected = incoming_specular_light * total_specularity;
#if SUPRESS_DIFFUSE == 0
  final_reflected += ( incoming_diffuse_light * diffuse_reflectivity );
#endif
  
  //FINAL PIXEL WRITE:
  //Restore gamma, add alpha, and Commit:
  gl_FragColor = vec4( sqrt(final_reflected), max(fresnel_refl,alpha) ) * cloaking.rrrg;
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
}


