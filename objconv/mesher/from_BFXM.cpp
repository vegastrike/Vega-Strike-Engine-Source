#include "mesh_io.h"
#include "from_BFXM.h"

void BFXMToXmesh(FILE* Inputfile, FILE* Outputfile){
  unsigned int intbuf;
  float floatbuf;
  unsigned char bytebuf;
  int word32index=0;
  union chunk32{
	  int i32val;
	  float f32val;
	  unsigned char c8val[4];
  } * inmemfile;
  fseek(Inputfile,4+sizeof(int),SEEK_SET);
  fread(&intbuf,sizeof(int),1,Inputfile);//Length of Inputfile
  int Inputlength=VSSwapHostIntToLittle(intbuf);
  inmemfile=(chunk32*)malloc(Inputlength);
  if(!inmemfile) {fprintf(stderr,"Buffer allocation failed, Aborting"); exit(-1);}
  rewind(Inputfile);
  fread(inmemfile,1,Inputlength,Inputfile);
  fclose(Inputfile);
  int Inputlength32=Inputlength/4;
  //Extract superheader fields
  word32index+=1;
  int version = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  word32index+=2;
  int Superheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
  word32index+=1;
  int NUMFIELDSPERVERTEX = VSSwapHostIntToLittle(inmemfile[word32index].i32val); //Number of fields per vertex:integer (8)
  word32index+=1;
  int NUMFIELDSPERPOLYGONSTRUCTURE = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//  Number of fields per polygon structure: integer (3)
  word32index+=1;
  int NUMFIELDSPERREFERENCEDVERTEX = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of fields per referenced vertex: integer (3)
  word32index+=1;
  int NUMFIELDSPERREFERENCEDANIMATION = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of fields per referenced animation: integer (1)
  word32index+=1;
  int numrecords = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of records: integer
  word32index=(Superheaderlength/4); // Go to first record
  //For each record
  for(int recordindex=0;recordindex<numrecords;recordindex++){
	  int recordbeginword=word32index;
	  //Extract Record Header
	  int recordheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	  word32index+=1;
	  int recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
      word32index+=1;
	  int nummeshes = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of meshes in the current record
	  word32index=recordbeginword+(recordheaderlength/4);
	  //For each mesh
	  for(int meshindex=0;meshindex<nummeshes;meshindex++){
		  if(recordindex>0||meshindex>0){
			string filename="";
			filename+=(char)(recordindex+48);
			filename+="_";
			filename+=(char)(meshindex+48);
			filename+=".xmesh";
			Outputfile=fopen(filename.c_str(),"w+");
		  }
		  //Extract Mesh Header
		  int meshbeginword=word32index;
		  int meshheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	      word32index+=1;
	      int meshlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
		  float scale=VSSwapHostFloatToLittle(inmemfile[meshbeginword+2].f32val);//scale
		  int reverse=VSSwapHostIntToLittle(inmemfile[meshbeginword+3].i32val);//reverse flag
		  int forcetexture=VSSwapHostIntToLittle(inmemfile[meshbeginword+4].i32val);//force texture flag
		  int sharevert=VSSwapHostIntToLittle(inmemfile[meshbeginword+5].i32val);//share vertex flag
		  float polygonoffset=VSSwapHostFloatToLittle(inmemfile[meshbeginword+6].f32val);//polygonoffset
		  int bsrc=VSSwapHostIntToLittle(inmemfile[meshbeginword+7].i32val);//Blendmode source
		  int bdst=VSSwapHostIntToLittle(inmemfile[meshbeginword+8].i32val);//Blendmode destination
		  float	power=VSSwapHostFloatToLittle(inmemfile[meshbeginword+9].f32val);//Specular: power
		  float	ar=VSSwapHostFloatToLittle(inmemfile[meshbeginword+10].f32val);//Ambient: red
		  float	ag=VSSwapHostFloatToLittle(inmemfile[meshbeginword+11].f32val);//Ambient: green
		  float	ab=VSSwapHostFloatToLittle(inmemfile[meshbeginword+12].f32val);//Ambient: blue
		  float	aa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+13].f32val);//Ambient: Alpha
		  float	dr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+14].f32val);//Diffuse: red
		  float	dg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+15].f32val);//Diffuse: green
		  float	db=VSSwapHostFloatToLittle(inmemfile[meshbeginword+16].f32val);//Diffuse: blue
		  float	da=VSSwapHostFloatToLittle(inmemfile[meshbeginword+17].f32val);//Diffuse: Alpha
		  float	er=VSSwapHostFloatToLittle(inmemfile[meshbeginword+18].f32val);//Emmissive: red
		  float	eg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+19].f32val);//Emmissive: green
		  float	eb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+20].f32val);//Emmissive: blue
		  float	ea=VSSwapHostFloatToLittle(inmemfile[meshbeginword+21].f32val);//Emmissive: Alpha
		  float	sr=VSSwapHostFloatToLittle(inmemfile[meshbeginword+22].f32val);//Specular: red
		  float	sg=VSSwapHostFloatToLittle(inmemfile[meshbeginword+23].f32val);//Specular: green
		  float	sb=VSSwapHostFloatToLittle(inmemfile[meshbeginword+24].f32val);//Specular: blue
		  float	sa=VSSwapHostFloatToLittle(inmemfile[meshbeginword+25].f32val);//Specular: Alpha
		  int cullface=VSSwapHostIntToLittle(inmemfile[meshbeginword+26].i32val);//CullFace
		  int lighting=VSSwapHostIntToLittle(inmemfile[meshbeginword+27].i32val);//lighting
		  int reflect=VSSwapHostIntToLittle(inmemfile[meshbeginword+28].i32val);//reflect
		  int usenormals=VSSwapHostIntToLittle(inmemfile[meshbeginword+29].i32val);//usenormals
		  //End Header
		  // Go to Arbitrary Length Attributes section
		  word32index=meshbeginword+(meshheaderlength/4);
		  int VSAbeginword=word32index;
		  int LengthOfArbitraryLengthAttributes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Length of Arbitrary length attributes section in bytes
		  word32index+=1;
		  fprintf(Outputfile,"<Mesh scale=\"%f\" reverse=\"%d\" forcetexture=\"%d\" sharevert=\"%d\" polygonoffset=\"%f\" blendmode=\"%d %d\" ",scale,reverse,forcetexture,sharevert,polygonoffset,bsrc,bdst);
		  
		  string detailtexturename="";
		  int detailtexturenamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//detailtexture name length
		  word32index+=1;
		  int stringindex=0;
		  int namebound=(detailtexturenamelen+3)/4;
		  for(stringindex=0;stringindex<namebound;stringindex++){
			for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			    detailtexturename+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
			  }
			}
			word32index+=1;
		  }
		  if(detailtexturename.size()!=0){fprintf(Outputfile," detailtexture=\"%s\" ",detailtexturename.c_str());}

		  vector <Mesh_vec3f> Detailplanes; //store detail planes until finish printing mesh attributes
		  int numdetailplanes=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of detailplanes
		  word32index+=1;
		  for(int detailplane=0;detailplane<numdetailplanes;detailplane++){
			float x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x-coord
			float y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y-coord
			float z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z-coord
			word32index+=3;
			Mesh_vec3f temp;
			temp.x=x;
			temp.y=y;
			temp.z=z;
			Detailplanes.push_back(temp);
		  } //End detail planes
		  //Textures
		  int numtextures=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of textures
		  word32index+=1;
		  for(int tex=0;tex<numtextures;tex++){
			int textype=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//texture type
			int texindex=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//texture index
			int texnamelen=VSSwapHostIntToLittle(inmemfile[word32index+2].i32val);//texture name length
			word32index+=3;
			string texname="";
			int namebound=(texnamelen+3)/4;
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
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
		  }
		  fprintf(Outputfile,">\n");
		  //End Textures
		  fprintf(Outputfile,"<Material power=\"%f\" cullface=\"%d\" reflect=\"%d\" lighting=\"%d\" usenormals=\"%d\">\n",power,cullface,lighting,reflect,usenormals);
		  fprintf(Outputfile,"\t<Ambient Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",ar,ag,ab,aa);
		  fprintf(Outputfile,"\t<Diffuse Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",dr,dg,db,da);
		  fprintf(Outputfile,"\t<Emissive Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",er,eg,eb,ea);
		  fprintf(Outputfile,"\t<Specular Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",sr,sg,sb,sa);
		  fprintf(Outputfile,"</Material>\n");

		  for(int detplane=0;detplane<Detailplanes.size();detplane++){
			  fprintf(Outputfile,"<DetailPlane x=\"%f\" y=\"%f\" z=\"%f\" />\n",Detailplanes[detplane].x,Detailplanes[detplane].y,Detailplanes[detplane].z);
		  }
		  //Logos
		  int numlogos=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of logos
		  word32index+=1;
		  for(int logo=0;logo<numlogos;logo++){
		    float size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//size
	        float offset=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//offset
	        float rotation=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//rotation
	        int type=VSSwapHostIntToLittle(inmemfile[word32index+3].i32val);//type
	        int numrefs=VSSwapHostIntToLittle(inmemfile[word32index+4].i32val);//number of reference points
			fprintf(Outputfile,"<Logo type=\"%d\" rotate=\"%f\" size=\"%f\" offset=\"%f\">\n",type,rotation,size,offset);
			word32index+=5;
			for(int ref=0;ref<numrefs;ref++){
		      int refnum=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Logo ref
		      float weight=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//reference weight
			  fprintf(Outputfile,"\t<Ref point=\"%d\" weight=\"%f\"/>\n",refnum,weight);
			  word32index+=2;
			}
			fprintf(Outputfile,"</Logo>\n");
		  }
		  //End logos
		  //LODs
		  int numLODs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of LODs
		  word32index+=1;
		  for(int LOD=0;LOD<numLODs;LOD++){
			float size=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//Size
			int index=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//Mesh index
			fprintf(Outputfile,"<LOD size=\"%f\" meshfile=\"%d_%d.xmesh\"/>\n",size,recordindex,index);
			word32index+=2;
		  }
		  //End LODs
		  //AnimationDefinitions
		  int numanimdefs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation definitions
		  word32index+=1;
		  for(int anim=0;anim<numanimdefs;anim++){
			int animnamelen=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of name
			word32index+=1;
			string animname="";
			int namebound=(animnamelen+3)/4;
			for(stringindex=0;stringindex<namebound;stringindex++){
			  for(int bytenum=0;bytenum<4;bytenum++){ // Extract chars
				if(inmemfile[word32index].c8val[bytenum]){ //If not padding
			      animname+=inmemfile[word32index].c8val[bytenum]; //Append char to end of string
				}
			  }
			  word32index+=1;
			}
			float FPS=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//FPS
			fprintf(Outputfile,"<AnimationDefinition AnimationName=\"%s\" FPS=\"%f\">\n",animname.c_str(),FPS);
			int numframerefs=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//number of animation frame references
		    word32index+=2;
			for(int fref=0;fref<numframerefs;fref++){
			  int ref=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		      word32index+=NUMFIELDSPERREFERENCEDANIMATION;
			  fprintf(Outputfile,"<AnimationFrameIndex AnimationMeshIndex=\"%d\"/>\n",ref);
			}
			fprintf(Outputfile,"</AnimationDefinition>\n");
		  }
		  //End AnimationDefinitions
		  //End VSA
		  //go to geometry
		  word32index=VSAbeginword+(LengthOfArbitraryLengthAttributes/4);
		  //Vertices
		  fprintf(Outputfile,"<Points>\n");
		  int numvertices=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int vert=0;vert<numvertices;vert++){
			float x=VSSwapHostFloatToLittle(inmemfile[word32index].f32val);//x
			float y=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//y
			float z=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//z
			float i=VSSwapHostFloatToLittle(inmemfile[word32index+3].f32val);//i
			float j=VSSwapHostFloatToLittle(inmemfile[word32index+4].f32val);//j
			float k=VSSwapHostFloatToLittle(inmemfile[word32index+5].f32val);//k
			float s=VSSwapHostFloatToLittle(inmemfile[word32index+6].f32val);//s
			float t=VSSwapHostFloatToLittle(inmemfile[word32index+7].f32val);//t
		    word32index+=NUMFIELDSPERVERTEX;
			fprintf(Outputfile,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n\t<Normal i=\"%f\" j=\"%f\" k=\"%f\"/>\n</Point>\n",x,y,z,s,t,i,j,k);
		  }
		  fprintf(Outputfile,"</Points>\n");
		  //End Vertices
		  //Lines
		  fprintf(Outputfile,"<Polygons>\n");
		  int numlines=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rvert=0;rvert<numlines;rvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Line flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Line>\n",flatshade,ind1,s1,t1,ind2,s2,t2);
		  }
		  //End Lines
		  //Triangles
		  int numtris=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rtvert=0;rtvert<numtris;rtvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Tri flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Tri>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3);
		  }
		  //End Triangles
		  //Quads
		  int numquads=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int rqvert=0;rqvert<numquads;rqvert++){
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			word32index+=1;
			int ind1=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			float s1=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t1=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		    word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind2=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 2
			float s2=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t2=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind3=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s3=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t3=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			int ind4=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 3
			float s4=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			float t4=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
			word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			fprintf(Outputfile,"\t<Quad flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Quad>\n",flatshade,ind1,s1,t1,ind2,s2,t2,ind3,s3,t3,ind4,s4,t4);
		  }
		  //End Quads
		  //Linestrips
		  int numlinestrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int lstrip=0;lstrip<numlinestrips;lstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Linestrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Linestrip>");
		  }
		  //End Linestrips
		  //Tristrips
		  int numtristrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int tstrip=0;tstrip<numtristrips;tstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Tristrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Tristrip>");
		  }
		  //End Tristrips
		  //Trifans
		  int numtrifans=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int tfan=0;tfan<numtrifans;tfan++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Trifan flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Trifan>");
		  }
		  //End Trifans
		  //Quadstrips
		  int numquadstrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int qstrip=0;qstrip<numquadstrips;qstrip++){
			int numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int flatshade=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//flatshade
			fprintf(Outputfile,"\t<Quadstrip flatshade=\"%d\"/>\n",flatshade);
			word32index+=2;
			for(int elem=0;elem<numstripelements;elem++){
			  int ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Quadstrip>");
		  }
		  //End Quadstrips
		  fprintf(Outputfile,"</Polygons>\n");
		  //End Geometry
		  //go to next mesh
		  fprintf(Outputfile,"</Mesh>\n");
		  word32index=meshbeginword+(meshlength/4);
	  }	
	  //go to next record
	  word32index=recordbeginword+(recordlength/4);
  }
}
