#include <string>
#include <vector>
using std::string;
using std::vector;
#include "endianness.h"
#include <stdlib.h>
#include <stdio.h>

void BFXMToXmeshOrOBJ(FILE* Inputfile, FILE* Outputfile,FILE * outputObj,FILE * mtl, std::string meshname,char hackaroundhack); //Translate BFXM file Inputfile to text file Outputfile
