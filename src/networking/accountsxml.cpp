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
#include "vsfilesystem.h"

#include <expat.h>

using std::vector;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

using namespace VSFileSystem;

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
  string file;

  Account tmpacct;
  //  Account acct;
  int level=-1;
  stdext::hash_map <string, Account*> accounttable;
  int nbaccounts = 0;
 
  void beginElement (void *userData, const XML_Char *name, const XML_Char **atts)
  {
    AttributeList attributes (atts);
    tmpacct=Account ();
    Names elem = (Names) element_map.lookup(string (name));
    
    AttributeList::const_iterator iter;
    switch (elem) {
    case UNKNOWN:
      tmpacct.type=Account::UNKNOWN;
      break;
    case ACCOUNTS:
      assert (level==-1);
      level++;
      break;
    case PLAYER:
    case ADMIN:
      assert (level==0);
      level++;
      switch(elem) {
      case PLAYER:
        tmpacct.type=Account::PLAYER;
        break;
      case ADMIN:
        tmpacct.type=Account::ADMIN;
        break;
      default:
        tmpacct.type=Account::UNKNOWN;
        break;
      }
      for (iter= attributes.begin(); iter!=attributes.end();iter++)
      {
        switch (attribute_map.lookup ((*iter).name)) {
        case UNKNOWN:
          VSFileSystem::vs_fprintf (stderr,"Unknown Account Element %s\n",(*iter).name.c_str());
          break;
        case NAME:
          tmpacct.callsign = (*iter).value;
          break;
        case PASSWORD:
          tmpacct.passwd = (*iter).value;
          break;
        case SERVERIP:
          tmpacct.serverip = (*iter).value;
          break;
        case SERVERPORT:
          tmpacct.serverport = (*iter).value;
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
      assert (level==1);
      level--;
      if (!accounttable[strtoupper(tmpacct.callsign)]) { 
        accounttable[strtoupper(tmpacct.callsign)]=new Account(tmpacct);
        nbaccounts++;
      }
      break;
    default:
      break;
    }
  }

}

using namespace accountXML;


vector<Account *> getAllAccounts() {
  vector<Account*>retval;
  for (stdext::hash_map<string,Account*>::iterator iter=accounttable.begin();
       iter!=accounttable.end();
       ++iter) {
    if(iter->second)
      retval.push_back(iter->second);
  }
  return retval;
}


Account*getAcctSerial(ObjSerial ser) {

  for (stdext::hash_map<string,Account*>::iterator iter=accounttable.begin();
       iter!=accounttable.end();
       ++iter) {
    if (iter->second) {
      if (iter->second->getSerial()==ser) {
        return iter->second;
      }
    }
  }
  return NULL;
}


Account*getAcctAddress(SOCKETALT ser) {

  for (stdext::hash_map<string,Account*>::iterator iter=accounttable.begin();
       iter!=accounttable.end();
       ++iter) {
    if (iter->second) {
      if (iter->second->getSocket().sameAddress(ser)) {
        return iter->second;
      }
    }
  }
  return NULL;
}
void LoadAccounts(const char *filename)
{
  const int chunk_size = 16384;
  file=filename;
  VSFile f;
  VSError err = f.OpenReadOnly(filename,AccountFile);
  if (err>Ok) {
    return;
  }
  XML_Parser parser = XML_ParserCreate (NULL);
  XML_SetElementHandler (parser, &beginElement, &endElement);
  std::string data(f.ReadFull());
  XML_Parse (parser,data.c_str(),f.Size(),1);
  f.Close();
  XML_ParserFree (parser);
}
void addAcct(string key, Account * acct) {
  accounttable[strtoupper(key)]=acct;
}

Account * getAcctNoReload(const string &key) {
  return accounttable[strtoupper(key)];
}
Account * getAcctTemplate(const string &key) {
  Account*acct= accounttable[strtoupper(key)];
  if (acct==NULL) {
    LoadAccounts(file.c_str());
    acct=accounttable[strtoupper(key)];
  }
  return acct;
}
