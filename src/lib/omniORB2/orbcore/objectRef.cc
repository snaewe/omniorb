// -*- Mode: C++; -*-
//                            Package   : omniORB2
// objectRef.cc               Created on: 20/5/96
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
  Revision 1.28.4.3  1999/10/05 20:35:35  sll
  Added support to GIOP 1.2 to recognise all TargetAddress mode.
  Now handles NEEDS_ADDRESSING_MODE and LOC_NEEDS_ADDRESSING_MODE.

  Revision 1.28.4.2  1999/10/02 18:21:29  sll
  Added support to decode optional tagged components in the IIOP profile.
  Added support to negogiate with a firewall proxy- GIOPProxy to invoke
  remote objects inside a firewall.
  Added tagged component TAG_ORB_TYPE to identify omniORB IORs.

  Revision 1.28.4.1  1999/09/15 20:18:30  sll
  Updated to use the new cdrStream abstraction.
  Marshalling operators for NetBufferedStream and MemBufferedStream are now
  replaced with just one version for cdrStream.
  Derived class giopStream implements the cdrStream abstraction over a
  network connection whereas the cdrMemoryStream implements the abstraction
  with in memory buffer.

  Revision 1.28  1999/08/30 17:10:24  sll
  omniObject::wrappedObjectTable initialiser fixed for MSVC++.

  Revision 1.27  1999/08/16 19:26:36  sll
  Replace static variable dtor with initialiser object to enumerate the
  list of remaining proxy objects on shutdown.
  This new scheme avoids the problem that dtor of static variables on
  different compilation units may be called in different order.

  Revision 1.26  1999/08/14 16:39:47  sll
  Changed locateObject. It does not throw an exception when an object is
  not found. Instead a nil pointer is returned.
  New locateObject for searching the python object table.

  Revision 1.25  1999/06/02 16:57:07  sll
  Changed createObjRef(). In the case when the proxyObjectFactories for
  the targetRepoId and the mostDerivedRepoId are not available, we now create
  an AnonymousObject. Previously, we throw a Marshal exception. Any attempt
  to narrow the returned object to the target interface identified by
  targetRepoId would results in a nil object being returned.

  Revision 1.24  1999/06/02 16:41:19  sll
  Moved class proxyObjectFactory into the CORBA namespace.

  Revision 1.23  1999/03/11 16:25:55  djr
  Updated copyright notice

  Revision 1.22  1999/02/01 15:40:53  djr
  Replace copy-initialisation of _var types with direct initialisation.

  Revision 1.21  1999/01/11 09:55:53  djr
  *** empty log message ***

  Revision 1.20  1999/01/07 16:31:43  djr
  Fixed memory leak in CORBA::UnMarshalObjRef().
  New singleton ProxyObjectTableCleaner, which releases proxy objects
  from the table when the program finishes. This allows programs such
  as purify to detect the leak. Also a message is logged if the
  traceLevel >= 15.

  Revision 1.19  1998/08/26 11:14:42  sll
  Minor updates to remove warnings when compiled with standard C++ compilers.

  Revision 1.18  1998/08/19 16:08:15  sll
  using omniORB::operator== now applies to all platforms that uses C++
  namespace.

  Revision 1.17  1998/08/15 14:31:41  sll
  Added using omniORB::operator== when the compiler supports namespace but
  no the Koenig Lookup rule.

  Revision 1.16  1998/08/14 13:50:28  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.15  1998/04/07 19:36:50  sll
  Replace cerr with omniORB::log.

// Revision 1.14  1998/01/27  15:35:55  ewc
// Added support for type any
//
  Revision 1.13  1997/12/10 13:23:41  sll
  Removed dependency on omniLC.h.

  Revision 1.12  1997/12/10 11:39:19  sll
  Updated life cycle runtime.

  Revision 1.11  1997/12/09 17:17:59  sll
  Updated to use the rope factory interfaces.
  Full support for late binding.
 
  Revision 1.9  1997/08/21 21:54:03  sll
  - fixed bug in disposeObject.
  - MarshalObjRef now always encode the repository ID of the most derived
    interface of the object reference.

  Revision 1.8  1997/08/08 09:34:57  sll
  MarshalObjRef now always pass the repository ID of the most derived type
  in the IOR.

  Revision 1.7  1997/05/06 15:26:37  sll
  Public release.

  */

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <objectManager.h>

#if defined(HAS_Cplusplus_Namespace)
using omniORB::operator==;
#endif

// The points to note are:
//
// 1. The interface information is provided by the stubs via the
//    proxyObjectFactory class. For an interface type A, the stub of A contains
//    a A_proxyObjectFactory class. This class is derived from the 
//    proxyObjectFactory class. The proxyObjectFactory is an abstract class
//    which contains 3 virtual functions that any derived classes, e.g.
//    A_proxyObjectFactory, have to implement. The functions allow the
//    runtime to query the IRid of the interface, to create a proxy object of 
//    the interface and to query whether a given IRid is a base interface.
//    Exactly one instance of A_proxyObjectFactory is declared as a local
//    constant of the stub. The instance is instantiated at runtime before the
//    main() is called. The ctor of proxyObjectFactory links the instance
//    to the chain headed by the local variable proxyStubs in this module.
//
// 2. To unmarshal an IOR, the runtime has two pieces of information in hand:
//       a. The repository ID field in the IOR which identifies the exact
//          type of the object
//       b. The type of the object reference this IOR is supposed to be. This
//          is determined by the operation signature in the IDL and is
//          identified by a "target" repository ID.
//    The runtime matches the repository ID in the IOR to those of the 
//    proxyFactories linked into the executable:
//
//       a. There is an exact match, all is well because the runtime can use
//          the proxyFactory to check if the target object reference type
//          is indeed identical or is the based interface of the
//          object. A proxy object can then be instantiated.
//
//       b. No match. This means that the runtime has no knowledge of the 
//          interface. An example of how this can happen is as follows:
//
//               Suppose A,B are interface types, and B inherits from A.
//               If only the stubs of A is linked into the
//               executable, the runtime would have no knowledge of B. 
//               If the IOR identifies itself as B and the target type is A,
//               the runtime cannot deduce the inheritance relation of B and A.
//
//          To sort this out, the runtime gives the IOR the benefits of
//          doubts and use the proxyFactory of the target type to instantiate
//          a proxy object. However, just before the proxy object is used
//          for the first time to perform a remote invocation, the
//          object is contacted to find out if it indeed supports the target
//          interface.


omni_mutex          omniObject::objectTableLock;
omniObject*         omniObject::proxyObjectTable = 0;
omniObject**        omniObject::localObjectTable = 0;
omniObject**        omniObject::localPyObjectTable = 0;
omni_mutex          omniObject::wrappedObjectTableLock;
void**              omniObject::wrappedObjectTable = 0;
CORBA::proxyObjectFactory* CORBA::proxyObjectFactory::proxyStubs = 0;


CORBA::proxyObjectFactory::proxyObjectFactory()
{
  pd_next = CORBA::proxyObjectFactory::proxyStubs;
  proxyStubs = this;
  return;
}

CORBA::proxyObjectFactory::~proxyObjectFactory()
{
}

CORBA::proxyObjectFactory_iterator::proxyObjectFactory_iterator()
{
  pd_f = CORBA::proxyObjectFactory::proxyStubs;
}

CORBA::proxyObjectFactory *
CORBA::proxyObjectFactory_iterator::operator() ()
{
  proxyObjectFactory *p = pd_f;
  if (pd_f)
    pd_f = pd_f->pd_next;
  return p;
}

// An AnonymousObject is used as a proxyObject when no proxyObjectFactory
// class for a give interface repository ID is found.
// Of course, one can only use such an object as a CORBA::Object_ptr and
// passes it around as the type "Object" in IDL operations and attributes.
// See also the comments in omni::createObjRef().
class AnonymousObject : public virtual omniObject,
			public virtual CORBA::Object 
{
public:
  AnonymousObject(GIOPObjectInfo* objInfo,const char* useAs) 
    : omniObject(objInfo,useAs) 
  {
    this->PR_setobj(this);
    omni::objectIsReady(this);
  }
  virtual ~AnonymousObject() {}
  
protected:
  virtual void* _widenFromTheMostDerivedIntf(const char* repoId,
					     CORBA::Boolean is_cxx_type_id);

private:
  AnonymousObject();
  AnonymousObject (const AnonymousObject&);
  AnonymousObject &operator=(const AnonymousObject&);
};

void*
AnonymousObject::_widenFromTheMostDerivedIntf(const char* repoId,
					      CORBA::Boolean is_cxx_type_id)
{
  if (is_cxx_type_id)
    return 0;
  if (!repoId)
    return (void*)((CORBA::Object_ptr)this);
  else
    return 0;
}


void
omni::objectIsReady(omniObject* obj)
{
  omniObject::objectTableLock.lock();
  if (obj->getRefCount() != 0) {
    omniObject::objectTableLock.unlock();
    throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
  }
    
  if (obj->is_proxy())
    {
      obj->pd_next = omniObject::proxyObjectTable;
      omniObject::proxyObjectTable = obj;
    }
  else
    {
      omniObject **p = &omniObject::localObjectTable[omniORB::hash(obj->pd_data.l.pd_key)];
      omniObject **pp = p;
      while (*p) {
	if ((*p)->pd_data.l.pd_key == obj->pd_data.l.pd_key) {
	  obj->pd_next = 0;
	  omniObject::objectTableLock.unlock();
	  throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
	}
	p = &((*p)->pd_next);
      }
      obj->pd_next = (*pp);
      *pp = obj;
    }
  obj->setRefCount(obj->getRefCount()+1);
  omniObject::objectTableLock.unlock();
  return;
}


void
omni::objectDuplicate(omniObject *obj)
{
  omniObject::objectTableLock.lock();
  if (obj->getRefCount() <= 0) {
    omniObject::objectTableLock.unlock();
    throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
  }
  obj->setRefCount(obj->getRefCount()+1);
  omniObject::objectTableLock.unlock();
  return;
}

void
omni::objectRelease(omniObject *obj)
{
  omniObject::objectTableLock.lock();

  if (obj->getRefCount() <= 0) {
    omniObject::objectTableLock.unlock();
    // This is most likely to be caused by the application code calling
    // CORBA::release() twice on an object reference.
    // 
    // One would like to throw a CORBA::INV_OBJREF exception at this stage
    // but the CORBA spec. says release *must not* throw CORBA exceptions.
    // Therefore, just generate a warning message and returns.
    if( omniORB::traceLevel > 0 ) {
      omniORB::log <<
	"omniORB: WARNING - try to release an object with reference count"
	" <= 0.\n"
	"Has CORBA::release() been called more than once on an object"
	" reference?\n";
      omniORB::log.flush();
    }
    return;
  }
  obj->setRefCount(obj->getRefCount()-1);
  if (obj->getRefCount() == 0) {
    if (obj->is_proxy()) {
      omniObject **p = &omniObject::proxyObjectTable;
      while (*p) {
	if (*p == obj) {
	  *p = obj->pd_next;
	  break;
	}
	p = &((*p)->pd_next);
      }
      omniObject::objectTableLock.unlock();
      delete obj;
    }
    else {
      omniObject **p;
      p = &omniObject::localObjectTable[omniORB::hash(obj->pd_data.l.pd_key)];
      while (*p) {
	if (*p == obj) {
	  *p = obj->pd_next;
	  break;
	}
	p = &((*p)->pd_next);
      }
      p = &omniObject::localPyObjectTable[omniORB::hash(obj->pd_data.l.pd_key)];
      while (*p) {
	if (*p == obj) {
	  *p = obj->pd_next;
	  break;
	}
	p = &((*p)->pd_next);
      }
      if (obj->pd_flags.disposed) {
	omniObject::objectTableLock.unlock();
	delete obj;   // call dtor if BOA->disposed() has been called.
      }
      else {
	omniObject::objectTableLock.unlock();
      }
    }
  }
  else
    omniObject::objectTableLock.unlock();
  return;
}

void
omni::disposeObject(omniObject *obj)
{
  if (obj->is_proxy())
    return;
  omniObject::objectTableLock.lock();
  if (obj->getRefCount() <= 0) {
    omniObject::objectTableLock.unlock();
    throw CORBA::INV_OBJREF(0,CORBA::COMPLETED_NO);
  }
  else
    obj->setRefCount(obj->getRefCount()-1);

  if (obj->getRefCount() == 0) {
    // object has _NOT_ already been removed from the object table
    omniObject **p;
    p = &omniObject::localObjectTable[omniORB::hash(obj->pd_data.l.pd_key)];
    while (*p) {
      if (*p == obj) {
	*p = obj->pd_next;
	break;
      }
      p = &((*p)->pd_next);
    }
    p = &omniObject::localPyObjectTable[omniORB::hash(obj->pd_data.l.pd_key)];
    while (*p) {
      if (*p == obj) {
	*p = obj->pd_next;
	break;
      }
      p = &((*p)->pd_next);
    }
    omniObject::objectTableLock.unlock();
    delete obj;
  }
  else {
    obj->pd_flags.disposed = 1;
    omniObject::objectTableLock.unlock();
  }
  return;
}

omniObject *
omni::locateObject(omniObjectManager*,omniObjectKey &k)
{
  omniObject::objectTableLock.lock();
  omniObject **p = &omniObject::localObjectTable[omniORB::hash(k)];
  while (*p) {
    if ((*p)->pd_data.l.pd_key == k) {
      (*p)->setRefCount((*p)->getRefCount()+1);
      omniObject::objectTableLock.unlock();
      return *p;
    }
    p = &((*p)->pd_next);
  }
  omniObject::objectTableLock.unlock();
  return 0;
}


omniObject *
omni::locatePyObject(omniObjectManager*,omniObjectKey &k)
{
  omniObject::objectTableLock.lock();
  omniObject **p = &omniObject::localPyObjectTable[omniORB::hash(k)];
  while (*p) {
    if ((*p)->pd_data.l.pd_key == k) {
      (*p)->setRefCount((*p)->getRefCount()+1);
      omniObject::objectTableLock.unlock();
      return *p;
    }
    p = &((*p)->pd_next);
  }
  omniObject::objectTableLock.unlock();
  return 0;
}


omniObject*
omni::createObjRef(const char* mostDerivedRepoId,
		   const char* targetRepoId,
		   IOP::TaggedProfileList* profiles)
{
  CORBA::Octet *objkey = 0;

  CORBA::proxyObjectFactory *p;
  {
    CORBA::proxyObjectFactory_iterator pnext;
    while ((p = pnext())) {
      if (strcmp(p->irRepoId(),mostDerivedRepoId) == 0) {
	if (!targetRepoId) {
	  // Target is just the pseudo object CORBA::Object
	  break;  // got it
	}
	else if (!p->is_a(targetRepoId)) {
	  // Object ref is neither the exact interface nor a derived 
	  // interface of the one requested.
	  delete profiles;
	  throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	}
	else {
	  break;  // got it
	}
      }
    }
  }

  // Once we reach here:
  // if (p != 0)
  //     p points to the proxy factory that is an exact match to the
  //     interface identified by <mostDerivedRepoId> and it has been
  //     verified that the interface identified by <targetRepoId> is
  //     equal or is a base class of <mostDerivedRepoId>.
  // else
  //     there is no proxy factory linked into this executable that
  //     matches the interface identified by <mostDerivedRepoId>


  GIOPObjectInfo_var objectInfo = new GIOPObjectInfo();

  objectInfo->iopProfiles_  = profiles;

  objectInfo->repositoryID_ = mostDerivedRepoId;

  omniObject_var localobj = ropeFactory::iopProfilesToRope(objectInfo);

  if (!(omniObject*)localobj) {
    // Create a proxy object

    CORBA::Object_ptr objptr;
    if (p) {
      // we have a proxy factory that matches the interface exactly.
      objptr = p->newProxyObject(objectInfo._retn());
      return objptr->PR_getobj();
    }
    else {
      // we don't have a proxy factory that matches the interface
      if (targetRepoId == 0) {
	// The target is just the pseudo object CORBA::Object
	// And we don't have a proxyObjectFactory() for this object
	// (that is why p == 0).
	// We just make an anonymous object
	objptr =  new AnonymousObject(objectInfo._retn(),
				      mostDerivedRepoId);
	return objptr->PR_getobj();
      }
      else {
	// we just give the object the benefit of doubts and 
	// instantiate a proxy object using the proxy factory
	// identified by the <targetRepoId>.
	
	// find the proxy factory for <targetRepoId>
	CORBA::proxyObjectFactory_iterator pnext;
	while ((p = pnext())) {
	  if (strcmp(p->irRepoId(),targetRepoId) == 0) {
	    break;  // got it
	  }
	}
	
	if (p)
	  objptr = p->newProxyObject(objectInfo._retn());
	else {
	  // We don't have a proxyObjectFactory for the target
	  // repoId. When using C++ stubs, this should never
	  // happen. It will happen when using stubs in another
	  // language, such as Python. Create an anonymous object
	  // with the target repoId, to be checked on the first
	  // invocation. If C++ stubs omit the proxyObjectFactory,
	  // all references to the associated interface will be
	  // returned as nil.
	  objptr = new AnonymousObject(objectInfo._retn(),targetRepoId);
	}
	return objptr->PR_getobj();
      }
    }
  }
  else {
    // A local object
    if (targetRepoId && !localobj->_real_is_a(targetRepoId)) {
      // According to the local object, it is neither the exact interface
      // nor a derived interface identified by <targetRepoId>
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    }
    return localobj.retn();
  }
}


char*
omni::objectToString(const omniObject *obj)
{
  if (!obj) {
    IOP::TaggedProfileList p;
    return (char*) IOP::iorToEncapStr((const char*)"",&p);
  }

  if (obj->is_proxy()) {
    CORBA::Boolean fwd;
    GIOPObjectInfo_var objInfo = ((omniObject*)obj)->getInvokeInfo(fwd);
    return IOP::iorToEncapStr(objInfo->repositoryID(),objInfo->iopProfiles());
  }
  else {
    IOP::TaggedProfileList_var pl(new IOP::TaggedProfileList);
    ropeFactory_iterator iter(obj->_objectManager()->incomingRopeFactories());
    incomingRopeFactory* rp;
    omniObjectKey k;
    obj->getKey(k);
    while ((rp = (incomingRopeFactory*) iter())) {
      rp->getIncomingIOPprofiles((CORBA::Octet*)&k,
				 sizeof(k),
				 *(pl.operator->()));
    }
    return IOP::iorToEncapStr(obj->NP_IRRepositoryId(),pl.operator->());
  }
}

omniObject*
omni::stringToObject(const char* str)
{
  CORBA::String_var  repoId;
  IOP::TaggedProfileList_var profiles;

  IOP::EncapStrToIor(str,repoId.out(), profiles.out());
  if (*repoId == '\0' && profiles->length() == 0) {
    // nil object reference
    return 0;
  }

  omniObject* newobj = omni::createObjRef(repoId,0,profiles._retn());
  return newobj;
}

void*
omniObject::_widenFromTheMostDerivedIntf(const char*, CORBA::Boolean)
{
  return 0;
}


void
cdrStream::marshalObjRef(omniObject* obj)
{
  if (!obj) {
    // nil object reference
    ::operator>>= ((CORBA::ULong)1,*this);
    ::operator>>= ((CORBA::Char) '\0',*this);
    ::operator>>= ((CORBA::ULong) 0,*this);
    return;
  }

  // non-nil object reference

  if (obj->is_proxy()) {

    CORBA::Boolean fwd;
    GIOPObjectInfo_var objInfo = obj->getInvokeInfo(fwd);

    const char* repoId = objInfo->repositoryID();
    size_t repoIdSize = strlen(repoId)+1;
    ::operator>>= ((CORBA::ULong) repoIdSize,*this);
    put_char_array((CORBA::Char*) repoId, repoIdSize);
    const IOP::TaggedProfileList * pl = objInfo->iopProfiles();
    *pl >>= *this;

  }
  else {

    IOP::TaggedProfileList_var pl(new IOP::TaggedProfileList);
    ropeFactory_iterator iter(obj->_objectManager()->incomingRopeFactories());
    incomingRopeFactory* rp;
    omniObjectKey k;
    obj->getKey(k);
    while ((rp = (incomingRopeFactory*) iter())) {
      rp->getIncomingIOPprofiles((CORBA::Octet*)&k,
				 sizeof(k),
				 *(pl.operator->()));
    }

    const char* repoId = obj->NP_IRRepositoryId();
    size_t repoIdSize = strlen(repoId)+1;
    ::operator>>= ((CORBA::ULong) repoIdSize,*this);
    put_char_array((CORBA::Char*) repoId, repoIdSize);
    (*(pl.operator->())) >>= *this;

  }

}

omniObject*
cdrStream::unMarshalObjRef(const char* repoId)
{
  CORBA::String_var id;
  IOP::TaggedProfileList_var profiles;

  id = IOP::IOR::unmarshaltype_id(*this);
  
  profiles = new IOP::TaggedProfileList();
  (IOP::TaggedProfileList&)profiles <<= *this;
  
  if (profiles->length() == 0 && strlen(id) == 0) {
    // This is a nil object reference
    return 0;
  }
  else {
    // It is possible that we reach here with the id string = '\0'.
    // That is alright because the actual type of the object will be
    // verified using _is_a() at the first invocation on the object.
    //
    // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
    // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
    // 
    omniObject* objptr = omni::createObjRef(id, repoId,profiles._retn());
    return objptr;
  }
}

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_objectRef_initialiser : public omniInitialiser {
public:

  void attach() {

    omniObject::proxyObjectTable = 0;
    omniObject::localObjectTable = new omniObject*[omniORB::hash_table_size];
    omniObject::localPyObjectTable = new omniObject*[omniORB::hash_table_size];
    unsigned int i;
    for (i=0; i<omniORB::hash_table_size; i++)
      omniObject::localObjectTable[i] = 0;

    for (i=0; i<omniORB::hash_table_size; i++)
      omniObject::localPyObjectTable[i] = 0;

    omniObject::wrappedObjectTable = new void *[omniORB::hash_table_size];

    for (i=0; i<omniORB::hash_table_size; i++)
      omniObject::wrappedObjectTable[i] = 0;

  }

  void detach() {

    if( omniObject::proxyObjectTable ) {
      // Checks the proxy object table when the application
      // is closed, and detects if any object references have not been
      // properly released.
      omniObject** p = &omniObject::proxyObjectTable;
      while( *p ) {
	// Print out a message giving the details of the dangling object
	// references, and also remove them from the list so that they
	// will be picked up by analysis tools such as purify.

	// Any objects held in omniInitialReferences will also show up
	// in this list.

	if( omniORB::traceLevel >= 15 ) {
	  const char* repoId = (*p)->NP_IRRepositoryId();
	  CORBA::Boolean fwd;
	  GIOPObjectInfo_var objInfo = (*p)->getInvokeInfo(fwd);
	  CORBA::String_var obj_ref(IOP::iorToEncapStr(objInfo->repositoryID(),
				     objInfo->iopProfiles()));

	  omniORB::log <<
	    "omniORB: WARNING - Proxy object not released.\n"
	    "  IR ID   : " << repoId << "\n"
	    "  RefCount: " << (*p)->getRefCount() << "\n"
	    "  ObjRef  : " << (const char*)obj_ref << "\n";
	  omniORB::log.flush();
	}
	omniObject** next = &((*p)->pd_next);
	*p = 0;
	p = next;
      }
    }
  }
};

static omni_objectRef_initialiser initialiser;

omniInitialiser& omni_objectRef_initialiser_ = initialiser;
