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
    vector <Animation *> animations; // Balls are animated
    vector <vector <Bolt> > bolts; // The inner vector is all of the same type.
    vector <vector <Bolt> > balls;

    BoltDrawManager();
    ~BoltDrawManager();

    static BoltDrawManager& getInstance() {
        static BoltDrawManager instance;    // Guaranteed to be destroyed.
        return instance;                    // Instantiated on first use.
    }
};

#endif // BOLTDRAWMANAGER_H
