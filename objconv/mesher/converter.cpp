#include "from_obj.h"
#include "from_BFXM.h"
void usage();
void usage(){
	fprintf(stderr,"usage:\n\tmesher <inputfile> <outputfile> <command> [-x|-y|-z<Translation Distance>] [-forceflatshade]\n\nWhere command is a 3 letter sequence of:\n\tInputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tOutputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tCommandflag:\n\t\ta: append to Outputfile\n\t\tc: create Outputfile\n");
}
bool forceflatshade=false;
bool flip=false;
bool flop=false;
float transx=0;
float transy=0;
float transz=0;
int main (int argc, char** argv) {
  {for (int i=0;i<argc;++i) {
    bool match=false;
    if (strcmp(argv[i],"-flip")==0) {
      match=true;
      flip=true;
    }
    if (strcmp(argv[i],"-flop")==0) {
      match=true;
      flop=true;
    }
    if (strcmp(argv[i],"-forceflatshade")==0) {
      match=true;
      forceflatshade=true;
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

  FILE * Outputfile;
  if(appendxmeshtobfxm){
	Outputfile=fopen(argv[2],"rb+"); //append to end, but not append, which doesn't do what you want it to.
	fseek(Outputfile, 0, SEEK_END);
	XML memfile=(LoadXML(argv[1],1));
    xmeshToBFXM(memfile,Outputfile,'a');
  }else if(createBFXMfromxmesh){
	Outputfile=fopen(argv[2],"wb+"); //create file for BFXM output
	XML memfile=(LoadXML(argv[1],1));
    xmeshToBFXM(memfile,Outputfile,'c');
  } else if(createxmeshesfromBFXM){
	FILE* Inputfile=fopen(argv[1],"rb");
	Outputfile=fopen(argv[2],"w+"); //create file for text output
	FILE * OutputObj=NULL;
	FILE * OutputMtl=NULL;
	string tmp = argv[2];
    int where=where=tmp.find_last_of(".");
    tmp = tmp.substr(0,where);
	BFXMToXmesh(Inputfile,Outputfile,OutputObj,OutputMtl,tmp,'x');
  }else if (createBFXMfromOBJ) {
     FILE* Inputfile=fopen(argv[1],"r");
     Outputfile=fopen(argv[2],"wb+"); //create file for text output
     string tmp = argv[1];
     int where=where=tmp.find_last_of(".");
     tmp = tmp.substr(0,where);
     string mtl = tmp+".mtl";
     FILE * InputMtl = fopen (mtl.c_str(),"r");
     ObjToBFXM(Inputfile, InputMtl,Outputfile);
  } else if(createOBJfromBFXM){
	  FILE* Inputfile=fopen(argv[1],"rb");
	  FILE* OutputObj=fopen(argv[2],"w+"); //create file for text output
	  string tmp = argv[2];
      int where=where=tmp.find_last_of(".");
      tmp = tmp.substr(0,where);
      string mtl = tmp+".mtl";
      FILE * OutputMtl = fopen (mtl.c_str(),"w");
	  Outputfile=NULL;
	BFXMToXmesh(Inputfile,Outputfile,OutputObj,OutputMtl,tmp,'o');
  } else if(createOBJfromxmesh||createxmeshesfromOBJ){
	fprintf(stderr,"some OBJ functions not yet supported: - aborting\n");
	exit(-1);
  } else {
	  fprintf(stderr,"Invalid command: %s - aborting",argv[3]);
	  usage();
	  exit(-1);
  }
  return 0;
}
