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
  account_info Managment - Network Client account_info Managment - written by Stephane Vaxelaire <svax@free.fr>
*/
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include "const.h"
#include "xml_support.h"
#include "hashtable.h"
#include "accountsxml.h"

#include <expat.h>

using std::vector;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

/**************************************************************/
/**** Display an Account's info                            ****/
/**************************************************************/

void	Account::display()
{
	cout<<"Serial #: "<<serial<<endl;
	cout<<"Name: "<<callsign<<endl;
	cout<<"Pass: "<<passwd<<endl;
	if( this->type==0)
		cout<<"Type : UNKNOWN"<<endl;
	else if( this->type==1)
		cout<<"Type : PLAYER"<<endl;
	else if( this->type==2)
		cout<<"Type : ADMIN"<<endl;
	else 
		cout<<"Type : ERROR"<<endl;
}


namespace accountXML
{
/**************************************************************/
/**** XML Things                                           ****/
/**************************************************************/

  enum Names {
    UNKNOWN,
    PLAYER,
    ADMIN,
	ACCOUNTS,
    //attributes
    NAME,
    PASSWORD,
	SERIAL,
	SERVERIP,
	SERVERPORT
  };
  const EnumMap::Pair element_names[] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),//don't add anything until below Admin so it maps to enum ACCOUNT_TYPE
    EnumMap::Pair ("PLAYER",PLAYER),
    EnumMap::Pair ("ADMIN",ADMIN),
    EnumMap::Pair ("ACCOUNTS",ACCOUNTS)
  };
  const EnumMap::Pair attribute_names [] = {
    EnumMap::Pair ("UNKNOWN",UNKNOWN),
    EnumMap::Pair ("NAME",NAME),
    EnumMap::Pair ("PASSWORD",PASSWORD),
    EnumMap::Pair ("SERVERIP",SERVERIP),
    EnumMap::Pair ("SERVERPORT",SERVERPORT),
  };
  const EnumMap element_map(element_names, 4);
  const EnumMap attribute_map(attribute_names, 5);
  string curname;
  Account * tmpacct;
  Account acct;
  int level=-1;
  Hashtable <string, Account,char[257]> accounttable;
  int nbaccounts = 0;
 
  void beginElement (void *userData, const XML_Char *name, const XML_Char **atts)
  {
    AttributeList attributes (atts);
    tmpacct = new Account();
    Names elem = (Names) element_map.lookup(string (name));

	AttributeList::const_iterator iter;
	//cout<<"Elem : "<<elem<<" - name : "<<name<<endl;
    switch (elem) {
    case UNKNOWN:
		tmpacct->type=Account::UNKNOWN;
      break;
    case ACCOUNTS:
      assert (level==-1);
      level++;
	  //cout<<"Level : "<<level<<endl;
      break;
    case PLAYER:
    case ADMIN:
      assert (level==0);
      level++;
      switch(elem) {
		case PLAYER:
		tmpacct->type=Account::PLAYER;
		break;
		case ADMIN:
		tmpacct->type=Account::ADMIN;
		break;
		default:
		tmpacct->type=Account::UNKNOWN;
		break;
		  }
      for (iter= attributes.begin(); iter!=attributes.end();iter++)
	  {
			switch (attribute_map.lookup ((*iter).name)) {
			case UNKNOWN:
			  fprintf (stderr,"Unknown Account Element %s\n",(*iter).name.c_str());
			  break;
			case NAME:
			  curname = (*iter).value;
			  tmpacct->callsign = curname;
			  break;
			case PASSWORD:
			  curname = (*iter).value;
			  tmpacct->passwd = curname;
			  break;
			case SERVERIP:
			  curname = (*iter).value;
			  tmpacct->serverip = curname;
			  break;
			case SERVERPORT:
			  curname = (*iter).value;
			  tmpacct->serverport = curname;
			  break;
			default:
			  assert (0);
			  break;
			}
      }
      break;
	}
  }

  void endElement (void *userData, const XML_Char *name) {
    Names elem = (Names)element_map.lookup(name);
    switch (elem) {
    case UNKNOWN:
      break;
    case ACCOUNTS:
      assert (level==0);
      level--;
      break;
    case PLAYER:
    case ADMIN:
		//cout<<"Level : "<<level<<endl;
      assert (level==1);
      level--;
	  //tmpacct->display();
      accounttable.Put(strtoupper(curname), tmpacct);
	  nbaccounts++;
	break;
    default:
      break;
    }
  }

}

using namespace accountXML;

Account * getAcctTemplate(const string &key) {
  return accounttable.Get(strtoupper(key));
}

vector<Account *> getAllAccounts() {
  return accounttable.GetAll();
}

void LoadAccounts(const char *filename)
{
  const int chunk_size = 16384;
  FILE * inFile= fopen (filename,"r");
  if (!inFile) {
    return;
  }
  XML_Parser parser = XML_ParserCreate (NULL);
  XML_SetElementHandler (parser, &beginElement, &endElement);
 do {
#ifdef BIDBG
    char *buf = (XML_Char*)XML_GetBuffer(parser, chunk_size);
#else
    char buf[chunk_size];
#endif
    int length;
    
    length = fread (buf,1, chunk_size,inFile);
    //length = inFile.gcount();
#ifdef BIDBG
    XML_ParseBuffer(parser, length, feof(inFile));
#else
    XML_Parse (parser,buf,length,feof (inFile));
#endif
  } while(!feof(inFile));
 fclose (inFile);
 XML_ParserFree (parser);
}
