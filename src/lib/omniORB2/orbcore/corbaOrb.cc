// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaOrb.cc                Created on: 6/2/96
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
//      Implementation of the ORB interface
//	

/*
  $Log$
  Revision 1.6  1997/05/06 15:11:03  sll
  Public release.

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
