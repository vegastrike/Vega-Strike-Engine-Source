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

#include "jvoipsampleconverter.h"
#include "jvoiperrors.h"
#include <string.h>
#include <stdio.h>

#include "debugnew.h"

#define GetSixteenBitSampleValue(buf,val) \
{\
	val = 0;\
	if (sourcebytespersample == 1)\
	{\
		if (sourcesigned)\
			val = (int)((char)(buf)[0]);\
		else\
		{\
			val = (int)(buf)[0];\
			val -= 128;\
		}\
		val *= 256;\
	}\
	else\
	{\
		unsigned char msb,lsb;\
		\
		if (sourceLE)\
		{\
			lsb = (buf)[0];\
			msb = (buf)[1];\
		}\
		else \
		{\
			lsb = (buf)[1];\
			msb = (buf)[0];\
		}\
		\
		if (sourcesigned)\
		{\
			if (msb&128) \
				val = (~0)^0xFFFF; \
			val |= (msb<<8);\
			val |= lsb;\
		}\
		else \
		{\
			val |= (msb<<8);\
			val |= lsb;\
			val -= 32768;\
		}\
	}\
}

#define EncodeSixteenBitSampleValue(buf,value)	\
{\
	if (destinationbytespersample == 1)\
	{\
		int val;\
		\
		val = (int)((((double)value)/32768.0)*128.0);\
		if (destinationsigned)\
		{\
			if (val > 127)\
				val = 127;\
			else if (val < -128)\
				val = -128;\
			(buf)[0] = (unsigned char)((char)val);\
		}\
		else\
		{\
			val += 128;\
			if (val > 255)\
				val = 255;\
			else if (val < 0)\
				val = 0;\
			(buf)[0] = (unsigned char)val;\
		}\
	}\
	else \
	{\
		unsigned char msb,lsb;\
		int val;\
		\
		if (destinationsigned)\
			val = value;\
		else \
			val = value+32768;\
		msb = (unsigned char)((val>>8)&0xFF);\
		lsb = (unsigned char)(val&0xFF);\
		\
		if (destinationLE)\
		{\
			(buf)[0] = lsb;\
			(buf)[1] = msb;\
		}\
		else\
		{\
			(buf)[1] = lsb;\
			(buf)[0] = msb;\
		}\
	}\
}

JVOIPSampleConverter::JVOIPSampleConverter()
{
	noconversion = true;
}

JVOIPSampleConverter::~JVOIPSampleConverter()
{
}

int JVOIPSampleConverter::SetConversionParams(int srcrate,bool srcstereo,int srcbytespersample,bool srcsigned,bool srcLE,
	                        	      int dstrate,bool dststereo,int dstbytespersample,bool dstsigned,bool dstLE)
{
	if (srcrate < 1 || dstrate < 1 || srcbytespersample < 1 || dstbytespersample < 1 ||
	    srcbytespersample > 2 || dstbytespersample > 2)
		return ERR_JVOIPLIB_SAMPCONV_BADCONVERSIONPARAM;
	sourcerate = srcrate;
	destinationrate = dstrate;
	sourcestereo = srcstereo;
	destinationstereo = dststereo;
	sourcebytespersample = srcbytespersample;
	destinationbytespersample = dstbytespersample;
	sourcesigned = srcsigned;
	destinationsigned  = dstsigned;
	if (sourcebytespersample == 2)
		sourceLE = srcLE;
	else
		sourceLE = false;
	if (destinationbytespersample == 2)
		destinationLE = dstLE;
	else
		destinationLE = false;
	rateratio = ((double)sourcerate)/((double)destinationrate);
	if (sourcerate == destinationrate && sourcestereo == destinationstereo &&
	    sourcebytespersample == destinationbytespersample && sourcesigned == destinationsigned &&
	    sourceLE == destinationLE)
		noconversion = true;
	else
		noconversion = false;
	return 0;
}

int JVOIPSampleConverter::Convert(unsigned char *srcbuffer,int srclen,unsigned char *dstbuffer,int dstlen)
{
	if (srclen < 1 || dstlen < 1 || srcbuffer == NULL || dstbuffer == NULL)
		return 0;
	if (noconversion)
	{
		int minlen;
		
		minlen = (srclen < dstlen)?srclen:dstlen;
		memcpy((void *)dstbuffer,(void *)srcbuffer,minlen);
		return minlen;
	}
	sourcebuffer = srcbuffer;
	destinationbuffer = dstbuffer;
	sourcelen = srclen;
	destinationlen = dstlen;
	
	if (sourcerate > destinationrate)
		DownRate_Convert();
	else if (sourcerate < destinationrate)
		UpRate_Convert();
	else
		SameRate_Convert();
	FillUp();		
	return dstpos;
}

inline void JVOIPSampleConverter::DownRate_Convert()
{
	if (sourcestereo && !destinationstereo)
		DownRate_DelStereo_Convert();
	else if (!sourcestereo && destinationstereo)
		DownRate_AddStereo_Convert();
	else
		DownRate_SameStereo_Convert();
}

void JVOIPSampleConverter::DownRate_DelStereo_Convert()
{
	int dif;
	int sourceadvancehalf;
	int srcpos,prevsrcpos,i;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	prevsrcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((dif = (sourcelen&0x03)) != 0)
			srclen2 -= dif;
		sourceadvance = 4;
		sourceadvancehalf = 2;
	}
	else
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
		sourceadvancehalf = 1;
	}
	if (destinationbytespersample == 2)
	{
		destadvance = 2;
		if ((destinationlen&0x01) != 0)
			dstlen2--;
	}
	else
		destadvance = 1;
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int leftval,rightval;
		int newval,count;

		newval = 0;
		for (i = prevsrcpos,count = 0 ; i <= srcpos ; i += sourceadvance,count++)
		{
			GetSixteenBitSampleValue(sourcebuffer+i,leftval);
			GetSixteenBitSampleValue(sourcebuffer+i+sourceadvancehalf,rightval);
			newval += leftval+rightval;
		}
		newval /= (count*2);
		
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		srcpos2 += rateratio;
		prevsrcpos = srcpos;
		srcpos = ((int)(srcpos2+0.5))*sourceadvance;
	}
}

void JVOIPSampleConverter::DownRate_AddStereo_Convert()
{
	int dif;
	int destadvancehalf;
	int srcpos,prevsrcpos,i;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	prevsrcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
	}
	else
		sourceadvance = 1;
	
	if (destinationbytespersample == 2)
	{
		destadvance = 4;
		if ((dif = (destinationlen&0x03)) != 0)
			dstlen2 -= dif;
		destadvancehalf = 2;
	}
	else
	{
		if ((destinationlen&0x01) != 0)
			dstlen2--;
		destadvance = 2;
		destadvancehalf = 1;
	}
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int newval,v,count;

		newval = 0;
		for (i = prevsrcpos,count = 0 ; i <= srcpos ; i += sourceadvance,count++)
		{
			GetSixteenBitSampleValue(sourcebuffer+i,v);
			newval += v;
		}
		newval /= count;
		
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newval);
		srcpos2 += rateratio;
		prevsrcpos = srcpos;
		srcpos = ((int)(srcpos2+0.5))*sourceadvance;
	}
}

void JVOIPSampleConverter::DownRate_SameStereo_Convert()
{
	int dif;
	int sourceadvancehalf,destadvancehalf;
	int srcpos,prevsrcpos,i;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;
	srcpos = 0;
	prevsrcpos = 0;
	srcpos2 = 0;
	
	if (sourcestereo)
	{
		if (sourcebytespersample == 2)
		{
			if ((dif = (sourcelen&0x03)) != 0)
				srclen2 -= dif;
			sourceadvance = 4;
			sourceadvancehalf = 2;
		}
		else
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
			sourceadvancehalf = 1;
	        }
		
		if (destinationbytespersample == 2)
		{
			destadvance = 4;
			if ((dif = (destinationlen&0x03)) != 0)
				dstlen2 -= dif;
			destadvancehalf = 2;
		}
		else
		{
			if ((destinationlen&0x01) != 0)
				dstlen2--;
			destadvance = 2;
			destadvancehalf = 1;
		}       	
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newlval,newrval,v,count;
                	
			newlval = 0;
			newrval = 0;
			for (i = prevsrcpos,count = 0 ; i <= srcpos ; i += sourceadvance,count++)
			{
				GetSixteenBitSampleValue(sourcebuffer+i,v);
				newlval += v;
				GetSixteenBitSampleValue(sourcebuffer+i+sourceadvancehalf,v);
				newrval += v;
			}
			newlval /= count;
			newrval /= count;
			
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newlval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newrval);
			srcpos2 += rateratio;
			prevsrcpos = srcpos;
			srcpos = ((int)(srcpos2+0.5))*sourceadvance;
		}
	}
	else // no stereo
	{
		if (sourcebytespersample == 2)
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
		}
		else
			sourceadvance = 1;
		
		if (destinationbytespersample == 2)
		{
			destadvance = 2;
			if ((destinationlen&0x01) != 0)
				dstlen2--;
		}
		else
			destadvance = 1;
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newval,count,v;
                	
			newval = 0;
			for (i = prevsrcpos,count = 0 ; i <= srcpos ; i += sourceadvance,count++)
			{
				GetSixteenBitSampleValue(sourcebuffer+i,v);
				newval += v;
			}
			newval /= count;
			
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
			srcpos2 += rateratio;
			prevsrcpos = srcpos;
			srcpos = ((int)(srcpos2+0.5))*sourceadvance;
		}
	}
}

inline void JVOIPSampleConverter::UpRate_Convert()
{
	if (sourcestereo && !destinationstereo)
		UpRate_DelStereo_Convert();
	else if (!sourcestereo && destinationstereo)
		UpRate_AddStereo_Convert();
	else
		UpRate_SameStereo_Convert();
}
/*
void JVOIPSampleConverter::UpRate_DelStereo_Convert()
{
	int dif;
	int sourceadvancehalf;
	int srcpos;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((dif = (sourcelen&0x03)) != 0)
			srclen2 -= dif;
		sourceadvance = 4;
		sourceadvancehalf = 2;
	}
	else
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
		sourceadvancehalf = 1;
	}
	if (destinationbytespersample == 2)
	{
		destadvance = 2;
		if ((destinationlen&0x01) != 0)
			dstlen2--;
	}
	else
		destadvance = 1;
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int leftval,rightval;
		int newval;

		GetSixteenBitSampleValue(sourcebuffer+srcpos,leftval);
		GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,rightval);
		newval = (leftval+rightval)/2;
		
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		srcpos2 += rateratio;
		srcpos = ((int)(srcpos2+0.5))*sourceadvance;
	}
}
*/
void JVOIPSampleConverter::UpRate_DelStereo_Convert()
{
	int dif;
	int sourceadvancehalf;
	int srcpos;
	int curval,nextval,prevadd;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((dif = (sourcelen&0x03)) != 0)
			srclen2 -= dif;
		sourceadvance = 4;
		sourceadvancehalf = 2;
	}
	else
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
		sourceadvancehalf = 1;
	}
	if (destinationbytespersample == 2)
	{
		destadvance = 2;
		if ((destinationlen&0x01) != 0)
			dstlen2--;
	}
	else
		destadvance = 1;
		
	if (srcpos < srclen2 && 0 < dstlen2)
	{
		int leftval,rightval;
		
		GetSixteenBitSampleValue(sourcebuffer+srcpos,leftval);
		GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,rightval);
		curval = (leftval+rightval)/2;
	}
	
	prevadd = 0;
	for (dstpos = 0 ; dstpos < dstlen2 ;                )
	{
		int leftval,rightval,prevpos;
		int newval;
		double pos;

		prevpos = srcpos;
		srcpos += sourceadvance;
		if (srcpos < srclen2)
		{
			GetSixteenBitSampleValue(sourcebuffer+srcpos,leftval);
			GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,rightval);
			nextval = (leftval+rightval)/2;
			prevadd = nextval-curval;
		}
		else
			nextval = curval + prevadd;
		
		while ((pos = (srcpos2+0.5)*sourceadvance) < (double)srcpos && dstpos < dstlen2)
		{
			newval = (int)((double)curval+(double)(nextval-curval)*((double)pos-prevpos)/(double)sourceadvance);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
			srcpos2 += rateratio;
			dstpos += destadvance;
		}
		
		curval = nextval;
	}
}

/*
void JVOIPSampleConverter::UpRate_AddStereo_Convert()
{
	int dif;
	int destadvancehalf;
	int srcpos;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
	}
	else
		sourceadvance = 1;
	
	if (destinationbytespersample == 2)
	{
		destadvance = 4;
		if ((dif = (destinationlen&0x03)) != 0)
			dstlen2 -= dif;
		destadvancehalf = 2;
	}
	else
	{
		if ((destinationlen&0x01) != 0)
			dstlen2--;
		destadvance = 2;
		destadvancehalf = 1;
	}
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int newval;

		GetSixteenBitSampleValue(sourcebuffer+srcpos,newval);
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newval);
		srcpos2 += rateratio;
		srcpos = ((int)(srcpos2+0.5))*sourceadvance;
	}
}
*/

void JVOIPSampleConverter::UpRate_AddStereo_Convert()
{
	int dif;
	int destadvancehalf;
	int srcpos;
	double srcpos2;
	int curval,nextval;
	int prevadd;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	srcpos2 = 0;
	if (sourcebytespersample == 2)
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
	}
	else
		sourceadvance = 1;
	
	if (destinationbytespersample == 2)
	{
		destadvance = 4;
		if ((dif = (destinationlen&0x03)) != 0)
			dstlen2 -= dif;
		destadvancehalf = 2;
	}
	else
	{
		if ((destinationlen&0x01) != 0)
			dstlen2--;
		destadvance = 2;
		destadvancehalf = 1;
	}

	if (srcpos < srclen2 && dstlen2 > 0)
	{
		GetSixteenBitSampleValue(sourcebuffer+srcpos,curval);
	}
	
	prevadd = 0;
	for (dstpos = 0 ; dstpos < dstlen2 ;          )
	{
		int newval,prevpos;
		double pos;
		
		prevpos = srcpos;
		srcpos += sourceadvance;
		if (srcpos < srclen2)
		{
			GetSixteenBitSampleValue(sourcebuffer+srcpos,nextval);
			prevadd = nextval-curval;
		}
		else
			nextval = curval + prevadd;
		
		while ((pos = (srcpos2+0.5)*sourceadvance) < (double)srcpos && dstpos < dstlen2)
		{
			newval = (int)((double)curval+(double)(nextval-curval)*((double)pos-prevpos)/(double)sourceadvance);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newval);
			srcpos2 += rateratio;
			dstpos += destadvance;
		}
		
		curval = nextval;
	}
}

/*
void JVOIPSampleConverter::UpRate_SameStereo_Convert()
{
	int dif;
	int sourceadvancehalf,destadvancehalf;
	int srcpos;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;
	srcpos = 0;
	srcpos2 = 0;
	
	if (sourcestereo)
	{
		if (sourcebytespersample == 2)
		{
			if ((dif = (sourcelen&0x03)) != 0)
				srclen2 -= dif;
			sourceadvance = 4;
			sourceadvancehalf = 2;
		}
		else
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
			sourceadvancehalf = 1;
	        }
		
		if (destinationbytespersample == 2)
		{
			destadvance = 4;
			if ((dif = (destinationlen&0x03)) != 0)
				dstlen2 -= dif;
			destadvancehalf = 2;
		}
		else
		{
			if ((destinationlen&0x01) != 0)
				dstlen2--;
			destadvance = 2;
			destadvancehalf = 1;
		}       	
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newlval,newrval;
                	
			GetSixteenBitSampleValue(sourcebuffer+srcpos,newlval);
			GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,newrval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newlval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newrval);
			srcpos2 += rateratio;
			srcpos = ((int)(srcpos2+0.5))*sourceadvance;
		}
	}
	else // no stereo
	{
		if (sourcebytespersample == 2)
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
		}
		else
			sourceadvance = 1;
		
		if (destinationbytespersample == 2)
		{
			destadvance = 2;
			if ((destinationlen&0x01) != 0)
				dstlen2--;
		}
		else
			destadvance = 1;
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newval;
                	
			GetSixteenBitSampleValue(sourcebuffer+srcpos,newval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
			srcpos2 += rateratio;
			srcpos = ((int)(srcpos2+0.5))*sourceadvance;
		}
	}
}
*/
void JVOIPSampleConverter::UpRate_SameStereo_Convert()
{
	int dif;
	int sourceadvancehalf,destadvancehalf;
	int srcpos;
	double srcpos2;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;
	srcpos = 0;
	srcpos2 = 0;
	if (sourcestereo)
	{
		int currval,curlval,nextrval,nextlval;
		int rprevadd,lprevadd;
		
		if (sourcebytespersample == 2)
		{
			if ((dif = (sourcelen&0x03)) != 0)
				srclen2 -= dif;
			sourceadvance = 4;
			sourceadvancehalf = 2;
		}
		else
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
			sourceadvancehalf = 1;
	        }
		
		if (destinationbytespersample == 2)
		{
			destadvance = 4;
			if ((dif = (destinationlen&0x03)) != 0)
				dstlen2 -= dif;
			destadvancehalf = 2;
		}
		else
		{
			if ((destinationlen&0x01) != 0)
				dstlen2--;
			destadvance = 2;
			destadvancehalf = 1;
		}       	
		
		if (srcpos < srclen2 && 0 < dstlen2)
		{
			GetSixteenBitSampleValue(sourcebuffer+srcpos,curlval);
			GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,currval);
		}
		
		rprevadd = 0;
		lprevadd = 0;
		for (dstpos = 0 ; dstpos < dstlen2 ;              )
		{
			int prevpos;
			int newlval,newrval;
			double pos;
			
			prevpos = srcpos;
			srcpos += sourceadvance;
			if (srcpos < srclen2)
			{
				GetSixteenBitSampleValue(sourcebuffer+srcpos,nextlval);
				GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,nextrval);
				rprevadd = nextrval-currval;
				lprevadd = nextlval-curlval;
			}
			else
			{
				nextrval = currval+rprevadd;
				nextlval = curlval+lprevadd;
			}
                	
			while ((pos = (srcpos2+0.5)*sourceadvance) < (double)srcpos && dstpos < dstlen2)
			{
				double factor;
				
				factor = ((double)pos-prevpos)/(double)sourceadvance;
				newlval = (int)((double)curlval+(double)(nextlval-curlval)*factor);
				newrval = (int)((double)currval+(double)(nextrval-currval)*factor);
				
				EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newlval);
				EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newrval);
				srcpos2 += rateratio;
				dstpos += destadvance;
			}
			
			currval = nextrval;
			curlval = nextlval;
		}
	}
	else // no stereo
	{
		int curval,nextval,prevadd;
	
		if (sourcebytespersample == 2)
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
		}
		else
			sourceadvance = 1;
		
		if (destinationbytespersample == 2)
		{
			destadvance = 2;
			if ((destinationlen&0x01) != 0)
				dstlen2--;
		}
		else
			destadvance = 1;
		
		if (srcpos < srclen2 && 0 < dstlen2)
			GetSixteenBitSampleValue(sourcebuffer+srcpos,curval);
			
		prevadd = 0;
		for (dstpos = 0 ; dstpos < dstlen2 ;            )
		{
			int newval,prevpos;
			double pos;
			
                	prevpos = srcpos;
                	srcpos += sourceadvance;
                	if (srcpos < srclen2)
                	{
                		GetSixteenBitSampleValue(sourcebuffer+srcpos,nextval);
                		prevadd = nextval-curval;
			}
                	else
                		nextval = curval+prevadd;

                	while ((pos = (srcpos2+0.5)*sourceadvance) < (double)srcpos && dstpos < dstlen2)
                	{
                		newval = (int)((double)curval+(double)(nextval-curval)*((double)pos-prevpos)/(double)sourceadvance);
				EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
				srcpos2 += rateratio;
				dstpos += destadvance;
			}
			
			curval = nextval;
		}
	}
}


inline void JVOIPSampleConverter::SameRate_Convert()
{
	if (sourcestereo && !destinationstereo)
		SameRate_DelStereo_Convert();
	else if (!sourcestereo && destinationstereo)
		SameRate_AddStereo_Convert();
	else
		SameRate_SameStereo_Convert();
}

void JVOIPSampleConverter::SameRate_DelStereo_Convert()
{
	int dif;
	int sourceadvancehalf;
	int srcpos;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	if (sourcebytespersample == 2)
	{
		if ((dif = (sourcelen&0x03)) != 0)
			srclen2 -= dif;
		sourceadvance = 4;
		sourceadvancehalf = 2;
	}
	else
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
		sourceadvancehalf = 1;
	}
	if (destinationbytespersample == 2)
	{
		destadvance = 2;
		if ((destinationlen&0x01) != 0)
			dstlen2--;
	}
	else
		destadvance = 1;
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int leftval,rightval;
		int newval;

		GetSixteenBitSampleValue(sourcebuffer+srcpos,leftval);
		GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,rightval);
		newval = (leftval+rightval)/2;
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		srcpos += sourceadvance;
	}
}

void JVOIPSampleConverter::SameRate_AddStereo_Convert()
{
	int dif;
	int destadvancehalf;
	int srcpos;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;

	srcpos = 0;
	if (sourcebytespersample == 2)
	{
		if ((sourcelen&0x01) != 0)
			srclen2--;
		sourceadvance = 2;
	}
	else
		sourceadvance = 1;
	
	if (destinationbytespersample == 2)
	{
		destadvance = 4;
		if ((dif = (destinationlen&0x03)) != 0)
			dstlen2 -= dif;
		destadvancehalf = 2;
	}
	else
	{
		if ((destinationlen&0x01) != 0)
			dstlen2--;
		destadvance = 2;
		destadvancehalf = 1;
	}
		
	for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
	{
		int newval;

		GetSixteenBitSampleValue(sourcebuffer+srcpos,newval);
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
		EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newval);
		srcpos += sourceadvance;
	}
}

void JVOIPSampleConverter::SameRate_SameStereo_Convert()
{
	int dif;
	int sourceadvancehalf,destadvancehalf;
	int srcpos;
	
	srclen2 = sourcelen;
	dstlen2 = destinationlen;
	srcpos = 0;
	
	if (sourcestereo)
	{
		if (sourcebytespersample == 2)
		{
			if ((dif = (sourcelen&0x03)) != 0)
				srclen2 -= dif;
			sourceadvance = 4;
			sourceadvancehalf = 2;
		}
		else
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
			sourceadvancehalf = 1;
	        }
		
		if (destinationbytespersample == 2)
		{
			destadvance = 4;
			if ((dif = (destinationlen&0x03)) != 0)
				dstlen2 -= dif;
			destadvancehalf = 2;
		}
		else
		{
			if ((destinationlen&0x01) != 0)
				dstlen2--;
			destadvance = 2;
			destadvancehalf = 1;
		}       	
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newlval,newrval;
                	
			GetSixteenBitSampleValue(sourcebuffer+srcpos,newlval);
			GetSixteenBitSampleValue(sourcebuffer+srcpos+sourceadvancehalf,newrval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newlval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos+destadvancehalf,newrval);
			srcpos += sourceadvance;
		}
	}
	else // no stereo
	{
		if (sourcebytespersample == 2)
		{
			if ((sourcelen&0x01) != 0)
				srclen2--;
			sourceadvance = 2;
		}
		else
			sourceadvance = 1;
		
		if (destinationbytespersample == 2)
		{
			destadvance = 2;
			if ((destinationlen&0x01) != 0)
				dstlen2--;
		}
		else
			destadvance = 1;
		
		for (dstpos = 0 ; srcpos < srclen2 && dstpos < dstlen2 ; dstpos += destadvance)
		{
			int newval;
                	
			GetSixteenBitSampleValue(sourcebuffer+srcpos,newval);
			EncodeSixteenBitSampleValue(destinationbuffer+dstpos,newval);
			srcpos += sourceadvance;
		}
	}
}

void JVOIPSampleConverter::FillUp()
{
	double srctime;
	int dsttimelen,dif;
	
	srctime = ((double)(srclen2/sourceadvance))/((double)sourcerate);
	dsttimelen = (int)(srctime*((double)destinationrate)*((double)destadvance)+0.5);
	if (destadvance > 1)
	{
		if ((dif = (dsttimelen%destadvance)) != 0)
			dsttimelen -= dif;
	}
	dsttimelen = (dsttimelen < dstlen2)?dsttimelen:dstlen2;
	
	if (dstpos < dsttimelen)
	{
		if (dstpos != 0)
		{
			unsigned char *buf;
			int i;
			
			buf = destinationbuffer+dstpos-destadvance;
			while (dstpos < dsttimelen)
			{
				for (i = 0 ; i < destadvance ; i++)
					destinationbuffer[dstpos+i] = buf[i];
				dstpos += destadvance;
			}
		}
		else // fill with silence
		{
			if (destinationstereo)
			{
				int halfadvance;
				
				halfadvance = destadvance/2;
				while (dstpos < dsttimelen)
				{
					EncodeSixteenBitSampleValue(destinationbuffer+dstpos,0);
					EncodeSixteenBitSampleValue(destinationbuffer+dstpos+halfadvance,0);
					dstpos += destadvance;
				}
			}
			else
			{
				while (dstpos < dsttimelen)
				{
					EncodeSixteenBitSampleValue(destinationbuffer+dstpos,0);
					dstpos += destadvance;
				}
			}
		}
	}
}
