// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initFile.h                 Created on: 10/01/97
//                            Author    : Eoin Carroll  (ewc)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.3  1997/03/04 11:08:25  ewc
  Added support for obtaining initial object reference from Windows NT
  registry

 * Revision 1.2  1997/01/21  15:03:00  ewc
 * Minor change - moved #define from source file to header.
 *
  */


// Class to read and store constants from initialization file.
// Used for ATMos, UNIX and Windows NT.

#ifndef __INITFILE_H__
#define __INITFILE_H__

class initFile
{
public:
  
  initFile();
  virtual ~initFile();

  void initialize();

private:
  char* fData;
  long fsize;

  inline void invref(char* entryname);
  inline void multerr(char* entryname);      
  inline void dataerr(char* entryname);
  inline void parseerr();
 
#ifdef __NT__ 
// NT-specific error reporting functions:
  inline void noValsFound();
  inline void formaterr(char* entryname);
#endif

  int read_file(char* config_fname);
  int getnextentry(char*& entryname, char*& data);
 
#ifdef __NT__
  int use_registry;
 
  HKEY init_hkey;
  DWORD init_maxValLen;
  DWORD init_maxDataLen;

  int getRegistryEntry(char*& entryname, char*& data);
#endif

public:
  
  omniObject* NameService;
};


#endif  
        // __INITFILE_H__
