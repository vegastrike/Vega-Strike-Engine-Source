#include "input_dfa.h"
#include "cmd/click_list.h"
#include "gfx/sprite.h"
//#include "vegastrike.h"
#include "cmd/unit_generic.h"
#include "order.h"
#include "gfx/coord_select.h"
#include "vs_globals.h"
#include "in_kb_data.h"
//needed as functions bound to keys may not be class member functions--Context switch handles it
InputDFA *CurDFA=NULL;

#define ACTIVE_SHIFT               1
#define ACTIVE_CTRL                2
#define ACTIVE_ALT                 4
OrderFactory *orderbindings [KEYMAP_SIZE]={NULL};

static float GetX (float x) {
  return ((float)2*x)/g_game.x_resolution-1;
}
static float GetY (float x) {
  return 1-((float)2*x)/g_game.y_resolution;
}

void InputDFA::OrderHandler (const KBData& keys, KBSTATE k) {
  int key = atoi(keys.data.c_str());
  if (k==PRESS) {
    if (k<='Z'&&k>='A')
      CurDFA->queueOrder=true;
    else
      CurDFA->queueOrder=false;
    if (orderbindings[key]->type()&Order::SLOCATION) {
      CurDFA->NewLocationSelect(); //set up us the location select
    }
    CurDFA->SetOrder(orderbindings[key]);
    if (orderbindings[key]->type()&Order::SLOCATION) {
      CurDFA->SetState (LOCATION_SELECT);
      CurDFA->ContextAcquire();
    }

  }
}
void InputDFA::NewLocationSelect(){
  if (selected==NULL)
    return;
  Unit * un;
  int cnt=0;
  Vector RunningTotal(0,0,0);
  for(un_iter ui = selected->createIterator();un = *ui;++ui){
    RunningTotal+=un->Position().Cast();
    cnt++;
  }
  if (cnt==0) return;
  RunningTotal = (RunningTotal.Scale((1./cnt)));
  if (locsel)
    delete locsel;

  locsel = new CoordinateSelect (RunningTotal.Cast());
}
void InputDFA::BindOrder (int key, OrderFactory *ofac){
  /*  if (orderbindings[KEYMAP_SIZE]) 
    delete orderbindings[KEYMAP_SIZE];
  */
  orderbindings[key]= ofac;
  char num[256];
  sprintf(num,"%d",key);
#ifdef USE_INPUT_DFA
  BindKey (key,0,InputDFA::OrderHandler,num);
#endif
}
void InputDFA::SetOrder (OrderFactory *ofac) {
  if (orderfac) {
    ///no don't delete!!! will still be bound to a key?    delete orderfac;
  }
  orderfac = ofac;
}
//Equiv of nonselect, but for Targets instead of selected ships
void InputDFA::TargetSelect (KBSTATE k,int x,int y, int delx, int dely, int mod) {
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  CurDFA->state=TARGET_SELECT;//to fool the Noneselect function into using targets
  //don't bind keys above...."quiet state update"
  if (mod&ACTIVE_CTRL) {
    mod-=ACTIVE_CTRL;
  }
  NoneSelect (k,x,y,delx,dely,mod);
  if (CurDFA->state==TARGET_SELECT) {
    //executeOrders from selected->target;
    if (k==RELEASE&&CurDFA->targetted!=NULL) {
      Unit * un;
	  for(un_iter tmp = CurDFA->selected->createIterator();un = *tmp;++tmp){
	Order * nAI = CurDFA->orderfac->newOrder();
	if (CurDFA->targetted) {
	  Unit *tar = CurDFA->targetted->u.front();
	  if (tar) {
	    nAI->AttachOrder(tar);
	  }
	}
	if (CurDFA->queueOrder) {
	  un->EnqueueAI(nAI);
	} else {
	  un->SetAI(nAI);//will take care of doing the setparent 
	}
      }
      delete CurDFA->targetted;
      CurDFA->targetted=NULL;  

      CurDFA->orderfac = NULL;//I know we don't dealloc
    }
    CurDFA->SetStateNone(); //go back up the heirarchy;
  }
}

void InputDFA::LocSelect (KBSTATE k, int x, int y, int delx, int dely, int mod) {
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  CoordinateSelect::MouseMoveHandle(k,x,y,delx,dely,mod);

  if (k==PRESS) {
    
      Unit * un;
      Vector tmplocselvec = CurDFA->locsel->GetVector().Cast();
	  for(un_iter tmp = CurDFA->selected->createIterator();un = *tmp;++tmp){
	Order * nAI = CurDFA->orderfac->newOrder();
	nAI->AttachOrder(tmplocselvec.Cast());
	if (CurDFA->queueOrder) {
	  un->EnqueueAI(nAI);
	} else {
	  un->SetAI(nAI);//will take care of doing the setparent 
	}
      }
      delete CurDFA->targetted;
      CurDFA->targetted=NULL;  
      
      CurDFA->orderfac = NULL;//I know we don't dealloc
  }
  CurDFA->SetStateNone(); //go back up the heirarchy;
}



void InputDFA::ClickSelect (KBSTATE k, int x, int y, int delx, int dely, int mod) {
  static int kmod;
  //  Vector v = GFXDeviceToEye(x,y);
  float xs,ys;
  CurDFA->MouseArrow.GetSize (xs,ys);
  CurDFA->MouseArrow.SetPosition (.5*xs+GetX(x), .5*ys+GetY(y));

  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again

  if (k==PRESS) {

    CurDFA->Selecting=false;
    kmod = mod;
    CurDFA->prevx=x;
    CurDFA->prevy=y;
  }
  if (kmod&ACTIVE_CTRL/*Do something similar with special right mouse button*/) {
    //CurDFA->SetOrder (new AttackOrderFactory);
    if (kmod&ACTIVE_SHIFT) {
      CurDFA->queueOrder =true;
    } else {
      CurDFA->queueOrder = false;
    }
    CurDFA->SetOrder (new OrderFactory());
  }


  if (CurDFA->orderfac!=NULL) {
    if (CurDFA->orderfac->type()&Order::SLOCATION) {
      LocSelect (k,x,y,delx,dely,kmod);
      return;
    }
    if (CurDFA->orderfac->type()&Order::STARGET||CurDFA->orderfac->type()&Order::SSELF) {
      TargetSelect(k,x,y,delx,dely,kmod);//add some provision for binding keys to orders
      return;
    }
    CurDFA->orderfac = NULL;//??
    return;
  }
  if (k==PRESS){

    //  Vector v = GFXDeviceToEye(x,y);
  CurDFA->SelectBox.SetPosition (GetX(x), GetY(y));
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection();
      tmpcollection->append(sel);
      if (mod&ACTIVE_SHIFT) {      
	CurDFA->appendCollection(tmpcollection);
	VSFileSystem::vs_fprintf (stderr,"Select:appendingselected\n");
      } else {
	CurDFA->replaceCollection (tmpcollection);
	VSFileSystem::vs_fprintf (stderr,"Select:replacingselected\n");
      }
//      cerr << *sel << endl;
      CurDFA->SetStateSomeSelected();
    }else {
      if (!(mod&ACTIVE_SHIFT)){
	CurDFA->UnselectAll();
	CurDFA->SetStateNone();
	VSFileSystem::vs_fprintf (stderr,"Select:missedReplaceselected\n");
      }
    }
  }
  if (k==DOWN) {
    if (delx||dely) {
      //      Vector v = GFXDeviceToEye(x-CurDFA->prevx, y-CurDFA->prevy);// - GFXDeviceToEye(0,0);
      float dumbx,dumby;
      dumbx = -GetX(x)+GetX(CurDFA->prevx);
      dumby = -GetY(y)+GetY(CurDFA->prevy);
      CurDFA->SelectBox.SetSize (-dumbx,-dumby); 
      CurDFA->SelectBox.SetPosition (GetX(CurDFA->prevx)+.5*(GetX(x)-GetX(CurDFA->prevx)), GetY(CurDFA->prevy)+.5*(GetY(y)-GetY(CurDFA->prevy)));

      CurDFA->Selecting=true;
      if (mod&ACTIVE_SHIFT) {
	//do clickb0x0rz on both CurDFA->selection && tmpcol FIXME
	/*CurDFA->appendCollection(tmpcol)*/;
      }else{
	//do clickb0x0rz on tmpcolFIXME
	//	CurDFA->replaceCollection(tmpcol);//so you can see click boxes around stuff
      }
    }
  }
  if (k==RELEASE&&CurDFA->Selecting) {
    CurDFA->Selecting=false;
    UnitCollection *tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
    if (!(kmod&ACTIVE_SHIFT)){
      CurDFA->replaceCollection(tmpcol);
	  if(!tmpcol->u.front()){
	CurDFA->SetStateNone();
	VSFileSystem::vs_fprintf (stderr,"SelectBoxMissed\n");
      } else 
	VSFileSystem::vs_fprintf (stderr,"SelectBoxReplace\n");
    }else {
      VSFileSystem::vs_fprintf (stderr,"Select:SelectBoxAppending\n");
      CurDFA->appendCollection(tmpcol);
    }
  }
}


//this function is bound in the NONE state...
void InputDFA::NoneSelect (KBSTATE k,int x, int y, int delx, int dely, int mod) {
  //  Vector v = GFXDeviceToEye(x,y);
  float xs,ys;

  CurDFA->MouseArrow.GetSize (xs,ys);
  CurDFA->MouseArrow.SetPosition (.5*xs+GetX(x), .5*ys+GetY(y));

  static int kmod;
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  if (mod&ACTIVE_CTRL)
    return; //you don't want control pressed
  if (k==PRESS) {
    //    Vector v = GFXDeviceToEye(x,y);
    CurDFA->SelectBox.SetPosition (GetX(x), GetY(y));
    CurDFA->Selecting=false;
    kmod = mod;
    CurDFA->prevx=x;
    CurDFA->prevy=y;
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection();
      tmpcollection->append(sel);
      VSFileSystem::vs_fprintf (stderr,"None::replacing Single Unit");if (CurDFA->state==TARGET_SELECT) VSFileSystem::vs_fprintf (stderr," to target\n");else VSFileSystem::vs_fprintf (stderr," to select\n");

//      cerr << *sel << endl;
      
      CurDFA->replaceCollection (tmpcollection);
      CurDFA->SetStateSomeSelected();
    }else {
      CurDFA->UnselectAll();
      VSFileSystem::vs_fprintf (stderr,"None::missed\n");if (CurDFA->state==TARGET_SELECT) VSFileSystem::vs_fprintf (stderr," to target");else VSFileSystem::vs_fprintf (stderr," to select");
    }
  }
  
  if (k==DOWN) {
    if (delx||dely) {
      //      Vector v = GFXDeviceToEye(x-CurDFA->prevx, y-CurDFA->prevy) - GFXDeviceToEye(0,0);
      float dumbx,dumby;
      dumbx = -GetX(x)+GetX(CurDFA->prevx);
      dumby = -GetY(y)+GetY(CurDFA->prevy);
      CurDFA->SelectBox.SetSize (-dumbx,-dumby); 
      CurDFA->SelectBox.SetPosition (GetX(CurDFA->prevx)+.5*(GetX(x)-GetX(CurDFA->prevx)), GetY(CurDFA->prevy)+.5*(GetY(y)-GetY(CurDFA->prevy)));
      CurDFA->Selecting=true;
    }
  }
  if (k==RELEASE&&CurDFA->Selecting) {
    CurDFA->Selecting=false;
    UnitCollection *tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
    CurDFA->replaceCollection(tmpcol);
	Unit *tUnit;
	if(tmpcol->u.front())
		CurDFA->SetStateSomeSelected();
/*		
	for(un_iter tmp2 = tmpcol->createIterator();tUnit = *tmp2;++tmp2){
      VSFileSystem::vs_fprintf (stderr,"None::replacing SelectBox Units");if (CurDFA->state==TARGET_SELECT) VSFileSystem::vs_fprintf (stderr," to target");else VSFileSystem::vs_fprintf (stderr," to select");
      while(tmp2->current()) {
	tmp2->advance();
      }
      CurDFA->SetStateSomeSelected();
    }else {
      VSFileSystem::vs_fprintf (stderr,"None::select box missed");
    }
    delete tmp2;
*/
  }
}

InputDFA::InputDFA (StarSystem * par) :MouseArrow ("mouse.spr"), SelectBox("selectbox.spr"){
  parentSystem= par;
  clickList = parentSystem->getClickList();
  state = NONE;
  locsel=NULL;
  ContextAcquire();//binds keys, etc
  Selecting=false;
  selected = NULL;
  targetted = NULL;
  orderfac = NULL;
}

InputDFA::~InputDFA() {
  delete clickList;
  if (locsel) 
    delete locsel;
}
/**
enum State InputDFA::startOver() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    return NONE;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    return UNITS_SELECTED;
  }
  return NONE;
}

enum State InputDFA::someSelected() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    return UNITS_SELECTED;
  case LOCATION_SELECT:
    return NONE;//invalid
  case TARGET_SELECT:
    return TARGET_SELECT;
  }
  return UNITS_SELECTED;
}
*/
UnitCollection * InputDFA::getCollection () {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    return selected;
  case TARGET_SELECT:
    return targetted;
  }
  return selected;
}
void InputDFA::UnselectAll() {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:   
    if (selected) {
	  Unit *tUnit;
	  for(un_iter it = selected->createIterator();tUnit = *it;++it)
		tUnit->Deselect();
      delete selected;
      selected = NULL;
    }
    
    break;
  case TARGET_SELECT:
    if (targetted) {
      delete targetted;
      targetted = NULL;
    }
    break;
  }
}
void InputDFA::replaceCollection (UnitCollection *newcol) {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    UnselectAll();
    selected = newcol;
    Unit *tUnit;
    for(un_iter it = selected->createIterator();tUnit = *it;++it){
      tUnit->Select();
    }
    break;
  case TARGET_SELECT:
    UnselectAll();
    targetted = newcol;
    break;
  }
}

void InputDFA::appendCollection (UnitCollection *newcol) {
  switch (state) {
  case LOCATION_SELECT:
  case UNITS_SELECTED:
  case NONE:
    if (selected) {
		Unit *tUnit;
	  for(un_iter it = newcol->createIterator();tUnit = *it;++it)
		tUnit->Select();
    
      UnitCollection::UnitIterator *tmpit =new un_iter(newcol->createIterator());
      selected->append (tmpit);
      delete tmpit;
      delete newcol;
      //remove duplicates FIXME
    } else {
      selected=newcol;
    }
    break;
  case TARGET_SELECT:
    if (targetted) {
	  un_iter tmpit = newcol->createIterator();
      targetted->append (&tmpit);
      delete newcol;
      //remove duplicates FIXME
    }else {
      targetted = newcol;
    }
    break;	
  }
}

void InputDFA::SetStateNone() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    SetState (NONE);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (UNITS_SELECTED);
    break;

  }
}

/*
void InputDFA::SetStateDragSelect() {
  switch (state) {
  case NONE:
  case UNITS_SELECTED:
    SetState (CLICK_DRAG);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (TARGET_DRAG);
    break;
  }
}
*/
void InputDFA::SetStateSomeSelected() {
  switch (state){
  case NONE:
  case UNITS_SELECTED:
    SetState (UNITS_SELECTED);
    break;
  case LOCATION_SELECT:
  case TARGET_SELECT:
    SetState (TARGET_SELECT);
    break;
  }
}

void InputDFA::SetState (State st) {
  state = st;
  ContextAcquire();
}

void InputDFA::Draw () {
  GFXBlendMode (ONE,ONE);
  switch (state) {
  case NONE:	//draw arrow
    if (Selecting) { 
      GFXDisable(CULLFACE);
      SelectBox.Draw();
      GFXEnable(CULLFACE);
    }
    MouseArrow.Draw();

  break;
  case UNITS_SELECTED: //draw arrow, boxes
    if (Selecting) {
      GFXDisable(CULLFACE);
      SelectBox.Draw();
      GFXEnable(CULLFACE);
    }
    MouseArrow.Draw();
    
    break;
  case LOCATION_SELECT:
    if (locsel)
      locsel->Draw();
    break;
  case TARGET_SELECT:
    if (Selecting) {
      SelectBox.Draw();
    }
    MouseArrow.Draw();
    
    break;
  }
}

void InputDFA::ContextAcquire() {
  if (CurDFA!=NULL){
    CurDFA->ContextSwitch();
  }
  CurDFA=this;
#ifdef USE_INPUT_DFA
  switch (state) {
  case NONE:	BindKey (0,NoneSelect);
  break;
  case UNITS_SELECTED:BindKey (0,ClickSelect);
    break;
  case LOCATION_SELECT:BindKey (0,LocSelect);
    break;
  case TARGET_SELECT:BindKey (0,ClickSelect);
    break;
  }
#endif
}

/** ContextSwitch unbinds the respective keys and appropriately to the given state*/
void InputDFA::ContextSwitch (){
#ifdef USE_INPUT_DFA
  switch (state) {
  case NONE:UnbindMouse(0);
      break;
  case UNITS_SELECTED:UnbindMouse(0);
    break;
  case LOCATION_SELECT:UnbindMouse(0);
    break;
  case TARGET_SELECT:UnbindMouse(0);
    break;
  }
#endif
}

