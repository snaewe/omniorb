// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniServant.cc             Created on: 26/2/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Base class for object implementations (servants).
//

/*
  $Log$
  Revision 1.4.2.2  2005/09/05 17:22:09  dgrisby
  Reference counted local call shortcut.

  Revision 1.4.2.1  2003/03/23 21:02:08  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.8  2001/08/15 10:26:13  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.2.2.7  2001/08/03 17:41:23  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.6  2001/05/31 16:18:14  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.5  2001/05/29 17:03:52  dpg1
  In process identity.

  Revision 1.2.2.4  2001/04/18 18:18:06  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.3  2000/11/09 12:27:58  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.2  2000/09/27 18:10:23  sll
  Use calldescriptor to handle upcalls to CORBA::Object operations.

  Revision 1.2.2.1  2000/07/17 10:35:56  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/27 17:32:14  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.3  1999/10/14 16:22:14  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/27 11:01:11  djr
  Modifications to logging.

  Revision 1.1.2.1  1999/09/22 14:27:00  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/omniServant.h>
#include <objectTable.h>
#include <exceptiondefs.h>
#include <omniORB4/IOP_S.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <objectStub.h>

OMNI_USING_NAMESPACE(omni)

omniServant::~omniServant()
{
  if( !pd_activations.empty() ) {
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "ERROR -- A servant has been deleted that is still activated.\n";
      omni::internalLock->lock();

      omnivector<omniObjTableEntry*>::iterator i    = pd_activations.begin();
      omnivector<omniObjTableEntry*>::iterator last = pd_activations.end();

      for (; i != last; i++) {
	l << "      id: " << *i << '\n';
      }
      omni::internalLock->unlock();
    }
  }
}


void*
omniServant::_ptrToInterface(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (void*) 1;

  return 0;
}


void*
omniServant::_downcast()
{
  return 0;
}


const char*
omniServant::_mostDerivedRepoId()
{
  return "";
}


CORBA::Boolean
omniServant::_is_a(const char* repoId)
{
  return _ptrToInterface(repoId) ? 1 : 0;
}


CORBA::Boolean
omniServant::_non_existent()
{
  return 0;
}

_CORBA_Boolean
omniServant::_dispatch(omniCallHandle& handle)
{
  const char* op = handle.operation_name();

  if( omni::strMatch(op, "_is_a") ) {
    omni_is_a_CallDesc call_desc("_is_a",sizeof("_is_a"),0,1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_non_existent") ) {
    omni_non_existent_CallDesc call_desc("_non_existent",
					 sizeof("_non_existent"),1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_interface") ) {
    omni_interface_CallDesc call_desc("_interface",
				      sizeof("_interface"),1);
    handle.upcall(this,call_desc);
    return 1;
  }

  if( omni::strMatch(op, "_implementation") ) {
    omniORB::logs(2,
     "WARNING -- received GIOP request \'_implementation\'.\n"
     " This operation is not supported.  CORBA::NO_IMPLEMENT was raised.");
    OMNIORB_THROW(NO_IMPLEMENT,NO_IMPLEMENT_Unsupported, CORBA::COMPLETED_NO);
  }
  return 0;
}

void
omniServant::_addActivation(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(entry);  OMNIORB_ASSERT(entry->servant() == this);

  pd_activations.push_back(entry);
}


void
omniServant::_removeActivation(omniObjTableEntry* entry)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(*omni::internalLock, 1);
  OMNIORB_ASSERT(entry);

  omnivector<omniObjTableEntry*>::iterator i    = pd_activations.begin();
  omnivector<omniObjTableEntry*>::iterator last = pd_activations.end();

  CORBA::Boolean activation_found = 0;

  for (; i != last; i++) {
    if (*i == entry) {
      pd_activations.erase(i);
      activation_found = 1;
      break;
    }
  }
  OMNIORB_ASSERT(activation_found);
}


void omniServant::_add_ref()    {}
void omniServant::_remove_ref() {}
