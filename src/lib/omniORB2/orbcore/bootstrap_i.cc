// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.10  1999/09/22 19:21:46  sll
  omniORB 2.8.0 public release.

  Revision 1.9.2.1  1999/09/21 20:37:15  sll
  -Simplified the scavenger code and the mechanism in which connections
   are shutdown. Now only one scavenger thread scans both incoming
   and outgoing connections. A separate thread do the actual shutdown.
  -omniORB::scanGranularity() now takes only one argument as there is
   only one scan period parameter instead of 2.
  -Trace messages in various modules have been updated to use the logger
   class.
  -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                 -ORBscanIncomingPeriod.

  Revision 1.9  1999/08/16 19:23:21  sll
  Replace static variable dtor with initialiser object to enumerate the
  list of initial object reference on shutdown.
  This new scheme avoids the problem that dtor of static variables on
  different compilation units may be called in different order.

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <ropeFactory.h>
#include <tcpSocket.h>
#include <bootstrap_i.h>


static omni_mutex lock;
static omniInitialReferences* _singleton;

omniInitialReferences::omniInitialReferences()
{
  pd_bootagentImpl = 0;
}


void
omniInitialReferences::set(const char* identifier, CORBA::Object_ptr obj)
{
  omni_mutex_lock sync(lock);

  CORBA::ULong index;
  for (index=0; index < pd_serviceList.length(); index++) {
    if (strcmp((const char*)pd_serviceList[index].id,identifier) == 0)
      break;
  }
  if (index == pd_serviceList.length()) {
    pd_serviceList.length(index+1);
    pd_serviceList[index].id = identifier;
  }
  pd_serviceList[index].ref = CORBA::Object::_duplicate(obj);
}


CORBA::Object_ptr
omniInitialReferences::get(const char* identifier)
{
  if( !identifier )  return CORBA::Object::_nil();

  CORBA::Object_ptr result = CORBA::Object::_nil();
  CORBA::Boolean    update = 0;
  {
    omni_mutex_lock sync(lock);

    for( CORBA::ULong index = 0; index < pd_serviceList.length(); index++ ) {
      if( !strcmp((const char*)pd_serviceList[index].id,identifier) ) {
	result = CORBA::Object::_duplicate(pd_serviceList[index].ref);
	break;
      }
    }
    if (CORBA::is_nil(result) && !CORBA::is_nil(pd_bootagent)) {
      try {
	// XXX we will end up in a deadlock if this invocation turns
	//     out to be a chain of invocations and eventually go back
	//     to us.
	if (omniORB::traceLevel >= 10) {
	  CORBA::String_var ior(omni::objectToString(pd_bootagent->PR_getobj()));
	  omniORB::log << "omniORB: Getting initial object reference for "
		       << identifier << " from '"
		       << (const char*) ior << "'...";
	  omniORB::log.flush();
	}

	result = pd_bootagent->get(identifier);
	if( !CORBA::is_nil(result) )
	  update = 1;

	if (omniORB::traceLevel >= 10) {
	  omniORB::log << "Done CORBA::InitialReferences::get()."
		       << ((CORBA::is_nil(result))?" Return value is nil.\n"
			   : "\n");
	  omniORB::log.flush();
	}
      }
      catch(...) {
	if (omniORB::traceLevel >= 10) {
	  omniORB::log << "omniORB: Caught exception in"
	    " CORBA::InitialReferences::get().\n";
	  omniORB::log.flush();
	}
      }
    }
  }
  if (update) 
    set(identifier,result);

  return result;
}


CORBA_InitialReferences::ObjIdList*
omniInitialReferences::list()
{
  omni_mutex_lock sync(lock);

  CORBA_InitialReferences::ObjIdList* result = new CORBA_InitialReferences::ObjIdList;

  result->length(pd_serviceList.length());

  for (CORBA::ULong index=0; index < pd_serviceList.length(); index++) {
    (*result)[index] = CORBA::string_dup(pd_serviceList[index].id);
  }

  // XXX Should we go out to find the listing from the boot agent?

  return result;
}


void
omniInitialReferences::initialise_bootstrap_agentImpl()
{
  omni_mutex_lock sync(lock);

  if (!pd_bootagentImpl) {
    try {
      CORBA::BOA_var boa(CORBA::BOA::getBOA());
      pd_bootagentImpl = new CORBA_InitialReferences_i(boa);
    }
    catch(...) {
    }
  }
}


CORBA_InitialReferences_i*
omniInitialReferences::has_bootstrap_agentImpl()
{
  omni_mutex_lock sync(lock);
  return pd_bootagentImpl;
}


void
omniInitialReferences::initialise_bootstrap_agent(const char* host,
						  CORBA::UShort port)
{
  omni_mutex_lock sync(lock);

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
      if (!f) return; // Error no IIOP ropefactory has been initialised.
    }

    tcpSocketEndpoint addr((CORBA::Char*)host,port);
    CORBA::Char objkey[4];
    objkey[0] = 'I'; objkey[1] = 'N'; objkey[2] = 'I'; objkey[3] = 'T';
    IOP::TaggedProfileList p;
    p.length(1);

    t->encodeIOPprofile((Endpoint*)&addr,objkey,4,p[0]);

    CORBA::String_var ior((char*) IOP::iorToEncapStr((const CORBA::Char*)
				CORBA_InitialReferences_IntfRepoID,&p));
    CORBA::Object_var o((CORBA::Object_ptr) (omni::stringToObject(ior)
                               ->_widenFromTheMostDerivedIntf(0)));
    pd_bootagent = CORBA_InitialReferences::_narrow(o);
    pd_bootagent->noExistentCheck();
  }
  catch (...) {
  }
}


omniInitialReferences*
omniInitialReferences::singleton()
{
  if (!_singleton) {
    _singleton = new omniInitialReferences;
  }
  return _singleton;
}

omniInitialReferences::~omniInitialReferences()
{
  if (pd_bootagentImpl) pd_bootagentImpl->_dispose();
}

void
_omni_set_NameService(CORBA::Object_ptr ns)
{
  omniInitialReferences::singleton()->set((const char*)"NameService", ns);
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_bootstrap_i_initialiser : public omniInitialiser {
public:
  void attach() {
  }

  void detach() {

    if (!_singleton) return;
    delete _singleton;
    _singleton = 0;
  }
};

static omni_bootstrap_i_initialiser initialiser;

omniInitialiser& omni_bootstrap_i_initialiser_ = initialiser;


