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
	float explosiontime;
	enum FOGMODE fogmode; // 0=OFF (I won't use this), 1=EXP, 2=EXP2, 3=LINEAR
	bool fogme;
	void LoadXML(const char * filename);
	void beginElem(const std::string&,const AttributeList&);
public:
	virtual enum clsptr isUnit() {return NEBULAPTR;}
	virtual void reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist);
  
protected:
    /// constructor only to be called by UnitFactory
    Nebula( const char * unitfile,
            bool SubU,
	    int faction,
	    Flightgroup* fg=NULL,
	    int fg_snumber=0 );

    friend class UnitFactory;

public:
	virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
	void SetFogState();

private:
    /// default constructor forbidden
    Nebula( );
    /// copy constructor forbidden
    Nebula( const Nebula& );
    /// assignment operator forbidden
    Nebula& operator=( const Nebula& );
};

#endif
