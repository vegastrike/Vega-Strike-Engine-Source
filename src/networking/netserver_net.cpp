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
	ClientPtr ret;
	return ret; // This function doesn't quite make sense.
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
#ifdef TIMEOUT_USING_UDP_EVEN_THOUGH_THEY_USE_TCP
			    if( cl->ingame==true && deltatmp > clienttimeout)
			    {
	                Unit * un;
				    un = cl->game_unit.GetUnit();
				    COUT<<"ACTIVITY TIMEOUT for client number ";
					if (un)
						cout<<un->GetSerial();
					else
						cout << "Dead unit";
					cout<<endl;
				    COUT<<"\t\tCurrent time : "<<curtime<<endl;
				    COUT<<"\t\tLatest timeout : "<<(cl->latest_timeout)<<endl;
				    COUT<<"t\tDifference : "<<deltatmp<<endl;
                                    cl->_disconnectReason = "UDP timeout";
                                    static bool dodisc=XMLSupport::parse_bool(vs_config->getVariable("server", "disconnect_client_on_timeout", "true"));
                                    if (dodisc)
                                      discList.push_back( cl );
					// NETFIXME: Should we actually disconnect them in a UDP timeout or should we just fallback to TCP?
			    }
#endif
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
		if (command!=CMD_POSUPDATE && command!=CMD_TARGET) {
			COUT << "Rcvd TCP: " << Cmd(command) << " from serial ";
			if (clt->game_unit.GetUnit())
				cout << clt->game_unit.GetUnit()->GetSerial();
			if (!clt->ingame)
				cout << " (not ingame)";
			cout << endl;
		}
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
    int ret = udpNetwork->recvbuf( &packet, &ipadr );
    if( ret > 0 )
    {
        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

		if (command!=CMD_POSUPDATE)
        COUT << "Rcvd UDP: " << Cmd(command) << " from serial " << nserial << endl;

        // Find the corresponding client
        ClientPtr tmp;
		bool      found = false;
        for( LI i=allClients.begin(); i!=allClients.end(); i++)
        {
            tmp = (*i);
			Unit *myun = tmp->game_unit.GetUnit();
			// NETFIXME: Cheat: We have to check the address and port of the sender to make sure it matches that of our client socket.
            if( myun && myun->GetSerial() == nserial)
            {
                clt = tmp;
                found = 1;
//				COUT << " found client " << *(clt.get()) << endl;
				break;
            }
        }
        if( !found )
        {
            COUT << "Error : UDP message received from unknown client !" << endl;
            // Maybe send an error packet handled by the client
            return;
        }

        // Check if the client's IP is still the same (a very little and unaccurate in some cases protection
		// against spoofing client serial#)
        if (clt && (ipadr!=clt->cltudpadr))
        {
			
			COUT << "Error : IP changed for client " << clt->callsign << endl;
//            clt->_disconnectReason = "possible IP spoofing";
//            discList.push_back( clt );
	    	/* It is not entirely impossible for this to happen; it would be nice
			 * to add an additional identity check. For now we consider it an error.
	     	*/
        }
        else
        {
	    	if (command == CMD_LOGIN || command == CMD_CONNECT) {
				// clt should be 0 because ObjSerial was 0
				return;
			}
			// NETFIXME: Only allow lossy packets!!!
			if (command != CMD_SNAPSHOT && command != CMD_PING &&
				command != CMD_POSUPDATE && command != CMD_SERVERTIME) {
				return;
			}
            if( clt )
				process = this->updateTimestamps( clt, packet);

			// Do not process a packet considered to be late and not important (positions and damage ?)
			if( process)
            	this->processPacket( clt, command, ipadr, packet );
        }
    }
}

