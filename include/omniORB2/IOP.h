// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IOP.h                      Created on: 8/2/96
//                            Author    : Sai Lai Lo (sll)
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
//       C++ mapping of the OMG IOP module
//       Reference: CORBA V2.0 10.6.2
//	
//	

/*
  $Log$
  Revision 1.2  1997/05/06 16:07:17  sll
  Public release.

 */

#ifndef __IOP_H__
#define __IOP_H__

class _OMNIORB2_NTDLL_ IOP {
public:
  typedef _CORBA_ULong ProfileId;
  static const   ProfileId    TAG_INTERNET_IOP;
  static const   ProfileId    TAG_MULTIPLE_COMPONENTS;

  struct TaggedProfile {
    ProfileId tag;
    _CORBA_Unbounded_Sequence_Octet profile_data;

    // the following are omniORB2 private functions
    size_t NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(ProfileId);
      return profile_data.NP_alignedSize(alignedsize);
    }
    void operator>>= (NetBufferedStream &s);
    void operator<<= (NetBufferedStream &s);
    void operator>>= (MemBufferedStream &s);
    void operator<<= (MemBufferedStream &s);

  };

  typedef _CORBA_Unbounded_Sequence<TaggedProfile> TaggedProfileList;

  struct IOR {
    _CORBA_Char       *type_id;
    TaggedProfileList  profiles;

    // the following are omniORB2 private functions
    size_t NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      alignedsize += strlen((const char *)type_id) + 1;
      return profiles.NP_alignedSize(alignedsize);
    }
  };

  typedef _CORBA_ULong ComponentId;

  struct TaggedComponent {
    ComponentId	            tag;
    _CORBA_Unbounded_Sequence_Octet component_data;

    // the following are omniORB2 private functions
    size_t NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      return component_data.NP_alignedSize(alignedsize);
    }
    void operator>>= (NetBufferedStream &s);
    void operator<<= (NetBufferedStream &s);
    void operator>>= (MemBufferedStream &s);
    void operator<<= (MemBufferedStream &s);
  };
  
  typedef _CORBA_Unbounded_Sequence<TaggedComponent> MultipleComponentProfile;

  typedef _CORBA_ULong ServiceID;

  struct ServiceContext {
    ServiceID              context_id;
    _CORBA_Unbounded_Sequence_Octet context_data;

    // the following are omniORB2 private functions
    size_t NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      return context_data.NP_alignedSize(alignedsize);
    }
    void operator>>= (NetBufferedStream &s);
    void operator<<= (NetBufferedStream &s);
    void operator>>= (MemBufferedStream &s);
    void operator<<= (MemBufferedStream &s);
  };

  typedef _CORBA_Unbounded_Sequence<ServiceContext> ServiceContextList;

  static const ServiceID TransactionService;


  // omniORB2 private function
  static _CORBA_Char * iorToEncapStr(const _CORBA_Char *type_id,
				     const TaggedProfileList *profiles);
  // returns a heap allocated and stringified IOR representation
  // (ref CORBA 2 spec. 10.6.5)

  // omniORB2 private function
  static void EncapStrToIor(const _CORBA_Char *str,
			    _CORBA_Char *&type_id,
			    TaggedProfileList *&profiles);
  // returns the type id and the tagged profile list encapsulated in
  // the stringified IOR representation. Both return values are heap
  // allocated.
  // (ref CORBA 2 spec. 10.6.5)

};

#endif // __IOP_H__
