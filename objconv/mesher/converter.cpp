#include "from_obj.h"
#include "from_BFXM.h"
void usage();
void usage(){
	fprintf(stderr,"usage:\n\tmesher <inputfile> <outputfile> <command>\n\nWhere command is a 3 letter sequence of:\n\tInputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tOutputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tCommandflag:\n\t\ta: append to Outputfile\n\t\tc: create Outputfile\n");
}
bool flip=false;
bool flop=false;
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
//		fprintf(kk,"wrong number of arguments %d , aborting\n",argc);
		for(int i = 0; i<argc;i++){
			fprintf(stderr,"%d : %s\n",i,argv[i]);			
		}
		usage();
//		fclose(kk);
		exit(-1);
	}
	if(strlen(argv[3])!=3){
		fprintf(stderr,"Invalid command %s, aborting\n",argv[3]);
//		fprintf(kk,"Invalid command %s, aborting\n",argv[3]);
		usage();
//		fclose(kk);
		exit(-1);
	}
//	fclose(kk);

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
        string tmp = argv[2];
        int where=where=tmp.find_last_of(".");
        tmp = tmp.substr(0,where);
        string obj = tmp+".obj";
        string mtl = tmp+".mtl";
        FILE * OutputObj = fopen (obj.c_str(),"w");
        FILE * OutputMtl = fopen (mtl.c_str(),"w");
	BFXMToXmesh(Inputfile,Outputfile,OutputObj,OutputMtl,tmp);
  }else if (createBFXMfromOBJ) {
     FILE* Inputfile=fopen(argv[1],"r");
     Outputfile=fopen(argv[2],"wb+"); //create file for text output
     string tmp = argv[1];
     int where=where=tmp.find_last_of(".");
     tmp = tmp.substr(0,where);
     string mtl = tmp+".mtl";
     FILE * InputMtl = fopen (mtl.c_str(),"r");
     ObjToBFXM(Inputfile, InputMtl,Outputfile);
  } else if(createOBJfromBFXM||createOBJfromxmesh||createxmeshesfromOBJ){
	fprintf(stderr,"OBJ functions not yet supported: - aborting\n");
	exit(-1);
  } else {
	  fprintf(stderr,"Invalid command: %s - aborting",argv[3]);
	  usage();
	  exit(-1);
  }
  return 0;
}
