#ifndef _NEBULA_H_
#define _NEBULA_H_
#include "unit.h"

class Nebula: public Unit {
private:
	static void Nebula::beginElement (void * Userdata,const XML_Char * name, const XML_Char ** atts);
	Vector color;
	float Density;
	float fognear;
	float fogfar;
	int index;
	char fogmode; // 0=OFF (I won't use this), 1=EXP, 2=EXP2, 3=LINEAR
	void LoadXML(const char * filename);
	void beginElem(string&,AttributeList&);
public:
	virtual void reactToCollision (Unit *un, const Vector & normal, float dist);
	Nebula(const char * filename, const char * unitfile, bool SubU, int faction, Flightgroup* fg=NULL, int fg_snumber=0);
};

#endif