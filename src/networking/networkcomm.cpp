#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif
#ifndef NETCOMM_NOSOUND
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#endif

#include "networkcomm.h"
#include "netbuffer.h"
#include "packet.h"

NetworkCommunication::NetworkCommunication()
{
#ifndef NETCOMM_NOSOUND
	this->session = NULL;
	this->params = NULL;
	this->rtpparams = NULL;
#endif
#ifndef NETCOMM_NOWEBCAM
	this->Webcam = NULL;
	// Init the webcam part
	Webcam = new WebcamSupport();
	if( Webcam->Init() == -1)
	{
		delete Webcam;
		this->Webcam = NULL;
	}
#endif
}

int		NetworkCommunication::GrabImage( ObjSerial serial)
{
#ifndef NETCOMM_NOWEBCAM
	string jpeg_str;
	if( Webcam)
	{
		//cerr<<"--- Trying to grab an image..."<<endl;
		//cerr<<"\t";
		jpeg_str = Webcam->CaptureImage();
		NetBuffer netbuf;
		netbuf.addString( jpeg_str);
		//cerr<<"--- grabbing finished"<<endl;
		// We send this capture to server which will redirect it to concerned clients
		Packet p;
		// We don't need that to be reliable in UDP mode
		p.send( CMD_CAMSHOT, serial, netbuf.getData(), netbuf.getDataLength(), SENDANDFORGET, NULL, this->clt_sock,
                      __FILE__, PSEUDO__LINE__(49) );
	}
#endif
	return 0;
}

int		NetworkCommunication::InitSession( float frequency)
{
	// Init the VoIP session
#ifndef NETCOMM_NOSOUND
	this->session = new JVOIPSession;
	this->params = new JVOIPSessionParams;
	this->rtpparams = new JVOIPRTPTransmissionParams;
#endif

#ifndef NETCOMM_NOWEBCAM
	if( Webcam)
		this->Webcam->StartCapture();
#endif

	this->freq = frequency;
	return 0;
}

int		NetworkCommunication::DestroySession()
{
#ifndef NETCOMM_NOSOUND
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
#endif
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
		this->Webcam->EndCapture();
#endif

	return 0;
}

NetworkCommunication::~NetworkCommunication()
{
#ifndef NETCOMM_NOSOUND
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
#endif
#ifndef NETCOMM_NOWEBCAM
	if( this->Webcam)
	{
		this->Webcam->Shutdown();
		delete this->Webcam;
		this->Webcam = NULL;
	}
#endif
}

bool	NetworkCommunication::WebcamEnabled()
{
	bool ret = false;
#ifndef NETCOMM_NOWEBCAM
	ret = (this->Webcam!=NULL);
#endif
	return ret;
}
bool	NetworkCommunication::WebcamTime()
{
	bool ret = false;
#ifndef NETCOMM_NOWEBCAM
	ret = this->Webcam->isReady();
#endif
	return ret;
}

