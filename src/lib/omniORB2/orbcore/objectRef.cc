// -*- Mode: C++; -*-
//                            Package   : omniORB2
// objectRef.cc               Created on: 20/5/96
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
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
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

#include <omniORB2/proxyFactory.h>
#include <ropeFactory.h>

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
omni_mutex          omniObject::wrappedObjectTableLock;
void**              omniObject::wrappedObjectTable = 0;
proxyObjectFactory* proxyObjectFactory::proxyStubs = 0;


proxyObjectFactory::proxyObjectFactory()
{
  pd_next = proxyObjectFactory::proxyStubs;
  proxyStubs = this;
  return;
}

proxyObjectFactory::~proxyObjectFactory()
{
}

proxyObjectFactory_iterator::proxyObjectFactory_iterator()
{
  pd_f = proxyObjectFactory::proxyStubs;
}

proxyObjectFactory *
proxyObjectFactory_iterator::operator() ()
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
  AnonymousObject(const char* repoId,
		  Rope* r,
		  CORBA::Octet* key,
		  size_t keysize,
		  IOP::TaggedProfileList* profiles,
		  CORBA::Boolean release) :
    omniObject(repoId,r,key,keysize,profiles,release) 
  {
    this->PR_setobj(this);
    // We use PR_IRRepositoryId() to indicate that we don't really know
    // anything about the interface type. Any subsequent queries about its
    // type, such as in interface narrowing, must be answered by
    // the object itself.
    PR_IRRepositoryId(repoId);
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
      omniObject **p = &omniObject::localObjectTable[omniORB::hash(obj->pd_objkey.native)];
      omniObject **pp = p;
      while (*p) {
	if ((*p)->pd_objkey.native == obj->pd_objkey.native) {
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
      omniObject **p = &omniObject::localObjectTable[omniORB::hash(obj->pd_objkey.native)];
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
    omniObject **p = &omniObject::localObjectTable[omniORB::hash(obj->pd_objkey.native)];
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
    if ((*p)->pd_objkey.native == k) {
      (*p)->setRefCount((*p)->getRefCount()+1);
      omniObject::objectTableLock.unlock();
      return *p;
    }
    p = &((*p)->pd_next);
  }
  omniObject::objectTableLock.unlock();
  throw CORBA::OBJECT_NOT_EXIST(0,CORBA::COMPLETED_NO);
#ifdef NEED_DUMMY_RETURN
  return 0;  // MS VC++ 4.0 needs this.
#endif
}


omniObject*
omni::createObjRef(const char* mostDerivedRepoId,
		   const char* targetRepoId,
		   IOP::TaggedProfileList* profiles,
		   CORBA::Boolean release)
{
  CORBA::Octet *objkey = 0;

  proxyObjectFactory *p;
  {
    proxyObjectFactory_iterator pnext;
    while ((p = pnext())) {
      if (strcmp(p->irRepoId(),mostDerivedRepoId) == 0) {
	if (!targetRepoId) {
	  // Target is just the pseudo object CORBA::Object
	  break;  // got it
	}
	else if (!p->is_a(targetRepoId)) {
	    // Object ref is neither the exact interface nor a derived 
	    // interface of the one requested.
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


  size_t ksize = 0;

  Rope_var    rope;
  omniObject* localobj = ropeFactory::iopProfilesToRope(profiles,objkey,
							ksize,rope);

  try {
    if (!localobj) {
      // Create a proxy object
      if (release) {
	CORBA::Object_ptr objptr;
	if (p) {
	  // we have a proxy factory that matches the interface exactly.
	  objptr = p->newProxyObject(rope,objkey,ksize,profiles,1);
	  rope._ptr = 0;
	  return objptr->PR_getobj();
	}
	else {
	  // we don't have a proxy factory that matches the interface
	  if (targetRepoId == 0) {
	    // The target is just the pseudo object CORBA::Object
	    // And we don't have a proxyObjectFactory() for this object
	    // (that is why p == 0).
	    // We just make an anonymous object
	    objptr =  new AnonymousObject(mostDerivedRepoId,rope,
					  objkey,ksize,profiles,1);
	    rope._ptr = 0;
	    return objptr->PR_getobj();
	  }
	  else {
	    // we just give the object the benefit of doubts and 
	    // instantiate a proxy object using the proxy factory
	    // identified by the <targetRepoId>.

	    // find the proxy factory for <targetRepoId>
	    proxyObjectFactory_iterator pnext;
	    while ((p = pnext())) {
	      if (strcmp(p->irRepoId(),targetRepoId) == 0) {
		break;  // got it
	      }
	    }
	    
	    if (!p) {
	      // this is terrible, the caller just give me a <targetRepoId>
	      // that we don't have a proxy factory for.
	      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	    }

	    objptr = p->newProxyObject(rope,objkey,ksize,profiles,1);
	    // The ctor of the proxy object sets its IR repository ID
	    // to <targetRepoId>, we reset it to <mostDerivedRepoId> because
	    // this identifies the true type of the object.
	    objptr->PR_getobj()->PR_IRRepositoryId(mostDerivedRepoId);
	    rope._ptr = 0;
	    return objptr->PR_getobj();
	  }
	}
      }
      else {
	IOP::TaggedProfileList *localcopy = 
	  new IOP::TaggedProfileList(*profiles);
	if (!localcopy) {
	  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
	}
	try {
	  CORBA::Object_ptr objptr;
	  if (p) {
	    // we have a proxy factory that matches the interface exactly.
	    objptr = p->newProxyObject(rope,objkey,ksize,localcopy,1);
	    rope._ptr = 0;
	    return objptr->PR_getobj();
	  }
	  else {
	    // we don't have a proxy factory that matches the interface
	    if (targetRepoId == 0) {
	      // The target is just the pseudo object CORBA::Object
	      // And we don't have a proxyObjectFactory() for this object
	      // (that is why p == 0).
	      // We just make an anonymous object
	      objptr =  new AnonymousObject(mostDerivedRepoId,rope,
					    objkey,ksize,localcopy,1);
	      rope._ptr = 0;
	      return objptr->PR_getobj();
	    }
	    else {
	      // we just give the object the benefit of doubts and 
	      // instantiate a proxy object using the proxy factory
	      // identified by the <targetRepoId>.

	      // find the proxy factory for <targetRepoId>
	      proxyObjectFactory_iterator pnext;
	      while ((p = pnext())) {
		if (strcmp(p->irRepoId(),targetRepoId) == 0) {
		  break;  // got it
		}
	      }
	    
	      if (!p) {
		// this is terrible, the caller just give me a <targetRepoId>
		// that we don't have a proxy factory for.
		throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
	      }

	      objptr = p->newProxyObject(rope,objkey,ksize,localcopy,1);
	      // The ctor of the proxy object sets its IR repository ID
	      // to <targetRepoId>, we reset it to <mostDerivedRepoId> because
	      // this identifies the true type of the object.
	      objptr->PR_getobj()->PR_IRRepositoryId(mostDerivedRepoId);
	      rope._ptr = 0;
	      return objptr->PR_getobj();
	    }
	  }
	}
	catch (...) {
	  delete localcopy;
	  throw;
	}
      }
    }
    else {
      // A local object
      if (targetRepoId && !localobj->_real_is_a(targetRepoId)) {
	// According to the local object, it is neither the exact interface
	// nor a derived interface identified by <targetRepoId>
	omni::objectRelease(localobj);
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
      }
      delete [] objkey;
      if (release)
	delete profiles;
      return localobj;
    }
  }
  catch (...) {
    if (objkey) delete [] objkey;
    throw;
  }
}

char*
omni::objectToString(const omniObject *obj)
{
  if (!obj) {
    IOP::TaggedProfileList p;
    return (char*) IOP::iorToEncapStr((const CORBA::Char*) "", &p);
  }
  else {
    return (char*) IOP::iorToEncapStr((const CORBA::Char*)
				      obj->NP_IRRepositoryId(),
				      obj->iopProfiles());
  }
}

omniObject*
omni::stringToObject(const char* str)
{
  char* repoId;
  IOP::TaggedProfileList* profiles;

  IOP::EncapStrToIor((const CORBA::Char*)str, (CORBA::Char*&)repoId, profiles);
  if (*repoId == '\0' && profiles->length() == 0) {
    // nil object reference
    delete [] repoId;
    delete profiles;
    return 0;
  }

  try {
    omniObject* newobj = omni::createObjRef(repoId,0,profiles,1);
    delete [] repoId;
    return newobj;
  }
  catch (...) {
    delete [] repoId;	
    delete profiles;
    throw;
  }
}


void*
omniObject::_widenFromTheMostDerivedIntf(const char*, CORBA::Boolean)
{
  return 0;
}


void
omniObject::globalInit()
{
  if (omniObject::localObjectTable) return;

  omniObject::proxyObjectTable = 0;
  omniObject::localObjectTable = new omniObject * [omniORB::hash_table_size];
  unsigned int i;
  for (i=0; i<omniORB::hash_table_size; i++)
    omniObject::localObjectTable[i] = 0;

  omniObject::wrappedObjectTable = (void**)
    (new void *[omniORB::hash_table_size]);

  for (i=0; i<omniORB::hash_table_size; i++)
    omniObject::wrappedObjectTable[i] = 0;
}


CORBA::Object_ptr
CORBA::UnMarshalObjRef(const char* repoId, NetBufferedStream& s)
{
  CORBA::ULong idlen;
  CORBA::Char* id = 0;
  IOP::TaggedProfileList* profiles = 0;

  try {
    idlen <<= s;

    switch (idlen) {

    case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
#else
      // According to the CORBA specification 2.0 section 10.6.2:
      //   Null object references are indicated by an empty set of
      //   profiles, and by a NULL type ID (a string which contain
      //   only *** a single terminating character ***).
      //
      // Therefore the idlen should be 1.
      // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
      // and sends out a 0 len string.
      // We quietly accept it here. Turn this off by defining
      //   NO_SLOPPY_NIL_REFERENCE
      id = new CORBA::Char[1];
      id[0] = (CORBA::Char)'\0';
#endif	
      break;

    case 1:
      id = new CORBA::Char[1];
      id[0] <<= s;
      if (id[0] != (CORBA::Char)'\0')
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      idlen = 0;
      break;

    default:
      if (idlen > s.RdMessageUnRead())
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      id = new CORBA::Char[idlen];
      if( !id )  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_MAYBE);
      s.get_char_array(id, idlen);
      if( id[idlen - 1] != '\0' )
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      break;
    }

    profiles = new IOP::TaggedProfileList();
    if( !profiles )  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_MAYBE);
    *profiles <<= s;

    if (profiles->length() == 0 && idlen == 0) {
      // This is a nil object reference
      delete profiles;
      delete[] id;
      return CORBA::Object::_nil();
    }
    else {
      // It is possible that we reach here with the id string = '\0'.
      // That is alright because the actual type of the object will be
      // verified using _is_a() at the first invocation on the object.
      //
      // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
      // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
      // 
      omniObject* objptr = omni::createObjRef((const char*) id, repoId,
					      profiles, 1);
      profiles = 0;
      delete [] id;
      id = 0;
      return (CORBA::Object_ptr)(objptr->_widenFromTheMostDerivedIntf(0));
    }
  }
  catch (...) {
    if( id )        delete[] id;
    if( profiles )  delete profiles;
    throw;
  }
}


void 
CORBA::MarshalObjRef(CORBA::Object_ptr obj,
		     const char* repoId,
		     size_t repoIdSize,
		     NetBufferedStream &s)
{
  if (CORBA::is_nil(obj)) {
    // nil object reference
    ::operator>>= ((CORBA::ULong)1,s);
    ::operator>>= ((CORBA::Char) '\0',s);
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  // non-nil object reference
  repoId = obj->PR_getobj()->NP_IRRepositoryId();
  repoIdSize = strlen(repoId)+1;
  ::operator>>= ((CORBA::ULong) repoIdSize,s);
  s.put_char_array((CORBA::Char*) repoId, repoIdSize);
  IOP::TaggedProfileList * pl = obj->PR_getobj()->iopProfiles();
  *pl >>= s;
}

size_t
CORBA::AlignedObjRef(CORBA::Object_ptr obj,
		     const char* repoId,
		     size_t repoIdSize,
		     size_t initialoffset)
{
  omni::ptr_arith_t msgsize = omni::align_to((omni::ptr_arith_t)
                                                   initialoffset,
						   omni::ALIGN_4);
  if (CORBA::is_nil(obj)) {
    return (size_t) (msgsize + 3 * sizeof(CORBA::ULong));
  }
  else {
    repoId = obj->PR_getobj()->NP_IRRepositoryId();
    repoIdSize = strlen(repoId)+1;
    msgsize += (omni::ptr_arith_t)(sizeof(CORBA::ULong)+repoIdSize);
    IOP::TaggedProfileList *pl = obj->PR_getobj()->iopProfiles();
    return pl->NP_alignedSize((size_t)msgsize);
  }
}


CORBA::Object_ptr
CORBA::UnMarshalObjRef(const char* repoId, MemBufferedStream& s)
{
  CORBA::ULong idlen;
  CORBA::Char* id = 0;
  IOP::TaggedProfileList* profiles = 0;

  try {
    idlen <<= s;

    switch (idlen) {

    case 0:
#ifdef NO_SLOPPY_NIL_REFERENCE
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
#else
      // According to the CORBA specification 2.0 section 10.6.2:
      //   Null object references are indicated by an empty set of
      //   profiles, and by a NULL type ID (a string which contain
      //   only *** a single terminating character ***).
      //
      // Therefore the idlen should be 1.
      // Visibroker for C++ (Orbeline) 2.0 Release 1.51 gets it wrong
      // and sends out a 0 len string.
      // We quietly accept it here. Turn this off by defining
      //   NO_SLOPPY_NIL_REFERENCE
      id = new CORBA::Char[1];
      id[0] = (CORBA::Char)'\0';
#endif	
      break;

    case 1:
      id = new CORBA::Char[1];
      id[0] <<= s;
      if (id[0] != (CORBA::Char)'\0')
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      idlen = 0;
      break;

    default:
      if (idlen > s.RdMessageUnRead())
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      id = new CORBA::Char[idlen];
      if( !id )  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_MAYBE);
      s.get_char_array(id, idlen);
      if( id[idlen - 1] != '\0' )
	throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
      break;
    }

    profiles = new IOP::TaggedProfileList();
    if( !profiles )  throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_MAYBE);
    *profiles <<= s;

    if (profiles->length() == 0 && idlen == 0) {
      // This is a nil object reference
      delete profiles;
      delete[] id;
      return CORBA::Object::_nil();
    }
    else {
      // It is possible that we reach here with the id string = '\0'.
      // That is alright because the actual type of the object will be
      // verified using _is_a() at the first invocation on the object.
      //
      // Apparently, some ORBs such as ExperSoft's do that. Furthermore,
      // this has been accepted as a valid behaviour in GIOP 1.1/IIOP 1.1.
      // 
      omniObject* objptr = omni::createObjRef((const char*) id, repoId,
					      profiles, 1);
      profiles = 0;
      delete [] id;
      id = 0;
      return (CORBA::Object_ptr)(objptr->_widenFromTheMostDerivedIntf(0));
    }
  }
  catch (...) {
    if( id )        delete[] id;
    if( profiles )  delete profiles;
    throw;
  }
}


void 
CORBA::MarshalObjRef(CORBA::Object_ptr obj,
		     const char* repoId,
		     size_t repoIdSize,
		     MemBufferedStream &s)
{
  if (CORBA::is_nil(obj)) {
    // nil object reference
    ::operator>>= ((CORBA::ULong)1,s);
    ::operator>>= ((CORBA::Char) '\0',s);
    ::operator>>= ((CORBA::ULong) 0,s);
    return;
  }

  // non-nil object reference
  repoId = obj->PR_getobj()->NP_IRRepositoryId();
  repoIdSize = strlen(repoId)+1;
  ::operator>>= ((CORBA::ULong) repoIdSize,s);
  s.put_char_array((CORBA::Char*) repoId, repoIdSize);
  IOP::TaggedProfileList * pl = obj->PR_getobj()->iopProfiles();
  *pl >>= s;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// This singleton checks the proxy object table when the application
// is closed, and detects if any object references have not been
// properly released.

class ProxyObjectTableCleaner {
public: // some compilers get upset
  ~ProxyObjectTableCleaner();
  static ProxyObjectTableCleaner theInstance;
};

ProxyObjectTableCleaner ProxyObjectTableCleaner::theInstance;


ProxyObjectTableCleaner::~ProxyObjectTableCleaner()
{
  if( omniObject::proxyObjectTable ) {

    omniObject** p = &omniObject::proxyObjectTable;
    while( *p ) {
      // Print out a message giving the details of the dangling object
      // references, and also remove them from the list so that they
      // will be picked up by analysis tools such as purify.

      // Any objects held in omniInitialReferences will also show up
      // in this list.

      if( omniORB::traceLevel >= 15 ) {
	const char* repoId = (*p)->NP_IRRepositoryId();
	CORBA::String_var obj_ref((char*)
                            IOP::iorToEncapStr((const CORBA::Char*) repoId,
					       (*p)->iopProfiles()));

	omniORB::log <<
	  "omniORB: WARNING - Proxy object not released.\n"
	  "  IR ID   : " << repoId << "\n"
	  "  RefCount: " << (*p)->getRefCount() << "\n"
	  "  ObjRef  : " << (char*)obj_ref << "\n";
	omniORB::log.flush();
      }
      omniObject** next = &((*p)->pd_next);
      *p = 0;
      p = next;
    }

  }
}
