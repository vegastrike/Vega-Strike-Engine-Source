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
  easyDom - easy DOM for expat - written by Alexander Rawass <alexannika@users.sourceforge.net>
*/


#include <expat.h>
#include "xml_support.h"
#include "easydom.h"

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

easyDomNode::easyDomNode(){
}

void easyDomNode::set(easyDomNode *_parent,string _name, AttributeList  *_attributes){
  parent=_parent;
  attributes=_attributes;

  for(AttributeList::const_iterator iter = _attributes->begin(); iter!=_attributes->end(); iter++) {
    //    cout <<  name << "::" << (*iter).name << endl;
    //    printf("iter=%x *iter=%x\n",iter,*iter);
    //cout << " " << (*iter).name << "=\"" << (*iter).value << "\"" << endl;
    att_name.push_back((*iter).name);
    att_value.push_back((*iter).value);
  }

  name=_name;
}

void easyDomNode::addChild(easyDomNode *child){
  subnodes.push_back(child);
}

string easyDomNode::attr_value(string search_name){
 vector<string>::const_iterator iter;
 vector<string>::const_iterator iter2;

  for(iter = att_name.begin(), iter2=att_value.begin() ; iter!=att_name.end(); iter++, iter2++) {
    if(search_name==(*iter)){
      return *iter2;
    }
  }

  string dummy;

  return dummy;
}

void easyDomNode::walk(int level){
 vector<string>::const_iterator iter;
 vector<string>::const_iterator iter2;

 cout << "<" << name ;

  for(iter = att_name.begin(), iter2=att_value.begin() ; iter!=att_name.end(); iter++, iter2++) {
       cout << " " << (*iter) << "=\"" << (*iter2) << "\"" ;
  }
  cout << ">" << endl;

  vector<easyDomNode *>::const_iterator siter;
  
  for(siter= subnodes.begin() ; siter!=subnodes.end() ; siter++){
    (*siter)->walk(level+1);
  }

  cout << "</" << name << ">" << endl;
}

easyDomFactory::easyDomFactory(){
}

void easyDomFactory::beginElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  ((easyDomFactory*)userData)->beginElement(name, AttributeList(atts));
}

void easyDomFactory::endElement(void *userData, const XML_Char *name) {
  ((easyDomFactory*)userData)->endElement(name);
}


void easyDomFactory::beginElement(const string &name, const AttributeList &attributes) {
  AttributeList::const_iterator iter;

  easyDomNode *parent;

  if(nodestack.empty()){
    parent=NULL;
  }
  else{
    parent=nodestack.top();
  }

  easyDomNode *thisnode=new easyDomNode();
  thisnode->set(parent,name,(AttributeList *) &attributes);

  for(iter = attributes.begin(); iter!=attributes.end(); iter++) {
    //cout <<  name << "::" << (*iter).name << endl;
  }

  if(parent==NULL){
    topnode=thisnode;
  }
  else{
    parent->addChild(thisnode);
  }
  nodestack.push(thisnode);

}

void easyDomFactory::endElement(const string &name) {

  easyDomNode *stacktop=nodestack.top();

  if(stacktop->Name()!=name){
    cout << "error: expected " << stacktop->Name() << " , got " << name << endl;
    exit(0);
  }
  else{
    nodestack.pop();
  }
  
}


easyDomNode *easyDomFactory::LoadXML(const char *filename) {

  const int chunk_size = 16384;

  FILE * inFile = fopen (filename, "r");
  if(!inFile) {
    assert(0);
    return NULL;
  }

  xml = new easyDomFactoryXML;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, this);
  XML_SetElementHandler(parser, &easyDomFactory::beginElement, &easyDomFactory::endElement);
  
  do {
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
    int length;

    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
    XML_ParseBuffer(parser, length, feof(inFile));
  } while(!feof(inFile));

  fclose (inFile);
  XML_ParserFree (parser);

  return topnode;
}

