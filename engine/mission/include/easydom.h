/*
 * easydom.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

/*
  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef VEGA_STRIKE_ENGINE_MISSION_EASYDOM_H
#define VEGA_STRIKE_ENGINE_MISSION_EASYDOM_H

#include <expat.h>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include "xml_support.h"
#include "vs_logging.h"

// FIXME: Shouldn't have these in a header file
using std::ostream;
using std::stack;
using std::string;
using std::vector;

using XMLSupport::AttributeList;

extern string parseCalike(char const *filename);

class easyDomNode {
public:
    easyDomNode();

    void set(easyDomNode *parent, string name, AttributeList *attributes);
    void printNode(ostream &out, int recurse_level, int level);

    void addChild(easyDomNode *child);

    string Name() {
        return name;
    }

    void set_attribute(string name, string value) {
        attribute_map[name] = value;
    };

    string attr_value(string attr_name);
    vector<easyDomNode *> subnodes;

private:
    easyDomNode *parent;
    AttributeList *attributes;

    map<string, string> attribute_map;

    //vector<string> att_name;
    //vector<string> att_value;

    string name;
};

typedef map<string, int> tagMap;

class tagDomNode : public easyDomNode {
public:
    int tag;

    void Tag(tagMap *tagmap) {
        tag = (*tagmap)[Name()];
        if (tag == 0) {
            VS_LOG(trace, (boost::format("cannot translate tag %1%") % Name()));
        }

        vector<easyDomNode *>::const_iterator siter;

        for (siter = subnodes.begin(); siter != subnodes.end(); siter++) {
            tagDomNode *tnode = (tagDomNode *) (*siter);
            tnode->Tag(tagmap);
        }
    };
};

template<class domNodeType>
class easyDomFactory {
public:
    easyDomFactory() {
    };

    void getColor(char *name, float color[4]);
    char *getVariable(char *section, char *name);

    void c_alike_to_xml(const char *filename);

    struct easyDomFactoryXML {
    } *xml;

    domNodeType *LoadXML(const char *filename) {

        const int chunk_size = 16384;

        FILE *inFile = fopen(filename, "r");
        if (!inFile) {
            //BOOST_LOG_TRIVIAL(trace) << "warning: could not open file: " << filename << endl;
            //    assert(0);
            return NULL;
        }

        xml = new easyDomFactoryXML;

        XML_Parser parser = XML_ParserCreate(NULL);
        XML_SetUserData(parser, this);
        XML_SetElementHandler(parser, &easyDomFactory::beginElement, &easyDomFactory::endElement);
        XML_SetCharacterDataHandler(parser, &easyDomFactory::charHandler);

        do {
            char *buf = (XML_Char *) XML_GetBuffer(parser, chunk_size);
            int length;

            length = fread(buf, 1, chunk_size, inFile);
            //length = inFile.gcount();
            XML_ParseBuffer(parser, length, feof(inFile));
        } while (!feof(inFile));

        fclose(inFile);
        XML_ParserFree(parser);

        return (domNodeType *) topnode;
    };

    static void charHandler(void *userData, const XML_Char *s, int len) {
        char buffer[2048];
        strncpy(buffer, s, len);
        // printf("XML-text: %s\n",buffer);
    };

    domNodeType *LoadCalike(const char *filename) {

        const int chunk_size = 16384;

        string module_str = parseCalike(filename);
        if (module_str.empty()) {
            //BOOST_LOG_TRIVIAL(trace) << "warning: could not open file: " << filename << endl;
            //    assert(0);
            return NULL;
        }

        xml = new easyDomFactoryXML;

        XML_Parser parser = XML_ParserCreate(NULL);
        XML_SetUserData(parser, this);
        XML_SetElementHandler(parser, &easyDomFactory::beginElement, &easyDomFactory::endElement);
        XML_SetCharacterDataHandler(parser, &easyDomFactory::charHandler);

        int index = 0;
        int string_size = module_str.size();
        int incr = chunk_size - 2;
        int is_final = false;

        do {
            char *buf = (XML_Char *) XML_GetBuffer(parser, chunk_size);

            int max_index = index + incr;
            int newlen = incr;

            printf("max_index=%d,string_size=%d\n", max_index, string_size);
            if (max_index >= string_size) {
                newlen = module_str.size() - index;
                printf("getting string from %d length %d\n", index, newlen);
                const string substr1 = module_str.substr(index, newlen);
                const char *strbuf = substr1.c_str();
                strncpy(buf, strbuf, newlen);
            } else {
                printf("getting string from %d length %d\n", index, incr);
                const string substr2 = module_str.substr(index, incr);
                const char *strbuf = substr2.c_str();
                strncpy(buf, strbuf, incr);
                newlen = incr;
            }

            index += newlen;

            if (index >= string_size) {
                is_final = true;
            }

            XML_ParseBuffer(parser, newlen, is_final);
        } while (!is_final);

        XML_ParserFree(parser);

        return (domNodeType *) topnode;
    };

    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
        ((easyDomFactory *) userData)->beginElement(name, AttributeList(atts));
    };

    static void endElement(void *userData, const XML_Char *name) {
        ((easyDomFactory *) userData)->endElement(name);
    };

    void beginElement(const string &name, const AttributeList &attributes) {
        AttributeList::const_iterator iter;

        domNodeType *parent;

        if (nodestack.empty()) {
            parent = NULL;
        } else {
            parent = nodestack.top();
        }

        domNodeType *thisnode = new domNodeType();
        thisnode->set(parent, name, (AttributeList *) &attributes);

        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            //BOOST_LOG_TRIVIAL(trace) <<  name << "::" << (*iter).name << endl;
        }

        if (parent == NULL) {
            topnode = thisnode;
        } else {
            parent->addChild(thisnode);
        }
        nodestack.push(thisnode);
    };

    void endElement(const string &name) {

        domNodeType *stacktop = nodestack.top();

        if (stacktop->Name() != name) {
            VS_LOG(trace, (boost::format("error: expected %1% , got %2%") % stacktop->Name() % name));
            exit(0);
        } else {
            nodestack.pop();
        }
    };

    stack<domNodeType *> nodestack;

    domNodeType *topnode;
};

#endif //VEGA_STRIKE_ENGINE_MISSION_EASYDOM_H
