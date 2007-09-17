#include <string>
#include "savenet_util.h"
#include "gfx/cockpit_generic.h"
#include "cmd/unit_generic.h"
#include "networking/client.h"
#include "faction_generic.h"

using std::string;

void	SaveNetUtil::GetSaveStrings( ClientPtr clt, string & savestr, string & xmlstr)
{
	if (!clt) return;
	Unit * un = clt->game_unit.GetUnit();
	if (!un) return;
	/*
	Cockpit * cp = _Universe->isPlayerStarship( un);
	const QVector POS( un->curr_physical_state.position);
	if (cp) {
		// Only get the player data, the dynamic universe part is separated
		savestr = cp->savegame->WritePlayerData ( POS, cp->unitfilename, cp->savegame->GetStarSystem().c_str(), cp->credits, FactionUtil::GetFactionName( cp->GetParent()->faction));
	}
	xmlstr = un->WriteUnitString();
	*/
	int cpnum = _Universe->whichPlayerStarship(un);
	if (cpnum<0) return;
	GetSaveStrings( cpnum, savestr, xmlstr );
}

void	SaveNetUtil::GetSaveStrings( int numplayer, string & savestr, string & xmlstr)
{
	Cockpit * cp;
	Unit * un;
	cp = _Universe->AccessCockpit( numplayer);
	if (cp) {
		un = cp->GetParent();
		if (un) {
			xmlstr = un->WriteUnitString();
			const QVector POS(un->LocalPosition());
			string fname (cp->activeStarSystem->getFileName());
			savestr = cp->savegame->WriteSaveGame (fname.c_str(), un->LocalPosition(), cp->credits, cp->unitfilename, numplayer,
						FactionUtil::GetFactionName( cp->GetParent()->faction), false);
			// Only get the player data, the dynamic universe part is separated
			//savestr = cp->savegame->WritePlayerData ( POS, cp->unitfilename, fname.c_str(), cp->credits, FactionUtil::GetFactionName( cp->GetParent()->faction));
		}
	}
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
	fp = VSFileSystem::OpenFile( savefile.c_str(), "wb");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		VSExit(1);
	}
	VSFileSystem::Write( xmlstr.c_str(), sizeof( char), xmlstr.length(), fp);
	VSFileSystem::Close( fp);
	// Write the save file
	savefile = path+cp->savegame->GetCallsign()+".save";
	fp = VSFileSystem::OpenFile( savefile.c_str(), "wb");
	if( !fp)
	{
		cout<<"Error opening save file "<<savefile<<endl;
		VSExit(1);
	}
	VSFileSystem::Write( savestr.c_str(), sizeof( char), savestr.length(), fp);
	VSFileSystem::Close( fp);
}
*/
