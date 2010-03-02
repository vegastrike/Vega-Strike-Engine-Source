#include "command/command.h"

//used when compiled as a command line target; otherwise
//a gui version would call command directly

int main(int argc, char *argv[])
{
    return command( argc, argv );
}


