#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "lin_time.h"
#include "networking/lowlevel/vsnet_serversocket.h"

extern double clienttimeout;

/**************************************************************/
/**** Handles new connections                              ****/
/**************************************************************/

ClientPtr NetServer::newConnection_udp( const AddressIP& ipadr )
{
    COUT << " enter " << "NetServer::newConnection_udp" << endl;

    SOCKETALT sock( udpNetwork->get_fd(), SOCKETALT::UDP, ipadr, _sock_set );

    ClientPtr ret = addNewClient( sock, false );
    nbclients++;

    return ret;
}

ClientPtr NetServer::newConnection_tcp( )
{
    ClientPtr ret;

    // Get new connections if there are - do nothing in standard UDP mode
    bool valid = false;
    do
    {
        SOCKETALT sock = tcpNetwork->acceptNewConn( );
        valid = sock.valid();
        if( valid )
        {
            ret = addNewClient( sock, true );
            nbclients++;
        }
    }
    while( valid );
    return ret;
}

/**************************************************************/
/**** Disconnects timed out clients                        ****/
/**************************************************************/

void	NetServer::checkTimedoutClients_udp()
{
	/********* Method 1 : compare latest_timestamp to current time and see if > CLIENTTIMEOUT */
	double curtime = (unsigned int) getNewTime();
	double deltatmp = 0;
	for (LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
        if( cl->isUdp() )
        {
		    deltatmp = (fabs(curtime - cl->latest_timeout));
		    if( cl->latest_timeout!=0)
		    {
			    //COUT<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			    // Here considering a delta > 0xFFFFFFFX where X should be at least something like 0.9
			    // This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			    // side -> when timestamp has grown enough to became bigger than what an u_int can store
			    if( cl->ingame && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.9) )
			    {
	                Unit * un;
				    un = cl->game_unit.GetUnit();
				    cerr<<"ACTIVITY TIMEOUT for client number "<<un->GetSerial()<<endl;
				    COUT<<"\t\tCurrent time : "<<curtime<<endl;
				    COUT<<"\t\tLatest timeout : "<<(cl->latest_timeout)<<endl;
				    COUT<<"t\tDifference : "<<deltatmp<<endl;
                    cl->_disconnectReason = "UDP timeout";
				    discList.push_back( cl );
			    }
		    }
        }
	}
}

/**************************************************************/
/**** Receive a message from a client                      ****/
/**************************************************************/

void	NetServer::recvMsg_tcp( ClientPtr clt )
{
    char	command;
    AddressIP	ipadr;
    // int nbpackets = 0;

    assert( clt );

    SOCKETALT sockclt( clt->sock );
	PacketMem mem;

    int recvbytes = sockclt.recvbuf( mem, &ipadr );

    if( recvbytes < 0 )
    {
	    cerr << ", disconnecting(error)" << endl;
        clt->_disconnectReason = "TCP error";
        discList.push_back( clt );
    }
    else if( recvbytes == 0 )
    {
	    cerr << ", disconnecting(eof)" << endl;
        clt->_disconnectReason = "TCP peer closed";
        discList.push_back( clt );
    }
    else
    {
        Packet packet( mem );
		packet.setNetwork( &ipadr, sockclt );
		command = packet.getCommand( );
        if( clt )
			this->updateTimestamps( clt, packet);

#ifdef VSNET_DEBUG
		COUT << "Created a packet with command " << displayCmd(Cmd(command)) << endl;
	    mem.dump( cerr, 3 );
#endif

		// In TCP we always process
        this->processPacket( clt, command, ipadr, packet );
    }
}

void NetServer::recvMsg_udp( )
{
    SOCKETALT sockclt( udpNetwork->get_fd(), SOCKETALT::UDP, udpNetwork->get_adr(), _sock_set );
    ClientPtr clt;
    AddressIP ipadr;
	bool process = true;

	PacketMem mem;
    int    ret;
    ret = sockclt.recvbuf( mem, &ipadr );
    if( ret > 0 )
    {
        Packet packet( mem );
	    packet.setNetwork( &ipadr, sockclt );

        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

        COUT << "Received from serial : " << nserial << endl;

        // Find the corresponding client
        ClientPtr tmp;
		bool      found = false;
        for( LI i=allClients.begin(); i!=allClients.end(); i++)
        {
            tmp = (*i);
            if( tmp->isUdp() && tmp->game_unit.GetUnit()->GetSerial() == nserial)
            {
                clt = tmp;
                found = 1;
				COUT << " found client " << *(clt.get()) << endl;
				break;
            }
        }
        if( !found && command!=CMD_LOGIN)
        {
            COUT << "Error : non-login message received from unknown client !" << endl;
            // Maybe send an error packet handled by the client
            return;
        }

        // Check if the client's IP is still the same (a very little and unaccurate in some cases protection
		// against spoofing client serial#)
        if (clt && (ipadr!=clt->cltadr))
        {
	    	assert( command != CMD_LOGIN ); // clt should be 0 because ObjSerial was 0

            COUT << "Error : IP changed for client # " << clt->game_unit.GetUnit()->GetSerial() << endl;
            clt->_disconnectReason = "possible IP spoofing";
            discList.push_back( clt );
	    	/* It is not entirely impossible for this to happen; it would be nice
			 * to add an additional identity check. For now we consider it an error.
	     	*/
        }
        else
        {
            if( clt )
				process = this->updateTimestamps( clt, packet);

			// Do not process a packet considered to be late and not important (positions and damage ?)
			if( process)
            	this->processPacket( clt, command, ipadr, packet );
        }
    }
}

