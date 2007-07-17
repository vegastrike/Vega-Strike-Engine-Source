#include <list>
#include <vector>
#include "collection.h"
#include "unit_generic.h"

using std::list;
using std::vector;
// UnitIterator  BEGIN:

UnitCollection::UnitIterator& UnitCollection::UnitIterator::operator=(const UnitCollection::UnitIterator& orig)
{
	if(col != orig.col){
		if(col)
			col->unreg(this);
		// If the collection isn't the same and both aren't null, unreg old collection
		// Then set the new collection and register ourselves to it. 
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
	it = orig->u.begin();
	col->reg(this);
}


UnitCollection::UnitIterator::~UnitIterator()
{
	if(col){
		it = col->u.end();
		col->unreg(this);
	}
	col = NULL;
}


bool UnitCollection::UnitIterator::isDone()
{
	if(it != col->u.end())
		return(false);
	return(true);
}


bool UnitCollection::UnitIterator::notDone()
{
	return(!isDone());
}


void UnitCollection::UnitIterator::remove()
{
	if(col && it != col->u.end())
		it = col->erase(it);
}


void UnitCollection::UnitIterator::moveBefore(UnitCollection& otherlist)
{
	if(col && it != col->u.end()) {
		// If our current iterator isn't the end, then we want to move all the 
		// Units left in the list to the place in the argument list.
		otherlist.prepend(*it);
		it = col->erase(it);
	}
}


void UnitCollection::UnitIterator::preinsert(Unit *unit)
{
	if(col && unit)
		it = col->insert(it,unit);
}


void UnitCollection::UnitIterator::postinsert(Unit *unit)
{
	list<Unit*>::iterator tmp = it;
	if(col && unit) {
		++tmp;
		col->insert(tmp,unit);
	}
}

void UnitCollection::UnitIterator::advance()
{
	if(!col || col->u.empty() || it == col->u.end()) return;
	// If the collection is not empty, and local iterator isn't at the end
	// we iterate 
	++it;
	while(it != col->u.end()) {
		if((*it) == NULL){
			// If our new iterator isn't at the end but references a NULL, erase it.
			it = col->erase(it);
		}
		else if((*it)->Killed()){
			// If our new iterator isn't at the end but references a killed unit, erase it
			it = col->erase(it);
		}
		else
			break;
		// If our new iterator isn't the at the end, it must reference a valid unit, return it
	}
	// Our unit is either end() or valid, let the calling code deal with it.
}


Unit* UnitCollection::UnitIterator::next()
{
	advance();
	if(!col || it == col->u.end())
		return(NULL);
	return (*it);
}


const UnitCollection::UnitIterator  UnitCollection::UnitIterator::operator ++(int)
{
	UnitCollection::UnitIterator tmp(*this);
	advance();
	return(tmp);
}


const UnitCollection::UnitIterator& UnitCollection::UnitIterator::operator ++()
{
	advance();
	return(*this);
}


Unit* UnitCollection::UnitIterator::operator *()
{
	if(col && it != col->u.end() && !col->empty())
		return (*it);
	return(NULL);
	// The return value for end() and empty lists is NULL, this means attempting 
	// a * operator at end of list wont cause a segfault. Most calling code looks for NULL anyway.
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
	it = orig->u.begin();
}


UnitCollection::ConstIterator::~ConstIterator()
{
	if(col)
		it = col->u.end();
	col = NULL;
}


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


void UnitCollection::ConstIterator::advance()
{
	if(!col) return;
	// Same idea as UnitIterator only we skip ahead instead of erase.
	if(col->u.empty() && it != col->u.end()) return;
	
	++it;
	while(it != col->u.end()) {
		if((*it) == NULL)
			++it;
		if((*it)->Killed())
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
	destr();
}


UnitCollection::UnitCollection( const UnitCollection& uc)
{
	destr();
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


UnitCollection::UnitIterator UnitCollection::createIterator()
{
	return(UnitIterator(this));
}


UnitCollection::FastIterator UnitCollection::fastIterator()
{
	return(FastIterator(this));
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
		// If unit isn't null and there exists no unit like it, insert at beginning.
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
	// Basically we concatenate our list onto the end of the argument list, 
	// the result is our new list. 
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
	// Same idea as prepend but only at the end.
}


list<Unit*>::iterator UnitCollection::insert(list<Unit*>::iterator temp,Unit* unit)
{
	if(unit) {
		unit->Ref();
		return(u.insert(temp,unit));
	}
	return(u.end());
}


void UnitCollection::clear()
{	
	destr();
	// By clear we mean, comlpetely obliterate the list. same as destr()
}


void UnitCollection::destr()
{
	for(list<Unit*>::iterator it = u.begin();it!=u.end();++it) {
		(*it)->UnRef();
	}
	for(vector<UnitIterator*>::iterator t = activeIters.begin();t != activeIters.end(); ++t)
		(*t)->it = u.end();		
	u.clear();
	// First we de-reference all our Units in the list.  Then we update all our iterators to 
	// a known position that they can understand (if somehow the list is destroyed before it)
	// Then we clear the list of all units.  
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


list<Unit*>::iterator  UnitCollection::erase(list<Unit*>::iterator it2)
{
	if(u.empty() || it2 == u.end())
		return (it2);
	// If the list is empty or argument iterator is the end(), return now.
	Unit* tUnit = *it2;
	for(vector<UnitIterator*>::iterator t = activeIters.begin();t != activeIters.end(); ++t){
		if(it2 == (*t)->it){
			// The following loop is an attempt to resolve race issues between
			// iterators, since we dont trick them with a fake list. 
			do {
				++((*t)->it);
				if((*t)->it == u.end())
					break;
				// Bascially: for any active UnitIterator that is referencing the iterator
				// that we want to remove, we increment it and make sure it's not end()
				// If it's end(), we return it.
			}
			while((*(*t)->it)->Killed());
			/* The loop is run until a non-killed unit is reached. 
			*  Hopefully this is good enough.  Though I fear it is not. 
			*  Going behind the backs of classes holding an iterator
			*  between frames means they wont know when their own unit is killed.
			*  This could mean a better container should be found...not sure.
			*/
		}
	}
	it2 = u.erase(it2);	
	tUnit->UnRef();	
	// we've made sure no UnitIterator is referencing the iterator we want to remove, not even the caller.
	// So we erase and unreference it. 
	return(it2);  
	// This returns the calling un_iter to the correct place, since we incremented it 
	// earlier with the rest.
	// The caller will then have the opportunity to remove any Units that are Killed in the list
	// that we skipped over in the activeIters for loop.
}

bool UnitCollection::remove(const Unit *unit)
{
	bool res = false;
	if(u.empty() || !unit)
		return(false);
	for(list<Unit*>::iterator it = u.begin(); it!= u.end();) {
		if((*it) == unit) {
			it = erase(it);		
			res = true;
		}
		else
			++it;
	}
	// This is basically a loop wrapper around erase().
	return (res);
}


const UnitCollection& UnitCollection::operator = (const UnitCollection& uc)
{
	// This function is technically illegal.
	destr();
	list<Unit*>::const_iterator in = uc.u.begin();
	while(in != uc.u.end()) {
		append(*in);
		++in;
	}
	return (*this);
}


void UnitCollection::cleanup()
{
	for(list<Unit*>::iterator it = u.begin();it!=u.end();++it){
		if(!(*it)){
			for(vector<UnitIterator*>::iterator t = activeIters.begin();t != activeIters.end(); ++t)
				++((*t)->it);
		}
		else if ((*it)->Killed()) {
			for(vector<UnitIterator*>::iterator t = activeIters.begin();t != activeIters.end(); ++t)
				++((*t)->it);
		}
	}
	// Traverse entire list, removing all invalid units, updating active UnitIterators
}

void UnitCollection::reg(un_iter* tmp)
{
	activeIters.push_back(tmp);
}

void UnitCollection::unreg(un_iter* tmp)
{
	vector<un_iter*>::iterator t = activeIters.end() - 1;
	for(int i = activeIters.size();i > 0;--i,--t){
		if(*t == tmp){					
			activeIters.erase(t);
			return;
		}
	}
	// Traverse backwards in activeIter vector since we most likely are at the very back
	// Might increase performance a little. 
}



// UnitCollection END:
