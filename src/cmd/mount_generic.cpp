#include "unit_generic.h"

void Mount::ReplaceMounts (const Mount * other) {
	short thisvol = volume;
	short thissize = size;
	Quaternion q =this->GetMountOrientation();
	Vector v = this->GetMountLocation();
	*this=*other;
	this->size=thissize;
	volume=thisvol;
	this->SetMountPosition(v);
	this->SetMountOrientation(q);	
	ref.gun=NULL;
	this->ReplaceSound();
}
double Mount::Percentage (const Mount *newammo) const{
	  float percentage=0;
	  int thingstocompare=0;
	  if (status==UNCHOSEN||status==DESTROYED)
		return 0;
	  if (newammo->ammo==-1) {
		if (ammo!=-1) {
		  thingstocompare++;
		}
	  } else {
		if (newammo->ammo>0) {
		  percentage+=ammo/newammo->ammo;
		  thingstocompare++;
		}
	  }
	  if (newammo->type->Range) {
		percentage+= type->Range/newammo->type->Range;
		thingstocompare++;
	  }
	  if (newammo->type->Damage+100*newammo->type->PhaseDamage) {
		percentage += (type->Damage+100*type->PhaseDamage)/(newammo->type->Damage+100*newammo->type->PhaseDamage);
		thingstocompare++;
	  }
	  if (thingstocompare) {
		return percentage/thingstocompare;
	  }else {
		return 0;
	  }
}
