// -*- Mode: C++; -*-
//                            Package   : omniORB
// initFile.h                 Created on: 10/01/97
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
  Revision 1.1.2.1  2001/02/23 16:47:03  sll
  Added new files.

  Revision 1.10.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.11  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.9.8.2  2000/05/24 17:21:07  dpg1
  Fix const-correctness in error functions

  Revision 1.9.8.1  1999/09/22 14:26:50  djr
  Major rewrite of orbcore to support POA.

  Revision 1.9  1999/03/11 16:25:53  djr
  Updated copyright notice

  Revision 1.8  1998/08/21 19:12:03  sll
  Removed public member NameService().

  Revision 1.7  1998/08/11 17:58:10  sll
  Do not use registry if the platform is Phar Lap ETS Kernel.

  Revision 1.6  1997/12/09 18:23:18  sll
  Updated interface to avoid memory leaks.

  Revision 1.5  1997/08/27 10:20:33  sll
  Moved from include/omniORB2. New private variables.

  Revision 1.4  1997/05/06 16:08:46  sll
  Public release.

  */


// Class to read and store constants from initialization file.
// Used for ATMos, UNIX and Windows NT.

#ifndef __INITFILE_H__
#define __INITFILE_H__

OMNI_NAMESPACE_BEGIN(omni)

class initFile
{
public:
  
  initFile();
  virtual ~initFile();

  void initialize();

private:

  char* fData;
  long fsize;
  long currpos;

  inline void invref(const char* entryname);
  inline void multerr(const char* entryname);      
  inline void dataerr(const char* entryname);
  inline void parseerr();
 
#ifdef __WIN32__ 
// WIN32 -specific error reporting functions:
  inline void noValsFound();
  inline void formaterr(char* entryname);
#endif

  int read_file(char* config_fname);
  int getnextentry(CORBA::String_var& entryname,CORBA::String_var& data);
 
#if defined(__WIN32__) && !defined(__ETS_KERNEL__)
  int use_registry;
 
  HKEY init_hkey;
  DWORD init_maxValLen;
  DWORD init_maxDataLen;
  DWORD curr_index;
  int getRegistryEntry(CORBA::String_var& entryname,
		       CORBA::String_var& data);
#endif

};

OMNI_NAMESPACE_END(omni)

#endif  
        // __INITFILE_H__
