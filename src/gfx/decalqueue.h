#include <vector>
#include <string>
class DecalQueue {
  std::vector<int> decalref;
  std::vector<Texture *> decal;
 public:
  DecalQueue(){}
  inline Texture * GetTexture (const unsigned int ref) {
    return decal[ref];
  }
  unsigned int AddTexture (const char *texname, enum FILTER mipmap) {
    Texture * tmpDecal = Texture::Exists(std::string(texname));
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
      if (nullio!=-1) {
	retval = nullio;
	decal[nullio]=new Texture (texname,0,mipmap);
	decalref[nullio]=1;
      } else{
	retval = decal.size();
	decal.push_back(new Texture (texname,0,mipmap));
	decalref.push_back (1);
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
