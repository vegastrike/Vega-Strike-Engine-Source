#include "mesh_io.h"
#include "from_BFXM.h"

string inverseblend[16]={"ZERO","ZERO","ONE","SRCCOLOR","INVSRCCOLOR","SRCALPHA","INVSRCALPHA",
"DESTALPHA","INVDESTALPHA","DESTCOLOR","INVDESTCOLOR","SRCALPHASAT","CONSTALPHA","INVCONSTALPHA",
"CONSTCOLOR","INVCONSTCOLOR"};

void BFXMToXmesh(FILE* Inputfile, FILE* Outputfile, FILE * OutputObj, FILE * mtl,std::string meshname){
  int32bit intbuf;
  char8bit bytebuf;
  int32bit word32index=0;
  union chunk32{
	  int32bit i32val;
	  float32bit f32val;
	  char8bit c8val[4];
  } * inmemfile;
  //CHECK MAGIC WORD
  fseek(Inputfile,0,SEEK_SET);
  fread(&bytebuf,sizeof(char8bit),1,Inputfile);
  if(bytebuf!='B'){
	  fprintf(stderr,"INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected B got %c",bytebuf);
	  exit(-1);
  }
  fseek(Inputfile,1,SEEK_SET);
  fread(&bytebuf,sizeof(char8bit),1,Inputfile);
  if(bytebuf!='F'){
	  fprintf(stderr,"INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected F got %c",bytebuf);
	  exit(-1);
  }
  fseek(Inputfile,2,SEEK_SET);
  fread(&bytebuf,sizeof(char8bit),1,Inputfile);
  if(bytebuf!='X'){
	  fprintf(stderr,"INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected X got %c",bytebuf);
	  exit(-1);
  }
  fseek(Inputfile,3,SEEK_SET);
  fread(&bytebuf,sizeof(char8bit),1,Inputfile);
  if(bytebuf!='M'){
	  fprintf(stderr,"INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected M got %c",bytebuf);
	  exit(-1);
  }

  fseek(Inputfile,4+sizeof(int32bit),SEEK_SET);
  fread(&intbuf,sizeof(int32bit),1,Inputfile);//Length of Inputfile
  int32bit Inputlength=VSSwapHostIntToLittle(intbuf);
  inmemfile=(chunk32*)malloc(Inputlength);
  if(!inmemfile) {fprintf(stderr,"Buffer allocation failed, Aborting"); exit(-1);}
  rewind(Inputfile);
  fread(inmemfile,1,Inputlength,Inputfile);
  fclose(Inputfile);
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
  fprintf(OutputObj,"mtllib %s.mtl\n",meshname.c_str());
  int vtxcount=1;
  int texcount=1;
  int normcount=1;
  int indoffset=1;
  int texoffset=1;
  int normoffset=1;
  for(int32bit recordindex=0;recordindex<numrecords;recordindex++){
	  int32bit recordbeginword=word32index;
	  //Extract Record Header
	  int32bit recordheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record header in bytes
	  word32index+=1;
	  int32bit recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//length of record in bytes
      word32index+=1;
	  int32bit nummeshes = VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Number of meshes in the current record
	  word32index=recordbeginword+(recordheaderlength/4);
	  //For each mesh
	  for(int32bit meshindex=0;meshindex<nummeshes;meshindex++){
            indoffset=vtxcount;
            texoffset=texcount;
            normoffset=normcount;
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
                  
                  fprintf(mtl,"newmtl tex%d_%d\n",recordindex,meshindex);
                  fprintf(OutputObj,"usemtl tex%d_%d\n",recordindex,meshindex);

 		  fprintf(Outputfile,"<Mesh scale=\"%f\" reverse=\"%d\" forcetexture=\"%d\" sharevert=\"%d\" polygonoffset=\"%f\" blendmode=\"%s %s\" ",scale,reverse,forcetexture,sharevert,polygonoffset,inverseblend[bsrc%16].c_str(),inverseblend[bdst%16].c_str());
		  
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
		  if(detailtexturename.size()!=0){fprintf(Outputfile," detailtexture=\"%s\" ",detailtexturename.c_str()); fprintf (mtl,"map_detail %s\n",detailtexturename.c_str());}


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
                        fprintf (mtl,"detail_plane %f %f %f\n",x,y,z);
		  } //End detail planes
		  //Textures
		  int32bit numtextures=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of textures
		  word32index+=1;
                  bool emit=false;
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
                        string textyp;
                        switch (texindex){
                        case 2:
                          textyp="Ka";
                          emit=true;
                          break;
                        case 1:
                          textyp="Ks";
                          break;
                        default:
                          if (textype==ALPHAMAP)
                            textyp="opacity";
                          else
                            textyp="Kd";
                        }                        
                        fprintf (mtl,"map_%s %s\n",textyp.c_str(),texname.c_str());
                        
		  }
		  fprintf(Outputfile,">\n");

		  //End Textures
		  fprintf(mtl,"Ns %f\n",power);
		  fprintf(mtl,"Ka %f %f %f\n",emit?1.0f:ar,emit?1.0f:ag,emit?1.0f:ab);
		  fprintf(mtl,"Kd %f %f %f\n",dr,dg,db);
		  fprintf(mtl,"Ke %f %f %f\n",er,eg,eb);
		  fprintf(mtl,"Ka %f %f %f\n",sr,sg,sb);

		  fprintf(Outputfile,"<Material power=\"%f\" cullface=\"%d\" reflect=\"%d\" lighting=\"%d\" usenormals=\"%d\">\n",power,cullface,lighting,reflect,usenormals);
		  fprintf(Outputfile,"\t<Ambient Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",ar,ag,ab,aa);
		  fprintf(Outputfile,"\t<Diffuse Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",dr,dg,db,da);
		  fprintf(Outputfile,"\t<Emissive Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",er,eg,eb,ea);
		  fprintf(Outputfile,"\t<Specular Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n",sr,sg,sb,sa);
		  fprintf(Outputfile,"</Material>\n");

		  for(int32bit detplane=0;detplane<Detailplanes.size();detplane++){
			  fprintf(Outputfile,"<DetailPlane x=\"%f\" y=\"%f\" z=\"%f\" />\n",Detailplanes[detplane].x,Detailplanes[detplane].y,Detailplanes[detplane].z);
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
			word32index+=5;
			for(int32bit ref=0;ref<numrefs;ref++){
		      int32bit refnum=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//Logo ref
		      float32bit weight=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//reference weight
			  fprintf(Outputfile,"\t<Ref point=\"%d\" weight=\"%f\"/>\n",refnum,weight);
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
			word32index+=NUMFIELDSPERANIMATIONDEF;
			int32bit numframerefs=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		    word32index+=1;
			for(int32bit fref=0;fref<numframerefs;fref++){
			  int32bit ref=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of animation frame references
		      word32index+=NUMFIELDSPERREFERENCEDANIMATION;
			  fprintf(Outputfile,"<AnimationFrameIndex AnimationMeshIndex=\"%d\"/>\n",ref-1-numLODs);
			}
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
			float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+6].f32val);//s
			float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+7].f32val);//t
		    word32index+=NUMFIELDSPERVERTEX;
			fprintf(Outputfile,"<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n\t<Normal i=\"%f\" j=\"%f\" k=\"%f\"/>\n</Point>\n",x,y,z,s,t,i,j,k);
                        fprintf (OutputObj,"v %f %f %f\nvn %f %f %f\n",scale*x,scale*y,scale*z,i,j,k);
                        if (sharevert) {
                          fprintf (OutputObj,"vt %f %f\n",s,t);                          
                          texcount+=1;
                        }
                        vtxcount+=1;
                        normcount+=1;
		  }
		  fprintf(Outputfile,"</Points>\n");
		  //End Vertices
		  //Lines
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
                        int texind1 = ind1+texoffset;
                        int texind2 = ind2+texoffset;
                        if (!sharevert) {
                          fprintf (OutputObj,"vt %f %f\n",s1,t1);
                          fprintf (OutputObj,"vt %f %f\n",s2,t2);
                          texcount+=2;
                          texind1=texcount-2;
                          texind2=texcount-1;
                        }
                        fprintf (OutputObj,"f %d/%d/%d %d/%d/%d\n",
                                 ind1+indoffset,texind1,ind1+normoffset,
                                 ind2+indoffset,texind2,ind2+normoffset);

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
                        int texind1 = ind1+texoffset;
                        int texind2 = ind2+texoffset;
                        int texind3 = ind3+texoffset;
                        if (!sharevert) {
                          fprintf (OutputObj,"vt %f %f\n",s1,t1);
                          fprintf (OutputObj,"vt %f %f\n",s2,t2);
                          fprintf (OutputObj,"vt %f %f\n",s3,t3);
                          texcount+=3;
                          texind1=texcount-3;
                          texind2=texcount-2;
                          texind3=texcount-1;
                        }
                        fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 ind1+indoffset,texind1,ind1+normoffset,
                                 ind2+indoffset,texind2,ind2+normoffset,
                                 ind3+indoffset,texind3,ind3+normoffset);
                        
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
                        int texind1 = ind1+texoffset;
                        int texind2 = ind2+texoffset;
                        int texind3 = ind3+texoffset;
                        int texind4 = ind4+texoffset;
                        if (!sharevert) {
                          fprintf (OutputObj,"vt %f %f\n",s1,t1);
                          fprintf (OutputObj,"vt %f %f\n",s2,t2);
                          fprintf (OutputObj,"vt %f %f\n",s3,t3);
                          fprintf (OutputObj,"vt %f %f\n",s4,t4);
                          texcount+=4;
                          texind1=texcount-4;
                          texind2=texcount-3;
                          texind3=texcount-2;
                          texind4=texcount-1;
                        }
                        fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 ind1+indoffset,texind1,ind1+normoffset,
                                 ind2+indoffset,texind2,ind2+normoffset,
                                 ind3+indoffset,texind3,ind3+normoffset,
                                 ind4+indoffset,texind4,ind4+normoffset);
                        
		  }
		  //End Quads
		  //Linestrips
		  int32bit numlinestrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit lstrip=0;lstrip<numlinestrips;lstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Linestrip flatshade=\"%d\">\n",flatshade);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
			}
			fprintf(Outputfile,"\t</Linestrip>");
		  }
		  //End Linestrips
		  //Tristrips
		  int32bit numtristrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit tstrip=0;tstrip<numtristrips;tstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Tristrip flatshade=\"%d\">\n",flatshade);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
                        int to1=0,to2=0;
                        int indo1=0,indo2=0;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
                          int tco=ind+texoffset;
                          int vind=ind+indoffset;
                          int nind=ind+normoffset;
                          if (!sharevert) {
                            fprintf (OutputObj,"vt %f %f\n",s,t);
                            tco=texcount;
                            texcount+=1;
                          }   
                          if (elem>1) {
                            if (elem%2) {
                              fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                       indo2,to2,indo2,indo1,to1,indo1,vind,tco,nind);
                            }else {
                              fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                       indo1,to1,indo1,indo2,to2,indo2,vind,tco,nind);
                            }
                          }
                          indo2=indo1;
                          indo1=vind;
                          to2=to1;
                          to1=tco;
			}
			fprintf(Outputfile,"\t</Tristrip>");
		  }
		  //End Tristrips
		  //Trifans
		  int32bit numtrifans=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit tfan=0;tfan<numtrifans;tfan++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Trifan flatshade=\"%d\">\n",flatshade);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
                        int indo1=0,indo2=0,to1=0,to2=0;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
                          int tco=ind+texoffset;
                          int nind = ind+normoffset;
                          int vind = ind+indoffset;
                          if (!sharevert) {
                            fprintf (OutputObj,"vt %f %f\n",s,t);
                            tco=texcount;
                            texcount+=1;
                          }   
                          if (elem>1) {
                            fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                     indo2,to2,indo2,indo1,to1,indo1,vind,tco,nind);
                          }
                          if (elem==0) {
                            indo2=vind;
                            to2=tco;
                          }
                          indo1=vind;
                          to1=tco;
			}
			fprintf(Outputfile,"\t</Trifan>");
		  }
		  //End Trifans
		  //Quadstrips
		  int32bit numquadstrips=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
		  word32index+=1;
		  for(int32bit qstrip=0;qstrip<numquadstrips;qstrip++){
			int32bit numstripelements=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//number of vertices
			int32bit flatshade=VSSwapHostIntToLittle(inmemfile[word32index+1].i32val);//flatshade
			fprintf(Outputfile,"\t<Quadstrip flatshade=\"%d\">\n",flatshade);
			word32index+=1+NUMFIELDSPERPOLYGONSTRUCTURE;
                        int indo1=0,indo2=0,to1=0,to2=0;
			for(int32bit elem=0;elem<numstripelements;elem++){
			  int32bit ind=VSSwapHostIntToLittle(inmemfile[word32index].i32val);//index 1
			  float32bit s=VSSwapHostFloatToLittle(inmemfile[word32index+1].f32val);//s
			  float32bit t=VSSwapHostFloatToLittle(inmemfile[word32index+2].f32val);//t
		      word32index+=NUMFIELDSPERREFERENCEDVERTEX;
			  fprintf(Outputfile,"\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n",ind,s,t);
                          int tco=ind+texoffset;
                          int nind = ind+normoffset;
                          int vind = ind+indoffset;                          
                          if (!sharevert) {
                            fprintf (OutputObj,"vt %f %f\n",s,t);
                            tco=texcount;
                            texcount+=1;
                          }   
                          if (elem>1) {
                            if (elem%2) {
                              fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                       indo2,to2,indo2,indo1,to1,indo1,vind,tco,nind);
                            }else {
                              fprintf (OutputObj,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                       indo1,to1,indo1,indo2,to2,indo2,vind,tco,nind);
                            }
                          }
                          indo2=indo1;
                          indo1=vind;
                          to2=to1;
                          to1=tco;
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
