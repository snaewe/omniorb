// -*- Mode: C++; -*-
//                            Package   : omniORB
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
  Revision 1.2.2.6  2001/05/09 16:59:08  sll
  Added unmarshalObjectKey() to allow quick extraction of the object key.

  Revision 1.2.2.5  2001/04/18 17:52:46  sll
  Rationalise marshalling and unmarshalling routines.

  Revision 1.2.2.4  2000/11/15 17:01:59  sll
  Default ProfileBody ctor set components max to 2.

  Revision 1.2.2.3  2000/10/04 16:50:06  sll
  Updated header comment.

  Revision 1.2.2.2  2000/09/27 17:06:43  sll
  New helper functions to decode an IOR.

  Revision 1.2.2.1  2000/07/17 10:35:34  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.4  2000/05/25 08:45:54  dpg1
  Forgot _core_attr onn IIOP::DEFAULT_CORBALOC_PORT

  Revision 1.1.2.3  2000/05/24 17:10:57  dpg1
  Rename IIOP::DEFAULT_PORT IIOP::DEFAULT_CORBALOC_PORT

  Revision 1.1.2.2  2000/04/27 10:35:49  dpg1
  Interoperable Naming Service

  Added IIOP default port constant.

  Revision 1.1.2.1  1999/09/24 09:51:40  djr
  Moved from omniORB2 + some new files.

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

#ifndef __OMNIORB_IIOP_H__
#define __OMNIORB_IIOP_H__

class IIOP {
public:

  typedef GIOP::Version Version;

  struct Address {
    _CORBA_String_member    host;
    _CORBA_UShort           port;
  };

  struct ProfileBody {
    Version                 version;
    Address                 address;
    _CORBA_Unbounded_Sequence_Octet  object_key;
    IOP::MultipleComponentProfile    components;

    ProfileBody() : components(2) {}
  };

  typedef _CORBA_Unbounded_Sequence<ProfileBody> ProfileBodyList;

  static _core_attr const _CORBA_UShort DEFAULT_CORBALOC_PORT;

  static void encodeProfile(const ProfileBody&,IOP::TaggedProfile&);
  static void encodeMultiComponentProfile(const IOP::MultipleComponentProfile&,
					  IOP::TaggedProfile&);

  static void unmarshalProfile(const IOP::TaggedProfile&, ProfileBody&);
  static void unmarshalMultiComponentProfile(const IOP::TaggedProfile&,
					     IOP::MultipleComponentProfile&);

  static void unmarshalObjectKey(const IOP::TaggedProfile& p,
				 _CORBA_Unbounded_Sequence_Octet& key);
  // The input profile must be TAG_INTERNET_IOP
  // Extract the object key into <key>. The octet buffer inside <key> is
  // still own by <p>. So p must not be deallocated before key is.
};


#endif // __OMNIORB_IIOP_H__
