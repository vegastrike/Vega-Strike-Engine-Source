/**
 * easydom.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) Alexander Rawass
 * Copyright (C) 2020 Stephen G. Tuggy, pyramid3d, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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
 *  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#ifndef _EASYDOM_H_
#define _EASYDOM_H_
#include <expat.h>
#include <string>
#include <vector>
#include <stack>
#include <gnuhash.h>
#include <stdlib.h>
#include "vsfilesystem.h"
#include "vs_globals.h"
//using namespace VSFileSystem;
// using VSFileSystem::VSFile;
// using VSFileSystem::VSError;
// using VSFileSystem::Ok;
// using VSFileSystem::FileNotFound;
// using VSFileSystem::MissionFile;
// using VSFileSystem::UnitFile;
// using VSFileSystem::UnknownFile;
// using VSFileSystem::AiFile;
#include "xml_support.h"
#include "vs_logging.h"

// using std::string;
// using std::vector;
// using std::stack;
// using std::ostream;

// using XMLSupport::AttributeList;

extern std::string parseCalike( char const *filename );

class easyDomNode
{
public: easyDomNode();

    void set( easyDomNode *parent, std::string name, const XML_Char **atts );
    void printNode( std::ostream &out, int recurse_level, int level );

    void addChild( easyDomNode *child );

    std::string Name()
    {
        return name;
    }

    void set_attribute( std::string name, std::string value )
    {
        attribute_map[name] = value;
    }

    std::string attr_value( std::string attr_name );
    std::vector< easyDomNode* >subnodes;

private:
    easyDomNode   *parent;
    XMLSupport::AttributeList *attributes;
    vsUMap< std::string, std::string >attribute_map;
//vector<string> att_name;
//vector<string> att_value;

    std::string name;
};

typedef vsUMap< std::string, int >tagMap;

class tagDomNode : public easyDomNode
{
public:
    int tag;

    void Tag( tagMap *tagmap )
    {
        tag = (*tagmap)[Name()];
        std::vector< easyDomNode* >::const_iterator siter;
        for (siter = subnodes.begin(); siter != subnodes.end(); siter++) {
            tagDomNode *tnode = (tagDomNode*) (*siter);
            tnode->Tag( tagmap );
        }
    }
};

extern const char *textAttr;   //should be a static const inside easyDomFactory...

template < class domNodeType >
class easyDomFactory
{
public: easyDomFactory() {}

    void getColor( char *name, float color[4] );
    char * getVariable( char *section, char *name );

    void c_alike_to_xml( const char *filename );

    struct easyDomFactoryXML
    {
        int   currentindex;
        char *buffer;
        easyDomFactoryXML()
        {
            buffer = 0;
            currentindex = 0;
        }
    }
    *xml;

    domNodeType * LoadXML( const char *filename )
    {
        topnode = nullptr;
        //Not really nice but should do its job
        unsigned int length = strlen( filename );
        VSFileSystem::VSFile  f;
        VSFileSystem::VSError err = VSFileSystem::VSError::FileNotFound;
        if ( length > 8 && !memcmp( (filename+length-7), "mission", 7 ) )
            err = f.OpenReadOnly( filename, VSFileSystem::MissionFile );
        if (err > VSFileSystem::VSError::Ok) {
            err = f.OpenReadOnly( filename, VSFileSystem::UnknownFile );
            if (err > VSFileSystem::VSError::Ok) {
                std::string rootthis = std::string( "/" )+filename;
                err = f.OpenReadOnly( rootthis, VSFileSystem::UnknownFile );
            }
        }
        if (err > VSFileSystem::VSError::Ok) {
            string prefix = ("../mission/");
            prefix += filename;
            err     = f.OpenReadOnly( prefix.c_str(), VSFileSystem::UnknownFile );
        }
        if (err > VSFileSystem::VSError::Ok) {
            string prefix = ("mission/");
            prefix += filename;
            err     = f.OpenReadOnly( prefix.c_str(), VSFileSystem::UnknownFile );
        }
        if (err > VSFileSystem::VSError::Ok) {
            string prefix = ("../");
            prefix += filename;
            err     = f.OpenReadOnly( prefix.c_str(), VSFileSystem::UnknownFile );
        }
        if (err > VSFileSystem::VSError::Ok) {
            return nullptr;
        }
        xml = new easyDomFactoryXML;

        XML_Parser parser = XML_ParserCreate( nullptr );
        XML_SetUserData( parser, this );
        XML_SetElementHandler( parser, &easyDomFactory::beginElement, &easyDomFactory::endElement );
        XML_SetCharacterDataHandler( parser, &easyDomFactory::charHandler );

        XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
        /*
         *  do {
         * #ifdef BIDBG
         *  char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
         * #else
         *  char buf[chunk_size];
         * #endif
         *  int length;
         *
         *  length = fread (buf,1, chunk_size,inFile);
         *  //length = inFile.gcount();
         * #ifdef BIDBG
         *  XML_ParseBuffer(parser, length, feof(inFile));
         * #else
         *  XML_Parse(parser, buf, length, feof(inFile));
         * #endif
         *  } while(!feof(inFile));
         */
        f.Close();
        XML_ParserFree( parser );
        delete xml;
        return (domNodeType*) topnode;
    }

    static void charHandler( void *userData, const XML_Char *s, int len )
    {
        easyDomFactoryXML *xml = ( (easyDomFactory< domNodeType >*)userData )->xml;
        if (!xml->buffer) {
            xml->buffer = (char*) malloc( sizeof (char)*(len+1) );
        } else {
            xml->buffer = (char*) realloc( xml->buffer, sizeof (char)*(len+1+xml->currentindex) );
        }
        strncpy( xml->buffer+xml->currentindex, s, len );
        xml->currentindex += len;
    }

    domNodeType * LoadCalike( const char *filename )
    {
        const int chunk_size = 262144;

        std::string    module_str = parseCalike( filename );
        if ( module_str.empty() ) {
            return nullptr;
        }
        xml = new easyDomFactoryXML;

        XML_Parser parser = XML_ParserCreate( nullptr );
        XML_SetUserData( parser, this );
        XML_SetElementHandler( parser, &easyDomFactory::beginElement, &easyDomFactory::endElement );
        XML_SetCharacterDataHandler( parser, &easyDomFactory::charHandler );

        int index       = 0;
        int string_size = module_str.size();
        int incr        = chunk_size-2;
        int is_final    = false;
        do {
            char buf[chunk_size];

            int  max_index = index+incr;
            int  newlen    = incr;
            if (max_index >= string_size) {
                newlen = module_str.size()-index;
                const char *strbuf = module_str.c_str();
                memcpy( buf, strbuf+index, sizeof (char)*newlen );
            } else {
                const char *strbuf = module_str.c_str();
                memcpy( buf, strbuf+index, sizeof (char)*incr );
                newlen = incr;
            }
            index += newlen;
            if (index >= string_size) {
                is_final = true;
            }
            XML_Parse( parser, buf, newlen, is_final );
        } while (!is_final);
        XML_ParserFree( parser );
        delete xml;
        return (domNodeType*) topnode;
    }

    static void beginElement( void *userData, const XML_Char *name, const XML_Char **atts )
    {
        ( (easyDomFactory*) userData )->beginElement( name, atts );
    }
    static void endElement( void *userData, const XML_Char *name )
    {
        ( (easyDomFactory*) userData )->endElement( name );
    }

//void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes){
    void doTextBuffer()
    {
        if ( !nodestack.size() ) {
            return;
        }
        domNodeType *stacktop = nodestack.top();
        if (xml->buffer) {
            xml->buffer[xml->currentindex] = '\0';
            stacktop->set_attribute( textAttr, ( stacktop->attr_value( textAttr ) )+(xml->buffer) );
            free( xml->buffer );
        }
        xml->buffer = 0;
        xml->currentindex = 0;
    }

    void beginElement( const std::string &name, const XML_Char **atts )
    {
        //XMLSupport::AttributeList::const_iterator iter;

        doTextBuffer();
        domNodeType *parent;
        bool hasParent = false;
        if ( nodestack.empty() ) {
            parent = nullptr;
        } else {
            hasParent = true;
            parent    = nodestack.top();
        }
        domNodeType *thisnode = new domNodeType();
        thisnode->set( parent, name, atts );
        if (!hasParent) {
            topnode = thisnode;
        } else {
            parent->addChild( thisnode );
        }
        nodestack.push( thisnode );
    }

    void endElement( const string &name )
    {
        doTextBuffer();
        domNodeType *stacktop = nodestack.top();
        if (stacktop->Name() != name) {
            VS_LOG_AND_FLUSH(fatal, (boost::format("error: expected %1% , got %2%") % stacktop->Name() % name));
            VSExit( 1 );
        } else {
            nodestack.pop();
        }
    }

    std::stack< domNodeType* >nodestack;

    domNodeType *topnode;
};

#endif //_EASYDOM_H_

