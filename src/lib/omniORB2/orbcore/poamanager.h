// -*- Mode: C++; -*-
//                            Package   : omniORB
// poamanager.h               Created on: 12/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Internal implementation of the PortableServer::POAManager.
//

/*
  $Log$
  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.4  2000/01/27 10:55:47  djr
  Mods needed for powerpc_aix.  New macro OMNIORB_BASE_CTOR to provide
  fqname for base class constructor for some compilers.

  Revision 1.1.2.3  1999/10/04 17:08:34  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.1.2.2  1999/09/30 11:52:33  djr
  Implemented use of AdapterActivators in POAs.

  Revision 1.1.2.1  1999/09/22 14:27:02  djr
  Major rewrite of orbcore to support POA.

*/

#ifndef __POAMANAGER_H__
#define __POAMANAGER_H__

#ifndef __OMNIPOA_H__
#include <omniORB3/poa.h>
#endif


class omniOrbPOA;


class omniOrbPOAManager : public PortableServer::POAManager {
public:
  virtual ~omniOrbPOAManager();
  inline omniOrbPOAManager(int is_nil = 0)
    : OMNIORB_BASE_CTOR(PortableServer::)POAManager(is_nil),
      pd_refCount(1),
      pd_state(HOLDING)
    {}

  ////////////////////////////////
  // PortableServer::POAManager //
  ////////////////////////////////
  virtual void activate();
  virtual void hold_requests(CORBA::Boolean wait_for_completion);
  virtual void discard_requests(CORBA::Boolean wait_for_completion);
  virtual void deactivate(CORBA::Boolean etherealize_objects,
			  CORBA::Boolean wait_for_completion);
  virtual State get_state();

  ////////////////////////////
  // Override CORBA::Object //
  ////////////////////////////
  virtual void* _ptrToObjRef(const char* repoId);
  virtual void _NP_incrRefCount();
  virtual void _NP_decrRefCount();

  //////////////
  // Internal //
  //////////////
  void gain_poa(omniOrbPOA* poa);
  // Adds <poa> to our list of POAs, and if our state is not
  // HOLDING, updates the POAs state to match our own.

  void lose_poa(omniOrbPOA* poa);


  typedef _CORBA_PseudoValue_Sequence<omniOrbPOA*> POASeq;


private:
  POASeq pd_poas;
  // We don't hold a reference to each POA - since they each
  // hold references to us.

  int    pd_refCount;
  State  pd_state;
};


#endif  // __POAMANAGER_H__
