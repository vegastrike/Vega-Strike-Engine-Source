#ifdef NETCOMM

#include "networking/networkcomm.h"

NetworkCommunication::NetworkCommunication()
{
}

void	NetworkCommunication::AddToSession( ClientPtr clt)
{
}

void	NetworkCommunication::RemoveFromSession( ClientPtr clt)
{
}

int		NetworkCommunication::InitSession( float freq)
{
	return 0;
}

/*
void	NetworkCommunication::SendImage( SOCKETALT & socket)
{
}
*/
void	NetworkCommunication::SendSound( SOCKETALT & socket, ObjSerial serial)
{
}
void	NetworkCommunication::SendMessage( SOCKETALT & socket, ObjSerial serial, string message)
{
}

int		NetworkCommunication::DestroySession()
{
	return 0;
}

NetworkCommunication::~NetworkCommunication()
{
}

char *	NetworkCommunication::GetWebcamCapture()
{
	return NULL;
}

char	NetworkCommunication::HasWebcam()
{
}

char	NetworkCommunication::HasPortaudio()
{
}

#endif

