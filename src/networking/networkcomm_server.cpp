#include "networking/networkcomm.h"

NetworkCommunication::NetworkCommunication() {}
NetworkCommunication::NetworkCommunication( float minfreq, float maxfreq, bool video, bool secured, string method) {}
NetworkCommunication::~NetworkCommunication() {}

void	NetworkCommunication::AddToSession( ClientPtr clt) {}
void	NetworkCommunication::RemoveFromSession( ClientPtr clt) {}
int		NetworkCommunication::InitSession( float freq) { return 0;}
int		NetworkCommunication::DestroySession() { return 0;}

//void	NetworkCommunication::SendImage( SOCKETALT & socket) {}
void	NetworkCommunication::SendSound( SOCKETALT & socket, ObjSerial serial) {}
void	NetworkCommunication::SendMessage( SOCKETALT & socket, ObjSerial serial, string message) {}
void	NetworkCommunication::RecvSound( char * sndbuffer, int length, bool encrypted) {}
void	NetworkCommunication::RecvMessage( string message, bool encrypted) {}

char *	NetworkCommunication::GetWebcamCapture() { return NULL;}
char *	NetworkCommunication::GetWebcamFromNetwork() { return NULL; }
char	NetworkCommunication::HasWebcam() { return 0;}
char	NetworkCommunication::HasPortaudio() { return 0;}
void	NetworkCommunication::SwitchWebcam() {}
void	NetworkCommunication::SwitchSecured() {}

void	NetworkCommunication::GenerateKey() {}
string	NetworkCommunication::EncryptBuffer( const char * buffer, unsigned int length) { return string("");}
string	NetworkCommunication::DecryptBuffer( const char * buffer, unsigned int length) { return string("");}

