// Naming_NT.hh -- manually altered for use with Windows NT
//

// This version of the Naming Stubs should be used until the IDL compiler
// generates code acceptable to MSVC++ 4.2

#ifndef __Naming_hh__
#define __Naming_hh__

#include <omniORB2/CORBA.h>

class CosNaming {
public:

  typedef char* Istring;
  typedef CORBA::String_var Istring_var;
  struct NameComponent {
    CORBA::String_member id;
    CORBA::String_member kind;
    
    size_t NP_alignedSize(size_t initialoffset) const;
    void operator>>= (NetBufferedStream &) const;
    void operator<<= (NetBufferedStream &);
    void operator>>= (MemBufferedStream &) const;
    void operator<<= (MemBufferedStream &);
  };

  typedef NameComponent CosNaming_NameComponent;
  typedef _CORBA_ConstrType_Variable_Var<NameComponent> NameComponent_var;

  typedef _CORBA_Unbounded_Sequence<CosNaming_NameComponent > Name;
  typedef Name CosNaming_Name;
  typedef _CORBA_ConstrType_Variable_Var<Name> Name_var;

  enum BindingType { nobject, ncontext };

  typedef BindingType CosNaming_BindingType;
  friend inline void operator>>= (BindingType _e,NetBufferedStream &s) {
    ::operator>>=((CORBA::ULong)_e,s);
  }

  friend inline void operator<<= (BindingType &_e,NetBufferedStream &s) {
    CORBA::ULong __e;
    __e <<= s;
    switch (__e) {
      case nobject:
      case ncontext:
        _e = (BindingType) __e;
        break;
      default:
        _CORBA_marshal_error();
    }
  }

  friend inline void operator>>= (BindingType _e,MemBufferedStream &s) {
    ::operator>>=((CORBA::ULong)_e,s);
  }

  friend inline void operator<<= (BindingType &_e,MemBufferedStream &s) {
    CORBA::ULong __e;
    __e <<= s;
    switch (__e) {
      case nobject:
      case ncontext:
        _e = (BindingType) __e;
        break;
      default:
        _CORBA_marshal_error();
    }
  }

  struct Binding {
    CosNaming_Name binding_name;
    CosNaming_BindingType binding_type;
    
    size_t NP_alignedSize(size_t initialoffset) const;
    void operator>>= (NetBufferedStream &) const;
    void operator<<= (NetBufferedStream &);
    void operator>>= (MemBufferedStream &) const;
    void operator<<= (MemBufferedStream &);
  };

  typedef Binding CosNaming_Binding;
  typedef _CORBA_ConstrType_Variable_Var<Binding> Binding_var;

  typedef _CORBA_Unbounded_Sequence<CosNaming_Binding > BindingList;
  typedef _CORBA_ConstrType_Variable_Var<BindingList> BindingList_var;
  typedef BindingList CosNaming_BindingList;

#ifndef __CosNaming_BindingIterator__
#define __CosNaming_BindingIterator__
  class   BindingIterator;
  typedef BindingIterator* BindingIterator_ptr;
  typedef BindingIterator_ptr BindingIteratorRef;
  typedef BindingIterator CosNaming_BindingIterator;
  typedef BindingIterator_ptr CosNaming_BindingIterator_ptr;

  class BindingIterator_Helper {
    public:
    static BindingIterator_ptr _nil();
    static CORBA::Boolean is_nil(BindingIterator_ptr p);
    static void release(BindingIterator_ptr p);
    static void duplicate(BindingIterator_ptr p);
    static size_t NP_alignedSize(BindingIterator_ptr obj,size_t initialoffset);
    static void marshalObjRef(BindingIterator_ptr obj,NetBufferedStream &s);
    static BindingIterator_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(BindingIterator_ptr obj,MemBufferedStream &s);
    static BindingIterator_ptr unmarshalObjRef(MemBufferedStream &s);
  };
#endif
#ifndef __CosNaming_NamingContext__
#define __CosNaming_NamingContext__
  class   NamingContext;
  typedef NamingContext* NamingContext_ptr;
  typedef NamingContext_ptr NamingContextRef;
  typedef NamingContext_ptr CosNaming_NamingContext_ptr;
  typedef NamingContext CosNaming_NamingContext;

  class NamingContext_Helper {
    public:
    static NamingContext_ptr _nil();
    static CORBA::Boolean is_nil(NamingContext_ptr p);
    static void release(NamingContext_ptr p);
    static void duplicate(NamingContext_ptr p);
    static size_t NP_alignedSize(NamingContext_ptr obj,size_t initialoffset);
    static void marshalObjRef(NamingContext_ptr obj,NetBufferedStream &s);
    static NamingContext_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(NamingContext_ptr obj,MemBufferedStream &s);
    static NamingContext_ptr unmarshalObjRef(MemBufferedStream &s);
  };

  typedef NamingContext_Helper CosNaming_NamingContext_Helper;
#endif
#ifndef __CosNaming_NamingContext__
#define __CosNaming_NamingContext__
  class   NamingContext;
  typedef NamingContext* NamingContext_ptr;
  typedef NamingContext_ptr NamingContextRef;
  typedef NamingContext_ptr CosNaming_NamingContext_ptr;
  typedef NamingContext CosNaming_NamingContext;

  class NamingContext_Helper {
    public:
    static NamingContext_ptr _nil();
    static CORBA::Boolean is_nil(NamingContext_ptr p);
    static void release(NamingContext_ptr p);
    static void duplicate(NamingContext_ptr p);
    static size_t NP_alignedSize(NamingContext_ptr obj,size_t initialoffset);
    static void marshalObjRef(NamingContext_ptr obj,NetBufferedStream &s);
    static NamingContext_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(NamingContext_ptr obj,MemBufferedStream &s);
    static NamingContext_ptr unmarshalObjRef(MemBufferedStream &s);
  };

  typedef NamingContext_Helper CosNaming_NamingContext_Helper;
#endif
#define CosNaming_NamingContext_IntfRepoID "IDL:CosNaming/NamingContext:1.0"


  class NamingContext : public virtual omniObject, public virtual CORBA::Object {
  public:

    enum NotFoundReason { missing_node, not_context, not_object };

	typedef NotFoundReason CosNaming_NamingContext_NotFoundReason;
    friend inline void operator>>= (NotFoundReason _e,NetBufferedStream &s) {
      ::operator>>=((CORBA::ULong)_e,s);
    }

    friend inline void operator<<= (NotFoundReason &_e,NetBufferedStream &s) {
      CORBA::ULong __e;
      __e <<= s;
      switch (__e) {
        case missing_node:
        case not_context:
        case not_object:
          _e = (NotFoundReason) __e;
          break;
        default:
          _CORBA_marshal_error();
      }
    }

    friend inline void operator>>= (NotFoundReason _e,MemBufferedStream &s) {
      ::operator>>=((CORBA::ULong)_e,s);
    }

    friend inline void operator<<= (NotFoundReason &_e,MemBufferedStream &s) {
      CORBA::ULong __e;
      __e <<= s;
      switch (__e) {
        case missing_node:
        case not_context:
        case not_object:
          _e = (NotFoundReason) __e;
          break;
        default:
          _CORBA_marshal_error();
      }
    }

#define CosNaming_NamingContext_NotFound_IntfRepoID "IDL:CosNaming/NamingContext/NotFound:1.0"

    class NotFound : public CORBA::UserException {
    public:

      CosNaming_NamingContext_NotFoundReason why;
      CosNaming_Name rest_of_name;
      
      NotFound() {};
      NotFound(const NotFound &);
      NotFound(CosNaming_NamingContext_NotFoundReason  _why, const CosNaming_Name & _rest_of_name);
      NotFound & operator=(const NotFound &);
      virtual ~NotFound() {};
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

#define CosNaming_NamingContext_CannotProceed_IntfRepoID "IDL:CosNaming/NamingContext/CannotProceed:1.0"

    class CannotProceed : public CORBA::UserException {
    public:

      _CORBA_ObjRef_Member<CosNaming_NamingContext,CosNaming_NamingContext_Helper> cxt;
      CosNaming_Name rest_of_name;
      
      CannotProceed() {};
      CannotProceed(const CannotProceed &);
      CannotProceed(CosNaming_NamingContext_ptr _cxt, const CosNaming_Name & _rest_of_name);
      CannotProceed & operator=(const CannotProceed &);
      virtual ~CannotProceed() {};
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

#define CosNaming_NamingContext_InvalidName_IntfRepoID "IDL:CosNaming/NamingContext/InvalidName:1.0"

    class InvalidName : public CORBA::UserException {
    public:

      
      InvalidName() {};
      InvalidName(const InvalidName &);
      InvalidName & operator=(const InvalidName &);
      virtual ~InvalidName() {};
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

#define CosNaming_NamingContext_AlreadyBound_IntfRepoID "IDL:CosNaming/NamingContext/AlreadyBound:1.0"

    class AlreadyBound : public CORBA::UserException {
    public:

      
      AlreadyBound() {};
      AlreadyBound(const AlreadyBound &);
      AlreadyBound & operator=(const AlreadyBound &);
      virtual ~AlreadyBound() {};
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

#define CosNaming_NamingContext_NotEmpty_IntfRepoID "IDL:CosNaming/NamingContext/NotEmpty:1.0"

    class NotEmpty : public CORBA::UserException {
    public:

      
      NotEmpty() {};
      NotEmpty(const NotEmpty &);
      NotEmpty & operator=(const NotEmpty &);
      virtual ~NotEmpty() {};
      size_t NP_alignedSize(size_t initialoffset);
      void operator>>= (NetBufferedStream &);
      void operator<<= (NetBufferedStream &);
      void operator>>= (MemBufferedStream &);
      void operator<<= (MemBufferedStream &);
    };

    virtual void bind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ) = 0;
    virtual void rebind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ) = 0;
    virtual void bind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ) = 0;
    virtual void rebind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ) = 0;
    virtual CORBA::Object_ptr  resolve ( const CosNaming_Name & n ) = 0;
    virtual void unbind ( const CosNaming_Name & n ) = 0;
    virtual CosNaming_NamingContext_ptr  new_context (  ) = 0;
    virtual CosNaming_NamingContext_ptr  bind_new_context ( const CosNaming_Name & n ) = 0;
    virtual void destroy (  ) = 0;
    virtual void list ( CORBA::ULong  how_many, CosNaming_BindingList *& bl, CosNaming_BindingIterator_ptr & bi ) = 0;
    static NamingContext_ptr _duplicate(NamingContext_ptr);
    static NamingContext_ptr _narrow(CORBA::Object_ptr);
    static NamingContext_ptr _nil();

    static inline size_t NP_alignedSize(NamingContext_ptr obj,size_t initialoffset) {
      return CORBA::AlignedObjRef(obj,CosNaming_NamingContext_IntfRepoID,32,initialoffset);
    }

    static inline void marshalObjRef(NamingContext_ptr obj,NetBufferedStream &s) {
      CORBA::MarshalObjRef(obj,CosNaming_NamingContext_IntfRepoID,32,s);
    }

    static inline NamingContext_ptr unmarshalObjRef(NetBufferedStream &s) {
      CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(CosNaming_NamingContext_IntfRepoID,s);
      NamingContext_ptr _result = CosNaming::NamingContext::_narrow(_obj);
      CORBA::release(_obj);
      return _result;
    }

    static inline void marshalObjRef(NamingContext_ptr obj,MemBufferedStream &s) {
      CORBA::MarshalObjRef(obj,CosNaming_NamingContext_IntfRepoID,32,s);
    }

    static inline NamingContext_ptr unmarshalObjRef(MemBufferedStream &s) {
      CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(CosNaming_NamingContext_IntfRepoID,s);
      NamingContext_ptr _result = CosNaming::NamingContext::_narrow(_obj);
      CORBA::release(_obj);
      return _result;
    }

    static CORBA::Boolean _is_a(const char *base_repoId);

  protected:

    NamingContext() {
      if (!is_proxy())
        omniObject::PR_IRRepositoryId(CosNaming_NamingContext_IntfRepoID);
      this->PR_setobj(this);
    }
    virtual ~NamingContext() {}
    virtual void *_widenFromTheMostDerivedIntf(const char *repoId) throw ();

  private:

    NamingContext(const NamingContext&);
    NamingContext &operator=(const NamingContext&);
  };

  typedef _CORBA_ObjRef_Var<NamingContext,NamingContext_Helper> NamingContext_var;

  class _sk_NamingContext :  public virtual NamingContext {
  public:

    _sk_NamingContext() {}
    virtual ~_sk_NamingContext() {}
    virtual CORBA::Object_ptr _this() { return (CORBA::Object_ptr) this; }
    virtual void bind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ) = 0;
    virtual void rebind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ) = 0;
    virtual void bind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ) = 0;
    virtual void rebind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ) = 0;
    virtual CORBA::Object_ptr  resolve ( const CosNaming_Name & n ) = 0;
    virtual void unbind ( const CosNaming_Name & n ) = 0;
    virtual CosNaming_NamingContext_ptr  new_context (  ) = 0;
    virtual CosNaming_NamingContext_ptr  bind_new_context ( const CosNaming_Name & n ) = 0;
    virtual void destroy (  ) = 0;
    virtual void list ( CORBA::ULong  how_many, CosNaming_BindingList *& bl, CosNaming_BindingIterator_ptr & bi ) = 0;
    virtual CORBA::Boolean dispatch(GIOP_S &s,const char *op,CORBA::Boolean response);

  private:
    _sk_NamingContext (const _sk_NamingContext&);
    _sk_NamingContext &operator=(const _sk_NamingContext&);
  };

  class _proxy_NamingContext :  public virtual NamingContext {
  public:

    _proxy_NamingContext (Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release) :
      omniObject(CosNaming_NamingContext_IntfRepoID,r,key,keysize,profiles,release) {
        omniORB::objectIsReady(this);
    }
    virtual ~_proxy_NamingContext() {}
    virtual void bind ( const CosNaming_Name & n, CORBA::Object_ptr  obj );
    virtual void rebind ( const CosNaming_Name & n, CORBA::Object_ptr  obj );
    virtual void bind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc );
    virtual void rebind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc );
    virtual CORBA::Object_ptr  resolve ( const CosNaming_Name & n );
    virtual void unbind ( const CosNaming_Name & n );
    virtual CosNaming_NamingContext_ptr  new_context (  );
    virtual CosNaming_NamingContext_ptr  bind_new_context ( const CosNaming_Name & n );
    virtual void destroy (  );
    virtual void list ( CORBA::ULong  how_many, CosNaming_BindingList *& bl, CosNaming_BindingIterator_ptr & bi );

  protected:

    _proxy_NamingContext () {}

  private:

    _proxy_NamingContext (const _proxy_NamingContext&);
    _proxy_NamingContext &operator=(const _proxy_NamingContext&);
  };

  class _nil_NamingContext :   public virtual NamingContext {
  public:
    _nil_NamingContext() { this->PR_setobj(0); }
    virtual ~_nil_NamingContext() {}
    void bind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    void rebind ( const CosNaming_Name & n, CORBA::Object_ptr  obj ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    void bind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    void rebind_context ( const CosNaming_Name & n, CosNaming_NamingContext_ptr  nc ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    CORBA::Object_ptr  resolve ( const CosNaming_Name & n ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      CORBA::Object_ptr _result= 0;
      return _result;
    }

    void unbind ( const CosNaming_Name & n ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    CosNaming_NamingContext_ptr  new_context (  ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      CosNaming_NamingContext_ptr _result= 0;
      return _result;
    }

    CosNaming_NamingContext_ptr  bind_new_context ( const CosNaming_Name & n ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      CosNaming_NamingContext_ptr _result= 0;
      return _result;
    }

    void destroy (  ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

    void list ( CORBA::ULong  how_many, CosNaming_BindingList *& bl, CosNaming_BindingIterator_ptr & bi ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

  };

  class NamingContext_proxyObjectFactory : public proxyObjectFactory {
  public:
    NamingContext_proxyObjectFactory () {}
    virtual ~NamingContext_proxyObjectFactory () {}
    virtual const char *irRepoId() const;
    virtual CORBA::Object_ptr newProxyObject(Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release);
    virtual CORBA::Boolean is_a(const char *base_repoId) const;
    static CosNaming_NamingContext_ptr _nil() {
      if (!__nil_NamingContext) {
        __nil_NamingContext = new CosNaming::_nil_NamingContext;
      }
      return __nil_NamingContext;
    }
  private:
    static CosNaming_NamingContext_ptr __nil_NamingContext;
  };

#ifndef __CosNaming_BindingIterator__
#define __CosNaming_BindingIterator__
  class   BindingIterator;
  typedef BindingIterator* BindingIterator_ptr;
  typedef BindingIterator_ptr BindingIteratorRef;

  class BindingIterator_Helper {
    public:
    static BindingIterator_ptr _nil();
    static CORBA::Boolean is_nil(BindingIterator_ptr p);
    static void release(BindingIterator_ptr p);
    static void duplicate(BindingIterator_ptr p);
    static size_t NP_alignedSize(BindingIterator_ptr obj,size_t initialoffset);
    static void marshalObjRef(BindingIterator_ptr obj,NetBufferedStream &s);
    static BindingIterator_ptr unmarshalObjRef(NetBufferedStream &s);
    static void marshalObjRef(BindingIterator_ptr obj,MemBufferedStream &s);
    static BindingIterator_ptr unmarshalObjRef(MemBufferedStream &s);
  };
#endif
#define CosNaming_BindingIterator_IntfRepoID "IDL:CosNaming/BindingIterator:1.0"

  class BindingIterator : public virtual omniObject, public virtual CORBA::Object {
  public:

    virtual CORBA::Boolean  next_one ( CosNaming_Binding *& b ) = 0;
    virtual CORBA::Boolean  next_n ( CORBA::ULong  how_many, CosNaming_BindingList *& bl ) = 0;
    virtual void destroy (  ) = 0;
    static BindingIterator_ptr _duplicate(BindingIterator_ptr);
    static BindingIterator_ptr _narrow(CORBA::Object_ptr);
    static BindingIterator_ptr _nil();

    static inline size_t NP_alignedSize(BindingIterator_ptr obj,size_t initialoffset) {
      return CORBA::AlignedObjRef(obj,CosNaming_BindingIterator_IntfRepoID,34,initialoffset);
    }

    static inline void marshalObjRef(BindingIterator_ptr obj,NetBufferedStream &s) {
      CORBA::MarshalObjRef(obj,CosNaming_BindingIterator_IntfRepoID,34,s);
    }

    static inline BindingIterator_ptr unmarshalObjRef(NetBufferedStream &s) {
      CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(CosNaming_BindingIterator_IntfRepoID,s);
      BindingIterator_ptr _result = CosNaming::BindingIterator::_narrow(_obj);
      CORBA::release(_obj);
      return _result;
    }

    static inline void marshalObjRef(BindingIterator_ptr obj,MemBufferedStream &s) {
      CORBA::MarshalObjRef(obj,CosNaming_BindingIterator_IntfRepoID,34,s);
    }

    static inline BindingIterator_ptr unmarshalObjRef(MemBufferedStream &s) {
      CORBA::Object_ptr _obj = CORBA::UnMarshalObjRef(CosNaming_BindingIterator_IntfRepoID,s);
      BindingIterator_ptr _result = CosNaming::BindingIterator::_narrow(_obj);
      CORBA::release(_obj);
      return _result;
    }

    static CORBA::Boolean _is_a(const char *base_repoId);

  protected:

    BindingIterator() {
      if (!is_proxy())
        omniObject::PR_IRRepositoryId(CosNaming_BindingIterator_IntfRepoID);
      this->PR_setobj(this);
    }
    virtual ~BindingIterator() {}
    virtual void *_widenFromTheMostDerivedIntf(const char *repoId) throw ();

  private:

    BindingIterator(const BindingIterator&);
    BindingIterator &operator=(const BindingIterator&);
  };

  typedef _CORBA_ObjRef_Var<BindingIterator,BindingIterator_Helper> BindingIterator_var;

  class _sk_BindingIterator :  public virtual BindingIterator {
  public:

    _sk_BindingIterator() {}
    virtual ~_sk_BindingIterator() {}
    virtual CORBA::Object_ptr _this() { return (CORBA::Object_ptr) this; }
    virtual CORBA::Boolean  next_one ( CosNaming_Binding *& b ) = 0;
    virtual CORBA::Boolean  next_n ( CORBA::ULong  how_many, CosNaming_BindingList *& bl ) = 0;
    virtual void destroy (  ) = 0;
    virtual CORBA::Boolean dispatch(GIOP_S &s,const char *op,CORBA::Boolean response);

  private:
    _sk_BindingIterator (const _sk_BindingIterator&);
    _sk_BindingIterator &operator=(const _sk_BindingIterator&);
  };

  class _proxy_BindingIterator :  public virtual BindingIterator {
  public:

    _proxy_BindingIterator (Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release) :
      omniObject(CosNaming_BindingIterator_IntfRepoID,r,key,keysize,profiles,release) {
        omniORB::objectIsReady(this);
    }
    virtual ~_proxy_BindingIterator() {}
    virtual CORBA::Boolean  next_one ( CosNaming_Binding *& b );
    virtual CORBA::Boolean  next_n ( CORBA::ULong  how_many, CosNaming_BindingList *& bl );
    virtual void destroy (  );

  protected:

    _proxy_BindingIterator () {}

  private:

    _proxy_BindingIterator (const _proxy_BindingIterator&);
    _proxy_BindingIterator &operator=(const _proxy_BindingIterator&);
  };

  class _nil_BindingIterator :   public virtual BindingIterator {
  public:
    _nil_BindingIterator() { this->PR_setobj(0); }
    virtual ~_nil_BindingIterator() {}
    CORBA::Boolean  next_one ( CosNaming_Binding *& b ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      CORBA::Boolean _result = 0;
      return _result;
    }

    CORBA::Boolean  next_n ( CORBA::ULong  how_many, CosNaming_BindingList *& bl ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      CORBA::Boolean _result = 0;
      return _result;
    }

    void destroy (  ){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      return;
    }

  };

  class BindingIterator_proxyObjectFactory : public proxyObjectFactory {
  public:
    BindingIterator_proxyObjectFactory () {}
    virtual ~BindingIterator_proxyObjectFactory () {}
    virtual const char *irRepoId() const;
    virtual CORBA::Object_ptr newProxyObject(Rope *r,CORBA::Octet *key,size_t keysize,IOP::TaggedProfileList *profiles,CORBA::Boolean release);
    virtual CORBA::Boolean is_a(const char *base_repoId) const;
    static CosNaming_BindingIterator_ptr _nil() {
      if (!__nil_BindingIterator) {
        __nil_BindingIterator = new CosNaming::_nil_BindingIterator;
      }
      return __nil_BindingIterator;
    }
  private:
    static CosNaming_BindingIterator_ptr __nil_BindingIterator;
  };

};

#endif // __Naming_hh__
