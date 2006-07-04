#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "lin_time.h"
#include "networking/lowlevel/vsnet_serversocket.h"

extern double clienttimeout;

/**************************************************************/
/**** Handles new connections                              ****/
/**************************************************************/

// Why use this? Most clients use TCP except for position updates, in which case they notify the server through synchronizing time.
ClientPtr NetServer::newConnection_udp( const AddressIP& ipadr )
{
	int clients_should_not_connect_with_udp=1;
	assert(clients_should_not_connect_with_udp==1);
    COUT << " enter " << "NetServer::newConnection_udp" << endl;

    SOCKETALT sock( udpNetwork.get_fd(), SOCKETALT::UDP, ipadr, _sock_set );

    ClientPtr ret = addNewClient( sock ); // no second argument because we don't currently allow connections of *only* UDP with no TCP for some things that can't get lost.
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
            ret = addNewClient( sock );
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
	double curtime = getNewTime();
	double deltatmp = 0;
	for (LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
        if( !cl->lossy_socket->isTcp() )
        {
			// NETFIXME: Does this delta and latest_timeout actually only check UDP or does this include TCP?
			// Time elapsed since latest packet in seconds
		    deltatmp = (fabs(curtime - cl->latest_timeout));
		    if( cl->latest_timeout!=0)
		    {
			    //COUT<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			    // Here considering a delta > 0xFFFFFFFX where X should be at least something like 0.9
			    // This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			    // side -> when timestamp has grown enough to became bigger than what an u_int can store

			    //if( cl->ingame && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.9) )
			    if( cl->ingame==true && deltatmp > clienttimeout)
			    {
	                Unit * un;
				    un = cl->game_unit.GetUnit();
				    cerr<<"ACTIVITY TIMEOUT for client number "<<un->GetSerial()<<endl;
				    COUT<<"\t\tCurrent time : "<<curtime<<endl;
				    COUT<<"\t\tLatest timeout : "<<(cl->latest_timeout)<<endl;
				    COUT<<"t\tDifference : "<<deltatmp<<endl;
                    cl->_disconnectReason = "UDP timeout";
//				    discList.push_back( cl );
					// NETFIXME: Should we actually disconnect them in a UDP timeout or should we just fallback to TCP?
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

    assert( clt );

    SOCKETALT sockclt( clt->tcp_sock );

    Packet    packet;
    AddressIP ipadr;
    int recvbytes = sockclt.recvbuf( &packet, &ipadr );

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
		// NETFIXME: Cheat: We may want to check the serial of this packet and make sure it is what we expect.
		command = packet.getCommand( );
        COUT << "Received TCP" << Cmd(command) << ", ser=";
	if (clt->game_unit.GetUnit())
		COUT << clt->game_unit.GetUnit()->GetSerial();
	COUT << endl;
        if( clt )
        {
			this->updateTimestamps( clt, packet);
        }

	    this->processPacket( clt, command, ipadr, packet ); // In TCP we always process
    }
}

void NetServer::recvMsg_udp( )
{
//    SOCKETALT sockclt( udpNetwork->get_fd(), SOCKETALT::UDP, udpNetwork->get_adr(), _sock_set );
    ClientPtr clt;
	bool process = true;

    Packet    packet;
    AddressIP ipadr;
//    int ret = sockclt.recvbuf( &packet, &ipadr );
    int ret = udpNetwork.recvbuf( &packet, &ipadr );
    if( ret > 0 )
    {
        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

        COUT << "Received UDP" << Cmd(command) << ", ser=" << nserial << endl;

        // Find the corresponding client
        ClientPtr tmp;
		bool      found = false;
        for( LI i=allClients.begin(); i!=allClients.end(); i++)
        {
            tmp = (*i);
			// NETFIXME: Cheat: We have to check the address and port of the sender to make sure it matches that of our client socket.
            if( tmp->game_unit.GetUnit()->GetSerial() == nserial)
            {
                clt = tmp;
                found = 1;
//				COUT << " found client " << *(clt.get()) << endl;
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
        if (clt && (ipadr!=clt->cltudpadr))
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

