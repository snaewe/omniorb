// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbOptionsReg.cc           Created on: 17/8/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.2.3  2001/08/20 15:38:56  sll
  Multi-options, such as endPoint, InitRef are support in NT registry.

  Revision 1.1.2.2  2001/08/20 10:43:28  sll
  New orb configuration parsing now works with NT registry.

  Revision 1.1.2.1  2001/08/20 08:19:24  sll
  Read the new ORB configuration file format. Can still read old format.
  Can also set configuration parameters from environment variables.

*/

#include <omniORB4/CORBA.h>
#include <orbOptions.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <winreg.h>

#define NEW_REGKEY "SOFTWARE\\ATT\\omniORB"
#define OLD_REGKEY "SOFTWARE\\ORL\\omniORB\\2.0"

OMNI_NAMESPACE_BEGIN(omni)

static void parseConfigReg(orbOptions& opt, HKEY rootkey);
static void parseOldConfigReg(orbOptions& opt, HKEY rootkey);

void
orbOptions::importFromRegistry() throw (orbOptions::Unknown,
					orbOptions::BadParam) {

  char* rootregname;
  HKEY  rootkey;

  rootregname = NEW_REGKEY;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,rootregname,0,
		   KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,&rootkey) == ERROR_SUCCESS) {
    try {
      parseConfigReg(*this,rootkey);
      RegCloseKey(rootkey);
    }
    catch (orbOptions::Unknown& ex) {
      RegCloseKey(rootkey);
      throw;
    }
    catch (orbOptions::BadParam& ex) {
      RegCloseKey(rootkey);
      throw;
    }
    return;
  }

  rootregname = OLD_REGKEY;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,rootregname,0,
		   KEY_QUERY_VALUE,&rootkey) == ERROR_SUCCESS) {
    try {
      parseOldConfigReg(*this,rootkey);
      RegCloseKey(rootkey);
    }
    catch (orbOptions::Unknown& ex) {
      RegCloseKey(rootkey);
      throw;
    }
    catch (orbOptions::BadParam& ex) {
      RegCloseKey(rootkey);
      throw;
    }
  }
}

#define KEYBUFSIZE   128
#define VALUEBUFSIZE 2048

static
CORBA::Boolean getRegEntry(HKEY rootkey, DWORD index,
			   char* keybuf, DWORD keybufsize,
			   char* valuebuf, DWORD valuebufsize,
			   char*& key, char*& value) {

  DWORD keylen   = keybufsize;
  DWORD valuelen = valuebufsize;
  DWORD valuetype;

  if (RegEnumValue(rootkey,index,
		   (LPTSTR) ((char*)keybuf),&keylen,NULL,
		   &valuetype,
		   (LPBYTE) ((char*)valuebuf), &valuelen) !=  ERROR_SUCCESS)
    return 0;

  if (valuetype != REG_SZ) {
    if ( omniORB::trace(1) ) {
      omniORB::logger log;
      log << "Error reading configuration data in the register. Reason: value is not of type REG_SZ\n";
    }
    throw orbOptions::Unknown("<missing>","<missing>");
  }

  char* p;

  p = keybuf;
  while ( isspace(*p) )
    p++;
  key = p;
  p += strlen(key) - 1;
  while ( isspace(*p) )
    p--;
  *(++p) = '\0';

  p = valuebuf;
  while ( isspace(*p) )
    p++;
  value = p;
  p += strlen(value) - 1;
  while ( isspace(*p) )
    p--;
  *(++p) = '\0';

  return 1;
}

static
void
parseConfigInSubKey(orbOptions& opt, HKEY rootkey, char* subkeyname) {

  HKEY subkey;

  if (RegOpenKeyEx(rootkey,subkeyname,0,
                   KEY_QUERY_VALUE,&subkey) == ERROR_SUCCESS) {
   try {

      DWORD total;
      DWORD keybufsize;
      DWORD valuebufsize;
      CORBA::String_var keybuf;
      CORBA::String_var valuebuf;

      RegQueryInfoKey(subkey,NULL,NULL,NULL,NULL,NULL,NULL,
		                  &total,&keybufsize,&valuebufsize,NULL,NULL);
      if (total) {
         keybuf = CORBA::string_alloc(keybufsize);
         valuebuf = CORBA::string_alloc(valuebufsize);
      }

      DWORD index = 0;
      while (index < total) {
		    char* key;
		    char* value;
        if (!getRegEntry(subkey,index,
                         keybuf,keybufsize+1,
                         valuebuf,valuebufsize+1,
		                     key,value))
          return;
        opt.addOption(subkeyname,value);
        index++;
      }
      RegCloseKey(subkey);
    }
    catch (orbOptions::Unknown& ex) {
      RegCloseKey(subkey);
      throw;
    }
    catch (orbOptions::BadParam& ex) {
      RegCloseKey(subkey);
      throw;
    }

  }
}

static
void parseConfigReg(orbOptions& opt, HKEY rootkey) {

  DWORD total;
  DWORD totalsubkeys;
  DWORD keybufsize;
  DWORD valuebufsize;
  DWORD subkeybufsize;
  CORBA::String_var keybuf;
  CORBA::String_var valuebuf;
	CORBA::String_var subkeybuf;

  RegQueryInfoKey(rootkey,NULL,NULL,NULL,&totalsubkeys,&subkeybufsize,NULL,
		  &total,&keybufsize,&valuebufsize,NULL,NULL);


  if (total) {
    keybuf = CORBA::string_alloc(keybufsize);
    valuebuf = CORBA::string_alloc(valuebufsize);
    if (totalsubkeys) subkeybuf = CORBA::string_alloc(subkeybufsize);
  }

  DWORD index = 0;
  while (index < total) {
		char* key;
		char* value;
    if (!getRegEntry(rootkey,index,
                     keybuf,keybufsize+1,
                     valuebuf,valuebufsize+1,
		                 key,value))
      return;
    opt.addOption(key,value);
    index++;
  }

  index = 0;
  while (index < totalsubkeys) {
		DWORD subkeylen = subkeybufsize + 1;
		if ( RegEnumKeyEx(rootkey,index,
		                  (LPTSTR) ((char*)subkeybuf),&subkeylen,
		                  NULL,NULL,NULL,NULL) != ERROR_SUCCESS ) {
			 omniORB::logger log;
			 log << "Cannot red subkey name for index " << index << "\n";
		}
		parseConfigInSubKey(opt,rootkey,subkeybuf);
		index++;
	}
}

static
void
oldconfig_warning(const char* key, const char* newkey) {
  if (omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: translated (" << key << ") to (" << newkey << ")\n";
  }
}

static
void parseOldConfigReg(orbOptions& opt, HKEY rootkey) {

  DWORD total;
  DWORD keybufsize;
  DWORD valuebufsize;
  CORBA::String_var keybuf;
  CORBA::String_var valuebuf;

  RegQueryInfoKey(rootkey,NULL,NULL,NULL,NULL,NULL,NULL,
		  &total,&keybufsize,&valuebufsize,NULL,NULL);

  if (total) {
    keybuf = CORBA::string_alloc(keybufsize);
    valuebuf = CORBA::string_alloc(valuebufsize);
  }

  if (omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: the registry entries are in the old pre-omniORB4 format.\n"
 "omniORB: For the moment this is accepted to maintain backword compatibility.\n"
 "omniORB: Please update to the new registry format ASAP.\n";
  }


  DWORD index = 0;
  while (index < total) {
		char* key;
		char* value;

    if (!getRegEntry(rootkey,index,
                     keybuf,keybufsize+1,
                     valuebuf,valuebufsize+1,
		                 key,value))
      return;

    if (strcmp(key,"ORBInitRef") == 0) {
      oldconfig_warning("ORBInitRef","InitRef");
      opt.addOption(key+3,value);
    }
    else if (strcmp(key,"ORBDefaultInitRef") == 0) {
      oldconfig_warning("ORBDefaultInitRef","DefaultInitRef");
      opt.addOption(key+3,value);
    }
    else if (strcmp(key,"NAMESERVICE") == 0) {
      oldconfig_warning("NAMESERVICE","InitRef NameService=");
      const char* format = "NameService=%s";
      CORBA::String_var v(CORBA::string_alloc(strlen(value)+strlen(format)));
      sprintf(v,format,value);
      opt.addOption("InitRef",v);
    }
    else if (strcmp(key,"INTERFACE_REPOSITORY") == 0) {
      oldconfig_warning("INTERFACE_REPOSITORY","InitRef InterfaceRepository=");
      const char* format = "InterfaceRepository=%s";
      CORBA::String_var v(CORBA::string_alloc(strlen(value)+strlen(format)));
      sprintf(v,format,value);
      opt.addOption("InitRef",v);
    }
    else if (strcmp(key,"ORBInitialHost") == 0) {
      oldconfig_warning("ORBInitialHost","bootstrapAgentHostname");
      opt.addOption("bootstrapAgentHostname",value);
    }
    else if (strcmp(key,"ORBInitialPort") == 0) {
      oldconfig_warning("ORBInitialPort","bootstrapAgentPort");
      opt.addOption("bootstrapAgentPort",value);
    }
    else if (strcmp(key,"GATEKEEPER_ALLOWFILE") == 0) {
      oldconfig_warning("GATEKEEPER_ALLOWFILE","ignored");
    }
    else if (strcmp(key,"GATEKEEPER_DENYFILE") == 0) {
      oldconfig_warning("GATEKEEPER_DENYFILE","ignored");
    }
    else {
      throw orbOptions::Unknown(key,value);
    }

    index++;
  }

}

OMNI_NAMESPACE_END(omni)
