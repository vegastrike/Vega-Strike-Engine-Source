#include <string>
#include "savenet_util.h"
#include "gfx/cockpit_generic.h"
#include "cmd/unit_generic.h"

using std::string;

void	SaveNetUtil::GetSaveStrings( Client * clt, string & savestr, string & xmlstr)
{
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	savestr = cp->savegame->WriteSaveGame( cp->savegame->GetStarSystem().c_str(), clt->current_state.getPosition(), cp->credits, cp->unitfilename, 0, false);
	xmlstr = clt->game_unit.GetUnit()->WriteUnitString();
}

void	SaveNetUtil::GetSaveStrings( int numplayer, string & savestr, string & xmlstr)
{
	Cockpit * cp;
	Unit * un;
	cp = _Universe->AccessCockpit( numplayer);
	un = cp->GetParent();
	xmlstr = un->WriteUnitString();
	savestr = cp->savegame->WriteSaveGame (cp->activeStarSystem->getFileName().c_str(),un->LocalPosition(),cp->credits,cp->unitfilename,0, false);
}

void	SaveNetUtil::GetSaveBuffer( string savestr, string xmlstr, char * buffer)
{
	unsigned int total_size = savestr.length() + xmlstr.length() + 2*sizeof(unsigned int);
	unsigned int nxmllen, nsavelen;
	//char * buffer = new char[total_size];
	memset( buffer, 0, total_size);
	nsavelen = htonl( savestr.length());
	nxmllen = htonl( xmlstr.length());
	// Put the save length in the buffer
	memcpy( buffer, &nsavelen, sizeof( unsigned int));
	// Put the save string in the buffer
	memcpy( buffer + sizeof( unsigned int), savestr.c_str(), savestr.length());
	// Put the xml length in the buffer
	memcpy( buffer + sizeof( unsigned int) + savestr.length(), &nxmllen, sizeof( unsigned int));
	// Put the xml string in the buffer
	memcpy( buffer + 2*sizeof( unsigned int) + savestr.length(), xmlstr.c_str(), xmlstr.length());

	//return buffer;
}

/*
void	SaveNetUtil::SaveFiles( string savestr, string xmlstr, string path, string name)
{
	string savefile;
	FILE * fp;
	savefile = path+cp->savegame->GetCallsign()+".xml";
	fp = fopen( savefile.c_str(), "w");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( xmlstr.c_str(), sizeof( char), xmlstr.length(), fp);
	fclose( fp);
	// Write the save file
	savefile = path+cp->savegame->GetCallsign()+".save";
	fp = fopen( savefile.c_str(), "w");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		exit(1);
	}
	fwrite( savestr.c_str(), sizeof( char), savestr.length(), fp);
	fclose( fp);
}
*/
