// -*- Mode: C++; -*-
//                            Package   : omniORB
// valueFactory.cc            Created on: 2003/09/03
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2003-2005 Apasphere Ltd.
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
//    ValueFactory map
//

/*
  $Log$
  Revision 1.1.2.6  2005/01/13 21:55:58  dgrisby
  Turn off -g debugging; suppress some compiler warnings.

  Revision 1.1.2.5  2005/01/06 16:39:25  dgrisby
  DynValue and DynValueBox implementations; misc small fixes.

  Revision 1.1.2.4  2004/04/02 13:26:23  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

  Revision 1.1.2.3  2004/02/16 10:10:29  dgrisby
  More valuetype, including value boxes. C++ mapping updates.

  Revision 1.1.2.2  2003/10/23 11:25:54  dgrisby
  More valuetype support.

  Revision 1.1.2.1  2003/09/26 16:12:54  dgrisby
  Start of valuetype support.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/valueType.h>
#include <omniORB4/objTracker.h>
#include <initialiser.h>


OMNI_USING_NAMESPACE(omni)

#ifdef HAS_Cplusplus_Namespace
namespace {
#endif

  struct ValueFactoryTableEntry {
    CORBA::String_var       repoId;
    CORBA::ValueFactory     factory;
    CORBA::Boolean          internal;
    ValueFactoryTableEntry* next;

    inline ValueFactoryTableEntry(const char* id,
				  CORBA::ValueFactory f,
				  CORBA::Boolean i)
      : repoId(id), factory(f), internal(i), next(0) {}
  };

  class valueFactoryTableTracker : public omniTrackedObject {
  public:
    valueFactoryTableTracker();
    virtual ~valueFactoryTableTracker();
  };

#ifdef HAS_Cplusplus_Namespace
};
#endif


static omni_tracedmutex*        vf_lock;
static ValueFactoryTableEntry** vf_table     = 0;
static CORBA::ULong             vf_tablesize = 131;


static inline void init_table()
{
  if (!vf_table) {
    valueFactoryTableTracker* vft = new valueFactoryTableTracker();
    registerTrackedObject(vft);
  }
}

valueFactoryTableTracker::
valueFactoryTableTracker()
{
  vf_lock = new omni_tracedmutex();

  omniORB::logs(25, "Initialise value factory table.");
  vf_table = new ValueFactoryTableEntry*[vf_tablesize];

  for (CORBA::ULong i=0; i < vf_tablesize; i++)
    vf_table[i] = 0;
}


valueFactoryTableTracker::
~valueFactoryTableTracker()
{
  OMNIORB_ASSERT(vf_table);

  omniORB::logs(25, "Release value factory table.");

  for (CORBA::ULong i=0; i < vf_tablesize; i++) {
    ValueFactoryTableEntry *f, *n;
    for (f = vf_table[i]; f; f = n) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Release internal value factory for '" << f->repoId << "'.\n";
      }
      n = f->next;
      f->factory->_remove_ref();
      delete f;
    }
  }
  delete [] vf_table;
  vf_table = 0;
  delete vf_lock;
  vf_lock = 0;
}


CORBA::ValueFactory
_omni_ValueFactoryManager::
register_factory(const char* id, CORBA::ULong hashval,
		 CORBA::ValueFactory f, CORBA::Boolean internal)
{
  init_table();

  if (!id)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);

  if (!f)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_ValueFactoryFailure,
		  CORBA::COMPLETED_NO);

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Register value factory for '" << id << "'.\n";
  }

  CORBA::ValueFactory old = 0;

  f->_add_ref();

  {
    omni_tracedmutex_lock sync(*vf_lock);

    CORBA::ULong h = hashval % vf_tablesize;

    ValueFactoryTableEntry* e;

    for (e = vf_table[h]; e; e = e->next) {
      if (omni::strMatch((const char*)e->repoId, id)) {
	old = e->factory;
	break;
      }
    }
    if (!e) {
      e = new ValueFactoryTableEntry(id, f, internal);
      e->next = vf_table[h];
      vf_table[h] = e;
    }
    e->factory = f;
  }
  return old;
}


void
_omni_ValueFactoryManager::
unregister_factory(const char* id, CORBA::ULong hashval)
{
  OMNIORB_ASSERT(vf_table);

  if (!id)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);

  ValueFactoryTableEntry* e;

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "Unregister value factory for '" << id << "'.\n";
  }

  {
    omni_tracedmutex_lock sync(*vf_lock);

    CORBA::ULong h = hashval % vf_tablesize;

    ValueFactoryTableEntry** prev = &(vf_table[h]);

    for (e = vf_table[h]; e; e = e->next) {
      if (omni::strMatch((const char*)e->repoId, id)) {
	*prev = e->next;
	break;
      }
      prev = &(e->next);
    }
  }

  if (e) {
    CORBA::ValueFactory f = e->factory;
    delete e;
    f->_remove_ref();
  }
  else {
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_ValueFactoryFailure,
		  CORBA::COMPLETED_NO);
  }
}


CORBA::ValueFactory
_omni_ValueFactoryManager::
lookup(const char* id, CORBA::ULong hashval)
{
  if (!id)
    OMNIORB_THROW(BAD_PARAM,
		  BAD_PARAM_NullStringUnexpected,
		  CORBA::COMPLETED_NO);

  {
    omni_tracedmutex_lock sync(*vf_lock);

    if (!vf_table)
      return 0;

    CORBA::ULong h = hashval % vf_tablesize;

    ValueFactoryTableEntry* e;

    for (e = vf_table[h]; e; e = e->next) {
      if (omni::strMatch((const char*)e->repoId, id)) {
	e->factory->_add_ref();
	return e->factory;
      }
    }
  }
  return 0;
}

CORBA::ValueBase*
_omni_ValueFactoryManager::
create_for_unmarshal(const char* id, CORBA::ULong hashval)
{
  OMNIORB_ASSERT(id);
  OMNIORB_ASSERT(vf_table);

  CORBA::ValueBase*   r;
  CORBA::ValueFactory f = 0;

  {
    omni_tracedmutex_lock sync(*vf_lock);

    CORBA::ULong h = hashval % vf_tablesize;

    ValueFactoryTableEntry* e;

    for (e = vf_table[h]; e; e = e->next) {
      if (omni::strMatch((const char*)e->repoId, id)) {
	f = e->factory;
	f->_add_ref();
	break;
      }
    }
  }
  if (f) {
    try {
      r = f->create_for_unmarshal();
    }
#ifdef HAS_Cplusplus_catch_exception_by_base
    catch (CORBA::SystemException& ex) {
      f->_remove_ref();
      throw;
    }
#else
#   define CATCH_AND_RETHROW(name) \
    catch (CORBA::name& ex) { \
      f->_remove_ref(); \
      throw; \
    }
    OMNIORB_FOR_EACH_SYS_EXCEPTION(CATCH_AND_RETHROW)
#endif
    catch (...) {
      f->_remove_ref();
      if (omniORB::trace(1)) {
	omniORB::logger l;
	l << "Unexpected exception from ValueFactory create_for_unmarshal, "
	  "id '" << id << "'\n";
      }
      OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_NO);
    }
    f->_remove_ref();
    return r;
  }
  else {
    return 0;
  }
}

//
// ORB member functions
//

CORBA::ValueFactory
CORBA::ORB::
register_value_factory(const char* id, CORBA::ValueFactory factory)
{
  CORBA::ULong h = omniValueType::hash_id(id);
  return _omni_ValueFactoryManager::register_factory(id, h, factory, 0);
}

void
CORBA::ORB::
unregister_value_factory(const char* id)
{
  CORBA::ULong h = omniValueType::hash_id(id);
  _omni_ValueFactoryManager::unregister_factory(id, h);
}

CORBA::ValueFactory
CORBA::ORB::
lookup_value_factory(const char* id)
{
  CORBA::ULong h = omniValueType::hash_id(id);
  return _omni_ValueFactoryManager::lookup(id, h);
}

//
// Module initialiser
//

OMNI_NAMESPACE_BEGIN(omni)

class omni_valueFactory_initialiser : public omniInitialiser {
public:

  omni_valueFactory_initialiser() {
    init_table(); // In case no factories are registered elsewhere.
  }
  void attach() { }
  void detach() {
    omni_tracedmutex_lock sync(*vf_lock);

    if (vf_table) {
      omniORB::logs(25, "Release registered value factories.");

      for (CORBA::ULong i=0; i < vf_tablesize; i++) {

	ValueFactoryTableEntry *f, *n;
	ValueFactoryTableEntry** prev = &(vf_table[i]);

	for (f = vf_table[i]; f; f = n) {
	  n = f->next;

	  if (f->internal) {
	    // Skip internally registered ones.
	    prev = &(f->next);
	  }
	  else {
	    if (omniORB::trace(26)) {
	      omniORB::logger l;
	      l << "Release value factory for '" << f->repoId << "'.\n";
	    }
	    *prev = n;
	    f->factory->_remove_ref();
	    delete f;
	  }
	}
      }
    }
  }
};

static omni_valueFactory_initialiser initialiser;

omniInitialiser& omni_valueFactory_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
