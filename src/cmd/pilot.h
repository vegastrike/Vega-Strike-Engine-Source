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
  unsigned char getGender() {return gender;}
  float getRank(){return rank;}//man it's rank in here
  typedef stdext::hash_map<const void*,float> relationmap;//non dereferencable Unit to float
  relationmap effective_relationship;
  std::vector <Animation *> *comm_face;
  float getAnger (const Unit *parent, const Unit * un)const;
  std::vector <Animation *> *getCommFaces(unsigned char &sex){sex=gender;return comm_face;}
  float GetEffectiveRelationship (const Unit * parent, const Unit * target) const ;
  Animation * getCommFace(Unit * parent, float moon, unsigned char &gender);
  float adjustSpecificRelationship(Unit * parent, void* aggressor, float value, int guessedFaction/*pass in neutral otherwise*/);
  void DoHit(Unit* parent, void * aggressor, int guessedFaction/*pass in neutral otherwise*/);
};
