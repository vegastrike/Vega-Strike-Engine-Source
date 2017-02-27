uniform sampler2D diffuseMap;

void main() 
{
  // Just sample the diffuse map - color doesn't matter, but alpha does!
  gl_FragColor = texture2D(diffuseMap, gl_TexCoord[0].xy);
}
