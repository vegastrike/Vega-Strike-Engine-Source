/*

    This file is a part of JVOIPLIB, a library designed to facilitate
    the use of Voice over IP (VoIP).

    Copyright (C) 2000-2002  Jori Liesenborgs (jori@lumumba.luc.ac.be)

    This library (JVOIPLIB) is based upon work done for my thesis at
    the School for Knowledge Technology (Belgium/The Netherlands)

    The full GNU Library General Public License can be found in the
    file LICENSE.LGPL which is included in the source code archive.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
    USA

*/

#ifndef VOICETRANSMITTER_H

#define VOICETRANSMITTER_H

#include "voicetypes.h"

// start namespace
namespace VoIPFramework
{

class VoiceBlock;

class VoiceTransmitter
{
public:
	VoiceTransmitter() { }
	virtual ~VoiceTransmitter() { }
	virtual int SendBlock(VoiceBlock *vb)=0;
	virtual int StartVoiceSourceIteration()=0;
	virtual void EndVoiceSourceIteration()=0;
	virtual bool GotoFirstVoiceSource()=0;
	virtual bool GotoNextVoiceSource()=0;
	virtual VOIPuint64 GetVoiceSourceID()=0;
	virtual int GetSampleBlock(VoiceBlock *vb)=0;
	virtual int Poll()=0;
	virtual bool SourceHasMoreData()=0;
	virtual int SetSampleOffset(VOIPdouble offset)=0;
};

// end namespace
};

#endif // VOICETRANSMITTER_H
