/*  
 *  Copyright 2002-2003 LibPK3, Inc. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *  Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *  Redistributions in binary form must reproduce the above copyright 
 *  notice, this list of conditions and the following disclaimer in 
 *  the documentation and/or other materials provided with the 
 *  distribution. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE LIBPK3 PROJECT 
 *  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
 *  BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 *  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE LIBPK3 PROJECT OR CONTRIBUTORS 
 *  BE LIABLE FORANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 *  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
 *  DAMAGE.
 *
 *  The views and conclusions contained in the software and 
 *  documentation are those of the authors and should not be 
 *  interpreted as representing official policies, either expressed or 
 *  implied, of the LibPK3 Project or Viktor Liehr alias picard.
 *
 *  $Id$
 *
 */

/* 
 * Modified by Stephane Vaxelaire on 2003/09/23
 * - Turned longs into ints in the headers (long are 8 bytes on some (all?) 64 bits archs)
 * - Added endianness support (was not working on big endian machine)
 */

#include "pk3.h"
#include <iostream>
#include "posh.h"
using std::cerr;
using std::endl;
using std::hex;

#pragma pack(2)

struct CPK3::TZipLocalHeader
{
  enum
  {
    SIGNATURE = 0x04034b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,
  };
  unsigned int    sig;
  unsigned short  version;
  unsigned short  flag;
  unsigned short  compression;      // COMP_xxxx
  unsigned short  modTime;
  unsigned short  modDate;
  unsigned int    crc32;
  unsigned int    cSize;
  unsigned int    ucSize;
  unsigned short  fnameLen;         // Filename string follows header.
  unsigned short  xtraLen;          // Extra field follows filename.

  void	correctByteOrder()
  {
  	sig = POSH_LittleU32( sig);
	version = POSH_LittleU16( version);
	flag = POSH_LittleU16( flag);
	compression = POSH_LittleU16( compression);
	modTime = POSH_LittleU16( modTime);
	modDate = POSH_LittleU16( modDate);
  	crc32 = POSH_LittleU32( crc32);
  	cSize = POSH_LittleU32( cSize);
  	ucSize = POSH_LittleU32( ucSize);
	fnameLen = POSH_LittleU16( fnameLen);
	xtraLen = POSH_LittleU16( xtraLen);
  }
};

struct CPK3::TZipDirHeader
{
  enum
  {
    SIGNATURE = 0x06054b50,
  };
  unsigned int    sig;
  unsigned short  nDisk;
  unsigned short  nStartDisk;
  unsigned short  nDirEntries;
  unsigned short  totalDirEntries;
  unsigned int    dirSize;
  unsigned int    dirOffset;
  unsigned short  cmntLen;

  void	correctByteOrder()
  {
  	sig = POSH_LittleU32( sig);
	nDisk = POSH_LittleU16(nDisk);
	nStartDisk = POSH_LittleU16(nStartDisk);
	nDirEntries = POSH_LittleU16(nDirEntries);
	totalDirEntries = POSH_LittleU16(totalDirEntries);
  	dirSize = POSH_LittleU32( dirSize);
  	dirOffset = POSH_LittleU32( dirOffset);
	cmntLen = POSH_LittleU16( cmntLen);
  }
};

struct CPK3::TZipDirFileHeader
{
  enum
  {
    SIGNATURE   = 0x02014b50,
    COMP_STORE  = 0,
    COMP_DEFLAT = 8,
  };
  unsigned int    sig;
  unsigned short  verMade;
  unsigned short  verNeeded;
  unsigned short  flag;
  unsigned short  compression;      // COMP_xxxx
  unsigned short  modTime;
  unsigned short  modDate;
  unsigned int    crc32;
  unsigned int    cSize;            // Compressed size
  unsigned int    ucSize;           // Uncompressed size
  unsigned short  fnameLen;         // Filename string follows header.
  unsigned short  xtraLen;          // Extra field follows filename.
  unsigned short  cmntLen;          // Comment field follows extra field.
  unsigned short  diskStart;
  unsigned short  intAttr;
  unsigned int    extAttr;
  unsigned int    hdrOffset;

  char *GetName   () const { return (char *)(this + 1);   }
  char *GetExtra  () const { return GetName() + fnameLen; }
  char *GetComment() const { return GetExtra() + xtraLen; }

  void	correctByteOrder()
  {
  	sig = POSH_LittleU32( sig);
	verMade = POSH_LittleU16( verMade);
	verNeeded = POSH_LittleU16( verNeeded);
	flag = POSH_LittleU16( flag);
	compression = POSH_LittleU16( compression);
	modTime = POSH_LittleU16( modTime);
	modDate = POSH_LittleU16( modDate);
  	crc32 = POSH_LittleU32( crc32);
  	cSize = POSH_LittleU32( cSize);
  	ucSize = POSH_LittleU32( ucSize);
	fnameLen = POSH_LittleU16( fnameLen);
	xtraLen = POSH_LittleU16( xtraLen);
	cmntLen = POSH_LittleU16( cmntLen);
	diskStart = POSH_LittleU16( diskStart);
	intAttr = POSH_LittleU16( intAttr);
  	extAttr = POSH_LittleU32( extAttr);
  	hdrOffset = POSH_LittleU32( hdrOffset);
  }
};

#pragma pack()

CPK3::CPK3(FILE *n_f)
{
  CheckPK3(n_f);
}

CPK3::CPK3(const char *filename)
{
  Open(filename);
}


bool CPK3::CheckPK3(FILE *f)
{
  if (f == NULL)
    return false;

  TZipDirHeader dh;

  fseek(f, -(int)sizeof(dh), SEEK_END);

  long dhOffset = ftell(f);
  memset(&dh, 0, sizeof(dh));
  
  fread(&dh, sizeof(dh), 1, f);
  dh.correctByteOrder();

  // Check
  if (dh.sig != TZipDirHeader::SIGNATURE)
  {
  	cerr<<"PK3 -- BAD DIR HEADER SIGNATURE, NOT A PK3 FILE !"<<endl;
	exit(1);
    return false;
  }

  // Go to the beginning of the directory.
  fseek(f, dhOffset - dh.dirSize, SEEK_SET);

  // Allocate the data buffer, and read the whole thing.
  m_pDirData = new char[dh.dirSize + dh.nDirEntries*sizeof(*m_papDir)];
  
  if (!m_pDirData)
  {
  	cerr<<"PK3 -- ERROR ALLOCATING DATA BUFFER !"<<endl;
	exit(1);
    return false;
  }
  
  memset(m_pDirData, 0, dh.dirSize + dh.nDirEntries*sizeof(*m_papDir));
  fread(m_pDirData, dh.dirSize, 1, f);

  // Now process each entry.
  char *pfh = m_pDirData;
  m_papDir = (const TZipDirFileHeader **)(m_pDirData + dh.dirSize);

  bool ret = true;


  for (int i = 0; i < dh.nDirEntries && ret == true; i++)
  {
    TZipDirFileHeader &fh = *(TZipDirFileHeader*)pfh;
	fh.correctByteOrder();

    // Store the address of nth file for quicker access.
    m_papDir[i] = &fh;
    // Check the directory entry integrity.
    if (fh.sig != TZipDirFileHeader::SIGNATURE)
	{
		cerr<<"PK3 -- ERROR BAD DIRECTORY SIGNATURE !"<<endl;
		exit(1);
		ret = false;
	}
    else
    {
      pfh += sizeof(fh);


      // Convert UNIX slashes to DOS backlashes.
      for (int j = 0; j < fh.fnameLen; j++)
        if (pfh[j] == '/')
          pfh[j] = '\\';

      // Skip name, extra and comment fields.
      pfh += fh.fnameLen + fh.xtraLen + fh.cmntLen;
    }
  }
  if (ret != true)
    delete[] m_pDirData;
  else
  {
    m_nEntries = dh.nDirEntries;
    this->f = f;
  }

  return ret;
}

bool CPK3::Open( const char *filename)
{
  f = fopen(filename, "rb");
  if(f)
  {
    strcpy(pk3filename,filename);
    return CheckPK3(f);
  }
  else
  {
    return false;
  }
}

bool CPK3::ExtractFile( const char *lp_name)
{
  return ExtractFile(lp_name,lp_name);
}

bool CPK3::ExtractFile( const char *lp_name, const char *new_filename)
{
  // open file tp write data
  FILE *new_f = NULL;
  int size = -1;

  char *data_content = ExtractFile(lp_name,&size);
  
  if(data_content)
  {
    if(size != -1)
    {
      new_f = fopen(new_filename,"wb");
      fwrite(data_content,1,size,new_f);
      fclose(new_f);
      delete data_content;
      return true;
    }
  }
  
  return false; // probably file not found
}

// Compares 2 c-strings but do not take into account '/' or '\'
int vsstrcmp( const char * lp, const char * str)
{
	unsigned int i, ok=1;
	unsigned int len = strlen( lp);
	for( i=0; ok && i<len; i++)
	{
		if( lp[i]!='/' && lp[i]!='\\' && lp[i]!=str[i])
			ok = 0;
	}
	return !ok;
}

int  CPK3::FileExists( const char * lpname)
{
  char	    str[PK3LENGTH];
  int		idx = -1; 

  memset(&str, 0, sizeof(str));
  //cerr<<"\t\tSearching through "<<m_nEntries<<" files"<<endl;
  for(int i=1; idx==-1 && i<m_nEntries; i++)
  {
		GetFilename(i, str);
		//cerr<<"\t\tExamining file num."<<i<<" filename : "<<str<<"\tand compared to : "<<lpname<<endl;
		int result = vsstrcmp(lpname,str);
		
    	if(result == 0)
		{
		  cerr<<"FOUND IN PK3 FILE : "<<lpname<<" with index="<<i<<endl;
		  idx = i;
		}
  }
  
  /*
  if( idx==-1)
  	cerr<<"DIDN'T FIND FILE : "<<lpname<<endl;
  */
  // if the file isn't in the archive idx=-1
  return idx;
}

char *CPK3::ExtractFile( int index, int *file_size)
{
  char *buffer;
  int flength = GetFileLen(index);
     
  buffer = new char[flength];

  if (!buffer)
  {
    cerr<<"Unable to allocate memory, probably to low memory !!!"<<endl;
    return NULL;
  }
   else 
  {
    if (true == ReadFile(index, buffer))
    {
      // everything went well !!!
    }
    else
    {
      cerr<<"\nThe file was found in the archive, but I was unable to extract it. Maybe the archive is broken."<<endl;
    }
  }

  *file_size = flength;
  return buffer;
}

char *CPK3::ExtractFile(const char *lpname, int *file_size)
{
  char	    str[PK3LENGTH];
  int		index = -1;
  char *buffer;

  memset(&str, 0, sizeof(str));

  for(int i=1; index==-1 && i<m_nEntries; i++)
	{
		GetFilename(i, str);
    	//printf("%s",str);
		int result = vsstrcmp(lpname,str);
		
    if(result == 0)
		  index = i;
  }
  
  // if the file isn't in the archive
  if (index == -1)
    return false;

  int flength = GetFileLen(index);
     
  buffer = new char[flength];

  if (!buffer)
  {
    printf("Unable to allocate memory, probably to low memory !!!\n");
    return NULL;
  }
   else 
  {
    if (true == ReadFile(index, buffer))
    {
      // everything went well !!!
    }
    else
    {
      printf("\nThe file was found in the archive, but I was unable to "\
             "extract it. Maybe the archive is broken.\n");
    }
  }

  *file_size = flength;
  return buffer;
}

bool CPK3::Close()
{
  fclose(f);
  delete[] m_pDirData;
  m_nEntries = 0;

  return true;
}

void CPK3::PrintFileContent()
{
  printf("PK3 File: %s\n",pk3filename);
  printf("files count: %d\n\n",m_nEntries);
  for(int i = 0; i < m_nEntries; i++)
  {

  }
}

void CPK3::GetFilename(int i, char *pszDest)  const
{
  if (pszDest != NULL)
  {
    if (i < 0 || i >= m_nEntries)
      *pszDest = '\0';
    else
    {
      memcpy(pszDest, m_papDir[i]->GetName(), m_papDir[i]->fnameLen);
      pszDest[m_papDir[i]->fnameLen] = '\0';
    }
  }
  return;
}

int CPK3::GetFileLen(int i) const
{
  if (i < 0 || i >= m_nEntries)
    return -1;
  else
    return m_papDir[i]->ucSize;
}



bool CPK3::ReadFile(int i, void *pBuf)
{
  if (pBuf == NULL || i < 0 || i >= m_nEntries)
  {
    cerr<<"PK3ERROR : ";
	if( pBuf==NULL)
		cerr<<" pBuf is NULL !!!"<<endl;
	else if( i<0)
		cerr<<" Bad index < 0 !!!"<<endl;
	else if( i>= m_nEntries)
		cerr<<" Index TOO BIG !!!"<<endl;
    return false;
  }

  // Quick'n dirty read, the whole file at once.
  // Ungood if the ZIP has huge files inside

  // Go to the actual file and read the local header.
  fseek(this->f, m_papDir[i]->hdrOffset, SEEK_SET);
  TZipLocalHeader h;

  memset(&h, 0, sizeof(h));
  fread(&h, sizeof(h), 1, this->f);
  h.correctByteOrder();
  if (h.sig != TZipLocalHeader::SIGNATURE)
  {
    cerr<<"PK3 - BAD LOCAL HEADER SIGNATURE !!!"<<endl;
    return false;
  }

  // Skip extra fields
  fseek(this->f, h.fnameLen + h.xtraLen, SEEK_CUR);

  if (h.compression == TZipLocalHeader::COMP_STORE)
  {
    // Simply read in raw stored data.
    fread(pBuf, h.cSize, 1, this->f);
    return true;
  }
  else if (h.compression != TZipLocalHeader::COMP_DEFLAT)
  {
    cerr<<"BAD Compression level, found="<<h.compression<<" - expected="<<TZipLocalHeader::COMP_DEFLAT<<endl;
    return false;
  }

  // Alloc compressed data buffer and read the whole stream
  char *pcData = new char[h.cSize];
  if (!pcData)
  {
    cerr<<"PK3ERROR : Could not allocate memory buffer for decompression"<<endl;
    return false;
  }	

  memset(pcData, 0, h.cSize);
  fread(pcData, h.cSize, 1, this->f);

  bool ret = true;

  // Setup the inflate stream.
  z_stream stream;
  int err, err2;

  stream.next_in = (Bytef*)pcData;
  stream.avail_in = (uInt)h.cSize;
  stream.next_out = (Bytef*)pBuf;
  stream.avail_out = h.ucSize;
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  // Perform inflation. wbits < 0 indicates no zlib header inside the data.
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err == Z_OK)
  {
    err = inflate(&stream, Z_FINISH);
    if (err == Z_STREAM_END)
      err = Z_OK;
	else if( err==Z_NEED_DICT)
		cerr<<"PK3ERROR : Needed a dictionary"<<endl;
	else if( err==Z_DATA_ERROR)
		cerr<<"PK3ERROR : Bad data buffer"<<endl;
	else if( err==Z_STREAM_ERROR)
		cerr<<"PK3ERROR : Bad parameter, stream error"<<endl;
    err2 = inflateEnd(&stream);
	if( err2==Z_STREAM_ERROR)
		cerr<<"PK3ERROR : Bad parameter, stream error"<<endl;
    err2 = inflateEnd(&stream);
	if( err2==Z_STREAM_ERROR)
		cerr<<"PK3ERROR : Bad parameter, stream error"<<endl;
  }
  else
  {
  	if( err==Z_STREAM_ERROR)
		cerr<<"PK3ERROR : Bad parameter, stream error"<<endl;
	else if( err==Z_MEM_ERROR)
		cerr<<"PK3ERROR : Memory error"<<endl;
  }

  if (err != Z_OK)
  {
    cerr<<"PK3ERROR : Bad decompression return code"<<endl;
    ret = false;
  }	

  delete[] pcData;
  return ret;
}

