#include "tactics.h"
#include "vegastrike.h"
#include "cmd/unit_generic.h"

void CloakFor::Execute()
{
    if (time == 0)
        parent->Cloak( enable );
    time += SIMULATION_ATOM;
    if (time > maxtime) {
        done = true;
        if (maxtime != 0)
            parent->Cloak( !enable );
        return;
    }
}
CloakFor::~CloakFor()
{
#ifdef ORDERDEBUG
    VSFileSystem::vs_fprintf( stderr, "clk%x\n", this );
    fflush( stderr );
#endif
    if (parent && time <= maxtime)
        parent->Cloak( !enable );
}

