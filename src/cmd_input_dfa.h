#ifndef _CMD_INPUT_DFA_H_
#define _CMD_INPUT_DFA_H_
#include "in_kb.h"
#include "in_mouse.h"
#include "gfx_camera.h"
#include "gfx_sprite.h"

//#include "gfx_click_list.h"
class StarSystem;
class UnitCollection;
class ClickList;
class Sprite;
class OrderFactory;


void BindOrder(int,OrderFactory *);

class InputDFA {
  enum State {NONE, UNITS_SELECTED, LOCATION_SELECT, TARGET_SELECT};
  OrderFactory *orderfac;
  bool queueOrder;

  Sprite MouseArrow;
  Sprite SelectBox;

  //wish the mouse-bound functions could be member functions
  static void ClickSelect (KBSTATE, int,int,int,int,int);
  static void TargetSelect (KBSTATE,int,int,int,int,int);
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
public:
  InputDFA (StarSystem * par);
  ~InputDFA();
  void Draw();
  void SetOrder (OrderFactory * ofac);
  void ContextAcquire();
  void UnselectAll();
  UnitCollection *getCollection();
  static void OrderHandler (int,KBSTATE);
};
#endif
