/*
 * criteria.cpp
 *
 * Copyright (C) 2003 Mike Byron
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
 * Copyright (C) 2021 Stephen G. Tuggy
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
#if defined (_WIN32) && !defined (__CYGWIN__) && !defined (__MINGW32__)
//For WIN32 debugging.
#include <crtdbg.h>
#endif

#include <assert.h>

#include "criteria.h"
#include "vs_globals.h"
#include "gfx/cockpit.h"
#include "galaxy_xml.h"
#include "savegame.h"
#include "universe_util.h"
#include "universe.h"

using std::string;
using std::set;

/////////////////////////////////////////////////////////////////

bool CriteriaRoot::isDestination( unsigned system ) const
{
    if (m_child)
        return m_child->isDestination( system );
    else
        return false;
}

string CriteriaRoot::getDescription() const
{
    if (m_child)
        return m_child->getDescription();
    else
        return "";
}

string CriteriaRoot::getText() const
{
    return "";
}

void CriteriaRoot::replaceChild( CriteriaNode *child, CriteriaNode *replacement )
{
    assert( child == m_child );
    m_child = replacement;
}

CriteriaNode* CriteriaRoot::unhook()
{
    return NULL;
}

CriteriaNode* CriteriaRoot::unhook( CriteriaNode *child )
{
    assert( child == m_child );
    m_child = NULL;
    return child;
}

vector< CriteriaNode* >CriteriaRoot::getChildren() const
{
    vector< CriteriaNode* >temp;
    if (m_child)
        temp.push_back( m_child );
    return temp;
}

CriteriaNode* CriteriaRoot::clone() const
{
    CriteriaNode *cloned_child = NULL;
    if (m_child)
        cloned_child = m_child->clone();
    CriteriaRoot *temp = new CriteriaRoot( cloned_child );
    return temp;
}

void CriteriaRoot::setChild( CriteriaNode *node )
{
    m_child = node;
    if (m_child)
        m_child->setParent( this );
}

CriteriaRoot::CriteriaRoot( CriteriaNode *child ) :
    CriteriaParent( NULL )
    , m_child( child )
{
    if (m_child)
        m_child->setParent( this );
}

CriteriaRoot::~CriteriaRoot()
{
    if (m_child != nullptr) {
        delete m_child;
        m_child = nullptr;
    }
}

/////////////////////////////////////////////////////////////////

bool CriteriaNot::isDestination( unsigned system ) const
{
    assert( m_child != NULL );

    return !( m_child->isDestination( system ) );
}

string CriteriaNot::getDescription() const
{
    assert( m_child != NULL );

    string temp = "NOT(";
    temp += m_child->getDescription();
    temp += ")";
    return temp;
}

string CriteriaNot::getText() const
{
    return "NOT";
}

void CriteriaNot::replaceChild( CriteriaNode *child, CriteriaNode *replacement )
{
    assert( child == m_child );
    m_child = replacement;
}

CriteriaNode* CriteriaNot::unhook()
{
    m_child->setParent( getParent() );
    getParent()->replaceChild( this, m_child );
    m_child = NULL;
    return this;
}

CriteriaNode* CriteriaNot::unhook( CriteriaNode *child )
{
    return getParent()->unhook( this );
}

vector< CriteriaNode* >CriteriaNot::getChildren() const
{
    vector< CriteriaNode* >temp;
    if (m_child)
        temp.push_back( m_child );
    return temp;
}

CriteriaNode* CriteriaNot::clone() const
{
    assert( m_child );
    CriteriaNode *cloned_child = m_child->clone();
    CriteriaNot  *temp = new CriteriaNot( cloned_child );
    return temp;
}

CriteriaNot::CriteriaNot( CriteriaNode *child ) :
    CriteriaParent()
{
    assert( child != NULL );
    setParent( child->getParent() );
    m_child = child;
    if ( child->getParent() )
        child->getParent()->replaceChild( child, this );
    child->setParent( this );
}

CriteriaNot::~CriteriaNot()
{
    if (m_child != nullptr) {
        delete m_child;
        m_child = nullptr;
    }
}

/////////////////////////////////////////////////////////////////

CriteriaBinaryOperator::CriteriaBinaryOperator( CriteriaNode *child, CriteriaNode *newNode ) :
    CriteriaParent( child->getParent() )
{
    assert( child != NULL );
    assert( newNode != NULL );
    m_left  = child;
    m_right = newNode;
    if ( child->getParent() )
        child->getParent()->replaceChild( child, this );
    child->setParent( this );

    newNode->setParent( this );
}

CriteriaNode* CriteriaBinaryOperator::unhook()
{
    return getParent()->unhook( this );
}

CriteriaNode* CriteriaBinaryOperator::unhook( CriteriaNode *child )
{
    assert( (m_left == child) || (m_right == child) );
    if (child == m_left) {
        m_right->setParent( getParent() );
        getParent()->replaceChild( this, m_right );
        m_right = NULL;
    } else {
        m_left->setParent( getParent() );
        getParent()->replaceChild( this, m_left );
        m_left = NULL;
    }
    return this;
}

vector< CriteriaNode* >CriteriaBinaryOperator::getChildren() const
{
    vector< CriteriaNode* >temp;
    if (m_left)
        temp.push_back( m_left );
    if (m_right)
        temp.push_back( m_right );
    return temp;
}

CriteriaBinaryOperator::~CriteriaBinaryOperator()
{
    if (m_left != nullptr) {
        delete m_left;
        m_left = nullptr;
    }
    if (m_right != nullptr) {
        delete m_right;
        m_right = nullptr;
    }
}

void CriteriaBinaryOperator::replaceChild( CriteriaNode *child, CriteriaNode *replacement )
{
    assert( (m_left == child) || (m_right == child) );
    if (child == m_left)
        m_left = replacement;
    else
        m_right = replacement;
}

/////////////////////////////////////////////////////////////////

bool CriteriaAnd::isDestination( unsigned system ) const
{
    assert( m_left != NULL );
    assert( m_right != NULL );

    return m_left->isDestination( system ) && m_right->isDestination( system );
}

string CriteriaAnd::getDescription() const
{
    assert( m_left != NULL );
    assert( m_right != NULL );

    string temp = "(";
    temp += m_left->getDescription();
    temp += " AND ";
    temp += m_right->getDescription();
    temp += ")";
    return temp;
}

string CriteriaAnd::getText() const
{
    return "AND";
}

CriteriaNode* CriteriaAnd::clone() const
{
    assert( m_left );
    assert( m_right );

    CriteriaNode *cloned_left  = m_left->clone();
    CriteriaNode *cloned_right = m_right->clone();

    CriteriaAnd  *temp = new CriteriaAnd( cloned_left, cloned_right );
    return temp;
}

/////////////////////////////////////////////////////////////////

bool CriteriaOr::isDestination( unsigned system ) const
{
    assert( m_left != NULL );
    assert( m_right != NULL );

    return m_left->isDestination( system ) || m_right->isDestination( system );
}

string CriteriaOr::getDescription() const
{
    assert( m_left != NULL );
    assert( m_right != NULL );

    string temp = "(";
    temp += m_left->getDescription();
    temp += " OR ";
    temp += m_right->getDescription();
    temp += ")";
    return temp;
}

string CriteriaOr::getText() const
{
    return "OR";
}

CriteriaNode* CriteriaOr::clone() const
{
    assert( m_left );
    assert( m_right );

    CriteriaNode *cloned_left  = m_left->clone();
    CriteriaNode *cloned_right = m_right->clone();

    CriteriaOr   *temp = new CriteriaOr( cloned_left, cloned_right );
    return temp;
}

/////////////////////////////////////////////////////////////////

CriteriaNode* CriteriaLeaf::unhook()
{
    return getParent()->unhook( this );
}

vector< CriteriaNode* >CriteriaLeaf::getChildren() const
{
    vector< CriteriaNode* >temp;
    return temp;
}

/////////////////////////////////////////////////////////////////

bool CriteriaContains::isDestination( unsigned system ) const
{
    string name = _Universe->AccessCockpit()->AccessNavSystem()->systemIter[system].GetName();

    //Check to make sure we have been there in person
    //Not only for realism
    //but the systems may have not been created yet.
    string key( string( "visited_" )+name );
    vector< float > *v = &_Universe->AccessCockpit()->savegame->getMissionData( key );
    if ( v->empty() )
        return false;
    if ( (*v)[0] != 1.0 )
        return false;
    string texture     = _Universe->getGalaxy()->getPlanetVariable( m_value, "texture", "" );
    if (texture == "")
        return false;
    set< string > types = getPlanetTypesFromXML( name.c_str() );
    for (set< string >::iterator i = types.begin(); i != types.end(); ++i)
        if ( (*i).find( texture, 0 ) != string::npos )
            return true;
    return false;
}

string CriteriaContains::getDescription() const
{
    string temp = "CONTAINS(";
    temp += m_value;
    temp += ")";
    return temp;
}

string CriteriaContains::getText() const
{
    return getDescription();
}

CriteriaNode* CriteriaContains::clone() const
{
    return new CriteriaContains( m_value, NULL );
}

/////////////////////////////////////////////////////////////////

bool CriteriaOwnedBy::isDestination( unsigned system ) const
{
    string name    = _Universe->AccessCockpit()->AccessNavSystem()->systemIter[system].GetName();
    string faction = UniverseUtil::GetGalaxyFaction( name );
    if (faction == m_value)
        return true;
    else
        return false;
}

string CriteriaOwnedBy::getDescription() const
{
    string temp = "OWNEDBY(";
    temp += m_value;
    temp += ")";
    return temp;
}

string CriteriaOwnedBy::getText() const
{
    return getDescription();
}

CriteriaNode* CriteriaOwnedBy::clone() const
{
    return new CriteriaOwnedBy( m_value, NULL );
}

/////////////////////////////////////////////////////////////////

bool CriteriaSector::isDestination( unsigned system ) const
{
    string name = _Universe->AccessCockpit()->AccessNavSystem()->systemIter[system].GetName();
    string sector, systemname;
    Beautify( name, sector, systemname );
    if (sector == m_value)
        return true;
    else
        return false;
}

string CriteriaSector::getDescription() const
{
    string temp = "SECTOR(";
    temp += m_value;
    temp += ")";
    return temp;
}

string CriteriaSector::getText() const
{
    return getDescription();
}

CriteriaNode* CriteriaSector::clone() const
{
    return new CriteriaSector( m_value, NULL );
}

