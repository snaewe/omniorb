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
  Revision 1.3  1997/01/22 14:33:51  ewc
  Small bug fix to file read.

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


#include <omniORB2/CORBA.h>


initFile::initFile()
{
NameService = NULL;
}


initFile::~initFile()
{

}


void initFile::initialize()
{
  // Note: Using standard C file functions for compatibility with ATMos

char* config_fname;

// Get filename:

#if defined(UnixArchitecture)

char* tmp_fname;

if ((tmp_fname = getenv("OMNIORB_CONFIG")) == NULL)
  {
    config_fname = strdup("/etc/omniORB.cfg");
  }
else
  {
    config_fname = strdup(tmp_fname);
  }

#elif defined(ATMosArchitecture)

config_fname = strdup("//isfs/omniORB.cfg");

#endif


// Read file:

FILE* iFile;
if ((iFile = fopen(config_fname,"r")) == NULL)
  {
    // Can't open file.
    // Don't throw exception, as information in file 
    // may not be used - fail only when a call that requires
    // the information is made.

    free(config_fname);
    return;
  }

free(config_fname);

fsize = 0;
fseek(iFile,0L,SEEK_END);
fsize = ftell(iFile);
rewind(iFile);

fData = new char[fsize+1];
if (fData == NULL)
  {
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  }

fread((void*) fData,1,fsize,iFile);
fclose(iFile);


// Get configuration data:

char* entryname;
char* data;
int multcheck[MAX_CONFIG] = { 0 }; 

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

#ifndef __atmos__
	cerr << "Configuration error:  Unknown field (" << entryname << ") "
	     << " found in configuration file." << endl;
#else
	kprintf("Configuration error:  ");
	kprintf("Unknown field (%s) found in configutation file.\n",entryname);
#endif
	exit(-1);
      }

    delete[] entryname;
    delete[] data;
  }

delete[] fData;
return;
}


int initFile::getnextentry(char*& entryname, char*& data)
{
  // Get next entry in config file, and associated data

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


// Implementations of inline error-report functions:

void initFile::multerr(char* entryname)
{
#ifndef __atmos__
cerr << "Configuration error: Multiple entries found for field "
     << entryname << " in configuration file." << endl;
#else
kprintf("Configuration error: ");
kprintf("Multiple entries found for field %s in configuration file.\n",
	                                                       entryname);
#endif

exit(-1);
}


void initFile::dataerr(char* entryname)
{
#ifndef __atmos__
cerr << "Configuration error: No data found for field "
     << entryname << " in configuration file." << endl;
#else
kprintf("Configuration error: No data found for field %s",entryname);
kprintf(" in configuration file.\n");
#endif

exit(-1);
}


void initFile::parseerr()
{
#ifndef __atmos__
cerr << "Configuration error: Parse error in config file." << endl;
#else
kprintf("Configuration error: Parse error in config file.\n");
#endif

exit(-1);
}


void initFile::invref(char* entryname)
{
#ifndef __atmos__
cerr << "Configuration error: Invalid object reference supplied for "
      << entryname << "." << endl;
#else
kprintf("Configuration error: Invalid object reference supplied for %s.\n",
	entryname);
#endif

exit(-1);
}

