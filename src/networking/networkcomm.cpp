#include <config.h>

#ifndef NETCOMM_NOWEBCAM
#include "networking/webcam_support.h"
#endif
#ifndef NETCOMM_NOSOUND
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"
#endif

#include "networkcomm.h"

NetworkCommunication::NetworkCommunication()
{
#ifndef NETCOMM_NOSOUND
	this->session = NULL;
	this->params = NULL;
	this->rtpparams = NULL;
#endif
#ifndef NETCOMM_NOWEBCAM
	this->Webcam = NULL;
	string webcam_enable = vs_config->getVariable ("network","use_webcam","false");
	// Init the webcam part
	if( webcam_enable == "true")
	{
		Webcam = new WebcamSupport();
		if( Webcam->Init() == -1)
		{
			delete Webcam;
			this->Webcam = NULL;
		}
	}
#endif
}

void	NetworkCommunication::AddToSession( ClientPtr clt)
{
	this->commClients.push_back( clt);
}

void	NetworkCommunication::RemoveFromSession( ClientPtr clt)
{
	this->commClients.remove( clt);
}

string	NetworkCommunication::GrabImage()
{
	string jpeg_str( "");
#ifndef NETCOMM_NOWEBCAM
	if( Webcam)
	{
		//cerr<<"--- Trying to grab an image..."<<endl;
		//cerr<<"\t";
		jpeg_str = Webcam->CaptureImage();
		//cerr<<"--- grabbing finished"<<endl;
	}
#endif
	return jpeg_str;
}

int		NetworkCommunication::InitSession( float frequency)
{
	// Init the VoIP session
#ifndef NETCOMM_NOSOUND
	this->session = new JVOIPSession;
	this->params = new JVOIPSessionParams;
	this->rtpparams = new JVOIPRTPTransmissionParams;

	this->session.Create( this->params);
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
	this->session.Destroy();

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
	if( this->Webcam)
		ret = this->Webcam->isReady();
#endif
	return ret;
}

