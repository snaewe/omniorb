// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_BOA.h                Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
//    CORBA::BOA
//

/*
  $Log$
  Revision 1.1.4.1  2003/03/23 21:04:25  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.1  2001/08/17 13:39:44  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////////// BOA ////////////////////////////////
//////////////////////////////////////////////////////////////////////

class BOA;
class BOA_var;
typedef BOA* BOA_ptr;
typedef BOA_ptr BOARef;


class ReferenceData : public _CORBA_Unbounded_Sequence_Octet {
public://??
  inline ReferenceData() {}
  inline ReferenceData(const ReferenceData& seq)
    : _CORBA_Unbounded_Sequence_Octet(seq) {}
  inline ReferenceData(CORBA::ULong max)
    : _CORBA_Unbounded_Sequence_Octet(max) {}
  inline ReferenceData(CORBA::ULong max, CORBA::ULong len, CORBA::Octet* val, CORBA::Boolean rel=0)
    : _CORBA_Unbounded_Sequence_Octet(max, len, val, rel) {}
  inline ReferenceData& operator = (const ReferenceData& seq) {
    _CORBA_Unbounded_Sequence_Octet::operator=(seq);
    return *this;
  };
};


class BOA : public Object {
public:
  virtual void impl_is_ready(ImplementationDef_ptr p=0, Boolean NoBlk=0) = 0;
  virtual void impl_shutdown() = 0;
  virtual void destroy() = 0;

  virtual void obj_is_ready(omniOrbBoaServant*, 
			    ImplementationDef_ptr p=0)=0;
  virtual void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0)=0;
  virtual void dispose(Object_ptr) = 0;

  virtual Object_ptr create(const ReferenceData&, _objref_InterfaceDef*,
			    ImplementationDef_ptr) = 0;
  virtual ReferenceData* get_id(Object_ptr) = 0;
  virtual void change_implementation(Object_ptr,ImplementationDef_ptr) = 0;
  virtual Principal_ptr get_principal(Object_ptr, Environment_ptr) = 0;
  virtual void deactivate_impl(ImplementationDef_ptr) = 0;
  virtual void deactivate_obj(Object_ptr) = 0;

  static BOA_ptr _duplicate(BOA_ptr);
  static BOA_ptr _narrow(Object_ptr);
  static BOA_ptr _nil();
  static BOA_ptr getBOA();

  static _core_attr const char* _PD_repoId;

protected:
  inline BOA(int nil) { _PR_setobj((omniObjRef*) (nil ? 0:1)); }
  virtual ~BOA();

private:
  BOA(const BOA&);
  BOA& operator=(const BOA&);
};
