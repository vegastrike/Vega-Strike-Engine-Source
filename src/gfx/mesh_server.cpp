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

void Mesh::CreateLogos(int faction, Flightgroup * fg) {
}
Texture * Mesh::TempGetTexture (int index, std::string factionname)const {
	return NULL;    
}
Texture * createTexture( const char * filename, int stage)
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

void Mesh::Draw(float lod, const Matrix &m, float toofar, short cloak, float nebdist)
{
}
void Mesh::DrawNow(float lod,  bool centered, const Matrix &m, short cloak, float nebdist) {
}
void Mesh::ProcessZFarMeshes () {
}

void Mesh::ProcessUndrawnMeshes(bool pushSpecialEffects) {
}
void Mesh::ProcessDrawQueue(int whichdrawqueue) {
}
