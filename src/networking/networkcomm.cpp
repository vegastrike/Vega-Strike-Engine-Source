#include "networking/webcam_support.h"
#include "networking/networkcomm.h"
#include "jvoipsession.h"
#include "jvoiprtptransmission.h"

NetworkCommunication::NetworkCommunication()
{
}

int		NetworkCommunication::InitWebcam()
{
	int	ret = 0;
	// GET VALUES FROM CONFIG SOON !
	Webcam = new WebcamSupport();
	if( (ret=Webcam->Init()) == -1)
		delete Webcam;

	return ret;
}

NetworkCommunication::~NetworkCommunication()
{
	if( Webcam != NULL)
		delete Webcam;
	if( session)
		delete session;
	if( params)
		delete params;
	if( rtpparams)
		delete rtpparams;
}

