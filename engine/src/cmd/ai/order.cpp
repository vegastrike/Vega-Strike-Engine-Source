/*
 * order.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#include "vegastrike.h"
#include "cmd/unit_generic.h"
#include "order.h"
#include "cmd/collection.h"
#include "communication.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_logging.h"

using std::vector;
using std::list;
//#define ORDERDEBUG  // FIXME ?
void Order::Execute()
{
    static float airesptime = XMLSupport::parse_float(vs_config->getVariable("AI", "CommResponseTime", "3"));
    ProcessCommunicationMessages(airesptime, true);
    int completed = 0;
    unsigned int i = 0;
    for (i = 0; i < suborders.size(); i++) {
        if ((completed & ((suborders[i])->getType() & (ALLTYPES))) == 0) {
            (suborders[i])->Execute();
            completed |= (suborders[i])->getType();
            if ((suborders[i])->Done()) {
                vector<Order *>::iterator ord = suborders.begin() + i;
                (*ord)->Destroy();
                suborders.erase(ord);
                i--;
            }
        }
    }
    if (suborders.size() == 0) {
        done = true;
    } else {
        done = false;
    }
}

Order *Order::queryType(unsigned int type)
{
    for (unsigned int i = 0; i < suborders.size(); i++) {
        if ((suborders[i]->type & type) == type) {
            return suborders[i];
        }
    }
    return nullptr;
}

Order *Order::queryAny(unsigned int type)
{
    for (unsigned int i = 0; i < suborders.size(); i++) {
        if ((suborders[i]->type & type) != 0) {
            return suborders[i];
        }
    }
    return nullptr;
}

void Order::eraseType(unsigned int type)
{
    for (unsigned int i = 0; i < suborders.size(); i++) {
        if ((suborders[i]->type & type) == type) {
            suborders[i]->Destroy();
            vector<Order *>::iterator j = suborders.begin() + i;
            suborders.erase(j);
            i--;
        }
    }
}

Order *Order::EnqueueOrder(Order *ord)
{
    if (ord == nullptr) {
        VS_LOG(warning, "NOT ENQEUEING NULL ORDER");
        VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
        return nullptr;
    }
    ord->SetParent(parent);
    suborders.push_back(ord);
    return this;
}

Order *Order::EnqueueOrderFirst(Order *ord)
{
    if (ord == nullptr) {
        VS_LOG(warning, "NOT ENQEUEING NULL ORDER");
        VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
        return nullptr;
    }
    ord->SetParent(parent);

    vector<Order *>::iterator first_elem = suborders.begin();
    suborders.insert(first_elem, ord);
    return this;
}

Order *Order::ReplaceOrder(Order *ord)
{
    for (vector<Order *>::iterator ordd = suborders.begin(); ordd != suborders.end();) {
        if ((ord->getType() & (*ordd)->getType() & (ALLTYPES))) {
            (*ordd)->Destroy();
            ordd = suborders.erase(ordd);
        } else {
            ordd++;
        }
    }
    suborders.push_back(ord);
    return this;
}

bool Order::AttachOrder(Unit *targets1)
{
    if (!(subtype & STARGET)) {
        if (subtype & SSELF) {
            return AttachSelfOrder(targets1);
        }              //can use attach order to do shit

        return false;
    }
    parent->Target(targets1);
    return true;
}

bool Order::AttachSelfOrder(Unit *targets1)
{
    if (!(subtype & SSELF)) {
        return false;
    }
    group.SetUnit(targets1);
    return true;
}

bool Order::AttachOrder(QVector targetv)
{
    if (!(subtype & SLOCATION)) {
        return false;
    }
    targetlocation = targetv;
    return true;
}

Order *Order::findOrder(Order *ord)
{
    if (ord == nullptr) {
        VS_LOG(warning, "FINDING EMPTY ORDER");
        VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
        return nullptr;
    }
    for (unsigned int i = 0; i < suborders.size(); i++) {
        if (suborders[i] == ord) {
            return suborders[i];
        }
    }
    return nullptr;
}

Order::~Order()
{
    VSDESTRUCT1
}

void Order::Destructor()
{
    delete this;
}

void Order::Destroy()
{
    unsigned int i;
    for (i = 0; i < suborders.size(); ++i) {
        if (suborders[i] == nullptr) {
            VS_LOG(warning, "ORDER: a null order");
            VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
        } else {
            suborders[i]->Destroy();
        }
    }
    {
        for (list<CommunicationMessage *>::iterator i = messagequeue.begin(); i != messagequeue.end(); ++i) {
            delete (*i);
        }
    }
    messagequeue.clear();
    suborders.clear();
    this->Destructor();
}

void Order::ClearMessages()
{
    unsigned int i;
    for (i = 0; i < suborders.size(); i++) {
        suborders[i]->ClearMessages();
    }
    {
        for (list<CommunicationMessage *>::iterator i = messagequeue.begin(); i != messagequeue.end(); i++) {
            delete (*i);
        }
    }
    messagequeue.clear();
}

void Order::eraseOrder(Order *ord)
{
    bool found = false;
    if (ord == nullptr) {
        VS_LOG(warning, "NOT ERASING A NULL ORDER");
        VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
        return;
    }
    for (unsigned int i = 0; i < suborders.size() && found == false; i++) {
        if (suborders[i] == ord) {
            suborders[i]->Destroy();
            vector<Order *>::iterator j = suborders.begin() + i;
            suborders.erase(j);
            found = true;
        }
    }
    if (!found) {
        VS_LOG(warning, "TOLD TO ERASE AN ORDER - NOT FOUND");
        VS_LOG(warning, (boost::format("this order: %1%") % getOrderDescription().c_str()));
    }
}

Order *Order::findOrderList()
{
    olist_t *orderlist = getOrderList();
    if (orderlist) {
        return this;
    }
    Order *found_order = NULL;
    for (unsigned int i = 0; i < suborders.size() && found_order == NULL; ++i) {
        found_order = suborders[i]->findOrderList();
    }
    return found_order;
}

string Order::createFullOrderDescription(int level)
{
    string tabs;
    for (int i = 0; i < level; ++i) {
        tabs = tabs + "   ";
    }
    string desc = tabs + "+" + getOrderDescription() + "\n";
    for (unsigned int j = 0; j < suborders.size(); ++j) {
        desc = desc + suborders[j]->createFullOrderDescription(level + 1);
    }
    return desc;
}

namespace Orders {

void ExecuteFor::Execute()
{
    if (child) {
        child->SetParent(parent);
        type = child->getType();
    }
    if (time > maxtime) {
        done = true;
        return;
    }
    time += SIMULATION_ATOM;
    if (child) {
        child->Execute();
    }
}

Join::Join(Unit *parent, Order *first, Order *second)
        : Order(first->getType() | second->getType(),
                first->getSubType()),
          first(first),
          second(second)
{
    assert((first->getType() & second->getType()) == 0);
    assert(first->getSubType() == second->getSubType());

    SetParent(parent);
    EnqueueOrder(first);
    EnqueueOrder(second);
}

void Join::Execute()
{
    // Execute both sub-orders
    Order::Execute();
    // Wait for both sub-orders to have finished
    if (first->Done() && second->Done()) {
        done = true;
    }
}

Sequence::Sequence(Unit *parent, Order *order, unsigned int excludeTypes)
        : Order(order->getType() | excludeTypes,
                order->getSubType()),
          order(order)
{
    SetParent(parent);
    EnqueueOrder(order);
}

void Sequence::Execute()
{
    Order::Execute();
    if (order->Done()) {
        done = true;
    }
}

} // namespace Orders
