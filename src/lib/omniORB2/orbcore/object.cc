// -*- Mode: C++; -*-
//                            Package   : omniORB2
// object.cc                  Created on: 13/5/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//      
 
/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

  */

#include <omniORB2/CORBA.h>

omniObject::omniObject()                     // ctor for local object
{
  omniObjectKey::generateNewKey(pd_objkey.native);
  pd_objkeysize = sizeof(pd_objkey.native);
  pd_repoId = 0;
  pd_rope = 0;
  pd_proxy = 0;
  pd_refCount = 0;
  pd_next = 0;
  pd_disposed = 0;
  pd_iopprofile = omniORB::objectToIopProfiles(this);
  return;
}


omniObject::omniObject(const char *repoId,   // ctor for proxy object
	       Rope *r,
	       CORBA::Octet *key,
	       size_t keysize,
	       IOP::TaggedProfileList *profiles,
	       CORBA::Boolean release)
{
  if (!repoId || !r || !key || !keysize || !profiles)
    throw CORBA::BAD_PARAM(0,CORBA::COMPLETED_NO);

  pd_repoId = new char[strlen(repoId)+1];
  if (!pd_repoId)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  strcpy(pd_repoId,repoId);

  pd_rope = r;  // caller has already done a r->incrRefCount()
  pd_proxy = 1;
  pd_refCount = 0;
  pd_next = 0;
  pd_disposed = 0;

  if (!release) {
    pd_iopprofile = 0;
    pd_objkey.foreign = 0;
    try {
      pd_iopprofile = new IOP::TaggedProfileList(*profiles);
      if (!pd_iopprofile)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      pd_objkeysize = keysize;
      pd_objkey.foreign = new CORBA::Octet[keysize];
      if (!pd_objkey.foreign)
	throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
      memcpy((void *)pd_objkey.foreign,(const void *)key,keysize);
    }
    catch (...) {
      delete [] pd_repoId;
      if (pd_iopprofile) delete pd_iopprofile;
      if (pd_objkey.foreign) delete [] pd_objkey.foreign;
      throw;
    }
  }
  else {
    pd_objkeysize = keysize;
    pd_objkey.foreign =  key;
    pd_iopprofile = profiles;
  }
  return;

}


void
omniObject::NP_objkey(const omniObjectKey &k)
{
  if (pd_refCount || pd_proxy)
    throw CORBA::BAD_PARAM(1,CORBA::COMPLETED_NO);
  pd_objkey.native = k;
  return;
}

void
omniObject::NP_rope(Rope *r)
{
  if (is_proxy()) {
    if (pd_rope) {
      pd_rope->decrRefCount();
    }
    pd_rope = r; // caller has already done a r->incrRefCount()
  }
  return;
}

void
omniObject::PR_IRRepositoryId(const char *ir)
{
  if (pd_refCount || pd_proxy)
    throw omniORB::fatalException(__FILE__,__LINE__,
      "omniObject::PR_IRRepositoryId()- tried to set IR Id for a proxy or an activated object");
  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoId = new char [strlen(ir)+1];
  if (!pd_repoId)
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  strcpy(pd_repoId,ir);
  return;
}

CORBA::Boolean
omniObject::dispatch(GIOP_S &s,const char *op,
		 CORBA::Boolean response_expected) {
  // If this code is ever called, it is certainly because of a bug.
  throw omniORB::fatalException(__FILE__,__LINE__,
	"omniObject::dispatch()- unexpected call to this function.");
  return 0;
}


omniObject::~omniObject()
{
  if (pd_refCount) {
    // A dtor should not be called if the reference count is not 0
    assert(0);
  }
  if (pd_rope)
    pd_rope->decrRefCount();
  pd_rope = 0;
  if (pd_repoId)
    delete [] pd_repoId;
  pd_repoId = 0;
  if (pd_proxy && pd_objkey.foreign) {
    delete [] pd_objkey.foreign;
    pd_objkey.foreign = 0;
  }
  return;
}

