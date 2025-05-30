/*
 * collection.cpp
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


#include "cmd/collection.h"

#if defined (USE_OLD_COLLECTION)
#include "oldcollection.cpp"
#elif defined (USE_STL_COLLECTION)

#include <list>
#include <vector>
#ifndef LIST_TESTING
#include "cmd/unit_util.h"
#include "cmd/unit_generic.h"

#else
#include "testcollection/unit.h"
#endif

#include "src/vs_logging.h"

using std::list;
using std::vector;
//UnitIterator  BEGIN:

UnitCollection::UnitIterator &UnitCollection::UnitIterator::operator=(const UnitCollection::UnitIterator &orig) {
    if (col != orig.col) {
        if (col) {
            col->unreg(this);
        }
        col = orig.col;
        if (col) {
            col->reg(this);
        }
    }
    it = orig.it;
    return *this;
}

UnitCollection::UnitIterator::UnitIterator(const UnitIterator &orig) {
    col = orig.col;
    it = orig.it;
    if (col) {
        col->reg(this);
    }
}

UnitCollection::UnitIterator::UnitIterator(UnitCollection *orig) {
    col = orig;
    it = col->u.begin();
    col->reg(this);
    while (it != col->u.end()) {
        if ((*it) == NULL) {
            ++it;
        } else {
            if ((*it)->Killed()) {
                col->erase(it);
            } else {
                break;
            }
        }
    }
}

UnitCollection::UnitIterator::~UnitIterator() {
    if (col) {
        col->unreg(this);
    }
}

void UnitCollection::UnitIterator::remove() {
    if (col && it != col->u.end()) {
        col->erase(it);
    }
}

void UnitCollection::UnitIterator::moveBefore(UnitCollection &otherlist) {
    if (col && it != col->u.end()) {
        otherlist.prepend(*it);
        col->erase(it);
    }
}

void UnitCollection::UnitIterator::preinsert(Unit *unit) {
    if (col && unit) {
        col->insert(it, unit);
    }
}

void UnitCollection::UnitIterator::postinsert(Unit *unit) {
    list<Unit *>::iterator tmp = it;
    if (col && unit && it != col->u.end()) {
        ++tmp;
        col->insert(tmp, unit);
    }
}

void UnitCollection::UnitIterator::advance() {
    if (!col || it == col->u.end()) {
        return;
    }
    ++it;
    while (it != col->u.end()) {
        if ((*it) == NULL) {
            ++it;
        } else {
            if ((*it)->Killed()) {
                col->erase(it);
            } else {
                break;
            }
        }
    }
}

Unit *UnitCollection::UnitIterator::next() {
    advance();
    return *it;
}

//UnitIterator END:

//ConstIterator Begin:

UnitCollection::ConstIterator &UnitCollection::ConstIterator::operator=(const UnitCollection::ConstIterator &orig) {
    col = orig.col;
    it = orig.it;
    return *this;
}

UnitCollection::ConstIterator::ConstIterator(const ConstIterator &orig) {
    col = orig.col;
    it = orig.it;
}

UnitCollection::ConstIterator::ConstIterator(const UnitCollection *orig) {
    col = orig;
    for (it = orig->u.begin(); it != col->u.end(); ++it) {
        if ((*it) && !(*it)->Killed()) {
            break;
        }
    }
}

UnitCollection::ConstIterator::~ConstIterator() {
}

Unit *UnitCollection::ConstIterator::next() {
    advance();
    if (col && it != col->u.end()) {
        return *it;
    }
    return NULL;
}

inline void UnitCollection::ConstIterator::advance() {
    if (!col || it == col->u.end()) {
        return;
    }
    ++it;
    while (it != col->u.end()) {
        if ((*it) == NULL) {
            ++it;
        } else {
            if ((*it)->Killed()) {
                ++it;
            } else {
                break;
            }
        }
    }
}

const UnitCollection::ConstIterator &UnitCollection::ConstIterator::operator++() {
    advance();
    return *this;
}

const UnitCollection::ConstIterator UnitCollection::ConstIterator::operator++(int) {
    UnitCollection::ConstIterator tmp(*this);
    advance();
    return tmp;
}

//ConstIterator  END:

//UnitCollection  BEGIN:

UnitCollection::UnitCollection() {
    activeIters.reserve(20);
}

UnitCollection::UnitCollection(const UnitCollection &uc) {
    list<Unit *>::const_iterator in = uc.u.begin();
    while (in != uc.u.end()) {
        append(*in);
        ++in;
    }
}

void UnitCollection::insert_unique(Unit *unit) {
    if (unit) {
        for (list<Unit *>::iterator it = u.begin(); it != u.end(); ++it) {
            if (*it == unit) {
                return;
            }
        }
        unit->Ref();
        u.push_front(unit);
    }
}

void UnitCollection::prepend(Unit *unit) {
    if (unit) {
        unit->Ref();
        u.push_front(unit);
    }
}

void UnitCollection::prepend(UnitIterator *it) {
    Unit *tmp = NULL;
    if (!it) {
        return;
    }
    list<Unit *>::iterator tmpI = u.begin();
    while ((tmp = **it)) {
        tmp->Ref();
        u.insert(tmpI, tmp);
        ++tmpI;
        it->advance();
    }
}

void UnitCollection::append(Unit *un) {
    if (un) {
        un->Ref();
        u.push_back(un);
    }
}

void UnitCollection::append(UnitIterator *it) {
    if (!it) {
        return;
    }
    Unit *tmp = NULL;
    while ((tmp = **it)) {
        tmp->Ref();
        u.push_back(tmp);
        it->advance();
    }
}

void UnitCollection::insert(list<Unit *>::iterator &temp, Unit *unit) {
    if (unit) {
        unit->Ref();
        temp = u.insert(temp, unit);
    }
    temp = u.end();
}

void UnitCollection::clear() {
    if (!activeIters.empty()) {
        VS_LOG(warning, "WARNING! Attempting to clear a collection with active iterators!\n");
        return;
    }

    for (list<Unit *>::iterator it = u.begin(); it != u.end(); ++it) {
        (*it)->UnRef();
        (*it) = NULL;
    }
    u.clear();
}

void UnitCollection::destr() {
    for (list<Unit *>::iterator it = u.begin(); it != u.end(); ++it) {
        if (*it) {
            (*it)->UnRef();
            (*it) = NULL;
        }
    }
    for (vector<un_iter *>::iterator t = activeIters.begin(); t != activeIters.end(); ++t) {
        (*t)->col = NULL;
    }
}

bool UnitCollection::contains(const Unit *unit) const {
    if (u.empty() || !unit) {
        return false;
    }
    for (list<Unit *>::const_iterator it = u.begin(); it != u.end(); ++it) {
        if ((*it) == unit && !(*it)->Killed()) {
            return true;
        }
    }
    return false;
}

inline void UnitCollection::erase(list<Unit *>::iterator &it2) {
    if (!(*it2)) {
        ++it2;
        return;
    }
    //If we have more than 4 iterators, just push node onto vector.
    if (activeIters.size() > 3) {
        removedIters.push_back(it2);
        (*it2)->UnRef();
        (*it2) = NULL;
        ++it2;
        return;
    }
    //If we have between 2 and 4 iterators, see if any are actually
    //on the node we want to remove, if so, just push onto vector.
    //Purpose : This special case is to reduce the size of the list in the
    //situation where removedIters isn't being processed.
    if (activeIters.size() > 1) {
        for (vector<UnitCollection::UnitIterator *>::size_type i = 0; i < activeIters.size(); ++i) {
            if (activeIters[i]->it == it2) {
                removedIters.push_back(it2);
                (*it2)->UnRef();
                (*it2) = NULL;
                ++it2;
                return;
            }
        }
    }
    //If we have 1 iterator, or none of the iterators are currently on the
    //requested node to be removed, then remove it right away.
    (*it2)->UnRef();
    (*it2) = NULL;
    it2 = u.erase(it2);
}

bool UnitCollection::remove(const Unit *unit) {
    if (u.empty() || !unit) {
        return false;
    }
    for (list<Unit *>::iterator it = u.begin(); it != u.end(); ++it) {
        if ((*it) == unit) {
            erase(it);
            return (true);
        }
    }
    return (false);
}

const UnitCollection &UnitCollection::operator=(const UnitCollection &uc) {
    destr();
    list<Unit *>::const_iterator in = uc.u.begin();
    while (in != uc.u.end()) {
        append(*in);
        ++in;
    }
    return *this;
}

inline void UnitCollection::reg(un_iter *iter) {
    activeIters.push_back(iter);
}

inline void UnitCollection::unreg(un_iter *iter) {
    for (vector<un_iter *>::iterator t = activeIters.begin(); t != activeIters.end(); ++t) {
        if ((*t) == iter) {
            activeIters.erase(t);
            break;
        }
    }
    if (activeIters.empty()
            || (activeIters.size() == 1 && (activeIters[0]->it == u.end() || (*(activeIters[0]->it))))) {
        while (!removedIters.empty()) {
            u.erase(removedIters.back());
            removedIters.pop_back();
        }
    }
}

//UnitCollection END:

#endif //USE_STL_COLLECTION

