#include "from_obj.h"
#include "from_BFXM.h"
#ifdef HAVE_OGRE
#include "to_OgreMesh.h"
#endif
void usage();
void usage(){
	fprintf(stderr,"usage:\n\tmesher <inputfile> <outputfile> <command> [-x|-y|-z<Translation Distance>] [-forceflatshade]\n\nWhere command is a 3 letter sequence of:\n\tInputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tOutputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tCommandflag:\n\t\ta: append to Outputfile\n\t\tc: create Outputfile\n");
}
bool flip=false;
bool flop=false;
bool flips=false;
bool flipt=false;
bool flipn=false;
bool autolod=false;
bool autoedge=false;
bool autotangent=false;
float transx=0;
float transy=0;
float transz=0;
const char* basepath="";
static bool forcenormals=true;
bool dims=false;
int main (int argc, char** argv) {

  // executable's path
  std::string rootpath=argv[0];
#if defined(_WIN32)&&!defined(__CYGWIN__)
  rootpath.erase(rootpath.rfind('\\'));
#else
  rootpath.erase(rootpath.rfind('/'));
#endif

  { 
	for (int i=0;i<argc;++i) {
    bool match=false;
    if (strcmp(argv[i],"-flip")==0) {
      match=true;
      flip=true;
    }
    if (strcmp(argv[i],"-flop")==0) {
      match=true;
      flop=true;
    }
    if (strcmp(argv[i],"-flips")==0) {
      match=true;
      flips=true;
    }
    if (strcmp(argv[i],"-flipt")==0) {
      match=true;
      flipt=true;
    }
    if (strcmp(argv[i],"-flipn")==0) {
      match=true;
      flipn=true;
    }
    if (strcmp(argv[i],"-autolod")==0) {
      match=true;
      autolod=true;
    }
    if (strcmp(argv[i],"-autoedge")==0) {
      match=true;
      autoedge=true;
    }
    if (strcmp(argv[i],"-autotangent")==0) {
      match=true;
      autotangent=true;
    }
    if (strcmp(argv[i],"-forceflatshade")==0) {
      match=true;
	  forcenormals=false;
    }
    if (strncmp(argv[i],"-basepath=",10)==0) {
      match=true;
      basepath = argv[i]+10;
    }
	
	if (strncmp(argv[i],"-dims",5)==0) {
		dims=true;
    }

	if (strncmp(argv[i],"-x",2)==0) {
		sscanf(argv[i]+2,"%f",&transx);
		match=true;
    }
    if (strncmp(argv[i],"-y",2)==0) {
		sscanf(argv[i]+2,"%f",&transy);
		match=true;
    }
    if (strncmp(argv[i],"-z",2)==0) {
		sscanf(argv[i]+2,"%f",&transz);
		match=true;
    }
    if(match) {
      for (int j=i;j+1<argc;++j) {
        argv[j]=argv[j+1];
      }
      argc--;
      i--;
    }
  }}

  if(dims){
	  fprintf(stderr,"Processing %s...\n",argv[2]);
	  BFXMtoBoxDims(fopen(argv[2],"rb"), argv[2]);
	  exit(0);
  }
	if (argc!=4){
		fprintf(stderr,"wrong number of arguments %d, aborting\n",argc);
		for(int i = 0; i<argc;i++){
			fprintf(stderr,"%d : %s\n",i,argv[i]);			
		}
		usage();
		exit(-1);
	}
	if(strlen(argv[3])!=3){
		fprintf(stderr,"Invalid command %s, aborting\n",argv[3]);
		usage();
		exit(-1);
	}

  bool appendxmeshtobfxm=(argv[3][0]=='x'&&argv[3][1]=='b'&&argv[3][2]=='a');
  bool createBFXMfromxmesh=(argv[3][0]=='x'&&argv[3][1]=='b'&&argv[3][2]=='c');
  bool createxmeshesfromBFXM=(argv[3][0]=='b'&&argv[3][1]=='x'&&argv[3][2]=='c');
  bool createOBJfromBFXM=(argv[3][0]=='b'&&argv[3][1]=='o'&&argv[3][2]=='c');
  bool createOBJfromxmesh=(argv[3][0]=='x'&&argv[3][1]=='o'&&argv[3][2]=='c');
  bool createBFXMfromOBJ=(argv[3][0]=='o'&&argv[3][1]=='b'&&argv[3][2]=='c');
  bool createxmeshesfromOBJ=(argv[3][0]=='o'&&argv[3][1]=='x'&&argv[3][2]=='c');

  char inputType = argv[3][0];
  char outputType = argv[3][1];
  char actionType = argv[3][2];

  FILE * Outputfile;
  if(appendxmeshtobfxm){
	Outputfile=fopen(argv[2],"rb+"); //append to end, but not append, which doesn't do what you want it to.
	fseek(Outputfile, 0, SEEK_END);
	XML memfile=(LoadXML(argv[1],1));
    xmeshToBFXM(memfile,Outputfile,'a',forcenormals);
  }else if(createBFXMfromxmesh){
	Outputfile=fopen(argv[2],"wb+"); //create file for BFXM output
	XML memfile=(LoadXML(argv[1],1));
    xmeshToBFXM(memfile,Outputfile,'c',forcenormals);
  } else if(createxmeshesfromBFXM){
	FILE* Inputfile=fopen(argv[1],"rb");
	Outputfile=fopen(argv[2],"w+"); //create file for text output
	FILE * OutputObj=NULL;
	FILE * OutputMtl=NULL;
	string tmp = argv[2];
    int where=where=tmp.find_last_of(".");
    tmp = tmp.substr(0,where);
	BFXMToXmeshOrOBJ(Inputfile,Outputfile,OutputObj,OutputMtl,tmp,'x');
  }else if (createBFXMfromOBJ) {
     FILE* Inputfile=fopen(argv[1],"r");
     Outputfile=fopen(argv[2],"wb+"); //create file for text output
     string tmp = argv[1];
     int where=where=tmp.find_last_of(".");
     tmp = tmp.substr(0,where);
     string mtl = ObjGetMtl(Inputfile,argv[2]);
     FILE * InputMtl = fopen (mtl.c_str(),"r");
     ObjToBFXM(Inputfile, InputMtl,Outputfile,forcenormals);
  } else if(createOBJfromBFXM){
	  FILE* Inputfile=fopen(argv[1],"rb");
	  FILE* OutputObj=fopen(argv[2],"w+"); //create file for text output
	  string tmp = argv[2];
      int where=where=tmp.find_last_of(".");
      tmp = tmp.substr(0,where);
      string mtl = tmp+".mtl";
      FILE * OutputMtl = fopen (mtl.c_str(),"w");
	  Outputfile=NULL;
	BFXMToXmeshOrOBJ(Inputfile,Outputfile,OutputObj,OutputMtl,tmp,'o');
  } else if(createOBJfromxmesh||createxmeshesfromOBJ){
	fprintf(stderr,"some OBJ functions not yet supported: - aborting\n");
	exit(-1);
  } else if(outputType=='m') {
      if (!(actionType=='a'||actionType=='c')) {
          fprintf(stderr,"unrecognized action (%c)\n",actionType);
#ifdef HAVE_OGRE
      } else if (!(inputType=='o'||inputType=='x')) {
          fprintf(stderr,"input type '%c' not implemented yet\n",inputType);
      } else {
          // OgreMesh
          OgreMeshConverter::ConverterInit();
          OgreMeshConverter::SetTemplatePath(rootpath);

          // Material file - same name and path, .material extension
          string matfile = argv[2];
          matfile.erase(matfile.rfind('.'));
          matfile += ".material";

          void *ctxt = 0;
          if (actionType=='a')
              ctxt = OgreMeshConverter::Init(argv[2],matfile.c_str()); else 
              ctxt = OgreMeshConverter::Init();
          OgreMeshConverter::SetName(ctxt,argv[2]);
          OgreMeshConverter::SetBasePath(ctxt,basepath);

          float refDistance=100;

          // If the input filename is "-", then we're just postprocessing the .mesh file
          if ((argv[1][0]!='-')||(argv[1][1]!='\0')) switch(inputType) {
          case 'o':
              {
                  FILE* Inputfile=fopen(argv[1],"r");
                  string mtl = ObjGetMtl(Inputfile,argv[2]);
                  FILE * InputMtl = fopen (mtl.c_str(),"r");

                  vector<XML> xmls;
                  ObjToXMESH(Inputfile, InputMtl, xmls, forcenormals);

                  for (vector<XML>::iterator it=xmls.begin(); it!=xmls.end(); it++)
                      OgreMeshConverter::Add(ctxt,*it);
              }
              break;
          case 'x':
              {
	              XML memfile=LoadXML(argv[1],1);
                  OgreMeshConverter::Add(ctxt,memfile);
              }
              break;
          case 'b':
          default:
              fprintf(stderr,"input type '%c' not implemented yet\n",inputType);
          }

          OgreMeshConverter::DoneMeshes(ctxt);

          // 4*radialsize - a sensible reference distance
          refDistance = 4*OgreMeshConverter::RadialSize(ctxt);

          // Roundup
          OgreMeshConverter::Optimize(ctxt);
          if (autolod)     OgreMeshConverter::AutoLOD(ctxt,false,4,0.75f,refDistance);
          if (autoedge)    OgreMeshConverter::AutoEdgeList(ctxt);
          if (autotangent) OgreMeshConverter::AutoTangents(ctxt);
          OgreMeshConverter::Dump(ctxt,argv[2],matfile.c_str());

          OgreMeshConverter::ConverterClose();
      }
#else
      } else
          fprintf(stderr,"input type '%c' not implemented yet\n",inputType);
#endif
  } else {
	  fprintf(stderr,"Invalid command: %s - aborting",argv[3]);
	  usage();
	  exit(-1);
  }
  return 0;
}
