#include "collection.h"

#if defined(USE_OLD_COLLECTION)
#include "oldcollection.cpp"
#elif defined(USE_STL_COLLECTION)

#include <list>
#include <vector>
#ifndef LIST_TESTING
#include "unit_generic.h"
#else
#include "testcollection/unit.h"
#endif

using std::list;
using std::vector;
// UnitIterator  BEGIN:

UnitCollection::UnitIterator& UnitCollection::UnitIterator::operator=(const UnitCollection::UnitIterator& orig)
{
	if(col != orig.col){
		if(col)
			col->unreg(this);
		col = orig.col;
		if (col)
			col->reg(this);
	}
	it = orig.it;
	return (*this);
}


UnitCollection::UnitIterator::UnitIterator(const UnitIterator& orig)
{
	col = orig.col;
	it = orig.it;
	if (col)
		col->reg(this);
}


UnitCollection::UnitIterator::UnitIterator(UnitCollection* orig)
{
	col = orig;
	for(it = orig->u.begin();it != col->u.end();++it){
		if(*it) 
			break;
	}
	col->reg(this);
}


UnitCollection::UnitIterator::~UnitIterator()
{
	if(col)
		col->unreg(this);
}


void UnitCollection::UnitIterator::remove()
{
	if(col && it != col->u.end())
		col->erase(it);
}


void UnitCollection::UnitIterator::moveBefore(UnitCollection& otherlist)
{
	if(col && it != col->u.end()) {
		otherlist.prepend(*it);
		col->erase(it);
	}
}


void UnitCollection::UnitIterator::preinsert(Unit *unit)
{
	if(col && unit)
		col->insert(it,unit);
}


void UnitCollection::UnitIterator::postinsert(Unit *unit)
{
	list<Unit*>::iterator tmp = it;
	if(col && unit && it != col->u.end()) {
		++tmp;
		col->insert(tmp,unit);
	}
}

void UnitCollection::UnitIterator::advance()
{
	if(!col || it == col->u.end()) return;
	if((*it) != NULL && (*it)->Killed())
		col->erase(it);
	else
		++it;
	while(it != col->u.end()) {
		if((*it) == NULL)
			++it;
		else if((*it)->Killed())
			col->erase(it);
		else
			break;
	}

}

Unit* UnitCollection::UnitIterator::next()
{
	advance();
	if(!col || it == col->u.end())
		return(NULL);
	return (*it);
}

// UnitIterator END:

// ConstIterator Begin:

UnitCollection::ConstIterator& UnitCollection::ConstIterator::operator=(const UnitCollection::ConstIterator& orig)
{
	col = orig.col;
	it = orig.it;
	return(*this);
}


UnitCollection::ConstIterator::ConstIterator(const ConstIterator& orig)
{
	col = orig.col;
	it = orig.it;
}


UnitCollection::ConstIterator::ConstIterator(const UnitCollection* orig)
{
	col = orig;
	for(it = orig->u.begin();it != col->u.end();++it){
		if(*it)
			break;
	}
}


UnitCollection::ConstIterator::~ConstIterator()
{;}


const Unit* UnitCollection::ConstIterator::next()
{
	advance();
	if(col && it!= col->u.end())
		return(*it);
	return(NULL);
}


bool UnitCollection::ConstIterator::isDone()
{
	if(col && it != col->u.end())
		return(false);
	return(true);
}


bool UnitCollection::ConstIterator::notDone()
{
	return(!isDone());
}


inline void UnitCollection::ConstIterator::advance()
{
	if(!col || it == col->u.end()) return;
	++it;
	while(it != col->u.end()) {
		if((*it) == NULL)
			++it;
		else if((*it)->Killed())
			++it;
		else
			break;
	}
}


const UnitCollection::ConstIterator& UnitCollection::ConstIterator::operator ++()
{
	advance();
	return(*this);
}


const UnitCollection::ConstIterator UnitCollection::ConstIterator::operator ++(int)
{
	UnitCollection::ConstIterator tmp(*this);
	advance();
	return(tmp);
}

// ConstIterator  END:

// UnitCollection  BEGIN:

UnitCollection::UnitCollection() 
{
	activeIters.reserve(20);	
}


UnitCollection::UnitCollection( const UnitCollection& uc) 
{
	list<Unit*>::const_iterator in = uc.u.begin();
	while(in != uc.u.end()) {
		append(*in);
		++in;
	}
}


UnitCollection::~UnitCollection()
{
	destr();
}


void UnitCollection::insert_unique(Unit* unit)
{
	if(unit) {
		for(list<Unit*>::iterator it = u.begin();it!= u.end();++it) {
			if(*it == unit)
				return;
		}
		unit->Ref();
		u.push_front(unit);
	}
}


void UnitCollection::prepend(Unit* unit)
{
	if(unit) {
		unit->Ref();
		u.push_front(unit);
	}
}


void UnitCollection::prepend(UnitIterator* it)
{
	Unit *tmp = NULL;
	if(!it) return;
	list<Unit*>::iterator tmpI = u.begin();
	while((tmp = **it)) {
		tmp->Ref();
		u.insert(tmpI,tmp);
		++tmpI;
		it->advance();
	}
}


void UnitCollection::append(Unit* un)
{
	if(un) {
		un->Ref();
		u.push_back(un);
	}
}


void UnitCollection::append(UnitIterator *it)
{
	if(!it) return;
	Unit *tmp = NULL;
	while((tmp = **it)) {
		tmp->Ref();
		u.push_back(tmp);
		it->advance();
	}
}


void UnitCollection::insert(list<Unit*>::iterator &temp,Unit* unit)
{
	if(unit) {
		unit->Ref();
		temp = u.insert(temp,unit);
	}
	temp = u.end();
}


void UnitCollection::clear()
{	
	if(!activeIters.empty()) return;
	printf("being called here\n");
/*	while(!removedIters.empty()){
		u.erase(removedIters.back());
		removedIters.pop_back();
	}
*/
	for(list<Unit*>::iterator it = u.begin();it != u.end();){
		(*it)->UnRef();
		(*it) = NULL;
		it = u.erase(it);
	}
}


void UnitCollection::destr()
{
	for(list<Unit*>::iterator it = u.begin();it!=u.end();++it) {
		if(*it) {
			(*it)->UnRef();
			(*it) = NULL;
		}			
	}
	for(vector<un_iter*>::iterator t = activeIters.begin();t != activeIters.end();++t)
		(*t)->col = NULL;
}


bool UnitCollection::contains(const Unit* unit) const
{
	if(u.empty() || !unit)
		return(false);
	for (list<Unit*>::const_iterator it = u.begin();it !=  u.end();++it) {
		if((*it) == unit && !(*it)->Killed())
			return(true);
	}
	return(false);
}

inline void  UnitCollection::erase(list<Unit*>::iterator &it2)
{
	// If we have more than 4 iterators, just push node onto vector.
	if(activeIters.size() > 3){
		removedIters.push_back(it2);
		(*it2)->UnRef();
		(*it2) = NULL;
		++it2;
		return;
	}
	// If we have between 2 and 4 iterators, see if any are actually 
	// on the node we want to remove, if so, just push onto vector.
	// Purpose : This special case is to reduce the size of the list in the
	//           situation where removedIters isn't being processed.
	if(activeIters.size() > 1){
		for(int i = 0;i<activeIters.size();++i){
			if(activeIters[i]->it == it2){
				removedIters.push_back(it2);
				(*it2)->UnRef();
				(*it2) = NULL;
				++it2;
				return;
			}
		}
	}
	// If we have 1 iterator, or none of the iterators are currently on the 
	// requested node to be removed, then remove it right away. 
	(*it2)->UnRef();
	(*it2) = NULL;
	it2 = u.erase(it2);
}


bool UnitCollection::remove(const Unit *unit)
{
	bool res = false;
	if(u.empty() || !unit)
		return(false);
	for(list<Unit*>::iterator it = u.begin(); it!= u.end();) {
		if((*it) == unit) {
			erase(it);		
			res = true;
		}
		else
			++it;
	}
	return (res);
}


const UnitCollection& UnitCollection::operator = (const UnitCollection& uc)
{
	destr();
	list<Unit*>::const_iterator in = uc.u.begin();
	while(in != uc.u.end()) {
		append(*in);
		++in;
	}
	return (*this);
}


inline void UnitCollection::reg(un_iter *iter)
{
	activeIters.push_back(iter);
}

inline void UnitCollection::unreg(un_iter *iter)
{
	for(vector<un_iter*>::iterator t = activeIters.begin();t != activeIters.end();++t){
		if((*t) == iter){
			activeIters.erase(t);
			break;
		}
	}
	if(activeIters.empty() || (activeIters.size() == 1 && (activeIters[0]->it == u.end() || (*(activeIters[0]->it)))) ){
		while(!removedIters.empty()){
			u.erase(removedIters.back());
			removedIters.pop_back();
		}
	}
}

// UnitCollection END:

#endif // USE_STL_COLLECTION
