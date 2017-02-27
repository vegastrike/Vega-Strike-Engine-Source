/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
 *  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <expat.h>
#include "easydom.h"

#include <assert.h>     /// needed for assert() calls.

using std::cout;
using std::cerr;
using std::endl;
easyDomNode::easyDomNode() {}

void easyDomNode::set( easyDomNode *_parent, string _name, const XML_Char **atts )
{
    parent = _parent;
    if (atts != NULL) {
        for (; *atts != NULL; atts += 2) {
#if 0
            att_name.push_back( (*iter).name );
            att_value.push_back( (*iter).value );
#endif
            attribute_map[atts[0]] = atts[1];
        }
    }
    name = _name;
}

void easyDomNode::addChild( easyDomNode *child )
{
    subnodes.push_back( child );
}

string easyDomNode::attr_value( string search_name )
{
    return attribute_map[search_name];
}

void easyDomNode::printNode( ostream &out, int recurse_level, int level )
{
    vsUMap< string, string >::const_iterator iter;

    out<<"<"<<name;
    for (iter = attribute_map.begin(); iter != attribute_map.end(); iter++)
        out<<" "<<(*iter).first<<"=\""<<(*iter).second<<"\"";
    out<<">"<<endl;

    vector< easyDomNode* >::const_iterator siter;
    if (recurse_level > 0)
        for (siter = subnodes.begin(); siter != subnodes.end(); siter++)
            (*siter)->printNode( out, recurse_level-1, level+1 );
    if ( !(recurse_level == 0 && level == 0) )
        out<<"</"<<name<<">"<<endl;
}

const char *textAttr = "Text_Attr";

