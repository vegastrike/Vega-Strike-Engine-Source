#include "cmd_input_dfa.h"
#include "gfx_click_list.h"
#include "gfx_sprite.h"
#include "vegastrike.h"
#include "cmd_unit.h"
#include "cmd_order.h"
#include "gfx_coordinate_select.h"
//needed as functions bound to keys may not be class member functions--Context switch handles it
InputDFA *CurDFA=NULL;

#define ACTIVE_SHIFT               1
#define ACTIVE_CTRL                2
#define ACTIVE_ALT                 4
OrderFactory *orderbindings [KEYMAP_SIZE]={NULL};
void InputDFA::OrderHandler (int key, KBSTATE k) {
  if (k==PRESS) {
    if (k<='Z'&&k>='A')
      CurDFA->queueOrder=true;
    else
      CurDFA->queueOrder=false;
    if (orderbindings[key]->type()&LOCATION) {
      CurDFA->NewLocationSelect(); //set up us the location select
    }
    CurDFA->SetOrder(orderbindings[key]);
    if (orderbindings[key]->type()&LOCATION) {
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
  UnitCollection::UnitIterator * ui = selected->createIterator();
  while (un=ui->current()) {
    RunningTotal+=un->Position();
    cnt++;
    ui->advance();
  }
  if (cnt==0) return;
  RunningTotal = (1./cnt)*(RunningTotal);
  if (locsel)
    delete locsel;

  locsel = new CoordinateSelect (RunningTotal);
}
void InputDFA::BindOrder (int key, OrderFactory *ofac){
  /*  if (orderbindings[KEYMAP_SIZE]) 
    delete orderbindings[KEYMAP_SIZE];
  */
  orderbindings[key]= ofac;
  BindKey (key,InputDFA::OrderHandler);
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
      UnitCollection::UnitIterator * tmp = CurDFA->selected->createIterator();
      Unit * un;
      while (un = tmp->current()) {
	Order * nAI = CurDFA->orderfac->newOrder();
	nAI->AttachOrder(CurDFA->targetted);
	if (CurDFA->queueOrder) {
	  un->EnqueueAI(nAI);
	} else {
	  un->SetAI(nAI);//will take care of doing the setparent 
	}
	tmp->advance();
      }
      delete tmp;
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
    
      UnitCollection::UnitIterator * tmp = CurDFA->selected->createIterator();
      Unit * un;
      Vector tmplocselvec = CurDFA->locsel->GetVector();
      while (un = tmp->current()) {
	Order * nAI = CurDFA->orderfac->newOrder();
	nAI->AttachOrder(tmplocselvec);
	if (CurDFA->queueOrder) {
	  un->EnqueueAI(nAI);
	} else {
	  un->SetAI(nAI);//will take care of doing the setparent 
	}
	tmp->advance();
      }
      delete tmp;
      delete CurDFA->targetted;
      CurDFA->targetted=NULL;  
      
      CurDFA->orderfac = NULL;//I know we don't dealloc
  }
  CurDFA->SetStateNone(); //go back up the heirarchy;
}



void InputDFA::ClickSelect (KBSTATE k, int x, int y, int delx, int dely, int mod) {
  static int kmod;
  Vector v = GFXDeviceToEye(x,y);
  CurDFA->MouseArrow.SetPosition (v.i, v.j);

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
    if (CurDFA->orderfac->type()&LOCATION) {
      LocSelect (k,x,y,delx,dely,kmod);
      return;
    }
    if (CurDFA->orderfac->type()&TARGET||CurDFA->orderfac->type()&SELF) {
      TargetSelect(k,x,y,delx,dely,kmod);//add some provision for binding keys to orders
      return;
    }
    CurDFA->orderfac = NULL;//??
    return;
  }
  if (k==PRESS){

  Vector v = GFXDeviceToEye(x,y);
  CurDFA->SelectBox.SetPosition (v.i, v.j);
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection;
      tmpcollection->append(sel);
      if (mod&ACTIVE_SHIFT) {      
	CurDFA->appendCollection(tmpcollection);
	fprintf (stderr,"Select:appendingselected\n");
      } else {
	CurDFA->replaceCollection (tmpcollection);
	fprintf (stderr,"Select:replacingselected\n");
      }
      cerr << *sel << endl;
      CurDFA->SetStateSomeSelected();
    }else {
      if (!(mod&ACTIVE_SHIFT)){
	CurDFA->UnselectAll();
	CurDFA->SetStateNone();
	fprintf (stderr,"Select:missedReplaceselected\n");
      }
    }
  }
  if (k==DOWN) {
    if (delx||dely) {
      Vector v = GFXDeviceToEye(x-CurDFA->prevx, y-CurDFA->prevy) - GFXDeviceToEye(0,0);
      CurDFA->SelectBox.SetSize (v.i, v.j);
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
      UnitCollection::UnitIterator * tmp2 = tmpcol->createIterator();
      if (!tmp2->current()) {
	CurDFA->SetStateNone();
	fprintf (stderr,"SelectBoxMissed\n");
      } else 
	fprintf (stderr,"SelectBoxReplace\n");//      SetStateSomeSelected(); already there
      delete tmp2;
    }else {
      fprintf (stderr,"Select:SelectBoxAppending\n");
      CurDFA->appendCollection(tmpcol);
    }
  }
}


//this function is bound in the NONE state...
void InputDFA::NoneSelect (KBSTATE k,int x, int y, int delx, int dely, int mod) {
  Vector v = GFXDeviceToEye(x,y);
  CurDFA->MouseArrow.SetPosition (v.i, v.j);
  static int kmod;
  if (k==RESET)
    return;///little hack to prevent the function from being 'primed' with reset and continuing on an infinite loop again and again and again
  if (mod&ACTIVE_CTRL)
    return; //you don't want control pressed
  if (k==PRESS) {
    Vector v = GFXDeviceToEye(x,y);
    CurDFA->SelectBox.SetPosition (v.i, v.j);
    CurDFA->Selecting=false;
    kmod = mod;
    CurDFA->prevx=x;
    CurDFA->prevy=y;
    Unit * sel = CurDFA->clickList->requestShip(x,y);
    if (sel!=NULL) {
      UnitCollection *tmpcollection=new UnitCollection;
      tmpcollection->append(sel);
      fprintf (stderr,"None::replacing Single Unit");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target\n");else fprintf (stderr," to select\n");

      cerr << *sel << endl;
      
      CurDFA->replaceCollection (tmpcollection);
      CurDFA->SetStateSomeSelected();
    }else {
      CurDFA->UnselectAll();
      fprintf (stderr,"None::missed\n");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target");else fprintf (stderr," to select");
    }
  }
  
  if (k==DOWN) {
    if (delx||dely) {
      Vector v = GFXDeviceToEye(x-CurDFA->prevx, y-CurDFA->prevy) - GFXDeviceToEye(0,0);
      CurDFA->SelectBox.SetSize (v.i, v.j);
      CurDFA->Selecting=true;
    }
  }
  if (k==RELEASE&&CurDFA->Selecting) {
    CurDFA->Selecting=false;
    UnitCollection *tmpcol = CurDFA->clickList->requestIterator(CurDFA->prevx,CurDFA->prevy,x,y);
    CurDFA->replaceCollection(tmpcol);
    UnitCollection::UnitIterator * tmp2 = tmpcol->createIterator();
    if (tmp2->current()) {
      fprintf (stderr,"None::replacing SelectBox Units");if (CurDFA->state==TARGET_SELECT) fprintf (stderr," to target");else fprintf (stderr," to select");
      while(tmp2->current()) {
	cerr << *tmp2->current() << endl;
	tmp2->advance();
      }
      CurDFA->SetStateSomeSelected();
    }else {
      fprintf (stderr,"None::select box missed");
    }
    delete tmp2;
  }
}

InputDFA::InputDFA (StarSystem * par) :MouseArrow ("mouse.spr", true), SelectBox("selectbox.spr", true){
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
      UnitCollection::UnitIterator *it = selected->createIterator();
      while(it->current()) {
	it->current()->Deselect();
	it->advance();
      }
      delete it;
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
    UnitCollection::UnitIterator *it;
    for(it = selected->createIterator();
	it->current();
	it->advance()) {
      it->current()->Select();
    }
    delete it;
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
      UnitCollection::UnitIterator *it;
      for(it = newcol->createIterator();
	  it->current();
	  it->advance()) {
	it->current()->Select();
      }
      delete it;
    
      UnitCollection::UnitIterator *tmpit =newcol->createIterator();
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
      UnitCollection::UnitIterator * tmpit = newcol->createIterator();
      targetted->append (tmpit);
      delete tmpit;
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
}

/** ContextSwitch unbinds the respective keys and appropriately to the given state*/
void InputDFA::ContextSwitch (){
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
}
