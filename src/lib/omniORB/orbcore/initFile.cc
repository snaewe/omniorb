// -*- Mode: C++; -*-
//                            Package   : omniORB
// initFile.cc                Created on: 10/01/97
//                            Author    : Eoin Carroll  (ewc)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
  Revision 1.32  2000/07/04 15:22:57  dpg1
  Merge from omni3_develop.

  Revision 1.30.6.9  2000/06/28 13:20:33  sll
  Pre-release 3 updates

  Revision 1.30.6.8  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.30.6.7  2000/06/19 14:18:33  dpg1
  Explicit cast to (const char*) when using String_var with logger.

  Revision 1.30.6.6  2000/05/24 17:21:07  dpg1
  Fix const-correctness in error functions

  Revision 1.30.6.5  2000/04/27 10:49:33  dpg1
  Interoperable Naming Service

  Add ORBInitRef and ORBDefaultInitRef config file keys. Existing
  NAMESERVICE and INTERFACE_REPOSITORY keys deprecated and converted to
  work with new omniInitialReferences.

  Revision 1.30.6.4  1999/10/16 13:22:53  djr
  Changes to support compiling on MSVC.

  Revision 1.30.6.3  1999/10/14 16:22:10  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.30.6.2  1999/09/24 15:01:34  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.30.6.1  1999/09/22 14:26:50  djr
  Major rewrite of orbcore to support POA.

  Revision 1.30  1999/09/01 13:13:55  sll
  Fixed #ifdef macro so that the code compiles for ETS kernel.

  Revision 1.29  1999/08/16 19:24:08  sll
  Added a per-compilation unit initialiser.

  Revision 1.28  1999/06/18 20:54:00  sll
  Added include Naming.hh as it is no longer included by default when this
  runtime library is compiled.

  Revision 1.27  1999/05/25 17:00:31  sll
  Added new variable INTERFACE_REPOSITORY in the config file.

  Revision 1.26  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.25  1998/09/23 09:56:24  sll
  Close registry key on WIN32 in the dtor of initFile.

  Revision 1.24  1998/08/21 19:11:36  sll
  Now store the initial object reference to the NameService into
  omniInitialReferences::singleton().
  Recognise new keys: ORBInitialHost and ORBInitialPort. Call into
  omniInitialReferences::singleton() to register the host+port where the
  special bootstrapping agent can be located.

  Revision 1.23  1998/08/14 13:47:39  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.22  1998/08/11 18:14:24  sll
  Added support for Phar Lap ETS kernel. It supports win32 API but do not
  have registry.

  Revision 1.21  1998/04/18 10:10:16  sll
  Added support for Borland C++.

  Revision 1.20  1998/04/07 19:34:45  sll
  Replace cerr with omniORB::log.

// Revision 1.19  1998/03/09  11:33:57  ewc
// Changes for NextStep made.
//
// Revision 1.18  1998/01/20  17:32:01  sll
// Added support for OpenVMS.
//
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

#include <omniORB3/CORBA.h>
#include <omniORB3/Naming.hh>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <initFile.h>
#include <omniORB3/omniObjRef.h>
#include <initRefs.h>
#include <gatekeeper.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <corbaOrb.h>

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


#ifndef INIT_ENV_VAR
#define INIT_ENV_VAR "OMNIORB_CONFIG"    
                      // Environment variable that points to configuration file. 
#endif

#ifndef INIT_REGKEY
#define INIT_REGKEY  "SOFTWARE\\ORL\\omniORB\\2.0"
                     // Path to Registry Keys for omniORB [NT specific]
#endif


#define INIT_MAX_CONFIG 10   
                     // Maximum number of entries in configuration 


initFile::initFile() : fData(0), fsize(0), currpos(0)
{
#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
  use_registry = 0;
  curr_index = 0;
#endif
}


initFile::~initFile()
{
  if (fData) {
    delete [] fData;
  }
#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
  if (use_registry) {
    RegCloseKey(init_hkey);
  }
#endif
}


void initFile::initialize()
{
  // Note: Using standard C file functions for compatibility with ATMos

  CORBA::String_var config_fname;
  int nameServiceSet         = 0;
  int interfaceRepositorySet = 0;

// Get filename:

#if defined(UnixArchitecture) || defined(NTArchitecture) || defined(__VMS)

  char* tmp_fname;

  if ((tmp_fname = getenv(INIT_ENV_VAR)) == NULL)
    {
#if defined(UnixArchitecture) || defined(__VMS) || defined(__ETS_KERNEL__)
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

#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
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
#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
    }
#endif

  // Get configuration data:

  CORBA::String_var entryname;
  CORBA::String_var data;
  CORBA::String_var bootstrapAgentHostname;
  CORBA::UShort     bootstrapAgentPort = 900;

  while(getnextentry(entryname,data)) {

    if (strcmp((const char*)entryname,"ORBInitRef") == 0) {
      unsigned int slen = strlen(data) + 1;
      CORBA::String_var id  = CORBA::string_alloc(slen);
      CORBA::String_var uri = CORBA::string_alloc(slen);
      if (sscanf(data, "%[^=]=%s", (char*)id, (char*)uri) != 2) {
	if (omniORB::trace(1)) {
	  omniORB::logger l;
	  l << "Configuration error: invalid ORBInitRef parameter `"
	    << (const char*)data << "'.\n";
	}
	OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
      }
      if (!strcmp(id, "NameService")) {
	if (nameServiceSet) multerr("NameService");
	nameServiceSet = 1;
      }
      else if (!strcmp(id, "InterfaceRepository")) {
	if (interfaceRepositorySet) multerr("InterfaceRepository");
	interfaceRepositorySet = 1;
      }
      if (!omniInitialReferences::setFromFile(id, uri)) {
	if (omniORB::trace(1)) {
	  omniORB::logger l;
	  l << "Configuration error: syntactically incorrect URI `"
	    << (const char*)uri << "'\n";
	}
	OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
      }
    }
    else if (strcmp((const char*)entryname,"ORBDefaultInitRef") == 0) {
      omniInitialReferences::setDefaultInitRefFromFile(data);
    }
    else if (strcmp((const char*)entryname,"NAMESERVICE") == 0) {
      if (nameServiceSet) multerr(entryname);
      nameServiceSet = 1;
      if (!omniInitialReferences::setFromFile("NameService", data))
	invref("NameService");
    }
    else if (strcmp(entryname, "INTERFACE_REPOSITORY") == 0) {
      if (interfaceRepositorySet) multerr(entryname);
      interfaceRepositorySet = 1;
      if (!omniInitialReferences::setFromFile("InterfaceRepository", data))
	invref("InterfaceRepository");
    }
#ifdef _MSC_VER
    //??
    else if (strcmp(entryname, "GATEKEEPER_ALLOWFILE") == 0) {
      omniORB::logs(1, "WARNING -- gatekeeper disabled.");
    }
    else if (strcmp(entryname, "GATEKEEPER_DENYFILE") == 0) {
      omniORB::logs(1, "WARNING -- gatekeeper disabled.");
    }
#else
    else if (strcmp(entryname, "GATEKEEPER_ALLOWFILE") == 0) {
      gateKeeper::allowFile = CORBA::string_dup(data);	  
    }
    else if (strcmp(entryname, "GATEKEEPER_DENYFILE") == 0) {
      gateKeeper::denyFile = CORBA::string_dup(data);
    }
#endif
    else if (strcmp(entryname, "ORBInitialHost") == 0) {
      bootstrapAgentHostname = CORBA::string_dup(data);
    }
    else if (strcmp(entryname, "ORBInitialPort") == 0) {
      unsigned int port;
      if (sscanf(data,"%u",&port) != 1 ||
	  (port == 0 || port >= 65536)) {
	invref(entryname);
      }
      bootstrapAgentPort = (CORBA::UShort)port;
    }
    else {
      if (omniORB::traceLevel > 0) {
#ifndef __atmos__
	omniORB::log << "Configuration error:  Unknown field (" 
		     << (const char*) entryname << ") "
		     << " found in configuration.\n";
	omniORB::log.flush();
#else
	kprintf("Configuration error:  ");
	kprintf("Unknown field (%s) found in configuration file.\n",(const char*)entryname);
#endif
      }
      OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
    }
  }
  if (!nameServiceSet && !interfaceRepositorySet) {
    if ((char*)bootstrapAgentHostname != 0) {
      omniInitialReferences::initialise_bootstrap_agent(bootstrapAgentHostname,
							bootstrapAgentPort);
    }
  }
}


int initFile::read_file(char* config_fname)
{
  // Test if the specified file exists and is not a directory
#if defined(UnixArchitecture) || defined(__VMS) || defined(__nextstep__) || defined(__BCPLUSPLUS__)
  {
    struct stat stbuf;
#if defined(__nextstep__)
    if (stat(config_fname,&stbuf) < 0 || !(S_IFREG && stbuf.st_mode)) {
#else
    if (stat(config_fname,&stbuf) < 0 || !S_ISREG(stbuf.st_mode)) {
#endif
      if (omniORB::traceLevel >= 2) {
	omniORB::log << "omniORB configuration file: "
	      << config_fname << " either does not exist or is not a file.\n";
	omniORB::log.flush();
      }
      return -1;
    }
  }
#endif

#if defined(__WIN32__) && defined(_MSC_VER)
  {
    struct _stat stbuf;
    if (_stat(config_fname,&stbuf) != 0 || ! (_S_IFREG && stbuf.st_mode)) {
      if (omniORB::traceLevel >= 2) {
	omniORB::log << "omniORB configuration file: " << config_fname
		     << " either does not exist or is not a file.\n";
	omniORB::log.flush();
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
    OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);

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

#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
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

  entryname = omni::allocString(currpos-startpos);
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

  data = omni::allocString(currpos - startpos);
  strncpy(data,(fData+startpos),(currpos-startpos));
  ((char*)data)[currpos-startpos] = '\0';

  return 1;
}



#if defined(__WIN32__) && !defined(__ETS_KERNEL__)

// NT member function to use registry:

int initFile::getRegistryEntry(CORBA::String_var& entryname,
			       CORBA::String_var& data)
{
  DWORD dataType;
  DWORD init_ValLen = init_maxValLen+1;
  DWORD init_DataLen = init_maxDataLen+1;
  entryname = omni::allocString(init_ValLen);
  data      = omni::allocString(init_DataLen);

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

void initFile::multerr(const char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    omniORB::log <<
      "omniORB: Configuration error: Multiple entries found for field\n"
      " " << entryname << " .\n";
    omniORB::log.flush();
#else
    kprintf("Configuration error: ");
    kprintf("Multiple entries found for field %s in configuration file.\n",
	    entryname);
#endif
  }
  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
}


void initFile::dataerr(const char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    omniORB::log <<
      "omniORB: Configuration error: No data found for field " << entryname <<
      "\n"
      " in configuration file.\n";
    omniORB::log.flush();
#else
    kprintf("Configuration error: No data found for field %s",entryname);
    kprintf(" in configuration file.\n");
#endif
  }
  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
}


void initFile::parseerr()
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    omniORB::log <<
      "omniORB: Configuration error: Parse error in config file.\n";
    omniORB::log.flush();
#else
    kprintf("Configuration error: Parse error in config file.\n");
#endif
  }
  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
}


void initFile::invref(const char* entryname)
{
  if (omniORB::traceLevel > 0) {
#ifndef __atmos__
    omniORB::log << "omniORB: Configuration error: Invalid object reference "
      "supplied for " << entryname << ".\n";
    omniORB::log.flush();
#else
    kprintf("Configuration error: Invalid object reference supplied for %s.\n",
	    entryname);
#endif
  }
  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
}


#if defined(__WIN32__) && !defined(__ETS_KERNEL__)

// NT-specific error reporting functions:

void initFile::noValsFound()
{
  if (omniORB::traceLevel > 0) {
    omniORB::log <<
      "omniORB: Configuration error: No values found in registry key\n" <<
      " HKEY_LOCAL_MACHINE\\" << INIT_REGKEY << "\n"
      " Either set the environment variable OMNIORB_CONFIG to point\n"
      " to the omniORB configuration file, or enter the IOR for the\n"
      " naming service in to the registry in the (string) value\n"
      " NAMESERVICE , under the registry entry\n"
      " HKEY_LOCAL_MACHINE\\" << INIT_REGKEY << "\n";
    omniORB::log.flush();
  }
}


void initFile::formaterr(char* entryname)
{
  if (omniORB::traceLevel > 0) {
    omniORB::log <<
      "omniORB: Configuration error: Data for value " << entryname << "\n"
      " is not a character string.\n";
    omniORB::log.flush();
  }
  OMNIORB_THROW(INITIALIZE,0,CORBA::COMPLETED_NO);
}

#endif

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_initFile_initialiser : public omniInitialiser {
public:

  void attach() {
    initFile configFile;
    configFile.initialize();
  }

  void detach() {
  }
};

static omni_initFile_initialiser initialiser;

omniInitialiser& omni_initFile_initialiser_ = initialiser;
