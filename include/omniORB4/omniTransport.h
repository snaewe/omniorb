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
  Revision 1.1.4.1  2001/04/18 17:26:28  sll
  Big checkin with the brand new internal APIs.

  */

#ifndef __OMNITRANSPORT_H__
#define __OMNITRANSPORT_H__

class omniCallDescriptor;

OMNI_NAMESPACE_BEGIN(omni)

// A Rope is an abstraction through which a client can connect to a
// remote address space.
//
// A rope creates network connections to the remote address on demand.
// At any time, there can be 0 to n number of network connections associated
// with each rope.
//
// Each network connection is represented by a Strand.

extern omni_tracedmutex* omniTransportLock;

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

  StrandList() : next(this), prev(this) {}

  void insert(StrandList& head);
  void remove();
  static _CORBA_Boolean is_empty(StrandList& head);

private:
  StrandList(const StrandList&);
  StrandList& operator=(const StrandList&);
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


  class Link {
  public:
    Link* next;
    Link* prev;

    Link() : next(this), prev(this) {}

    void insert(Link& head);
    void remove();
    static _CORBA_Boolean is_empty(Link& head);

  private:
    Link(const Link&);
    Link& operator=(const Link&);
  };

  friend class Strand;

protected:
  Link pd_strands; // this is a list of strands that connects to the same
                   // remote address space.

private:
  Rope(const Rope&);
  Rope& operator=(const Rope&);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Strand : public Rope::Link, public StrandList {
public:

  Strand() {}
  virtual ~Strand() {}

private:
  Strand(const Strand&);
  Strand& operator=(const Strand&);
};

OMNI_NAMESPACE_END(omni)

#endif // __ROPE_H__
