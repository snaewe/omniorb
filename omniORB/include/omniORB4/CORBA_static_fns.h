// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_static_fns.h         Created on: 2001/08/17
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
//    Static is_nil() and release() functions
//

/*
  $Log$
  Revision 1.1.2.2  2001/10/17 16:44:00  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.1  2001/08/17 13:39:49  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// is_nil ///////////////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_MODULE_FN inline Boolean is_nil(Environment_ptr p) {
  if (!Environment::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("Environment"));
}
_CORBA_MODULE_FN inline Boolean is_nil(Context_ptr p) {
  if (!Context::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("Context"));
}
_CORBA_MODULE_FN Boolean is_nil(Principal_ptr);
_CORBA_MODULE_FN inline Boolean is_nil(NamedValue_ptr p) {
  if (!NamedValue::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("NamedValue"));
}
_CORBA_MODULE_FN inline Boolean is_nil(NVList_ptr p) {
  if (!NVList::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("NVList"));
}
_CORBA_MODULE_FN inline Boolean is_nil(Request_ptr p) {
  if (!Request::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("Request"));
}
_CORBA_MODULE_FN inline Boolean is_nil(ExceptionList_ptr p) {
  if (!ExceptionList::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("ExceptionList"));
}
_CORBA_MODULE_FN inline Boolean is_nil(ContextList_ptr p) {
  if (!ContextList::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("ContextList"));
}
_CORBA_MODULE_FN inline Boolean is_nil(TypeCode_ptr p) {
  if (!TypeCode::PR_is_valid(p)) 
    return 0;
  else
    return (p ? p->NP_is_nil() :
	    _CORBA_use_nil_ptr_as_nil_pseudo_objref("TypeCode"));
}


//////////////////////////////////////////////////////////////////////
////////////////////////////// release ///////////////////////////////
//////////////////////////////////////////////////////////////////////

_CORBA_MODULE_FN void release(Environment_ptr p);
_CORBA_MODULE_FN void release(Context_ptr p);
_CORBA_MODULE_FN void release(Principal_ptr);
_CORBA_MODULE_FN void release(NamedValue_ptr p);
_CORBA_MODULE_FN void release(NVList_ptr p);
_CORBA_MODULE_FN void release(Request_ptr p);
_CORBA_MODULE_FN void release(ExceptionList_ptr p);
_CORBA_MODULE_FN void release(ContextList_ptr p);
_CORBA_MODULE_FN void release(TypeCode_ptr o);
