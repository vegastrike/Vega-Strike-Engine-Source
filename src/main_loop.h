#ifndef _MAIN_LOOP_H_
#define _MAIN_LOOP_H_

namespace CockpitKeys {
  void SkipMusicTrack(int,KBSTATE newState);
   void  PitchDown(int,KBSTATE newState) ;
   void  PitchUp(int,KBSTATE newState) ;
   void  YawLeft(int,KBSTATE newState) ;
   void  YawRight(int,KBSTATE newState) ;
   void  Inside(int,KBSTATE newState) ;
   void  ZoomOut (int, KBSTATE newState) ;
   void  ZoomIn (int, KBSTATE newState) ;
   void  InsideLeft(int,KBSTATE newState) ;
   void InsideRight(int,KBSTATE newState) ;
   void  InsideBack(int,KBSTATE newState) ;
   void  SwitchLVDU(int,KBSTATE newState) ;
   void  SwitchRVDU(int,KBSTATE newState) ;
   void  Behind(int,KBSTATE newState) ;
   void  Pan(int,KBSTATE newState) ;

   void  Quit(int,KBSTATE newState) ;
}





void createObjects();
void destroyObjects();
void InitializeInput();
void main_loop();











#endif
