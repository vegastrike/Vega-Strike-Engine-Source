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
#include "gfx/vec.h"
#include "vertex.h"
#include "aux_logo.h"
#include "aux_texture.h"
#include <assert.h>
#include "gfxlib.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "xml_support.h"
#if 0
static float scl=0;
static float offs=1;

void incscl (int, KBSTATE k) {
  if (k==DOWN) {
    VSFileSystem::Fprintf (stderr,"scl:%f",scl);
    scl+=.1;
  }

}

void decscl (int, KBSTATE k) {
  if (k==DOWN) {
    VSFileSystem::Fprintf (stderr,"scl:%f",scl);
    scl-=.1;
  }


}
void incoff (int, KBSTATE k) {
  if (k==DOWN) {
    VSFileSystem::Fprintf (stderr,"off:%f",offs);
    offs+=.1;
  }


}

void decoff (int, KBSTATE k) {
  if (k==DOWN) {
    VSFileSystem::Fprintf (stderr,"off:%f",offs);
    offs-=.1;
  }
}
#endif
vector<Logo*> undrawn_logos;
Hashtable<int, Logo,char[257]> Logo::decalHash;

Logo::Logo(int numberlogos,  Vector* center,Vector* normal, float* size, float* rotation, float offset,Texture* Dec, Vector * Ref)
{
  offset=0;
  refcount = -1;
	draw_queue = NULL;

	numlogos = numberlogos;
	GFXVertex *vertices = new GFXVertex[numlogos*4];
	GFXVertex *LogoCorner = vertices;
	//LogoCorner = new glVertex* [numlogos]; //hope to hell we have enough mem
	Vector p,q,r , v1,v2,v3,v4; //temps
	will_be_drawn = false;
	for (int i=0; i< numlogos;i++, LogoCorner+=4)
	{
		r = normal[i];
		Normalize(r);
		Vector translation = r * offset;
		
		if (Ref[i].i||Ref[i].j||Ref[i].k)
		{
			p = Ref[i];
			Normalize (p);
			ScaledCrossProduct (r,p,q);
			::Roll(rotation[i], p,q,r);
			float tsize = size[i]*0.50F;
			Vector tcenter = center[i] + translation;
			v4 = p * -tsize-q * tsize + tcenter;
			v3 = p * -tsize+q * tsize + tcenter;
			v2 =p*tsize+ q * tsize + tcenter;
			v1 = p * tsize-q*tsize + tcenter;
		}
		else
		{///backwards compatibility shit
			Vector y;
			if ((r.i ==1||r.i==-1)&&!r.j&&!r.k)
				y = Vector (0, 1, 0);
			else
				y= Vector (1, 0, 0);
			ScaledCrossProduct (r,y,p);
			ScaledCrossProduct (r,p,q);
			::Roll(rotation[i], p,q,r);
			float tsize = size[i]*0.50F;
			Vector tcenter = center[i] + translation;
			v1 = q * tsize + tcenter;
			v2 = p * -tsize + tcenter;
			v3 = q * -tsize + tcenter;
			v4 = p * tsize + tcenter;
		}

		LogoCorner[0].SetVertex(v1).SetNormal(r).SetTexCoord(0,0);
		LogoCorner[1].SetVertex(v2).SetNormal(r).SetTexCoord(0,1);
		LogoCorner[2].SetVertex(v3).SetNormal(r).SetTexCoord(1,1);
		LogoCorner[3].SetVertex(v4).SetNormal(r).SetTexCoord(1,0);
		//LogoCorner[4] = LogoCorner[2];
		//LogoCorner[5] = LogoCorner[1];
	}
	vlist = new GFXVertexList(GFXQUAD,4*numlogos, vertices,4*numlogos);
	delete [] vertices;
	SetDecal(Dec);
}

void Logo::SetDecal(Texture *decal)
{
  Decal = decal;
  //Check which draw_queue to use:
  Logo *l;
  if((l=decalHash.Get(decal->name))!=NULL) {
    draw_queue = l->draw_queue;
    owner_of_draw_queue = l;
    l->refcount++;
  } else {
    l = new Logo (*this);//(Logo*)malloc (sizeof (Logo));
    //    memcpy (l,this,sizeof(Logo));
    decalHash.Put(decal->name, l);
    draw_queue = l->draw_queue = new vector<DrawContext>();
    owner_of_draw_queue = l->owner_of_draw_queue = l;
    l->refcount = 1;
  }
}

/*Logo::Logo(int numberlogos,  Vector* center,Vector* normal, float* size, float* rotation, float* offset,char *tex, char *alp)
{
	Decal = NULL;
	Decal = new Texture (tex,alp);
	if (Decal)
	{
		if (!Decal->Data)
		{
			delete Decal;
			Decal = new Texture (tex,NULL);
		}
	}



}*/




void Logo::Draw(const Matrix &m)
{
	if (!numlogos)
		return;
	//	Matrix m;
	//	GFXGetMatrix(MODEL, m);
	draw_queue->push_back(DrawContext(m, vlist));
	if(!owner_of_draw_queue->will_be_drawn) {
	  undrawn_logos.push_back(owner_of_draw_queue);
	  owner_of_draw_queue->will_be_drawn=true;
	}
}

void Logo::ProcessDrawQueue() {
  if (!g_game.use_logos) {
    return;
  }
  static float offs = XMLSupport::parse_float (vs_config->getVariable ("graphics","LogoOffset","-4")); 
  static float scl = XMLSupport::parse_float(vs_config->getVariable ("graphics","LogoOffsetScale","-30.8"));

	GFXEnable(TEXTURE0);
	GFXEnable(TEXTURE1);
	Decal->MakeActive();
	GFXSelectTexcoordSet(0, 0);
	GFXSelectTexcoordSet(1, 1);
	GFXDisable (DEPTHWRITE);
	GFXDisable (LIGHTING);
	GFXColor4f (1,1,1,1);
	GFXPolygonOffset (offs,scl);
	GFXBlendMode(SRCALPHA,INVSRCALPHA);
	//GFXBlendMode(ONE,ZERO);
        DrawContext c = draw_queue->back();
	c.vlist->LoadDrawState();
	while(draw_queue->size()) {
	  c = draw_queue->back();
	  draw_queue->pop_back();
	  GFXLoadMatrixModel ( c.m);
	  c.vlist->BeginDrawState(GFXFALSE);
	  c.vlist->Draw();
	  c.vlist->EndDrawState(GFXFALSE);
	}
	GFXEnable (DEPTHWRITE);
	GFXPolygonOffset (0,0);
}

Logo::~Logo ()
{
  if (owner_of_draw_queue!=this)
    delete vlist;
	
	//if(LogoCorner!=NULL)
	//	delete [] LogoCorner;
	if(owner_of_draw_queue!=NULL) {
	  if (owner_of_draw_queue!=this)
	    owner_of_draw_queue->refcount--;
	  if(owner_of_draw_queue->refcount==0 && owner_of_draw_queue!=this) 
	    delete owner_of_draw_queue;
	}
	if(owner_of_draw_queue == this) {
	  assert(refcount == 0);
	  decalHash.Delete(Decal->name);
	  delete draw_queue;
	}
}

