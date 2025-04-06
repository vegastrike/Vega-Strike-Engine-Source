/**
 * order_comm.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2025 Stephen G. Tuggy, and other Vega Strike Contributors
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "order.h"
#include "communication.h"
#include "root_generic/configxml.h"

using std::list;
using std::vector;

void Order::AdjustRelationTo(Unit *un, float factor) {
    //virtual stub function
}

void Order::Communicate(const CommunicationMessage &c) {
    int completed = 0;
    unsigned int i = 0;
    CommunicationMessage *newC = new CommunicationMessage(c);
    for (i = 0; i < suborders.size(); i++) {
        if ((completed & ((suborders[i])->getType() & (MOVEMENT | FACING | WEAPON))) == 0) {
            (suborders[i])->Communicate(*newC);
            completed |= (suborders[i])->getType();
        }
    }
    Unit *un;
    bool already_communicated = false;
    for (list<CommunicationMessage *>::iterator ii = messagequeue.begin(); ii != messagequeue.end(); ii++) {
        un = (*ii)->sender.GetUnit();
        bool thisissender = (un == newC->sender.GetUnit());
        if (un == NULL || thisissender) {
            delete (*ii);
            if (thisissender) {
                already_communicated = true;
            }
            if ((ii = messagequeue.erase(ii)) == messagequeue.end()) {
                break;
            }
        }
    }
    if ((un = newC->sender.GetUnit())) {
        if (un != parent) {
            const bool talk_more_helps = vega_config::config->ai.talking_faster_helps;
            const float talk_factor = vega_config::config->ai.talk_relation_factor;
            if (talk_more_helps || !already_communicated) {
                AdjustRelationTo(un, newC->getDeltaRelation() * talk_factor);
            }
            messagequeue.push_back(newC);
        }
    }
}

void Order::ProcessCommMessage(CommunicationMessage &c) {
}

void Order::ProcessCommunicationMessages(float AICommresponseTime, bool RemoveMessageProcessed) {
    float time = AICommresponseTime / SIMULATION_ATOM;
    if (time <= .001) {
        time += .001;
    }
    if (!messagequeue.empty()) {
        bool cleared = false;
        if (messagequeue.back()->curstate == messagequeue.back()->fsm->GetRequestLandNode()) {
            cleared = true;
            RemoveMessageProcessed = true;
            Unit *un = messagequeue.back()->sender.GetUnit();
            if (un) {
                CommunicationMessage c(parent, un, NULL, 0);
                if (parent->getRelation(un) >= 0
                        || (parent->getFlightgroup() && parent->getFlightgroup()->name == "Base")) {
                    parent->RequestClearance(un);
                    c.SetCurrentState(c.fsm->GetAbleToDockNode(), NULL, 0);
                } else {
                    c.SetCurrentState(c.fsm->GetUnAbleToDockNode(), NULL, 0);
                }
                Order *o = un->getAIState();
                if (o) {
                    o->Communicate(c);
                }
            }
        }
        if (cleared || (((float) rand()) / RAND_MAX) < (1 / time)) {
            FSM::Node *n;
            if ((n = messagequeue.back()->getCurrentState())) {
                ProcessCommMessage(*messagequeue.back());
            }
            if (RemoveMessageProcessed) {
                delete messagequeue.back();
                messagequeue.pop_back();
            } else {
                messagequeue.push_front(messagequeue.back());
                messagequeue.pop_back();
            }
        }
    }
}

