#include "networking/webcam_support.h"
#include "networking/networkcomm.h"
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"

NetworkCommunication::NetworkCommunication()
{
}

int		NetworkCommunication::InitWebcam()
{
	return 0;
}

int		NetworkCommunication::InitSession( float frequency)
{
	// Init the VoIP session
	this->session = new JVOIPSession;
	this->params = new JVOIPSessionParams;
	this->rtpparams = new JVOIPRTPTransmissionParams;

	// Init the webcam part
	int	ret = 0;
	// GET VALUES FROM CONFIG SOON !
	Webcam = new WebcamSupport();
	if( (ret=Webcam->Init()) == -1)
		delete Webcam;

	return ret;
}

int		NetworkCommunication::DestroySession()
{
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
	if( this->Webcam)
		delete this->Webcam;

	return 0;
}

NetworkCommunication::~NetworkCommunication()
{
	if( this->session)
		delete this->session;
	if( this->params)
		delete this->params;
	if( this->rtpparams)
		delete this->rtpparams;
	if( this->Webcam)
		delete this->Webcam;
}

