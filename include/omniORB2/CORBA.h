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
 Revision 1.2  1997/01/13 15:06:51  sll
 Added marshalling routines for CORBA::Object.

 Revision 1.1  1997/01/08 17:28:30  sll
 Initial revision

*/

#ifndef __CORBA_H__
#define __CORBA_H__

#include <omniORB2/omniORB.h>

class CORBA {

public:

#define Status void

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
//                   Sequence of Primitive types                      //
////////////////////////////////////////////////////////////////////////
typedef _CORBA_Unbounded_Sequence_Boolean Unbounded_Sequence_Boolean;
typedef _CORBA_Unbounded_Sequence_Char    Unbounded_Sequence_Char;
typedef _CORBA_Unbounded_Sequence_Octet   Unbounded_Sequence_Octet;
typedef _CORBA_Unbounded_Sequence_Short   Unbounded_Sequence_Short;
typedef _CORBA_Unbounded_Sequence_UShort  Unbounded_Sequence_UShort;
typedef _CORBA_Unbounded_Sequence_Long    Unbounded_Sequence_Long;
typedef _CORBA_Unbounded_Sequence_ULong   Unbounded_Sequence_ULong;
#ifndef NO_FLOAT
typedef _CORBA_Unbounded_Sequence_Float   Unbounded_Sequence_Float;
typedef _CORBA_Unbounded_Sequence_Double  Unbounded_Sequence_Double;
#endif

////////////////////////////////////////////////////////////////////////
//                   Type String                                      //
////////////////////////////////////////////////////////////////////////
  
  static char *string_alloc(ULong len);
  static void string_free(char *);
  static char *string_dup(const char *);


  class String_member;

  class String_var {
  public:
    typedef char* ptr_t;

    String_var() {
      _data = 0;
    }

    String_var(char *p) {
      _data = p;
    }

    String_var(const char* p) {
      if (p) {
	_data = string_alloc((ULong)(strlen(p)+1));
	strcpy(_data,p);
      }
      else
	_data = 0;
    }

    String_var(const String_var &s) {
      if ((const char *)s) {
	_data = string_alloc((ULong)(strlen(s)+1));
	strcpy(_data,s);
      }
      else {
	_data = 0;
      }
    }

    String_var(const String_member &s);

    ~String_var() {
      if (_data)
	string_free(_data);
    }

    String_var &operator= (char *p) {
      if (_data) {
	string_free(_data);
	_data = 0;
      }
      _data = p;
      return *this;
    }

    String_var &operator= (const char *p) {
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

    String_var &operator= (const String_var &s) {
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

    operator ptr_t& () { return (ptr_t&) _data; }
#if !defined(__GNUG__) || __GNUG__ != 2 || __GNUC_MINOR__ > 7
    operator char* () {
      return _data;
    }
#endif
    operator const char* () const {
      return _data;
    }

    char &operator[] (ULong index);

    char operator[] (ULong index) const;

  private:
    char* _data;
  };


  class String_member
  {
  public:
    typedef char* ptr_t;
    String_member() {
      _ptr = 0;
    }

    ~String_member() {
      if (_ptr) string_free(_ptr);
    }

    String_member(const String_member &s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      if (s._ptr) {
	_ptr = string_alloc((ULong)(strlen(s._ptr)+1));
	strcpy(_ptr,s._ptr);
      }
    }

    String_member& operator= (char *s) {
      if (_ptr) {
	string_free(_ptr);
	_ptr = 0;
      }
      _ptr = s;
      return *this;
    }

    String_member& operator= (const char *s) {
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
    

    String_member& operator= (const String_member & s) {
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

    operator char* () const {
      return _ptr;
    }

    operator const char* () const {
      return (const char*) _ptr;
    }
    operator ptr_t& () { return _ptr; }

    char* _ptr;

    void operator>>= (NetBufferedStream &s) const;
    void operator<<= (NetBufferedStream &s);

    void operator>>= (MemBufferedStream &s) const;
    void operator<<= (MemBufferedStream &s);
    size_t NP_alignedSize(size_t initialoffset) const;
  };


  //////////////////////////////////////////////////////////////////////
  //    Interface repository types                                    //
  //////////////////////////////////////////////////////////////////////
  typedef char *RepositoryId;
  class InterfaceDef;
  typedef class InterfaceDef *InterfaceDef_ptr;

////////////////////////////////////////////////////////////////////////
//                   Type Any                                         //
////////////////////////////////////////////////////////////////////////

  class TypeCode;
  typedef class TypeCode *TypeCode_ptr;
  class Any {
  public:
    Any();
    Any(const Any&);
    Any(TypeCode_ptr tc, void *value, Boolean release = 0);
    ~Any();

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


  class Any_var {
  public:
    Any_var();
    Any_var(Any *a);
    Any_var(const Any_var &a);
    ~Any_var();

    Any_var &operator=(Any *a);
    Any_var &operator=(const Any_var &a);
    Any *operator->();
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

#   ifdef minor
    // Digital Unix 3.2, and may be others as well, defines minor() as
    // a macro in its sys/types.h. Get rid of it!
#   undef minor
#   endif

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
    Any &exception() { return pd_data; }
  protected:
    Any pd_data;
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
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Environment                                 //
////////////////////////////////////////////////////////////////////////

  typedef class Environment *Environment_ptr;

  class Environment {
  public:
    void exception(Exception *);
    Exception *exception() const;
    void clear();

    static Environment_ptr _duplicate();
    static Environment_ptr _nil();
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL NamedValue                                  //
////////////////////////////////////////////////////////////////////////

  enum Flags { ARG_IN, ARG_OUT, ARG_INOUT };

  typedef class NamedValue *NamedValue_ptr;

  class NamedValue {
  public:
    const char *name() const;
    Any *value() const;
    Flags flags() const;

    static NamedValue_ptr _duplicate();
    static NamedValue_ptr _nil();
  };

  typedef class NVList *NVList_ptr;

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
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Context                                     //
////////////////////////////////////////////////////////////////////////

  typedef class Context *Context_ptr;

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
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Principal                                   //
////////////////////////////////////////////////////////////////////////

  typedef class Principal *Principal_ptr;

  typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<_CORBA_Octet,1,1> PrincipalID;

  class Principal {
  public:
    static Principal_ptr _duplicate();
    static Principal_ptr _nil();
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL Object                                      //
////////////////////////////////////////////////////////////////////////

  class Request;
  typedef class Request *Request_ptr;

  class Object;
  typedef class Object *Object_ptr;
  typedef Unbounded_Sequence_Octet ReferenceData;
  class ImplementatationDef;
  typedef class ImplementationDef *ImplementationDef_ptr;

  class Object_Helper {
  public:
    // omniORB2 specifics
    static Object_ptr _nil();
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
    _CORBA_Boolean NP_is_nil();
    void PR_setobj(omniObject *obj);
    omniObject *PR_getobj();
    static size_t NP_alignedSize(Object_ptr obj,size_t initialoffset);
    static void marshalObjRef(Object_ptr obj,NetBufferedStream &s);
    static Object_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(Object_ptr obj,MemBufferedStream &s);
    static Object_ptr unmarshalObjRef(MemBufferedStream &s);

  private:
    omniObject *pd_obj;
  };

  typedef _CORBA_ObjRef_Var<Object,Object_Helper> Object_var;

////////////////////////////////////////////////////////////////////////
//                   PIDL Request                                     //
////////////////////////////////////////////////////////////////////////

#if defined(SUPPORT_DII)
  class Request {
  public:
    Object_ptr target() const;
    const char *operation() const;
    NVList_ptr arguments();
    NameValue_ptr result();
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
  };
#endif // SUPPORT_DII

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
    // see omniORB::disposeObject()

    void impl_is_ready(ImplementationDef_ptr=0);
    void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0);

    static BOA_ptr _duplicate(BOA_ptr p);
    static BOA_ptr _nil();

    BOA();
    ~BOA();
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL ORB                                         //
////////////////////////////////////////////////////////////////////////

  class ORB;
  typedef class ORB *ORB_ptr;

  class ORB  {
  public:

    ORB();
    ~ORB();

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

#ifdef NOT_SUPPORTED_YET
    typedef Char * ObjectId;
   
    class ObjectIdList {
    public:
    };

    class InvalidName {
    public:
    };

    ObjectIdList *list_initial_services() ;
    Object_ptr resolve_initial_references(const char *identifier) ;
#endif

    static ORB_ptr _duplicate(ORB_ptr p);
    static ORB_ptr _nil();
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
  static Boolean is_nil(Object_ptr o) { return o->NP_is_nil(); }
  static Boolean is_nil(BOA_ptr);
  static Boolean is_nil(ORB_ptr);
#if defined(SUPPORT_DII)
  static Boolean is_nil(NamedValue_ptr);
  static Boolean is_nil(NVList_ptr);
  static Boolean is_nil(Request_ptr);
#endif // SUPPORT_DII

  static void release(TypeCode_ptr);
  static void release(Environment_ptr);
  static void release(Context_ptr);
  static void release(Principal_ptr);
  static void release(Object_ptr o) 
  { 
    // see also omniORBA::objectRelease()
    if (!CORBA::is_nil(o))
      o->NP_release(); 
    return;
  }
  static void release(BOA_ptr);
  static void release(ORB_ptr);
#if defined(SUPPORT_DII)
  static void release(NamedValue_ptr);
  static void release(NVList_ptr);
  static void release(Request_ptr);
#endif // SUPPORT_DII


  // omniORB2 extensions - marshalling routines for pseudo objects

  static Object_ptr UnMarshalObjRef(const char *repoId,
				    NetBufferedStream &s);
  static void MarshalObjRef(Object_ptr obj,
			    const char *repoId,
			    size_t repoIdSize,
			    NetBufferedStream &s);
  static size_t AlignedObjRef(Object_ptr obj,
			      const char *repoId,
			      size_t repoIdSize,
			      size_t initialoffset);
  static Object_ptr UnMarshalObjRef(const char *repoId,
				    MemBufferedStream &s);
  static void MarshalObjRef(Object_ptr obj,
			    const char *repoId,
			    size_t repoIdSize,
			    MemBufferedStream &s);
};

#endif // __CORBA_H__
