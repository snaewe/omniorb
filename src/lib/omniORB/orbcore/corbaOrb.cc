// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaOrb.cc                Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//      Implementation of the ORB interface
//	

/*
  $Log$
  Revision 1.5  1997/04/21 10:24:52  ewc
  Bug fix to resolve_initial_references()

// Revision 1.4  1997/03/10  11:54:28  sll
// Minor changes to accomodate the creation of a public API for omniORB2.
//
  Revision 1.3  1997/01/23 16:38:33  sll
  Locals like boa_initialised are now static members of the omniORB class.

// Revision 1.2  1997/01/21  14:19:44  ewc
// Added support for initial references interface.
//
// Revision 1.1  1997/01/08  17:26:01  sll
// Initial revision
//
 */

#include <omniORB2/CORBA.h>

CORBA::
ORB::ORB()
{
  return;
}

CORBA::
ORB::~ORB()
{
  return;
}


CORBA::ORB_ptr
CORBA::ORB_init(int &argc,char **argv,const char *orb_identifier)
{
  omni::initLock.lock();
  if (!omni::orb_initialised) {
    omni::init(argc,argv,orb_identifier);
    omni::orb_initialised = 1;
  }
  omni::initLock.unlock();
  return &CORBA::ORB::orb;
}

CORBA::BOA_ptr
CORBA::
ORB::BOA_init(int &argc, char **argv, const char *boa_identifier)
{
  omni::initLock.lock();
  if (!omni::boa_initialised) {
    omni::boaInit(argc,argv,boa_identifier);
    omni::boa_initialised = 1;
  }
  omni::initLock.unlock();
  return &CORBA::BOA::boa;
}

CORBA::BOA_ptr
CORBA::
BOA::getBOA()
{
  if (!omni::boa_initialised) {
    throw CORBA::OBJ_ADAPTER(0,CORBA::COMPLETED_NO);
  }
  return &CORBA::BOA::boa;
}


char *
CORBA::
ORB::object_to_string(CORBA::Object_ptr p)
{
  if (CORBA::is_nil(p))
    return omni::objectToString(0);
  else 
    return omni::objectToString(p->PR_getobj());
}

CORBA::Object_ptr
CORBA::
ORB::string_to_object(const char *m)
{
  omniObject *objptr = omni::stringToObject(m);
  if (objptr)
    return (CORBA::Object_ptr)(objptr->_widenFromTheMostDerivedIntf(0));
  else
    return CORBA::Object::_nil();
}

CORBA::ORB_ptr
CORBA::
ORB::_duplicate(CORBA::ORB_ptr p)
{
  return p;
}

CORBA::ORB_ptr
CORBA::
ORB::_nil()
{
  return 0;
}

void
CORBA::release(ORB_ptr p)
{
  return;
}

CORBA::Boolean
CORBA::is_nil(ORB_ptr p)
{
  return (p == 0) ? 1 : 0;
}

// Implementation of Initial Reference member functions:


CORBA::Object_ptr 
CORBA::ORB::resolve_initial_references(const char* identifier)
{
  omniObject* objptr = omni::resolveInitRef(identifier);

  if (objptr)
    {
      CORBA::Object_ptr retobj = (CORBA::Object_ptr) objptr->_widenFromTheMostDerivedIntf(0);
      CORBA::Object::_duplicate(retobj);
      return retobj;
    }
  else return CORBA::Object::_nil();
}


CORBA::
ORB::ObjectIdList* CORBA::ORB::list_initial_services()
{
char** servicelist;

unsigned long listlen = omni::listInitServices(servicelist);

ObjectIdList* idlist;

if (listlen == 0) idlist = new ObjectIdList(0,0,NULL);
else idlist = 
      new ObjectIdList(listlen,listlen,(CORBA::String_member*) servicelist,1);

return idlist;
}


// Implementation of InvalidName member functions:
// [ InvalidName = exception raised by resolve_initial_references() ]

CORBA::
ORB::InvalidName::InvalidName(const ORB::InvalidName &_s)
{
}

CORBA::
ORB::InvalidName & CORBA::ORB::InvalidName::operator=(const ORB::InvalidName &_s)
{
  return *this;
}

size_t
CORBA::
ORB::InvalidName::NP_alignedSize(size_t _initialoffset)
{
  size_t _msgsize = _initialoffset;
  return _msgsize;
}

void
CORBA::
ORB::InvalidName::operator>>= (NetBufferedStream &_n)
{
}

void
CORBA::
ORB::InvalidName::operator<<= (NetBufferedStream &_n)
{
}

void
CORBA::
ORB::InvalidName::operator>>= (MemBufferedStream &_n)
{
}

void
CORBA::
ORB::InvalidName::operator<<= (MemBufferedStream &_n)
{
}
