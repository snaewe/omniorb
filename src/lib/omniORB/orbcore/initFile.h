// -*- Mode: C++; -*-
//                            Package   : omniORB2
// initFile.h                 Created on: 10/01/97
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
  Revision 1.4  1997/05/06 16:08:46  sll
  Public release.

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
