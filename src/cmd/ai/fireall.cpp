#include "cmd/unit_generic.h"
#include "fireall.h"
#include "cmd/role_bitmask.h"
namespace Orders {
  FireAllYouGot::FireAllYouGot ():Order (WEAPON,STARGET) {}
  void FireAllYouGot::Execute() {
    parent->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS|ROLES::FIRE_MISSILES,true);
  }
}
