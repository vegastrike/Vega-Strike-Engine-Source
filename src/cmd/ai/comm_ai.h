#include "order.h"
/// this class is meant to be inherited by AI wishing to process comm messages
class CommunicatingAI:public Order{
 public:
   CommunicatingAI (int ttype);
   virtual void ProcessCommMessage (class CommunicationMessage &c); 
   virtual void AdjustRelationTo (Unit * un, float factor);
   virtual ~CommunicatingAI ();
   virtual int selectCommunicationMessage (class CommunicationMessage &c);
};
