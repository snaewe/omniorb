// -*- Mode: C++; -*-
//                            Package   : omniORB2
// ropeFactory.h              Created on: 30/9/97
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
 $Log$
 Revision 1.7.4.1  1999/09/15 20:18:29  sll
 Updated to use the new cdrStream abstraction.
 Marshalling operators for NetBufferedStream and MemBufferedStream are now
 replaced with just one version for cdrStream.
 Derived class giopStream implements the cdrStream abstraction over a
 network connection whereas the cdrMemoryStream implements the abstraction
 with in memory buffer.

 Revision 1.7  1999/08/16 19:27:41  sll
 The ctor of ropeFactory_iterator now takes a pointer argument.

 Revision 1.6  1999/07/02 19:10:08  sll
 Removed inlined virtual destructors. Some compilers generate a copy of
 each destructor in each compilation unit.

 Revision 1.5  1999/03/11 16:25:55  djr
 Updated copyright notice

 Revision 1.4  1998/11/09 10:55:19  sll
 Removed the use of reserved keyword "export".

 Revision 1.3  1997/12/09 18:44:43  sll
 New and extended ropeFactory interface.

*/

#ifndef __ROPEFACTORY_H__
#define __ROPEFACTORY_H__

#ifndef __GIOPOBJECTINFO_H_EXTERNAL_GUARD__
#define __GIOPOBJECTINFO_H_EXTERNAL_GUARD__
#include <giopObjectInfo.h>
#endif

// Each derived ropeFactory implements a set of policies:
//
//   1. Determines the threading model to dispatch incoming requests.
//
//   2. Determines how many strands can be created concurrently per
//      incoming rope.
//
//   3. *Does not* determine the threading model of outgoing requests.
//      This is fixed by the upper layer- the implemantation of the GIOP_C
//      class. The current implemntation dictates that there can be only
//      one request outstanding per strand. In otherwords, each thread 
//      has exclusive access to a strand when it has a request outstanding.
//
//   3. Determines how many strands can be created concurrently per
//      outgoing rope.
//
//     

// NOTES
//  To add a new network transport to the ORB:
//    1. Define derived classes for the following abstract classes:
//           a) ropeFactoryType
//           b) ropeFactory
//           c) incomingRopeFactory
//           d) outgoingRopeFactory
//           e) Strand
//           f) Rope
//           g) Endpoint
//    2. If this transport is to be used automatically by the ORB to
//       create outgoing ropes, insert an instance of its outgoingRopeFactory
//       to globalRopeFactories (see below).
//    3. If this transport is to be used to create incoming ropes for a
//       omniObjectManager, instantiate one instance of the incomingRopeFactory
//       the objectManager's rope factory list. 
//       For an example, see corbaBOA.cc: BOAobjectManager.

class ropeFactoryList;
class ropeFactory_iterator;
class ropeFactory;
class ropeFactoryType;

// This is the list of rope factory types the ORB understands. 
// For instance, the ORB will use the ropeFactoryType::is_IOPprofileID() to
// match a IOP::ProfileId it finds in an IOR.
// Notice that a ropeFactoryType instance should be a singleton.
extern ropeFactoryType* ropeFactoryTypeList;

class ropeFactoryType {
public:
  virtual CORBA::Boolean is_IOPprofileId(IOP::ProfileId tag) const = 0;
  // returns TRUE (1) if this rope factory uses the <tag> to identify the
  // IOP profiles it creates in getIncomingIOPprofiles().
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual CORBA::Boolean is_protocol(const char* name) const = 0;
  // Returns TRUE (1) if this rope factory supports the protocol identified
  // by <name>.  Returns False (0) otherwise.
  // 
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual CORBA::Boolean decodeIOPprofile(const IOP::TaggedProfile& profile,
					  // return values:
					  Endpoint*&     addr,
					  GIOPObjectInfo* objectInfo)const = 0;
  // If the return value is TRUE (1), the IOP profile can be decoded by
  // this factory (i.e. is_IOPprofileId(profile.tag) returns TRUE). Its
  // content is returned in <addr> and <objectInfo>>. <addr> is
  // are heap allocated by this function and should be released by the caller.
  //
  // This function may raise a CORBA::MARSHALL or a CORBA::NO_MEMORY exception.
  //
  // This function is thread-safe.

  virtual void encodeIOPprofile(const Endpoint* addr,
				const CORBA::Octet* objkey,
				const size_t objkeysize,
				IOP::TaggedProfile& profile) const = 0;
  // Encode <addr>, <objkey> into a IOP profile.
  // <profile> is heap allocated by this function and should be released by
  // the caller.
  //
  // This function may raise a CORBA::NO_MEMORY exception.
  //
  // This function is thread-safe.

  friend class ropeFactory;

protected:

  ropeFactoryType() { next = ropeFactoryTypeList; ropeFactoryTypeList = this; }
  virtual ~ropeFactoryType();

  ropeFactoryType* next;
};

class ropeFactory {
public:
  virtual CORBA::Boolean isIncoming(Endpoint* addr) const = 0;
  // Returns TRUE (1) if the endpoint <addr> identifies one of the incoming
  // rope instantiated by this factory.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.
  //
  // Concurrency Control:
  //      MUTEX = pd_anchor.pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX>
  // Post-condition:
  //      Does not hold <MUTEX>


  virtual CORBA::Boolean isOutgoing(Endpoint* addr) const = 0;
  // Returns TRUE (1) if the endpoint <addr> identifies one of the outgoing
  // rope instantiated by this factory.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.
  //
  // Concurrency Control:
  //      MUTEX = pd_anchor.pd_lock
  // Pre-condition:
  //      Does not hold <MUTEX>
  // Post-condition:
  //      Does not hold <MUTEX>

  virtual const ropeFactoryType* getType() const = 0;

  Anchor* anchor() { return &pd_anchor; }
  
  friend class ropeFactory_iterator;
  friend class ropeFactoryList;

  ropeFactory() {}
  virtual ~ropeFactory();

  // To iterate through all the ropes instantiated by this factory, use the
  // Rope_iterator() (defined in rope.h) and pass this factory instance to
  // its ctor.

  static omniObject* iopProfilesToRope(GIOPObjectInfo* objectInfo);

  // Look at the IOP tagged profile list <profiles>, returns the most
  // most suitable Rope to talk to the object and its object key.
  // If no suitable Rope can be found, throw an exception.
  // If the object is in fact a local object, return the object as well.
  // Otherwise, return a nil (0) pointer. The caller should use <objkey>,
  // <keysize> and <rope> to create a proxy object.
  // The returned value <objkey> is heap allocated by this function and
  // should be freed by the caller. The reference count of <rope> is
  // incremented. 

protected:
  Anchor       pd_anchor;
  ropeFactory* pd_next;

};

class incomingRopeFactory : public ropeFactory {
public:

  virtual CORBA::Boolean isIncoming(Endpoint* addr) const = 0;
  // Returns TRUE (1) if the endpoint <addr> identifies one of the incoming
  // rope instantiated by this factory.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual CORBA::Boolean isOutgoing(Endpoint* addr) const { return 0; }

  virtual void instantiateIncoming(Endpoint* addr,
				   CORBA::Boolean exportflag) = 0;
  // Create an incoming rope to receive on the endpoint <addr>.
  // The endpoint type must be supported by this factory. Otherwise, a
  // omniORB::fatalException is raised.
  // If startIncoming() has already been called on this factory, incoming
  // requests on this rope will be served by the factory immediately.
  //
  // The value of <exportflag> determines whether this endpoint is added to
  // the profile list returned by getIncomingIopProfiles() (see below).
  // If <exportflag> is TRUE (1), the endpoint is added to the list.
  //
  // This function may raise a CORBA::SystemException.
  //
  // This function is thread-safe.

  virtual void startIncoming() = 0;
  // When this function returns, this factory will service
  // requests from its incoming ropes.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual void stopIncoming() = 0;
  // When this function returns, this rope factory will no longer serve
  // the requests from its incoming ropes.
  // Existing strands would be shutdown. 
  // However, the ropes will stay.
  // This factory will serve incoming requests again when startIncoming()
  // is called.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual void removeIncoming() = 0;
  // When this function returns, all incoming ropes created
  // via instantiateIncoming() would be removed.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual Rope* findIncoming(Endpoint* addr) const = 0;
  // Search all the incoming ropes instantiated by all the rope factories
  // derived from this class. Returns the rope that matches <addr>.  If no
  // rope matches the endpoint, return 0.
  // The reference count of the rope returned will be increased by 1.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual void getIncomingIOPprofiles(const CORBA::Octet*     objkey,
				      const size_t            objkeysize,
				      IOP::TaggedProfileList& profilelist) const = 0;
  // Append the IOP profiles for the incoming ropes instantiated by this
  // factory to <profilelist>. The supplied object key is inserted into
  // the new IOP profiles.
  //
  // This function may raise a CORBA::SystemException.
  //
  // This function is thread-safe.

  incomingRopeFactory() {}
  virtual ~incomingRopeFactory();

};


class outgoingRopeFactory : public ropeFactory {
public:

  virtual CORBA::Boolean isIncoming(Endpoint* addr) const { return 0; }

  virtual CORBA::Boolean isOutgoing(Endpoint* addr) const = 0;
  // Returns TRUE (1) if the endpoint <addr> identifies one of the outgoing
  // rope instantiated by this factory.
  //
  // This function does not raise an exception.
  //
  // This function is thread-safe.

  virtual Rope*  findOrCreateOutgoing(Endpoint* addr) = 0;
  // If <addr> is not the endpoint type supported by this factory, return 0.
  // else
  //     search all outgoing ropes instantiated by this
  //     factory. Returns the rope that matches <addr>.
  //     If no rope matches the endpoint, instantiate a new outgoing rope 
  //     to connect to that endpoint.
  //     The reference count of the rope returned will be increased by 1.
  //
  // This function may raise a CORBA::SystemException.
  //
  // This function is thread-safe.

  outgoingRopeFactory() {}
  virtual ~outgoingRopeFactory();
};

class ropeFactoryList {
public:
  ropeFactoryList() : pd_head(0) {}
  virtual ~ropeFactoryList();

  virtual void insert(ropeFactory* p) { p->pd_next = pd_head; pd_head = p; }

  friend class ropeFactory_iterator;
private:
  virtual void lock() {}
  virtual void unlock() {}
  ropeFactory* pd_head;
};


class ropeFactoryList_ThreadSafe : public ropeFactoryList {
public:
  ropeFactoryList_ThreadSafe() {}
  virtual ~ropeFactoryList_ThreadSafe();

  virtual void insert(ropeFactory* p) { 
    omni_mutex_lock sync(pd_lock);
    ropeFactoryList::insert(p);
  }
  

private:
  virtual void lock() { pd_lock.lock(); }
  virtual void unlock() { pd_lock.unlock(); }
  omni_mutex pd_lock;
};

class ropeFactory_iterator {
public:
  ropeFactory_iterator(ropeFactoryList* l) : pd_l(*l) { 
    pd_l.lock(); 
    pd_this = pd_l.pd_head; 
  }
  virtual ~ropeFactory_iterator();
  const ropeFactory* operator() () {
    ropeFactory* p = pd_this;
    if (pd_this)
      pd_this = pd_this->pd_next;
    return p;
  }
private:
  ropeFactoryList& pd_l;
  ropeFactory* pd_this;
};


// This is the list of rope factories that can be used by the ORB to
// make outgoing ropes. To make the ORB recognise and use a new rope
// factory instance, the instance must be registered using:
//   globalRopeFactories.insert().
extern ropeFactoryList* globalOutgoingRopeFactories;

#endif // __ROPEFACTORY_H__
