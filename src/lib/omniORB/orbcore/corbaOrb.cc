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
  Revision 1.2  1997/01/21 14:19:44  ewc
  Added support for initial references interface.

// Revision 1.1  1997/01/08  17:26:01  sll
// Initial revision
//
 */

#include <omniORB2/CORBA.h>

static omni_mutex initLock;
static CORBA::Boolean orb_initialised = 0;
static CORBA::Boolean boa_initialised = 0;
static CORBA::ORB orb;
static CORBA::BOA boa;

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
  initLock.lock();
  if (!orb_initialised) {
    omniORB::init(argc,argv,orb_identifier);
    orb_initialised = 1;
  }
  initLock.unlock();
  return &orb;
}

CORBA::BOA_ptr
CORBA::
ORB::BOA_init(int &argc, char **argv, const char *boa_identifier)
{
  initLock.lock();
  if (!boa_initialised) {
    omniORB::boaInit(argc,argv,boa_identifier);
    boa_initialised = 1;
  }
  initLock.unlock();
  return &boa;
}

char *
CORBA::
ORB::object_to_string(CORBA::Object_ptr p)
{
  if (CORBA::is_nil(p))
    return omniORB::objectToString(0);
  else 
    return omniORB::objectToString(p->PR_getobj());
}

CORBA::Object_ptr
CORBA::
ORB::string_to_object(const char *m)
{
  omniObject *objptr = omniORB::stringToObject(m);
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
  omniObject* objptr = omniORB::resolveInitRef(identifier);

  if (objptr)
      return (CORBA::Object_ptr) (objptr->_widenFromTheMostDerivedIntf(0));
  else return CORBA::Object::_nil();
}


CORBA::
ORB::ObjectIdList* CORBA::ORB::list_initial_services()
{
char** servicelist;

unsigned long listlen = omniORB::listInitServices(servicelist);

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
