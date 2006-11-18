/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <assert.h>
#include "gfxlib.h"
#include <string>
#include "endianness.h"
#include "hashtable.h"
#include "vsfilesystem.h"
#include "vsimage.h"
#include "vs_globals.h"
#include "in_kb.h"
#include "main_loop.h"
#include "aux_texture.h"

using std::string;
using namespace VSFileSystem;

///holds all the textures in a huge hash table
Hashtable<string, Texture, 4007> texHashTable;
Hashtable<string, bool, 4007> badtexHashTable;
///returns if a texture exists
Texture * Texture::Exists (string s, string a) {
  return Texture::Exists (s+a);
}
Texture * Texture::Exists (string s) {
  Texture * tmp = texHashTable.Get(VSFileSystem::GetHashName(s));
  if (tmp==NULL){
    string tmpo;
    tmp = texHashTable.Get (VSFileSystem::GetSharedTextureHashName (s));
  }
  if (tmp)
    return tmp->Original();
  return tmp;
}

bool Texture::operator < (Texture & b) {
  return Original()<b.Original();
}

bool Texture::operator == (Texture & b) {
  return Original()==b.Original();
}

void Texture::setReference(Texture *other)
{
    original = other;
    original->refcount++;

	// Copy shared attributes
	texfilename = other->texfilename;
	data = other->data;
	name = other->name;
	bound = other->bound;
	boundSizeX = other->boundSizeX;
	boundSizeY = other->boundSizeY;
	boundMode = other->boundMode;
	texture_target = other->texture_target;
	image_target = other->image_target;
}

GFXBOOL Texture::checkold(const string &s, bool shared, string & hashname)
{
  hashname = shared?VSFileSystem::GetSharedTextureHashName(s):VSFileSystem::GetHashName(s);
  Texture *oldtex= texHashTable.Get (hashname);
  if(oldtex!=NULL) {
	//*this = *oldtex;//will be obsoleted--unpredictable results with string()
    setReference(oldtex);
	//cerr<<"Found cached texture : "<<s<<" with hashname="<<hashname<<endl;
    return GFXTRUE;
  } else {
    return GFXFALSE;
  }
}
void Texture::modold (const string &s, bool shared, string & hashname) {
  hashname = shared?VSFileSystem::GetSharedTextureHashName(s):VSFileSystem::GetHashName(s);
  Texture * oldtex = new Texture;
  //  oldtex->InitTexture();new calls this
  oldtex->name=-1;
  oldtex->refcount=1;
  oldtex->original=NULL;
  oldtex->palette=NULL;
  oldtex->data=NULL;
  texHashTable.Put(hashname, oldtex);
  original = oldtex;
}
Texture::Texture () {
  data=NULL;
  InitTexture();
  name=-1;
  palette=NULL;
}

void Texture::setold()
{
  //	*original = *this;//will be obsoleted in new C++ standard unpredictable results when using string() (and its strangeass copy constructor)
  *original = *this;
  //memcpy (original, this, sizeof (Texture));
	original->original = NULL;
	original->refcount++;
}
Texture * Texture::Original() {
  if (original) {
    return original->Original();
  }else {
    return this;
  }
}

Texture *Texture::Clone () {
  Texture * retval = new Texture();
  Texture * target = Original();
  *retval = *target;
  //  memcpy (this, target, sizeof (Texture));
  if (retval->name!=-1) {
    retval->original = target;
    retval->original->refcount++;
  } else {
    retval->original = NULL;
  }
  retval->refcount = 0;
  return retval;
  //assert (!original->original);
  
}
void Texture::FileNotFound(const string &texfilename) {
	  // We may need to remove from texHashTable if we found the file but it is a bad one
	  texHashTable.Delete (texfilename);

	  setbad( texfilename);
	  name=-1;
	  data = NULL;
	  if( original != NULL)
	  {
	  	original->name=-1;
	  	delete original;
	  	original=NULL;
	  }
	  palette=NULL;

	  return;
}

bool Texture::checkbad(const string & s)
{
  string hashname = VSFileSystem::GetSharedTextureHashName(s);
  bool * found=NULL;
  found = badtexHashTable.Get (hashname);
  if(found!=NULL) {
	return true;
  }
  hashname = VSFileSystem::GetHashName(s);
  found= badtexHashTable.Get (hashname);
  if(found!=NULL) {
	return true;
  }
  return false;
}

void Texture::setbad( const string & s)
{
	// Put both current path+texfile and shared texfile since they both have been looked for
	bool * b = new bool( true);
	if( VSFileSystem::current_path.back()!="")
		badtexHashTable.Put( VSFileSystem::GetHashName(s), b);
	badtexHashTable.Put( VSFileSystem::GetSharedTextureHashName(s), b);
}

Texture::Texture(VSFile * f, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, GFXBOOL force_load, int maxdimension,GFXBOOL detailtexture,GFXBOOL nocache,enum ADDRESSMODE address_mode)
{

  data = NULL;
  ismipmapped  = mipmap;
  InitTexture();
  palette = NULL;
  texture_target =target;
  image_target=imagetarget;
  this->stage = stage;
  data = this->ReadImage( f, NULL, true, NULL);

	if (data)
	{
		Bind(maxdimension,detailtexture);
		free(data);
		data = NULL;
		if (!nocache) setold();
	}
	else
		FileNotFound(texfilename);
}

Texture::Texture(const char * FileName, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, GFXBOOL force_load, int maxdimension,GFXBOOL detailtexture,GFXBOOL nocache,enum ADDRESSMODE address_mode)
{
    InitTexture();
    Load(FileName,stage,mipmap,target,imagetarget,force_load,maxdimension,detailtexture,nocache,address_mode);
}

void Texture::Load(const char * FileName, int stage, enum FILTER mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, GFXBOOL force_load, int maxdimension,GFXBOOL detailtexture,GFXBOOL nocache,enum ADDRESSMODE address_mode)
{
  if (data) free(data);
  if (palette) free(palette);
  data = NULL;
  palette = NULL;
  ismipmapped  = mipmap;
  texture_target =target;
  image_target=imagetarget;
  this->stage = stage;
  this->address_mode = address_mode;
  string texfn = string(FileName);
  if(checkbad(texfn))
  	return;
  if (!nocache) {
	  string tempstr;
      if (  checkold(texfn,false,tempstr)
		  ||checkold(texfn,true,tempstr)  ) {
        texfilename = tempstr;
        return;
      }
  }

  //VSFileSystem::vs_fprintf (stderr,"1.Loading bmp file %s ",FileName);
	char *t= strdup (FileName);
	int tmp = strlen(FileName);
	if (tmp>3) {
	  t[tmp-3] = 'a';
	  t[tmp-2] = 'l';
	  t[tmp-1] = 'p';
	}
	VSFile f2;
	VSError err2=VSFileSystem::FileNotFound;
	if (t){
		if (t[0]!='\0'){
		static bool use_alphamap              = parse_bool(vs_config->getVariable("graphics",
                                                  "bitmap_alphamap",
                                                  "true"));
		if (use_alphamap)
		    err2 = f2.OpenReadOnly(t, TextureFile);
		}
	}

	
	if(err2<=Ok) {
	  //texfilename += string(t);
	}
	//	this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());
	VSFile f;
	VSError err;
	if (FileName)
	  if (FileName[0])
	    err = f.OpenReadOnly (FileName, TextureFile);
	bool shared = (err==Shared);
	//modold (texfilename,shared,texfilename);
	/*
	if (shared) {
	  if (FileName)
	    if (FileName[0]) {
	      //string tmp =VSFileSystem::GetSharedTexturePath (FileName);
	      err = f.OpenReadOnly (FileName,TextureFile);
	    }
	}
	*/
	free ( t);
	if (err<=Ok&&g_game.use_textures==0&&!force_load) {
	  f.Close();
	  err = Unspecified;
	}
	if (err>Ok)
	{
		FileNotFound(texfn);
      	VSFileSystem::vs_fprintf (stderr, "\n%s, not found\n",FileName);
		if (err2<=Ok) {
			f2.Close();
		}
		return;
	}
	if (!nocache) {
		string tempstr;
		modold (texfn,shared,tempstr);
		texfilename = tempstr;
	}
	if (texfn.find("white")==string::npos)
		bootstrap_draw("Loading "+string(FileName));
	
	//	strcpy(filename, FileName);
	if( err2>Ok)
		data = this->ReadImage( &f, NULL, true, NULL);
	else
		data = this->ReadImage( &f, NULL, true, &f2);
	/*
	data = readImage (f,bpp,format,sizeX,sizeY,palette,NULL,true);
	if (data) {
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(format&PNG_HAS_COLOR)||(format&PNG_HAS_PALETTE)) {
	    mode=_8BIT;
	    if (!(format&PNG_HAS_COLOR)){
	      palette = (unsigned char *) malloc(sizeof(unsigned char)*(256*4+1));
	      for (unsigned int i =0;i<256;i++) {
		palette[i*4]=i;
		palette[i*4+1]=i;
		palette[i*4+2]=i;
		palette[i*4+3]=255;
	      }
	    }
	  } 
	  if (format&PNG_HAS_COLOR) {
	    if (format&PNG_HAS_ALPHA) {
	      mode=_24BITRGBA;
	    } else {
	      mode=_24BIT;
	    }
	  }
	}else {
	  char head1;
	  char head2;
	  f.Begin();
	  f.Read (&head1, 1);
	  f.Read(&head2,1);
	  if (toupper(head1)!='B'||toupper (head2)!='M') {
		FileNotFound(texfilename);
		f.Close();
		if (err2<=Ok) {
			f2.Close();
		}
		return;
	  }
	  //seek back to beginning
	  f.GoTo(SIZEOF_BITMAPFILEHEADER);
	  //long temp;
	  BITMAPINFOHEADER info;
	  f.Read(&info, SIZEOF_BITMAPINFOHEADER);
	  sizeX = le32_to_cpu(info.biWidth);	
	  sizeY = le32_to_cpu(info.biHeight);
	  
	  
	  //while(1);
	  
	  if(le16_to_cpu(info.biBitCount) == 24)
	    {
	      mode = _24BITRGBA;
	      if(err2<=Ok)
		mode = _24BITRGBA;
	      data = NULL;
	      data= (unsigned char *)malloc (sizeof(unsigned char)*4*sizeY*sizeX); // all bitmap data needs to be 32 bits
	      if (!data)
		return;
	      for (int i=sizeY-1; i>=0;i--)
		{
		  int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		  for (unsigned int j=0; j<sizeX;j++)
		    {
		      if(err2<=Ok)
					f2.Read(data+3, sizeof(unsigned char));
		      else
			*(data+3) = 0xff; // default alpha = 1
		      for (int k=2; k>=0;k--)
			{
					f.Read (data+k+4*j+itimes4width,sizeof (unsigned char));
			}
		      
		    }
		}
	    }
	  else if(le16_to_cpu(info.biBitCount) == 8)
	    {
	      mode = _8BIT;
	      data = NULL;
	      data= (unsigned char *)malloc(sizeof(unsigned char)*sizeY*sizeX);
	      palette = (unsigned char *)malloc(sizeof(unsigned char)* (256*4+1));
	      memset (palette,0,(256*4+1)*sizeof(unsigned char));
	      unsigned char *paltemp = palette;
	      unsigned char ctemp;
		for(int palcount = 0; palcount < 256; palcount++)
		  {
		    f.Read(paltemp, SIZEOF_RGBQUAD);
		    ctemp = paltemp[0];
		    paltemp[0] = paltemp[2];
		    paltemp[2] = ctemp;
		    paltemp+=4; // pal size
		  }
		if (!data)
		  return;
		for (int i=sizeY-1; i>=0;i--)
		  {
		    for (unsigned int j=0; j<sizeX;j++)
		      {
				f.Read(data+ j + i * sizeX,sizeof (unsigned char));
		      }
		  }
	    }
	}
	//VSFileSystem::vs_fprintf (stderr,"Bind... ");
 	f.Close();
	if (err2<=Ok)
	  f2.Close();
	*/
	if (data)
	{
		Bind(maxdimension,detailtexture);
		free(data);
		data = NULL;
		if (!nocache) setold();
	}
	else
		FileNotFound(texfilename);
	f.Close();
	if( f2.Valid())
		f2.Close();
	//VSFileSystem::vs_fprintf (stderr," Load Success\n");
}

Texture::Texture (const char * FileNameRGB, const char *FileNameA, int stage, enum FILTER  mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval, GFXBOOL force_load, int maxdimension,GFXBOOL detailtexture,GFXBOOL nocache,enum ADDRESSMODE address_mode)
{
    InitTexture();
    Load(FileNameRGB, FileNameA, stage, mipmap, target, imagetarget, alpha, zeroval, force_load, maxdimension, detailtexture, nocache,address_mode);
}

void Texture::Load (const char * FileNameRGB, const char *FileNameA, int stage, enum FILTER  mipmap, enum TEXTURE_TARGET target, enum TEXTURE_IMAGE_TARGET imagetarget, float alpha, int zeroval, GFXBOOL force_load, int maxdimension,GFXBOOL detailtexture,GFXBOOL nocache,enum ADDRESSMODE address_mode)
{
  if (data) free(data);
  if (palette) free(palette);
  data = NULL;
  palette = NULL;
  ismipmapped  = mipmap;

	this->stage = stage;
    this->address_mode = address_mode;
	texture_target=target;
	image_target=imagetarget;
	string texfn = string(FileNameRGB) + string("|") + string(FileNameA);
    if (!nocache) {
	  string tempstr;
      if (  checkold(texfn,false,tempstr)
		  ||checkold(texfn,true,tempstr)  ) {
        texfilename = tempstr;
        return;
      }
    }
	//VSFileSystem::vs_fprintf (stderr,"2.Loading bmp file %s alp %s ",FileNameRGB,FileNameA);
	//this->texfilename = texfilename;
	//strcpy (filename,texfilename.c_str());
	VSFile f;
	VSError err = Unspecified;
	err = f.OpenReadOnly(FileNameRGB, TextureFile);
    if (!nocache) {
	    bool shared = (err==Shared);
		string tempstr;
	    modold (texfn,shared,tempstr);
		texfilename = tempstr;
    }
	if (err<=Ok&&g_game.use_textures==0&&!force_load) {
	  f.Close();
	  err = Unspecified;
	}
	if (err>Ok)
	{
	  FileNotFound(texfilename);
	  return;
	}
	VSFile f1;
	VSError err1=Unspecified;
	bool shared1;
	if (FileNameA)
	{
           static bool use_alphamap 
              = parse_bool(vs_config->getVariable("graphics",
                                                  "bitmap_alphamap",
                                                  "true"));
           if (use_alphamap) {
              std::string tmp;
              f1.OpenReadOnly( FileNameA, TextureFile);
              
              shared1 = (err1==Shared);
              if (err1>Ok)
                 {
                    data = NULL;
                    FileNameA = NULL;
                    //VSFileSystem::vs_close(fp);
                    //*this = Texture(FileNameRGB, NULL);
                    //return;
                 }
           }else FileNameA=0;
	}
	if( err1>Ok)
		data = this->ReadImage( &f, NULL, true, NULL);
	else
		data = this->ReadImage( &f, NULL, true, &f1);
	/*
	if (data) {
	  //FIXME deal with palettes and grayscale with alpha
	  if (!(format&PNG_HAS_COLOR)||(format&PNG_HAS_PALETTE)) {
	    mode=_8BIT;
	    if (!(format&PNG_HAS_COLOR)){
	      palette = (unsigned char *)malloc (sizeof(unsigned char)*(256*4+1));
	      for (unsigned int i =0;i<256;i++) {
		palette[i*4]=i;
		palette[i*4+1]=i;
		palette[i*4+2]=i;
		palette[i*4+3]=255;
	      }
	    }

	  } 
	  if (format&PNG_HAS_COLOR) {
	    if (format&PNG_HAS_ALPHA) {
	      mode=_24BITRGBA;
	    } else {
	      mode=_24BIT;
	    }
	  }
	}else {
	  char head1;
	  char head2;
	  f.Begin();
	  f.Read(&head1,1);
	  f.Read(&head2,1);
	  if (toupper(head1)!='B'||toupper (head2)!='M') {
		FileNotFound(texfilename);
		f.Close();
		if (err1<=Ok) {
			f1.Close();
		}
		return;
	  }
	  ///seek back to the beginning
	  f.GoTo(SIZEOF_BITMAPFILEHEADER);
	  //long temp;
	  BITMAPINFOHEADER info;
	  f.Read(&info, SIZEOF_BITMAPINFOHEADER);
	  sizeX = le32_to_cpu(info.biWidth);
	  sizeY = le32_to_cpu(info.biHeight);
	  BITMAPINFOHEADER info1;
	  
	  if (FileNameA)
	  {
	    std::string tmp;
		f1.OpenReadOnly( FileNameA, TextureFile);
	     
		shared = (err1==Shared);
	    if (err1>Ok)
		{
		  data = NULL;
		  FileNameA = NULL;
		  //VSFileSystem::vs_close(fp);
		  // *this = Texture(FileNameRGB, NULL);
		  //return;
		}
	    else
		{
		  f1.GoTo(SIZEOF_BITMAPFILEHEADER);
		  
		  f1.Read(&info1,SIZEOF_BITMAPINFOHEADER);
		  if (sizeX != (unsigned int) le32_to_cpu(info1.biWidth)||sizeY!=(unsigned int)le32_to_cpu(info1.biHeight))
		    {
		      data = NULL;
		      f1.Close();
		      if (err<=Ok) 
				f.Close();
		      return;
		    }
		  RGBQUAD ptemp1;	
		  if (le16_to_cpu(info1.biBitCount) == 8)
		    {
		      for (int i=0; i<256; i++)
				f1.Read(&ptemp1, sizeof(RGBQUAD)); //get rid of the palette for a b&w greyscale alphamap
		      
		    }
		}
	    }
	  if(le16_to_cpu(info.biBitCount) == 24) {
	    mode = _24BITRGBA;
	    data= (unsigned char *)malloc( sizeof (unsigned char)*4*sizeY*sizeX);
	    for (int i=sizeY-1; i>=0;i--)
	      {
		int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		for (unsigned int j=0; j<sizeX;j++)
		  {
		    for (int k=2; k>=0;k--) {
		      f.Read(data+k+4*j+itimes4width,sizeof (unsigned char));
		    }
		    if (FileNameA){
		      if (le16_to_cpu(info1.biBitCount)==24)
		      for (int k=2; k>=0;k--) {
				f1.Read(data+3+4*j+itimes4width,sizeof (unsigned char));
			// *(data+3+4*j+itimes4width) = 30;
		      } else {
				f1.Read(data+3+4*j+itimes4width,sizeof (unsigned char));
		      }
		    }
		    else {
		      if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
			data[4*j+itimes4width+3] = 0;
		      else
			data[4*j+itimes4width+3] = 255;
		    }
		    // *(data+3+4*j+itimes4width) = 30;
		    
		    
		  }
	      }
	  }
	  else if(le16_to_cpu(info.biBitCount) == 8) {
	    unsigned char index = 0;
	    mode = _24BITRGBA;
	    data = NULL;
	    data= (unsigned char *)malloc(sizeof(unsigned char)*4*sizeY*sizeX);
	    palette = (unsigned char *)malloc (sizeof (unsigned char)*(256*4+1));
	    unsigned char *paltemp = palette;
	    unsigned char ctemp;
	    for(int palcount = 0; palcount < 256; palcount++) {
	      f.Read(paltemp, sizeof(RGBQUAD));
	      ctemp = paltemp[0];
	      paltemp[0] = paltemp[2];
	      paltemp[2] = ctemp;
	      paltemp+=4;
	    }
	    if (!data) {
	      if (err<=Ok)
			f.Close();
	      if (err1<=Ok)
			f1.Close();
	      return;
	    }
		//FIXME VEGASTRIKE???		int k=0;
	    for (int i=sizeY-1; i>=0;i--) {
	      for (unsigned int j=0; j<sizeX;j++)
		{
		  f.Read(&index,sizeof (unsigned char));
		  data [4*(i*sizeX+j)] = palette[((short)index)*4];	
		  data [4*(i*sizeX+j)+1] = palette[((short)index)*4+1];
		  data [4*(i*sizeX+j)+2] = palette[((short)index)*4+2];
		}
	    }
	    delete [] palette;
	    palette = NULL;
	    if (FileNameA)
	      {
		for (int i=sizeY-1; i>=0;i--)
		  {
		    int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		    for (unsigned int j=0; j<sizeX;j++) {
		      if (le16_to_cpu(info1.biBitCount)==24)
			for (int k=2; k>=0;k--) {
			  f1.Read(data+3+4*j+itimes4width,sizeof (unsigned char));
			}
		      else {
				f1.Read(data+3+4*j+itimes4width,sizeof (unsigned char));
		      }
		    }
		  }
	      } else{
		for (unsigned int i=0; i<sizeY;i++) {
		  int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
		  for (unsigned int j=0; j<sizeX;j++){
		    if (!data[4*j+itimes4width]&&!data[4*j+itimes4width+1]&&!data[4*j+itimes4width+2])
		      data[4*j+itimes4width+3]=0;
		    else
		      data[4*j+itimes4width+3]=255;
		  }
		}
		
	      }
	  }
	  
	  if (alpha!=1) {
	    float tmpclamp;
	    for (unsigned int i=0; i<sizeY;i++) {
	      int itimes4width= 4*i*sizeX;//speed speed speed (well if I really wanted speed Pos'd have wrote this function)
	      for (unsigned int j=0; j<sizeX;j++){
		tmpclamp = data[4*j+itimes4width+3];
		if (tmpclamp>zeroval) {
		  tmpclamp /=255.;
		  tmpclamp =pow (tmpclamp,alpha);
		  tmpclamp *=255;
		if (tmpclamp>255)
		  tmpclamp = 255;
		data[4*j+itimes4width+3]= (unsigned char)tmpclamp;
		
		}
	    }
	    }
	    
	  }
	}
	//VSFileSystem::vs_fprintf (stderr,"Bind... ");
	*/
	if (data)
	{
	  Bind(maxdimension,detailtexture);
	  free(data);
	  data = NULL;
	  if (!nocache) setold();
	}
	else
		FileNotFound( texfilename);
	f.Close();
	if (err1<=Ok) {
	  f1.Close();
	}
	//VSFileSystem::vs_fprintf (stderr,"Load Success\n");
}
Texture::~Texture()
	{
		if(original == NULL)
		{
		  /**DEPRECATED
			if(data != NULL)
			{
				delete [] data;

				data = NULL;
			}
		  */
            UnBind();
            if (palette !=NULL) {
              free(palette);
              palette = NULL;
            }
		}
		else
		{
			original->refcount--;
			if(original->refcount == 0)
				delete original;
		}
	}

void Texture::UnBind()
{
    if (name!=-1) {
      texHashTable.Delete (texfilename);
      GFXDeleteTexture(name);
      name = -1;
    }
    //glDeleteTextures(1, &name);
}

void Texture::Transfer (int maxdimension,GFXBOOL detailtexture)
{
	//Implement this in D3D
	//if(mode == _8BIT)
	//	glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, palette);

	switch (mode)
	{
	case _24BITRGBA:
		GFXTransferTexture(data, name,RGBA32,image_target,maxdimension,detailtexture);
		break;
	case _24BIT:
		GFXTransferTexture(data, name,RGB24,image_target,maxdimension,detailtexture);
		break;
	case _8BIT:
		GFXTransferTexture(data, name,PALETTE8, image_target,maxdimension,detailtexture);
		//TODO: Do something about this, and put in some code to check that we can actually do 8 bit textures

		break;
	}
	
}
int Texture::Bind(int maxdimension,GFXBOOL detailtexture)
{
    if (!bound||(boundSizeX!=sizeX)||(boundSizeY!=sizeY)||(boundMode!=mode)) {
        UnBind();

        switch(mode)
	    {
	    case _24BITRGBA:
		    //GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage);
		    GFXCreateTexture(sizeX, sizeY, RGBA32, &name, NULL, stage,ismipmapped, texture_target, address_mode);
		    break;
	    case _24BIT:
		    //not supported by most cards, so i use rgba32
		    //GFXCreateTexture(sizeX, sizeY, RGB24, &name);
		    GFXCreateTexture(sizeX, sizeY, RGB24, &name, NULL, stage,ismipmapped,texture_target, address_mode);
		    break;
	    case _8BIT:
		    GFXCreateTexture(sizeX, sizeY, PALETTE8, &name, (char *)palette, stage,ismipmapped,texture_target, address_mode);
		    break;
	    }
    }
    boundSizeX=sizeX;
    boundSizeY=sizeY;
    boundMode=mode;
    bound=true;

	Transfer(maxdimension,detailtexture);

	return name;

}
void Texture::Prioritize (float priority) {
  GFXPrioritizeTexture (name, priority);
}
static void ActivateWhite(int stage) {
	static Texture * white = new Texture("white.bmp",0,MIPMAP,TEXTURE2D,TEXTURE_2D, 1 );
	if (white->LoadSuccess())
		white->MakeActive(stage);
}
void Texture::MakeActive(int stag, int pass)
{
  static bool missing=false;
  if ((name==-1)||(pass!=0)) {
	ActivateWhite(stag);
  } else {
    GFXActiveTexture(stag);
    GFXSelectTexture(name,stag);
    GFXTextureAddressMode(address_mode,texture_target); //In case it changed - it's possible
  }
}
