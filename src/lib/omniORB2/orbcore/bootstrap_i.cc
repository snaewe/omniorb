// -*- Mode: C++; -*-
//                            Package   : omniORB
// bootstrap_i.cc             Created on: 20/08/98
//                            Author    : Sai-Lai Lo
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
  Revision 1.9.6.1  1999/09/22 14:26:42  djr
  Major rewrite of orbcore to support POA.

  Revision 1.8  1999/05/25 17:24:39  sll
  CORBA::ORB::ObjectIdList and CORBA_InitialReferences::ObjIdList are
  now different types. Previously they are the same template type instance.

  Revision 1.7  1999/03/11 16:25:51  djr
  Updated copyright notice

  Revision 1.6  1999/02/01 15:13:04  djr
  Replace copy-initialisation of _var types with direct initialisation.

  Revision 1.5  1999/01/11 09:45:53  djr
  *** empty log message ***

  Revision 1.4  1999/01/07 15:13:08  djr
  Added singleton class omniInitialRefLister to list the initial object
  references found (if traceLevel >= 15).

  Revision 1.3  1998/08/26 11:19:05  sll
   Minor upates to remove warnings when compiled with standard C++ compilers.

  Revision 1.2  1998/08/25 19:01:35  sll
  Moved auto-variable declaration in set() to make it acceptable to old
  and new compilers.

  Revision 1.1  1998/08/21 19:28:16  sll
  Initial revision

  */

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <bootstrap_i.h>
#include <ropeFactory.h>
#include <tcpSocket.h>


static omni_mutex the_lock;
static CORBA_InitialReferences_ptr the_bootagent = 0;
static CORBA_InitialReferences_i*  the_bootagentImpl = 0;

struct serviceRecord {
  CORBA::String_member id;
  CORBA::Object_member ref;
};

static _CORBA_Pseudo_Unbounded_Sequence<serviceRecord,serviceRecord>
                                   the_serviceList;


//////////////////////////////////////////////////////////////////////
////////////////////// CORBA_InitialReferences_i /////////////////////
//////////////////////////////////////////////////////////////////////

class CORBA_InitialReferences_i : public POA_CORBA_InitialReferences {
public:
  CORBA::Object_ptr get(const char* id);
  CORBA_InitialReferences::ObjIdList* list();
  inline CORBA_InitialReferences_i()   {}
  virtual ~CORBA_InitialReferences_i() {}

private:
  CORBA_InitialReferences_i(const CORBA_InitialReferences_i&);
  CORBA_InitialReferences_i& operator=(const CORBA_InitialReferences_i&);
};


CORBA::Object_ptr
CORBA_InitialReferences_i::get(const char* id)
{
  return omniInitialReferences::get(id);
}


CORBA_InitialReferences::ObjIdList*
CORBA_InitialReferences_i::list()
{
  omni_mutex_lock sync(the_lock);

  CORBA_InitialReferences::ObjIdList* result =
    new CORBA_InitialReferences::ObjIdList(the_serviceList.length());
  CORBA_InitialReferences::ObjIdList& l(*result);

  l.length(the_serviceList.length());

  for( CORBA::ULong index = 0; index < the_serviceList.length(); index++ )
    l[index] = CORBA::string_dup(the_serviceList[index].id);

  // XXX Should we go out to find the listing from the boot agent?

  return result;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniInitialReferences::set(const char* identifier, CORBA::Object_ptr obj)
{
  omni_mutex_lock sync(the_lock);

  CORBA::ULong index;
  for (index=0; index < the_serviceList.length(); index++) {
    if (strcmp((const char*)the_serviceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_serviceList.length()) {
    the_serviceList.length(index+1);
    the_serviceList[index].id = identifier;
  }
  the_serviceList[index].ref = CORBA::Object::_duplicate(obj);
}


CORBA::Object_ptr
omniInitialReferences::get(const char* identifier)
{
  if( !identifier )  return CORBA::Object::_nil();

  CORBA::Object_ptr result = CORBA::Object::_nil();
  CORBA::Boolean    update = 0;
  {
    omni_mutex_lock sync(the_lock);

    for( CORBA::ULong index = 0; index < the_serviceList.length(); index++ ) {
      if( !strcmp((const char*)the_serviceList[index].id,identifier) ) {
	result = CORBA::Object::_duplicate(the_serviceList[index].ref);
	break;
      }
    }
    if( CORBA::is_nil(result) && !CORBA::is_nil(the_bootagent) ) {
      try {
	// XXX we will end up in a deadlock if this invocation turns
	//     out to be a chain of invocations and eventually go back
	//     to us.
	if (omniORB::traceLevel >= 10) {
	  CORBA::String_var ior(
			omni::objectToString(the_bootagent->_PR_getobj()));
	  omniORB::log <<
	    "omniORB: Getting initial object reference for: " << identifier <<
	    "\n"
	    " from: " << (const char*) ior << "\n";
	  omniORB::log.flush();
	}

	result = the_bootagent->get(identifier);
	if( !CORBA::is_nil(result) )
	  update = 1;

	if (omniORB::traceLevel >= 10) {
	  omniORB::log << "Done CORBA::InitialReferences::get()."
		       << (CORBA::is_nil(result) ? " Return value is nil.\n"
			   : "\n");
	  omniORB::log.flush();
	}
      }
      catch(...) {
	if (omniORB::traceLevel >= 10) {
	  omniORB::log <<
	    "omniORB: Caught exception in CORBA::InitialReferences::get().\n";
	  omniORB::log.flush();
	}
      }
    }
  }
  if( update )  set(identifier,result);

  return result;
}


CORBA::ORB::ObjectIdList*
omniInitialReferences::list()
{
  omni_mutex_lock sync(the_lock);

  CORBA::ORB::ObjectIdList* result =
    new CORBA::ORB::ObjectIdList(the_serviceList.length());
  CORBA::ORB::ObjectIdList& l(*result);

  l.length(the_serviceList.length());

  for( CORBA::ULong index = 0; index < the_serviceList.length(); index++ )
    l[index] = CORBA::string_dup(the_serviceList[index].id);

  // XXX Should we go out to find the listing from the boot agent?

  return result;
}


void
omniInitialReferences::initialise_bootstrap_agentImpl()
{
  the_lock.lock();

  if( !the_bootagentImpl )
    the_bootagentImpl = new CORBA_InitialReferences_i();

  the_lock.unlock();
}


int
omniInitialReferences::invoke_bootstrap_agentImpl(GIOP_S& giop_s)
{
  omni_mutex_lock sync(the_lock);

  if( !the_bootagentImpl )  return 0;

  ((omniServant*) the_bootagentImpl)->_dispatch(giop_s);
  return 1;
}


int
omniInitialReferences::is_bootstrap_agentImpl_initialised()
{
  return the_bootagentImpl ? 1 : 0;
}


void
omniInitialReferences::initialise_bootstrap_agent(const char* host,
						  CORBA::UShort port)
{
  omni_mutex_lock sync(the_lock);

  try {
    
    const ropeFactoryType* t;

    {
      ropeFactory_iterator next(globalOutgoingRopeFactories);
      const ropeFactory* f;
      while ((f = next())) {
	t = f->getType();
	if (t->is_IOPprofileId(IOP::TAG_INTERNET_IOP))
	  break;
      }
      if( !f )  return;  // Error no IIOP ropefactory has been initialised.
    }

    tcpSocketEndpoint addr((CORBA::Char*) host, port);
    CORBA::Char objkey[4];
    objkey[0] = 'I'; objkey[1] = 'N'; objkey[2] = 'I'; objkey[3] = 'T';
    IOP::TaggedProfileList p;
    p.length(1);

    t->encodeIOPprofile((Endpoint*) &addr, objkey, 4, p[0]);

    CORBA::String_var ior((char*) IOP::iorToEncapStr((const CORBA::Char*)
				     CORBA_InitialReferences::_PD_repoId, &p));
    omniObjRef* obj;
    if( !omni::stringToObject(obj, ior) )  return;
    CORBA::Object_var o;
    if( obj )
      o = (CORBA::Object_ptr) obj->_ptrToObjRef(CORBA::Object::_PD_repoId);
    the_bootagent = CORBA_InitialReferences::_narrow(o);
    the_bootagent->_noExistentCheck();
  }
  catch (...) {
  }
}


void
_omni_set_NameService(CORBA::Object_ptr ns)
{
  omniInitialReferences::set((const char*) "NameService", ns);
}
