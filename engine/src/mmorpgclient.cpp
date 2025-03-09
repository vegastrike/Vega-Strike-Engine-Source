/*
 * mmorpgclient.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "command.h"
#include "mmorpgclient.h"
#include "SDL/SDL.h"
#include "SDL_thread.h"
#include "vegastrike.h"
#include "vs_globals.h"

//#ifdef WIN32
//#include <winsock.h>     // For socket(), connect(), send(), and recv()
//typedef int socklen_t;
//#else
//#include <sys/types.h>   // For data types
//#include <sys/socket.h>  // For socket(), connect(), send(), and recv()
//#include <netdb.h>       // For gethostbyname()
//#include <arpa/inet.h>   // For inet_addr()
//#include <unistd.h>      // For close()
//#include <netinet/in.h>  // For sockaddr_in
//#endif
#include "networking/inet.h"

//This is created in command.cpp with single like (search for mmoc)
//If you wish to disable this module, just comment out that single line in command.cpp,
//and don't compile this .cpp file.

mmoc::mmoc() {
    //{{{
    status = false;     //used to let the thread exit
    binmode = false;
    POSmode = false;
    INET_startup();
    //add the connectto to the players command interp.
    cmd = new Functor<mmoc>(this, &mmoc::connectTo);
    CommandInterpretor->addCommand(cmd, "connectto");
    //{{{
    //add commands here to be parsed from network input.
    //some notes:
    //just make them like normal, and the word "FOO" will be the "tag" that triggers  it.
    //eg: the server sends <FOO>args</FOO>, the input routines will parse it to:
    //FOO , and try sending it through the LOCAL command processor (NOT the main one used for the player console).

    //now some notes more to myself
    //The users console at the highest level (in the main ship rendering routines) will need to gather data from the mmoc console to render it there.
    //and never call mmoc::renderconsole. I'll need to make a mutex'd function for this.
    //}}}
}  //}}}
void mmoc::connectTo(const char *address_in, const char *port_in) {
    //{{{
//sockaddr_in m_addr;
    char *address = (char *) address_in;
    char *port = (char *) port_in;
    if (address == NULL) {
        CommandInterpretor->conoutf("Need a host at least, a host and port at most!");
        return;
    }
    if (port == NULL) {
        port = "5555";
    }
    std::string test;
    test.append(address);
    if (test.compare("default") == 0) {
        address = "ant.infice.com";
        port = "5555";
    }
    if ((socket = INET_ConnectTo(address, atoi(port))) < 0) {
        CommandInterpretor->conoutf(
                "Error connecting. Specify another host or verify the status if your network connection.");
        return;
    }
//hostent *server;  // Resolve name
//if ((server = gethostbyname(address)) == NULL) {
//CommandInterpretor->conoutf("Error, couldn't find host");
    //strerror() will not work for gethostbyname() and hstrerror()
    //is supposedly obsolete
//return;
//}

//bzero((char *) &m_addr, sizeof(m_addr));
//m_addr.sin_family = AF_INET;
//bcopy((char *)server->h_addr,
//(char *)&m_addr.sin_addr.s_addr,
//server->h_length);
//m_addr.sin_port = htons(atoi(port));
//int status = 0;

//if ( (status = ::connect(socket,reinterpret_cast<sockaddr *>(&m_addr),sizeof(m_addr))) < 0) {
//CommandInterpretor->conoutf("Couldn't Connect\n");
//return;
//}
    std::string hellomsg;
    hellomsg.append("Vegastrike-user");
    send(hellomsg);
    cl = new Functor<mmoc>(this, &mmoc::close);
    CommandInterpretor->addCommand(cl, "disconnect");

    csay = new Functor<mmoc>(this, &mmoc::send);
    CommandInterpretor->addCommand(csay, "send");

    createThread();
} //}}}
bool mmoc::getStatus(int in) {
    //{{{
//if in = 0, return status, else toggle status.
    SDL_mutex *m = SDL_CreateMutex();
    SDL_mutexP(m);
    if (in != 0) {
        status = !status;
    }
    bool returner = status;
    SDL_mutexV(m);
    SDL_DestroyMutex(m);
    return returner;
} //}}}
void mmoc::ParseRemoteInput(char *buf) {
    //{{{ Main parser
//add  binary modes here.
    if (buf != NULL) {
        start.append(buf);
    }
    size_t teststart;
    if ((teststart = start.find("\n")) < std::string::npos) {
        //we have a full string and are ready to process
        if (teststart < start.size() - 2) {
            for (unsigned int soda = teststart + 1; soda < start.size(); soda++) {
                tempstr += start[soda];
            }
            while (start.size() > teststart) {
                std::string::iterator iter = start.end();
                iter--;
                start.erase(iter);                 //pop goes the weasle
            }
        }
    } else {
//return;
    }
    unsigned int end = start.size();
    std::string hackedup;
    bool open = false, ignore2close = false;
    unsigned int fopend = 0;
    unsigned int counter;
    //parse commands to be executed <FOO=BAR>
    for (counter = 0; counter < end; counter++) {
        if (open) {
            if (start[counter] == '=') {
                hackedup.append("\" ");
            } else if (start[counter] == '>') {
                open = false;
                start.replace(fopend, (counter + 1) - fopend, "");
                unsigned int offset = (counter + 1) - fopend;
                counter = counter - offset;
                commandI::execute(&hackedup, true, socket);
                hackedup.erase();
                end = start.size();
            } else {
                hackedup += start[counter];
            }
        } else if (start[counter] == '<') {
            open = true;
            fopend = counter;
            hackedup.append("\"");
        }
    }
    //optimize by moving this above.
    unsigned int ender = start.size();
    for (counter = 0; counter < ender; counter++) {      //remove \r's
        if (start[counter] == '\r') {
            start.replace(counter, 1, "");
            counter = counter - 2;
            ender = start.size();
        }
    }
    CommandInterpretor->conoutf(start);     //print what's left to the console

    start.erase();
    if (tempstr.size() > 0) {
        start.append(tempstr);
        tempstr.erase();
    }
    {
        size_t asdf;
        while ((asdf = start.find("\n")) < std::string::npos) {
            ParseRemoteInput(NULL);
        }
//if(start.size() > 0 ) {
//start.append("\n\r");
//ParseRemoteInput(NULL);
//}
    }
} //}}}
class POSpack {
public:
    int playernum;  //who's position is this
    double x, y, z;     //the position
};

bool mmoc::listenThread() {
    //{{{
    const int MAXBUF = 1000;
    char buffer[MAXBUF + 1];
    bool stat;
    while ((stat = getStatus(0)) == true) {
        bzero(buffer, MAXBUF);
        if (!binmode) {
            if (::INET_Recv(socket, buffer, sizeof(buffer) - 1) <= 0) {
                getStatus(1);                 //1 toggles status, 0 reads status
                return false;
            } else {
                ParseRemoteInput(buffer);
            }
        }
        if (binmode) {
            if (POSmode) {
                //if Position mode
                POSpack position;
                if (::INET_Read(socket, reinterpret_cast< char * > (&position), sizeof(POSpack)) <= 0) {
                    //I believe INET_Read will keep looping until size is filled
                    getStatus(1);                     //toggle status
                    return false;
                }
                ParseMovement(position);
                POSmode = false;
            }
            //other bin modes
            binmode = false;             //done
        }
    }
    /*---Clean up---*/
    return stat;
} //}}}
void mmoc::createThread() {
    //{{{
    ::SDL_CreateThread(startThread, reinterpret_cast< void * > (this));
} //}}}
void mmoc::send(char *buffer, int size) {
    //{{{
    ::INET_Write(socket, size, buffer);     //or write(socket, buffer, size) for windwos?
} //}}}
void mmoc::send(std::string &instring) {
    //{{{
    unsigned int x = instring.find("send ");
    if (x == 0) {
        instring.replace(0, 5, "");
    }
    instring.append("\r\n");
    send((char *) instring.c_str(), instring.size());
} //}}}
void mmoc::negotiate(std::vector<std::string *> *d) {
    std::vector<std::string *>::iterator iter = d->begin();
    iter++;
    if (iter >= d->end()) {
        return;
    }      //nothing to negotiate
    if ((*(iter))->compare("P")) {
        binarysize = sizeof(POSpack);
        //check the next iterator for a number (X), if there is one
        //set it up to loop in the listenThread() to read X Position packets
        binmode = true;
        POSmode = true;
        std::string ack;
        ack.append("ap");         //ack position, tell the server to send it
        send(ack);
    }
}

void mmoc::ParseMovement(POSpack &in) {
    //.....
}

int startThread(void *mmoc2use) {
    //{{{
    mmoc *looper = reinterpret_cast< mmoc * > (mmoc2use);
    if (!looper->getStatus(0)) {
        looper->getStatus(1);
    }
    CommandInterpretor->conoutf("Connected.");
    while (looper->listenThread()) {
    }
    ::close(looper->getSocket());
    CommandInterpretor->remCommand(looper->csay);
    CommandInterpretor->remCommand(looper->cl);
    CommandInterpretor->conoutf("Closed");
    if (looper->getStatus(0)) {
        looper->getStatus(1);
    }
    return 0;
} //}}}
void mmoc::close() {
    if (getStatus(0)) {
        getStatus(1);
    }
}

void mmoc::conoutf(std::string &in, int x, int y, int z) {
    CommandInterpretor->conoutf(in, x, y, z);
}
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */

