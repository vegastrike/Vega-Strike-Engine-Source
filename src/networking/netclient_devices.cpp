#include "networking/lowlevel/vsnet_debug.h"
#include "vsfilesystem.h"
#include "networking/netclient.h"
#include "cmd/unit_generic.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/networkcomm.h"
#include "networking/lowlevel/packet.h"
#include "networking/fileutil.h"

/******************************************************************************************/
/*** WEAPON STUFF                                                                      ****/
/******************************************************************************************/

// Send a info request about the target
void	NetClient::scanRequest( Unit * target)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( target->GetSerial());
	netbuf.addShort( this->game_unit.GetUnit()->activeStarSystem->GetZone());

	p.send( CMD_TARGET, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1485) );
}

// In fireRequest we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetClient::fireRequest( ObjSerial serial, int mount_index, char mis)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);
	netbuf.addChar( mis);

	p.send( CMD_FIREREQUEST, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1503) );
}

void	NetClient::unfireRequest( ObjSerial serial, int mount_index)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);

	p.send( CMD_UNFIREREQUEST, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1518) );
}

bool	NetClient::jumpRequest( string newsystem, ObjSerial jumpserial)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addString( newsystem);
	netbuf.addSerial( jumpserial);
	netbuf.addShort( this->game_unit.GetUnit()->activeStarSystem->GetZone());
#ifdef CRYPTO
	unsigned char * hash = new unsigned char[FileUtil::Hash.DigestSize()];
	bool autogen;
	FileUtil::HashFileCompute( VSFileSystem::GetCorrectStarSysPath( newsystem+".system", autogen), hash, SystemFile);
	netbuf.addBuffer( hash, FileUtil::Hash.DigestSize());
#endif

	p.send( CMD_JUMP, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1534) );
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

void	NetClient::dockRequest( ObjSerial utdw_serial)
{
	// Send a packet with CMD_DOCK with serial and an ObjSerial = unit_to_dock_with_serial
	Packet p;
	NetBuffer netbuf;

	cerr<<"SENDING A DOCK REQUEST FOR UNIT "<<utdw_serial<<endl;
	netbuf.addSerial( utdw_serial);
	p.send( CMD_DOCK, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(97) );
}

void	NetClient::undockRequest( ObjSerial utdw_serial)
{
	// Send a packet with CMD_UNDOCK with serial and an ObjSerial = unit_to_undock_with_serial
	Packet p;
	NetBuffer netbuf;

	cerr<<"SENDING A UNDOCK NOTIFICATION FOR UNIT "<<utdw_serial<<endl;
	netbuf.addSerial( utdw_serial);
	p.send( CMD_UNDOCK, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(110) );
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
		p.send( CMD_STARTNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock,
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
		p.send( CMD_STOPNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock,
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
		NetComm->SendMessage( this->clt_sock, this->serial, message);
}

/**************************************************************/
/**** Check if we have to send a webcam picture            ****/
/**************************************************************/

char *	NetClient::getWebcamCapture()
{
	if( NetComm != NULL)
		return NetComm->GetWebcamCapture();
	return NULL; // We have no choice...
}

char *	NetClient::getWebcamFromNetwork()
{
	if( NetComm != NULL)
		return NetComm->GetWebcamFromNetwork();
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

