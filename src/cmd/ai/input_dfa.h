#ifndef _CMD_INPUT_DFA_H_
#define _CMD_INPUT_DFA_H_
#include "in_kb.h"
#include "in_mouse.h"
#include "gfx/camera.h"
#include "gfx/sprite.h"

//#include "gfx_click_list.h"
class StarSystem;
class UnitCollection;
class ClickList;
class VSSprite;
class OrderFactory;
class CoordinateSelect;



class InputDFA {
  enum State {NONE, UNITS_SELECTED, LOCATION_SELECT, TARGET_SELECT};
  OrderFactory *orderfac;
  bool queueOrder;
  CoordinateSelect * locsel;
  VSSprite MouseArrow;
  VSSprite SelectBox;

  //wish the mouse-bound functions could be member functions
  static void ClickSelect (KBSTATE, int,int,int,int,int);
  static void TargetSelect (KBSTATE,int,int,int,int,int);
  static void LocSelect (KBSTATE,int,int,int,int,int);
  static void NoneSelect (KBSTATE,int,int,int,int,int);
  void SetStateDragSelect();
  void SetStateSomeSelected();
  void SetStateNone();
  //bindorders:: UNITS_SELCTED  ->  TARGET_SELECT or LocationSelect
  void SetState(State st);
  State state;
  StarSystem *parentSystem;
  ClickList *clickList;

  UnitCollection * selected;
  UnitCollection * targetted;
  void ContextSwitch();
  bool Selecting;//is the selection box being drawn
  int prevx;
  int prevy; 
  //  enum State startOver ();
  //  enum State someSelected();
  //  enum State dragSelect();
  void replaceCollection (UnitCollection * newcol);
  void appendCollection (UnitCollection *newcol);
  void NewLocationSelect();
public:
  InputDFA (StarSystem * par);
  ~InputDFA();
  
  void Draw();
  void SetOrder (OrderFactory * ofac);
  static void BindOrder(int,OrderFactory *);
  void ContextAcquire();
  void UnselectAll();
  UnitCollection *getCollection();
  static void OrderHandler (const KBData&,KBSTATE);
};
#endif
