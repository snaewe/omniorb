// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_Context.h            Created on: 2001/08/17
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
//    CORBA::Context, ContextList
//

/*
  $Log$
  Revision 1.1.2.2  2001/11/06 15:41:34  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.1.2.1  2001/08/17 13:39:44  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
/////////////////////////////// Context //////////////////////////////
//////////////////////////////////////////////////////////////////////

class Context;
typedef Context* Context_ptr;
typedef Context_ptr ContextRef;
typedef _CORBA_PseudoObj_Var<Context> Context_var;
typedef _CORBA_PseudoObj_Out<Context,Context_var> Context_out;

class Context {
public:
  typedef Context_ptr _ptr_type;
  typedef Context_var _var_type;

  virtual ~Context();

  virtual const char* context_name() const = 0;
  virtual CORBA::Context_ptr parent() const = 0;

  virtual void create_child(const char*, Context_out) = 0;
  virtual void set_one_value(const char*, const Any&) = 0;
  virtual void set_values(CORBA::NVList_ptr) = 0;
  virtual void delete_values(const char*) = 0;
  virtual void get_values(const char* start_scope,
			  CORBA::Flags op_flags,
			  const char* pattern,
			  CORBA::NVList_out values) = 0;
  // Throws BAD_CONTEXT if <start_scope> is not found.
  // Returns a nil NVList in <values> if no matches are found.

  virtual Boolean NP_is_nil() const = 0;
  virtual CORBA::Context_ptr NP_duplicate() = 0;

  static Context_ptr _duplicate(Context_ptr);
  static Context_ptr _nil();

  // omniORB specifics.
  static void marshalContext(Context_ptr ctxt, const char*const* which,
			     int whichlen, cdrStream& s);
  static Context_ptr unmarshalContext(cdrStream& s);

  static inline _CORBA_Boolean PR_is_valid(Context_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  Context() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  Context(const Context&);
  Context& operator=(const Context&);
};


//////////////////////////////////////////////////////////////////////
///////////////////////////// ContextList ////////////////////////////
//////////////////////////////////////////////////////////////////////

class ContextList;
typedef ContextList* ContextList_ptr;
typedef ContextList_ptr ContextListRef;
typedef _CORBA_PseudoObj_Var<ContextList> ContextList_var;
typedef _CORBA_PseudoObj_Out<ContextList,ContextList_var> ContextList_out;

class ContextList {
public:
  virtual ~ContextList();

  virtual ULong count() const = 0;
  virtual void add(const char* ctxt) = 0;
  virtual void add_consume(char* ctxt) = 0;
  // consumes ctxt

  virtual const char* item(ULong index) = 0;
  // retains ownership of return value

  virtual void remove(ULong index) = 0;

  virtual Boolean NP_is_nil() const = 0;
  virtual ContextList_ptr NP_duplicate() = 0;

  static ContextList_ptr _duplicate(ContextList_ptr);
  static ContextList_ptr _nil();

  // OMG Interface:

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(Bounds, _dyn_attr)

    static inline _CORBA_Boolean PR_is_valid(ContextList_ptr p ) {
    return ((p) ? (p->pd_magic == PR_magic) : 1);
  }

  static _dyn_attr const _CORBA_ULong PR_magic;

protected:
  ContextList() { pd_magic = PR_magic; }

private:
  _CORBA_ULong pd_magic;

  ContextList(const ContextList&);
  ContextList& operator=(const ContextList&);
};
