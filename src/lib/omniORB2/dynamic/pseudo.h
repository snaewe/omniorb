// -*- Mode: C++; -*-
//                            Package   : omniORB2
// pseudo.h                   Created on: 9/1998
//                            Author    : David Riddoch (djr)
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
//   This file provides concrete versions of the abstract interfaces
//   to the DII pseudo object types defined in CORBA.h.
//

#ifndef __PSEUDO_H__
#define __PSEUDO_H__

#include <omniORB2/CORBA.h>
#include <deferredRequest.h>


//?? Might it be better to use omnivectors instead of some
// of the sequences used in this file?
//  But don't forget bounds checking ...


//////////////////////////////////////////////////////////////////////
/////////////////////////// OmniPseudoBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

//: Base class for (implementation of) pseudo objects.

class PseudoObjBase {
public:
  PseudoObjBase() : pd_refCount(1) {}
  virtual ~PseudoObjBase();

  void incrRefCount() { pd_refCount++; }
  void decrRefCount();

private:
  PseudoObjBase(const PseudoObjBase&);             // not implemented
  PseudoObjBase& operator=(const PseudoObjBase&);  // not implemented

  unsigned pd_refCount;
};

//////////////////////////////////////////////////////////////////////
///////////////////////////// NamedValue /////////////////////////////
//////////////////////////////////////////////////////////////////////

class NamedValueImpl : public CORBA::NamedValue,
		       public PseudoObjBase
{
public:
  NamedValueImpl(CORBA::Flags flags);
  NamedValueImpl(const char* name, CORBA::Flags flags);
  NamedValueImpl(const char* name, const CORBA::Any& value,
		 CORBA::Flags flags);

  NamedValueImpl(char* name, CORBA::Flags flags);
  // Consumes <name>.

  NamedValueImpl(char* name, CORBA::Any* value, CORBA::Flags flags);
  // Consumes <name> and <value>.

  virtual const char* name() const;
  virtual CORBA::Any* value() const;
  virtual CORBA::Flags flags() const;
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::NamedValue_ptr NP_duplicate();

private:
  CORBA::Flags      pd_flags;
  CORBA::String_var pd_name;
  CORBA::Any_var    pd_value;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////////// NVList ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class NVListImpl : public CORBA::NVList, public PseudoObjBase {
public:
  NVListImpl();
  virtual ~NVListImpl();

  virtual CORBA::ULong count() const;
  virtual CORBA::NamedValue_ptr add(CORBA::Flags);
  virtual CORBA::NamedValue_ptr add_item(const char*, CORBA::Flags);
  virtual CORBA::NamedValue_ptr add_value(const char*, const CORBA::Any&,
					  CORBA::Flags);
  virtual CORBA::NamedValue_ptr add_item_consume(char*, CORBA::Flags);
  virtual CORBA::NamedValue_ptr add_value_consume(char*, CORBA::Any*,
						  CORBA::Flags);
  virtual CORBA::NamedValue_ptr item(CORBA::ULong);
  virtual CORBA::Status remove(CORBA::ULong);
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::NVList_ptr NP_duplicate();

private:
  typedef _CORBA_Pseudo_Unbounded_Sequence<CORBA::NamedValue_ptr> NVSeq;
  NVSeq pd_list;
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// Context ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class ContextImpl : public CORBA::Context {
public:
  ContextImpl(const char* name, CORBA::Context_ptr parent);
  virtual ~ContextImpl();

  virtual const char* context_name() const;
  virtual CORBA::Context_ptr parent() const;
  virtual CORBA::Status create_child(const char*, CORBA::Context_out);
  virtual CORBA::Status set_one_value(const char*, const CORBA::Any&);
  virtual CORBA::Status set_values(CORBA::NVList_ptr);
  virtual CORBA::Status delete_values(const char*);
  virtual CORBA::Status get_values(const char* start_scope,
				   CORBA::Flags op_flags,
				   const char* pattern,
				   CORBA::NVList_out values);
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::Context_ptr NP_duplicate();

  ///////////////////
  // omni internal //
  ///////////////////
  const char* lookup_single(const char* name) const;
  // Lookup single entry, starting at current scope, and looking in
  // parent/grandparents etc. if necassary.
  //  Retains ownership of the return value. Returns 0 if not found.

  void insert_single_consume(char* name, char* value);
  // Inserts a single entry, consuming the given name/value pair.

  void decrRefCount();
  // Decrease the reference count, and delete this context
  // if there are no more references or children.
  //  Must not hold <pd_lock>.

private:
  ContextImpl(const ContextImpl&);             // not implemented
  ContextImpl& operator=(const ContextImpl&);  // not implemented

  int matchPattern(const char* pat, CORBA::ULong& bottom,
		   CORBA::ULong& top) const;
  // Returns 1 if finds a match, or 0 if no match.
  // The index of the first match is in <bottom>, and the last
  // match + 1 in <top>.
  //  Must hold <pd_lock>.

  static void add_values(ContextImpl* c, CORBA::Flags op_flags,
			 const char* pattern, int wildcard,
			 CORBA::NVList_ptr val_list);
  // If( wildcard ), then pattern has terminating '*', and all matching
  // name-value pairs are added to val_list. Recurses on parent of context
  // if allowed by op_flags.
  //  If( !wildcard ), looks for a single match starting at the given
  // context, looking in parent if it is not found and op_flags allows.

  void check_context_name(const char* n);
  void check_property_name(const char* n);
  // These check that the given (non-null) identifier is a valid context
  // name or property name respectively.

  void addChild(ContextImpl* c) {
    omni_mutex_lock lock(pd_lock);
    c->pd_nextSibling = pd_children;
    pd_children = c;
  }
  // Add the given ContextImpl into the list of children.

  void loseChild(ContextImpl* child);
  // The given child is dying - remove from the list of dependents. This
  // must only be called by a child of this context from its d'tor.

  struct Entry {
    char* name;
    char* value;
  };
  typedef _CORBA_Pseudo_Unbounded_Sequence<Entry> EntrySeq;

  CORBA::String_var  pd_name;         // set once - never changes
  CORBA::Context_ptr pd_parent;       // set once - never changes
  EntrySeq           pd_entries;      // sorted list of entries
  ContextImpl*       pd_children;     // list of Ctxts which depend on this
  ContextImpl*       pd_nextSibling;  // linked list of siblings
  unsigned           pd_refCount;

  omni_mutex         pd_lock;
  // Manages access to <pd_entries>, <pd_children>, <pd_refCount> and the
  // <pd_nextSibling> pointers in its children.
};

//////////////////////////////////////////////////////////////////////
////////////////////////////// Request ///////////////////////////////
//////////////////////////////////////////////////////////////////////

class RequestImpl : public CORBA::Request, public PseudoObjBase {
public:
  RequestImpl(CORBA::Object_ptr target, const char* operation);

  RequestImpl(CORBA::Object_ptr target, const char* operation,
	      CORBA::Context_ptr context, CORBA::NVList_ptr arguments,
	      CORBA::NamedValue_ptr result);

  RequestImpl(CORBA::Object_ptr target, const char* operation,
	      CORBA::Context_ptr context, CORBA::NVList_ptr arguments,
	      CORBA::NamedValue_ptr result,
	      CORBA::ExceptionList_ptr exceptions,
	      CORBA::ContextList_ptr contexts);

  virtual ~RequestImpl();

  virtual CORBA::Object_ptr        target() const;
  virtual const char*              operation() const;
  virtual CORBA::NVList_ptr        arguments();
  virtual CORBA::NamedValue_ptr    result();
  virtual CORBA::Environment_ptr   env();
  virtual CORBA::ExceptionList_ptr exceptions();
  virtual CORBA::ContextList_ptr   contexts();
  virtual CORBA::Context_ptr       ctxt() const;
  virtual void                     ctx(CORBA::Context_ptr);
  virtual CORBA::Any& add_in_arg();
  virtual CORBA::Any& add_in_arg(const char* name);
  virtual CORBA::Any& add_inout_arg();
  virtual CORBA::Any& add_inout_arg(const char* name);
  virtual CORBA::Any& add_out_arg();
  virtual CORBA::Any& add_out_arg(const char* name);
  virtual void        set_return_type(CORBA::TypeCode_ptr tc);
  virtual CORBA::Any& return_value();
  virtual CORBA::Status  invoke();
  virtual CORBA::Status  send_oneway();
  virtual CORBA::Status  send_deferred();
  virtual CORBA::Status  get_response();
  virtual CORBA::Boolean poll_response();
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::Request_ptr NP_duplicate();

private:
  CORBA::ULong calculateArgDataSize(CORBA::ULong msize);
  void marshalArgs(GIOP_C& giop_client);
  void marshalContext(GIOP_C& giop_client);
  void unmarshalArgs(GIOP_C& giop_client);

  enum State {
    RS_READY,
    RS_DONE,
    RS_DEFERRED
  };

  CORBA::Object_var        pd_target;
  CORBA::String_var        pd_operation;
  CORBA::NVList_var        pd_arguments;
  CORBA::Boolean           pd_i_own_arguments;
  CORBA::NamedValue_var    pd_result;
  CORBA::Boolean           pd_i_own_result;
  CORBA::Environment_var   pd_environment;
  CORBA::ExceptionList_var pd_exceptions;  // may be nil
  CORBA::ContextList_var   pd_contexts;    // may be nil
  CORBA::Context_var       pd_context;     // may be nil
  State                    pd_state;
  DeferredRequest*         pd_deferredRequest;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// Environment /////////////////////////////
//////////////////////////////////////////////////////////////////////

class EnvironmentImpl : public CORBA::Environment,
			public PseudoObjBase
{
public:
  EnvironmentImpl() : pd_exception(0) {}
  virtual ~EnvironmentImpl();

  virtual void exception(CORBA::Exception*);
  virtual CORBA::Exception* exception() const;
  virtual void clear();
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::Environment_ptr NP_duplicate();

private:
  CORBA::Exception* pd_exception;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// ContextList /////////////////////////////
//////////////////////////////////////////////////////////////////////

class ContextListImpl : public CORBA::ContextList,
			public PseudoObjBase
{
public:
  ContextListImpl();
  virtual ~ContextListImpl();

  virtual CORBA::ULong count() const;
  virtual void add(const char* ctxt);
  virtual void add_consume(char* ctxt);
  virtual const char* item(CORBA::ULong index);
  virtual CORBA::Status remove(CORBA::ULong index);
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::ContextList_ptr NP_duplicate();

private:
  void free_entries();

  typedef _CORBA_Pseudo_Unbounded_Sequence<char*> ContextSeq;
  ContextSeq pd_list;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// ExceptionList ////////////////////////////
//////////////////////////////////////////////////////////////////////

class ExceptionListImpl : public CORBA::ExceptionList,
			  public PseudoObjBase
{
public:
  ExceptionListImpl();
  virtual ~ExceptionListImpl();

  virtual CORBA::ULong count() const;
  virtual void add(CORBA::TypeCode_ptr tc);
  virtual void add_consume(CORBA::TypeCode_ptr tc);
  virtual CORBA::TypeCode_ptr item(CORBA::ULong index);
  virtual CORBA::Status remove(CORBA::ULong index);
  virtual CORBA::Boolean NP_is_nil() const;
  virtual CORBA::ExceptionList_ptr NP_duplicate();

private:
  void free_entries();

  typedef _CORBA_Pseudo_Unbounded_Sequence<CORBA::TypeCode_ptr> ExceptionSeq;
  ExceptionSeq pd_list;
};


#endif  // __PSEUDO_H__
