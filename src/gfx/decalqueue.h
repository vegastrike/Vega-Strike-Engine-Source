#include <vector>
#include <string>
#include "vs_globals.h"
class DecalQueue {
  int nullity;
  std::vector<int> decalref;
  std::vector<Texture *> decal;
 public:
  DecalQueue(){
    nullity=-1;
  }
  inline Texture * GetTexture (const unsigned int ref) {
    return decal[ref];
  }
  unsigned int AddTexture (const char *texname, enum FILTER mipmap) {
    Texture * tmpDecal = Texture::Exists(std::string(texname));
    if (nullity!=-1&&tmpDecal==NULL&&!g_game.use_textures) {
      tmpDecal=decal[nullity];
    }
    unsigned int i=0;
    unsigned int retval=0;
    int nullio=-1;
    if (tmpDecal) {
      for (;i<decal.size();i++) {
	if (decal[i]) {
	  if ((*decal[i])==(*tmpDecal)) {
	    retval = i;
	    decalref[i]++;
	    break;
	  }
	}
	if (!decal[i]&&decalref[i]==0)
	  nullio=i;
      }
    }
    if (!tmpDecal||i==decal.size()) { //make sure we have our own to delete upon refcount =0
      Texture * tex=new Texture (texname,0,mipmap,TEXTURE2D,TEXTURE_2D,GFXTRUE);
      if ((nullity==-1)||tex->LoadSuccess()) {
	if (nullio!=-1) {
	  retval = nullio;
	  decal[nullio]=tex;
	  decalref[nullio]=1;
	} else{
	  retval = decal.size();
	  decal.push_back(tex);
	  decalref.push_back (1);
	}
      }
      if (!tex->LoadSuccess()) {
	if (nullity==-1) {
	  nullity=retval;
	}else {
	  delete tex;
	  tex=NULL;
	  retval=nullity;
	  decalref[nullity]++;
	}
      }
    }
    return retval;
  }
  bool DelTexture (unsigned int ref) {
    decalref[ref]--;
    if (decalref[ref]<=0) {
      delete decal[ref];
      decal[ref]=NULL;
      return true;
    }
    return false;
  }
};
