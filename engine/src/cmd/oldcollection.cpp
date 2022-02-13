/**
 * oldcollection.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include <stdlib.h>
#include <vector>
#include "collection.h"
#include <assert.h>
#ifndef LIST_TESTING
#include "unit_generic.h"
#endif

UnitCollection::UnitListNode::UnitListNode(Unit *unit) : unit(unit), next(NULL) {
    if (unit) {
        unit->Ref();
    }
}

UnitCollection::UnitListNode::UnitListNode(Unit *unit, UnitListNode *next) : unit(unit), next(next) {
    if (unit) {
        unit->Ref();
    }
}

UnitCollection::UnitListNode::~UnitListNode() {
    if (NULL != unit) {
        unit->UnRef();
    }
    unit = NULL;
    next = NULL;
}

void UnitCollection::destr() {
    UnitListNode *tmp;
    while (u->next) {
        tmp = u->next;
        u->next = u->next->next;
        PushUnusedNode(tmp);
    }
    u->unit = NULL;
    u->next = NULL;
    PushUnusedNode(u);
}

void *UnitCollection::PushUnusedNode(UnitListNode *node) {
    static UnitListNode cat(NULL, NULL);
    static UnitListNode dog(NULL, &cat);
    static bool cachunk = true;
    if (cachunk) {
        cachunk = false;
    }
    static std::vector<UnitCollection::UnitListNode *> dogpile;
    if (node == NULL) {
        return &dogpile;
    } else {
        node->next = &dog;
        dogpile.push_back(node);
    }
    return NULL;
}

void UnitCollection::FreeUnusedNodes() {
    static std::vector<UnitCollection::UnitListNode *> bakdogpile;
    std::vector<UnitCollection::UnitListNode *>
            *dogpile = (std::vector<UnitCollection::UnitListNode *> *) PushUnusedNode(
            NULL);
    bakdogpile.swap(*dogpile);
    while (!dogpile->empty()) {
        delete dogpile->back();
        dogpile->pop_back();
    }
}

void UnitCollection::UnitIterator::moveBefore(UnitCollection &otherList) {
    if (pos->next->unit) {
        UnitListNode *tmp = pos->next->next;
        otherList.prepend(pos->next);
        pos->next = tmp;
    } else {
        assert(0);
    }
}

void UnitCollection::prepend(UnitIterator *iter) {
    UnitListNode *n = u;
    Unit *tmp;
    while ((tmp = iter->current())) {
        //iter->current checks for killed()
        n->next = new UnitListNode(tmp, n->next);
        iter->advance();
    }
}

void UnitCollection::append(UnitIterator *iter) {
    UnitListNode *n = u;
    while (n->next->unit != NULL) {
        n = n->next;
    }
    Unit *tmp;
    while ((tmp = iter->current())) {
        n->next = new UnitListNode(tmp, n->next);
        n = n->next;
        iter->advance();
    }
}

void UnitCollection::append(Unit *unit) {
    UnitListNode *n = u;
    while (n->next->unit != NULL) {
        n = n->next;
    }
    n->next = new UnitListNode(unit, n->next);
}

void UnitCollection::UnitListNode::PostInsert(Unit *unit) {
    if (next->unit != NULL) {
        next->next = new UnitListNode(unit, next->next);
    } else {
        next = new UnitListNode(unit, next);
    }
}

void UnitCollection::UnitIterator::postinsert(Unit *unit) {
    pos->PostInsert(unit);
}

void UnitCollection::FastIterator::postinsert(Unit *unit) {
    pos->PostInsert(unit);
}

void UnitCollection::UnitListNode::Remove() {
    if (next->unit) {
        UnitListNode *tmp = next->next;
        //delete next; //takes care of unref! And causes a shitload of bugs
        //concurrent lists, man
        PushUnusedNode(next);
        next = tmp;
    } else {
        assert(0);
    }
}

void UnitCollection::UnitIterator::remove() {
    pos->Remove();
}

void UnitCollection::FastIterator::remove() {
    pos->Remove();
}

void UnitCollection::ConstIterator::GetNextValidUnit() {
    while (pos->next->unit ? pos->next->unit->Killed() : false) {
        pos = pos->next;
    }
}

const UnitCollection &UnitCollection::operator=(const UnitCollection &uc) {
#ifdef _DEBUG
    printf( "warning could cause problems with concurrent lists. Make sure no one is traversing gotten list" );
#endif
    destr();
    init();
    un_iter ui = createIterator();
    const UnitListNode *n = uc.u;
    while (n) {
        if (n->unit) {
            ui.postinsert(n->unit);
            ++ui;
        }
        n = n->next;
    }
    return uc;
}

UnitCollection::UnitCollection(const UnitCollection &uc) : u(NULL) {
    init();
    un_iter ui = createIterator();
    const UnitListNode *n = uc.u;
    while (n) {
        if (n->unit) {
            ui.postinsert(n->unit);
            ++ui;
        }
        n = n->next;
    }
}

bool UnitCollection::contains(const Unit *unit) const {
    if (empty()) {
        return false;
    }
    ConstFastIterator it = constFastIterator();
    while (!it.isDone()) {
        if (it.current() == unit) {
            return true;
        } else {
            it.advance();
        }
    }
    return false;
}

bool UnitCollection::remove(const Unit *unit) {
    bool res = false;
    if (empty()) {
        return false;
    }
    FastIterator it = fastIterator();
    while (!it.isDone()) {
        if (it.current() == unit) {
            it.remove(), res = true;
        } else {
            it.advance();
        }
    }
    return res;
}

void UnitCollection::cleanup() {
    //NOTE: advance() will be cleaning up the list by itself
    un_iter ui = createIterator();
    while (!ui.isDone()) {
        ui.advance();
    }
}

