/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2022 Stephen G. Tuggy
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

/*
 *  xml Mission Scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"
#include "cmd/collection.h"
#include "cmd/unit_generic.h"
#include "cmd/ai/order.h"
#include "cmd/ai/aggressive.h"
#include "cmd/ai/navigation.h"
#include "cmd/ai/flybywire.h"
#include "cmd/ai/tactics.h"
#include "cmd/ai/missionscript.h"
#include "gfx/cockpit_generic.h"
#include "mission.h"
#include "easydom.h"
#include "universe.h"

#include "vs_globals.h"
#include "configxml.h"

/* *********************************************************** */

varInst *Mission::call_order(missionNode *node, int mode)
{
    varInst *viret = NULL;
    if (mode == SCRIPT_PARSE) {
        string cmd = node->attr_value("name");
        node->script.method_id = module_order_map[cmd];
    }
    callback_module_order_type method_id = (callback_module_order_type) node->script.method_id;
    if (method_id == CMT_ORDER_newAggressiveAI) {
        string filestr = getStringArgument(node, mode, 0);
        string intstr = getStringArgument(node, mode, 1);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new Orders::AggressiveAI(filestr.c_str());
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        return viret;
    } else if (method_id == CMT_ORDER_newMoveTo) {
        missionNode *pos_node = getArgument(node, mode, 0);
        varInst *pos_vi = checkObjectExpr(pos_node, mode);
        missionNode *ab_node = getArgument(node, mode, 1);
        bool afterburn = checkBoolExpr(ab_node, mode);
        missionNode *sw_node = getArgument(node, mode, 2);
        int nr_switchbacks = checkIntExpr(sw_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            QVector vec3 = call_olist_tovector(pos_node, mode, pos_vi);
            my_order = new Orders::MoveTo(vec3, afterburn, nr_switchbacks);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(pos_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newChangeHeading) {
        missionNode *pos_node = getArgument(node, mode, 0);
        varInst *pos_vi = checkObjectExpr(pos_node, mode);
        missionNode *sw_node = getArgument(node, mode, 1);
        int nr_switchbacks = checkIntExpr(sw_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            QVector vec3 = call_olist_tovector(pos_node, mode, pos_vi);
            my_order = new Orders::ChangeHeading(vec3, nr_switchbacks);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(pos_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newFaceTarget) {
        missionNode *itts_node = getArgument(node, mode, 0);
        bool itts = checkBoolExpr(itts_node, mode);
        missionNode *fini_node = getArgument(node, mode, 1);
        bool fini = checkBoolExpr(fini_node, mode);
        missionNode *acc_node = getArgument(node, mode, 2);
        int acc = checkIntExpr(acc_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            if (itts) {
                my_order = new Orders::FaceTargetITTS(fini, acc);
            } else {
                my_order = new Orders::FaceTarget(fini, acc);
            }
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        return viret;
    } else if (method_id == CMT_ORDER_newFireAt) {
        missionNode *aggr_node = getArgument(node, mode, 1);
        float aggr = checkFloatExpr(aggr_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new Orders::FireAt(aggr);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        return viret;
    } else if (method_id == CMT_ORDER_newExecuteFor) {
        missionNode *enq_node = getArgument(node, mode, 0);
        varInst *enq_vi = checkObjectExpr(enq_node, mode);
        Order *enq_order = getOrderObject(enq_node, mode, enq_vi);
        missionNode *time_node = getArgument(node, mode, 1);
        float fortime = checkFloatExpr(time_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new Orders::ExecuteFor(enq_order, fortime);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(enq_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newCloakFor) {
        missionNode *val_node = getArgument(node, mode, 0);
        bool res = checkBoolExpr(val_node, mode);
        missionNode *time_node = getArgument(node, mode, 1);
        float fortime = checkFloatExpr(time_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new CloakFor(res, fortime);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        return viret;
    } else if (method_id == CMT_ORDER_newMatchVelocity) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        missionNode *desa_node = getArgument(node, mode, 1);
        varInst *desa_vi = checkObjectExpr(desa_node, mode);
        missionNode *local_node = getArgument(node, mode, 2);
        bool local = checkBoolExpr(local_node, mode);
        missionNode *afburn_node = getArgument(node, mode, 3);
        bool afburn = checkBoolExpr(afburn_node, mode);
        missionNode *fini_node = getArgument(node, mode, 4);
        bool fini = checkBoolExpr(fini_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            Vector des3 = call_olist_tovector(des_node, mode, des_vi).Cast();
            Vector desa3 = call_olist_tovector(desa_node, mode, desa_vi).Cast();
            my_order = new Orders::MatchVelocity(des3, desa3, local, afburn, fini);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        deleteVarInst(desa_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newMatchAngularVelocity) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        missionNode *local_node = getArgument(node, mode, 1);
        bool local = checkBoolExpr(local_node, mode);
        missionNode *fini_node = getArgument(node, mode, 2);
        bool fini = checkBoolExpr(fini_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            Vector des3 = call_olist_tovector(des_node, mode, des_vi).Cast();
            my_order = new Orders::MatchAngularVelocity(des3, local, fini);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newMatchLinearVelocity) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        missionNode *local_node = getArgument(node, mode, 1);
        bool local = checkBoolExpr(local_node, mode);
        missionNode *afburn_node = getArgument(node, mode, 2);
        bool afburn = checkBoolExpr(afburn_node, mode);
        missionNode *fini_node = getArgument(node, mode, 3);
        bool fini = checkBoolExpr(fini_node, mode);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            Vector des3 = call_olist_tovector(des_node, mode, des_vi).Cast();
            my_order = new Orders::MatchLinearVelocity(des3, local, afburn, fini);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newFlyToWaypoint) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        float vel = getFloatArg(node, mode, 1);
        bool afburn = getBoolArg(node, mode, 2);
        float range = getFloatArg(node, mode, 3);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            QVector des3 = call_olist_tovector(des_node, mode, des_vi);
            my_order = new AIFlyToWaypoint(des3, vel, afburn, range);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newFlyToWaypointDefend) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        float vel = getFloatArg(node, mode, 1);
        bool afburn = getBoolArg(node, mode, 2);
        float range = getFloatArg(node, mode, 3);
        float defend_range = getFloatArg(node, mode, 4);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            QVector des3 = call_olist_tovector(des_node, mode, des_vi);
            my_order = new AIFlyToWaypointDefend(des3, vel, afburn, range, defend_range);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newFlyToJumppoint) {
        missionNode *des_node = getArgument(node, mode, 0);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        Unit *des_unit = getUnitObject(des_node, mode, des_vi);
        float vel = getFloatArg(node, mode, 1);
        bool afburn = getBoolArg(node, mode, 2);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new AIFlyToJumppoint(des_unit, vel, afburn);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newPatrol) {
        int patrol_mode = getIntArg(node, mode, 0);
        missionNode *des_node = getArgument(node, mode, 1);
        varInst *des_vi = checkObjectExpr(des_node, mode);
        double range = getFloatArg(node, mode, 2);
        missionNode *unit_node = getArgument(node, mode, 3);
        varInst *unit_vi = checkObjectExpr(unit_node, mode);
        Unit *around_unit = getUnitObject(unit_node, mode, unit_vi);
        float patrol_speed = getFloatArg(node, mode, 4);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            QVector des3 = call_olist_tovector(des_node, mode, des_vi);
            my_order = new AIPatrol(patrol_mode, des3, range, around_unit, patrol_speed);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(des_vi);
        deleteVarInst(unit_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newOrderList) {
        missionNode *unit_node = getArgument(node, mode, 0);
        varInst *unit_vi = checkObjectExpr(unit_node, mode);
        olist_t *orderlist = getOListObject(unit_node, mode, unit_vi);
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new AIOrderList(orderlist);
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        deleteVarInst(unit_vi);
        return viret;
    } else if (method_id == CMT_ORDER_newSuperiority) {
        Order *my_order = NULL;
        if (mode == SCRIPT_RUN) {
            my_order = new AISuperiority();
        }
        viret = newVarInst(VI_TEMP);
        viret->type = VAR_OBJECT;
        viret->objectname = "order";
        viret->object = (void *) my_order;
        return viret;
    } else {
        varInst *ovi = getObjectArg(node, mode);
        Order *my_order = getOrderObject(node, mode, ovi);
        if (mode == SCRIPT_RUN) {
            Unit *player = _Universe->AccessCockpit()->GetParent();
            if (player) {
                if (my_order == player->getAIState()) {
                    printf("IGNOREING order for player\n");
                    viret = newVarInst(VI_TEMP);
                    viret->type = VAR_VOID;
                    return viret;
                }
            }
        }
        if (method_id == CMT_ORDER_enqueueOrder) {
            missionNode *enq_node = getArgument(node, mode, 1);
            varInst *enq_vi = checkObjectExpr(enq_node, mode);
            if (mode == SCRIPT_RUN) {
                debug(3, node, mode, "enqueueing order");
            }
            deleteVarInst(enq_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_enqueueOrderFirst) {
            missionNode *enq_node = getArgument(node, mode, 1);
            varInst *enq_vi = checkObjectExpr(enq_node, mode);
            if (mode == SCRIPT_RUN) {
                debug(3, node, mode, "enqueueing order as first");
            }
            deleteVarInst(enq_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_eraseOrder) {
            missionNode *enq_node = getArgument(node, mode, 1);
            varInst *enq_vi = checkObjectExpr(enq_node, mode);
            Order *enq_order = getOrderObject(enq_node, mode, enq_vi);
            if (mode == SCRIPT_RUN) {
                my_order->eraseOrder(enq_order);
                debug(3, node, mode, "erasing order");
            }
            deleteVarInst(enq_vi);
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_findOrder) {
            missionNode *enq_node = getArgument(node, mode, 1);
            varInst *enq_vi = checkObjectExpr(enq_node, mode);
            Order *enq_order = getOrderObject(enq_node, mode, enq_vi);
            Order *res_order = NULL;
            if (mode == SCRIPT_RUN) {
                res_order = my_order->findOrder(enq_order);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_OBJECT;
            viret->objectname = "order";
            viret->object = (void *) res_order;
            deleteVarInst(enq_vi);
        } else if (method_id == CMT_ORDER_SteerUp) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->Up(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerRight) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->Right(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerRollRight) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->RollRight(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerStop) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->Stop(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerAccel) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->Accel(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerAfterburn) {
            missionNode *val_node = getArgument(node, mode, 1);
            float val = checkFloatExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->Afterburn(val);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_SteerSheltonSlide) {
            missionNode *val_node = getArgument(node, mode, 1);
            bool res = checkBoolExpr(val_node, mode);
            if (mode == SCRIPT_RUN) {
                //this will crash if order is no FlyByWire
                //is there a way to check that?
                ((FlyByWire *) my_order)->SheltonSlide(res);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_print) {
            if (mode == SCRIPT_RUN) {
                printf("print: order=%s\n", my_order->getOrderDescription().c_str());
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else if (method_id == CMT_ORDER_setActionString) {
            string astring = getStringArgument(node, mode, 1);
            if (mode == SCRIPT_RUN) {
                my_order->setActionString(astring);
            }
            viret = newVarInst(VI_TEMP);
            viret->type = VAR_VOID;
        } else {
            fatalError(node, mode, "no such method in orders " + node->script.name);
            assert(0);
        }
        deleteVarInst(ovi);
        return viret;
    }
    return NULL;     //never reach
}

Order *Mission::getOrderObject(missionNode *node, int mode, varInst *ovi)
{
    Order *my_object = NULL;
    if (mode == SCRIPT_RUN) {
        my_object = (Order *) ovi->object;
        if (my_object == NULL) {
            fatalError(node, mode, "order: no object");
            assert(0);
        }
    }
    return my_object;
}

