uniform sampler2D diffuseMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;

void main() 
{
  // Sample textures
  vec4 diffusemap  = texture2D(diffuseMap, gl_TexCoord[0].xy);
  
  gl_FragColor = diffusemap * gl_Color;
  gl_FragColor.rgb += gl_SecondaryColor.rgb;
  gl_FragColor *= cloaking.rrrg;
}
