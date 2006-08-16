#include "gnuhash.h"
#include <vector>
class Animation;
class Unit;

class Pilot {
  unsigned char gender;//which sound should play
  float reaction_time;
  float rank;
  int faction;//duplicate data...any way round this??
public:
  Pilot(int faction);
  void SetComm(Unit * comm_unit);//so we can specialize base sort of people
  float getReactionTime() {return reaction_time;}
  float getGender() {return gender;}
  float getRank(){return rank;}//man it's rank in here
  typedef stdext::hash_map<const Unit*,float> relationmap;
  relationmap effective_relationship;
  std::vector <Animation *> *comm_face;
  float getAnger (const Unit * un)const;
  std::vector <Animation *> *getCommFaces(unsigned char &sex){return comm_face;}
  float GetEffectiveRelationship (const Unit * target)const ;
  Animation * getCommFace(Unit * parent, float moon, unsigned char &gender);
};
