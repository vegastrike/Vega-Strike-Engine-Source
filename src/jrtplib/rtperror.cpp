/*

  This file is a part of JRTPLIB
  Copyright (c) 1999-2002 Jori Liesenborgs

  Contact: jori@lumumba.luc.ac.be

  This library (JRTPLIB) was partially developed for my thesis at the
  School for Knowledge Technology (Belgium/The Netherlands)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#include "rtperror.h"
#include <stdlib.h>

struct RTPErrorStringStruct
{
	int errorvalue;
	char *errormessage;
};

static char rtpnoerrstr[] = "No error";
static char rtpunknownerrstr[] = "Unknown error";

static RTPErrorStringStruct errorstrings[] =
{
	{ ERR_RTP_OUTOFMEM,			"Out of memory" },
	{ ERR_RTP_CONNALREADYCREATED,		"Connection already created" },
	{ ERR_RTP_PORTBASENOTEVEN,		"Specified portbase not even" },
	{ ERR_RTP_CANTCREATESOCKET,		"Can't create a socket"},
	{ ERR_RTP_CANTBINDSOCKET,		"Can't bind a socket"},
	{ ERR_RTP_ENTRYNOTINDESTLIST,		"Entry not in list of destinations"},
	{ ERR_RTP_INVALIDRECEIVEMODE,		"Invalid receive mode"},
	{ ERR_RTP_ILLEGALPORT,			"Illegal port number"},
	{ ERR_RTP_IPTABLEENTRYEXISTS,		"IP entry already exists"},
	{ ERR_RTP_IPTABLEENTRYDOESNTEXIST,	"IP entry was not found"},
	{ ERR_RTP_PACKETTOOLARGE,		"Data won't fit within the current RTP packet size"},
	{ ERR_RTP_SOCKETSNOTOPEN,		"Sockets not opened"},
	{ ERR_RTP_MAXPACKSIZETOOSMALL,		"The specified maximum packet size is too small"},
	{ ERR_RTP_NORTPDATAAVAILABLE,		"No RTP data available"},
	{ ERR_RTP_NORTCPDATAAVAILABLE,		"No RTCP data available"},
	{ ERR_RTP_SSRCNOTFOUND,			"Specified SSRC not the list"},
	{ ERR_RTP_SDESINDEXOUTOFRANGE,		"SDES index out of range"},
	{ ERR_RTP_CSRCNOTINLIST,		"The CSRC was not found in the list"},
	{ ERR_RTP_ARGUMENTCANTBENULL,		"Function argument may not be null"},
	{ ERR_RTP_PACKETPROCESSORNOTINITIALIZED,"Packet processor was not initialized"},
	{ ERR_RTP_PACKETISNOTFORTHISSOURCE,	"Error while inserting packet: ssrc's don't match"},
	{ ERR_RTP_SOURCESNOTINITALIZED,		"Source table not initialized"},
	{ ERR_RTP_COLLISIONBETWEENSSRCS,	"Collision between two SSRCs"},
	{ ERR_RTP_RTCPMODULENOTINITIALIZED,	"RTCP module not initialized"},
	{ ERR_RTP_SESSIONNOTCREATED,		"Session was not created"},
	{ ERR_RTP_SESSIONALREADYINITIALIZED,	"Session already initialized"},
	{ ERR_RTP_NODEFAULTVALUESSET,		"No default values set"},
	{ ERR_RTP_CONTRIBSOURCESNOTINIT,	"Contributing sources not initialized"},
	{ ERR_RTP_CANTSETSOCKETBUFFER,		"Can't set a socket buffer"},
	{ ERR_RTP_NODEFAULTINCREMENTSET,	"No default timestamp increment set"},
	{ ERR_RTP_MAXPACKSIZETOOBIG,		"Maximum packet size to large"},
	{ ERR_RTP_CANTGETSOCKETPORT,		"Can't get the send socket's port number"},
	{ ERR_RTP_CANTGETLOCALIP,		"Can't get the local IP address"},
	{ ERR_RTP_MULTICASTINGNOTSUPPORTED,     "Multicasting not supported in this library"},
	{ ERR_RTP_CANTSETMULTICASTTTL,		"Can't set the IP Time To Live field for multicast datagrams"},
	{ ERR_RTP_NOTAMULTICASTADDRESS,		"Not a multicast address"},
	{ ERR_RTP_ALREADYINMCASTGROUP,		"Already in specified multicast group"},
	{ ERR_RTP_NOTINMCASTGROUP,		"Not in the specified multicast group"},
	{ ERR_RTP_MULTICASTJOINFAILED,		"Unable to join multicast group"},
	{ ERR_RTP_SUBTYPETOOLARGE,		"Specified subtype value too large (maximum is 31)"},
	{ ERR_RTP_CANTSETTOS,			"Unable to set the IP ToS (Type of Service) field"},
	{ 0,					NULL}
};

char *RTPGetErrorString(int rtperrnr)
{
	int i;
	bool found,done;

	if (rtperrnr >= 0)
		return rtpnoerrstr;
	
	found = false;
	done = false;
	i = 0;
	while (!found && !done)
	{
		if (errorstrings[i].errorvalue == 0)
			done = true;
		else if (errorstrings[i].errorvalue == rtperrnr)
			found = true;
		else
			i++;
	}
	if (found)
		return errorstrings[i].errormessage;
	return rtpunknownerrstr;
}
