// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IOP.h                      Created on: 8/2/96
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
//       C++ mapping of the OMG IOP module
//       Reference: CORBA V2.0 10.6.2
//	
//	

/*
  $Log$
  Revision 1.2.2.1  2000/07/17 10:35:34  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/24 09:51:41  djr
  Moved from omniORB2 + some new files.

  Revision 1.7  1999/06/18 21:13:56  sll
  Updated copyright notice.

  Revision 1.6  1999/06/18 20:36:05  sll
  Replaced _LC_attr with _core_attr.

  Revision 1.5  1999/01/07 18:22:25  djr
  Replaced _OMNIORB_NTDLL_IMPORT with _LC_attr.

  Revision 1.4  1998/04/07 19:59:00  sll
   Replace _OMNIORB2_NTDLL_ specification on class IOP with
  _OMNIORB_NTDLL_IMPORT on static member constants.

  Revision 1.3  1997/12/09 20:36:28  sll
  Added TaggedProfileList_var class.

 * Revision 1.2  1997/05/06  16:07:17  sll
 * Public release.
 *
 */

#ifndef __OMNIORB_IOP_H__
#define __OMNIORB_IOP_H__


class IOP {
public:
  typedef _CORBA_ULong ProfileId;
  static _core_attr const   ProfileId    TAG_INTERNET_IOP;
  static _core_attr const   ProfileId    TAG_MULTIPLE_COMPONENTS;

  struct TaggedProfile {
    ProfileId tag;
    _CORBA_Unbounded_Sequence_Octet profile_data;

    // the following are omniORB2 private functions
    size_t _NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(ProfileId);
      return profile_data._NP_alignedSize(alignedsize);
    }
    void operator>>= (NetBufferedStream &s);
    void operator<<= (NetBufferedStream &s);
    void operator>>= (MemBufferedStream &s);
    void operator<<= (MemBufferedStream &s);
  };

  typedef _CORBA_Unbounded_Sequence<TaggedProfile> TaggedProfileList;

  class TaggedProfileList_var {
  public:
    TaggedProfileList_var() { _ptr = 0; }
    TaggedProfileList_var(TaggedProfileList* p) { _ptr = p; }
    TaggedProfileList_var(TaggedProfileList_var& p);
    ~TaggedProfileList_var() { if (_ptr) delete _ptr; }
    TaggedProfileList_var& operator= (const TaggedProfileList_var& p) {
      if (p._ptr) {
	if (!_ptr) {
	  _ptr = new TaggedProfileList;
	  if (!_ptr) {
	    _CORBA_new_operator_return_null();
	    // never reach here
	  }
	}
	*_ptr = *p._ptr;
      }
      else {
	if (_ptr) delete _ptr;
	_ptr = 0;
      }
      return *this;
    }
    TaggedProfileList_var& operator= (TaggedProfileList* p) {
      if (_ptr) delete _ptr;
      _ptr = p;
      return *this;
    }
    TaggedProfile& operator[] (_CORBA_ULong index) { 
      return (_ptr->NP_data())[index]; 
    }
    const TaggedProfile& operator[] (_CORBA_ULong index) const {
      return (_ptr->NP_data())[index];
    }
    inline TaggedProfileList* operator->() const { 
      return (TaggedProfileList*)_ptr; 
    }
    TaggedProfileList* _ptr;
  };

  struct IOR {
    _CORBA_Char       *type_id;
    TaggedProfileList  profiles;

    // the following are omniORB2 private functions
    size_t _NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      alignedsize += strlen((const char *)type_id) + 1;
      return profiles._NP_alignedSize(alignedsize);
    }
  };

  typedef _CORBA_ULong ComponentId;

  struct TaggedComponent {
    ComponentId	            tag;
    _CORBA_Unbounded_Sequence_Octet component_data;

    // the following are omniORB2 private functions
    size_t _NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      return component_data._NP_alignedSize(alignedsize);
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
    _CORBA_Unbounded_Sequence__Octet context_data;

    // the following are omniORB2 private functions
    size_t _NP_alignedSize(size_t initialoffset) {
      size_t alignedsize = ((initialoffset+3) & ~((int)3))
	     + sizeof(_CORBA_ULong);
      return context_data._NP_alignedSize(alignedsize);
    }
    void operator>>= (NetBufferedStream &s);
    void operator<<= (NetBufferedStream &s);
    void operator>>= (MemBufferedStream &s);
    void operator<<= (MemBufferedStream &s);
  };

  typedef _CORBA_Unbounded_Sequence<ServiceContext> ServiceContextList;

  static _core_attr const ServiceID TransactionService;


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


#endif // __OMNIORB_IOP_H__
