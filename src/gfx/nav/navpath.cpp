/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vegastrike.h"
#if defined( _WIN32) && !defined(__CYGWIN__) && !defined( __MINGW32__)
// For WIN32 debugging.
#include <crtdbg.h>
#endif

#include "navpath.h"
#include "gfx/cockpit.h"
#include "navscreen.h"

#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <string>
#include <iostream>

using namespace std;
//*******************************************************************//
//                                                                   //
//                            NavPath Class                          //
//                                                                   //
//*******************************************************************//

bool NavPath::isAbsolute() const {
  if(!isComplete())
    return false;

  if(!source->isAbsolute() || !destination->isAbsolute())
    return false;

  return true;
}

bool NavPath::isComplete() const {
  if(!source || !destination)
    return false;

  if(source->getRequiredPath())
    if(!source->getRequiredPath()->isEvaluated())
      return false;

  if(destination->getRequiredPath())
    if(!destination->getRequiredPath()->isEvaluated())
      return false;

  return true;
}

bool NavPath::isCurrentDependant() const {
  if(source)
    if(source->isCurrentDependant())
      return true;

  if(destination)
    if(destination->isCurrentDependant())
      return true;
  
  return false;
}

bool NavPath::isTargetDependant() const {
  if(source)
    if(source->isTargetDependant())
      return true;

  if(destination)
    if(destination->isTargetDependant())
      return true;
  
  return false;
}

string NavPath::getDescription() const {
  string temp;
  
  temp = getName();
  temp += "#n#";
  temp += "----------#n#";
  temp += "Visible: ";
  temp += (getVisible()?"True":"False");
  temp += "#n#";

  if(source) {
    temp += "Source:#n#";
    temp += source->getDescription();
    temp += "#n#";
  }

  if(destination) {
    temp += "Destination:#n#";
    temp += destination->getDescription();
    temp += "#n#";
  }

  if(!source || !destination)
    temp += "#n#INCOMPLETE#n#";
  else if(!isComplete())
    temp += "#n#PATH CHAIN IS UNSOLVED BEFORE THIS POINT#n#";
  else if(!isEvaluated())
    temp += "#n#PATH NOT FOUND#n#";	  
  
  return temp;
}

void NavPath::setVisible(bool vis) {
  visible = vis;
}

void NavPath::setColor(GFXColor col) {
  color = col;
}

void NavPath::setName(string n) {
  name = n;
}

bool NavPath::getVisible() const {
  return visible;
}

GFXColor NavPath::getColor() const {
  return color;
}

string NavPath::getName() const {
  return name;
}

bool NavPath::setSourceNode(PathNode *node) {
  if(!node)
    return false;

  if(!node->isSourceable())
    return false;

  PathNode * oldNode = source;
  source = node;
  if(checkForCycles()) {
    source = oldNode;
    delete node;
    node = NULL;
    return false;
  }
  else {
    if(oldNode) {
      if(oldNode->getRequiredPath())
	oldNode->getRequiredPath()->removeDependant(this);
    }
    if(node->getRequiredPath())
      node->getRequiredPath()->addDependant(this);
    if(oldNode)
      delete oldNode;
    oldNode=NULL;
    _Universe->AccessCockpit()->AccessNavSystem()->pathman->updateSpecificPath(this);
    return true;
  }
}

bool NavPath::setDestinationNode(PathNode *node) {
  if(!node)
    return false;

  PathNode * oldNode = destination;
  destination = node;
  if(checkForCycles()) {
    destination = oldNode;
    delete node;
    node = NULL;
    return false;
  }
  else {
    if(oldNode) {
      if(oldNode->getRequiredPath())
	oldNode->getRequiredPath()->removeDependant(this);
    }
    if(node->getRequiredPath())
      node->getRequiredPath()->addDependant(this);
    if(oldNode)
      delete oldNode;
    oldNode = NULL;
    _Universe->AccessCockpit()->AccessNavSystem()->pathman->updateSpecificPath(this);
    return true;
  }
}

unsigned NavPath::getAbsoluteSource() const {
  return path.front();
}

unsigned NavPath::getAbsoluteDestination() const {
  return path.back();
}

std::vector<unsigned> NavPath::getAllPoints() const {
  std::vector<unsigned> temp;
  for(list<unsigned>::const_iterator i=path.begin(); i != path.end(); ++i)
    temp.push_back(*i);
  return temp;
}

void NavPath::addDependant(NavPath * dependant) {
  if(dependant==NULL)
    return;
  
  dependants.insert(dependant);
}

void NavPath::removeDependant(NavPath * dependant) {
  if(dependant==NULL)
    return;
  
  dependants.erase(dependant);
}  

std::vector<NavPath *> NavPath::getDependants() const {
  return std::vector<NavPath *>(dependants.begin(), dependants.end());
}

std::vector<NavPath *> NavPath::getRequiredPaths() const {
  std::vector<NavPath *> temp;

  if(source)
    if(source->getRequiredPath())
      temp.push_back(source->getRequiredPath());

  if(destination)
    if(destination->getRequiredPath())
      temp.push_back(destination->getRequiredPath());
  
  return temp;
}

bool NavPath::checkForCycles() const {
  const NavPath *v;
  vector<NavPath *> neighbors;
  unsigned i;
  deque<const NavPath *> pathStack;
  pathStack.push_back(this);
  bool cycle = false;
  
  while(!pathStack.empty() && !cycle) {
    v = pathStack.back();
    pathStack.pop_back();
    neighbors = v->getRequiredPaths();

    for(i=0; i<neighbors.size(); ++i) {
      if(neighbors[i]==this) {
	cycle = true;
	break;
      }
      pathStack.push_back(neighbors[i]);
    }
  }

  return cycle;
}

bool NavPath::evaluate() {
  NavigationSystem::CachedSystemIterator & systemIter=_Universe->AccessCockpit()->AccessNavSystem()->systemIter;
  path.clear();

  if(!isComplete())
    return false;
    
  if(isAbsolute()) {               //Using a double-rooted BFS search
    unsigned originIndex=source->initSearchQueue().front();
    unsigned destIndex=destination->initSearchQueue().front();
    
    if(originIndex==destIndex) {
      path.push_back(originIndex);
      return true;
    }

    vector<unsigned> prev(systemIter.size());
    vector<unsigned> visited(systemIter.size(), 0);
    deque<unsigned> oriFront, destFront;
    bool found=false;
    unsigned midNode;
    unsigned midNodePrevOri;
    unsigned midNodePrevDest;
    
    bool oriTurn=true;
    deque<unsigned> * front;
    unsigned visitMark;
    if (originIndex>=visited.size()||destIndex>=visited.size()) {
      fprintf (stderr,"(previously) FATAL error with nav system, referencing value too big %d %d with visited size %d\n",(int)originIndex,(int)destIndex,(int)visited.size());
      return false;
    }
    oriFront.push_back(originIndex);
    visited[originIndex]=1;
    destFront.push_back(destIndex);
    visited[destIndex]=2;
    
    while(!oriFront.empty() && !destFront.empty() && !found) {
      if(oriTurn) {
	front=&oriFront;
	visitMark=1;
      }
      else {
	front=&destFront;
	visitMark=2;
      }
      
      unsigned index=front->front();
      front->pop_front();
      
      for(unsigned adjs=0; adjs<systemIter[index].GetDestinationSize(); ++adjs) {
	unsigned adjIndex=systemIter[index].GetDestinationIndex(adjs);
	if(systemIter[adjIndex].isDrawable()) {
	  if(visited[adjIndex]==0) {
	    visited[adjIndex]=visitMark;
	    prev[adjIndex]=index;
	    front->push_back(adjIndex);
	  }
	  else if(visited[adjIndex]!=visitMark) {
	    midNode=adjIndex;
	    if(oriTurn) {
	      midNodePrevDest=prev[adjIndex];
	      midNodePrevOri=index;
	    }
	    else {
	      midNodePrevOri=prev[adjIndex];
	      midNodePrevDest=index;
	    }				
	    found = true;
	    break;
	  }
	}
      }
      oriTurn=!oriTurn;
    }
    
    if(found) {
      unsigned index=midNodePrevOri;
      while(index != originIndex) {
	path.push_front(index);
	index=prev[index];
      }
      path.push_front(originIndex);
      
      if(destIndex==midNode) {
	path.push_back(destIndex);
      }
      else {
	path.push_back(midNode);
	
	unsigned index=midNodePrevDest;
	while(index != destIndex) {
	  path.push_back(index);
	  index=prev[index];
	}
	path.push_back(destIndex);
      }
    }
    return found;
  }
  else {                    // Using single-rooted BFS search
    vector<unsigned> prev(systemIter.size());
    vector<bool> visited(systemIter.size(),false);
    deque<unsigned> frontier = source->initSearchQueue();
    set<unsigned> origins;
    bool found=false;
    unsigned index, destIndex;
    
    for(unsigned i=0; i<frontier.size(); ++i) {
      index=frontier.front();
      frontier.pop_front();
      visited[index]=true;
      frontier.push_back(index);
      origins.insert(index);
      
      if(destination->isDestination(index)) {
	path.push_back(index);
	return true;
      }
    }
    
    while(!frontier.empty() && !found) {
      index=frontier.front();
      frontier.pop_front();
      
      for(unsigned adjs=0; adjs<systemIter[index].GetDestinationSize(); ++adjs) {
	unsigned adjIndex=systemIter[index].GetDestinationIndex(adjs);
	if(!visited[adjIndex] && systemIter[adjIndex].isDrawable()) {
	  visited[adjIndex]=true;
	  prev[adjIndex]=index;
	  frontier.push_back(adjIndex);
	  
	  if(destination->isDestination(adjIndex)) {
	    found = true;
	    destIndex = adjIndex;
	    break;
	  }
	  
	}
      }
    }
    
    if(found) {
      index=destIndex;
      path.push_front(index);
      do{
	  index=prev[index];
	  path.push_front(index);
      } while(!origins.count(index));    //While the index is not an origin
    }
    return found;
  }
}

void NavPath::removeOldPath() {
  NavigationSystem::CachedSystemIterator & systemIter=_Universe->AccessCockpit()->AccessNavSystem()->systemIter;

  //Erase old path
  //*************************
  for(list<unsigned>::iterator i=path.begin(); i!=path.end(); ++i) {
    if(systemIter[*i].paths.erase(this)) {   //This erases this path from the list of paths in system
      if(systemIter[*i].paths.empty())
	systemIter[*i].part_of_path=false;
    }
  }
  pathNeighbors.clear();
}
 
void NavPath::addNewPath() {
  NavigationSystem::CachedSystemIterator & systemIter=_Universe->AccessCockpit()->AccessNavSystem()->systemIter;

  int i;
  unsigned system;
   //Inscribe new path
   //*************************
  
  list<unsigned>::iterator prev, next;
  for(list<unsigned>::iterator i=path.begin(); i!=path.end(); ++i) {
    prev = next = i;
    --prev;
    ++next;
    if((*i)!=path.front())
      pathNeighbors[(*i)].first=(*prev);
    if((*i)!=path.back())
      pathNeighbors[(*i)].second=(*next);
    systemIter[*i].part_of_path=true;
    systemIter[*i].paths.insert(this);
  }
}

bool NavPath::isNeighborPath(unsigned system, unsigned neighbor) {
  map<unsigned,pair<unsigned,unsigned> >::iterator i = pathNeighbors.find(system);
  
  if(i==pathNeighbors.end())
    return false;
    
  if(system!=path.front())
    if((*i).second.first==neighbor)
      return true;

  if(system!=path.back())
    if((*i).second.second==neighbor)
      return true;

  return false;
}

bool NavPath::update() {
  removeOldPath();
  if(evaluate()) {
    addNewPath();
    return true;
  }
  else
    return false;
}

NavPath::NavPath() {
  name = "New Path";
  visible = true;
  color = GFXColor(1, 0, 0);
  source = NULL;
  destination = NULL;
}

NavPath::~NavPath() {
  removeOldPath();
  
  if(source) {
    if(source->getRequiredPath())
      source->getRequiredPath()->removeDependant(this);
    delete source;
    source = NULL;
  }
  
  if(destination) {
    if(destination->getRequiredPath())
      destination->getRequiredPath()->removeDependant(this);
    delete source;
    source = NULL;
  }

  vector<NavPath *> depList = getDependants();
  for(std::vector<NavPath *>::iterator i = depList.begin(); i < depList.end(); ++i) {
    if((*i)->source->getRequiredPath()==this) {
      delete (*i)->source;
      (*i)->source = NULL;
    }
    
    if((*i)->destination->getRequiredPath()==this) {
      delete (*i)->destination;
      (*i)->destination = NULL;
    }
  } 
}

//*******************************************************************//
//                                                                   //
//                        PathManager Class                          //
//                                                                   //
//*******************************************************************//

void PathManager::addPath() {
  NavPath *path = new NavPath();
  path->setSourceNode(new CurrentPathNode());
  paths.push_back(path);
}

bool PathManager::removePath(NavPath * path) {
  bool ret=false;
  for(std::vector<NavPath *>::iterator i = paths.begin(); i < paths.end(); ++i)
    if((*i)==path) {
      delete (*i);
	  ret=true;
      paths.erase(i);
    }
  return ret;
}

void PathManager::showAll() {
  for(std::vector<NavPath *>::iterator i = paths.begin(); i < paths.end(); ++i)
    (*i)->setVisible(true);
}

void PathManager::showNone() {
  for(std::vector<NavPath *>::iterator i = paths.begin(); i < paths.end(); ++i)
    (*i)->setVisible(false);
}

bool PathManager::updateSpecificPath(NavPath * path) {
  path->updated = true;
  path->update();
  updateDependants(path);
  return path->updated;
}

void PathManager::updatePaths(UpdateType type) {
  std::list<NavPath *>::iterator i;
  DFS();
  if(type==ALL) {
    for(std::vector<NavPath *>::iterator j = paths.begin(); j < paths.end(); ++j) {
      std::cerr<<"Updating path: "<<(*j)->getName()<<endl;
      (*j)->update();
    }
  }
  else if(type==CURRENT) {
    for(i = topoOrder.begin(); i != topoOrder.end(); ++i)
      (*i)->updated=false;
    
    for(i = topoOrder.begin(); i != topoOrder.end(); ++i)
      if((*i)->updated==false && (*i)->isCurrentDependant()) {
	std::cerr<<"Updating path: "<<(*i)->getName()<<endl;
	updateSpecificPath(*i);
      }
  }
  else{
    for(i = topoOrder.begin(); i != topoOrder.end(); ++i)
      (*i)->updated=false;
    
    for(i = topoOrder.begin(); i != topoOrder.end(); ++i)
      if((*i)->updated==false && (*i)->isTargetDependant()) {
	std::cerr<<"Updating path: "<<(*i)->getName()<<endl;
	updateSpecificPath(*i);
      }
  }
}

void PathManager::updateDependants(NavPath * parent) {
  vector<NavPath *> dependants = parent->getDependants();
  for(std::vector<NavPath *>::iterator i = dependants.begin(); i<dependants.end(); ++i)
    updateSpecificPath(*i);
} 

void PathManager::DFS() {
  topoOrder.clear();
  std::vector<NavPath *>::iterator u;
  for(u = paths.begin(); u<paths.end(); ++u)
    (*u)->topoColor = TOPO_WHITE;
  
  topoTime=0;

  for(u = paths.begin(); u<paths.end(); ++u)
    if((*u)->topoColor==TOPO_WHITE)
      dfsVisit(*u);
}

void PathManager::dfsVisit(NavPath * path) {
  path->topoColor=TOPO_GRAY;

  std::vector<NavPath *> dependants = path->getDependants();
  for(std::vector<NavPath *>::iterator v = dependants.begin(); v<dependants.end(); ++v)
    if((*v)->topoColor==TOPO_WHITE)
      dfsVisit(*v);

  path->topoColor=TOPO_BLACK;
  ++topoTime;
  path->topoTime=topoTime;
  topoOrder.push_front(path);
}

PathManager::PathManager() {
}

PathManager::~PathManager() {
  for(std::vector<NavPath *>::iterator i = paths.begin(); i < paths.end(); ++i) {
    delete (*i);
    (*i) = NULL;
  }
}


//*******************************************************************//
//                                                                   //
//                        AbsolutePathNode Class                     //
//                                                                   //
//*******************************************************************//

std::string AbsolutePathNode::getDescription() const {
  return _Universe->AccessCockpit()->AccessNavSystem()->systemIter[system].GetName();
}

std::deque<unsigned> AbsolutePathNode::initSearchQueue() const {
  deque<unsigned> temp;
  temp.push_back(system);
  return temp;
}


//*******************************************************************//
//                                                                   //
//                        CurrentPathNode Class                      //
//                                                                   //
//*******************************************************************//

bool CurrentPathNode::isDestination(unsigned index) const {
  return _Universe->AccessCockpit()->AccessNavSystem()->currentsystemindex==index;
}

std::deque<unsigned> CurrentPathNode::initSearchQueue() const {
  deque<unsigned> temp;
  temp.push_back(_Universe->AccessCockpit()->AccessNavSystem()->currentsystemindex);
  return temp;
}


//*******************************************************************//
//                                                                   //
//                        TargetPathNode Class                       //
//                                                                   //
//*******************************************************************//

bool TargetPathNode::isDestination(unsigned index) const {
  return _Universe->AccessCockpit()->AccessNavSystem()->destinationsystemindex==index;
}

std::deque<unsigned> TargetPathNode::initSearchQueue() const {
  deque<unsigned> temp;
  temp.push_back(_Universe->AccessCockpit()->AccessNavSystem()->destinationsystemindex);
  return temp;
}


//*******************************************************************//
//                                                                   //
//                       CriteriaPathNode Class                       //
//                                                                   //
//*******************************************************************//

std::string CriteriaPathNode::getDescription() const {
  assert(criteria != NULL);

  string temp = "Criteria: ";
  temp += criteria->getDescription();
  return temp;
}
  
bool CriteriaPathNode::isDestination(unsigned index) const {
  assert(criteria != NULL);
  return criteria->isDestination(index);
}

PathNode* CriteriaPathNode::clone() const {
  assert(criteria != NULL);

  CriteriaPathNode * newNode = new CriteriaPathNode();
  newNode->criteria = static_cast<CriteriaRoot*>(criteria->clone());
  return newNode;
}

CriteriaPathNode::CriteriaPathNode() {
  criteria = new CriteriaRoot();
}

CriteriaPathNode::~CriteriaPathNode() {
  assert(criteria != NULL);

  delete criteria;
}


//*******************************************************************//
//                                                                   //
//                         ChainPathNode Class                       //
//                                                                   //
//*******************************************************************//

std::string ChainPathNode::getDescription() const {
  string temp = "Chain: ";
  temp += supplierPath->getName();
  if(type==SOURCE)
    temp += " (Source)";
  else if(type==DESTINATION)
    temp += " (Destination)";
  else
    temp += " (All Points)";

  return temp;
}

std::deque<unsigned> ChainPathNode::initSearchQueue() const {
  deque<unsigned> systemDeque;
  if(type==SOURCE)
    systemDeque.push_back(supplierPath->getAbsoluteSource());
  else if(type==DESTINATION)
    systemDeque.push_back(supplierPath->getAbsoluteDestination());
  else {
    vector<unsigned> systems=supplierPath->getAllPoints();
    for(unsigned i=0; i<systems.size(); ++i)
      systemDeque.push_back(systems[i]);
  }

  return systemDeque;
}

bool ChainPathNode::isDestination(unsigned index) const {
  if(type==SOURCE)
    return supplierPath->getAbsoluteSource()==index;
  else if(type==DESTINATION)
    return supplierPath->getAbsoluteDestination()==index;
  else {
    vector<unsigned> systems=supplierPath->getAllPoints();
    for(unsigned i=0; i<systems.size(); ++i)
      if(systems[i]==index)
	return true;
    return false;
  }
}
