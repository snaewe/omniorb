// -*- Mode: C++; -*-
//                            Package   : omniORB
// initRefs.cc                Created on: 20/08/98
//                            Author    : Sai-Lai Lo
//
//    Copyright (C) 1996-2000 AT&T Laboratories Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//	

/*
  $Log$
  Revision 1.2.2.2  2000/09/27 18:15:16  sll
  Use the new omniIOR class and createObjRef() to create the object reference
  for the bootagent.

  Revision 1.2.2.1  2000/07/17 10:35:54  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/30 14:12:07  dpg1
  Minor fixes for FreeBSD.

  Revision 1.1.2.4  2000/06/27 16:15:11  sll
  New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
  _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
  sequence of string and a sequence of object reference.

  Revision 1.1.2.3  2000/06/22 10:40:15  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.2  2000/06/19 13:56:25  dpg1
  Explicit cast to (const char*) when using String_var with logger.

  Revision 1.1.2.1  2000/04/27 10:50:04  dpg1
  Interoperable Naming Service

  Moved from bootstrap_i.cc and extended with INS functions.

  Revision 1.9.6.4  1999/12/10 12:33:16  djr
  Fixed deadlock in bootstrap agent.

  Revision 1.9.6.3  1999/10/04 17:08:31  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.9.6.2  1999/09/24 15:01:32  djr
  Added module initialisers, and sll's new scavenger implementation.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <initRefs.h>
#include <ropeFactory.h>
#include <tcpSocket.h>
#include <initialiser.h>
#include <exceptiondefs.h>
#include <poaimpl.h>
#include <omniORB4/omniURI.h>

static CORBA_InitialReferences_i*  the_bootagentImpl = 0;
static omni_tracedmutex ba_lock;

// When initial references are first set, they contain an id and a
// uri. The first time they are resolved, the uri is replaced with an
// object reference.
struct serviceRecord {
  CORBA::String_member id;
  CORBA::String_member uri;
  CORBA::Object_Member ref;
};

static _CORBA_PseudoValue_Sequence<serviceRecord> the_argsServiceList;
static _CORBA_PseudoValue_Sequence<serviceRecord> the_fileServiceList;
static char*                                      the_argsDefaultInitRef = 0;
static char*                                      the_fileDefaultInitRef = 0;
static CORBA_InitialReferences_ptr                the_bootagent          = 0;

static omni_tracedmutex sl_lock;


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
  return omniInitialReferences::resolve(id);
  // *** What happens if resolve() returns a pseudo object?
}


CORBA_InitialReferences::ObjIdList*
CORBA_InitialReferences_i::list()
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA_InitialReferences::ObjIdList* result =
    new CORBA_InitialReferences::ObjIdList(the_argsServiceList.length() +
					   the_fileServiceList.length());
  if (!result) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  CORBA_InitialReferences::ObjIdList& l = *result;

  l.length(the_argsServiceList.length() + the_fileServiceList.length());

  CORBA::ULong i, j;

  for (i=0,j=0; i < the_argsServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_argsServiceList[i].id);
    if (!(char*)l[j]) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  }
  for (i=0; i < the_fileServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_fileServiceList[i].id);
    if (!(char*)l[j]) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  }
  // XXX Should we go out to find the listing from the boot agent?

  return result;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::Boolean
omniInitialReferences::setFromArgs(const char* identifier,
				   const char* uri)
{
  if (!omniURI::uriSyntaxIsValid(uri)) return 0;

  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_argsServiceList.length(); index++) {
    if (strcmp((const char*)the_argsServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_argsServiceList.length()) {
    the_argsServiceList.length(index+1);
    the_argsServiceList[index].id = identifier;
  }
  the_argsServiceList[index].uri = uri;
  the_argsServiceList[index].ref = CORBA::Object::_nil();
  return 1;
}

static void
setFromArgs(const char* identifier, CORBA::Object_ptr obj)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_argsServiceList.length(); index++) {
    if (strcmp((const char*)the_argsServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_argsServiceList.length()) {
    the_argsServiceList.length(index+1);
    the_argsServiceList[index].id = identifier;
  }
  the_argsServiceList[index].uri = (char*)0;
  the_argsServiceList[index].ref = CORBA::Object::_duplicate(obj);
}

CORBA::Boolean
omniInitialReferences::setFromFile(const char* identifier,
				   const char* uri)
{
  if (!omniURI::uriSyntaxIsValid(uri)) return 0;

  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_fileServiceList.length(); index++) {
    if (strcmp((const char*)the_fileServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_fileServiceList.length()) {
    the_fileServiceList.length(index+1);
    the_fileServiceList[index].id = identifier;
  }
  the_fileServiceList[index].uri = uri;
  the_fileServiceList[index].ref = CORBA::Object::_nil();
  return 1;
}

static void
setFromFile(const char* identifier, CORBA::Object_ptr obj)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong index;
  for (index=0; index < the_fileServiceList.length(); index++) {
    if (strcmp((const char*)the_fileServiceList[index].id,identifier) == 0)
      break;
  }
  if (index == the_fileServiceList.length()) {
    the_fileServiceList.length(index+1);
    the_fileServiceList[index].id = identifier;
  }
  the_fileServiceList[index].uri = (char*)0;
  the_fileServiceList[index].ref = CORBA::Object::_duplicate(obj);
}

void
omniInitialReferences::setDefaultInitRefFromArgs(const char* defInit)
{
  omni_tracedmutex_lock sync(sl_lock);
  if (the_argsDefaultInitRef) CORBA::string_free(the_argsDefaultInitRef);
  the_argsDefaultInitRef = CORBA::string_dup(defInit);
}

void
omniInitialReferences::setDefaultInitRefFromFile(const char* defInit)
{
  omni_tracedmutex_lock sync(sl_lock);
  if (the_fileDefaultInitRef) CORBA::string_free(the_fileDefaultInitRef);
  the_fileDefaultInitRef = CORBA::string_dup(defInit);
}



static CORBA::Object_ptr
resolvePseudo(const char* id, unsigned int cycles)
{
  if( !strcmp(id, "POACurrent") ) {
    OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
  }
  else if( !strcmp(id, "RootPOA") ) {
    // Instantiate the root POA on demand.
    // NB. No race condition problem here - this fn is thread safe.
    return omniOrbPOA::rootPOA();

    // We cannot insert the reference into the initial references
    // map, since holding a reference there would prevent the poa
    // from being released properly when it has been destroyed.
  }
  else if( !strcmp(id, "omniINSPOA") ) {
    // Instantiate the INS POA on demand.
    // NB. No race condition problem here - this fn is thread safe.
    return omniOrbPOA::omniINSPOA();

    // We cannot insert the reference into the initial references
    // map, since holding a reference there would prevent the poa
    // from being released properly when it has been destroyed.
  }
  return 0;
}


static CORBA::Object_ptr
resolveArgs(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr ref = 0;
  CORBA::String_var uri;
  CORBA::ULong      i;
  {
    omni_tracedmutex_lock sync(sl_lock);
    for (i=0; i < the_argsServiceList.length(); i++) {
      if (!strcmp((const char*)the_argsServiceList[i].id, id)) {

	if (!CORBA::is_nil(the_argsServiceList[i].ref)) {
	  ref = CORBA::Object::_duplicate(the_argsServiceList[i].ref);
	}
	else {
	  OMNIORB_ASSERT((char*)(the_argsServiceList[i].uri));
	  uri = CORBA::string_dup(the_argsServiceList[i].uri);
	  if (!(char*)uri) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
	}
	break;
      }
    }
  }
  if ((char*)uri) {
    ref = omniURI::stringToObject(uri, cycles);

    // Store the object reference in the list, unless another thread has
    // got there first
    if (!CORBA::is_nil(ref)) {
      omni_tracedmutex_lock sync(sl_lock);
      for (i=0; i < the_argsServiceList.length(); i++) {
	if (!strcmp((const char*)the_argsServiceList[i].id, id)) {

	  if (CORBA::is_nil(the_argsServiceList[i].ref)) {
	    the_argsServiceList[i].ref = CORBA::Object::_duplicate(ref);
	    the_argsServiceList[i].uri = (char*)0;
	  }
	  break;
	}
      }
    }
  }
  if (ref && omniORB::trace(10)) {
    omniORB::logger l;
    l << "Initial reference `" << id
      << "' resolved from -ORBInitRef argument.\n";
  }
  return ref;
}

static CORBA::Object_ptr
resolveFile(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr ref = 0;
  CORBA::String_var uri;
  CORBA::ULong      i;
  {
    omni_tracedmutex_lock sync(sl_lock);
    for (i=0; i < the_fileServiceList.length(); i++) {
      if (!strcmp((const char*)the_fileServiceList[i].id, id)) {

	if (!CORBA::is_nil(the_fileServiceList[i].ref)) {
	  ref = CORBA::Object::_duplicate(the_fileServiceList[i].ref);
	}
	else {
	  OMNIORB_ASSERT((char*)(the_fileServiceList[i].uri));
	  uri = CORBA::string_dup(the_fileServiceList[i].uri);
	  if (!(char*)uri) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
	}
	break;
      }
    }
  }
  if ((char*)uri) {
    ref = omniURI::stringToObject(uri, cycles);

    // Store the object reference in the list, unless another thread has
    // got there first
    if (!CORBA::is_nil(ref)) {
      omni_tracedmutex_lock sync(sl_lock);
      for (i=0; i < the_fileServiceList.length(); i++) {
	if (!strcmp((const char*)the_fileServiceList[i].id, id)) {

	  if (CORBA::is_nil(the_fileServiceList[i].ref)) {
	    the_fileServiceList[i].ref = CORBA::Object::_duplicate(ref);
	    the_fileServiceList[i].uri = (char*)0;
	  }
	  break;
	}
      }
    }
  }
  if (ref && omniORB::trace(10)) {
    omniORB::logger l;
    l << "Initial reference `" << id
      << "' resolved from configuration file.\n";
  }
  return ref;
}

static CORBA::Object_ptr
resolveArgsDefault(const char* id, unsigned int cycles)
{
  CORBA::String_var uri;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_argsDefaultInitRef) return 0;

    uri = CORBA::string_alloc(strlen(the_argsDefaultInitRef) + strlen(id) + 2);
    if (!(char*)uri) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    strcpy(uri, the_argsDefaultInitRef);
    strcat(uri, "/");
    strcat(uri, id);
  }
  try {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Trying to resolve initial reference `" << id << "'\n";
      l << " with default `" << (const char*)uri << "'\n";
    }
    CORBA::Object_ptr obj = omniURI::stringToObject(uri, cycles);

    // Store the retrieved object in the args list, so future
    // resolves return the same thing.

    // Note that there's a race condition here. Another thread might
    // have been through here just before us, and stored a different
    // object reference in the list. That's so unlikely that we don't
    // bother to prevent it. The spec. makes no guarantees about when
    // initial reference resolution happens anyway.
    setFromArgs(id, obj);
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Initial reference `" << id
	<< "' resolved with -ORBDefaultInitRef prefix.\n";
    }
    return obj;
  }
  catch(...) {
  }
  return 0;
}

static CORBA::Object_ptr
resolveFileDefault(const char* id, unsigned int cycles)
{
  CORBA::String_var uri;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_fileDefaultInitRef) return 0;

    uri = CORBA::string_alloc(strlen(the_fileDefaultInitRef) + strlen(id) + 2);
    if (!(char*)uri) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
    strcpy(uri, the_fileDefaultInitRef);
    strcat(uri, "/");
    strcat(uri, id);
  }
  try {
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Trying to resolve initial reference `" << id << "'\n";
      l << " with configuration file default `" << (const char*)uri << "'\n";
    }
    CORBA::Object_ptr obj = omniURI::stringToObject(uri, cycles);

    // Store the retrieved object in the file list, so future
    // resolves return the same thing.

    // Note that there's a race condition here. Another thread might
    // have been through here just before us, and stored a different
    // object reference in the list. That's so unlikely that we don't
    // bother to prevent it. The spec. makes no guarantees about when
    // initial reference resolution happens anyway.
    setFromFile(id, obj);
    if (omniORB::trace(10)) {
      omniORB::logger l;
      l << "Initial reference `" << id
	<< "' resolved with configuration file ORBDefaultInitRef prefix.\n";
    }
    return obj;
  }
  catch(...) {
  }
  return 0;
}


static CORBA::Object_ptr
resolveBootAgent(const char* id, unsigned int cycles)
{
  CORBA::Object_ptr result = 0;
  CORBA::Boolean    update = 0;
  {
    omni_tracedmutex_lock sync(sl_lock);

    if (!the_bootagent || CORBA::is_nil(the_bootagent)) return 0;

    try {
      // XXX we will end up in a deadlock if this invocation turns
      //     out to be a chain of invocations and eventually go back
      //     to us.
      if (omniORB::trace(10)) {
	CORBA::String_var ior(omniURI::objectToString(the_bootagent));
	omniORB::logger l;
	l << "Trying to resolve initial reference `" << id << "'\n";
	l << " with boot agent: " << (const char*)ior << "\n";
      }
      result = the_bootagent->get(id);

      if (CORBA::is_nil(result))
	result = 0;
      else
	update = 1;

      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "Initial reference `" << id << "' resolved with boot agent.\n";
      }
    }
    catch(...) {
      if (omniORB::trace(10)) {
	omniORB::logger l;
	l << "Caught exception trying to resolve `" << id
	  << "' with boot agent.";
      }
    }
  }
  if (update) setFromArgs(id, result);
  return result;
}


CORBA::Object_ptr
omniInitialReferences::resolve(const char* id, unsigned int cycles)
{
  if (!id) throw CORBA::ORB::InvalidName();

  CORBA::Object_ptr result;

  // Look for the id in the order prescribed by the CORBA spec, with
  // extensions for ORBDefaultInitRef in the config file, and the
  // bootstrap agent.
  if ((result = resolvePseudo     (id, cycles))) return result;
  if ((result = resolveArgs       (id, cycles))) return result;
  if ((result = resolveArgsDefault(id, cycles))) return result;
  if ((result = resolveFile       (id, cycles))) return result;
  if ((result = resolveFileDefault(id, cycles))) return result;
  if ((result = resolveBootAgent  (id, cycles))) return result;

  // No more possibilities. Throw the correct exception to the caller.
  if (!strcmp(id, "InterfaceRepository") ||
      !strcmp(id, "NameService") ||
      !strcmp(id, "TradingService") ||
      !strcmp(id, "SecurityCurrent") ||
      !strcmp(id, "TransactionCurrent"))
    // Resource not found.
    OMNIORB_THROW(NO_RESOURCES,0,CORBA::COMPLETED_NO);

  // The identifier is not defined.
  if (omniORB::trace(10)) {
    omniORB::logger l;
    l << "resolve_initial_references throws CORBA::ORB::InvalidName\n";
  }
  throw CORBA::ORB::InvalidName();

  // Never get here...
  return 0;
}


CORBA::ORB::ObjectIdList*
omniInitialReferences::list()
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ORB::ObjectIdList* result =
    new CORBA::ORB::ObjectIdList(the_argsServiceList.length() +
				 the_fileServiceList.length());
  if (!result) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  CORBA::ORB::ObjectIdList& l = *result;

  l.length(the_argsServiceList.length() + the_fileServiceList.length());

  CORBA::ULong i, j;

  for (i=0,j=0; i < the_argsServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_argsServiceList[i].id);
    if (!(char*)l[j]) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  }
  for (i=0; i < the_fileServiceList.length(); i++,j++) {
    l[j] = CORBA::string_dup(the_fileServiceList[i].id);
    if (!(char*)l[j]) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);
  }
  return result;
}



void
omniInitialReferences::remFromFile(const char* id)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong i;
  for (i=0; i < the_fileServiceList.length(); i++) {

    if (strcmp((const char*)the_fileServiceList[i].id, id) == 0) {

      for (i++; i < the_fileServiceList.length(); i++) {
	the_fileServiceList[i-1].id  = the_fileServiceList[i].id;
	the_fileServiceList[i-1].uri = the_fileServiceList[i].uri;
	the_fileServiceList[i-1].ref = the_fileServiceList[i].ref;
      }
      the_fileServiceList.length(the_fileServiceList.length() - 1);
      return;
    }
  }
}

void
omniInitialReferences::remFromArgs(const char* id)
{
  omni_tracedmutex_lock sync(sl_lock);

  CORBA::ULong i;
  for (i=0; i < the_argsServiceList.length(); i++) {

    if (strcmp((const char*)the_argsServiceList[i].id, id) == 0) {

      for (i++; i < the_argsServiceList.length(); i++) {
	the_argsServiceList[i-1].id  = the_argsServiceList[i].id;
	the_argsServiceList[i-1].uri = the_argsServiceList[i].uri;
	the_argsServiceList[i-1].ref = the_argsServiceList[i].ref;
      }
      the_argsServiceList.length(the_argsServiceList.length() - 1);
      return;
    }
  }
}


void
omniInitialReferences::initialise_bootstrap_agentImpl()
{
  ba_lock.lock();

  if( !the_bootagentImpl )
    the_bootagentImpl = new CORBA_InitialReferences_i();

  if (!the_bootagentImpl) OMNIORB_THROW(NO_MEMORY,0,CORBA::COMPLETED_NO);

  ba_lock.unlock();
}


int
omniInitialReferences::invoke_bootstrap_agentImpl(GIOP_S& giop_s)
{
  omni_tracedmutex_lock sync(ba_lock);

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
  omni_tracedmutex_lock sync(sl_lock);

  try {
    IIOP::Address addr;
    addr.host = host;
    addr.port = port;

    _CORBA_Unbounded_Sequence_Octet objkey;
    objkey.length(4);
    objkey[0] = 'I'; objkey[1] = 'N'; objkey[2] = 'I'; objkey[3] = 'T';

    omniIOR* ior= new omniIOR(CORBA_InitialReferences::_PD_repoId,
			      objkey,&addr,1);
    
    omniObjRef* objref = omni::createObjRef(
                              CORBA_InitialReferences::_PD_repoId,ior,0);
    if (!objref->_is_nil()) {
      the_bootagent = (CORBA_InitialReferences_ptr) 
   	      objref->_ptrToObjRef(CORBA_InitialReferences::_PD_repoId);
      the_bootagent->_noExistentCheck();
    }
  }
  catch(...) {}
}


void
_omni_set_NameService(CORBA::Object_ptr ns)
{
  setFromArgs((const char*) "NameService", ns);
}


/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_initRefs_initialiser : public omniInitialiser {
public:
  void attach() {
  }

  void detach() {
    if (the_argsDefaultInitRef) CORBA::string_free(the_argsDefaultInitRef);
    the_argsDefaultInitRef = 0;
    if (the_fileDefaultInitRef) CORBA::string_free(the_fileDefaultInitRef);
    the_fileDefaultInitRef = 0;
    if( the_bootagentImpl ) delete the_bootagentImpl;
    the_bootagentImpl = 0;
    if( the_bootagent )  CORBA::release(the_bootagent);
    the_bootagent = 0;
  }
};

static omni_initRefs_initialiser initialiser;

omniInitialiser& omni_initRefs_initialiser_ = initialiser;
