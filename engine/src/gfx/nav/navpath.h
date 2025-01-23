/*
 * navpath.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAV_PATH_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAV_PATH_H

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <string>
#include "criteria.h"

#include "gfxlib.h"

enum TopoColor { TOPO_WHITE, TOPO_GRAY, TOPO_BLACK };

//*******************************************************************//
////
//NavPath Class                          //
////
//*******************************************************************//

class NavPath;
class PathManager;
class PathNode;

class NavPath {
public:
    bool isAbsolute() const;

    bool isEvaluated() const {
        return path.size() != 0;
    }

    bool isComplete() const;
    bool isCurrentDependant() const;
    bool isTargetDependant() const;

    std::string getDescription() const;

    void setVisible(bool vis);
    void setColor(GFXColor col);
    void setName(std::string n);
    bool getVisible() const;
    GFXColor getColor() const;
    std::string getName() const;

    bool setSourceNode(PathNode *node);
    bool setDestinationNode(PathNode *node);

    PathNode *getSourceNode() {
        return source;
    }

    PathNode *getDestinationNode() {
        return destination;
    }

    const PathNode *getSourceNode() const {
        return source;
    }

    const PathNode *getDestinationNode() const {
        return destination;
    }

    unsigned getAbsoluteSource() const;
    unsigned getAbsoluteDestination() const;
    const std::list<unsigned> *getAllPoints() const;
    std::list<unsigned> *getAllPoints();

    void addDependant(NavPath *dependant);
    void removeDependant(NavPath *dependant);
    const std::set<NavPath *> *getDependants() const;
    std::set<NavPath *> *getDependants();

    std::vector<NavPath *> getRequiredPaths() const;
    bool checkForCycles() const;
    bool evaluate();
    void removeOldPath();
    void addNewPath();
    bool update();

    bool isNeighborPath(unsigned system, unsigned neighbor);

    NavPath();
    ~NavPath();

protected:
    friend class PathManager;

    bool visible;
    std::string name;
    GFXColor color;
    PathNode *source;
    PathNode *destination;
    std::list<unsigned> path;
    std::map<unsigned, std::pair<unsigned, unsigned> > pathNeighbors;
    std::set<NavPath *> dependants;

    TopoColor topoColor;
    unsigned topoTime;
    bool updated;
};

//*******************************************************************//
////
//PathManager Class                         //
////
//*******************************************************************//

class PathManager {
public:
    void addPath();
    bool removePath(NavPath *path);
    void showAll();
    void showNone();

    enum UpdateType { ALL, CURRENT, TARGET };
    bool updateSpecificPath(NavPath *path);
    void updatePaths(UpdateType type = ALL);
    void updateDependants(NavPath *parent);

    PathManager();
    ~PathManager();

protected:
    friend class NavPath;
    friend class NavComputer;

    std::vector<NavPath *> paths;
    std::list<NavPath *> topoOrder;

    void DFS();
    void dfsVisit(NavPath *path);
    unsigned topoTime;
};

//*******************************************************************//
////
//PathNode Class                         //
////
//*******************************************************************//

class PathNode {
public:
    virtual bool isAbsolute() const = 0;
//Desc: True IFF this node can dereference into one absolute system.
    virtual bool isSourceable() const = 0;
//Desc True IFF this node can be used as a source in a path finding algorithm

    virtual bool isCurrentDependant() const {
        return false;
    }

    virtual bool isTargetDependant() const {
        return false;
    }

    virtual NavPath *getRequiredPath() {
        return NULL;
    }

//Desc: Returns the list of paths this node is dependant on.
    virtual std::string getDescription() const = 0;
//Desc: Returns a textual description of the node

    virtual std::deque<unsigned> initSearchQueue() const = 0;
//Desc: Returns a deque that may be used to start a search
    virtual bool isDestination(unsigned index) const = 0;
//Desc: True IFF the system index is equivalent to the system
//specified in the node

    virtual PathNode *clone() const = 0;

    PathNode() {
    }

    virtual ~PathNode() {
    }
};

//*******************************************************************//
////
//AbsolutePathNode Class                        //
////
//*******************************************************************//

class AbsolutePathNode : public PathNode {
public:
    bool isAbsolute() const {
        return true;
    }

    bool isSourceable() const {
        return true;
    }

    std::string getDescription() const;

    std::deque<unsigned> initSearchQueue() const;

    bool isDestination(unsigned index) const {
        return index == system;
    }

    unsigned getSystemIndex() {
        return system;
    }
//Desc: Gets the system referenced by this node

    PathNode *clone() const {
        return new AbsolutePathNode(system);
    }

    AbsolutePathNode(unsigned index) {
        system = index;
    }

    ~AbsolutePathNode() {
    }

protected:
    unsigned system;
};

//*******************************************************************//
////
//CurrentPathNode Class                         //
////
//*******************************************************************//

class CurrentPathNode : public PathNode {
public:
    bool isAbsolute() const {
        return true;
    }

    bool isSourceable() const {
        return true;
    }

    std::string getDescription() const {
        return "Current System";
    }

    bool isCurrentDependant() const {
        return true;
    }

    std::deque<unsigned> initSearchQueue() const;
    bool isDestination(unsigned index) const;

    PathNode *clone() const {
        return new CurrentPathNode();
    }

    CurrentPathNode() {
    }

    ~CurrentPathNode() {
    }
};

//*******************************************************************//
////
//TargetPathNode Class                        //
////
//*******************************************************************//

class TargetPathNode : public PathNode {
public:
    bool isAbsolute() const {
        return true;
    }

    bool isSourceable() const {
        return true;
    }

    std::string getDescription() const {
        return "Target System";
    }

    bool isTargetDependant() const {
        return true;
    }

    std::deque<unsigned> initSearchQueue() const;
    bool isDestination(unsigned index) const;

    PathNode *clone() const {
        return new TargetPathNode();
    }

    TargetPathNode() {
    }

    ~TargetPathNode() {
    }
};

//*******************************************************************//
////
//CriteriaPathNode Class                        //
////
//*******************************************************************//

class CriteriaPathNode : public PathNode {
public:
    bool isAbsolute() const {
        return false;
    }

    bool isSourceable() const {
        return false;
    }

    std::string getDescription() const;

    std::deque<unsigned> initSearchQueue() const {
        std::deque<unsigned> temp;
        return temp;
    }

    bool isDestination(unsigned index) const;

    CriteriaRoot *getRoot() {
        return criteria;
    }

    PathNode *clone() const;
    CriteriaPathNode();
    ~CriteriaPathNode();

private:
    CriteriaRoot *criteria;
};

//*******************************************************************//
////
//ChainPathNode Class                       //
////
//*******************************************************************//

class ChainPathNode : public PathNode {
public:
    bool isAbsolute() const {
        return type == ALL_POINTS ? false : true;
    }

    bool isSourceable() const {
        return true;
    }

    std::string getDescription() const;

    NavPath *getRequiredPath() {
        return supplierPath;
    }

    std::deque<unsigned> initSearchQueue() const;
    bool isDestination(unsigned index) const;

    void setSupplierPath(NavPath *supplier) {
        supplierPath = supplier;
    }

    NavPath *getSupplierPath() {
        return supplierPath;
    }

    enum PartType { SOURCE, DESTINATION, ALL_POINTS };

    void setPartType(PartType part) {
        type = part;
    }

    PartType getPartType() {
        return type;
    }

    PathNode *clone() const {
        return new ChainPathNode(supplierPath, type);
    }

    ChainPathNode() {
        supplierPath = NULL;
    }

    ChainPathNode(NavPath *supplier, PartType part) {
        supplierPath = supplier;
        type = part;
    }

    ~ChainPathNode() {
    }

private:
    NavPath *supplierPath;
    PartType type;
};

#endif   //VEGA_STRIKE_ENGINE_GFX_NAV_NAV_PATH_H
