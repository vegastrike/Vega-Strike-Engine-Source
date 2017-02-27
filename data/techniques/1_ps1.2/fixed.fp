uniform sampler2D diffuseMap;
uniform sampler2D envMap;
uniform sampler2D specMap;
uniform sampler2D glowMap;
uniform sampler2D damageMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;

float lerp(float f, float a, float b){return (1.0-f)*a+f*b; }
vec2  lerp(float f, vec2 a, vec2 b) { return (1.0-f)*a+f*b; }
vec3  lerp(float f, vec3 a, vec3 b) { return (1.0-f)*a+f*b; }
vec4  lerp(float f, vec4 a, vec4 b) { return (1.0-f)*a+f*b; }

void main() 
{
  // Sample textures
  vec4 damagecolor = texture2D(damageMap , gl_TexCoord[0].xy);
  vec4 diffusecolor= texture2D(diffuseMap, gl_TexCoord[0].xy);
  vec4 speccolor   = texture2D(specMap   , gl_TexCoord[0].xy);
  vec4 glowcolor   = texture2D(glowMap   , gl_TexCoord[0].xy);
  vec4 env         = texture2D(envMap    , gl_TexCoord[1].xy);
  vec4 diffusemap  = lerp(damage.x, diffusecolor, damagecolor);
  vec4 specmap     = speccolor;
  
  gl_FragColor = diffusemap * gl_Color;
  gl_FragColor.rgb += specmap.rgb * (env * envColor + gl_SecondaryColor).rgb;
  gl_FragColor.rgb += glowcolor.rgb;
  gl_FragColor *= cloaking.rrrg;
}
