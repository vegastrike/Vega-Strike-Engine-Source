#ifndef VS_DEBUG

#else
#include <stdio.h>

#include "hashtable.h"
#if defined (_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif
Hashtable <long,char,char[65535]> constructed;

Hashtable <long,char,char[65535]> destructed;
bool DEBUG_ERROR_IN_MY_CODE=true;
void VS_DEBUG_ERROR () {
  fprintf(stderr,"WARNING: invalid refcount in vegastrike object\n");
#if defined (_MSC_VER) && defined(_DEBUG)
  if (DEBUG_ERROR_IN_MY_CODE) {
    _RPT0(_CRT_ERROR, "WARNING: invalid refcount in vegastrike object\n");
  }
  return;
#endif
  while (DEBUG_ERROR_IN_MY_CODE) {
    printf ("ack");
  }
}
char *Constructed (void * v) {
  return constructed.Get ((long)v);
}
char *Destructed (void * v) {
  return destructed.Get ((long)v);
}
#endif
