// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynAny.h                   Created on: 2001/09/25
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Research Cambridge
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
//    DynamicAny module

/*
  $Log$
  Revision 1.1.2.1  2001/10/17 16:44:01  dpg1
  Update DynAny to CORBA 2.5 spec, const Any exception extraction.

*/

#ifndef __OMNIDYNANY_H__
#define __OMNIDYNANY_H__

_CORBA_MODULE DynamicAny
_CORBA_MODULE_BEG

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynAny ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynAny;
  typedef DynAny* DynAny_ptr;

  class DynAny_Helper {
  public:
    typedef DynAny_ptr _ptr_type;

    static _ptr_type _nil();
    static _CORBA_Boolean is_nil(_ptr_type);
    static void release(_ptr_type);
    static void duplicate(_ptr_type);
  };

  class DynAny : public CORBA::Object {
  public:
    typedef DynAny_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynAny> _var_type;

    OMNIORB_DECLARE_USER_EXCEPTION(InvalidValue, _dyn_attr)
    OMNIORB_DECLARE_USER_EXCEPTION(TypeMismatch, _dyn_attr)

    // IDL defined functions
    virtual CORBA::TypeCode_ptr type() const = 0;

    virtual void assign(DynAny_ptr dyn_any) = 0;
    virtual void from_any(const CORBA::Any& value) = 0;
    virtual CORBA::Any* to_any() = 0;

    virtual CORBA::Boolean equal(DynAny_ptr dyn_any) = 0;

    virtual void destroy() = 0;
    virtual DynAny_ptr copy() = 0;

    virtual void insert_boolean   (CORBA::Boolean      value) = 0;
    virtual void insert_octet     (CORBA::Octet        value) = 0;
    virtual void insert_char      (CORBA::Char         value) = 0;
    virtual void insert_short     (CORBA::Short        value) = 0;
    virtual void insert_ushort    (CORBA::UShort       value) = 0;
    virtual void insert_long      (CORBA::Long         value) = 0;
    virtual void insert_ulong     (CORBA::ULong        value) = 0;
#ifndef NO_FLOAT
    virtual void insert_float     (CORBA::Float        value) = 0;
    virtual void insert_double    (CORBA::Double       value) = 0;
#endif
    virtual void insert_string    (const char*         value) = 0;
    virtual void insert_reference (CORBA::Object_ptr   value) = 0;
    virtual void insert_typecode  (CORBA::TypeCode_ptr value) = 0;
#ifdef HAS_LongLong
    virtual void insert_longlong  (CORBA::LongLong     value) = 0;
    virtual void insert_ulonglong (CORBA::ULongLong    value) = 0;
#endif
#ifdef HAS_LongDouble
    virtual void insert_longdouble(CORBA::LongDouble   value) = 0;
#endif
    virtual void insert_wchar     (CORBA::WChar        value) = 0;
    virtual void insert_wstring   (const CORBA::WChar* value) = 0;
    virtual void insert_any       (const CORBA::Any&   value) = 0;
    virtual void insert_dyn_any   (DynAny_ptr          value) = 0;

    virtual CORBA::Boolean      get_boolean()    = 0;
    virtual CORBA::Octet        get_octet()      = 0;
    virtual CORBA::Char         get_char()       = 0;
    virtual CORBA::Short        get_short()      = 0;
    virtual CORBA::UShort       get_ushort()     = 0;
    virtual CORBA::Long         get_long()       = 0;
    virtual CORBA::ULong        get_ulong()      = 0;
#ifndef NO_FLOAT
    virtual CORBA::Float        get_float()      = 0;
    virtual CORBA::Double       get_double()     = 0;
#endif				
    virtual char*               get_string()     = 0;
    virtual CORBA::Object_ptr   get_reference()  = 0;
    virtual CORBA::TypeCode_ptr get_typecode()   = 0;
#ifdef HAS_LongLong
    virtual CORBA::LongLong     get_longlong()   = 0;
    virtual CORBA::ULongLong    get_ulonglong()  = 0;
#endif				
#ifdef HAS_LongDouble
    virtual CORBA::LongDouble   get_longdouble() = 0;
#endif				
    virtual CORBA::WChar  	get_wchar()      = 0;
    virtual CORBA::WChar* 	get_wstring()    = 0;
    virtual CORBA::Any*   	get_any()        = 0;
    virtual DynAny_ptr    	get_dyn_any()    = 0;

    // CORBA 2.5 sequence operations
    virtual void insert_boolean_seq   (CORBA::BooleanSeq&    value) = 0;
    virtual void insert_octet_seq     (CORBA::OctetSeq&      value) = 0;
    virtual void insert_char_seq      (CORBA::CharSeq&       value) = 0;
    virtual void insert_short_seq     (CORBA::ShortSeq&      value) = 0;
    virtual void insert_ushort_seq    (CORBA::UShortSeq&     value) = 0;
    virtual void insert_long_seq      (CORBA::LongSeq&       value) = 0;
    virtual void insert_ulong_seq     (CORBA::ULongSeq&      value) = 0;
#ifndef NO_FLOAT
    virtual void insert_float_seq     (CORBA::FloatSeq&      value) = 0;
    virtual void insert_double_seq    (CORBA::DoubleSeq&     value) = 0;
#endif
#ifdef HAS_LongLong
    virtual void insert_longlong_seq  (CORBA::LongLongSeq&   value) = 0;
    virtual void insert_ulonglong_seq (CORBA::ULongLongSeq&  value) = 0;
#endif
#ifdef HAS_LongDouble
    virtual void insert_longdouble_seq(CORBA::LongDoubleSeq& value) = 0;
#endif
    virtual void insert_wchar_seq     (CORBA::WCharSeq&      value) = 0;

    virtual CORBA::BooleanSeq*    get_boolean_seq()    = 0;
    virtual CORBA::OctetSeq*      get_octet_seq()      = 0;
    virtual CORBA::CharSeq*       get_char_seq()       = 0;
    virtual CORBA::ShortSeq*      get_short_seq()      = 0;
    virtual CORBA::UShortSeq*     get_ushort_seq()     = 0;
    virtual CORBA::LongSeq*       get_long_seq()       = 0;
    virtual CORBA::ULongSeq* 	  get_ulong_seq()      = 0;
#ifndef NO_FLOAT
    virtual CORBA::FloatSeq* 	  get_float_seq()      = 0;
    virtual CORBA::DoubleSeq*     get_double_seq()     = 0;
#endif
#ifdef HAS_LongLong
    virtual CORBA::LongLongSeq*   get_longlong_seq()   = 0;
    virtual CORBA::ULongLongSeq*  get_ulonglong_seq()  = 0;
#endif
#ifdef HAS_LongDouble
    virtual CORBA::LongDoubleSeq* get_longdouble_seq() = 0;
#endif
    virtual CORBA::WCharSeq*      get_wchar_seq()      = 0;


    virtual CORBA::Boolean seek(CORBA::Long index) = 0;
    virtual void rewind() = 0;
    virtual CORBA::Boolean next() = 0;
    virtual CORBA::ULong component_count() = 0;
    virtual DynAny_ptr current_component() = 0;

    static DynAny_ptr _duplicate(DynAny_ptr);
    static DynAny_ptr _narrow(CORBA::Object_ptr);
    static DynAny_ptr _nil();

    virtual int NP_nodetype() const = 0;
    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char*        _PD_repoId;
    static _dyn_attr const CORBA::ULong PR_magic;

    static inline CORBA::Boolean PR_is_valid(const DynAny* p) {
      return p ? (p->pd_dynmagic == PR_magic) : 1;
    }

  protected:
    inline DynAny(int is_nil = 0) : pd_dynmagic(PR_magic) {
      _PR_setobj((omniObjRef*)(is_nil ? 0:1));
    }
    virtual ~DynAny();

  private:
    DynAny(const DynAny&);
    DynAny& operator=(const DynAny&);

    CORBA::ULong pd_dynmagic;
  };

  typedef DynAny::_var_type DynAny_var;
  typedef _CORBA_PseudoObj_Member<DynAny,DynAny_var> DynAny_member;
  typedef _CORBA_PseudoObj_Out<DynAny,DynAny_var>    DynAny_out;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynFixed /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynFixed;
  typedef DynFixed* DynFixed_ptr;

  class DynFixed : public virtual DynAny {
  public:
    typedef DynFixed_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynFixed> _var_type;

    virtual char* get_value() = 0;
    virtual CORBA::Boolean set_value(const char* val) = 0;

    static DynFixed_ptr _duplicate(DynFixed_ptr);
    static DynFixed_ptr _narrow(CORBA::Object_ptr);
    static DynFixed_ptr _nil();

    virtual ~DynFixed();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynFixed() {}
  private:
    DynFixed(const DynFixed&);
    DynFixed& operator=(const DynFixed&);
  };

  typedef DynFixed::_var_type DynFixed_var;
  typedef _CORBA_PseudoObj_Member<DynFixed,DynFixed_var> DynFixed_member;
  typedef _CORBA_PseudoObj_Out<DynFixed,DynFixed_var>    DynFixed_out;


  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// DynEnum //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynEnum;
  typedef DynEnum* DynEnum_ptr;

  class DynEnum :  public virtual DynAny {
  public:
    typedef DynEnum_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynEnum> _var_type;

    virtual char* get_as_string() = 0;
    virtual void set_as_string(const char* value) = 0;
    virtual CORBA::ULong get_as_ulong() = 0;
    virtual void set_as_ulong(CORBA::ULong value) = 0;

    static DynEnum_ptr _duplicate(DynEnum_ptr);
    static DynEnum_ptr _narrow(CORBA::Object_ptr);
    static DynEnum_ptr _nil();

    virtual ~DynEnum();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynEnum() {}
  private:
    DynEnum(const DynEnum&);
    DynEnum& operator=(const DynEnum&);
  };

  typedef DynEnum::_var_type DynEnum_var;
  typedef _CORBA_PseudoObj_Member<DynEnum,DynEnum_var> DynEnum_member;
  typedef _CORBA_PseudoObj_Out<DynEnum,DynEnum_var>    DynEnum_out;


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynStruct /////////////////////////////
  //////////////////////////////////////////////////////////////////////

  typedef char* FieldName;
  typedef CORBA::String_var FieldName_var;
  typedef CORBA::String_out FieldName_out;

  struct NameValuePair {
    typedef _CORBA_ConstrType_Variable_Var<NameValuePair> _var_type;
    CORBA::String_member id;
    CORBA::Any value;
  };

  typedef NameValuePair::_var_type NameValuePair_var;
  typedef _CORBA_ConstrType_Variable_OUT_arg<NameValuePair,NameValuePair_var > NameValuePair_out;

  // Generated code for NameValuePairSeq
  class NameValuePairSeq_var;

  class NameValuePairSeq : public _CORBA_Unbounded_Sequence< NameValuePair>  {
  public:
    typedef NameValuePairSeq_var _var_type;
    inline NameValuePairSeq() {}
    inline NameValuePairSeq(const NameValuePairSeq& s)
      : _CORBA_Unbounded_Sequence< NameValuePair> (s) {}

    inline NameValuePairSeq(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence< NameValuePair> (_max) {}
    inline NameValuePairSeq(_CORBA_ULong _max, _CORBA_ULong _len, NameValuePair* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence< NameValuePair> (_max, _len, _val, _rel) {}

    inline NameValuePairSeq& operator = (const NameValuePairSeq& s) {
      _CORBA_Unbounded_Sequence< NameValuePair> ::operator=(s);
      return *this;
    }
  };

  class NameValuePairSeq_out;

  class NameValuePairSeq_var {
  public:
    typedef NameValuePairSeq T;
    typedef NameValuePairSeq_var T_var;
      
    inline NameValuePairSeq_var() : _pd_seq(0) {}
    inline NameValuePairSeq_var(T* s) : _pd_seq(s) {}
    inline NameValuePairSeq_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~NameValuePairSeq_var() { if( _pd_seq )  delete _pd_seq; }
      
    inline T_var& operator = (T* s) {
      if( _pd_seq )  delete _pd_seq;
      _pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s._pd_seq ) {
        if( !_pd_seq )  _pd_seq = new T;
        *_pd_seq = *s._pd_seq;
      } else if( _pd_seq ) {
        delete _pd_seq;
        _pd_seq = 0;
      }
      return *this;
    }

    inline NameValuePair& operator [] (_CORBA_ULong s) {
      return (*_pd_seq)[s];
    }
    inline T* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
    inline operator T& () const { return *_pd_seq; }
#else
    inline operator const T& () const { return *_pd_seq; }
    inline operator T& () { return *_pd_seq; }
#endif
    inline const T& in() const { return *_pd_seq; }
    inline T&       inout()    { return *_pd_seq; }
    inline T*&      out() {
      if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
      return _pd_seq;
    }
    inline T* _retn() { T* tmp = _pd_seq; _pd_seq = 0; return tmp; }
      
    friend class NameValuePairSeq_out;
    
  private:
    T* _pd_seq;
  };

  class NameValuePairSeq_out {
  public:
    typedef NameValuePairSeq T;
    typedef NameValuePairSeq_var T_var;

    inline NameValuePairSeq_out(T*& s) : _data(s) { _data = 0; }
    inline NameValuePairSeq_out(T_var& s)
      : _data(s._pd_seq) { s = (T*) 0; }
    inline NameValuePairSeq_out(const NameValuePairSeq_out& s) : _data(s._data) {}
    inline NameValuePairSeq_out& operator = (const NameValuePairSeq_out& s) {
      _data = s._data;
      return *this;
    }  inline NameValuePairSeq_out& operator = (T* s) {
      _data = s;
      return *this;
    }
    inline operator T*&()  { return _data; }
    inline T*& ptr()       { return _data; }
    inline T* operator->() { return _data; }

    inline NameValuePair& operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }
    T*& _data;

  private:
    NameValuePairSeq_out();
    NameValuePairSeq_out& operator=(const T_var&);
  };

  // End of generated code


  struct NameDynAnyPair {
    typedef _CORBA_ConstrType_Variable_Var<NameDynAnyPair> _var_type;
    CORBA::String_member id;
    DynAny_member value;
  };

  typedef NameDynAnyPair::_var_type NameDynAnyPair_var;
  typedef _CORBA_ConstrType_Variable_OUT_arg<NameDynAnyPair,NameDynAnyPair_var> NameDynAnyPair_out;

  // Generated code for NameDynAnyPairSeq

  class NameDynAnyPairSeq_var;

  class NameDynAnyPairSeq : public _CORBA_Unbounded_Sequence< NameDynAnyPair>  {
  public:
    typedef NameDynAnyPairSeq_var _var_type;
    inline NameDynAnyPairSeq() {}
    inline NameDynAnyPairSeq(const NameDynAnyPairSeq& s)
      : _CORBA_Unbounded_Sequence< NameDynAnyPair> (s) {}

    inline NameDynAnyPairSeq(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence< NameDynAnyPair> (_max) {}
    inline NameDynAnyPairSeq(_CORBA_ULong _max, _CORBA_ULong _len, NameDynAnyPair* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence< NameDynAnyPair> (_max, _len, _val, _rel) {}

    inline NameDynAnyPairSeq& operator = (const NameDynAnyPairSeq& s) {
      _CORBA_Unbounded_Sequence< NameDynAnyPair> ::operator=(s);
      return *this;
    }
  };

  class NameDynAnyPairSeq_out;

  class NameDynAnyPairSeq_var {
  public:
    typedef NameDynAnyPairSeq T;
    typedef NameDynAnyPairSeq_var T_var;
      
    inline NameDynAnyPairSeq_var() : _pd_seq(0) {}
    inline NameDynAnyPairSeq_var(T* s) : _pd_seq(s) {}
    inline NameDynAnyPairSeq_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~NameDynAnyPairSeq_var() { if( _pd_seq )  delete _pd_seq; }
      
    inline T_var& operator = (T* s) {
      if( _pd_seq )  delete _pd_seq;
      _pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s._pd_seq ) {
        if( !_pd_seq )  _pd_seq = new T;
        *_pd_seq = *s._pd_seq;
      } else if( _pd_seq ) {
        delete _pd_seq;
        _pd_seq = 0;
      }
      return *this;
    }

    inline NameDynAnyPair& operator [] (_CORBA_ULong s) {
      return (*_pd_seq)[s];
    }
    inline T* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
    inline operator T& () const { return *_pd_seq; }
#else
    inline operator const T& () const { return *_pd_seq; }
    inline operator T& () { return *_pd_seq; }
#endif
    inline const T& in() const { return *_pd_seq; }
    inline T&       inout()    { return *_pd_seq; }
    inline T*&      out() {
      if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
      return _pd_seq;
    }
    inline T* _retn() { T* tmp = _pd_seq; _pd_seq = 0; return tmp; }
      
    friend class NameDynAnyPairSeq_out;
    
  private:
    T* _pd_seq;
  };

  class NameDynAnyPairSeq_out {
  public:
    typedef NameDynAnyPairSeq T;
    typedef NameDynAnyPairSeq_var T_var;

    inline NameDynAnyPairSeq_out(T*& s) : _data(s) { _data = 0; }
    inline NameDynAnyPairSeq_out(T_var& s)
      : _data(s._pd_seq) { s = (T*) 0; }
    inline NameDynAnyPairSeq_out(const NameDynAnyPairSeq_out& s) : _data(s._data) {}
    inline NameDynAnyPairSeq_out& operator = (const NameDynAnyPairSeq_out& s) {
      _data = s._data;
      return *this;
    }  inline NameDynAnyPairSeq_out& operator = (T* s) {
      _data = s;
      return *this;
    }
    inline operator T*&()  { return _data; }
    inline T*& ptr()       { return _data; }
    inline T* operator->() { return _data; }

    inline NameDynAnyPair& operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }
    T*& _data;
    
  private:
    NameDynAnyPairSeq_out();
    NameDynAnyPairSeq_out& operator=(const T_var&);
  };

  // End of generated code

  

  class DynStruct;
  typedef DynStruct* DynStruct_ptr;

  class DynStruct :  public virtual DynAny {
  public:
    typedef DynStruct_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynStruct> _var_type;

    virtual char* current_member_name() = 0;
    virtual CORBA::TCKind current_member_kind() = 0;
    virtual NameValuePairSeq* get_members() = 0;
    virtual void set_members(const NameValuePairSeq& value) = 0;
    virtual NameDynAnyPairSeq* get_members_as_dyn_any() = 0;
    virtual void set_members_as_dyn_any(const NameDynAnyPairSeq& value) = 0;

    static DynStruct_ptr _duplicate(DynStruct_ptr);
    static DynStruct_ptr _narrow(CORBA::Object_ptr);
    static DynStruct_ptr _nil();

    virtual ~DynStruct();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynStruct() {}
  private:
    DynStruct(const DynStruct&);
    DynStruct& operator=(const DynStruct&);
  };

  typedef DynStruct::_var_type DynStruct_var;
  typedef _CORBA_PseudoObj_Member<DynStruct,DynStruct_var> DynStruct_member;
  typedef _CORBA_PseudoObj_Out<DynStruct,DynStruct_var>    DynStruct_out;


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynUnion //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynUnion;
  typedef DynUnion* DynUnion_ptr;

  class DynUnion :  public virtual DynAny {
  public:
    typedef DynUnion_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynUnion> _var_type;

    virtual DynAny_ptr get_discriminator() = 0;
    virtual void set_discriminator(DynAny_ptr d) = 0;
    virtual void set_to_default_member() = 0;
    virtual void set_to_no_active_member() = 0;
    virtual CORBA::Boolean has_no_active_member() = 0;
    virtual CORBA::TCKind discriminator_kind() = 0;
    virtual DynAny_ptr member() = 0;
    virtual char*  member_name() = 0;
    virtual CORBA::TCKind member_kind() = 0;
    virtual CORBA::Boolean is_set_to_default_member() = 0;

    static DynUnion_ptr _duplicate(DynUnion_ptr);
    static DynUnion_ptr _narrow(CORBA::Object_ptr);
    static DynUnion_ptr _nil();

    virtual ~DynUnion();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynUnion() {}
  private:
    DynUnion(const DynUnion&);
    DynUnion& operator=(const DynUnion&);
  };

  typedef DynUnion::_var_type DynUnion_var;
  typedef _CORBA_PseudoObj_Member<DynUnion,DynUnion_var> DynUnion_member;
  typedef _CORBA_PseudoObj_Out<DynUnion,DynUnion_var>    DynUnion_out;


  //////////////////////////////////////////////////////////////////////
  ///////////////////////////// DynSequence ////////////////////////////
  //////////////////////////////////////////////////////////////////////

  // Generated code for AnySeq

  class AnySeq_var;

  class AnySeq : public _CORBA_Unbounded_Sequence< CORBA::Any>  {
  public:
    typedef AnySeq_var _var_type;
    inline AnySeq() {}
    inline AnySeq(const AnySeq& s)
      : _CORBA_Unbounded_Sequence< CORBA::Any> (s) {}

    inline AnySeq(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence< CORBA::Any> (_max) {}
    inline AnySeq(_CORBA_ULong _max, _CORBA_ULong _len, CORBA::Any* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence< CORBA::Any> (_max, _len, _val, _rel) {}

    inline AnySeq& operator = (const AnySeq& s) {
      _CORBA_Unbounded_Sequence< CORBA::Any> ::operator=(s);
      return *this;
    }
  };

  class AnySeq_out;

  class AnySeq_var {
  public:
    typedef AnySeq T;
    typedef AnySeq_var T_var;
    
    inline AnySeq_var() : _pd_seq(0) {}
    inline AnySeq_var(T* s) : _pd_seq(s) {}
    inline AnySeq_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~AnySeq_var() { if( _pd_seq )  delete _pd_seq; }
    
    inline T_var& operator = (T* s) {
      if( _pd_seq )  delete _pd_seq;
      _pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s._pd_seq ) {
	if( !_pd_seq )  _pd_seq = new T;
	*_pd_seq = *s._pd_seq;
      } else if( _pd_seq ) {
	delete _pd_seq;
	_pd_seq = 0;
      }
      return *this;
    }

    inline CORBA::Any& operator [] (_CORBA_ULong s) {
      return (*_pd_seq)[s];
    }
    inline T* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
    inline operator T& () const { return *_pd_seq; }
#else
    inline operator const T& () const { return *_pd_seq; }
    inline operator T& () { return *_pd_seq; }
#endif
    inline const T& in() const { return *_pd_seq; }
    inline T&       inout()    { return *_pd_seq; }
    inline T*&      out() {
      if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
      return _pd_seq;
    }
    inline T* _retn() { T* tmp = _pd_seq; _pd_seq = 0; return tmp; }
    
    friend class AnySeq_out;
  
  private:
    T* _pd_seq;
  };

  class AnySeq_out {
  public:
    typedef AnySeq T;
    typedef AnySeq_var T_var;

    inline AnySeq_out(T*& s) : _data(s) { _data = 0; }
    inline AnySeq_out(T_var& s)
      : _data(s._pd_seq) { s = (T*) 0; }
    inline AnySeq_out(const AnySeq_out& s) : _data(s._data) {}
    inline AnySeq_out& operator = (const AnySeq_out& s) {
      _data = s._data;
      return *this;
    }  inline AnySeq_out& operator = (T* s) {
      _data = s;
      return *this;
    }
    inline operator T*&()  { return _data; }
    inline T*& ptr()       { return _data; }
    inline T* operator->() { return _data; }

    inline CORBA::Any& operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }
    T*& _data;
  private:
    AnySeq_out();
    AnySeq_out& operator=(const T_var&);
  };

  // End of generated code


  // Generated code for DynAnySeq

  class DynAnySeq_var;

  class DynAnySeq : public _CORBA_Unbounded_Sequence_ObjRef< DynAny, _CORBA_ObjRef_Element< DynAny, DynAny_Helper> , DynAny_Helper>  {
  public:
    typedef DynAnySeq_var _var_type;
    inline DynAnySeq() {}
    inline DynAnySeq(const DynAnySeq& s)
      : _CORBA_Unbounded_Sequence_ObjRef< DynAny, _CORBA_ObjRef_Element< DynAny, DynAny_Helper> , DynAny_Helper> (s) {}

    inline DynAnySeq(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence_ObjRef< DynAny, _CORBA_ObjRef_Element< DynAny, DynAny_Helper> , DynAny_Helper> (_max) {}
    inline DynAnySeq(_CORBA_ULong _max, _CORBA_ULong _len, DynAny_ptr* _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence_ObjRef< DynAny, _CORBA_ObjRef_Element< DynAny, DynAny_Helper> , DynAny_Helper> (_max, _len, _val, _rel) {}

    inline DynAnySeq& operator = (const DynAnySeq& s) {
      _CORBA_Unbounded_Sequence_ObjRef< DynAny, _CORBA_ObjRef_Element< DynAny, DynAny_Helper> , DynAny_Helper> ::operator=(s);
      return *this;
    }
  };

  class DynAnySeq_out;

  class DynAnySeq_var {
  public:
    typedef DynAnySeq T;
    typedef DynAnySeq_var T_var;
      
    inline DynAnySeq_var() : _pd_seq(0) {}
    inline DynAnySeq_var(T* s) : _pd_seq(s) {}
    inline DynAnySeq_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~DynAnySeq_var() { if( _pd_seq )  delete _pd_seq; }
      
    inline T_var& operator = (T* s) {
      if( _pd_seq )  delete _pd_seq;
      _pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s._pd_seq ) {
        if( !_pd_seq )  _pd_seq = new T;
        *_pd_seq = *s._pd_seq;
      } else if( _pd_seq ) {
        delete _pd_seq;
        _pd_seq = 0;
      }
      return *this;
    }

    inline _CORBA_ObjRef_Element< DynAny, DynAny_Helper>  operator [] (_CORBA_ULong s) {
      return (*_pd_seq)[s];
    }
    inline T* operator -> () { return _pd_seq; }
#if defined(__GNUG__)
    inline operator T& () const { return *_pd_seq; }
#else
    inline operator const T& () const { return *_pd_seq; }
    inline operator T& () { return *_pd_seq; }
#endif
    inline const T& in() const { return *_pd_seq; }
    inline T&       inout()    { return *_pd_seq; }
    inline T*&      out() {
      if( _pd_seq ) { delete _pd_seq; _pd_seq = 0; }
      return _pd_seq;
    }
    inline T* _retn() { T* tmp = _pd_seq; _pd_seq = 0; return tmp; }
      
    friend class DynAnySeq_out;
    
  private:
    T* _pd_seq;
  };

  class DynAnySeq_out {
  public:
    typedef DynAnySeq T;
    typedef DynAnySeq_var T_var;

    inline DynAnySeq_out(T*& s) : _data(s) { _data = 0; }
    inline DynAnySeq_out(T_var& s)
      : _data(s._pd_seq) { s = (T*) 0; }
    inline DynAnySeq_out(const DynAnySeq_out& s) : _data(s._data) {}
    inline DynAnySeq_out& operator = (const DynAnySeq_out& s) {
      _data = s._data;
      return *this;
    }  inline DynAnySeq_out& operator = (T* s) {
      _data = s;
      return *this;
    }
    inline operator T*&()  { return _data; }
    inline T*& ptr()       { return _data; }
    inline T* operator->() { return _data; }

    inline _CORBA_ObjRef_Element< DynAny, DynAny_Helper>  operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }

    T*& _data;
    
  private:
    DynAnySeq_out();
    DynAnySeq_out& operator=(const T_var&);
  };


  // End of generated code  


  class DynSequence;
  typedef DynSequence* DynSequence_ptr;
  
  class DynSequence :  public virtual DynAny {
  public:
    typedef DynSequence_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynSequence> _var_type;

    virtual CORBA::ULong get_length() = 0;
    virtual void set_length (CORBA::ULong len) = 0;
    virtual AnySeq* get_elements() = 0;
    virtual void set_elements(const AnySeq& value) = 0;
    virtual DynAnySeq* get_elements_as_dyn_any() = 0;
    virtual void set_elements_as_dyn_any(const DynAnySeq& value) = 0;

    static DynSequence_ptr _duplicate(DynSequence_ptr);
    static DynSequence_ptr _narrow(CORBA::Object_ptr);
    static DynSequence_ptr _nil();

    virtual ~DynSequence();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynSequence() {}

  private:
    DynSequence(const DynSequence&);
    DynSequence& operator=(const DynSequence&);
  };

  typedef DynSequence::_var_type DynSequence_var;
  typedef _CORBA_PseudoObj_Member<DynSequence,DynSequence_var>
                                                            DynSequence_member;
  typedef _CORBA_PseudoObj_Out<DynSequence,DynSequence_var> DynSequence_out;


  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynArray //////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynArray;
  typedef DynArray* DynArray_ptr;

  class DynArray : public virtual DynAny {
  public:
    typedef DynArray_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynArray> _var_type;

    virtual AnySeq* get_elements() = 0;
    virtual void set_elements(const AnySeq& value) = 0;
    virtual DynAnySeq* get_elements_as_dyn_any() = 0;
    virtual void set_elements_as_dyn_any(const DynAnySeq& value) = 0;

    static DynArray_ptr _duplicate(DynArray_ptr);
    static DynArray_ptr _narrow(CORBA::Object_ptr);
    static DynArray_ptr _nil();

    virtual ~DynArray();

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

  protected:
    DynArray() {}

  private:
    DynArray(const DynArray&);
    DynArray& operator=(const DynArray&);
  };

  typedef DynArray::_var_type DynArray_var;
  typedef _CORBA_PseudoObj_Member<DynArray,DynArray_var> DynArray_member;
  typedef _CORBA_PseudoObj_Out<DynArray,DynArray_var>    DynArray_out;



  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// DynAnyFactory /////////////////////////
  //////////////////////////////////////////////////////////////////////

  class DynAnyFactory;
  typedef DynAnyFactory* DynAnyFactory_ptr;

  class DynAnyFactory : public CORBA::Object {
  public:
    typedef DynAnyFactory_ptr _ptr_type;
    typedef _CORBA_PseudoObj_Var<DynAnyFactory> _var_type;

    OMNIORB_DECLARE_USER_EXCEPTION(InconsistentTypeCode, _dyn_attr)

    // IDL defined functions
    virtual DynAny_ptr create_dyn_any(const CORBA::Any& value) = 0;
    virtual DynAny_ptr create_dyn_any_from_type_code(CORBA::TypeCode_ptr type) = 0;

    static DynAnyFactory_ptr _duplicate(DynAnyFactory_ptr);
    static DynAnyFactory_ptr _narrow(CORBA::Object_ptr);
    static DynAnyFactory_ptr _nil();

    // omniORB internal.
    static _dyn_attr const char* _PD_repoId;

    virtual void* _ptrToObjRef(const char* repoId) = 0;

    inline DynAnyFactory(int is_nil = 0) {
      _PR_setobj((omniObjRef*)(is_nil ? 0:1));
    }
    virtual ~DynAnyFactory();

  private:
    DynAnyFactory(const DynAnyFactory&);
    DynAnyFactory& operator=(const DynAnyFactory&);
  };

  typedef DynAnyFactory::_var_type DynAnyFactory_var;
  typedef _CORBA_PseudoObj_Member<DynAnyFactory,DynAnyFactory_var> DynAnyFactory_member;
  typedef _CORBA_PseudoObj_Out<DynAnyFactory,DynAnyFactory_var> DynAnyFactory_out;


_CORBA_MODULE_END  // DynamicAny


#endif // __OMNIDYNANY_H__
