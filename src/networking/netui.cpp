#include <config.h>

#include "const.h"
#include "netui.h"
#include "vsnet_serversocket.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	#include <winsock.h>
#else
	//#warning "GCC platform"
	#define SOCKET_ERROR -1
	#include <fcntl.h>
#endif

static void static_initNetwork( )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    static bool first_time = true;
    if( first_time )
    {
        first_time = false;

        COUT <<"Initializing Winsock"<<std::endl;
		WORD wVersionRequested = MAKEWORD( 1, 1 );
		WSADATA wsaData; 
		int res = WSAStartup(wVersionRequested,&wsaData);
		if( res != 0)
				COUT <<"Error initializing Winsock"<<std::endl;
    }
#endif
}

/**************************************************************/
/**************************************************************/
/**** Create (and bind) a socket on host                   ****/
/**************************************************************/

// Creates and bind the socket designed to receive coms
// host == NULL -> localhost

SOCKETALT NetUITCP::createSocket( char * host, unsigned short srv_port )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << std::endl;

    static_initNetwork( );

	int            local_fd;
	AddressIP      remote_ip;

	// this->server = 0;
	// SOCKETALT part
	struct hostent	*he = NULL;
#if defined(_WIN32) && !defined(__CYGWIN__)
	int sockerr= INVALID_SOCKET;
#else
	int sockerr= -1;
#endif

	if( (local_fd = socket( PF_INET, SOCK_STREAM, 0))==sockerr)
		f_error( "Could not create socket");

	// If port is not given, use the defaults ones --> do not work with specified ones yet... well, didn't try
	if( srv_port == 0 )
	    srv_port = SERVER_PORT;

	// Gets the host info for host
	if( host[0]<48 || host[0]>57)
	{
		COUT <<"Resolving host name... ";
		if( (he = gethostbyname( host)) == NULL)
			f_error( "\nCould not resolve hostname ");
		memcpy( &remote_ip.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
		COUT <<"found : "<<inet_ntoa( remote_ip.sin_addr)<<std::endl;
	}
	else
	{
#if defined(_WIN32) && !defined(__CYGWIN__)
		if( (remote_ip.sin_addr.s_addr=inet_addr( host)) == 0)
			f_error( "Error inet_addr ");
#else			
		if( inet_aton( host, &remote_ip.sin_addr) == 0)
			f_error( "Error inet_aton ");
#endif
	}
	// Store it in srv_ip struct
	remote_ip.sin_port   = htons( srv_port );
	remote_ip.sin_family = AF_INET;

	COUT << "Connecting to " << inet_ntoa( remote_ip.sin_addr) << " on port " << srv_port << std::endl;
	if( connect( local_fd, (sockaddr *)&remote_ip, sizeof( struct sockaddr))==SOCKET_ERROR)
	{
		perror( "Can't connect to server ");
		//exit( -1 );
	}
	COUT << "Connected to " << inet_ntoa( remote_ip.sin_addr) << ":" << srv_port << std::endl;

	SOCKETALT ret( local_fd, SOCKETALT::TCP, remote_ip );
	COUT << "SOCKETALT n° : " << ret.get_fd() << std::endl;
	return ret;
}

ServerSocket* NetUITCP::createServerSocket( unsigned short port )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << std::endl;

    static_initNetwork( );
    
    int       local_fd;
    AddressIP local_ip;

#if defined(_WIN32) && !defined(__CYGWIN__)
    int sockerr= INVALID_SOCKET;
#else
    int sockerr= -1;
#endif

    // this->server = 1;

    if( (local_fd = socket( PF_INET, SOCK_STREAM, 0))==sockerr )
    {
        f_error( "Could not create socket");
    }

    // If port is not given, use the defaults ones --> do not work with specified ones yet... well, didn't try
    if( port == 0 ) port = SERVER_PORT;

    memset( &local_ip, 0, sizeof(AddressIP) );
    local_ip.sin_addr.s_addr = htonl(INADDR_ANY);
    local_ip.sin_port        = htons( port );
    local_ip.sin_family      = AF_INET;
    // binds socket
    COUT << "Bind on " << ntohl(local_ip.sin_addr.s_addr) << ", port " << ntohs( local_ip.sin_port) << std::endl;
    if( bind( local_fd, (sockaddr *)&local_ip, sizeof( struct sockaddr_in) )==SOCKET_ERROR )
    {
        perror( "Problem binding socket" );
        exit( -1 );
    }

    COUT << "Accepting max : " << SOMAXCONN << std::endl;
    if( listen( local_fd, SOMAXCONN)==SOCKET_ERROR)
    {
        perror( "Problem listening on socket" );
        exit( -1 );
    }
    COUT << "Listening on socket " << local_fd << std::endl
         << "*** ServerSocket n° : " << local_fd << std::endl;
    return new ServerSocketTCP( local_fd, local_ip );
}

/**************************************************************/
/**** Create (and bind) a socket on host                   ****/
/**************************************************************/

// Creates and bind the socket designed to receive coms
// host == NULL -> localhost

SOCKETALT NetUIUDP::createSocket( char * host, unsigned short srv_port )
{
    COUT << " enter " << __PRETTY_FUNCTION__ << std::endl;
    static_initNetwork( );

    AddressIP local_ip;
    AddressIP remote_ip;
    int       local_fd;
#if defined(_WIN32) && !defined(__CYGWIN__)
    static const int sockerr = INVALID_SOCKET;
#else
    static const int sockerr = -1;
#endif

    // this->server = 0;

    // If port is not given, use the defaults ones --> do not work with specified ones yet... well, didn't try
    if( srv_port==0 ) srv_port = SERVER_PORT;

    if( (local_fd = socket( PF_INET, SOCK_DGRAM, 0))==sockerr)
    {
        f_error( "Could not create socket");
    }

#if !defined(_WIN32) || defined(__CYGWIN__)
    if( fcntl( local_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror( "Error fcntl : ");
        exit( 1);
    }
#else
    unsigned long datato = 1;
    if( ioctlsocket( local_fd, FIONBIO, &datato ) !=0 )
    {
        perror( "Error fcntl : ");
        exit( 1);
    }
#endif

    // Gets the host info for host
    struct hostent	*he = NULL;

    if( host[0]<48 || host[0]>57)
    {
        COUT <<"Resolving host name... ";
        if( (he = gethostbyname( host)) == NULL)
        f_error( "\nCould not resolve hostname ");
        memcpy( &remote_ip.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
        COUT <<"found : "<<inet_ntoa( remote_ip.sin_addr)<<std::endl;
    }
    else
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
        if( (remote_ip.sin_addr.s_addr=inet_addr( host)) == 0)
#else			
        if( inet_aton( host, &remote_ip.sin_addr) == 0)
#endif
	{
            f_error( "Error inet_aton ");
	}
    }
    // Store it in srv_ip struct
    remote_ip.sin_port= htons( srv_port);
    remote_ip.sin_family= AF_INET;

    local_ip.sin_addr.s_addr = htonl(INADDR_ANY);
    local_ip.sin_port = htons( 0);
    local_ip.sin_family = AF_INET;

    // binds socket
    if( bind( local_fd, (sockaddr *)&local_ip, sizeof(struct sockaddr_in))==SOCKET_ERROR )
    {
	perror( "Can't bind socket" );
	exit( -1 );
    }

    SOCKETALT ret( local_fd, SOCKETALT::UDP, remote_ip );
    COUT << "Bind on localhost, " << ret << std::endl;
    return ret;
}

ServerSocket* NetUIUDP::createServerSocket( unsigned short port )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << std::endl;
    static_initNetwork( );

    int       local_fd;
    AddressIP local_ip;

#if defined(_WIN32) && !defined(__CYGWIN__)
    static const int sockerr= INVALID_SOCKET;
#else
    static const int sockerr= -1;
#endif

    // this->server = 1;
    // If port is not given, use the defaults ones --> do not work with specified ones yet... well, didn't try
    if( port == 0 ) port = SERVER_PORT;

    if( (local_fd = socket( PF_INET, SOCK_DGRAM, 0 ))==sockerr )
        f_error( "Could not create socket");

#if !defined(_WIN32) || defined(__CYGWIN__)
    if( fcntl( local_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        perror( "Error fcntl : ");
        exit( 1);
    }
#else
    unsigned long datato = 1;
    if( ioctlsocket( local_fd,FIONBIO,&datato ) !=0 )
    {
        perror( "Error fcntl : ");
        exit( 1);
    }
#endif

    memset( &local_ip, 0, sizeof(AddressIP) );
    local_ip.sin_addr.s_addr = htonl(INADDR_ANY);
    local_ip.sin_port        = htons( port );
    local_ip.sin_family      = AF_INET;
    // binds socket
    if( bind( local_fd, (sockaddr *)&local_ip, sizeof( struct sockaddr_in ) )==SOCKET_ERROR )
    {
        perror( "Cannot bind socket");
	exit( -1 );
    }

    ServerSocket* ret = new ServerSocketUDP( local_fd, local_ip );
    COUT << "Bind on localhost, " << *ret << std::endl;
    return ret;
}

void f_error( char *s)
{
    perror( s);
    exit(1);
}

