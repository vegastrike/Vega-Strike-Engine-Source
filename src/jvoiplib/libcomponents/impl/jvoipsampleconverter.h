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

#ifndef JVOIPSAMPLECONVERTER_H

#define JVOIPSAMPLECONVERTER_H

#include "jvoipexception.h"

class JVOIPSampleConverter
{
public:
	JVOIPSampleConverter();
	~JVOIPSampleConverter();
	int SetConversionParams(int srcrate,bool srcstereo,int srcbytespersample,bool srcsigned,bool srcLE,
	                        int dstrate,bool dststereo,int dstbytespersample,bool dstsigned,bool dstLE);
	int Convert(unsigned char *srcbuffer,int srclen,unsigned char *dstbuffer,int dstlen);
private:
	inline void DownRate_Convert();
	void DownRate_DelStereo_Convert();
	void DownRate_AddStereo_Convert();
	void DownRate_SameStereo_Convert();
	inline void UpRate_Convert();
	void UpRate_DelStereo_Convert();
	void UpRate_AddStereo_Convert();
	void UpRate_SameStereo_Convert();
	inline void SameRate_Convert();
	void SameRate_DelStereo_Convert();
	void SameRate_AddStereo_Convert();
	void SameRate_SameStereo_Convert();
	void FillUp();

	int sourcerate,destinationrate;
	bool sourcestereo,destinationstereo;
	int sourcebytespersample,destinationbytespersample;
	bool sourcesigned,destinationsigned;
	bool sourceLE,destinationLE; // endianness
	bool noconversion;
	double rateratio;
	
	int srclen2,dstlen2;
	int dstpos;
	int sourceadvance,destadvance;
	
	unsigned char *sourcebuffer,*destinationbuffer;
	int sourcelen,destinationlen;
};

#endif // JVOIPSAMPLECONVERTER_H
