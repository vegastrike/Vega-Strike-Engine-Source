struct GFXMaterial
{  /// ambient rgba, if you don't like these things, ask me to rename them
	float ar;float ag;float ab;float aa;
  /// diffuse rgba
	float dr;float dg;float db;float da;
  /// specular rgba
	float sr;float sg;float sb;float sa;
  /// emissive rgba
	float er;float eg;float eb;float ea;
  /// specular power
	float power; 
   GFXMaterial () {
      dr=dg=db=da=sr=sg=sb=sa=ea=aa=1.0f;
      er=eg=eb=ar=ag=ab=0.0f;
      power=60.0f;
      //defaults for material struct    
   }
};
enum BLENDFUNC{
    ZERO            = 1,
	ONE             = 2, 
    SRCCOLOR        = 3,
	INVSRCCOLOR     = 4, 
    SRCALPHA        = 5,
	INVSRCALPHA     = 6, 
    DESTALPHA       = 7,
	INVDESTALPHA    = 8, 
    DESTCOLOR       = 9,
	INVDESTCOLOR    = 10, 
    SRCALPHASAT     = 11,
    CONSTALPHA    = 12, 
    INVCONSTALPHA = 13,
    CONSTCOLOR = 14,
    INVCONSTCOLOR = 15
};
struct Mesh_vec3f {
   float x;
   float y;
   float z;
};
enum textype{
	ALPHAMAP,
	ANIMATION,
	TEXTURE
};
