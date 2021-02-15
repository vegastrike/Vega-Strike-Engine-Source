#ifndef BOLTDRAWMANAGER_H
#define BOLTDRAWMANAGER_H

#include "gfx/decalqueue.h"
#include "bolt.h"

#include <vector>

class Animation;

class BoltDrawManager
{
public:
    class DecalQueue boltdecals;
    static GFXVertexList * boltmesh;
    vector <std::string> animationname;
    vector <Animation *> animations;
    vector <vector <Bolt> > bolts;
    vector <vector <Bolt> > balls;
    vector <int> cachedecals;

    BoltDrawManager();
    ~BoltDrawManager();
};

#endif // BOLTDRAWMANAGER_H
