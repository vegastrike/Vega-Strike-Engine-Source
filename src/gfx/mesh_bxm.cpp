#include "mesh_io.h"
#include "mesh_bxm.h"
#include "mesh.h"
#include "mesh_xml.h"

#ifndef STANDALONE
#include "aux_texture.h"
#include "animation.h"
#include "faction_generic.h"
#endif
#include <assert.h>
#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
string inverseblend[16]={"ZERO","ZERO","ONE","SRCCOLOR","INVSRCCOLOR","SRCALPHA","INVSRCALPHA",
"DESTALPHA","INVDESTALPHA","DESTCOLOR","INVDESTCOLOR","SRCALPHASAT","CONSTALPHA","INVCONSTALPHA",
"CONSTCOLOR","INVCONSTCOLOR"};
void fcloseInput (FILE * fp) {
   fclose(fp);
}
int aprintf(...) {
  return 0;
}
FILE * aopen(...) {return NULL;}
#ifndef STANDALONE
#define fprintf aprintf
#define fopen aopen
#define fclose aopen
#else
Texture * LoadTexture(string nam) {
  return new Texture(nam);
}
Texture * LoadAnimation (string Name) {
  return new Animation(Name);
}
#endif
struct OrigMeshLoader{
  Mesh * m;
  vector<float> sizes; 
  unsigned int num;
  OrigMeshLoader(){m=0;num=0;}
};
//#define DLIST

#ifdef DLIST
#define DLISTBEGINSTATE(stat) if(laststate!=stat) {if (laststate!=GL_COMPILE) glEnd();glBegin(stat);laststate=stat;}
#define DLISTENDSTATE(stat) if (laststate!=GL_COMPILE) {glEnd();laststate=GL_COMPILE;}
#define DLISTDOVERTEX(num) glTexCoord2f(vtx.s,vtx.t);glNormal3f(vtx.i,vtx.j,vtx.k);glVertex3f(vtx.x*xml.scale.i,vtx.y*xml.scale.j,vtx.z*xml.scale.k);
#else
#define DLISTBEGINSTATE(stat)
#define DLISTDOVERTEX(num)
#define DLISTENDSTATE(stat)
#endif

//sets up the appropriate lists for the below functions to utilize
#define BEGIN_GL_LINES xml.active_list=&xml.lines; xml.active_ind= &xml.lineind; xml.num_vertices=2;
#define BEGIN_GL_TRIANGLES xml.active_list=&xml.tris; xml.active_ind= &xml.triind; xml.num_vertices=3; xml.trishade.push_back(flatshade);
#define BEGIN_GL_QUADS xml.active_list=&xml.quads; xml.active_ind= &xml.quadind; xml.num_vertices=4; xml.quadshade.push_back(flatshade);
#define BEGIN_GL_TRIANGLE_FAN xml.trifans.push_back(std::vector<GFXVertex>()); xml.active_list=&xml.trifans.back(); xml.tfancnt = xml.trifanind.size(); xml.active_ind=&xml.trifanind;
#define BEGIN_GL_QUAD_STRIP     xml.num_vertices=4; xml.quadstrips.push_back (vector<GFXVertex>()); xml.active_list = &xml.quadstrips.back(); xml.qstrcnt = xml.quadstripind.size(); xml.active_ind = &xml.quadstripind;
#define BEGIN_GL_TRIANGLE_STRIP xml.num_vertices=3; xml.tristrips.push_back(vector<GFXVertex>()); xml.tstrcnt = xml.tristripind.size(); xml.active_ind = &xml.tristripind;
#define BEGIN_GL_LINE_STRIP     xml.num_vertices=2; xml.linestrips.push_back (vector<GFXVertex>()); xml.active_list = &xml.linestrips.back(); xml.lstrcnt = xml.linestripind.size();   xml.active_ind = &xml.linestripind;


#define END_GL_LINES 
#define END_GL_TRIANGLES 
#define END_GL_QUADS 
#define END_GL_TRIANGLE_FAN     {for (unsigned int i=xml.tfancnt+2;i<xml.trifanind.size();i++) { \
      xml.nrmltrifan.push_back (xml.trifanind[xml.tfancnt]); \
      xml.nrmltrifan.push_back (xml.trifanind[i-1]); \
      xml.nrmltrifan.push_back (xml.trifanind[i]); \
    }}
#define END_GL_QUAD_STRIP       {for (unsigned int i=xml.qstrcnt+3;i<xml.quadstripind.size();i+=2) { \
      xml.nrmlquadstrip.push_back (xml.quadstripind[i-3]); \
      xml.nrmlquadstrip.push_back (xml.quadstripind[i-2]); \
      xml.nrmlquadstrip.push_back (xml.quadstripind[i]); \
      xml.nrmlquadstrip.push_back (xml.quadstripind[i-1]); \
    }}
#define END_GL_TRIANGLE_STRIP {for (unsigned int i=xml.tstrcnt+2;i<xml.tristripind.size();i++) { \
      if ((i-xml.tstrcnt)%2) { \
	xml.nrmltristrip.push_back (xml.tristripind[i-2]); \
	xml.nrmltristrip.push_back (xml.tristripind[i-1]); \
	xml.nrmltristrip.push_back (xml.tristripind[i]); \
      } else { \
	xml.nrmltristrip.push_back (xml.tristripind[i-1]); \
	xml.nrmltristrip.push_back (xml.tristripind[i-2]); \
	xml.nrmltristrip.push_back (xml.tristripind[i]); \
      } \
    }}
#define END_GL_LINE_STRIP       {for (unsigned int i=xml.lstrcnt+1;i<xml.linestripind.size();i++) { \
      xml.nrmllinstrip.push_back (xml.linestripind[i-1]); \
      xml.nrmllinstrip.push_back (xml.linestripind[i]); \
    } }

#define END_GL_COMPILE

#define BEGINSTATE(stat) BEGIN_##stat; DLISTBEGINSTATE(stat)
#define ENDSTATE(stat) END_##stat; DLISTENDSTATE(stat)
#define DOVERTEX(num) vtx = xml.vertices[ind##num]; \
   if (!xml.sharevert) { \
     vtx.s=s##num;vtx.t=t##num; \
   }  \
   xml.vertex=vtx;  \
   xml.vertexcount[ind##num]+=1;  \
   if ((!vtx.i)&&(!vtx.j)&&(!vtx.k)&&!xml.recalc_norm)  \
     xml.recalc_norm=true; \
   xml.active_list->push_back(xml.vertex); \
   xml.active_ind->push_back(ind##num);     \
   if (xml.reverse) { \
      unsigned int i; \
      for (i=xml.active_ind->size()-1;i>0;i--) { \
	(*xml.active_ind)[i]=(*xml.active_ind)[i-1]; \
      } \
      (*xml.active_ind)[0]=ind##num; \
      for ( i=xml.active_list->size()-1;i>0;i--) { \
	(*xml.active_list)[i]=(*xml.active_list)[i-1]; \
      } \
      (*xml.active_list)[0]=xml.vertex; \
   } \
    xml.num_vertices--; \
   DLISTDOVERTEX(stat)

#ifdef STANDALONE
void Mesh::BFXMToXmesh(FILE* Inputfile, FILE* Outputfile, vector<Mesh*>&output, Vector overallscale,int fac){
  Flightgroup * fg=0;
#else
vector<Mesh*> Mesh::LoadMeshes(VSFileSystem::VSFile & Inputfile, const Vector & scalex, int faction, class Flightgroup * fg,std::string hash_name){
  Vector overallscale=scalex;
  int fac=faction;
  FILE * Outputfile=0;
  vector<Mesh*> output;
#endif
  vector<OrigMeshLoader> meshes;
  int32bit intbuf;
  int32bit word32index=0;
  Mesh * mesh = 0;
  union chunk32{
	  int32bit i32val;
	  float32bit f32val;
	  char8bit c8val[4];
  } * inmemfile;
#ifdef STANDALONE
  fseek(Inputfile,4+sizeof(int32bit),SEEK_SET);
  fread(&intbuf,sizeof(int32bit),1,Inputfile);//Length of Inputfile
  int32bit Inputlength=VSSwapHostIntToLittle(intbuf);
  inmemfile=(chunk32*)malloc(Inputlength);
  if(!inmemfile) {fprintf(stderr,"Buffer allocation failed, Aborting"); exit(-1);}
  rewind(Inputfile);
  fread(inmemfile,1,Inputlength,Inputfile);
  fcloseInput(Inputfile);
#else
  int32bit Inputlength = Inputfile.Size();
  inmemfile=(chunk32*)malloc(Inputlength);
  if(!inmemfile) {fprintf(stderr,"Buffer allocation failed, Aborting"); exit(-1);}
  Inputfile.Read(inmemfile,Inputlength);
  Inputfile.Close();  
#endif
  int32bit Inputlength32=Inputlength/4;
  //Extract superheader fields
  word32index+=1;
  int32bit version = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  word32index+=2;
  int32bit Superheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  int32bit NUMFIELDSPERVERTEX = VSSwapHostIntToLittle(inmemfile[word32index+1].i32val); //Number of fields per vertex:integer (8)
  int32bit NUMFIELDSPERPOLYGONSTRUCTURE = VSSwapHostIntToLittle(inmemfile[word32index+2].i32val);//  Number of fields per polygon structure: integer (1)
  int32bit NUMFIELDSPERREFERENCEDVERTEX = VSSwapHostIntToLittle(inmemfile[word32index+3].i32val);//Number of fields per referenced vertex: integer (3)
  int32bit NUMFIELDSPERREFERENCEDANIMATION = VSSwapHostIntToLittle(inmemfile[word32index+4].i32val);//Number of fields per referenced animation: integer (1)
  int32bit numrecords = VSSwapHostIntToLittle(inmemfile[word32index+5].i32val);//Number of records: integer
  int32bit NUMFIELDSPERANIMATIONDEF = VSSwapHostIntToLittle(inmemfile[word32index+6].i32val);//Number of fields per animationdef: integer (1)
  word32index=(Superheaderlength/4); // Go to first record
  //For each record
  for(int32bit recordindex=0;recordindex<numrecords;recordindex++){
	  int32bit recordbeginword=word32index;
	  //Extract Record Header
	  int32bit recordheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	  word32index+=1;
	  int32bit recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
      word32index+=1;
	  int32bit nummeshes = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of meshes in the current record
	  word32index=recordbeginword+(recordheaderlength/4);
          meshes.push_back(OrigMeshLoader());
          meshes.back().num=nummeshes;
          meshes.back().m = new Mesh[nummeshes];
          meshes.back().sizes.insert(meshes.back().sizes.begin(),nummeshes,0);
	  //For each mesh
	  for(int32bit meshindex=0;meshindex<nummeshes;meshindex++){
		  Mesh * mesh = &meshes.back().m[meshindex];
                  mesh->draw_queue = new vector<MeshDrawContext>;
                  MeshXML xml;
                  xml.fg = fg;
                  xml.faction=fac;
		  if(recordindex>0||meshindex>0){
			char filenamebuf[56]; // Is more than enough characters - int can't be this big in decimal
			int32bit error=sprintf(filenamebuf,"%d_%d.xmesh",recordindex,meshindex);
			if(error==-1){ // if wasn't enough characters - something is horribly wrong.
				exit(error);
			}
			string filename=string(filenamebuf);
			Outputfile=fopen(filename.c_str(),"w+");
		  }
		  //Extract Mesh Header
		  int32bit meshbeginword=word32index;
		  int32bit meshheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	      word32index+=1;
	      int32bit meshlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
		  float32bit scale=VSSwapHostFloatToLittle(inmemfile[meshbeginword+2].f32val);//scale
		  int32bit reverse=VSSwapHostIntToLittle(inmemfile[meshbeginword+3].i32val);//reverse flag
		  int32bit forcetexture=VSSwapHostIntToLittle(inmemfile[meshbeginword+4].i32val);//force texture flag
		  int32bit sharevert=VSSwapHostIntToLittle(inmemfile[meshbeginword+5].i32val);//share vertex flag
		  float32bit polygonoffset=VSSwapHostFloatToLittle(inmemfile[meshbeginword+6].f32val);//polygonoffset
		  int32bit bsrc=VSSwapHostIntToLittle(inmemfile[meshbeginword+7].i32val);//Blendmode source
		  int32bit bdst=VSSwapHostIntToLittle(inmemfile[meshbeginword+8].i32val);//Blendmode destination
		  float32bit	power=VSSwapHostFloatToLittle(inmemfile[meshbeginword+9].f32val);//Specular: power
		  float32bit	ar=VSSwapHostFloatToLittle(inmemfile[meshbeginword+10].f32val);//Ambient: red
		  float32bit	ag=VSSwapHostFloatToLittle(inmemfile[meshbeginword+11].f32val);//Ambient: green
		  float32bit	ab=VSSwapHostFloatToLittle(inmemfile[meshbeginword+12].f32val);//Ambient: blue
		  float32bit	aa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+13].f32val);//Ambient: Alpha
		  float32bit	dr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+14].f32val);//Diffuse: red
		  float32bit	dg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+15].f32val);//Diffuse: green
		  float32bit	db=VSSwapHostFloatToLittle(inmemfile[meshbeginword+16].f32val);//Diffuse: blue
		  float32bit	da=VSSwapHostFloatToLittle(inmemfile[meshbeginword+17].f32val);//Diffuse: Alpha
		  float32bit	er=VSSwapHostFloatToLittle(inmemfile[meshbeginword+18].f32val);//Emmissive: red
		  float32bit	eg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+19].f32val);//Emmissive: green
		  float32bit	eb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+20].f32val);//Emmissive: blue
		  float32bit	ea=VSSwapHostFloatToLittle(inmemfile[meshbeginword+21].f32val);//Emmissive: Alpha
		  float32bit	sr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+22].f32val);//Specular: red
		  float32bit	sg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+23].f32val);//Specular: green
		  float32bit	sb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+24].f32val);//Specular: blue
		  float32bit	sa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+25].f32val);//Specular: Alpha
		  int32bit cullface=VSSwapHostIntToLittle(inmemfile[meshbeginword+26].i32val);//CullFace
		  int32bit lighting=VSSwapHostIntToLittle(inmemfile[meshbeginword+27].i32val);//lighting
		  int32bit reflect=VSSwapHostIntToLittle(inmemfile[meshbeginword+28].i32val);//reflect
		  int32bit usenormals=VSSwapHostIntToLittle(inmemfile[meshbeginword+29].i32val);//usenormals
		  //End Header
		  // Go to Arbitrary Length Attributes section
		  word32index=meshbeginword+(meshheaderlength/4);
		  int32bit VSAbeginword=word32index;
		  int32bit LengthOfArbitraryLengthAttributes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Length of Arbitrary length attributes section in bytes
		  word32index+=1;
		  fprintf(Outputfile,"<Mesh scale=\"%f\" reverse=\"%d\" forcetexture=\"%d\" sharevert=\"%d\" polygonoffset=\"%f\" blendmode=\"%s %s\" ",scale,reverse,forcetexture,sharevert,polygonoffset,inverseblend[bsrc%16].c_str(),inverseblend[bdst%16].c_str());
		  xml.scale=scale*overallscale;
                  xml.lodscale=overallscale;
                  xml.reverse=reverse;
                  xml.force_texture=forcetexture;
                  xml.sharevert=sharevert;
                  mesh->polygon_offset=polygonoffset;
                  mesh->SetBlendMode((BLENDFUNC)bsrc,(BLENDFUNC)bdst);

		  string detailtexturename="";
		  int32bit detailtexturenamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//detailtexture name length
		  word32index+=1;
		  int32bit stringindex=0;
		  int32bit namebound=(detailtexturenamelen+3)/4;
		  for(stringindex=0;stringindex<namebound;stringindex++){
			for(int32bit bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			    detailtexturename+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
			  }
			}
			word32index+=1;
		  }
		  if(detailtexturename.size()!=0){
                    fprintf(Outputfile," detailtexture=\"%s\" ",detailtexturename.c_str());
                    mesh->detailTexture=mesh->TempGetTexture(&xml,detailtexturename,FactionUtil::GetFaction(xml.faction),GFXTRUE);//LoadTexture(detailtexturename);                    
                  }else {
                    mesh->detailTexture=0;
                  }

		  vector <Mesh_vec3f> Detailplanes; //store detail planes until finish printing mesh attributes
		  int32bit numdetailplanes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of detailplanes
		  word32index+=1;
		  for(int32bit detailplane=0;detailplane<numdetailplanes;detailplane++){
			float32bit x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x-coord
			float32bit y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y-coord
			float32bit z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z-coord
			word32index+=3;
			Mesh_vec3f temp;
			temp.x=x;
			temp.y=y;
			temp.z=z;
			Detailplanes.push_back(temp);
		  } //End detail planes
		  //Textures
		  int32bit numtextures=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of textures
		  word32index+=1;
		  for(int32bit tex=0;tex<numtextures;tex++){
			int32bit textype=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//texture type
			int32bit texindex=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//texture index
			int32bit texnamelen=VSSwapHostIntToLittle(inmemfile[word32index+2].i32val);//texture name length
			word32index+=3;
			string texname="";
			int32bit namebound=(texnamelen+3)/4;
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int32bit bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			      texname+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
				}
			  }
			  word32index+=1;
			}
			switch(textype){
			case ALPHAMAP:
				fprintf(Outputfile," alphamap");                                
				break;
			case ANIMATION:
				fprintf(Outputfile," animation");
				break;
			case TEXTURE:
				fprintf(Outputfile," texture");
				break;
			}
			if(texindex){
				fprintf(Outputfile,"%d",texindex);
			}
			fprintf(Outputfile,"=\"%s\" ",texname.c_str());
                        while (mesh->Decal.size()<=texindex){
                          mesh->Decal.push_back (0);
                        }
                        while (xml.decals.size()<=texindex){
                          MeshXML::ZeTexture z;
                          xml.decals.push_back(z);
                        }
                        switch(textype) {
                        case ALPHAMAP:
                          xml.decals[texindex].alpha_name=texname;
                          break;
                        case TEXTURE:
                          //mesh->Decal[texindex]=LoadTexture (texname);
                          xml.decals[texindex].decal_name=texname;
                          break;
                        case ANIMATION:
                          //mesh->Decal[texindex]=LoadAnimation(texname);
                          xml.decals[texindex].animated_name=texname;
                          break;
                        }
		  }
		  fprintf(Outputfile,">\n");
		  //End Textures
		  fprintf(Outputfile,"<Material power=\"%f\" cullface=\"%d\" reflect=\"%d\" lighting=\"%d\" usenormals=\"%d\">\n",power,cullface,lighting,reflect,usenormals);
		  fprintf(Outputfile,"\t<Ambient Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",ar,ag,ab,aa);
		  fprintf(Outputfile,"\t<Diffuse Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",dr,dg,db,da);
		  fprintf(Outputfile,"\t<Emissive Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",er,eg,eb,ea);
		  fprintf(Outputfile,"\t<Specular Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",sr,sg,sb,sa);
		  fprintf(Outputfile,"</Material>\n");
                  mesh->setEnvMap(reflect);
                  mesh->forceCullFace(cullface);
                  mesh->setLighting(lighting);
                  xml.usenormals=usenormals;
                  xml.material.ar=ar;xml.material.ag=ag;xml.material.ab=ab;xml.material.aa=aa;
                  xml.material.dr=dr;xml.material.dg=dg;xml.material.db=db;xml.material.da=da;
                  xml.material.er=er;xml.material.eg=eg;xml.material.eb=eb;xml.material.ea=ea;
                  xml.material.sr=sr;xml.material.sg=sg;xml.material.sb=sb;xml.material.sa=sa;
                  xml.material.power=power;
#ifdef STANDALONE
                  mesh->myMatNum=xml.material;
#endif

		  for(int32bit detplane=0;detplane<Detailplanes.size();detplane++){
			  fprintf(Outputfile,"<DetailPlane x=\"%f\" y=\"%f\" z=\"%f\" />\n",Detailplanes[detplane].x,Detailplanes[detplane].y,Detailplanes[detplane].z);
                          mesh->detailPlanes.push_back(Vector(Detailplanes[detplane].x,
                                                              Detailplanes[detplane].y,
                                                              Detailplanes[detplane].z));
		  }
		  //Logos
		  int32bit numlogos=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of logos
		  word32index+=1;
		  for(int32bit logo=0;logo<numlogos;logo++){
		    float32bit size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//size
	        float32bit offset=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//offset
	        float32bit rotation=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//rotation
	        int32bit type=VSSwapHostIntToLittle(inmemfile[word32index+3].i32val);//type
	        int32bit numrefs=VSSwapHostIntToLittle(inmemfile[word32index+4].i32val);//number of reference points
			fprintf(Outputfile,"<Logo type=\"%d\" rotate=\"%f\" size=\"%f\" offset=\"%f\">\n",type,rotation,size,offset);
                        struct MeshXML::ZeLogo l;
                        xml.logos.push_back(l);
                        xml.logos.back().type=type;
                        xml.logos.back().rotate=rotation;
                        xml.logos.back().size=size;
                        xml.logos.back().offset=offset;

			word32index+=5;
			for(int32bit ref=0;ref<numrefs;ref++){
		      int32bit refnum=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Logo ref
		      float32bit weight=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//reference weight
			  fprintf(Outputfile,"\t<Ref point=\"%d\" weight=\"%f\"/>\n",refnum,weight);
                          xml.logos.back().refpnt.push_back(refnum);
                          xml.logos.back().refweight.push_back(weight);
			  word32index+=2;
			}
			fprintf(Outputfile,"</Logo>\n");
                        
		  }
		  //End logos
		  //LODs
		  int32bit numLODs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of LODs
		  word32index+=1;
		  for(int32bit LOD=0;LOD<numLODs;LOD++){
			float32bit size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//Size
			int32bit index=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//Mesh index
			fprintf(Outputfile,"<LOD size=\"%f\" meshfile=\"%d_%d.xmesh\"/>\n",size,recordindex,index);
                        meshes.back().sizes[LOD]=size;
			word32index+=2;
		  }
		  //End LODs
		  //AnimationDefinitions
		  int32bit numanimdefs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation definitions
		  word32index+=1;
		  if(meshindex==0){
			  for(int framecount=numLODs+1;framecount<nummeshes;framecount++){
				  fprintf(Outputfile,"<Frame FrameMeshName=\"%d_%d.xmesh\"/>\n",recordindex,framecount);
			  }
		  }
		  for(int32bit anim=0;anim<numanimdefs;anim++){
			int32bit animnamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of name
			word32index+=1;
			string animname="";
			int32bit namebound=(animnamelen+3)/4;
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int32bit bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			      animname+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
				}
			  }
			  word32index+=1;
			}
			float32bit FPS=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//FPS
			fprintf(Outputfile,"<AnimationDefinition AnimationName=\"%s\" FPS=\"%f\">\n",animname.c_str(),FPS);

                        vector <int> *framerefs = new vector<int>;
                        mesh->framespersecond=FPS;
			word32index+=NUMFIELDSPERANIMATIONDEF;
			int32bit numframerefs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		    word32index+=1;
			for(int32bit fref=0;fref<numframerefs;fref++){
			  int32bit ref=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		      word32index+=NUMFIELDSPERREFERENCEDANIMATION;
			  fprintf(Outputfile,"<AnimationFrameIndex AnimationMeshIndex=\"%d\"/>\n",ref-1-numLODs);
                          framerefs->push_back(ref);
			}
                        animationSequences.Put(hash_name+animname,framerefs);
			fprintf(Outputfile,"</AnimationDefinition>\n");
		  }
		  //End AnimationDefinitions
		  //End VSA
		  //go to geometry
		  word32index=VSAbeginword+(LengthOfArbitraryLengthAttributes/4);
		  //Vertices
		  fprintf(Outputfile,"<Points>\n");
		  int32bit numvertices=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit vert=0;vert<numvertices;vert++){
			float32bit x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x
			float32bit y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y
			float32bit z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z
			float32bit i=VSSwapHostFloatToLittle(inmemfile[word32index+3].f32val);//i
			float32bit j=VSSwapHostFloatToLittle(inmemfile[word32index+4].f32val);//j
			float32bit k=VSSwapHostFloatToLittle(inmemfile[word32index+5].f32val);//k
                        if (i==0&&j==0&&k==0) {
                          i=x;j=y;k=z;
                          float ms=i*i+j*j+k*k;
                          if (ms>.000001) {
                            float m = sqrt(ms);
                            i/=m;j/=m;k/=m;
                          }else {
                            i=0;j=0;k=1;
                          }
                        }
			float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+6].f32val);//s
			float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+7].f32val);//t
		    word32index+=NUMFIELDSPERVERTEX;
			fprintf(Outputfile,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n\t<Normal i=\"%f\" j=\"%f\" k=\"%f\"/>\n</Point>\n",x,y,z,s,t,i,j,k);
                        xml.vertices.push_back(GFXVertex(Vector(x,y,z),Vector(i,j,k),s,t));
                        // NOTE: postprocessing takes care of scale |-
                        xml.vertexcount.push_back(0);
		  }
		  fprintf(Outputfile,"</Points>\n");
		  //End Vertices
		  //Lines
                  GFXVertex vtx;
#ifdef DLIST
                  static GLenum laststate=GL_COMPILE;
                  mesh->vlist=glGenLists(1);
                  glNewList(mesh->vlist,GL_COMPILE);
#endif
		  fprintf(Outputfile,"<Polygons>\n");
		  int32bit numlines=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit rvert=0;rvert<numlines;rvert++){
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=NUMFIELDSPERPOLYGONSTRUCTURE;
			int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float32bit s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Line flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Line>\n",flatshade,ind1,s1,t1,ind2,s2,t2);
                        BEGINSTATE(GL_LINES);
                        DOVERTEX(1);
                        DOVERTEX(2);

                        
		  }
		  //End Lines
		  //Triangles
		  int32bit numtris=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit rtvert=0;rtvert<numtris;rtvert++){
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=NUMFIELDSPERPOLYGONSTRUCTURE;
			int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float32bit s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float32bit s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Tri flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Tri>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3);
                        BEGINSTATE(GL_TRIANGLES);
                        DOVERTEX(1);
                        DOVERTEX(2);
                        DOVERTEX(3);

		  }
		  //End Triangles
		  //Quads
		  int32bit numquads=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit rqvert=0;rqvert<numquads;rqvert++){
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=NUMFIELDSPERPOLYGONSTRUCTURE;
			int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float32bit s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float32bit s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int32bit ind4=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float32bit s4=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float32bit t4=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Quad flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Quad>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3,ind4,s4,t4);
                        BEGINSTATE(GL_QUADS);
                        DOVERTEX(1);
                        DOVERTEX(2);
                        DOVERTEX(3);
                        DOVERTEX(4);

		  }
		  //End Quads
		  //Linestrips
		  int32bit numlinestrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit lstrip=0;lstrip<numlinestrips;lstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Linestrip flatshade=\"%d\">\n",flatshade);
                        BEGINSTATE(GL_LINE_STRIP);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind1,s1,t1);
                          DOVERTEX(1)
			}
			fprintf(Outputfile,"\t</Linestrip>");
                        ENDSTATE(GL_LINE_STRIP);
		  }
		  //End Linestrips
		  //Tristrips
		  int32bit numtristrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit tstrip=0;tstrip<numtristrips;tstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Tristrip flatshade=\"%d\">\n",flatshade);
                        BEGINSTATE(GL_TRIANGLE_STRIP);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind1,s1,t1);
                          DOVERTEX(1)
			}
			fprintf(Outputfile,"\t</Tristrip>");
                        ENDSTATE(GL_TRIANGLE_STRIP);
		  }
		  //End Tristrips
		  //Trifans
		  int32bit numtrifans=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit tfan=0;tfan<numtrifans;tfan++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Trifan flatshade=\"%d\">\n",flatshade);
                        BEGINSTATE(GL_TRIANGLE_FAN);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind1,s1,t1);
                          DOVERTEX(1)
			}
			fprintf(Outputfile,"\t</Trifan>");
                        ENDSTATE(GL_TRIANGLE_FAN);
		  }
		  //End Trifans
		  //Quadstrips
		  int32bit numquadstrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit qstrip=0;qstrip<numquadstrips;qstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Quadstrip flatshade=\"%d\">\n",flatshade);
                        BEGINSTATE(GL_QUAD_STRIP);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind1,s1,t1);
                          DOVERTEX(1)
			}
			fprintf(Outputfile,"\t</Quadstrip>");
                        ENDSTATE(GL_QUAD_STRIP);
		  }
		  //End Quadstrips
                  ENDSTATE(GL_COMPILE);
#ifdef DLIST
                  glEndList();
#endif
		  fprintf(Outputfile,"</Polygons>\n");
		  //End Geometry
		  //go to next mesh
		  fprintf(Outputfile,"</Mesh>\n");
                  mesh->PostProcessLoading(&xml);
		  word32index=meshbeginword+(meshlength/4);
	  }
	  //go to next record
	  word32index=recordbeginword+(recordlength/4);
          output.push_back(new Mesh());
          *output.back()=*meshes.back().m;//use builtin
          output.back()->orig=meshes.back().m;
          for (unsigned int i=1;i<meshes.back().sizes.size();++i) {
            output.back()->orig[i].lodsize=meshes.back().sizes[i];
          }
          output.back()->numlods=output.back()->orig->numlods = meshes.back().num;
          
  }
#ifndef STANDALONE
  return output;
#endif
}
