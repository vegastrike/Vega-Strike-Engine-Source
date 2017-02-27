uniform sampler2D diffuseMap;
uniform sampler2D envMap;
uniform sampler2D specMap;
uniform sampler2D glowMap;
uniform sampler2D damageMap;
uniform vec4 cloaking;
uniform vec4 damage;
uniform vec4 envColor;
uniform vec4 ambientLevel;

float lerp(float f, float a, float b){return (1.0-f)*a+f*b; }
vec2  lerp(float f, vec2 a, vec2 b) { return (1.0-f)*a+f*b; }
vec3  lerp(float f, vec3 a, vec3 b) { return (1.0-f)*a+f*b; }
vec4  lerp(float f, vec4 a, vec4 b) { return (1.0-f)*a+f*b; }

vec3  cel(vec3 x, float ambient)
{
    float l = length(x);
    if (l >= 0.25)
        return normalize(x) * ceil(l*4.0) * 0.25; 
    else if (l >= ambientLevel.x)
        return normalize(x) * ambientLevel.y;
    else
        return x;
}

void main() 
{
  // Sample textures
  vec4 diffusemap  = texture2D(diffuseMap, gl_TexCoord[0].xy);
  
  vec3 d = diffusemap.rgb * gl_Color.rgb;
  vec3 s = gl_SecondaryColor.rgb;
  
  vec4 rv;
  rv.rgb = cel(d + s, ambientLevel);
  rv.a = diffusemap.a * gl_Color.a;
  
  gl_FragColor = rv * cloaking.rrrg;
}
