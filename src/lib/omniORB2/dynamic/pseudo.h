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

#ifndef __CORBA_H__
#include <omniORB2/CORBA.h>
#endif


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
  virtual ~NamedValueImpl();

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
