#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
//#include "unit_collide.h"
#include "gfx/aux_texture.h"
#include "gfx/decalqueue.h"
using std::vector;
#include "audiolib.h"
#include "configxml.h"
#include "images.h"
static DecalQueue beamdecals;
static vector <vector <DrawContext> > beamdrawqueue;

Beam::Beam (const Transformation & trans, const weapon_info & clne, void * own, int sound) :vlist(NULL), Col(clne.r,clne.g,clne.b,clne.a){
  VSCONSTRUCT2('B')
    listen_to_owner=false;//warning this line of code is also present in beam_server.cpp change one, change ALL
#ifdef PERBOLTSOUND
  sound = AUDCreateSound (clne.sound,true);
#else
  this->sound = sound;
#endif
  decal = beamdecals.AddTexture (clne.file.c_str(),TRILINEAR);
  if (decal>=beamdrawqueue.size()) {
    beamdrawqueue.push_back (vector<DrawContext>());
  }
  Init(trans,clne,own);
}

Beam::~Beam () {
  VSDESTRUCT2
  //  fprintf (stderr,"Deleting %x",this);
#ifdef PERBOLTSOUND
  AUDDeleteSound (sound);
#endif
#ifdef BEAMCOLQ
  RemoveFromSystem(true);
#endif
  delete vlist;
  beamdecals.DelTexture(decal);
}

extern void AdjustMatrixToTrackTarget (Matrix &mat,Unit * target, float speed, bool lead, float cone);

void Beam::Draw (const Transformation &trans, const Matrix &m, Unit * targ, float tracking_cone) {//hope that the correct transformation is on teh stack
  if (curthick==0) 
    return;
  Matrix cumulative_transformation_matrix;
  local_transformation.to_matrix(cumulative_transformation_matrix);
  Transformation cumulative_transformation = local_transformation;
  cumulative_transformation.Compose(trans, m);
  cumulative_transformation.to_matrix(cumulative_transformation_matrix);
  AdjustMatrixToTrackTarget (cumulative_transformation_matrix,targ,speed,false,tracking_cone);
#ifdef PERFRAMESOUND
  AUDAdjustSound (sound,cumulative_transformation.position,speed*Vector (cumulative_transformation_matrix[8],cumulative_transformation_matrix[9],cumulative_transformation_matrix[10]));
#endif
  AUDSoundGain (sound,curthick*curthick/(thickness*thickness));

  RecalculateVertices();

  beamdrawqueue[decal].push_back(DrawContext (cumulative_transformation_matrix,vlist));

}

void Beam::ProcessDrawQueue() {
  //  fprintf (stderr,"DrawingAll\n");
    GFXDisable (LIGHTING);
    GFXDisable (CULLFACE);//don't want lighting on this baby
    GFXDisable (DEPTHWRITE);
    GFXPushBlendMode();
    static bool blendbeams = XMLSupport::parse_bool (vs_config->getVariable("graphics","BlendGuns","true"));
    if (blendbeams==true) {
      GFXBlendMode(ONE,ONE);
    }else {
      GFXBlendMode(ONE,ZERO);
    }

  GFXEnable (TEXTURE0);
  GFXDisable (TEXTURE1);
  DrawContext c;
  for (unsigned int decal = 0;decal < beamdrawqueue.size();decal++) {	
    Texture * tex = beamdecals.GetTexture(decal);
    if (tex) {
      tex->MakeActive();
      if (beamdrawqueue[decal].size()) {
	beamdrawqueue[decal].back().vlist->LoadDrawState();//loads clarity+color
	while (beamdrawqueue[decal].size()) {
	  c= beamdrawqueue[decal].back();
	  beamdrawqueue[decal].pop_back();
	  GFXLoadMatrixModel ( c.m);
	  c.vlist->BeginDrawState(GFXFALSE);
	  c.vlist->Draw();
	  c.vlist->EndDrawState(GFXFALSE);
	}
      }
    }
  }
  //  GFXEnable (TEXTURE1);
  GFXEnable (DEPTHWRITE);
  GFXEnable (CULLFACE);
  GFXDisable (LIGHTING);
  GFXPopBlendMode();
}
