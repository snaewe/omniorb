// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA.h                    Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
// 	A complete set of C++ definitions for the CORBA module. The definitions
//	appear within the C++ class named CORBA. This mapping is fully
//      compliant with the CORBA 2.0 specification.
// 

/*
 $Log$
 Revision 1.11  1997/04/23 14:00:47  sll
 Added BOA_var and ORB_var.

 * Revision 1.10  1997/03/26  17:32:38  ewc
 * Runtime converted to Win32 DLL
 *
 * Revision 1.9  1997/03/19  18:54:04  sll
 * Status is now typedef to void*.
 *
 * Revision 1.8  1997/03/14  10:18:29  sll
 * operator->() in T_var types is const. Fixed.
 *
 * Revision 1.7  1997/03/09  14:42:50  sll
 * String_var and Object_var can now be passed directly as arguments
 * to operations that have string and Object as INOUT and OUT parameters.
 *
 * Revision 1.6  1997/02/19  11:12:37  ewc
 * Added support for Windows NT / MSVC++ 4.2
 *
 Revision 1.5  1997/01/30 20:18:23  sll
 Added new member functions in Object_Helper.

 Revision 1.4  1997/01/23 14:55:59  sll
 Removed non-essential template dependency

 * Revision 1.3  1997/01/21  14:49:15  ewc
 * Added support for initial reference interface and also includes
 * the COSS Naming Service Interface.
 *
 * Revision 1.2  1997/01/13  15:06:51  sll
 * Added marshalling routines for CORBA::Object.
 *
 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

*/

#ifndef __CORBA_H__
#define __CORBA_H__

#include <omniORB2/omniInternal.h>

#include <omniORB2/omniORB.h>

class _OMNIORB2_NTDLL_ CORBA {

public:

typedef void* Status;

////////////////////////////////////////////////////////////////////////
//                   Primitive types                                  //
////////////////////////////////////////////////////////////////////////
  
typedef _CORBA_Boolean Boolean;
typedef _CORBA_Char    Char;
typedef _CORBA_Octet   Octet;
typedef _CORBA_Short   Short;
typedef _CORBA_UShort  UShort;
typedef _CORBA_Long    Long;
typedef _CORBA_ULong   ULong;
#ifndef NO_FLOAT
typedef _CORBA_Float   Float;
typedef _CORBA_Double  Double;
#endif

////////////////////////////////////////////////////////////////////////
//                   Type String                                      //
////////////////////////////////////////////////////////////////////////
  
  static char *string_alloc(ULong len);
  static void string_free(char *);
  static char *string_dup(const char *);


  class String_member;
  class String_INOUT_arg;
  class String_OUT_arg;

  class String_var {
  public:
    typedef char* ptr_t;

    inline String_var() {
      _data = 0;
    }

    inline String_var(char *p) {
      _data = p;
    }

    inline String_var(const char* p) {
      if (p) {
	_data = string_alloc((ULong)(strlen(p)+1));
	strcpy(_data,p);
      }
      else
	_data = 0;
    }

    inline String_var(const String_var &s) {
      if ((const char *)s) {
	_data = string_alloc((ULong)(strlen(s)+1));
	strcpy(_data,s);
      }
      else {
	_data = 0;
      }
    }

    String_var(const String_member &s);

    inline ~String_var() {
      if (_data)
	string_free(_data);
    }

    inline String_var &operator= (char *p) {
      if (_data) {
	string_free(_data);
	_data = 0;
      }
      _data = p;
      return *this;
    }

    inline String_var &operator= (const char *p) {
      if (_data) {
	string_free(_data);
	_data = 0;
      }
      if (p) {
	_data = string_alloc((ULong)(strlen(p)+1));
	strcpy(_data,p);
      }
      return *this;
    }

    inline String_var &operator= (const String_var &s) {
      if (_data) {
	string_free(_data);
	_data = 0;
      }
      if ((const char *)s) {
	_data = string_alloc((ULong)(strlen(s)+1));
	strcpy(_data,s);
      }
      return *this;
    }

    String_var &operator= (const String_member &s);

    inline operator char* () const { return _data; }
    inline operator const char* () const { return _data; }

    char &operator[] (ULong index);

    char operator[] (ULong index) const;

    friend class String_INOUT_arg;
    friend class String_OUT_arg;

  private:
    char* _data;
  };

  // omniORB2 private class
  class String_member
  {
  public:
    typedef char* ptr_t;
    inline String_member() {
      _ptr = 0;
    }

    inline ~String_member() {
      if (_ptr) string_free(_ptr);
    }

    inline String_member(const String_member &s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      if (s._ptr) {
	_ptr = string_alloc((ULong)(strlen(s._ptr)+1));
	strcpy(_ptr,s._ptr);
      }
    }

    inline String_member& operator= (char *s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      _ptr = s;
      return *this;
    }

    inline String_member& operator= (const char *s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      if (s) {
	_ptr = string_alloc((ULong)(strlen(s)+1));
	strcpy(_ptr,s);
      }
      return *this;
    }
    

    inline String_member& operator= (const String_member & s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      if (s._ptr) {
	_ptr = string_alloc((ULong)(strlen(s._ptr)+1));
	strcpy(_ptr,s._ptr);
      }
      return *this;
    }

    String_member& operator= (const String_var & s);

    inline operator char* () { return _ptr; }
    inline operator const char*() const { return _ptr; }

    char* _ptr;

    void operator>>= (NetBufferedStream &s) const;
    void operator<<= (NetBufferedStream &s);

    void operator>>= (MemBufferedStream &s) const;
    void operator<<= (MemBufferedStream &s);
    size_t NP_alignedSize(size_t initialoffset) const;
  };

  // omniORB2 private class
  class String_INOUT_arg {
  public:
    inline String_INOUT_arg(char*& p) : _data(p) {}
    inline String_INOUT_arg(String_var& p) : _data(p._data) {}
    inline String_INOUT_arg(String_member& p) : _data(p._ptr) {}
    inline ~String_INOUT_arg() {}

    char*& _data;

  private:
    String_INOUT_arg();
  };

  // omniORB2 private class
  class String_OUT_arg {
  public:
    inline String_OUT_arg(char*& p) : _data(p) { }
    inline String_OUT_arg(String_var& p) : _data(p._data) { p = (char*)0; }
    inline String_OUT_arg(String_member& p) : _data(p._ptr) { p = (char*)0; }
    inline ~String_OUT_arg() {}

    char*& _data;

  private:
    String_OUT_arg();
  };

  //////////////////////////////////////////////////////////////////////
  //    Interface repository types                                    //
  //////////////////////////////////////////////////////////////////////
  typedef char *RepositoryId;
  class InterfaceDef {
  public:
    _CORBA_Unbounded_Sequence_Octet _data;
  private:
    InterfaceDef();
    // Not implemented yet
  };
  typedef class InterfaceDef *InterfaceDef_ptr;

////////////////////////////////////////////////////////////////////////
//                   Type Any                                         //
////////////////////////////////////////////////////////////////////////

  class TypeCode;
  typedef class TypeCode *TypeCode_ptr;
  class Any {
  public:
    Any();
    ~Any();
    Any(const Any&);
    Any(TypeCode_ptr tc, void *value, Boolean release = 0);

    Any &operator=(const Any&);

    void operator<<=(Short);
    void operator<<=(UShort);
    void operator<<=(Long);
    void operator<<=(ULong);
#if !defined(NO_FLOAT)
    void operator<<=(Float);
    void operator<<=(Double);
#endif
    void operator<<=(const Any&);
    void operator<<=(const char*);

    Boolean operator>>=(Short&) const;
    Boolean operator>>=(UShort&) const;
    Boolean operator>>=(Long&) const;
    Boolean operator>>=(ULong&) const;
#if !defined(NO_FLOAT)
    Boolean operator>>=(Float&) const;
    Boolean operator>>=(Double&) const;
#endif
    Boolean operator>>=(Any&) const;
    Boolean operator>>=(char*&) const;
    
    struct from_boolean {
      from_boolean(Boolean b) : val(b) {}
      Boolean val;
    };

    struct from_octet {
      from_octet(Octet b) : val(b) {}
      Octet val;
    };

    struct from_char {
      from_char(Char b) : val(b) {}
      Char val;
    };

    void operator<<=(from_boolean);
    void operator<<=(from_char);
    void operator<<=(from_octet);

    struct to_boolean {
      to_boolean(Boolean &b) : ref(b) {}
      Boolean &ref;
    };

    struct to_char {
      to_char(Char &b) : ref(b) {}
      Char &ref;
    };

    struct to_octet {
      to_octet(Octet &b) : ref(b) {}
      Octet &ref;
    };

    Boolean operator>>=(to_boolean) const;
    Boolean operator>>=(to_char) const;
    Boolean operator>>=(to_octet) const;

    void replace(TypeCode_ptr, void *value, Boolean release = 0);

    TypeCode_ptr type() const;
    const void *value() const;

  private:
    void operator<<=(unsigned char);
    Boolean operator>>=(unsigned char&) const;
    
    MemBufferedStream pd_data;
    MemBufferedStream pd_typecode;
  };

  class Any_OUT_arg;

  class Any_var {
  public:
    Any_var();
    Any_var(Any *a);
    Any_var(const Any_var &a);
    ~Any_var();

    Any_var &operator=(Any *a);
    Any_var &operator=(const Any_var &a);
    Any *operator->() const;
    
    friend class Any_OUT_arg;
  };

  class Any_OUT_arg {
  public:
    Any_OUT_arg(char*& p);
    Any_OUT_arg(Any_var& p);
    ~Any_OUT_arg();
    char*& _data;

  private:
    Any_OUT_arg();
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Exception                                   //
////////////////////////////////////////////////////////////////////////

  class Exception {
  public:
    virtual ~Exception() {}
  protected:
    Exception() {}
  };

  enum CompletionStatus { COMPLETED_YES, COMPLETED_NO, COMPLETED_MAYBE };
  class SystemException : public Exception {
  public:
    SystemException() {
      pd_minor = 0;
      pd_status = COMPLETED_NO;
    }

    SystemException(const SystemException &e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
    }

    SystemException(ULong minor, CompletionStatus status) {
      pd_minor = minor;
      pd_status = status;
      return;
    }

    virtual ~SystemException() {}

    SystemException &operator=(const SystemException &e) {
      pd_minor = e.pd_minor;
      pd_status = e.pd_status;
      return *this;
    }

#ifdef minor
    // Digital Unix 3.2, and may be others as well, defines minor() as
    // a macro in its sys/types.h. Get rid of it!
#undef minor
#endif

    ULong minor() const { return pd_minor; }

    void minor(ULong m) { pd_minor = m; return; }

    CompletionStatus completed() const { return pd_status; }

    void completed(CompletionStatus s) { pd_status = s; return; }

    virtual const char * NP_RepositoryId() const { return ""; }

  protected:
    ULong             pd_minor;
    CompletionStatus  pd_status;
  };

#define  STD_EXCEPTION(name) \
  class name : public SystemException { \
  public: \
       name (ULong minor = 0, CompletionStatus completed = COMPLETED_NO \
       ) : SystemException (minor,completed) {} \
       virtual const char * NP_RepositoryId() const; \
  }

  STD_EXCEPTION (UNKNOWN);
  STD_EXCEPTION (BAD_PARAM);
  STD_EXCEPTION (NO_MEMORY);
  STD_EXCEPTION (IMP_LIMIT);
  STD_EXCEPTION (COMM_FAILURE);
  STD_EXCEPTION (INV_OBJREF);
  STD_EXCEPTION (OBJECT_NOT_EXIST);
  STD_EXCEPTION (NO_PERMISSION);
  STD_EXCEPTION (INTERNAL);
  STD_EXCEPTION (MARSHAL);
  STD_EXCEPTION (INITIALIZE);
  STD_EXCEPTION (NO_IMPLEMENT);
  STD_EXCEPTION (BAD_TYPECODE);
  STD_EXCEPTION (BAD_OPERATION);
  STD_EXCEPTION (NO_RESOURCES);
  STD_EXCEPTION (NO_RESPONSE);
  STD_EXCEPTION (PERSIST_STORE);
  STD_EXCEPTION (BAD_INV_ORDER);
  STD_EXCEPTION (TRANSIENT);
  STD_EXCEPTION (FREE_MEM);
  STD_EXCEPTION (INV_IDENT);
  STD_EXCEPTION (INV_FLAG);
  STD_EXCEPTION (INTF_REPOS);
  STD_EXCEPTION (BAD_CONTEXT);
  STD_EXCEPTION (OBJ_ADAPTER);
  STD_EXCEPTION (DATA_CONVERSION);

#undef STD_EXCEPTION

  class UserException : public Exception {
  public:
    UserException() {}
    virtual ~UserException() {}
    UserException(const UserException &e) {}
    UserException &operator= (const UserException &e) { return *this; }
  };

  class UnknownUserException : public UserException {
  public:
    Any &exception();
  private:
    UnknownUserException(); // not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL TypeCode                                    //
////////////////////////////////////////////////////////////////////////

  enum TCKind {
    tk_null     = 0,
    tk_void     = 1,
    tk_short    = 2,
    tk_long     = 3,
    tk_ushort	= 4,
    tk_ulong	= 5,
    tk_float	= 6,
    tk_double	= 7,
    tk_boolean	= 8,
    tk_char	= 9,
    tk_octet	= 10,
    tk_any	= 11,
    tk_TypeCode	= 12,
    tk_Principal= 13,
    tk_objref	= 14,
    tk_struct	= 15,
    tk_union	= 16,
    tk_enum	= 17,
    tk_string	= 18,
    tk_sequence	= 19,
    tk_array	= 20,
    tk_alias	= 21,
    tk_except	= 22,
    //
    // This symbol is not defined by CORBA2.  It's used to
    // speed up dispatch based on TCKind values, and lets
    // many important ones just be table lookups.  It must
    // always be the last enum value!!
    //
    TC_KIND_COUNT
  };
  
  
  class TypeCode {
  public:
    class Bounds : public UserException {
    public: 
      Bounds () : UserException () {}
      ~Bounds() { }
    };

    class BadKind : public UserException {
    public:
      BadKind () : UserException () {}
      ~BadKind () { }
    };

    TCKind kind() const;
    Boolean equal(TypeCode_ptr) const;

    const char* id() const;
    const char* name() const;
    
    ULong member_count() const;
    const char* member_name(ULong index) const;

    TypeCode_ptr member_type(ULong index) const;

    Any *member_label(ULong index) const;
    TypeCode_ptr discriminator_type() const;
    Long default_index() const;

    ULong length() const;

    TypeCode_ptr content_type() const;

    Long param_count() const;
    Any *parameter(Long) const;

    static TypeCode_ptr _duplicate();
    static TypeCode_ptr _nil();
  private:
    TypeCode(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Environment                                 //
////////////////////////////////////////////////////////////////////////

  class Environment;
  typedef Environment *Environment_ptr;

  class Environment {
  public:
    void exception(Exception *);
    Exception *exception() const;
    void clear();

    static Environment_ptr _duplicate();
    static Environment_ptr _nil();
  private:
    Environment(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL NamedValue                                  //
////////////////////////////////////////////////////////////////////////

  enum Flags { ARG_IN, ARG_OUT, ARG_INOUT };

  class NamedValue;
  typedef NamedValue* NamedValue_ptr;

  class NamedValue {
  public:
    const char *name() const;
    Any *value() const;
    Flags flags() const;

    static NamedValue_ptr _duplicate();
    static NamedValue_ptr _nil();
  private:
    NamedValue(); // Not implemented yet
  };

  class NVList;
  typedef NVList *NVList_ptr;

  class NVList {
  public:
    NamedValue_ptr add(Flags);
    NamedValue_ptr add_item(const char*, Flags);
    NamedValue_ptr add_value(const char*, const Any&, Flags);
    NamedValue_ptr add_item_consume(char*,Flags);
    NamedValue_ptr add_value_consume(char*, Any *, Flags);

    ULong count() const;
    NamedValue_ptr item(ULong);
    Status remove (ULong);
    
    static NVList_ptr _duplicate();
    static NVList_ptr _nil();
  private:
    NVList(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Context                                     //
////////////////////////////////////////////////////////////////////////

  class Context;
  typedef Context *Context_ptr;

  class Context {
  public:
    const char *context_name() const;
    Context_ptr parent() const;
    Status create_child(const char *, Context_ptr&);
    Status set_one_value(const char *,const Any&);
    Status delete_values(const char *);
    Status get_values(const char*, Flags, const char*, NVList_ptr &);
    
    static Context_ptr _duplicate();
    static Context_ptr _nil();
  private:
    Context(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL ContextList                                 //
////////////////////////////////////////////////////////////////////////

  class ContextList;
  typedef ContextList* ContextList_ptr;

  class ContextList {
  public:
    ULong count();
    void add(const char* ctxt);
    void add_consume(char* ctxt);
    const char* item(ULong index);
    Status remove(ULong index);
  private:
    ContextList(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Principal                                   //
////////////////////////////////////////////////////////////////////////

  class Principal;
  typedef Principal* Principal_ptr;

  typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1> PrincipalID;

  class Principal {
  public:
    static Principal_ptr _duplicate();
    static Principal_ptr _nil();
  private:
    Principal(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL ExceptionList                               //
////////////////////////////////////////////////////////////////////////

  class ExceptionList {
  public:
    ULong count();
    void add(TypeCode_ptr tc);
    void add_consume(TypeCode_ptr tc);
    TypeCode_ptr item(ULong index);
    Status remove(ULong index);
  private:
    ExceptionList(); // Not implemented yet
  };
  typedef ExceptionList* ExceptionList_ptr;

////////////////////////////////////////////////////////////////////////
//                   PIDL Object                                      //
////////////////////////////////////////////////////////////////////////

  class Request;
  typedef Request *Request_ptr;

  class Object;
  typedef Object *Object_ptr;

  typedef _CORBA_Unbounded_Sequence_Octet ReferenceData;

  class ImplementationDef;
  typedef ImplementationDef* ImplementationDef_ptr;

  class ImplementationDef {
  public:
    _CORBA_Unbounded_Sequence_Octet _data;
  private:
    ImplementationDef(); // Not implemented yet;
  };

  class Object_Helper {
  public:
    // omniORB2 specifics
    static Object_ptr _nil();
    static _CORBA_Boolean is_nil(Object_ptr obj);
    static void release(Object_ptr obj);
    static void duplicate(Object_ptr obj);
    static size_t NP_alignedSize(Object_ptr obj,size_t initialoffset);
    static void marshalObjRef(Object_ptr obj,NetBufferedStream &s);
    static Object_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(Object_ptr obj,MemBufferedStream &s);
    static Object_ptr unmarshalObjRef(MemBufferedStream &s);
  };

  class Object {
  public:

#if defined(SUPPORT_DII)

     Status _create_request(Context_ptr ctx,
			   const char *operation,
			   NVList_ptr arg_list,
			   NamedValue_ptr result,
			   Request_ptr &request,
			   Flags req_flags) ;

     Status _create_request(Context_ptr ctx,
			   const char *operation,
			   NVList_ptr arg_list,
			   NamedValue_ptr result,
			   ExceptionList_ptr exceptions,
			   ContextList_ptr ctxlist,
			   Request_ptr &request,
			   Flags req_flags) ;

     Request_ptr _request(const char* operation) ;

#endif // SUPPORT_DII

     ImplementationDef_ptr _get_implementation() ;
     InterfaceDef_ptr      _get_interface() ;
    _CORBA_Boolean _is_a(const char *repoId);
    _CORBA_Boolean _non_existent();
    _CORBA_Boolean _is_equivalent(Object_ptr other_object);
    _CORBA_ULong   _hash(_CORBA_ULong maximum);

    static Object_ptr _duplicate(Object_ptr obj);
    static Object_ptr _nil();

    Object();
    virtual ~Object() {}

    // omniORB2 specifics
    void NP_release();
    _CORBA_Boolean NP_is_nil() const;
    void PR_setobj(omniObject *obj);
    omniObject *PR_getobj();
    static size_t NP_alignedSize(Object_ptr obj,size_t initialoffset);
    static void marshalObjRef(Object_ptr obj,NetBufferedStream &s);
    static Object_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(Object_ptr obj,MemBufferedStream &s);
    static Object_ptr unmarshalObjRef(MemBufferedStream &s);
    static Object CORBA_Object_nil;
  private:
    omniObject *pd_obj;
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Request                                     //
////////////////////////////////////////////////////////////////////////

  class Request {
  public:
    Object_ptr target() const;
    const char *operation() const;
    NVList_ptr arguments();
    NamedValue_ptr result();
    Environment_ptr env();
    ExceptionList_ptr exceptions();
    ContextList_ptr contexts();

    void ctx(Context_ptr);
    Context_ptr ctxt() const;

    Status invoke();
    Status send_oneway();
    Status send_deferred();
    Status get_response();
    Boolean poll_response();

    static Request_ptr _duplicate();
    static Request_ptr _nil();
  private:
    Request(); // Not implemented yet
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL BOA                                         //
////////////////////////////////////////////////////////////////////////

  class BOA;
  typedef class BOA *BOA_ptr;

  class BOA {
  public:
    Object_ptr create( const ReferenceData&,
			       InterfaceDef_ptr,
			       ImplementationDef_ptr) ;
    ReferenceData *get_id(Object_ptr) ;
    void change_implementation(Object_ptr,ImplementationDef_ptr) ;
    Principal_ptr get_principal(Object_ptr, Environment_ptr) ;
    void deactivate_impl(ImplementationDef_ptr) ;
    void deactivate_obj(Object_ptr) ;

    void dispose(Object_ptr) ;
    // see omni::disposeObject()

    void impl_is_ready(ImplementationDef_ptr=0,Boolean NonBlocking=0);
    // The argument <NonBlocking> is omniORB2 specific

    void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0);

    static BOA_ptr _duplicate(BOA_ptr);
    static BOA_ptr _nil();

    static BOA_ptr getBOA();
    // omniORB2 specific. Must be called after BOA_init is called.
    // Otherwise a CORBA::OBJ_ADAPTOR exception is raised.

    BOA();
    ~BOA();
    
    static BOA boa;
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL ORB                                         //
////////////////////////////////////////////////////////////////////////

  class ORB;
  typedef class ORB *ORB_ptr;

  class ORB  {
  public:

    char *object_to_string(Object_ptr) ;
    Object_ptr string_to_object(const char *) ;

#if defined(SUPPORT_DII)
    typedef sequence<Request_ptr> RequestSeq;
     Status create_list(Long, NVList_ptr&) ;
     Status create_operation_list(OperationDef_ptr, NVList_ptr&) ;
     Status create_named_value(NamedValue_ptr&) ;
     Status create_exception_list(ExceptionList_ptr&) ;
     Status create_context_list(ContextList_ptr&) ;
    
     Status get_default_context(Context_ptr&) ;
     Status create_environment(Environment_ptr&) ;
    
     Status send_multiple_requests_oneway(const RequestSeq&) ;
     Status send_multiple_requests_deferred(const RequestSeq&) ;
     Boolean poll_next_response() ;
     Status get_next_response(Request_ptr&) ;
#endif // SUPPORT_DII

    typedef char *OAid;
    
    BOA_ptr BOA_init(int &argc, char **argv, const char *boa_identifier=0) ;

    // Initial Object Reference definitions:

    typedef char* ObjectId;
    typedef String_var ObjectId_var;

    typedef _CORBA_Unbounded_Sequence<String_member > ObjectIdList;

    class ObjectIdList_OUT_arg;

    class ObjectIdList_var {
    public:
      typedef ObjectIdList* ptr_t;
      inline ObjectIdList_var() { pd_data = 0; }
      inline ObjectIdList_var(ObjectIdList* p) { pd_data = p; }
      inline ObjectIdList_var(const ObjectIdList_var& p) {
	if (!p.pd_data) {
	  pd_data = 0;
	  return;
	}
	else {
	  pd_data = new ObjectIdList;
	  if (!pd_data) {
	    _CORBA_new_operator_return_null();
	    // never reach here
	  }
	  *pd_data = *p.pd_data;
	}
      }
      inline ~ObjectIdList_var() {  if (pd_data) delete pd_data; }
      inline ObjectIdList_var& operator= (ObjectIdList* p) {
	if (pd_data) delete pd_data;
	pd_data = p;
	return *this;
      }
      inline ObjectIdList_var& operator= (const ObjectIdList_var& p) {
	if (p.pd_data) {
	  if (!pd_data) {
	    pd_data = new ObjectIdList;
	    if (!pd_data) {
	      _CORBA_new_operator_return_null();
	      // never reach here
	    }
	  }
	  *pd_data = *p.pd_data;
	}
	else {
	  if (pd_data) delete pd_data;
	  pd_data = 0;
	}
	return *this;
      }
      inline ObjectIdList* operator->() const { return pd_data; }

#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
      inline operator ObjectIdList& () const { return *pd_data; }
#else
      inline operator const ObjectIdList& () const { return *pd_data; }
      inline operator ObjectIdList& () { return *pd_data; }
#endif

      friend class ObjectIdList_OUT_arg;

    private:
      ObjectIdList* pd_data;
    };

    // omniORB2 private class
    class ObjectIdList_OUT_arg {
    public:
      inline ObjectIdList_OUT_arg(ObjectIdList*& p) : _data(p) {}
      inline ObjectIdList_OUT_arg(ObjectIdList_var& p) : _data(p.pd_data) {
	p = (ObjectIdList*)0;
      }
      ObjectIdList*& _data;
    private:
      ObjectIdList_OUT_arg();
    };

    class InvalidName : public UserException {
    public:

      inline InvalidName() { }
      InvalidName(const InvalidName &);

      InvalidName & operator=(const InvalidName &);
      virtual ~InvalidName() { }
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

    ObjectIdList* list_initial_services() ;
    Object_ptr resolve_initial_references(const char *identifier) ;

    static ORB_ptr _duplicate(ORB_ptr p);
    static ORB_ptr _nil();

    ORB();
    ~ORB();

    static ORB orb;
  };

  typedef Char *ORBid;
  static ORB_ptr ORB_init(int &argc, char **argv, const char *orb_identifier=0);

////////////////////////////////////////////////////////////////////////
//                   PIDL release and is_null                         //
////////////////////////////////////////////////////////////////////////

  static Boolean is_nil(TypeCode_ptr);
  static Boolean is_nil(Environment_ptr);
  static Boolean is_nil(Context_ptr);
  static Boolean is_nil(Principal_ptr);
  static inline Boolean is_nil(Object_ptr o) { return o->NP_is_nil(); }
  static Boolean is_nil(BOA_ptr p);
  static Boolean is_nil(ORB_ptr p);
  static Boolean is_nil(NamedValue_ptr);
  static Boolean is_nil(NVList_ptr);
  static Boolean is_nil(Request_ptr);

  static void release(TypeCode_ptr);
  static void release(Environment_ptr);
  static void release(Context_ptr);
  static void release(Principal_ptr);
  static inline void release(Object_ptr o) 
  { 
    // see also omni::objectRelease()
    if (!CORBA::is_nil(o))
      o->NP_release(); 
    return;
  }
  static void release(BOA_ptr);
  static void release(ORB_ptr);
  static void release(NamedValue_ptr);
  static void release(NVList_ptr);
  static void release(Request_ptr);

  // omniORB2 private function.
  static Object_ptr UnMarshalObjRef(const char *repoId, NetBufferedStream &s); 

  // omniORB2 private function.
  static void MarshalObjRef(Object_ptr obj,
			    const char *repoId,
			    size_t repoIdSize,
			    NetBufferedStream &s);

  // omniORB2 private function
  static size_t AlignedObjRef(Object_ptr obj,
			      const char *repoId,
			      size_t repoIdSize,
			      size_t initialoffset);

  // omniORB2 private function
  static Object_ptr UnMarshalObjRef(const char *repoId,
				    MemBufferedStream &s);

  // omniORB2 private function
  static void MarshalObjRef(Object_ptr obj,
			    const char *repoId,
			    size_t repoIdSize,
			    MemBufferedStream &s);

  class Object_member;
  class Object_INOUT_arg;
  class Object_OUT_arg;

  class Object_var {
  public:
    inline Object_var() { pd_objref = CORBA::Object::_nil(); }
    inline Object_var(Object_ptr p) { pd_objref = p; }
    inline ~Object_var() {  
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
    }
    inline Object_var(const Object_var& p) {
      if (!CORBA::is_nil(p.pd_objref)) {
	pd_objref = CORBA::Object::_duplicate(p.pd_objref);
      }
      else
	pd_objref = CORBA::Object::_nil();
    }
    Object_var(const Object_member& p);
    inline Object_var& operator= (Object_ptr p) {
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
      pd_objref = p;
      return *this;
    }
    inline Object_var& operator= (const Object_var& p) {
      if (!CORBA::is_nil(pd_objref)) CORBA::release(pd_objref);
      if (!CORBA::is_nil(p.pd_objref)) {
	pd_objref = CORBA::Object::_duplicate(p.pd_objref);
      }
      else
	pd_objref = CORBA::Object::_nil();
      return *this;
    }
    Object_var& operator= (const Object_member& p);
    inline Object_ptr operator->() const { return (Object_ptr)pd_objref; }

    inline operator Object_ptr() const { return pd_objref; }

    friend class Object_member;
    friend class Object_INOUT_arg;
    friend class Object_OUT_arg;

private:
    Object_ptr pd_objref;
  };
  
  // omniORB2 private class
  class Object_member {
  public:
    inline Object_member() { _ptr = CORBA::Object::_nil(); }
    inline Object_member(Object_ptr p) { _ptr = p; }
    inline Object_member(const Object_member& p) {
      if (!CORBA::is_nil(p._ptr)) {
	_ptr = CORBA::Object::_duplicate(p._ptr);
      }
      else
	_ptr = CORBA::Object::_nil();
    }
    inline ~Object_member() {
      if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
    }
    inline Object_member& operator= (Object_ptr p) {
      if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      _ptr = p;
      return *this;
    }
    inline Object_member& operator= (const Object_member& p) {
      if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      if (!CORBA::is_nil(p._ptr)) {
	_ptr = CORBA::Object::_duplicate(p._ptr);
      }
      else
	_ptr = CORBA::Object::_nil();
      return *this;
    }
    inline Object_member& operator= (const Object_var& p) {
      if (!CORBA::is_nil(_ptr)) CORBA::release(_ptr);
      if (!CORBA::is_nil(p.pd_objref)) {
	_ptr = CORBA::Object::_duplicate(p.pd_objref);
      }
      else
	_ptr = CORBA::Object::_nil();
      return *this;
    }
    inline size_t NP_alignedSize(size_t initialoffset) const {
      return CORBA::Object_Helper::NP_alignedSize(_ptr,initialoffset);
    }
    inline void operator>>= (NetBufferedStream &s) const {
      CORBA::Object_Helper::marshalObjRef(_ptr,s);
    }
    inline void operator<<= (NetBufferedStream &s) {
      Object_ptr _result = CORBA::Object_Helper::unmarshalObjRef(s);
      CORBA::release(_ptr);
      _ptr = _result;
    }
    inline void operator>>= (MemBufferedStream &s) const {
      CORBA::Object_Helper::marshalObjRef(_ptr,s);
    }
    inline void operator<<= (MemBufferedStream &s) {
      Object_ptr _result = CORBA::Object_Helper::unmarshalObjRef(s);
      CORBA::release(_ptr);
      _ptr = _result;
    }

    inline Object_ptr operator->() const { return (Object_ptr)_ptr; }
    inline operator Object_ptr () const { return _ptr; }
    Object_ptr _ptr;
  };

  // omniORB2 private class
  class Object_INOUT_arg {
  public:
    inline Object_INOUT_arg(Object_ptr& p) : _data(p) {}
    inline Object_INOUT_arg(Object_var& p) : _data(p.pd_objref) {}
    inline Object_INOUT_arg(Object_member& p) : _data(p._ptr) {}
    inline ~Object_INOUT_arg() {}

    Object_ptr& _data;

  private:
    Object_INOUT_arg();
  };

  // omniORB2 private class
  class Object_OUT_arg {
  public:
    inline Object_OUT_arg(Object_ptr& p) : _data(p) { }
    inline Object_OUT_arg(Object_var& p) : _data(p.pd_objref) { 
      p = CORBA::Object::_nil(); 
    }
    inline Object_OUT_arg(Object_member& p) : _data(p._ptr) { 
      p = CORBA::Object::_nil();
    }
    inline ~Object_OUT_arg() {}

    Object_ptr& _data;

  private:
    Object_OUT_arg();
  };

  class BOA_var {
  public:
    inline BOA_var() : pd_boaref(BOA::_nil()) {}
    inline BOA_var(BOA_ptr p) { pd_boaref = p; }
    inline ~BOA_var() {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
    }
    inline BOA_var(const BOA_var& p) { 
      pd_boaref = BOA::_duplicate(p.pd_boaref); 
    }
    inline BOA_var& operator= (BOA_ptr p) {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
      pd_boaref = p; 
      return *this; 
    }
    inline BOA_var& operator= (const BOA_var& p) {
      if (!CORBA::is_nil(pd_boaref)) {
	CORBA::release(pd_boaref);
      }
      pd_boaref = BOA::_duplicate(p.pd_boaref);
      return *this;
    }
    inline BOA_ptr operator->() const { return (BOA_ptr)pd_boaref; }
    inline operator BOA_ptr() const { return pd_boaref; }

  private:
    BOA_ptr pd_boaref;
  };

  class ORB_var {
  public:
    inline ORB_var() : pd_orbref(ORB::_nil()) {}
    inline ORB_var(ORB_ptr p) { pd_orbref = p; }
    inline ~ORB_var() {
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
    }
    inline ORB_var(const ORB_var& p) { 
      pd_orbref = ORB::_duplicate(p.pd_orbref); 
    }
    inline ORB_var& operator= (ORB_ptr p) { 
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
      pd_orbref = p; 
      return *this; 
    }
    inline ORB_var& operator= (const ORB_var& p) {
      if (!CORBA::is_nil(pd_orbref)) {
	CORBA::release(pd_orbref);
      }
      pd_orbref = ORB::_duplicate(p.pd_orbref);
      return *this;
    }
    inline ORB_ptr operator->() const { return (ORB_ptr)pd_orbref; }
    inline operator ORB_ptr() const { return pd_orbref; }

  private:
    ORB_ptr pd_orbref;
  };

};


#include <omniORB2/templates.h>
#include <omniORB2/proxyFactory.h>

// Include the COSS Naming Service header:
#ifdef __NT__
#include <omniORB2/Naming_NT.hh>
#else
#include <omniORB2/Naming.hh>
#endif

#endif // __CORBA_H__
