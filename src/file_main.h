#include <stdio.h>
#include <string.h>
#include "gfxlib.h"

extern FILE *fpread;

/*File utility functions*/
inline void LoadFile(char *filename)
{
	fpread = fopen(filename, "rb");
}
inline void CloseFile()
{
	fclose(fpread);
}

/*Read simple data*/
inline void ReadInt(int &integer)
{
	fread(&integer, sizeof(int), 1, fpread);
}
inline void ReadFloat(float &num)
{
	fread(&num, sizeof(float), 1, fpread);
}

inline void ReadString(char *string)
{
	int length = strlen(string);

	ReadInt(length);
	fread(string, length, 1, fpread);
	string[length] = '\0';
}

/*Read aggregated data*/
inline void ReadVector(float &x, float &y, float &z)
{
	ReadFloat(x);
	ReadFloat(y);
	ReadFloat(z);
}

inline void ReadVector(Vector &v)
{
	ReadVector(v.i, v.j, v.k);
}

inline void ReadGeneric(char *string, float &x, float &y, float &z)
{
	ReadString(string);
	ReadVector(x,y,z);
}

/*The goods*/
inline void ReadUnit(char *filename, int &type, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadMesh(char *filename, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadWeapon(char *filename, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadRestriction(int &isrestricted, float &start, float &end)
{
	ReadInt(isrestricted);
	ReadFloat(start);
	ReadFloat(end);
}

inline long GetPosition()
{
	return ftell(fpread);
}

inline void SetPosition(long position)
{
	fseek(fpread,position,SEEK_SET);
}