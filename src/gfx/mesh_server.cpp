#include "mesh.h"

#ifdef __cplusplus
extern "C"
{
void winsys_exit( int code )
{
	exit(code);
}
}
#endif

void Mesh::RestoreCullFace (int whichdrawqueue) {
}
void Mesh::SelectCullFace (int whichdrawqueue) {
}
void Mesh::CreateLogos(int faction, Flightgroup * fg) {
}
Texture * Mesh::TempGetTexture (int index, std::string factionname)const {
	return NULL;    
}
Texture * createTexture( const char * filename, int stage=0,enum FILTER f1= MIPMAP,enum TEXTURE_TARGET t0=TEXTURE2D,enum TEXTURE_IMAGE_TARGET t=TEXTURE_2D,unsigned char c=GFXFALSE,int i=65536)
{
	return NULL;
}
Texture * createTexture( char const * ccc,char const * cc,int k= 0,enum FILTER f1= MIPMAP,enum TEXTURE_TARGET t0=TEXTURE2D,enum TEXTURE_IMAGE_TARGET t=TEXTURE_2D,float f=1,int j=0,unsigned char c=GFXFALSE,int i=65536)
{
	return NULL;
}
AnimatedTexture * createAnimatedTexture( char const * c,int i,enum FILTER f)
{
	return NULL;
}
Logo * createLogo(int numberlogos,Vector* center, Vector* normal, float* sizes, float* rotations, float offset, Texture * Dec, Vector *Ref)
{
	return NULL;
}

Mesh::~Mesh()
{
	if(!orig||orig==this)
	{
	  delete vlist;
	  if (meshHashTable.Get(hash_name)==this)
	    meshHashTable.Delete(hash_name);
	  
	  orig->refcount--;
	  //printf ("orig refcount: %d",refcount);
	  if(orig->refcount == 0)
	    delete [] orig;	      
	}
}

void Mesh::Draw(float lod, const Matrix &m, float toofar, short cloak, float nebdist, unsigned char damage, bool renormalize)
{
}
void Mesh::DrawNow(float lod,  bool centered, const Matrix &m, short cloak, float nebdist) {
}
void Mesh::ProcessZFarMeshes () {
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects) {
}
void Mesh::ProcessDrawQueue(int,int whichdrawqueue) {
}
void AddWarp (Unit * un, QVector, float) {}
void WarpTrailDraw() {}
