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
#ifndef WIN32
//this file isn't available on my system (all win32 machines?) i dun even know what it has or if we need it as I can compile without it
#include <unistd.h>
#endif

#include <expat.h>
#include "xml_support.h"

#include "vegastrike.h"

#include "cmd/unit_generic.h"
#include "mission.h"
#include "easydom.h"

void Mission::checkStatement(missionNode *node, int mode) {
    //no difference
    if (node->tag == DTAG_IF) {
        doIf(node, mode);
    } else if (node->tag == DTAG_BLOCK) {
        doBlock(node, mode);
    } else if (node->tag == DTAG_SETVAR) {
        doSetVar(node, mode);
    } else if (node->tag == DTAG_DEFVAR) {
        doDefVar(node, mode);
    } else if (node->tag == DTAG_EXEC) {
        doExec(node, mode);
    } else if (node->tag == DTAG_RETURN) {
        doReturn(node, mode);
    } else if (node->tag == DTAG_CALL) {
        varInst *vi = doCall(node, mode);
        if (vi->type != VAR_VOID) {
            fatalError(node, mode, "expected void as return from call, got different");
            assert(0);
        }
        deleteVarInst(vi);
    } else if (node->tag == DTAG_WHILE) {
        doWhile(node, mode);
    }
}

void Mission::doIf(missionNode *node, int mode) {
    if (mode == SCRIPT_PARSE) {
        vector<easyDomNode *>::const_iterator siter;

        int nr_subnodes = node->subnodes.size();
        if (nr_subnodes != 3) {
            fatalError(node, mode, (boost::format("an if-statement needs exact three subnodes, not %1%") % nr_subnodes).str());
            printf("nr_of_subnodes: %d\n", nr_subnodes);

            assert(0);
        }

        node->script.if_block[0] = (missionNode *) node->subnodes[0];
        debug(8, node->script.if_block[0], mode, "if-node");

        node->script.if_block[1] = (missionNode *) node->subnodes[1];
        debug(8, node->script.if_block[1], mode, "if-node");

        node->script.if_block[2] = (missionNode *) node->subnodes[2];
        debug(8, node->script.if_block[2], mode, "if-node");
    }
    bool ok = checkBoolExpr(node->script.if_block[0], mode);
    if (mode == SCRIPT_PARSE) {
        checkStatement(node->script.if_block[1], mode);
        checkStatement(node->script.if_block[2], mode);
    } else {
        if (ok) {
            checkStatement(node->script.if_block[1], mode);
        } else {
            checkStatement(node->script.if_block[2], mode);
        }
    }
}

void Mission::doWhile(missionNode *node, int mode) {
    if (mode == SCRIPT_PARSE) {
        int len = node->subnodes.size();
        if (len != 2) {
            fatalError(node, mode, "a while-expr needs exact two subnodes");
            assert(0);
        }
        node->script.while_arg[0] = (missionNode *) node->subnodes[0];
        node->script.while_arg[1] = (missionNode *) node->subnodes[1];

    } else {
        //runtime
        while (checkBoolExpr(node->script.while_arg[0], mode)) {
            checkStatement(node->script.while_arg[1], mode);
        }
    }
}

