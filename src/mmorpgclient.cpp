#include "command.h"
#include "mmorpgclient.h"
#include "SDL/SDL.h"
#include "SDL_thread.h"
#include "vegastrike.h"
#include "vs_globals.h"

#ifdef WIN32
	#include <winsock.h>     // For socket(), connect(), send(), and recv()
typedef int socklen_t;
#else
	#include <sys/types.h>   // For data types
	#include <sys/socket.h>  // For socket(), connect(), send(), and recv()
	#include <netdb.h>       // For gethostbyname()
	#include <arpa/inet.h>   // For inet_addr()
	#include <unistd.h>      // For close()
	#include <netinet/in.h>  // For sockaddr_in
#endif

//This is created in command.cpp with single like (search for mmoc)
//If you wish to disable this module, just comment out that single line in command.cpp,
//and don't compile this .cpp file. 

extern commandI CommandInterpretor;
mmoc::mmoc() { // {{{
	status = false; // used to let the thread exit
//	if( (socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
//		RText::conoutf("Error creating the socket. Will continue, but your game will\n likely spew if you try to connect to a host.");
//	}
	//add the connectto to the players command interp.
	cmd = new Functor<mmoc>(this, &mmoc::connectTo);
	CommandInterpretor.addCommand(cmd, "connectto", ARG_2CSTR);
	// {{{
	//add commands here to be parsed from network input.
	//some notes:
	//just make them like normal, and the word "FOO" will be the "tag" that triggers  it.
	// eg: the server sends <FOO>args</FOO>, the input routines will parse it to:
	// FOO , and try sending it through the LOCAL command processor (NOT the main one used for the player console).

	//now some notes more to myself
	// The users console at the highest level (in the main ship rendering routines) will need to gather data from the mmoc console to render it there.
	// and never call mmoc::renderconsole. I'll need to make a mutex'd function for this.
	 // }}}
}; // }}}
void mmoc::connectTo(const char *address_in, const char *port_in) { // {{{
	sockaddr_in m_addr;
	char *address = (char *)address_in;
	char *port = (char *)port_in;
	if(address == NULL) {
		std::cout << "Need a host at least, a host and port at most!" << std::endl;
		return;
	}
	if(port == NULL) {
		port = "5555";
	}

	std::string test;
	test.append(address);
	if(test.compare("default") == 0) {
		address = "ant.infice.com";
		port = "5555";
	}
    if( (socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        RText::conoutf("Error creating the socket. Will continue, but your game will\n likely spew if you try to connect to a host.");
    }

	hostent *server;  // Resolve name
	if ((server = gethostbyname(address)) == NULL) {
		CommandInterpretor.conoutf("Error, couldn't find host");
		// strerror() will not work for gethostbyname() and hstrerror() 
		// is supposedly obsolete
	return;
	}

	bzero((char *) &m_addr, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		(char *)&m_addr.sin_addr.s_addr,
		server->h_length);
	m_addr.sin_port = htons(atoi(port));
	int status = 0;

	if ( (status = ::connect(socket,reinterpret_cast<sockaddr *>(&m_addr),sizeof(m_addr))) < 0) {
		CommandInterpretor.conoutf("Couldn't Connect\n");
		return;
	}
	std::string hellomsg;
	hellomsg.append("Vegastrike-user");
	send(hellomsg);
	cl = new Functor<mmoc>(this, &mmoc::close);
	CommandInterpretor.addCommand(cl, "disconnect", ARG_NONE);
	
    csay = new Functor<mmoc>(this, &mmoc::send);
    CommandInterpretor.addCommand(csay, "send", ARG_1STR);

	createThread(); 
} // }}}
bool mmoc::getStatus(int in) { // {{{
// if in = 0, return status, else toggle status.
SDL_mutex * m = SDL_CreateMutex();
SDL_mutexP(m);
	bool returner;
	if(in == 0) returner = status;
	else {
		if(status) status = false;
		else status = true;
	}
SDL_mutexV(m);
SDL_DestroyMutex(m);
	return returner;
} // }}}
void mmoc::ParseRemoteInput(char *buf) { // {{{ Main parser	
//add  binary modes here.
	if(buf != NULL) 
		start.append(buf);
	size_t teststart;
	if( (teststart = start.find("\n")) < std::string::npos) { //we have a full string and are ready to process
		if( teststart < start.size()-2 ) {
			for(unsigned int soda = teststart+1; soda < start.size(); soda++) 
			{
				tempstr += start[soda];
			}
			while( start.size() > teststart ) {
				std::string::iterator iter = start.end();
				iter--;
				start.erase(iter); //pop goes the weasle
			}
		}
	} else {
//		return;
	}
	unsigned int end = start.size(); 
	std::string hackedup;
	bool open = false, ignore2close = false;
	unsigned int fopend = 0;
	unsigned int counter;
	//parse commands to be executed <FOO=BAR>
	for(counter = 0; counter < end; counter++) {
		if(open) {
			if(start[counter] == '=') hackedup.append("\" ");
			else if(start[counter] == '>') {
				open = false;
				start.replace(fopend, (counter+1)-fopend, "");
				unsigned int offset = (counter+1) - fopend;
				counter = counter - offset;
				commandI::execute(&hackedup, true, socket);
				hackedup.erase();
				end = start.size();
			} else{ 
				hackedup += start[counter];
			}
		} else {
			if(start[counter] == '<') { 
				open = true;
				fopend = counter;
				hackedup.append("\"");
			}
		}
	}
	//optimize by moving this above.
	unsigned int ender = start.size();
	for(counter = 0; counter < ender; counter++) { //remove \r's
		if(start[counter] == '\r') { 
			start.replace(counter, 1, "");
			counter = counter -2;
			ender = start.size();
		}
	}
	CommandInterpretor.conoutf(start); //print what's left to the console

	start.erase();
	if(tempstr.size() > 0) {
		start.append(tempstr);
		tempstr.erase();
	}
	{
		size_t asdf;
		while( (asdf = start.find("\n")) < std::string::npos) ParseRemoteInput(NULL);
//		if(start.size() > 0 ) {
//			start.append("\n\r");
//			ParseRemoteInput(NULL);
//		}
	}
} // }}}
bool mmoc::listenThread() { // {{{
	const int MAXBUF=1000;
	char buffer[MAXBUF+1];
	bool stat;
	while( (stat = getStatus(0)) == true ) {
		bzero(buffer, MAXBUF);
	    if( ::read(socket, buffer, sizeof(buffer)-1) <= 0 ) { 
			getStatus(1); //1 toggles status, 0 reads status
			return false;
		} else {
			ParseRemoteInput(buffer);
		}
	}
    /*---Clean up---*/
	return stat;
} // }}}
void mmoc::createThread() { // {{{
	::SDL_CreateThread(startThread, reinterpret_cast<void *>(this));
} // }}}
void mmoc::send(char *buffer, int size) { // {{{
	::send(socket, buffer, size, 0); //or write(socket, buffer, size) for windwos?
} // }}}
void mmoc::send(std::string &instring) { // {{{
	unsigned int x = instring.find("send ");
	if(x = 0) instring.replace(0, 5, "");
	instring.append("\r\n");
	send( (char *)instring.c_str(), instring.size() );
} // }}}

int startThread(void *mmoc2use) { // {{{
	mmoc *looper = reinterpret_cast<mmoc *>(mmoc2use);
	if(!looper->getStatus(0)) looper->getStatus(1);
	CommandInterpretor.conoutf("Connected.");
	while(looper->listenThread());

	::close(looper->getSocket());
	CommandInterpretor.remCommand(looper->csay);
	CommandInterpretor.remCommand(looper->cl);
	CommandInterpretor.conoutf("Closed");
	if(looper->getStatus(0)) looper->getStatus(1);
	return 0;
} // }}}
void mmoc::close() {
	if(getStatus(0))
		getStatus(1);
	return;
}
void mmoc::conoutf(std::string &in, int x, int y, int z) {
	CommandInterpretor.conoutf(in, x, y, z);
}
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

