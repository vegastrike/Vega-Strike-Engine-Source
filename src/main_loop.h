#ifndef _MAIN_LOOP_H_
#define _MAIN_LOOP_H_
#include "in.h"
#include "cmd/unit_generic.h"
void bootstrap_draw (const std::string &message, Animation * SplashScreen=NULL);

namespace CockpitKeys {
  void SkipMusicTrack(const std::string&,KBSTATE newState);
   void  PitchDown(const std::string&,KBSTATE newState) ;
   void  PitchUp(const std::string&,KBSTATE newState) ;
   void  YawLeft(const std::string&,KBSTATE newState) ;
   void  YawRight(const std::string&,KBSTATE newState) ;
   void  Inside(const std::string&,KBSTATE newState) ;
   void  ZoomOut (const std::string&,KBSTATE newState) ;
   void  ZoomIn (const std::string&,KBSTATE newState) ;
   void  InsideLeft(const std::string&,KBSTATE newState) ;
   void InsideRight(const std::string&,KBSTATE newState) ;
   void  InsideBack(const std::string&,KBSTATE newState) ;
   void  SwitchLVDU(const std::string&,KBSTATE newState) ;
   void  SwitchRVDU(const std::string&,KBSTATE newState) ;
   void  SwitchMVDU(const std::string&,KBSTATE newState) ;
   void  SwitchULVDU(const std::string&,KBSTATE newState) ;
   void  SwitchURVDU(const std::string&,KBSTATE newState) ;
   void  SwitchUMVDU(const std::string&,KBSTATE newState) ;
   void  CommModeVDU(const std::string&,KBSTATE);
   void  ObjectiveModeVDU(const std::string&,KBSTATE);
   void  TargetModeVDU(const std::string&,KBSTATE);
   void  ManifestModeVDU(const std::string&,KBSTATE);
   void  DamageModeVDU(const std::string&,KBSTATE);
   void  GunModeVDU(const std::string&,KBSTATE);
   void  MissileModeVDU(const std::string&,KBSTATE);
   void  ScanningModeVDU(const std::string&,KBSTATE);
   void  ReverseGunModeVDU(const std::string&,KBSTATE);
   void  ReverseMissileModeVDU(const std::string&,KBSTATE);
   void  ViewModeVDU(const std::string&,KBSTATE);
   void  ScrollUp(const std::string&,KBSTATE newState) ;
   void  ScrollDown(const std::string&,KBSTATE newState) ;
   void  Behind(const std::string&,KBSTATE newState) ;
   void  Pan(const std::string&,KBSTATE newState) ;
   void  PanTarget(const std::string&,KBSTATE newState) ;
   void  ViewTarget(const std::string&,KBSTATE newState) ;
   void  OutsideTarget(const std::string&,KBSTATE newState) ;
   void  Quit(const std::string&,KBSTATE newState) ;

   void  QuitNow();
}



struct SavedUnits;
void AddUnitToSystem (const SavedUnits * su);
void createObjects(std::vector <std::string> &playersaveunit, std::vector <StarSystem *> &ssys, std::vector <QVector>& savedloc, vector<vector<string> > &savefiles);
void destroyObjects();
void InitializeInput();
void main_loop();
void restore_main_loop();






#endif
