#include "mesh_io.h"
#include "to_BFXM.h"
#include "from_BFXM.h"

void usage();
void usage(){
	fprintf(stderr,"usage:\n\tmesher <inputfile> <outputfile> <command>\n\nWhere command is a 3 letter sequence of:\n\tInputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tOutputfiletype:\n\t\tb:BFXM\n\t\to:OBJ\n\t\tx:xmesh\n\tCommandflag:\n\t\ta: append to Outputfile\n\t\tc: create Outputfile\n");
}

int main (int argc, char** argv) {
	if (argc!=4){
		fprintf(stderr,"wrong number of arguments, aborting\n");
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
	BFXMToXmesh(Inputfile,Outputfile);
  } else if(createOBJfromBFXM||createOBJfromxmesh||createBFXMfromOBJ||createxmeshesfromOBJ){
	fprintf(stderr,"OBJ functions not yet supported: - aborting\n");
	exit(-1);
  } else {
	  fprintf(stderr,"Invalid command: %s - aborting",argv[3]);
	  usage();
	  exit(-1);
  }
  return 0;
}
