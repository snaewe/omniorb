// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initFile.cc                Created on: 10/01/97
//                            Author    : Eoin Carroll  (ewc)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
  Revision 1.18  1998/01/20 17:32:01  sll
  Added support for OpenVMS.

  Revision 1.17  1997/12/18 17:27:51  sll
  *** empty log message ***

  Revision 1.16  1997/12/12 21:00:33  sll
  *** empty log message ***

  Revision 1.15  1997/12/12 18:42:50  sll
  New configuration option GATEKEEPER_ALLOWFILE and GATEKEEPER_DENYFILE.

  Revision 1.14  1997/12/09 18:21:46  sll
  Fixed memory leaks.

  Revision 1.13  1997/08/27 10:19:52  sll
  Removed static variables inside functions.

  Revision 1.12  1997/05/12 14:13:33  ewc
  Minor cosmetic change.

// Revision 1.9  1997/05/06  15:21:25  sll
// Public release.
//
  */


#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __atmos__
#include <kernel.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#include <winreg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined(UnixArchitecture) || defined(__VMS)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <omniORB2/CORBA.h>
#include <initFile.h>

#include "gatekeeper.h"

#ifndef INIT_ENV_VAR
#define INIT_ENV_VAR "OMNIORB_CONFIG"    
                      // Environment variable that points to configuration file. 
#endif

#ifndef INIT_REGKEY
#define INIT_REGKEY  "SOFTWARE\\ORL\\omniORB\\2.0"
                     // Path to Registry Keys for omniORB2 [NT specific]
#endif


#define INIT_MAX_CONFIG 10   
                     // Maximum number of entries in configuration 

static const CosNaming::NamingContext_proxyObjectFactory CosNaming_NamingContext_proxyObjectFactory1; // To ensure that Naming Stubs are linked.


initFile::initFile() : fData(0), fsize(0), currpos(0)
{
#ifdef __WIN32__
  use_registry = 0;
  curr_index = 0;
#endif
  _NameService = CORBA::Object::_nil();
}


initFile::~initFile()
{
  if (fData) {
    delete [] fData;
  }
}


void initFile::initialize()
{
  // Note: Using standard C file functions for compatibility with ATMos

  CORBA::String_var config_fname;

// Get filename:

#if defined(UnixArchitecture) || defined(NTArchitecture) || defined(__VMS)

  char* tmp_fname;

  if ((tmp_fname = getenv(INIT_ENV_VAR)) == NULL)
    {
#if defined(UnixArchitecture) || defined(__VMS)
      config_fname = CORBA::string_dup(CONFIG_DEFAULT_LOCATION);
#elif defined(NTArchitecture)

      // Can't find configuration file using the
      // environment variable -- NT will attempt to use the registry to find
      // configuration information.

      // Open the registry, and get a handle to the key:

      char* omniORBSubKey = INIT_REGKEY;
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,omniORBSubKey,0,KEY_QUERY_VALUE,
		        &init_hkey) != ERROR_SUCCESS)
	{
	  // Don't throw exception, as information in file 
	  // may not be used - fail only when a call that requires
	  // the information is made.

	  return;
	}
      else 
	{
	  DWORD numVals;
	  RegQueryInfoKey(init_hkey,NULL,NULL,NULL,NULL,NULL,NULL,
			  &numVals,&init_maxValLen,&init_maxDataLen,NULL,NULL);

	  if(numVals == 0) 
	    {
	      if (omniORB::traceLevel >= 2)
		noValsFound();
	      return;
	    }
	  
	  use_registry = 1;
	}
#endif
    }
  else
    {
      config_fname = CORBA::string_dup(tmp_fname);
    }
  
#elif defined(ATMosArchitecture)
  
  config_fname = CORBA::string_dup("//isfs/omniORB.cfg");

#endif

#ifdef __WIN32__
  if (!use_registry) 
    {
#endif
      if (read_file(config_fname) < 0)
	{
	  // Can't open file.
	  // Don't throw exception, as information in file 
	  // may not be used - fail only when a call that requires
	  // the information is made.
	  return;
	}
#ifdef __WIN32__
    }
#endif

  // Get configuration data:

  CORBA::String_var entryname;
  CORBA::String_var data;
  int multcheck[INIT_MAX_CONFIG] = { 0 }; 
  
  while(getnextentry(entryname,data))
    {
      if (strcmp((const char*)entryname,"NAMESERVICE") == 0)
	{
	  if (multcheck[0] == 1)
	    multerr(entryname);
	  else 
	    multcheck[0] = 1;
	  
	  try
	    {
	      omniObject* objptr = omni::stringToObject(data);
	      _NameService = (CORBA::Object_ptr) 
		                objptr->_widenFromTheMostDerivedIntf(0);
	      
	    }
	  catch(const CORBA::MARSHAL&)
	    {
	      invref(entryname);
	    }

	  if((_NameService->PR_getobj()->_widenFromTheMostDerivedIntf(
				  CosNaming_NamingContext_IntfRepoID)) == 0)
	    {
	      // The object reference supplied is not for the NamingService
	      
	      invref(entryname);
	    }    
	}
      else if (strcmp(entryname, "GATEKEEPER_ALLOWFILE") == 0)
	{
	  gateKeeper::allowFile = CORBA::string_dup(data);	  
	}
      else if (strcmp(entryname, "GATEKEEPER_DENYFILE") == 0)
	{
	  gateKeeper::denyFile = CORBA::string_dup(data);
	}
      else
	{
	  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
	    cerr << "Configuration error:  Unknown field (" 
		 << (const char*) entryname << ") "
		 << " found in configuration." << endl;
#else
	    kprintf("Configuration error:  ");
	    kprintf("Unknown field (%s) found in configuration file.\n",(const char*)entryname);
#endif
	  }
	 throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
	}
    }
  return;
}


int initFile::read_file(char* config_fname)
{
  // Test if the specified file exists and is not a directory
#if defined(UnixArchitecture) || defined(__VMS)
  {
    struct stat stbuf;
    if (stat(config_fname,&stbuf) < 0 || !S_ISREG(stbuf.st_mode)) {
      if (omniORB::traceLevel >= 2) {
	cerr << "omniORB configuration file: "
	     << config_fname << " either does not exist or is not a file."
	     << endl;
      }
      return -1;
    }
  }
#endif

#ifdef __WIN32__
  {
    struct _stat stbuf;
    if (_stat(config_fname,&stbuf) != 0 || ! (_S_IFREG && stbuf.st_mode)) {
      if (omniORB::traceLevel >= 2) {
	cerr << "omniORB configuration file: "
	     << config_fname << " either does not exist or is not a file."
	     << endl;
      }
      return -1;
    }
  }
#endif


  // Read the configuration file:
  FILE* iFile;
  if ((iFile = fopen(config_fname,"r")) == NULL) 
    return -1;

  fsize = 0;
  fseek(iFile,0L,SEEK_END);
  fsize = ftell(iFile);
  rewind(iFile);

  fData = new char[fsize+1];
  if (fData == NULL) 
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);

  size_t result = fread((void*) fData,1,fsize,iFile);
  fclose(iFile);

#if defined(__WIN32__) || defined(__VMS)
  // If file contains CRs, fsize will not be true length of string
  fsize = result;
#endif
  fData[fsize]='\0';

  return 0;
}



int initFile::getnextentry(CORBA::String_var& entryname,
			   CORBA::String_var& data)
{
  // Get next entry in config file, and associated data

#ifdef __WIN32__
  // Use registry for NT if environment variable for config file not set:
  if (use_registry)
    return getRegistryEntry(entryname,data);
#endif

// Skip initial whitespace:
  if (currpos == fsize) return 0;
  while (isspace(fData[currpos]) || fData[currpos] == '#')
    {
      if (fData[currpos] == '#') 
	{
	  while(fData[currpos] != '\n') 
	    {
	      currpos++;
	      if (currpos == fsize) return 0;
	    }
	  continue;
	}
      currpos++;
      if (currpos == fsize) return 0;
    }

  // Read in field name:
  long startpos = currpos;

  do
    {
      currpos++;
    
      if (currpos == fsize) parseerr();
      else if (fData[currpos] == '\n') parseerr();
    }
  while (!isspace(fData[currpos]));

  entryname = CORBA::string_alloc((currpos-startpos) + 1);
  strncpy(entryname,(fData+startpos),(currpos-startpos));
  ((char*)entryname)[currpos-startpos] = '\0';


  // Skip whitespace between keyword and data:

  while (isspace(fData[currpos]))
    {
      if (currpos == fsize) 
	parseerr();
      else 
	if (fData[currpos] == '\n') 
	  dataerr(entryname);
      currpos++;
    }

  // Read data:

  startpos = currpos;

  while (!isspace(fData[currpos]))
    {
      if (currpos == fsize)
	break;
      currpos++;
    }

  if (startpos == currpos)
    parseerr();

  data = CORBA::string_alloc((currpos - startpos)+1);
  strncpy(data,(fData+startpos),(currpos-startpos));
  ((char*)data)[currpos-startpos] = '\0';

  return 1;
}



#ifdef __WIN32__

// NT member function to use registry:

int initFile::getRegistryEntry(CORBA::String_var& entryname,
			       CORBA::String_var& data)
{
  DWORD dataType;
  DWORD init_ValLen = init_maxValLen+1;
  DWORD init_DataLen = init_maxDataLen+1;
  entryname = CORBA::string_alloc(init_ValLen);
  data      = CORBA::string_alloc(init_DataLen);

  if (RegEnumValue(init_hkey,curr_index++,(LPTSTR) ((char*)entryname),
		   &init_ValLen,NULL,
		   &dataType,(LPBYTE) ((char*)data), 
		   &init_DataLen) != ERROR_SUCCESS)
    {
      return 0;
    }

  if (dataType != REG_SZ)
    {
      // Data is not a character string.
      formaterr(entryname);
    }
    
  return 1;
}

#endif



// Implementations of inline error-report functions:

void initFile::multerr(char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    cerr << "Configuration error: Multiple entries found for field "
	 << entryname << " ." << endl;
#else
    kprintf("Configuration error: ");
    kprintf("Multiple entries found for field %s in configuration file.\n",
	    entryname);
#endif
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
}


void initFile::dataerr(char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    cerr << "Configuration error: No data found for field "
	 << entryname << " in configuration file." << endl;
#else
    kprintf("Configuration error: No data found for field %s",entryname);
    kprintf(" in configuration file.\n");
#endif
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
}


void initFile::parseerr()
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    cerr << "Configuration error: Parse error in config file." << endl;
#else
    kprintf("Configuration error: Parse error in config file.\n");
#endif
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
}


void initFile::invref(char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    cerr << "Configuration error: Invalid object reference supplied for "
	 << entryname << "." << endl;
#else
    kprintf("Configuration error: Invalid object reference supplied for %s.\n",
	    entryname);
#endif
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
}


#ifdef __WIN32__

// NT-specific error reporting functions:

void initFile::noValsFound()
{
  if (omniORB::traceLevel > 0) {
    cerr << "Configuration error: No values found in registry key"
         << "\nHKEY_LOCAL_MACHINE\\" 
	 << INIT_REGKEY << endl;
    cerr << "Either set the environment variable OMNIORB_CONFIG to point"
         << "\nto the omniORB configuration file, or enter the IOR for the"
         << "\nnaming service in to the registry in the (string) value"
         << "\nNAMESERVICE , under the registry entry HKEY_LOCAL_MACHINE\\" 
	 << INIT_REGKEY << endl;
  }
}


void initFile::formaterr(char* entryname)
{
  if (omniORB::traceLevel > 0) {
    cerr << "Configuration error: Data for value " << entryname 
	 << " is not a character string." << endl;
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
}

#endif
