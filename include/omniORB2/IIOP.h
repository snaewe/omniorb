// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IIOP.h                     Created on: 8/2/96
//                            Author    : Sai Lai Lo (sll)
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
//       C++ mapping of the OMG IIOP module
//       Reference: CORBA V2.0 12.8.2
//	
//	

/*
  $Log$
  Revision 1.10.4.1  1999/09/15 20:18:15  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.10  1999/06/18 21:13:24  sll
  Updted to copyright notice.

  Revision 1.9  1999/06/18 20:35:53  sll
  Replaced _LC_attr with _core_attr

  Revision 1.8  1999/01/07 18:20:05  djr
  Replaced _OMNIORB_NTDLL_IMPORT with _LC_attr.

  Revision 1.7  1998/04/07 19:57:16  sll
  Replace _OMNIORB2_NTDLL_ specification on class IIOP with
  _OMNIORB_NTDLL_IMPORT on static member constants.

  Revision 1.6  1997/12/09 20:39:38  sll
  Removed profileToEncapStream and EncapStreamToProfile.

  Revision 1.5  1997/08/21 22:21:19  sll
  ProfileBody now has a dtor to deallocate the storage for the host field.

 * Revision 1.4  1997/05/06  16:06:55  sll
 * Public release.
 *
 */

#ifndef __IIOP_H__
#define __IIOP_H__


class IIOP {
public:

  struct Version { 
    _CORBA_Char major;
    _CORBA_Char minor; 
  };

  struct ProfileBody {
    Version                 version;
    _CORBA_String_member    host;
    _CORBA_UShort           port;
    _CORBA_Unbounded_Sequence_Octet  object_key;
    IOP::MultipleComponentProfile    components;
  };

};


#endif // __IIOP_H__
