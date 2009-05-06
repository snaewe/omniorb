// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA_ORB.h                Created on: 2001/08/17
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2005 Apasphere Ltd
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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
//    CORBA::ORB
//

/*
  $Log$
  Revision 1.1.4.5  2009/05/06 16:16:15  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.4  2005/01/06 23:08:06  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.3  2004/04/02 13:26:25  dgrisby
  Start refactoring TypeCode to support value TypeCodes, start of
  abstract interfaces support.

  Revision 1.1.4.2  2003/09/26 16:12:53  dgrisby
  Start of valuetype support.

  Revision 1.1.4.1  2003/03/23 21:04:24  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.6  2001/11/13 14:11:43  dpg1
  Tweaks for CORBA 2.5 compliance.

  Revision 1.1.2.5  2001/11/06 15:41:34  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.1.2.4  2001/10/29 17:42:35  dpg1
  Support forward-declared structs/unions, ORB::create_recursive_tc().

  Revision 1.1.2.3  2001/10/17 16:44:00  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

  Revision 1.1.2.2  2001/08/17 17:00:50  sll
  ORB_init now takes an extra options argument.

  Revision 1.1.2.1  2001/08/17 13:39:47  dpg1
  Split CORBA.h into separate bits.

*/

#ifndef INSIDE_OMNIORB_CORBA_MODULE
#  error "Must only be #included by CORBA.h"
#endif

//////////////////////////////////////////////////////////////////////
///////////////////////////////// ORB ////////////////////////////////
//////////////////////////////////////////////////////////////////////

class ORB;
class ORB_var;
typedef ORB* ORB_ptr;
typedef ORB_ptr ORBRef;

class ORB : public Object {
public:
  typedef ORB_ptr _ptr_type;
  typedef ORB_var _var_type;

  typedef char* OAid;

  typedef char* ObjectId;
  typedef String_var ObjectId_var;

  typedef _CORBA_Pseudo_Unbounded_Sequence<Request,Request_member> RequestSeq;

  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InvalidName, _core_attr)
  OMNIORB_DECLARE_USER_EXCEPTION_IN_CORBA(InconsistentTypeCode, _dyn_attr)

  ///////////////////////
  // ORB::ObjectIdList //
  ///////////////////////

  class ObjectIdList_var;

  class ObjectIdList : public _CORBA_Unbounded_Sequence_String {
  public:
    typedef ObjectIdList_var _var_type;
    inline ObjectIdList() {}
    inline ObjectIdList(const ObjectIdList& seq)
      : _CORBA_Unbounded_Sequence_String(seq) {}
    inline ObjectIdList(ULong max)
      : _CORBA_Unbounded_Sequence_String(max) {}
    inline ObjectIdList(ULong max, ULong len, char** val,
			Boolean rel=0)
      : _CORBA_Unbounded_Sequence_String(max, len, val, rel) {}
    inline ObjectIdList& operator = (const ObjectIdList& seq) {
      _CORBA_Unbounded_Sequence_String::operator=(seq);
      return *this;
    }
  };

  class ObjectIdList_out;

  class ObjectIdList_var {
  public:
    typedef ObjectIdList T;
    typedef ObjectIdList_var T_var;

    inline ObjectIdList_var() : pd_seq(0) {}
    inline ObjectIdList_var(T* s) : pd_seq(s) {}
    inline ObjectIdList_var(const T_var& sv) {
      if( sv.pd_seq ) {
	pd_seq = new T;
	*pd_seq = *sv.pd_seq;
      } else
	pd_seq = 0;
    }
    inline ~ObjectIdList_var() { if( pd_seq ) delete pd_seq; }

    inline T_var& operator = (T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& sv) {
      if( sv.pd_seq ) {
	if( !pd_seq )  pd_seq = new T;
	*pd_seq = *sv.pd_seq;
      } else if( pd_seq ) {
	delete pd_seq;
	pd_seq = 0;
      }
      return *this;
    }

    inline _CORBA_String_element operator [] (_CORBA_ULong i) {   return (*pd_seq)[i]; }
    inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator T& () const { return *pd_seq; }
#else
    inline operator const T& () const { return *pd_seq; }
    inline operator T& () { return *pd_seq; }
#endif

    inline const T& in() const { return *pd_seq; }
    inline T&       inout()    { return *pd_seq; }
    inline T*& out() { if (pd_seq) { delete pd_seq; pd_seq = 0; } return pd_seq; }
    inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }

    friend class ObjectIdList_out;

  private:
    T* pd_seq;
  };

  class ObjectIdList_out {
  public:
    typedef ObjectIdList T;
    typedef ObjectIdList_var T_var;

    inline ObjectIdList_out(T*& s) : _data(s) { _data = 0; }
    inline ObjectIdList_out(T_var& sv)
      : _data(sv.pd_seq) { sv = (T*) 0; }
    inline ObjectIdList_out(const ObjectIdList_out& s) : _data(s._data) {}
    inline ObjectIdList_out& operator=(const ObjectIdList_out& s) {
      _data = s._data; return *this;
    }
    inline ObjectIdList_out& operator=(T* s) { _data = s; return *this; }
    inline operator T*&() { return _data; }
    inline T*& ptr() { return _data; }
    inline T* operator->() { return _data; }
    inline _CORBA_String_element operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }
    T*& _data;

  private:
    ObjectIdList_out();
    ObjectIdList_out operator=(const T_var&);
  };

  /////////////////
  // ORB Methods //
  /////////////////

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(Object_ptr);
  static _ptr_type _nil();

  virtual char* id() = 0;

  virtual char* object_to_string(Object_ptr) = 0;
  virtual Object_ptr string_to_object(const char*) = 0;

  virtual ObjectIdList* list_initial_services() = 0;
  virtual Object_ptr resolve_initial_references(const char* identifier) = 0;

  virtual Boolean work_pending() = 0;
  virtual void perform_work() = 0;
  virtual void run() = 0;
  virtual void shutdown(Boolean wait_for_completion) = 0;
  virtual void destroy() = 0;

  inline void NP_destroy() { destroy(); }
  // Deprecated omniORB specific method.  Use 'destroy()'.

  BOA_ptr BOA_init(int& argc, char** argv, const char* boa_identifier=0);

  void create_list(Long, NVList_out);
  void create_operation_list(_objref_OperationDef*, // OperationDef_ptr
			     NVList_out);
  void create_named_value(NamedValue_out);
  void create_exception_list(ExceptionList_out);
  void create_context_list(ContextList_out);

  void get_default_context(Context_out context_out);
  // Returns a reference to the default context, which should be
  // released when finished with.

  void create_environment(Environment_out);

  void send_multiple_requests_oneway(const RequestSeq&);
  void send_multiple_requests_deferred(const RequestSeq&);
  Boolean poll_next_response();
  void get_next_response(Request_out);

  TypeCode_ptr create_struct_tc(const char* id, const char* name,
				const StructMemberSeq& members);
  TypeCode_ptr create_union_tc(const char* id, const char* name,
			       TypeCode_ptr discriminator_type,
			       const UnionMemberSeq& members);
  TypeCode_ptr create_enum_tc(const char* id, const char* name,
			      const EnumMemberSeq& members);
  TypeCode_ptr create_alias_tc(const char* id, const char* name,
			       TypeCode_ptr original_type);
  TypeCode_ptr create_exception_tc(const char* id, const char* name,
				   const StructMemberSeq& members);
  TypeCode_ptr create_interface_tc(const char* id, const char* name);
  TypeCode_ptr create_string_tc(ULong bound);
  TypeCode_ptr create_wstring_tc(ULong bound);
  TypeCode_ptr create_fixed_tc(UShort digits, Short scale);
  TypeCode_ptr create_sequence_tc(ULong bound,
				  TypeCode_ptr element_type);
  TypeCode_ptr create_recursive_sequence_tc(ULong bound, ULong offset);
  // deprecated

  TypeCode_ptr create_array_tc(ULong length, TypeCode_ptr etype);

  TypeCode_ptr create_value_tc(const char* id, const char* name,
			       ValueModifier type_modifier,
			       TypeCode_ptr concrete_base,
			       const ValueMemberSeq& members);

  TypeCode_ptr create_value_box_tc(const char* id, const char* name,
				   TypeCode_ptr boxed_type);

#if 0
  TypeCode_ptr create_native_tc(const char* id, const char* name);
#endif

  TypeCode_ptr create_recursive_tc(const char* id);

  TypeCode_ptr create_abstract_interface_tc(const char* id, const char* name);

  TypeCode_ptr create_local_interface_tc(const char* id, const char* name);
  

  Policy_ptr create_policy(PolicyType,const Any&);

  ValueFactory register_value_factory(const char* id,
				      ValueFactory factory);

  void unregister_value_factory(const char* id);

  ValueFactory lookup_value_factory(const char* id);

  virtual void register_initial_reference(const char* id, Object_ptr obj) = 0;
    
  // omniORB internal.
  static _core_attr const char* _PD_repoId;
  virtual ~ORB();

protected:
  inline ORB(int nil) {
    if (nil)
      _PR_setobj((omniObjRef*)0);
    else
      _PR_setobj((omniObjRef*)1);
  }

private:
  ORB(const ORB&);
  ORB& operator = (const ORB&);
};

typedef char* ORBid;
typedef String_var ORBid_var;

_CORBA_MODULE_FN ORB_ptr ORB_init(int& argc, char** argv,
				  const char* orb_identifier="",
				  const char* options[][2]=0);
