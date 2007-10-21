#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_factory.h"
#include "networking/savenet_util.h"
#include "networking/prediction.h"
#include "networking/lowlevel/vsnet_sockethttp.h"
#include "lin_time.h"
#include "vs_random.h"
#include "save_util.h"

extern QVector DockToSavedBases( int n);
extern StarSystem * GetLoadedStarSystem( const char * system);

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

ClientPtr NetServer::addNewClient( SOCKETALT &sock )
{
    ClientPtr newclt( new Client( sock ) );
    // New client -> now registering it in thx active client list "Clients"
    // Store the associated socket

    allClients.push_back( newclt);

    COUT << " - Actual number of clients : " << allClients.size() << endl;

    return newclt;
}

void NetServer::broadcastUnit(Unit *un, unsigned short zone) {
	newUnits.push_back(un);
}
void NetServer::sendNewUnitQueue() {
	unsigned short vers[4]={0,4951,4952,65535};
	for (unsigned short zone=0;zone<zonemgr->getZoneNumber();zone++) {
		for (int verind = 0; verind<4; verind+=2) {
			unsigned short minver=vers[verind], maxver=vers[verind+1];
			NetBuffer netbuf;
			if (maxver!=65535) netbuf.setVersion(maxver);
			bool added=false;
			for (std::vector<UnitContainer>::iterator iter=newUnits.begin();
					 iter!=newUnits.end();
					 ++iter) {
				Unit *un = (*iter).GetUnit();
				if (un) {
					StarSystem *sys = un->getStarSystem();
					if (sys) {
						unsigned short unzone = sys->GetZone();
						if (unzone == zone) {
							added=true;
							UnitFactory::addBuffer(netbuf, un, true);
						}
					}
				} else {
					COUT << "New Unit already killed before being sent!!!" << endl;
				}
			}
			if (added) {
				UnitFactory::endBuffer(netbuf);
				Packet p2;
				p2.bc_create(CMD_ENTERCLIENT, 0, netbuf.getData(), netbuf.getDataLength(),
							 SENDRELIABLE, __FILE__, PSEUDO__LINE__(66));
				zonemgr->broadcast( zone, 0, &p2, true, minver, maxver);
				invalidateSnapshot();
			}
		}
	}
	newUnits.clear();
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

// **** Make sure to use Lock around system creation in ZoneMgr. ****

void	NetServer::addClient( ClientPtr clt)
{
	Unit * un = clt->game_unit.GetUnit();
	if (!un) {
		COUT<<"Error: adding NULL (dead) client! "<<clt->callsign<<endl;
		return;
	}
	COUT<<">>> SEND ENTERCLIENT =( serial #"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	Packet packet2,packet3;
	string savestr, xmlstr;
	NetBuffer netbuf;
	StarSystem * sts;
	StarSystem * st2;

	QVector nullVec( 0, 0, 0);
	int player = _Universe->whichPlayerStarship( un);
	if (player==-1)
		return;
	cerr<<"ADDING Player number "<<player<<endl;
	Cockpit * cp = _Universe->AccessCockpit(player);
	string starsys = cp->savegame->GetStarSystem();

	unsigned short zoneid;
	// If we return an existing starsystem we broadcast our info to others
	sts=zonemgr->addClient( clt, starsys, zoneid);

	//st2 = _Universe->getStarSystem( starsys);
	string sysfile = starsys+".system";
	st2 = GetLoadedStarSystem( sysfile.c_str());
	if( !st2)
	{
		cerr<<"!!! FATAL ERROR : star system '"<<starsys<<"' not found"<<endl;
		st2 = _Universe->GenerateStarSystem(sysfile.c_str(), "", Vector(0,0,0));
	}

	// On server side this is not done in Cockpit::SetParent()
	cp->activeStarSystem = st2;
	un->activeStarSystem = st2;
	// Cannot use sts pointer since it may be NULL if the system was just created
	// Try to see if the player is docked on start

	
        bool besafe=true;
	QVector safevec; // ( DockToSavedBases( player));
	Unit *dockedUnit = NULL;
        
        if (getSaveStringLength(player, "jump_from")>0) {
          std::string srcsys=getSaveString(player, "jump_from", 0);
          Unit * grav;
          for (un_iter ui=st2->gravitationalUnits().createIterator();
               (grav=*ui)!=NULL;
               ++ui) {
            size_t siz=grav->GetDestinations().size();
            for (unsigned int i=0;i<siz;++i) {
              if (srcsys==grav->GetDestinations()[i]) {
                cp->savegame->SetPlayerLocation(grav->LocalPosition()+QVector(vsrandom.uniformExc(-un->rSize(),un->rSize()),
                                                                              vsrandom.uniformExc(-un->rSize(),un->rSize()),
                                                                              vsrandom.uniformExc(-un->rSize(),un->rSize())
                                                                              ));
                besafe=grav->isUnit()!=PLANETPTR;
              }
            }
          }
          eraseSaveString(player, "jump_from", 0);
        } else {
			dockedUnit = DockToSavedBases(player, safevec);
		}
	if( !dockedUnit ) //safevec == nullVec)
	{
          if (besafe) {
		safevec = UniverseUtil::SafeStarSystemEntrancePoint( st2, cp->savegame->GetPlayerLocation(), clt->game_unit.GetUnit()->radial_size);
          }else{
            safevec=cp->savegame->GetPlayerLocation();
          }
		cerr<<"PLAYER NOT DOCKED - STARTING AT POSITION : x="<<safevec.i<<",y="<<safevec.j<<",z="<<safevec.k<<endl;
		clt->ingame   = true;
	}
	else
		cerr<<"PLAYER DOCKED TO " << dockedUnit->getFullname() << " - STARTING DOCKED AT POSITION : x="<<safevec.i<<",y="<<safevec.j<<",z="<<safevec.k<<endl;
	//COUT<<"\tposition : x="<<safevec.i<<" y="<<safevec.j<<" z="<<safevec.k<<endl;
	cp->savegame->SetPlayerLocation( safevec);
	
	cp->credits = (cp->savegame->GetSavedCredits());
	
	// UPDATE THE CLIENT Unit's state
	un->SetPosition( safevec);

	if( sts)
	{
		// DO NOT DO THAT HERE ANYMORE (VERY BLOCKING ON SERVER SIDE) -> CLIENT ASKS FOR THE NEW SYSTEM UNITS
		// AND DOWNLOADS INFO
		// Send info about other ships in the system to "clt"
		//zonemgr->sendZoneClients( clt);

		// Send savebuffers and name

		// Don't need to send ZoneMgr::AddClient
		// Uses packet's serial number
		netbuf.addChar(ZoneMgr::AddClient);
		netbuf.addSerial( un->GetSerial() );
		
		netbuf.addString( clt->callsign);
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr, false);
		netbuf.addString( savestr);
		netbuf.addString( xmlstr);
		netbuf.addTransformation( un->curr_physical_state );
		
		netbuf.addChar(ZoneMgr::End);
		// Put the save buffer after the ClientState
		packet2.bc_create( CMD_ENTERCLIENT, un->GetSerial(),
                           netbuf.getData(), netbuf.getDataLength(),
                           SENDRELIABLE,
                           __FILE__, PSEUDO__LINE__(1311));
		COUT<<"<<< SEND ENTERCLIENT("<<un->GetSerial()<<") TO OTHER CLIENT IN THE ZONE------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2, true);
		COUT<<"Serial : "<<un->GetSerial()<<endl;
	}
	// In all case set the zone and send the client the zone which it is in
	COUT<<">>> SEND ADDED YOU =( serial #"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	un->activeStarSystem->SetZone( zoneid);
	Packet pp;
	netbuf.Reset();
	netbuf.setVersion(clt->netversion);
	//netbuf.addShort( zoneid);
	//netbuf.addString( _Universe->current_stardate.GetFullTrekDate());
	un->BackupState();
    clt->setLatestTimestamp(packet.getTimestamp( ));
	clt->last_packet=un->old_state;
	clt->prediction->InitInterpolation(un, un->old_state, 0, clt->getNextDeltatime());
	// Add initial position to make sure the client is starting from where we tell him
	netbuf.addTransformation(un->curr_physical_state);
	pp.send( CMD_ADDEDYOU, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(170) );
	netbuf.Reset();
	getZoneInfo(un->activeStarSystem->GetZone(), netbuf);
	packet3.send( CMD_ENTERCLIENT, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(174) );

	Packet p2;
	netbuf.Reset();
	addUnitCargoSnapshot(un, netbuf);
	netbuf.addSerial(0);
	p2.bc_create( CMD_SNAPCARGO, 0, netbuf.getData(), netbuf.getDataLength(),
			SENDRELIABLE, __FILE__, PSEUDO__LINE__(179) );
	zonemgr->broadcast( un->activeStarSystem->GetZone(), un->GetSerial(), &p2, true ); 

	COUT<<"ADDED client n "<<un->GetSerial()<<" in ZONE "<<un->activeStarSystem->GetZone()<<" at STARDATE "<<_Universe->current_stardate.GetFullTrekDate()<<endl;
	sendCredits(un->GetSerial(), cp->credits);
	sendCargoSnapshot(un->GetSerial(), st2->getUnitList());
	if (dockedUnit) {
		int dockport=-1;
		for (unsigned int i=0;i<dockedUnit->image->dockedunits.size();++i) {
			Unit * tempun;
			if ((tempun=dockedUnit->image->dockedunits[i]->uc.GetUnit())!=NULL) {
				if (tempun==un) {
					dockport=i;
				}
			}
		}
		if (dockport>=0) {
			this->sendDockAuthorize( un->GetSerial(), dockedUnit->GetSerial(), dockport, un->activeStarSystem->GetZone());
		}
	}

	//delete cltsbuf;
	//COUT<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
}

void	NetServer::serverTimeInitUDP( ClientPtr clt, NetBuffer &netbuf)
{
	Packet p2;
	NetBuffer timeBuf;
	// If client sent an unsigned short.
	if (netbuf.getSize()>1) {
		unsigned short clt_port = netbuf.getShort();
		if (clt_port) {
			AddressIP adr (clt->cltadr, clt_port);
			clt->setUDP( udpNetwork, adr );
		} else {
			clt->setTCP();
		}
	}
	timeBuf.addDouble(queryTime()); // get most "up-to-date" time.
	// NETFIXME: is SENDANDFORGET really UDP? No.  Use the client's lossy_socket.
	p2.send( CMD_SERVERTIME, 0, timeBuf.getData(), timeBuf.getDataLength(), SENDANDFORGET, &clt->cltudpadr, *clt->lossy_socket, __FILE__, PSEUDO__LINE__(691) );
}

/***************************************************************/
/**** Removes a client and notify other clients                */
/***************************************************************/

void	NetServer::removeClient( ClientPtr clt)
{
	Packet packet2;
	Unit * un = clt->game_unit.GetUnit();
	if (!un)
		return; // Don't broadcast if already dead.
	clt->ingame = false;
	// Remove the client from its current starsystem
	zonemgr->removeClient( clt);
	// Broadcast to other players
	packet2.bc_create( CMD_EXITCLIENT, un->GetSerial(),
                       NULL, 0, SENDRELIABLE,
                       __FILE__, PSEUDO__LINE__(1311));
	zonemgr->broadcast( clt, &packet2, true );
}


Quaternion MinRotationFromDirections(Vector start, Vector finish, double &theta/*just for kicks*/) {
  Vector rotation=finish.Cross(start);
  double mag=rotation.Cast().Magnitude();
  if (mag>=1)
    mag=1;
  if (mag<1e-6) {
    theta=0;
    return identity_quaternion;
  }
  theta=asin(mag);
  if (start.Dot(finish)<0)theta+=M_PI/2;
  float s = (float)cos(theta*.5);
  rotation=rotation.Scale(1./mag);
  return Quaternion(s,rotation*sin(theta*.5));
}
Vector ApplyQuaternion(const Quaternion & quat, Vector input) {
  Matrix tmp;
  quat.to_matrix(tmp);
  return TransformNormal(tmp,input);
}
/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/
ClientState aim_assist(ClientState cs, ClientState ocs/*old*/,
                QVector realtargetpos,Vector realtargetvel, Vector targetpos, Vector targetvel) {
  float t=0;
  static bool debug_predict=XMLSupport::parse_bool(vs_config->getVariable("network","debug_prediction","false"));    
  double velaimtheta=0;
  double orientaimtheta=0;
  Matrix trans;
  ClientState OrientationAim(cs);
  ClientState VelocityAim(cs);
  bool VelocityAimValid=false;
  bool OrientationAimValid=false;
  cs.getOrientation().to_matrix(trans);
  Vector dir=trans.getR();
  targetvel=targetvel-cs.getVelocity();
  realtargetvel=realtargetvel-cs.getVelocity();
  Vector targetperp=targetpos.Cross(targetvel);
  realtargetpos=(realtargetpos-cs.getPosition());
  Vector realtargetperp=realtargetpos.Cast().Cross(realtargetvel);
  Vector targetdir=targetpos;
  targetdir.Normalize();
  float targetperpmag=targetperp.Magnitude();
  float realtargetperpmag=realtargetperp.Magnitude();
  static float cos_min_angle=acos(M_PI*(1./180)*XMLSupport::parse_float(vs_config->getVariable("network","max_lead_prediction_angle","60")));//120 degree leeway for using this method
  if (targetperpmag>.001&&realtargetperpmag>.001&&targetvel.MagnitudeSquared()>.25&&realtargetvel.MagnitudeSquared()>.25&&dir.Dot(targetdir)>=cos_min_angle) {//gotta make sure target is at least in viewscreen before giving benefit of the doubt lead
    targetperp=targetperp*(1./targetperpmag);
    realtargetperp=realtargetperp*(1./realtargetperpmag);// compute a unit direction perpendicular to the plane made by the target and its relative velocity (to playerstarship)... we'll use this to see how far off that plane the velocity actually is
    float distoffplane=targetperp.Dot(dir);
    Vector dirperp = targetperp.Scale(distoffplane);
    Vector dirplane = dir-dirperp;// this is the projection of the player's orientation direction onto the plane if the target and its velocity... if he were aligned with the arc of travel of the target he would Dot(targetperp,dir) would be zero and this wouldn't change the dir
    //now we have to figure out how far out the intersection point of dirplane and targetvel are...
    // solve for t and k    reltargetpos+targetvel*t=k*dirplane
    float posx=targetpos.i;
    float posy=targetpos.j;
    float dirx=dirplane.i;
    float diry=dirplane.j;
    float velx=targetvel.i;
    float vely=targetvel.j;
    if (fabs(dirplane.i)<=fabs(dirplane.j)&&fabs(dirplane.i)<=fabs(dirplane.k)) {
      posx=targetpos.k;
      posy=targetpos.j;
      dirx=dirplane.k;
      diry=dirplane.j;
      velx=targetvel.k;
      vely=targetvel.j;
    }
    if (fabs(dirplane.j)<=fabs(dirplane.i)&&fabs(dirplane.j)<=fabs(dirplane.k)) {
      posx=targetpos.i;
      posy=targetpos.k;
      dirx=dirplane.i;
      diry=dirplane.k;
      velx=targetvel.i;
      vely=targetvel.k;
    }
    if (fabs(diry)>fabs(dirx)) {
      float tmp=posx;
      posx=posy;
      posy=tmp;
      tmp=dirx;
      dirx=diry;
      diry=tmp;
      tmp=velx;
      velx=vely;
      vely=tmp;
    }

    {
      float num=diry*posx/dirx-posy;
      float denom=vely-diry*velx/dirx;
      if (fabs(denom)<.0001) {
        t=0;
      }else {
        t=num/denom;
      }
      //      if (t<=0) t=0;
    }
    static float velaimlim=XMLSupport::parse_float(vs_config->getVariable("network","max_lead_prediction","10"));
    if (fabs(t)<velaimlim) {
      Vector newdirplane=realtargetpos+realtargetvel*t;
      Vector newdirperp=realtargetperp.Scale(distoffplane);
      newdirplane.Normalize();
      newdirplane*=dirplane.Magnitude();
      Vector newdir = newdirplane+newdirperp;
      newdir.Normalize();
      Vector newright,newup;
      CrossProduct(trans.getQ(),newdir,newright);
      CrossProduct(newdir,newright,newup);
      newright.Normalize();
      newup.Normalize();
      Quaternion forient=Quaternion::from_vectors(newright,newup,newdir);
      Vector fvel=ApplyQuaternion(MinRotationFromDirections(dir,newdir,velaimtheta),cs.getAngularVelocity());
      VelocityAimValid=FINITE(fvel.i)&&FINITE(fvel.j)&&FINITE(fvel.k)&&FINITE(forient.s)&&FINITE(forient.v.i)&&FINITE(forient.v.j)&&FINITE(forient.v.k);
      VelocityAim.setAngularVelocity(fvel);      
      VelocityAim.setOrientation(forient);

    }
  }
  {
    //some other scheme for when target is still...
    targetpos.Normalize();
    realtargetpos.Normalize();
    Quaternion quat=MinRotationFromDirections(targetpos,realtargetpos,orientaimtheta);
    Quaternion forient=cs.getOrientation()*quat;
    Vector fvel=ApplyQuaternion(quat,cs.getAngularVelocity());

    OrientationAimValid=FINITE(fvel.i)&&FINITE(fvel.j)&&FINITE(fvel.k)&&FINITE(forient.s)&&FINITE(forient.v.i)&&FINITE(forient.v.j)&&FINITE(forient.v.k);    
    OrientationAim.setOrientation(forient);
    OrientationAim.setAngularVelocity(fvel);

  }

  if (VelocityAimValid){
    if (debug_predict)
      printf ("Velocity aim lead %f, rot %f deg\n",t,velaimtheta*180/M_PI);
    return VelocityAim;
  }
  if (OrientationAimValid) {
    if (debug_predict)
      printf ("Using orientation aim rot %f deg\n",orientaimtheta*180/M_PI);
    return OrientationAim;
  }
  printf("Using raw aim\n");
  return cs;
}
ClientState aim_assist(ClientState cs, ClientState ocs/*old*/,
                Unit * target, Vector targetpos, Vector targetvel) {
  return aim_assist(cs,ocs,target->Position(),target->Velocity,targetpos,targetvel);
}
ClientState aim_assist_debug(float x,float y,float z,
                             float vx,float vy,float vz,
                             float rx,float ry,float rz,
                             float vrx,float vry, float vrz,
                             float forex, float forey, float forez) {

  Vector fore(forex,forey,forez);
  fore.Normalize();
  Vector up(0,1,0);
  if (fore.j>.9) {
    up=Vector(0,0,1);
  }
  Vector right;
  CrossProduct(up,fore,right);
  CrossProduct(fore,right,up);
  right.Normalize();
  up.Normalize();
  fore.Normalize();
  ClientState tmp(1,QVector(0,0,0),Quaternion::from_vectors(right,up,fore),Vector(0,0,0),Vector(0,0,0),Vector(0,1,0));
  tmp=aim_assist(tmp,tmp,QVector(rx,ry,rz),Vector(vrx,vry,vrz),Vector(x,y,z),Vector(vx,vy,vz));
  Matrix mat;
  tmp.getOrientation().to_matrix(mat);
  printf ("Final Direction %.3f %.3f %.3f (Up %.2f %.2f %.2f)\n",mat.getR().i,mat.getR().j,mat.getR().k,mat.getQ().i,mat.getQ().j,mat.getQ().k);
  return tmp;
}
                             
void	NetServer::posUpdate( ClientPtr clt)
{


//NETFIXME: do a sanity check on the position.



  NetBuffer netbuf( packet.getData(), packet.getDataLength());
  Unit * un = clt->game_unit.GetUnit();
  if (!un)
    return; // Don't receive data if dead.
  ObjSerial clt_serial = netbuf.getSerial();
  //   clt->setLatestTimestamp(packet.getTimestamp( ));
  clt->setLatestTimestamp(packet.getTimestamp( ));
  clt->elapsed_since_packet = 0;
  if( clt_serial != un->GetSerial())
  {
    cerr<<"!!! ERROR : Received an update from a serial that differs with the client we found !!!"<<endl;
    //			VSExit(1);
  }
  ClientState cs2;
  // Set old position
  un->BackupState();
  // Update client position in client list : should be enough like it is below
  cs2 = netbuf.getClientState();
  ClientState cs(cs2);
  static bool do_aim_assist=XMLSupport::parse_bool(vs_config->getVariable("network","aim_assist","true"));
  if (do_aim_assist&&netbuf.getSize()>netbuf.getOffset()+2) {
    ObjSerial target_serial=netbuf.getSerial();
    if (target_serial!=0) {
      unsigned int zone = un->getStarSystem()->GetZone();
      Unit * target=zonemgr->getUnit( target_serial, zone);
      Vector rel_target_position=netbuf.getVector();
      QVector target_position=cs.getPosition()+rel_target_position;
      Vector target_velocity=netbuf.getVector();
      if (target) {
        cs=aim_assist(cs,clt->last_packet,target,rel_target_position,target_velocity);
      }
    }
  }
  static bool debugPos = XMLSupport::parse_bool(vs_config->getVariable("network", "debug_position_interpolation", "false"));
  if (debugPos) COUT<<"Received ZoneMgr::PosUpdate from client "<<clt_serial<<"   *** cs="<<cs<< endl;
  un->curr_physical_state.position = cs.getPosition();
  un->curr_physical_state.orientation = cs.getOrientation();
  un->Velocity = cs.getVelocity();
  un->AngularVelocity=cs.getAngularVelocity();
  assert( clt->prediction );
  clt->prediction->InitInterpolation( un, clt->last_packet, clt->getDeltatime(), clt->getNextDeltatime());
  //	un->curr_physical_state.position = clt->prediction->InterpolatePosition( un, 0);
  clt->last_packet=cs;
  // deltatime has already been updated when the packet was received
  Cockpit * cp = _Universe->isPlayerStarship( un );
  if (cp)
    cp->savegame->SetPlayerLocation( un->curr_physical_state.position);
  else if (debugPos)
    COUT << "ERROR: Client has NULL cockpit. Does he have two clients ingame? ser="<< un->GetSerial() << ", callsign="<<clt->callsign << endl;
  snapchanged = 1;
}

/**************************************************************/
/**** Disconnect a client                                  ****/
/**************************************************************/
void AddWriteSave(std::string &netbuf, int cpnum){
  string savestr, xmlstr;
  SaveNetUtil::GetSaveStrings( cpnum, savestr, xmlstr, true);
  addSimpleString(netbuf, savestr);
  addSimpleString(netbuf, xmlstr);
}
void AcctLogout(VsnetHTTPSocket* acct_sock,ClientPtr clt) {
  if (acct_sock==NULL) return;
  if (clt) {
    std::string netbuf;
    
    Unit * un = clt->game_unit.GetUnit();
	int cpnum = _Universe->whichPlayerStarship(un);
    Cockpit * cp = cpnum==-1?NULL:_Universe->AccessCockpit(cpnum);
    bool dosave= false; //(cp!=NULL&&un!=NULL&&_Universe->star_system.size()>0&&cp->activeStarSystem&&clt->jumpok==0);
    addSimpleChar(netbuf,dosave?ACCT_SAVE_LOGOUT:ACCT_LOGOUT);
    addSimpleString(netbuf, clt->callsign );
    addSimpleString(netbuf, clt->passwd );
    if (dosave) {
      AddWriteSave(netbuf,cpnum);
    }
    if (!acct_sock->sendstr(netbuf)) {
      COUT<<"ERROR sending LOGOUT to account server"<<endl;
    }
  }
}
void	NetServer::disconnect( ClientPtr clt, const char* debug_from_file, int debug_from_line )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
         << " *** from " << debug_from_file << ":" << debug_from_line << endl
         << " *** disconnecting " << clt->callsign << " because of "
         << clt->_disconnectReason << endl;
	
	logout(clt);
}

/*** Same as disconnect but do not respond to client since we assume clean exit ***/
void	NetServer::logout( ClientPtr clt )
{
	Packet p, p1, p2;
	std::string netbuf;
	Unit * un = clt->game_unit.GetUnit();
	std::string callsign = clt->callsign;

	if (clt->loginstate==Client::WAITLISTED) {
		std::map<std::string, WaitListEntry>::iterator iter = waitList.find(clt->callsign);
		if( waitList.end()!=iter) {
			waitList.erase(iter);
		}
		clt->loginstate=Client::CONNECTED;
	}
	if( acctserver && clt->loginstate >= Client::LOGGEDIN) {
		AcctLogout(acct_sock,clt);
	}

	clt->tcp_sock.disconnect( __PRETTY_FUNCTION__ );
	COUT <<"Client "<<clt->callsign<<" disconnected"<<endl;
	COUT <<"There were "<< allClients.size() <<" clients - ";
	allClients.remove( clt );
	// Removes the client from its starsystem
	this->removeClient( clt );
	// Say true as 2nd arg because we don't want the server to broadcast since player is leaving hte game
	if( un)
		un->Kill( true, true);

	if (clt->loginstate >= Client::LOGGEDIN) {
		for (int i=0;i<_Universe->numPlayers();i++) {
			Cockpit *cp = _Universe->AccessCockpit(i);
			if (cp->savegame && cp->savegame->GetCallsign() == callsign) {
				cp->savegame->SetCallsign("");
				unused_players.push(i);
			}
		}
	}
	
	clt.reset( );
	COUT << allClients.size() <<" clients left"<<endl;
	nbclients--;
}

/************************************************************************************************/
/**** getZoneClients : returns a buffer containing zone info                                *****/
/************************************************************************************************/

// Send one by one a CMD_ADDLCIENT to the client for every ship in the star system we enter
void  NetServer::getZoneInfo( unsigned short zoneid, NetBuffer & netbuf)
{
	LI k;
	int nbclients=0;
	Packet packet2;
	string savestr, xmlstr;

	// Loop through client in the same zone to send their current_state and save and xml to "clt"
        std::set<ObjSerial> activeObjects;
        ClientList* lst = zonemgr->getZoneList(zoneid);
        if( lst == NULL )
        {
          COUT << "\t>>> WARNING: Did not send info about " << nbclients << " other ships because of empty (inconsistent?) zone" << endl;
        }
        else
	for( k=lst->begin(); k!=lst->end(); k++)
	{
        ClientPtr kp( *k );

		// Test if *k is the same as clt in which case we don't need to send info
		if( true ) // kp->ingame)
		{
			Unit *un = kp->game_unit.GetUnit();
			if (!un)
				continue;
			SaveNetUtil::GetSaveStrings( kp, savestr, xmlstr, false);
			// Add the ClientState at the beginning of the buffer -> NO THIS IS IN THE SAVE !!
			//netbuf.addClientState( ClientState( kp->game_unit.GetUnit()));
			// Add the callsign and save and xml strings
			netbuf.addChar( ZoneMgr::AddClient);
			netbuf.addSerial( un->GetSerial());
			netbuf.addString( kp->callsign);
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			netbuf.addTransformation(kp->game_unit.GetUnit()->curr_physical_state);
                        activeObjects.insert(un->GetSerial());
			nbclients++;
		}
	}
        Unit *un;
        for (un_iter ui=_Universe->star_system[zoneid]->getUnitList().createIterator();
             (un=*ui)!=NULL;
             ++ui) {
          ObjSerial ser=un->GetSerial();
          if (activeObjects.find(ser)==activeObjects.end()) {
            UnitFactory::addBuffer(netbuf, un, false);
            activeObjects.insert(un->GetSerial());
            nbclients++;
       
          }
        }
        netbuf.addChar( ZoneMgr::End);
          
	COUT<<"\t>>> GOT INFO ABOUT "<<nbclients<<" OTHER SHIPS"<<endl;
}
