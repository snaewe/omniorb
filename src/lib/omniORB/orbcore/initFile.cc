// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initFile.cc                Created on: 10/01/97
//                            Author    : Eoin Carroll  (ewc)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.6  1997/03/10 14:22:18  sll
  Minor changes to filter out error messages depending on the trace level.

// Revision 1.5  1997/03/04  10:31:15  ewc
// Added support for obtaining initial object references from Windows NT/'95
// registry
//
// Revision 1.3  1997/01/22  14:33:51  ewc
// Small bug fix to file read.
//
// Revision 1.2  1997/01/21  15:05:43  ewc
// Minor change: Moved #defines to header files.
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

#ifdef __NT__
#include <windows.h>
#include <winreg.h>
#endif

#include <omniORB2/CORBA.h>


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

initFile::initFile()
{
  NameService = NULL;

#ifdef __NT__
  use_registry = 0;
#endif
}


initFile::~initFile()
{

}


void initFile::initialize()
{
  // Note: Using standard C file functions for compatibility with ATMos

  char* config_fname;

// Get filename:

#if defined(UnixArchitecture) || defined(NTArchitecture)

  char* tmp_fname;

  if ((tmp_fname = getenv(INIT_ENV_VAR)) == NULL)
    {
#if defined(UnixArchitecture)
      config_fname = strdup(CONFIG_DEFAULT_LOCATION);
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
	      // No values found:
	      noValsFound();
	    }
	  
	  use_registry = 1;
	}
#endif
    }
  else
    {
      config_fname = strdup(tmp_fname);
    }
  
#elif defined(ATMosArchitecture)
  
  config_fname = strdup("//isfs/omniORB.cfg");

#endif

#ifdef __NT__
  if (!use_registry) 
    {
#endif
      if (read_file(config_fname) < 0)
	{
	  // Can't open file.
	  // Don't throw exception, as information in file 
	  // may not be used - fail only when a call that requires
	  // the information is made.
	  
	  free(config_fname);
	  return;
	}
      free(config_fname);
#ifdef __NT__
    }
#endif

  // Get configuration data:

  char* entryname;
  char* data;
  int multcheck[INIT_MAX_CONFIG] = { 0 }; 
  
  while(getnextentry(entryname,data))
    {
      if (strcmp(entryname,"NAMESERVICE") == 0)
	{
	  if (multcheck[0] == 1) multerr(entryname);
	  else multcheck[0] = 1;
	  
	  omniObject* objptr;

	  try
	    {
	      objptr = omniORB::stringToObject(data);
	    }
	  catch(CORBA::MARSHAL& ex)
	    {
	      invref(entryname);
	    }

	  if (objptr)
	    {
	      if((objptr->_widenFromTheMostDerivedIntf(
				  CosNaming_NamingContext_IntfRepoID)) == 0)
		{
		  // The object reference supplied is not for the NamingService
		  invref(entryname);
		}    
	      else NameService = objptr;
	    }
	  else invref(entryname);

	}
      else
	{
	  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
	    cerr << "Configuration error:  Unknown field (" << entryname << ") "
		 << " found in configuration." << endl;
#else
	    kprintf("Configuration error:  ");
	    kprintf("Unknown field (%s) found in configuration file.\n",entryname);
#endif
	  }
	 throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
	}

      delete[] entryname;
      delete[] data;
    }
  
  delete[] fData;
  return;
}


int initFile::read_file(char* config_fname)
{
  // Read the configuration file:

  FILE* iFile;
  if ((iFile = fopen(config_fname,"r")) == NULL) return -1;

  fsize = 0;
  fseek(iFile,0L,SEEK_END);
  fsize = ftell(iFile);
  rewind(iFile);

  fData = new char[fsize+1];
  if (fData == NULL) throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);

  fread((void*) fData,1,fsize,iFile);
  fclose(iFile);
  return 0;
}



int initFile::getnextentry(char*& entryname, char*& data)
{
  // Get next entry in config file, and associated data

#ifdef __NT__
  // Use registry for NT if environment variable for config file not set:
  if (use_registry)
    return getRegistryEntry(entryname, data);
#endif

  static long currpos = 0;

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

  entryname = new char[(currpos-startpos) + 1];

  strncpy(entryname,(fData+startpos),(currpos-startpos));
  entryname[currpos-startpos] = '\0';


  // Skip whitespace between keyword and data:

  while  (isspace(fData[currpos]))
    {
      if(currpos == fsize) parseerr();
      else if (fData[currpos] == '\n') dataerr(entryname);
      currpos++;
    }

  // Read data:

  startpos = currpos;

  while (!isspace(fData[currpos]))
    {
      if (currpos == fsize) break;
      currpos++;
    }

  if (startpos == currpos) parseerr();

  data = new char[(currpos - startpos)+1];

  strncpy(data,(fData+startpos),(currpos-startpos));
  data[currpos-startpos] = '\0';

  return 1;
}



#ifdef __NT__

// NT member function to use registry:

int initFile::getRegistryEntry(char*& entryname, char*& data)
{
  static DWORD curr_index = 0;

  DWORD dataType;
  DWORD init_ValLen = init_maxValLen+1;
  DWORD init_DataLen = init_maxDataLen+1;
  entryname = new char[init_ValLen];
  data = new char[init_DataLen];

  if (RegEnumValue(init_hkey,curr_index++,(LPTSTR) entryname,&init_ValLen,NULL,
		   &dataType,(LPBYTE) data, &init_DataLen) != ERROR_SUCCESS)
    {
      delete[] entryname;
      delete[] data;
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


#ifdef __NT__

// NT-specific error reporting functions:

void initFile::noValsFound()
{
  if (omniORB::traceLevel > 0) {
    cerr << "Configuration error: No values found in registry key " 
	 << INIT_REGKEY << endl;
  }
  throw CORBA::INITIALIZE(0,CORBA::COMPLETED_NO);
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
