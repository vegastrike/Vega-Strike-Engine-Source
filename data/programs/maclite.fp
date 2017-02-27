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
varying vec4 tc0;
varying vec4 tc1;
vec4 tc2,tc3,tc4,tc5,tc6,tc7;
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
   /* //Old manual sqrt version
   float fzp1=f.z+1.0;
   float m=2.0*sqrt(f.x*f.x+f.y*f.y+(fzp1)*(fzp1));
   return vec2(f.x/m+.5,f.y/m+.5);
   */
   vec3 fzp1=vec3(f.x,f.y,f.z+1.0);
   return 0.5*(normalize(fzp1).xy)+vec2(0.5,0.5);
}

void main() {
  //begin bumpmapping
  vec3 iNormal=tc1.xyz;
  vec3 iTangent=tc2.xyz;
  vec3 iBinormal=tc3.xyz;
//download matrix and dont recompute (as below you might... slightly higher precision, no visual diff)
  iBinormal=vec3(gl_ModelViewMatrix[2][0],gl_ModelViewMatrix[2][1],gl_ModelViewMatrix[2][2]);
  iTangent=normalize(cross(iBinormal,iNormal));
  iBinormal=cross(iNormal,iTangent);
  

  vec3 iLightVec=tc5.xyz;
  vec3 pos=vec3(tc0.z,tc0.w,tc1.w);
  vec3 iEyeDir=normalize(pos);
  vec3 lightDir=normalize(gl_LightSource[0].position.xyz-pos*gl_LightSource[0].position.w);//iLightVec;//lightDir=normalize(matmul(iTangent,iBinormal,iNormal,iLightVec));
  vec3 eyeDir=normalize(pos);//eyeDir=normalize(matmul(iTangent,iBinormal,iNormal,iEyeDir));
//keep everything in world space
  //vec3 halfAngle=normalize(eyeDir+lightDir);
  vec3 normal;//=normalize(expand(texture2D(normalMap,tc0.xy).wyz));
//transform normal from normalMap to world space
  normal=normalize(imatmul(iTangent,iBinormal,iNormal,expand(texture2D(normalMap,tc0.xy).wyz)));
  //begin shading
//compute half angle dot with light (not used)
  //float nDotH=dot(normal,halfAngle);
//compute normal dot with light
  float nDotL=dot(normal,lightDir);
//compute eye direction reflected across normal
  vec3 reflection=reflect(-eyeDir,normal);
//compute dot light dir and reflected vector
  float lDotR=dot(lightDir,reflection);
//get damaged and undamaged colors
  vec4 undamagedcolor=texture2D(diffuseMap,tc0.xy);
  vec4 damagecolor=texture2D(damageMap,tc0.xy);
//mix the colors based on damage... if you had detail maps, these would go here
  vec4 diffsurface=
	mix(undamagedcolor,damagecolor,damage.x);
	//	+texture2D(detail0Map,tc0.xy);//do not support yet
//have a specular surface unless the damage color is quite bright (maybe dark!?)
  vec4 specsurface=texture2D(specMap,tc0.xy)*(1.0-damagecolor*damage.x*2.0);
//add in lights 1..8 and ambient terms
  vec4 ambient=gl_Color;
//add in the dot of light and normal* diffuse mat*diffuse light
  ambient+=max(nDotL,0.0)*gl_FrontLightProduct[0].diffuse;
//find the color of specularity of this surface... lookup in env map the normal
  vec4 specularity=gl_SecondaryColor+texture2D(envMap,EnvMapGen(reflection));
//add in the lDotR product
 specularity+=pow(max(lDotR,0.0),gl_FrontMaterial.shininess)*gl_FrontLightProduct[0].specular;
//sum everything up including glow from surface...and if cloaking, fade it out :-)
  gl_FragColor=(ambient*diffsurface+specularity*specsurface+texture2D(glowMap,tc0.xy))*cloaking.rrrg;
}
