uniform int light_enabled[gl_MaxLights];
uniform int max_light_enabled;
uniform sampler2D diffuseMap;
uniform sampler2D envMap;
uniform sampler2D specMap;
uniform sampler2D glowMap;
uniform sampler2D normalMap;
uniform sampler2D damageMap;
uniform sampler2D detail0Map;
uniform sampler2D detail1Map;
uniform vec4 cloaking;
uniform vec4 damage;
//varying vec4 gl_FrontColor;
vec3 matmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return vec3(dot(lightVec,tangent),dot(lightVec,binormal),dot(lightVec,normal));
}
vec3 imatmul(vec3 tangent, vec3 binormal, vec3 normal,vec3 lightVec) {
  return lightVec.xxx*tangent+lightVec.yyy*binormal+lightVec.zzz*normal;
}

vec3 expand(vec3 expandme) {
  //debug only--if you dont want to use bump mapreturn vec3(0,0,1);
  return expandme*vec3(2.0)-vec3(1.0);
}
vec2 EnvMapGen(vec3 f) {
   float fzp1=f.z+1.0;
   float m=2.0*sqrt(f.x*f.x+f.y*f.y+(fzp1)*(fzp1));
   return vec2(f.x/m+.5,f.y/m+.5);
}

void main() {
//get damaged and undamaged colors
  vec4 undamagedcolor=texture2D(diffuseMap,gl_TexCoord[0].xy);
  vec4 damagecolor=texture2D(damageMap,gl_TexCoord[0].xy);
//mix the colors based on damage... if you had detail maps, these would go here
  vec4 diffsurface=
	mix(undamagedcolor,damagecolor,damage.x);
	//	+texture2D(detail0Map,gl_TexCoord[7].xy);//do not support yet
//have a specular surface unless the damage color is quite bright (maybe dark!?)
  vec4 specsurface=texture2D(specMap,gl_TexCoord[0].xy)*(1.0-damagecolor*damage.x*2.0);
//add in lights 0..8 and ambient terms
  vec4 ambient=gl_Color;
//add in the dot of light and normal* diffuse mat*diffuse light
//find the color of specularity of this surface... lookup in env map the normal
  vec4 specularity=gl_SecondaryColor+texture2D(envMap,gl_TexCoord[1].xy);
//sum everything up including glow from surface...and if cloaking, fade it out :-)
  gl_FragColor=(ambient*diffsurface+specularity*specsurface+texture2D(glowMap,gl_TexCoord[0].xy))*cloaking.rrrg;
}
