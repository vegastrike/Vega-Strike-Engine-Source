/*
 * criteria.h
 *
 * Copyright (C) 2003 Mike Byron
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
 * Copyright (C) 2021-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_CRITERIA_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_CRITERIA_H

class CriteriaParent;

#include <string>
#include <vector>
#include <set>

#include "root_generic/xml_support.h"

enum CriteriaType { CONTAINS, SECTOR, OWNEDBY };

/////////////////////////////////////////////////////////////////

class CriteriaNode {
public:
    virtual bool isDestination(unsigned system) const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getText() const = 0;

    virtual CriteriaNode *unhook() = 0;
    virtual std::vector<CriteriaNode *> getChildren() const = 0;

    virtual CriteriaNode *clone() const = 0;

    CriteriaNode(CriteriaParent *parent = NULL) : m_parent(parent) {
    }

    virtual ~CriteriaNode() {
    }

    void setParent(CriteriaParent *parent) {
        m_parent = parent;
    }

    CriteriaParent *getParent() {
        return m_parent;
    }

private:
    CriteriaParent *m_parent;
};

/////////////////////////////////////////////////////////////////

class CriteriaParent : public CriteriaNode {
public:
    virtual CriteriaNode *unhook(CriteriaNode *child) = 0;

    virtual CriteriaNode *clone() const = 0;

    CriteriaParent(CriteriaParent *parent = NULL) : CriteriaNode(parent) {
    }

    virtual ~CriteriaParent() {
    }

    virtual void replaceChild(CriteriaNode *child, CriteriaNode *replacement) = 0;
};

/////////////////////////////////////////////////////////////////

class CriteriaRoot : public CriteriaParent {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *unhook();
    virtual CriteriaNode *unhook(CriteriaNode *child);
    virtual std::vector<CriteriaNode *> getChildren() const;

    CriteriaNode *getChild() const {
        return m_child;
    }

    void setChild(CriteriaNode *node);

    virtual CriteriaNode *clone() const;
    CriteriaRoot(CriteriaNode *child = NULL);
    virtual ~CriteriaRoot();

    virtual void replaceChild(CriteriaNode *child, CriteriaNode *replacement);

protected:
    CriteriaNode *m_child;
};

/////////////////////////////////////////////////////////////////

class CriteriaNot : public CriteriaParent {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *unhook();
    virtual CriteriaNode *unhook(CriteriaNode *child);
    virtual std::vector<CriteriaNode *> getChildren() const;

    virtual CriteriaNode *clone() const;
    CriteriaNot(CriteriaNode *child);
    virtual ~CriteriaNot();

    virtual void replaceChild(CriteriaNode *child, CriteriaNode *replacement);

protected:
    CriteriaNode *m_child;
};

/////////////////////////////////////////////////////////////////

class CriteriaBinaryOperator : public CriteriaParent {
public:
    virtual CriteriaNode *unhook();
    virtual CriteriaNode *unhook(CriteriaNode *child);
    virtual std::vector<CriteriaNode *> getChildren() const;

    virtual CriteriaNode *clone() const = 0;
    CriteriaBinaryOperator(CriteriaNode *child, CriteriaNode *newNode);
    virtual ~CriteriaBinaryOperator();

    virtual void replaceChild(CriteriaNode *child, CriteriaNode *replacement);

protected:
    CriteriaNode *m_left;
    CriteriaNode *m_right;
};

/////////////////////////////////////////////////////////////////

class CriteriaAnd : public CriteriaBinaryOperator {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *clone() const;

    CriteriaAnd(CriteriaNode *child, CriteriaNode *newNode) :
            CriteriaBinaryOperator(child, newNode) {
    }

    virtual ~CriteriaAnd() {
    }
};

/////////////////////////////////////////////////////////////////

class CriteriaOr : public CriteriaBinaryOperator {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *clone() const;

    CriteriaOr(CriteriaNode *child, CriteriaNode *newNode) :
            CriteriaBinaryOperator(child, newNode) {
    }

    virtual ~CriteriaOr() {
    }
};

/////////////////////////////////////////////////////////////////

class CriteriaLeaf : public CriteriaNode {
public:
    virtual CriteriaNode *unhook();
    virtual std::vector<CriteriaNode *> getChildren() const;

    virtual CriteriaNode *clone() const = 0;

    CriteriaLeaf(CriteriaParent *parent, std::string value) :
            CriteriaNode(parent), m_value(value) {
    }

    virtual ~CriteriaLeaf() {
    }

protected:
    std::string m_value;
};

/////////////////////////////////////////////////////////////////

class CriteriaContains : public CriteriaLeaf {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *clone() const;

    CriteriaContains(std::string value, CriteriaParent *parent = NULL) :
            CriteriaLeaf(parent, value) {
    }

    virtual ~CriteriaContains() {
    }

private:
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);
    std::set<std::string> getPlanetTypesFromXML(const char *filename) const;
};

/////////////////////////////////////////////////////////////////

class CriteriaOwnedBy : public CriteriaLeaf {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *clone() const;

    CriteriaOwnedBy(std::string value, CriteriaParent *parent = NULL) :
            CriteriaLeaf(parent, value) {
    }

    virtual ~CriteriaOwnedBy() {
    }
};

/////////////////////////////////////////////////////////////////

class CriteriaSector : public CriteriaLeaf {
public:
    virtual bool isDestination(unsigned system) const;
    virtual std::string getDescription() const;
    virtual std::string getText() const;

    virtual CriteriaNode *clone() const;

    CriteriaSector(std::string value, CriteriaParent *parent = NULL) :
            CriteriaLeaf(parent, value) {
    }

    virtual ~CriteriaSector() {
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_CRITERIA_H
