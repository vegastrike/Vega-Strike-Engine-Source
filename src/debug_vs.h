#ifndef _DEBUG_VS_H_
#define _DEBUG_VS_H_

//#define VS_DEBUG
//#define VS_DEBUG1
//#define VS_DEBUG2
//#define VS_DEBUG3

#ifndef VS_DEBUG
inline void VS_DEBUG_ERROR () {

}
#define VSCONSTRUCT1(a)
#define VSDESTRUCT1
#define VSCONSTRUCT2(a)
#define VSDESTRUCT2
#define VSCONSTRUCT3(a)
#define VSDESTRUCT3

#else
void VS_DEBUG_ERROR();

#error
#include "hashtable.h"
extern Hashtable <long,char,char[65535]> constructed;
extern Hashtable <long,char,char[65535]> destructed;
#define VSCONST(a) if (constructed.Get((long)this)!=NULL) VS_DEBUG_ERROR (); if (destructed.Get((long)this)!=NULL) destructed.Delete((long)this); if (destructed.Get((long)this)!=NULL) VS_DEBUG_ERROR(); constructed.Put((long)this,(char *)a);

#define VSDEST if (constructed.Get((long)this)==NULL) VS_DEBUG_ERROR (); else constructed.Delete((long)this); if (constructed.Get((long)this)!=NULL) VS_DEBUG_ERROR(); if (destructed.Get((long)this)!=NULL) VS_DEBUG_ERROR();   destructed.Put((long)this,(char *)this);
#define VSCONSTRUCT1(a) VSCONST(a)
#define VSDESTRUCT1 VSDEST

  #ifdef VS_DEBUG2
  #define VSCONSTRUCT2(a) VSCONST(a)
  #define VSDESTRUCT2 VSDEST
   #ifdef VS_DEBUG3
   #define VSCONSTRUCT3(a) VSCONST(a)
   #define VSDESTRUCT3 VSDEST

   #else
   #define VSCONSTRUCT2(a)
   #define VSDESTRUCT2
   #define VSCONSTRUCT3(a)
   #define VSDESTRUCT3
   #endif
  #else
#define VSCONSTRUCT1(a)
#define VSDESCRUCT1
#define VSCONSTRUCT2(a)
#define VSDESCRUCT2
#define VSCONSTRUCT3(a)
#define VSDESCRUCT3


#endif
#endif
#endif
