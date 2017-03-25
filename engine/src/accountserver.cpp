//! Account Server.
/*!
 * Very simply creates a new "server" object for other users to connect 
 * to.
 */

#include <iostream>
#include <stdlib.h>
#include <string>
using std::string;
#include "networking/acctserver.h"
#include "lin_time.h"

void VSExit( int code )
{
    exit( code );
}

string getStarSystemSector( const string &in )
{
    return string( "" );
}

string GetUnitDir( string filename )
{
    return string( "" );
}

char  SERVER = 2;
float simulation_atom_var = (float) 1.0/10.0;
class NetClient
{};
NetClient *Network;

int main( int argc, char **argv )
{
    InitTime();
    setNewTime( ( (double) time( NULL ) )-VEGA_EPOCH );
    AccountServer *Server = new AccountServer;

    Server->start();

    delete Server;
    return 0;
}

