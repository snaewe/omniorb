// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniTransport.h            Created on: 05/01/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.6.1  2003/03/23 21:04:08  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.4  2002/03/27 11:44:51  dpg1
  Check in interceptors things left over from last week.

  Revision 1.1.4.3  2001/08/03 17:48:43  sll
  Make sure dll import spec for win32 is properly done.

  Revision 1.1.4.2  2001/06/13 20:07:25  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.4.1  2001/04/18 17:26:28  sll
  Big checkin with the brand new internal APIs.

  */

#ifndef __OMNITRANSPORT_H__
#define __OMNITRANSPORT_H__

class omniCallDescriptor;

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

// A Rope is an abstraction through which a client can connect to a
// remote address space.
//
// A rope creates network connections to the remote address on demand.
// At any time, there can be 0 to n number of network connections associated
// with each rope.
//
// Each network connection is represented by a Strand.

extern _core_attr omni_tracedmutex* omniTransportLock;

class IOP_C;
class IOP_S;
class Rope;
class Strand;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class StrandList {
public:
  StrandList* next;
  StrandList* prev;

  StrandList() {
		next = this;
		prev = this;
	}

  void insert(StrandList& head);
  void remove();
  static _CORBA_Boolean is_empty(StrandList& head);

private:
  StrandList(const StrandList&);
  StrandList& operator=(const StrandList&);
};


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class RopeLink {
public:
  RopeLink* next;
  RopeLink* prev;

  RopeLink() {
		next = this;
		prev = this;
	}

  void insert(RopeLink& head);
  void remove();
  static _CORBA_Boolean is_empty(RopeLink& head);

private:
  RopeLink(const RopeLink&);
  RopeLink& operator=(const RopeLink&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Rope {
public:

  Rope() {}

  virtual ~Rope() {}

  virtual IOP_C* acquireClient(const omniIOR*,
			       const _CORBA_Octet*,_CORBA_ULong,
			       omniCallDescriptor*) = 0;
  virtual void releaseClient(IOP_C*) = 0;

  virtual void incrRefCount() = 0;
  virtual void decrRefCount() = 0;



  friend class Strand;

protected:
  RopeLink pd_strands; // this is a list of strands that connects to the same
                       // remote address space.

private:
  Rope(const Rope&);
  Rope& operator=(const Rope&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Strand : public RopeLink, public StrandList {
public:

  Strand() {}
  virtual ~Strand() {}

private:
  Strand(const Strand&);
  Strand& operator=(const Strand&);
};

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __OMNITRANSPORT_H__
