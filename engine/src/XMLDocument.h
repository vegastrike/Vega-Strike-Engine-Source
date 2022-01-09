/**
* XMLDocument.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#ifndef __XMLDOCUMENT_H__INCLUDED__
#define __XMLDOCUMENT_H__INCLUDED__

#include <string>
#include <map>
#include <vector>
#include <iterator>

#include <ostream>

namespace XMLDOM
{
class XMLDocument;

class XMLElement
{
public:
    enum Type
    {
        XET_TAG,
        XET_CDATA,
        XET_COMMENT,
        XET_ROOT
    };

private:
    Type mType;
    std::string mTagName;
    std::string mContents;     //for cdata elements

    typedef std::vector< XMLElement* >          ElementList;
    typedef std::map< std::string, XMLElement* >ElementMap;
    typedef std::map< std::string, std::string >AttributeMap;

    ElementList  mChildren;
    ElementMap   mById;
    ElementMap   mByName;
    AttributeMap mAttributes;

    XMLElement  *mParent;
    XMLDocument *mDocument;

public:
    typedef ElementList::iterator        child_iterator;
    typedef ElementList::const_iterator  const_child_iterator;

    typedef AttributeMap::iterator       attribute_iterator;
    typedef AttributeMap::const_iterator const_attribute_iterator;

private:
    const_attribute_iterator mIdAttribute;
    const_attribute_iterator mNameAttribute;

public:
    //Constructors

    /** Creates an empty cdata element */
    XMLElement();

    /** Creates an element of type 'type' - with 'data' as contents
     *  @remarks
     *   empty if type is not XET_CDATA or XET_COMMENT
     */
    XMLElement( Type type, const std::string &data = std::string() );

    /** Creates a CData element with 'cdata' as contents */
    XMLElement( const std::string &cdata );

    /** Creates a TAG element with specified tagName and attributes */
    XMLElement( const char *tagName, const char*const *attrValuePairList );

    /** Creates a TAG element with specified tagName and attributes */
    XMLElement( const char *tagName, const char*const *attrValuePairList, unsigned int nAttr );

    /** Creates a TAG element with specified tagName and attributes */
    XMLElement( const std::string &tagName, const std::vector< std::string > &attrValuePairList );

    /** Creates a TAG element with specified tagName and attributes */
    XMLElement( const std::string &tagName, const std::map< std::string, std::string > &attrValuePairList );

    ~XMLElement();

protected:
    void setParent( XMLElement *parent );

    /** @note Cascade effect */
    void setDocument( XMLDocument *document );

public:
    /** Returns the type of element - only a few types have been defined, nowhere near true DOM types */
    Type type() const
    {
        return mType;
    }

    /** Changes the type of element - beware, though... it destroys the element */
    void setType( Type newType );

    /** Returns the tag name of this XET_TAG element - otherwise... undefined */
    const std::string& tagName() const
    {
        return mTagName;
    }

    /** Returns the element containing this element, if any - NULL otherwise */
    const XMLElement * getParent() const
    {
        return mParent;
    }

    /** Returns the element containing this element, if any - NULL otherwise */
    XMLElement * getParent()
    {
        return mParent;
    }

    /** Returns the element containing this element, if any - NULL otherwise */
    const XMLDocument * getDocument() const
    {
        return mDocument;
    }

    /** Returns the element containing this element, if any - NULL otherwise */
    XMLDocument * getDocument()
    {
        return mDocument;
    }

    /** Returns the element's content
     *  @remarks
     *   For cdata/comment elements, the character data inside the element.
     *   For tag elements, the contents of the first child if its an cdata element,
     *   otherwise just empty.
     */
    const std::string& contents() const;

    /** Returns a read-write reference to the element's content
     *  @remarks
     *   For cdata/comment elements only - calling it with tag elements will probably just
     *   bring an 'assertion failed' dialog ;)
     */
    std::string& contents();

    /** Appends character contents to this element.
     *  @remarks
     *   Works for all element types. For CDATA elements, quite straightforwardly
     *   appends the specified string to the contents string. For TAG elements,
     *   it checks the last child element - if it's a CDATA element, it appends
     *   to this element. If not, it creates a new CDATA element with those contents
     *   instead. Useful for tidy XML document parsing.
     */
    void appendContents( const std::string &cont );

    /** Returns the number of children this element has attached */
    unsigned int numChildren() const
    {
        return mChildren.size();
    }

    /** Returns a read-only pointer to the specified children, or NULL if idx is out of range */
    const XMLElement * getChild( unsigned int idx ) const
    {
        return ( idx < mChildren.size() ) ? mChildren[idx] : 0;
    }

    /** Returns a pointer to the specified children, or NULL if idx is out of range */
    XMLElement * getChild( unsigned int idx )
    {
        return ( idx < mChildren.size() ) ? mChildren[idx] : 0;
    }

    /** Returns a read-only pointer to the specified children, or NULL if id isn't found */
    const XMLElement * getChildById( const std::string &id ) const;

    /** Returns a pointer to the specified children, or NULL if id isn't found */
    XMLElement * getChildById( const std::string &id );

    /** Returns a read-only pointer to the specified children, or NULL if name isn't found */
    const XMLElement * getChildByName( const std::string &name ) const;

    /** Returns a pointer to the specified children, or NULL if name isn't found */
    XMLElement * getChildByName( const std::string &name );

    /** Returns the "id" attribute's value - much quicker than getAttribute() */
    const std::string& getId() const
    {
        static std::string empty;
        return ( mIdAttribute == mAttributes.end() ) ? empty : mIdAttribute->second;
    }

    /** Returns the "name" attribute's value - much quicker than getAttribute() */
    const std::string& getName() const
    {
        static std::string empty;
        return ( mNameAttribute == mAttributes.end() ) ? empty : mNameAttribute->second;
    }

    /** Returns a read-only pointer to the specified children, or NULL if id isn't found
     *  @remarks
     *   Hierarchical IDs are tree-like IDs which sometimes are able to resolve ambiguities.
     *   They're specified like file paths: <name1>/<name2>/<name3> involves searching for
     *   'name1', then inside the found element (if any), search for 'name2', and then inside
     *   'name2' search for 'name3'. Notice that there still is room for ambiguity... but
     *   with well design structures, it should be avoidable.
     */
    const XMLElement * getChildByHierarchicalId( const std::string &id ) const;

    /** Returns a pointer to the specified children, or NULL if id isn't found
     *  @remarks
     *   Hierarchical IDs are tree-like IDs which sometimes are able to resolve ambiguities.
     *   They're specified like file paths: <name1>/<name2>/<name3> involves searching for
     *   'name1', then inside the found element (if any), search for 'name2', and then inside
     *   'name2' search for 'name3'. Notice that there still is room for ambiguity... but
     *   with well design structures, it should be avoidable.
     */
    XMLElement * getChildByHierarchicalId( const std::string &id );

    /** Returns a read-only pointer to the specified children, or NULL if id isn't found
     *  @see getChildByHierarchicalId
     */
    const XMLElement * getChildByHierarchicalName( const std::string &name ) const;

    /** Returns a pointer to the specified children, or NULL if id isn't found
     *  @see getChildByHierarchicalId
     */
    XMLElement * getChildByHierarchicalName( const std::string &name );

    /** Add the specified element as a child to this one.
     *  @remarks
     *   The class takes ownership of the pointer.
     */
    unsigned int appendChild( XMLElement *newElem );

    /** Remove the specified element from this one.
     *  @remarks
     *   May be costly... since internal housekeeping has to be done as well.
     */
    void removeChild( unsigned int idx );

    /** @copydoc removeChild */
    void removeChild( const XMLElement *which );

    /** @copydoc removeChild */
    void removeChildById( const std::string &id )
    {
        removeChild( getChildById( id ) );
    }

    /** @copydoc removeChild */
    void removeChildByName( const std::string &name )
    {
        removeChild( getChildByName( name ) );
    }

    /** Remove everything - including attributes, if specified - in a very quick manner */
    void clear( bool doAttributes = true );

    /** Get an iterator pointing to the specified attribute */
    const_attribute_iterator getAttribute( const std::string &name ) const
    {
        return mAttributes.find( name );
    }

    /** Get an iterator pointing to the specified attribute */
    attribute_iterator getAttribute( const std::string &name )
    {
        return mAttributes.find( name );
    }

    /** Remove the specified attribute */
    void removeAttribute( const std::string &name );

    /** Set the contents of the specified attribute */
    void setAttribute( const std::string &name, const std::string &value );

    /** Get the contents of the specified attribute - undefined if the attribute does not exist */
    const std::string& getAttributeValue( const std::string &name ) const
    {
        return getAttribute( name )->second;
    }

    /** Get the contents of the specified attribute - or the specified default if it does not exist */
    const std::string& getAttributeValue( const std::string &name, const std::string &def ) const
    {
        const_attribute_iterator it = getAttribute( name );
        return ( it == attributesEnd() ) ? def : it->second;
    }

    /** Get an iterator pointing to the first attribute */
    attribute_iterator attributesBegin()
    {
        return mAttributes.begin();
    }
    
    /** Get an iterator pointing to the first attribute */
    const_attribute_iterator attributesBegin() const
    {
        return mAttributes.begin();
    }
    /** Get an iterator pointing to the last attribute */
    attribute_iterator attributesEnd()
    {
        return mAttributes.end();
    }
    
    /** Get an iterator pointing to the last attribute */
    const_attribute_iterator attributesEnd() const
    {
        return mAttributes.end();
    }

    /** Get an iterator pointing to the first child */
    child_iterator childrenBegin()
    {
        return mChildren.begin();
    }
    
    /** Get an iterator pointing to the first child */
    const_child_iterator childrenBegin() const
    {
        return mChildren.begin();
    }
    
    /** Get an iterator pointing to the last child */
    child_iterator childrenEnd()
    {
        return mChildren.end();
    }
    
    /** Get an iterator pointing to the last child */
    const_child_iterator childrenEnd() const
    {
        return mChildren.end();
    }

    /** Recreate the internal structures for getChildByName/getChildById
     *  @param deepScan If true (default), children will be added to the map
     *   so that a getChildByX() may return deep children. If false, only
     *   direct descendants will be added, which is much less likely to
     *   generate collisions, and would be preferred for highly structured
     *   sources.
     */
    void rebuildNamedBindings( bool deepScan = true );

private:
    static void JoinMaps( ElementMap &dst, const ElementMap &src );
};

class XMLDocument
{
public:
    /** Document type, as defined in <!DOCTYPE ...> */
    std::string docType;
    std::string sysId;
    std::string pubId;

    /** Set to true whenever the document changes */
    bool dirty;

    /** The root element containing the entire document */
    XMLElement root;

public: XMLDocument() : dirty( false )
        , root( XMLElement::XET_ROOT ) {}
    ~XMLDocument() {}

public:

    //
    //A series of handy wrappers, to mimic JavaScript.
    //(mostly for Python GUI event handlers)
    //

    /** @copydoc XMLElement::rebuildNamedBindings */
    void rebuildNamedBindings( bool deepScan = true )
    {
        root.rebuildNamedBindings( deepScan );
    }

    /** @copydoc XMLElement::removeChild */
    void removeElementById( const std::string &id )
    {
        root.removeChildById( id );
    }

    /** @copydoc XMLElement::removeChild */
    void removeElementByName( const std::string &name )
    {
        root.removeChildByName( name );
    }

    /** Remove everything in a very quick manner */
    void clear()
    {
        root.clear();
    }

    /** @copydoc XMLElement::getChildById */
    const XMLElement * getElementById( const std::string &id ) const
    {
        return root.getChildById( id );
    }

    /** @copydoc XMLElement::getChildById */
    XMLElement * getElementById( const std::string &id )
    {
        return root.getChildById( id );
    }

    /** @copydoc XMLElement::getChildByName */
    const XMLElement * getElementByName( const std::string &name ) const
    {
        return root.getChildByName( name );
    }

    /** @copydoc XMLElement::getChildByName */
    XMLElement * getElementByName( const std::string &name )
    {
        return root.getChildByName( name );
    }

    /** @copydoc XMLElement::getChildByHierarchicalId */
    const XMLElement * getElementByHierarchicalId( const std::string &id ) const
    {
        return root.getChildByHierarchicalId( id );
    }

    /** @copydoc XMLElement::getChildByHierarchicalId */
    XMLElement * getElementByHierarchicalId( const std::string &id )
    {
        return root.getChildByHierarchicalId( id );
    }

    /** @copydoc XMLElement::getChildByHierarchicalName */
    const XMLElement * getElementByHierarchicalName( const std::string &name ) const
    {
        return root.getChildByHierarchicalName( name );
    }

    /** @copydoc XMLElement::getChildByHierarchicalName */
    XMLElement * getElementByHierarchicalName( const std::string &name )
    {
        return root.getChildByHierarchicalName( name );
    }
};

class XMLProcessor;

class XMLSerializer
{
public:
    enum Options
    {
        OPT_WANT_CDATA=0x01,
        OPT_WANT_COMMENTS    =0x02,
        OPT_WANT_NAMEBINDINGS=0x04,
        OPT_WANT_NAMEBINDINGS_DEEPSCAN=0x08,
        OPT_WANT_ALL=OPT_WANT_CDATA|OPT_WANT_COMMENTS,
        OPT_WANT_DEEPBINDINGS=OPT_WANT_NAMEBINDINGS|OPT_WANT_NAMEBINDINGS_DEEPSCAN,
        OPT_DEFAULT=OPT_WANT_ALL|OPT_WANT_DEEPBINDINGS
    };
    int   options;

private:
    void *internals;

public: XMLSerializer( const char *encoding = 0, XMLDocument *doc = 0, XMLElement *elem = 0 );
    virtual ~XMLSerializer();

    /** Parse the specified data chunk from memory
     *  @remarks
     *   Base implementation of data parsing - override this one
     *   if you want to do it differently, intercept the data
     *   chunks, preprocess, or things like that.
     *  @par
     *   Returns false on error.
     */
    virtual bool parse( const void *buf, unsigned int len );

    /** Parse the specified file
     *  @remarks
     *   You can mix it with other kinds of parsing - the
     *   result is rather interesting: concatenation of data.
     *   Calls the parse(const void*,unsigned int) implementation.
     *  @par
     *   Override, for instance, to retrieve data in special ways
     *   (like from special resource providers).
     *  @par
     *   Returns false on error.
     */
    virtual bool importXML( const std::string &path );

    /** Write the document to the specified stream.
     *  @remarks
     *   You must pass... hence... a document in the constructor.
     *   That... or you're just going to write the current document.
     *   Pick your choice.
     */
    virtual bool exportXML( std::ostream &stream );

    /** Write the document to the specified file. @see write(std::ostream&) */
    bool exportXML( const std::string &filename );

    /** Add the specified processor (a subclass of XMLProcessor)
     *  @remarks
     *   This allows definition of xml processors without requiring
     *   a subclass of XMLParser
     */
    virtual void addProcessor( XMLProcessor *proc );

    /** Initialise parsing - destroy any current data if already initialised
     *  @remarks
     *   If you pass a document, its contents will be 'inherited'.
     *   If you pass an element pointer (make certain it is an element of the
     *   passed document), all parsing results are added as a child of that
     *   element - if not, the document's root element is used for that purpose.
     *  @par
     *   Have in mind that until close() is called, or the parser is destroyed,
     *   it will own 'doc' (and hence 'elem') - which means it will delete it
     *   if destroyed.
     *  @param encoding If NULL, the default character encoding (UTF8). If not,
     *   the name of another encoding. Note, though, that handling of different
     *   encodings is not yet supported - this is a placeholder for future enhancement,
     *   and as such should always be set to NULL, until the day custom encodings
     *   are handled.
     *  @param doc If NULL, a new document will be created. If not, parsed elements will
     *   be added to the specified document.
     *  @param elem If NULL, parsed elements will be added to the document's root element
     *   (either the automatically generated document, or the specified document). If not,
     *   they will be added as child elements of the specified element.
     *   Be certain to pass an element within doc... otherwise, results will be undefined.
     */
    virtual bool initialise( const char *encoding = 0, XMLDocument *doc = 0, XMLElement *elem = 0 );

    /** Close parsing - do cleanup, free memory, etc...
     *  @remarks
     *   Returns the parsed document - make sure to grab it, or delete it.
     */
    virtual XMLDocument * close();
};

class XMLProcessor
{
public: XMLProcessor() {}
    virtual ~XMLProcessor() {}

    /** Specify which instruction target this processor will handle */
    virtual const std::string& getTarget() const = 0;

    /** Execute a processing instruction found in the document
     *  @remarks
     *   Modify the document or current element as a result of the processing.
     *   There is no mechanism for 'textual processing'... rather, logic structure
     *   processing (you handle XMLDocument and XMLElement objects, not text).
     *  @par
     *   Return false, and the error description, to abort parsing.
     */
    virtual std::pair< bool, std::string >execute( XMLSerializer *serializer,
                                                   XMLDocument *doc,
                                                   XMLElement *current,
                                                   const std::string &data ) = 0;
};
}

#endif //__XMLDOCUMENT_H__INCLUDED__

