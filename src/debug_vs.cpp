#include <stdio.h>
#include "hashtable.h"
Hashtable <long,char,char[65535]> constructed;

Hashtable <long,char,char[65535]> destructed;
bool DEBUG_ERROR_IN_MY_CODE=true;
void VS_DEBUG_ERROR () {
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
