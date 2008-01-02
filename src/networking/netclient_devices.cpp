#include "networking/lowlevel/vsnet_debug.h"
#include "vsfilesystem.h"
#include "networking/savenet_util.h"
#include "networking/netclient.h"
#include "cmd/unit_generic.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/networkcomm.h"
#include "networking/lowlevel/packet.h"
#include "networking/fileutil.h"

void NetClient::send( Cmd cmd, NetBuffer &netbuf, bool mode, const char *file, int line) {
	Unit *un = this->game_unit.GetUnit();
	int serial = 0;
	if (un)
		serial = un->GetSerial();
	Packet p;
	p.send( cmd, serial,
			netbuf.getData(), netbuf.getDataLength(),
			mode, NULL,
			*(mode==SENDANDFORGET?this->lossy_socket:this->clt_tcp_sock),
			file, line);
}

/******************************************************************************************/
/*** WEAPON STUFF                                                                      ****/
/******************************************************************************************/
void NetClient::sendCloak(bool engage) {
	NetBuffer netbuf;

	netbuf.addChar( engage?1:0 );
	// Shold people be allowed to scan units in other zones?
//	netbuf.addShort( un->activeStarSystem->GetZone());

	send ( CMD_CLOAK, netbuf, SENDRELIABLE, __FILE__, __LINE__ );

}

void	NetClient::sendCustom( string command, string args, string id)
{
	NetBuffer netbuf;

	netbuf.addString(command);
	netbuf.addString(args);
	netbuf.addString(id);
	
	send ( CMD_CUSTOM, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

// Send a info request about the target
void	NetClient::scanRequest( Unit * target)
{
	Packet p;
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return;

	netbuf.addSerial( target->GetSerial());
	// Shold people be allowed to scan units in other zones?
//	netbuf.addShort( un->activeStarSystem->GetZone());

	send ( CMD_SCAN, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

// Send a info request about the target
void	NetClient::targetRequest( Unit * target)
{
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return;

	netbuf.addSerial( target->GetSerial());
        
	send( CMD_TARGET, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
        if (target->GetSerial()==0) {
          //not networked unit
          un->computer.target.SetUnit(target);
        }
}

// In fireRequest we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetClient::fireRequest( ObjSerial serial, const vector<int> &mount_indicies, char mis)
{
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addChar( mis);
	netbuf.addInt32( mount_indicies.size());
	for (unsigned int i=0;i<mount_indicies.size();++i) {
		netbuf.addInt32(mount_indicies[i]);
	}

	//  NETFIXME: Use UDP for fire requests? or only from server->other clients
	send( CMD_FIREREQUEST, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

void	NetClient::unfireRequest( ObjSerial serial, const vector<int> &mount_indicies)
{
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_indicies.size());
	for (unsigned int i=0;i<mount_indicies.size();++i) {
		netbuf.addInt32(mount_indicies[i]);
	}

	send( CMD_UNFIREREQUEST, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

void	NetClient::cargoRequest( ObjSerial buyer, ObjSerial seller, const std::string &cargo,unsigned int quantity,
								 int mountOffset, int subunitOffset)
{
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return;

	netbuf.addSerial( buyer ); // If the buyer is the player, it is buying cargo.
	netbuf.addSerial( seller ); // If seller is the player, it is selling cargo.
	netbuf.addInt32( quantity );
	netbuf.addString( cargo );
	netbuf.addInt32( (unsigned int)(mountOffset) );
	netbuf.addInt32 ((unsigned int)(subunitOffset) );
	
	send( CMD_CARGOUPGRADE, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

bool	NetClient::jumpRequest( string newsystem, ObjSerial jumpserial)
{
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return false;
        /*
	netbuf.addString( newsystem);
	netbuf.addSerial( jumpserial);
	netbuf.addShort( un->getStarSystem()->GetZone());
#ifdef CRYPTO
	unsigned char * hash = new unsigned char[FileUtil::Hash.DigestSize()];
	bool autogen;
	FileUtil::HashFileCompute( VSFileSystem::GetCorrectStarSysPath( newsystem+".system", autogen), hash, SystemFile);
	netbuf.addBuffer( hash, FileUtil::Hash.DigestSize());
#endif
        */
	send( CMD_JUMP, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
	// NO, WE MUST NOT BLOCK THE GAME WHILE WE ARE WAITING FOR SERVER AUTH
	// Should wait for jump authorization
	/*
	this->PacketLoop( CMD_JUMP);
	bool ret;
	if( this->jumpok)
		ret = true;
	else
		ret = false;

	jumpok = false;
	*/

	return true;
}

bool	NetClient::readyToJump()
{
	return jumpok;
}

void	NetClient::unreadyToJump()
{
	jumpok = false;
}

void	NetClient::dieRequest( )
{
	// Send a packet with CMD_DOCK with serial and an ObjSerial = unit_to_dock_with_serial
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return;

	cerr<<"SENDING A KILL REQUEST FOR PLAYER "<<callsign<<endl;
	send( CMD_KILL, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

void	NetClient::saveRequest( )
{
	// Send a packet with CMD_DOCK with serial and an ObjSerial = unit_to_dock_with_serial
	NetBuffer netbuf;
	Unit *un = this->game_unit.GetUnit();
	if (!un) return;

	cerr<<"SENDING A SAVE REQUEST FOR PLAYER "<<callsign<<endl;
	send( CMD_SAVEACCOUNTS, netbuf, SENDRELIABLE, __FILE__, __LINE__ );

	int cpnum = _Universe->whichPlayerStarship(un);
	if (cpnum>=0 && lastsave.size()>=2) {
		SaveNetUtil::GetSaveStrings(cpnum, lastsave[0], lastsave[1], true);
	}
	
}

void	NetClient::dockRequest( ObjSerial utdw_serial)
{
	// Send a packet with CMD_DOCK with serial and an ObjSerial = unit_to_dock_with_serial
	NetBuffer netbuf;

	cerr<<"SENDING A DOCK REQUEST FOR UNIT "<<utdw_serial<<endl;
	netbuf.addSerial( utdw_serial);
	send( CMD_DOCK, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

void	NetClient::missionRequest( unsigned short packetType, string mission, int pos)
{
	NetBuffer netbuf;
	
	netbuf.addShort(packetType);
	netbuf.addString(mission);
	netbuf.addInt32(pos);
	
	send( CMD_MISSION, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

void	NetClient::undockRequest( ObjSerial utdw_serial)
{
	// Send a packet with CMD_UNDOCK with serial and an ObjSerial = unit_to_undock_with_serial
	NetBuffer netbuf;

	cerr<<"SENDING A UNDOCK NOTIFICATION FOR UNIT "<<utdw_serial<<endl;
	netbuf.addSerial( utdw_serial);
	send( CMD_UNDOCK, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
}

/******************************************************************************************/
/*** COMMUNICATION STUFF                                                               ****/
/******************************************************************************************/

void	NetClient::createNetComm( float minfreq, float maxfreq, bool video, bool secured, string method)
{
	if( NetComm!=NULL)
	{
		current_freq = minfreq;
		selected_freq = maxfreq;
		this->NetComm = new NetworkCommunication( minfreq, maxfreq, video, secured, method);
	}
}

void	NetClient::destroyNetComm()
{
	if( NetComm!=NULL)
	{
		delete NetComm;
		NetComm = NULL;
	}
}

void	NetClient::startCommunication()
{
	if( NetComm!=NULL)
	{
		char webcam_support = NetComm->HasWebcam();
		char portaudio_support = NetComm->HasPortaudio();
		selected_freq = current_freq;
		NetBuffer netbuf;
		netbuf.addFloat( selected_freq);
		netbuf.addChar( NetComm->IsSecured());
		netbuf.addChar( webcam_support);
		netbuf.addChar( portaudio_support);
		NetComm->InitSession( selected_freq);
		//cerr<<"Session started."<<endl;
		//cerr<<"Grabbing an image"<<endl;
		Packet p;
		p.send( CMD_STARTNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, *this->clt_tcp_sock,
   	         __FILE__, PSEUDO__LINE__(1565) );
		cerr<<"Starting communication session\n\n"<<endl;
		//NetComm->GrabImage();
	}
}

void	NetClient::stopCommunication()
{
	if( NetComm!=NULL)
	{
		NetBuffer netbuf;
		netbuf.addFloat( selected_freq);
		Packet p;
		p.send( CMD_STOPNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, *this->clt_tcp_sock,
   	         __FILE__, PSEUDO__LINE__(1578) );
		NetComm->DestroySession();
		cerr<<"Stopped communication session"<<endl;
	}
}

void	NetClient::decreaseFrequency()
{
	if( NetComm!=NULL)
	{
		if( current_freq == NetComm->MinFreq())
			current_freq = NetComm->MaxFreq();
		else
			current_freq -= .1;
	}
}

void	NetClient::increaseFrequency()
{
	if( NetComm!=NULL)
	{
		if( current_freq == NetComm->MaxFreq())
			current_freq = NetComm->MinFreq();
		else
			current_freq += .1;
	}
}

float	NetClient::getSelectedFrequency()
{ return this->selected_freq;}

float	NetClient::getCurrentFrequency()
{ return this->current_freq;}

void	NetClient::sendTextMessage( string message)
{
	// Only send if netcomm is active and we are connected on a frequency
	if( NetComm!=NULL && NetComm->IsActive())
		NetComm->SendMessage( *this->clt_tcp_sock, this->serial, message);
}

/**************************************************************/
/**** Check if we have to send a webcam picture            ****/
/**************************************************************/

void	NetClient::startWebcamTransfer()
{ this->NetComm->StartWebcamTransfer(); }

void	NetClient::stopWebcamTransfer()
{ this->NetComm->StopWebcamTransfer(); }

char *	NetClient::getWebcamCapture()
{
	if( NetComm != NULL)
		return NetComm->GetWebcamCapture();
	return NULL; // We have no choice...
}

char *	NetClient::getWebcamFromNetwork( int & length)
{
	if( NetComm != NULL)
		return NetComm->GetWebcamFromNetwork( length);
	return NULL; // We have no choice...
}

bool NetClient::IsNetcommActive() const
{
    return ( this->NetComm==NULL ? false : this->NetComm->IsActive() );
}

bool NetClient::IsNetcommSecured() const
{
	bool ret = false;
	if( this->NetComm!=NULL)
		ret = this->NetComm->IsSecured();
	
    return ret;
}

void	NetClient::switchSecured()
{
	if( NetComm!=NULL)
		NetComm->SwitchSecured();
}
void	NetClient::switchWebcam()
{
	if( NetComm!=NULL)
		NetComm->SwitchWebcam();
}
bool	NetClient::hasWebcam()
{ return NetComm->HasWebcam(); }

