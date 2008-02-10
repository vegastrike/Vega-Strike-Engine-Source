#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "vsfilesystem.h"
#include "networking/savenet_util.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/fileutil.h"
#include "networking/lowlevel/vsnet_sockethttp.h"
extern int acct_con;

/**************************************************************/
/**** Check account server activity                        ****/
/**************************************************************/

void	NetServer::checkAcctMsg( SocketSet& sets )
{

  
	AddressIP	ipadr;
	ClientPtr   clt;
	unsigned char cmd=0;
	// Watch account server socket
	// Get the number of active clients
	if( !acct_sock || acct_sock->isActive( ))
	{
		//COUT<<"Net activity !"<<endl;
		// Receive packet and process according to command

		std::string    p;

		if( acct_sock && acct_sock->recvstr( p)!=0 &&p.length()!=0)
		{
			// Maybe copy that in a "else" condition too if when it fails we
            // have to disconnect a client

			// Here we get the latest client which asked for a login
			// Since coms between game servers and account server are TCP
            // the order of request/answers
			// should be ok and we can use a "queue" for waiting clients
			if (!p.empty()) cmd = getSimpleChar(p);
			string ptemp (p);
			string username = getSimpleString(ptemp);
			string server, port;
			if (cmd == ACCT_SUCCESS) {
				server = getSimpleString(ptemp);
				port = getSimpleString(ptemp);
			}
			std::map<std::string, WaitListEntry>::iterator iter = waitList.find(username);
			if( waitList.end()!=iter)
			{
				WaitListEntry entry( (*iter).second );
				char flags = 0;
				if( entry.tcp )
				{
					clt = entry.t;
					COUT << "Got response for TCP client" << endl;
				}
				else
				{
					ipadr = entry.u;
					COUT << "Got response for client IP : " << ipadr << endl;
				}
					
				if (entry.type == (int)WaitListEntry::CONNECTING && cmd!=ACCT_SUCCESS && cmd != 0) {
				waitList.erase(iter);
					
				if (clt) clt->loginstate=Client::CONNECTED;
				ObjSerial serial =0;
			switch(cmd)
			{
				case ACCT_LOGIN_NEW :
					COUT << ">>> NEW LOGIN =( serial #"<<serial<<" )= --------------------------------------"<<endl;
					// We received a login authorization for a new account (no ship created)
					this->loginAccept(p, clt, 1, flags);
					COUT << "<<< NEW LOGIN ----------------------------------------------------------------"<<endl;
				break;
				case ACCT_LOGIN_ACCEPT :
					// Login is ok
					COUT<<">>> LOGIN ACCEPTED =( serial #"<<serial<<" )= --------------------------------------"<<endl;
					loginAccept(p, clt, 0, flags);
					COUT<<"<<< LOGIN ACCEPTED -----------------------------------------------------------"<<endl;
				break;
				case ACCT_LOGIN_ERROR :
					COUT<<">>> LOGIN ERROR =( DENIED )= --------------------------------------"<<endl;
					// Login error -> disconnect
					this->sendLoginError( clt);
					COUT<<"<<< LOGIN ERROR ---------------------------------------------------"<<endl;
				break;
				case ACCT_LOGIN_ALREADY :
					COUT<<">>> LOGIN ALREADY =( ALREADY LOGGED IN -> serial #"<<serial<<" )= --------------------------------------"<<endl;
					// Client already logged in -> disconnect
					this->sendLoginAlready( clt);
					COUT<<"<<< LOGIN ALREADY --------------------------------------------------------------"<<endl;
				break;
				default:
					COUT<<">>> UNKNOWN COMMAND =( "<<(unsigned int)cmd<<" )= --------------------------------------"<<endl<<"Full datastream was:"<<p<<endl;
			}
			} else if (entry.type == (int)WaitListEntry::JUMPING) {
					waitList.erase(iter);
					sendJumpFinal(clt, server, (unsigned short)(atoi(port.c_str())));
				}
			}
		}
		else
		{
			cerr<<"Connection to account server lost !!"<<endl;
//			acct_con = 0;
		}
	}
}

/**************************************************************/
/**** Save the server state                                ****/
/**************************************************************/

// For now it only save units and player saves
void	NetServer::save()
{
	using namespace VSFileSystem;

	Packet pckt;
	Cockpit * cp;
	Unit * un;
	FILE * fp=NULL;
	string xmlstr, savestr;
	//unsigned int xmllen, savelen, nxmllen, nsavelen;
	NetBuffer netbuf;

	// Save the Dynamic Universe in the data dir for now
	string dynuniv_path = "dynaverse.dat";
	VSFile f;
	VSError err = f.OpenCreateWrite( dynuniv_path, ::VSFileSystem::UnknownFile);
	if( err>Ok)
	{
		cerr<<"FATAL ERROR: Error opening dynamic universe file"<<endl;
	}
	else
	{
		string dyn_univ = globalsave->WriteDynamicUniverse();
		f.Write( dyn_univ);
		f.Close();
	}

	// Loop through all clients and write saves
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		saveAccount(i);
	}
}

bool NetServer::saveAccount(int i)
{
	string xmlstr, savestr;
	//unsigned int xmllen, savelen, nxmllen, nsavelen;
	Cockpit *cp = _Universe->AccessCockpit( i);

	// Write the save and xml unit
	//FileUtil::WriteSaveFiles( savestr, xmlstr, VSFileSystem::datadir+"/serversaves", cp->savegame->GetCallsign());
	// SEND THE BUFFERS TO ACCOUNT SERVER
	if ( cp && acctserver && acct_con)
	{
		Unit *un=cp->GetParent();
		ClientPtr clt;
		if (un) {
			clt=getClientFromSerial( un->GetSerial());
		}
		if( !clt || !un )
		{
			cerr<<"Error client/unit for "<<(clt?clt->callsign:"")<<", serial "<<(un?un->GetSerial():0)<<" not found in save process !!!!"<<endl;
			return false;
		}
		if (clt->loginstate < Client::INGAME) {
			return false; // Cannot save at this point.
		}
		SaveNetUtil::GetSaveStrings( i, savestr, xmlstr, true);
		if (savestr.empty() || xmlstr.empty()) {
			//cerr<<"Unable to generate CSV and Save data for player."<<endl;
			return false;
		}
		std::string snetbuf;
		bool found = false;
		// Loop through clients to find the one corresponding to the unit (we need its serial)
		// Fix CMD_RESPAWN.  I expect the client to do the same thing here.
		clt->savegame.clear();
		clt->savegame.push_back(savestr);
		clt->savegame.push_back(xmlstr);
		
		addSimpleChar(snetbuf,ACCT_SAVE);
		addSimpleString(snetbuf,clt->callsign);
		addSimpleString(snetbuf,clt->passwd);
		addSimpleString(snetbuf, savestr);
		addSimpleString(snetbuf, xmlstr);
		if (!acct_sock->sendstr(snetbuf)) {
			//buffer = new char[savestr.length() + xmlstr.length() + 2*sizeof( unsigned int)];
			//SaveNetUtil::GetSaveBuffer( savestr, xmlstr, buffer);
			COUT<<"ERROR sending SAVE to account server for "<<clt->callsign<<" ("<<un->GetSerial()<<")"<<endl;
			return false;
		}
		{
			// Tell client that we saved the game.
			Packet p1;
			NetBuffer netbuf;
			p1.send(CMD_SAVEACCOUNTS, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(),
				SENDRELIABLE, NULL, clt->tcp_sock, __FILE__, __LINE__);
		}
		return true;
	}
	return false;
}
