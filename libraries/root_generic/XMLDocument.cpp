/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "XMLDocument.h"

#include <cassert>
#include <fstream>
#include <expat.h>
#include <algorithm>

#define PARSING_BUFFER_SIZE 4096

namespace XMLDOM {
/******************************************************************
*                                                                *
*                                                                *
*                       XMLElement implementation                *
*                                                                *
*                                                                *
******************************************************************/

XMLElement::XMLElement() :
        mType(XET_CDATA),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
}

XMLElement::XMLElement(const std::string &cdata) :
        mType(XET_CDATA),
        mContents(cdata),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
}

XMLElement::XMLElement(Type type, const std::string &data) :
        mType(type), mContents((type == XET_CDATA || type == XET_COMMENT) ? data : std::string()),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
}

XMLElement::XMLElement(const char *tagName, const char *const *attrValuePairList, unsigned int nAttr) :
        mType(XET_TAG),
        mTagName(tagName),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
    for (; (nAttr >= 2 && attrValuePairList[0] && attrValuePairList[1]); attrValuePairList += 2) {
        setAttribute(attrValuePairList[0], attrValuePairList[1]);
    }
}

XMLElement::XMLElement(const char *tagName, const char *const *attrValuePairList) :
        mType(XET_TAG),
        mTagName(tagName),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
    for (; (attrValuePairList[0] && attrValuePairList[1]); attrValuePairList += 2) {
        setAttribute(attrValuePairList[0], attrValuePairList[1]);
    }
}

XMLElement::XMLElement(const std::string &tagName, const std::vector<std::string> &attrValuePairList) :
        mType(XET_TAG),
        mTagName(tagName),
        mAttributes(),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.end()),
        mNameAttribute(mAttributes.end()) {
    for (std::vector<std::string>::size_type i = 0; i + 1 < attrValuePairList.size(); i += 2) {
        setAttribute(attrValuePairList[i], attrValuePairList[i + 1]);
    }
}

XMLElement::XMLElement(const std::string &tagName, const std::map<std::string, std::string> &attrValuePairList) :
        mType(XET_TAG),
        mTagName(tagName),
        mAttributes(attrValuePairList),
        mParent(nullptr),
        mDocument(nullptr),
        mIdAttribute(mAttributes.find("id")),
        mNameAttribute(mAttributes.find("name")) {
}

XMLElement::~XMLElement() {
    clear();
    mParent = nullptr;
    mDocument = nullptr;
}

void XMLElement::clear(bool doAttributes) {
    //Remove all - quickly
    for (child_iterator cit = childrenBegin(); cit != childrenEnd(); ++cit) {
        delete *cit;
    }
    mChildren.clear();
    mById.clear();
    mByName.clear();
    if (doAttributes) {
        mAttributes.clear();
    }
    mIdAttribute = mNameAttribute = mAttributes.end();
}

const std::string &XMLElement::contents() const {
    static std::string empty;
    switch (type()) {
        case XET_CDATA
            :
        case XET_COMMENT:
            return mContents;

        case XET_TAG:
        case XET_ROOT:
            if (numChildren() > 0 && getChild(0)->type() == XET_CDATA) {
                return getChild(0)->contents();
            } else {
                return empty;
            }
        default:
            return empty;
    }
}

std::string &XMLElement::contents() {
    assert(type() == XET_CDATA || type() == XET_COMMENT);
    //Dirtify document
    if (mDocument) {
        mDocument->dirty = true;
    }
    return mContents;
}

const XMLElement *XMLElement::getChildById(const std::string &id) const {
    ElementMap::const_iterator cit = mById.find(id);
    if (cit == mById.end()) {
        return nullptr;
    } else {
        return cit->second;
    }
}

XMLElement *XMLElement::getChildById(const std::string &id) {
    ElementMap::iterator cit = mById.find(id);
    if (cit == mById.end()) {
        return nullptr;
    } else {
        return cit->second;
    }
}

const XMLElement *XMLElement::getChildByName(const std::string &name) const {
    ElementMap::const_iterator cit = mByName.find(name);
    if (cit == mByName.end()) {
        return nullptr;
    } else {
        return cit->second;
    }
}

XMLElement *XMLElement::getChildByName(const std::string &name) {
    ElementMap::iterator cit = mByName.find(name);
    if (cit == mByName.end()) {
        return nullptr;
    } else {
        return cit->second;
    }
}

unsigned int XMLElement::appendChild(XMLElement *newElem) {
    //Dirtify document
    if (mDocument) {
        mDocument->dirty = true;
    }
    mChildren.push_back(newElem);
    newElem->setParent(this);
    newElem->setDocument(mDocument);
    return mChildren.size() - 1;
}

void XMLElement::removeChild(unsigned int idx) {
    if (idx < mChildren.size() && mChildren[idx]) {
        //Dirtify document
        if (mDocument) {
            mDocument->dirty = true;
        }
        delete mChildren[idx];
        mChildren.erase(mChildren.begin() + idx);
    }
}

void XMLElement::removeChild(const XMLElement *which) {
    removeChild(std::find(mChildren.begin(), mChildren.end(), which) - mChildren.begin());
}

void XMLElement::removeAttribute(const std::string &name) {
    mAttributes.erase(name);
}

void XMLElement::setAttribute(const std::string &name, const std::string &value) {
    static std::string _id("id");
    static std::string _name("name");
    //Dirtify document
    if (mDocument) {
        mDocument->dirty = true;
    }
    std::pair<attribute_iterator, bool> rv =
            mAttributes.insert(std::pair<std::string, std::string>(name, value));
    if (rv.second) {
        if (name == _id) {
            mIdAttribute = rv.first;
        } else if (name == _name) {
            mNameAttribute = rv.first;
        }
    }
}

void XMLElement::appendContents(const std::string &cont) {
    switch (type()) {
        case XET_CDATA:
        case XET_COMMENT:
            //Dirtify document
            if (mDocument) {
                mDocument->dirty = true;
            }
            mContents += cont;

            break;
        case XET_TAG:
        case XET_ROOT:
            if (numChildren() == 0 || getChild(numChildren() - 1)->type() != XET_CDATA) {
                appendChild(new XMLElement(cont));
            } else {
                getChild(numChildren() - 1)->appendContents(cont);
            }
            break;
    }
}

void XMLElement::setParent(XMLElement *parent) {
    mParent = parent;
}

void XMLElement::setDocument(XMLDocument *document) {
    mDocument = document;
    for (child_iterator cit = childrenBegin(); cit != childrenEnd(); ++cit) {
        (*cit)->setDocument(document);
    }
}

void XMLElement::setType(Type newType) {
    if (newType != type()) {
        if (mParent) {
            //Harder
            //Remove children by hand - to allow them to deregister themselves.
            while (numChildren()) {
                removeChild(numChildren() - 1);
            }
            removeAttribute("id");
            removeAttribute("name");
            mAttributes.clear();
        } else {
            //Faster
            clear();
        }
        mType = newType;
    }
}

void XMLElement::JoinMaps(ElementMap &dst, const ElementMap &src) {
    ElementMap::iterator dit = dst.begin();
    ElementMap::const_iterator sit = src.begin();
    while (sit != src.end()) {
        dit = dst.insert(dit, *sit);
        ++sit;
    }
}

void XMLElement::rebuildNamedBindings(bool deepScan) {
    mById.clear();
    mByName.clear();
    for (child_iterator it = childrenBegin(); it != childrenEnd(); ++it) {
        (*it)->rebuildNamedBindings(deepScan);
        if (deepScan) {
            JoinMaps(mById, (*it)->mById);
            JoinMaps(mByName, (*it)->mByName);
        }
        const_attribute_iterator iit = (*it)->mIdAttribute;
        if (iit != (*it)->attributesEnd()) {
            mById.insert(std::pair<std::string, XMLElement *>(iit->second, *it));
        }
        const_attribute_iterator nit = (*it)->mNameAttribute;
        if (nit != (*it)->attributesEnd()) {
            mByName.insert(std::pair<std::string, XMLElement *>(nit->second, *it));
        }
    }
}

const XMLElement *XMLElement::getChildByHierarchicalId(const std::string &id) const {
    std::string::size_type sep = id.find('/');
    if (sep != std::string::npos) {
        const XMLElement *sub = getChildById(id.substr(0, sep));
        return sub ? sub->getChildByHierarchicalId(id.substr(sep + 1)) : nullptr;
    } else {
        return getChildById(id);
    }
}

XMLElement *XMLElement::getChildByHierarchicalId(const std::string &id) {
    std::string::size_type sep = id.find('/');
    if (sep != std::string::npos) {
        XMLElement *sub = getChildById(id.substr(0, sep));
        return sub ? sub->getChildByHierarchicalId(id.substr(sep + 1)) : nullptr;
    } else {
        return getChildById(id);
    }
}

const XMLElement *XMLElement::getChildByHierarchicalName(const std::string &name) const {
    std::string::size_type sep = name.find('/');
    if (sep != std::string::npos) {
        const XMLElement *sub = getChildByName(name.substr(0, sep));
        return sub ? sub->getChildByHierarchicalName(name.substr(sep + 1)) : nullptr;
    } else {
        return getChildByName(name);
    }
}

XMLElement *XMLElement::getChildByHierarchicalName(const std::string &name) {
    std::string::size_type sep = name.find('/');
    if (sep != std::string::npos) {
        XMLElement *sub = getChildByName(name.substr(0, sep));
        return sub ? sub->getChildByHierarchicalName(name.substr(sep + 1)) : nullptr;
    } else {
        return getChildByName(name);
    }
}

/******************************************************************
*                                                                *
*                                                                *
*                       XMLParser  implementation                *
*                                                                *
*                                                                *
******************************************************************/

struct XMLParserContext {
    typedef std::map<std::string, XMLProcessor *> ProcessorMap;

    ProcessorMap processors;
    XMLSerializer *xparser{};
    XMLDocument *document{};
    XMLElement *current{};

    XML_Parser parser{};
};

namespace ExpatHandlers {
/******************************************************************
*                                                                *
*                     XMLSerializer  implementation              *
*                                                                *
*                           (ExpatHandlers)                      *
*                                                                *
******************************************************************/


/* atts is array of name/value pairs, terminated by 0;
 *   names and values are 0 terminated. */
static void StartElement(void *userData, const XML_Char *name, const XML_Char **atts) {
    XMLParserContext *internals = (XMLParserContext *) userData;
    assert(internals);
    assert(internals->current);
    internals->current =
            internals->current->getChild(
                    internals->current->appendChild(
                            new XMLElement(
                                    (const char *) name,
                                    (const char *const *) atts
                            )
                    )
            );
}

static void EndElement(void *userData, const XML_Char *name) {
    XMLParserContext *internals = (XMLParserContext *) userData;
    assert(internals);
    assert(internals->current);
    internals->current = internals->current->getParent();
}

/* s is not 0 terminated. */
static void CData(void *userData, const XML_Char *s, int len) {
    XMLParserContext *internals = (XMLParserContext *) userData;
    assert(internals);
    assert(internals->current);
    assert(internals->xparser);
    if (!(internals->xparser->options & XMLSerializer::OPT_WANT_CDATA)) {
        return;
    }
    internals->current->appendContents(std::string(s, len));
}

/* target and data are 0 terminated */
static void PI(void *userData, const XML_Char *target, const XML_Char *data) {
    XMLParserContext *internals = (XMLParserContext *) userData;
    assert(internals);
    XMLParserContext::ProcessorMap::const_iterator it = internals->processors.find(std::string(target));
    if (it != internals->processors.end()) {
        it->second->execute(
                internals->xparser,
                internals->document,
                internals->current,
                std::string(data));
    }
}

static void Comment(void *userData, const XML_Char *data) {
    XMLParserContext *internals = (XMLParserContext *) userData;
    assert(internals);
    assert(internals->current);
    assert(internals->xparser);
    if (!(internals->xparser->options & XMLSerializer::OPT_WANT_COMMENTS)) {
        return;
    }
    internals->current->appendChild(new XMLElement(XMLElement::XET_COMMENT, std::string(data)));
}
};

/******************************************************************
*                                                                *
*                     XMLSerializer  implementation              *
*                                                                *
*                           (member functions)                   *
*                                                                *
******************************************************************/

XMLSerializer::XMLSerializer(const char *encoding, XMLDocument *doc, XMLElement *elem) :
        options(OPT_DEFAULT), internals(nullptr) {
    initialise(encoding, doc, elem);
}

XMLSerializer::~XMLSerializer() {
    delete close();
}

bool XMLSerializer::parse(const void *buf, unsigned int len) {
    XMLParserContext *ctxt = (XMLParserContext *) internals;
    return XML_Parse(ctxt->parser, (const XML_Char *) buf, len, false) != 0;
}

bool XMLSerializer::importXML(const std::string &path) {
    std::ifstream fi(path.c_str());
    if (fi.fail()) {
        return false;
    } else {
        bool bok = true;
        while (bok && !fi.fail() && !fi.eof()) {
            char buffer[PARSING_BUFFER_SIZE];
            fi.read(buffer, sizeof(buffer));
            bok = parse(buffer, fi.gcount());
        }
        fi.close();
        return bok;
    }
}

bool _exportXML(std::ostream &stream, XMLElement *elem) {
    if (!elem) {
        return false;
    }
    XMLElement::attribute_iterator ait;
    unsigned int i;
    //Prolog
    switch (elem->type()) {
        case XMLElement::XET_TAG:
            stream << "<" << elem->tagName().c_str();
            for (ait = elem->attributesBegin(); ait != elem->attributesEnd(); ++ait) {
                if (ait->second.find("\"") != std::string::npos) {
                    stream << " " << ait->first.c_str() << "=\'" << ait->second.c_str() << "\'";
                } else {
                    stream << " " << ait->first.c_str() << "=\"" << ait->second.c_str() << "\"";
                }
            }
            if (elem->numChildren() > 0) {
                stream << ">";
            } else {
                stream << "/>";
            }
            break;
        case XMLElement::XET_CDATA:
            //To-do: Find out if it needs to be enveloped within '<![CDATA['/']]>' constructs.
            stream << elem->contents().c_str();
            return !stream.fail();         //No children

        case XMLElement::XET_COMMENT:
            stream << "<!--" << elem->contents().c_str() << "-->";
            return !stream.fail();         //No children

        case XMLElement::XET_ROOT:
            //WTF?
            return true;
    }
    if (stream.fail()) {
        return false;
    }
    //Children
    for (i = 0; i < elem->numChildren(); ++i) {
        if (!_exportXML(stream, elem->getChild(i))) {
            return false;
        }
    }
    if (stream.fail()) {
        return false;
    }
    //Epilog
    switch (elem->type()) {
        case XMLElement::XET_TAG:
            if (elem->numChildren() > 0) {
                stream << "</" << elem->tagName().c_str() << ">";
            }
            break;
        case XMLElement::XET_COMMENT:
        case XMLElement::XET_CDATA:
        case XMLElement::XET_ROOT:
            //WTF?
            return true;
    }
    if (stream.fail()) {
        return false;
    }
    return true;
}

bool _exportXML(std::ostream &stream, XMLDocument *doc) {
    if (!doc) {
        return false;
    }
    //Output xml && <!DOCTYPE...> tags
    if (doc->sysId.empty()) {
        stream << "<?xml version=\"1.0\"?>\n";
    } else {
        stream << "<?xml version=\"1.1\"?>\n";
    }
    if (!doc->sysId.empty()) {
        stream << "<!DOCTYPE " << doc->docType.c_str();
        if (!doc->pubId.empty()) {
            stream << " PUBLIC \"" << doc->pubId.c_str() << "\"";
        } else {
            stream << " SYSTEM";
        }
        if (doc->sysId.find('\"') != std::string::npos) {
            stream << " \'" << doc->sysId.c_str() << "\'";
        } else {
            stream << " \"" << doc->sysId.c_str() << "\"";
        }
        stream << ">\n";
    }
    //Did we fail?
    if (stream.fail()) {
        return false;
    }
    //Output the rest of the document
    //NOTE: Although XML 1.1 requires that 'root' may only have one child,
    //we can't make that assumption because we count as elements comments
    //and other productions referred to as 'misc' by the standard.
    for (unsigned int i = 0; i < doc->root.numChildren(); ++i) {
        if (!_exportXML(stream, doc->root.getChild(i))) {
            return false;
        }
    }
    return true;
}

bool XMLSerializer::exportXML(std::ostream &stream) {
    return !stream.fail()
            && _exportXML(stream, ((XMLParserContext *) internals)->document);
}

bool XMLSerializer::exportXML(const std::string &filename) {
    std::ofstream of(filename.c_str());
    return exportXML(of);
}

void XMLSerializer::addProcessor(XMLProcessor *proc) {
    assert(internals);

    XMLParserContext *ctxt = (XMLParserContext *) internals;
    ctxt->processors.insert(std::pair<std::string, XMLProcessor *>(proc->getTarget(), proc));
}

bool XMLSerializer::initialise(const char *encoding, XMLDocument *doc, XMLElement *elem) {
    //DO NOT assert encoding==0... in case an encoding natively supported by expat is passed.
    //DO assert, though, that doc==0 => elem==0
    assert(!(!doc && elem));

    delete close();
    XMLParserContext *ctxt = new XMLParserContext;
    ctxt->xparser = this;
    ctxt->document = doc ? doc : new XMLDocument();
    ctxt->current = elem ? elem : &(ctxt->document->root);

    //Create Expat parser, and initialize
    ctxt->parser = XML_ParserCreate((const XML_Char *) encoding);
    XML_SetUserData(ctxt->parser, ctxt);
    XML_SetElementHandler(ctxt->parser, &ExpatHandlers::StartElement, &ExpatHandlers::EndElement);
    XML_SetCharacterDataHandler(ctxt->parser, &ExpatHandlers::CData);
    XML_SetProcessingInstructionHandler(ctxt->parser, &ExpatHandlers::PI);
    XML_SetCommentHandler(ctxt->parser, ExpatHandlers::Comment);

    internals = ctxt;
    return true;
}

XMLDocument *XMLSerializer::close() {
    XMLDocument *doc = nullptr;
    XMLParserContext *ctxt = (XMLParserContext *) internals;
    if (ctxt) {
        XML_ParserFree(ctxt->parser);
        doc = ctxt->document;
        if (options & OPT_WANT_NAMEBINDINGS) {
            doc->root.rebuildNamedBindings((options & OPT_WANT_NAMEBINDINGS_DEEPSCAN) != 0);
        }
        delete ctxt;
    }
    internals = nullptr;

    return doc;
}
} //namespace XMLDOM

