// -*- Mode: C++; -*-
//                            Package   : omniORB2
// CORBA.h                    Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
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
// 	A complete set of C++ definitions for the CORBA module. The definitions
//	appear within the C++ class named CORBA. This mapping is fully
//      compliant with the CORBA 2.0 specification.
// 

/*
 $Log$
 Revision 1.19  1998/02/20 14:44:44  ewc
 Changed to compile with aCC on HPUX

 * Revision 1.18  1998/02/03  16:47:09  ewc
 * Updated some interfaces.
 *
 * Revision 1.17  1998/01/27  16:02:34  ewc
 * Added TypeCode and type Any
 *
 Revision 1.16  1997/12/18 17:37:20  sll
 Added (const char*) type casting to help strcpy().

 Revision 1.15  1997/12/09 20:35:16  sll
 New members BOA::impl_shutdown, BOA::destroy.

 Revision 1.14  1997/08/21 22:20:17  sll
 - String_member copy ctor bug fix.
 - New system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
   INVALID_TRANSACTION and WRONG_TRANSACTION.
 - Correct ORB_init() signature.
 - CORBA::is_nil(Object_ptr) is now more sympathetic to applications
   treating a nil pointer as a nil object reference.

 * Revision 1.13  1997/05/21  15:01:40  sll
 * Added typedef <type>_ptr <type>Ref;
 *
 * Revision 1.12  1997/05/06  16:04:43  sll
 * Public release.
 *
*/

#ifndef __CORBA_H__
#define __CORBA_H__

#include <omniORB2/omniInternal.h>

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
	strcpy(_data,(const char*)s);
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
	strcpy(_data,(const char*)s);
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
      _ptr = 0;
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


////////////////////////////////////////////////////////////////////////
//                   Type Any                                         //
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
    tk_except	= 22
  };
  
  class Object;
  typedef Object *Object_ptr;
  typedef Object_ptr ObjectRef;

  class TypeCode;
  typedef class TypeCode *TypeCode_ptr;
  typedef TypeCode_ptr TypeCodeRef;

  static const TypeCode_ptr _tc_null;
  static const TypeCode_ptr _tc_void;
  static const TypeCode_ptr _tc_short;
  static const TypeCode_ptr _tc_long;
  static const TypeCode_ptr _tc_ushort;
  static const TypeCode_ptr _tc_ulong;
  static const TypeCode_ptr _tc_float;
  static const TypeCode_ptr _tc_double;
  static const TypeCode_ptr _tc_boolean;
  static const TypeCode_ptr _tc_char;
  static const TypeCode_ptr _tc_octet;
  static const TypeCode_ptr _tc_any;
  static const TypeCode_ptr _tc_TypeCode;
  static const TypeCode_ptr _tc_Principal;
  static const TypeCode_ptr _tc_Object;
  static const TypeCode_ptr _tc_string;

  static const TypeCode_ptr __nil_TypeCode; 


  class Any {
  public:
    Any() : pd_data(0) { 
      pd_tc = new TypeCode(tk_null); 
    }

    ~Any() { 
      CORBA::release(pd_tc); 
      PR_deleteData();
    }

    Any(const Any& a) : pd_data(0) {
      if ((a.pd_tc)->NP_is_nil()) pd_tc = __nil_TypeCode;
      else pd_tc = new TypeCode(*(a.pd_tc));
      pd_mbuf = a.pd_mbuf;
    }
    
    Any(TypeCode_ptr tc, void* value, Boolean release = 0,
	Boolean nocheck = 0);	

    
    void operator>>= (NetBufferedStream& s) const;  // Marshalling and 
    void operator<<= (NetBufferedStream& s);  // unmarshalling member functions

    void operator>>= (MemBufferedStream& s) const;
    void operator<<= (MemBufferedStream& s);

    size_t NP_alignedSize(size_t initialoffset) const;

    void NP_memAlignMarshal(MemBufferedStream& m);

    inline ULong NP_length() const {
      return (ULong) pd_mbuf.unRead();
    }

    void NP_holdData(void* data, void (*del)(void*)) const;
    void NP_getBuffer(MemBufferedStream& mbuf) const;
    void* NP_data() const { return pd_data; }
	
    void NP_replaceData(TypeCode_ptr tcp, const MemBufferedStream& mb);

  protected:
    inline void PR_fill(UShort fillerLen,MemBufferedStream& mbuf) const {
      if (fillerLen > 0) {
	Char* filler = new Char[fillerLen];
	try {
	  mbuf.put_char_array(filler,fillerLen);
	}
	catch (...) {
	  delete[] filler;
	  throw;
	}
	delete[] filler;
      }
    }

    inline void PR_deleteData() {
      if (pd_data) {
	(*deleteData)(pd_data);	
	pd_data = 0;
      }
    }

  public:

    inline Any &operator=(const Any& a) {
      if (this != &a) {
	CORBA::release(pd_tc);
	PR_deleteData();
	if ((a.pd_tc)->NP_is_nil()) pd_tc = __nil_TypeCode;
	else pd_tc = new TypeCode(*(a.pd_tc));
	pd_mbuf = a.pd_mbuf;
      }
      return *this;
    }    



    inline void operator<<=(Short s) {
      MemBufferedStream mbuf;
      s >>= mbuf;
      NP_replaceData(_tc_short, mbuf);
    }

    inline void operator<<=(UShort u) {
      MemBufferedStream mbuf;
      u >>= mbuf;
      NP_replaceData(_tc_ushort, mbuf);
    }	

    inline void operator<<=(Long l) {
      MemBufferedStream mbuf;
      l >>= mbuf;
      NP_replaceData(_tc_long, mbuf);
    }

    inline void operator<<=(ULong u) {
      MemBufferedStream mbuf;
      u >>= mbuf;
      NP_replaceData(_tc_ulong, mbuf);
    }

#if !defined(NO_FLOAT)
    inline void operator<<=(Float f) {
      MemBufferedStream mbuf;
      f >>= mbuf;
      NP_replaceData(_tc_float, mbuf); 	
    }

    inline void operator<<=(Double d) {
      MemBufferedStream mbuf;
      d >>= mbuf;
      NP_replaceData(_tc_double, mbuf);
    }
#endif

    inline void operator<<=(const Any& a) {
      MemBufferedStream mbuf;
      a >>= mbuf;
      NP_replaceData(_tc_any,mbuf);
    }	

    inline void operator<<=(TypeCode_ptr tc) {
      // Copying version
      MemBufferedStream mbuf;
      *tc >>= mbuf;
      NP_replaceData(_tc_TypeCode, mbuf);
    }

    inline void operator<<=(TypeCode_ptr* tcp) {
      // Non - copying version
      this->operator<<=(*tcp);
      CORBA::release(*tcp);
    }
    
    inline void operator<<=(const char*& s) {
      MemBufferedStream mbuf;
      ULong _len = strlen(s) + 1;
      _len >>= mbuf;
      mbuf.put_char_array((const CORBA::Char*) s, _len);
      NP_replaceData(_tc_string, mbuf);
    }
	

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

    struct from_string {
      from_string(char* s, ULong b, Boolean nocopy = 0) : val(s), bound(b),
	nc(nocopy) 
	{ }
      
      char* val;
      ULong bound;
      Boolean nc;
    };


    inline void operator<<=(from_boolean f) {
      MemBufferedStream mbuf;
      f.val >>= mbuf;
      NP_replaceData(_tc_boolean, mbuf);
    }
      
    inline void operator<<=(from_char c) {
      MemBufferedStream mbuf;
      c.val >>= mbuf;
      NP_replaceData(_tc_char, mbuf);
    }
	

    inline void operator<<=(from_octet o) {
      MemBufferedStream mbuf;
      o.val >>= mbuf;
      NP_replaceData(_tc_octet, mbuf);
    }
    
    void operator<<=(from_string s);


    inline Boolean operator>>=(Short& s) const {
	if (!pd_tc->NP_expandEqual(_tc_short,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  s <<= tmp_mbuf;
	  return 1;
	}
    }
    
    inline Boolean operator>>=(UShort& u) const {
	if (!pd_tc->NP_expandEqual(_tc_ushort,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  u <<= tmp_mbuf;
	  return 1;
	}
    }

    inline Boolean operator>>=(Long& l) const {
	if (!pd_tc->NP_expandEqual(_tc_long,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  l <<= tmp_mbuf;
	  return 1;
	}
    }

    inline Boolean operator>>=(ULong& u) const {
	if (!pd_tc->NP_expandEqual(_tc_ulong,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  u <<= tmp_mbuf;
	  return 1;
	}
    }
    
#if !defined(NO_FLOAT)
    inline Boolean operator>>=(Float& f) const {
	if (!pd_tc->NP_expandEqual(_tc_float,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  f <<= tmp_mbuf;
	  return 1;
	}
      }

    inline Boolean operator>>=(Double& d) const {
	if (!pd_tc->NP_expandEqual(_tc_double,1)) return 0;
	else {
	  MemBufferedStream tmp_mbuf(pd_mbuf,1);
	  d <<= tmp_mbuf;
	  return 1;
	}
    }

#endif

    Boolean operator>>=(Any& a) const; 

    Boolean operator>>=(TypeCode_ptr& tc) const; 

    Boolean operator>>=(char*& s) const;


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

    struct to_string {
      to_string(char*& s, ULong b) : val(s), bound(b) { }
      
      char*& val;
      ULong bound;
    };
      
    struct to_object {
      to_object(Object_ptr& obj) : ref(obj) { }
      Object_ptr& ref;
    };
    
    inline Boolean operator>>=(to_boolean b) const {
      if (!pd_tc->NP_expandEqual(_tc_boolean,1)) return 0;
      else {
	MemBufferedStream tmp_mbuf(pd_mbuf,1);
	b.ref <<= tmp_mbuf;
	return 1;
      }
    }

    inline Boolean operator>>=(to_char c) const {
      if (!pd_tc->NP_expandEqual(_tc_char,1)) return 0;
      else {
	MemBufferedStream tmp_mbuf(pd_mbuf,1);
	c.ref <<= tmp_mbuf;
	return 1;
      }
    }
    
    inline Boolean operator>>=(to_octet o) const {
      if (!pd_tc->NP_expandEqual(_tc_octet,1)) return 0;
      else {
	MemBufferedStream tmp_mbuf(pd_mbuf,1);
	o.ref <<= tmp_mbuf;
	return 1;
      }
    }

    Boolean operator>>=(to_string s) const;

    Boolean operator>>=(to_object o) const;

    void replace(TypeCode_ptr TCp, void* value, Boolean release = 0);

    inline TypeCode_ptr type() const {
      if (pd_tc->NP_is_nil()) return __nil_TypeCode;
      else return new TypeCode(*pd_tc);
      }

    inline const void *value() const {
      return pd_mbuf.data();
    }


  private:
    void operator<<=(unsigned char);
    Boolean operator>>=(unsigned char&) const;

    void* pd_data;
    void (*deleteData)(void*);

    TypeCode_ptr pd_tc;
    MemBufferedStream pd_mbuf;
  };

  
  class Any_OUT_arg;

  class Any_var {
  public:
    inline Any_var() { pd_data = 0; }
    inline Any_var(Any* p) { pd_data = p; }
    inline Any_var(const Any_var &p) {
      if (!p.pd_data) {
	pd_data = 0;
	return;
      }
      else {
	pd_data = new Any;
	if (!pd_data) {
	  _CORBA_new_operator_return_null();
	  // never reach here
	}
	*pd_data = *p.pd_data;
      }
    }

    inline ~Any_var() {  if (pd_data) delete pd_data; }
    inline Any_var &operator= (Any* p) {  
      if (pd_data) delete pd_data;
      pd_data = p;
      return *this;
    }

    inline Any_var &operator= (const Any_var &p) {
        if (p.pd_data) {
	  if (!pd_data) {
	    pd_data = new Any;
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

    inline Any* operator->() const { return (Any*)pd_data; }
    
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator Any& () const { return (Any&) *pd_data; }
#else
    inline operator const Any& () const { return *pd_data; }
    inline operator Any& () { return *pd_data; }
#endif


    // Any member-function insertion operators:

    inline void operator<<=(Short s) {
      *pd_data <<= s;
    }

    inline void operator<<=(UShort u) {
      *pd_data <<= u;
    }	

    inline void operator<<=(Long l) {
      *pd_data <<= l;
   }

    inline void operator<<=(ULong u) {
      *pd_data <<= u;
    }

#if !defined(NO_FLOAT)
    inline void operator<<=(Float f) {
      *pd_data <<= f;
    }

    inline void operator<<=(Double d) {
      *pd_data <<= d;
    }
#endif

    inline void operator<<=(const Any& a) {
      *pd_data <<= a;
    }	

    inline void operator<<=(TypeCode_ptr tc) {
      *pd_data <<= tc;
    }

    inline void operator<<=(TypeCode_ptr* tcp) {
      *pd_data <<= tcp;
    }
    
    inline void operator<<=(const char*& s) {
      *pd_data <<= s;
    }
	
    inline void operator<<=(Any::from_boolean f) {
      *pd_data <<= f;
    }
      
    inline void operator<<=(Any::from_char c) {
      *pd_data <<= c;
    }
	

    inline void operator<<=(Any::from_octet o) {
      *pd_data <<= o;
    }
    
    inline void operator<<=(Any::from_string s){
      *pd_data <<= s;
    }


    // Any member-function extraction operators:

    inline Boolean operator>>=(Short& s) const {
      return (*pd_data >>= s);
    }
    
    inline Boolean operator>>=(UShort& u) const {
      return (*pd_data >>= u);
    }

    inline Boolean operator>>=(Long& l) const {
      return (*pd_data >>= l);
    }

    inline Boolean operator>>=(ULong& u) const {
      return (*pd_data >>= u);
    }
    
#if !defined(NO_FLOAT)
    inline Boolean operator>>=(Float& f) const {
      return (*pd_data >>= f);
      }

    inline Boolean operator>>=(Double& d) const {
      return (*pd_data >>= d);
    }

#endif

    inline Boolean operator>>=(Any& a) const {
      return (*pd_data >>= a);
    }

    inline Boolean operator>>=(TypeCode_ptr& tc) const {
      return (*pd_data >>= tc);
    }

    inline Boolean operator>>=(char*& s) const {
      return (*pd_data >>= s);
    }

    inline Boolean operator>>=(Any::to_boolean b) const {
      return (*pd_data >>= b);
    }

    inline Boolean operator>>=(Any::to_char c) const {
      return (*pd_data >>= c);
    }
    
    inline Boolean operator>>=(Any::to_octet o) const {
      return (*pd_data >>= o);
    }

    inline Boolean operator>>=(Any::to_string s) const {
      return (*pd_data >>= s);
    }

    inline Boolean operator>>=(Any::to_object o) const {
      return (*pd_data >>= o);
    }


    friend class Any_OUT_arg;

  private:
    Any* pd_data;
  };


  class Any_OUT_arg {
  public:
    inline Any_OUT_arg(Any*& p) : _data(p) {}
    inline Any_OUT_arg(Any_var& p) : _data(p.pd_data) {
      p = (Any*)0;
    }
    Any*& _data;
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
  STD_EXCEPTION (TRANSACTION_REQUIRED);
  STD_EXCEPTION (TRANSACTION_ROLLEDBACK);
  STD_EXCEPTION (INVALID_TRANSACTION);
  STD_EXCEPTION (WRONG_TRANSACTION);

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
//                   PIDL Environment                                 //
////////////////////////////////////////////////////////////////////////

  class Environment;
  typedef Environment *Environment_ptr;
  typedef Environment_ptr EnvironmentRef;

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
  typedef NamedValue_ptr NamedValueRef;

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
  typedef NVList_ptr NVListRef;


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
  typedef Context_ptr ContextRef;

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
  typedef ContextList_ptr ContextListRef;

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
  typedef Principal_ptr PrincipalRef;

  typedef _CORBA_Unbounded_Sequence_Octet PrincipalID;

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
  typedef ExceptionList_ptr ExceptionListRef;


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
  typedef InterfaceDef_ptr InterfaceDefRef;

  enum DefinitionKind { 
    dk_none, dk_all, dk_Attribute, dk_Constant, 
    dk_Exception, dk_Interface, dk_Module, dk_Operation, 
    dk_Typedef, dk_Alias, dk_Struct, dk_Union, dk_Enum, 
    dk_Primitive, dk_String, dk_Sequence, dk_Array, dk_Repository 
  };

  friend inline void operator>>= (DefinitionKind _e,NetBufferedStream &s) {
    ::operator>>=((CORBA::ULong)_e,s);
  }

  friend inline void operator<<= (DefinitionKind &_e,NetBufferedStream &s) {
    CORBA::ULong __e;
    __e <<= s;
    switch (__e) {
      case dk_none:
      case dk_all:
      case dk_Attribute:
      case dk_Constant:
      case dk_Exception:
      case dk_Interface:
      case dk_Module:
      case dk_Operation:
      case dk_Typedef:
      case dk_Alias:
      case dk_Struct:
      case dk_Union:
      case dk_Enum:
      case dk_Primitive:
      case dk_String:
      case dk_Sequence:
      case dk_Array:
      case dk_Repository:
        _e = (DefinitionKind) __e;
        break;
      default:
        _CORBA_marshal_error();
    }
  }

  friend inline void operator>>= (DefinitionKind _e,MemBufferedStream &s) {
    ::operator>>=((CORBA::ULong)_e,s);
  }

  friend inline void operator<<= (DefinitionKind &_e,MemBufferedStream &s) {
    CORBA::ULong __e;
    __e <<= s;
    switch (__e) {
      case dk_none:
      case dk_all:
      case dk_Attribute:
      case dk_Constant:
      case dk_Exception:
      case dk_Interface:
      case dk_Module:
      case dk_Operation:
      case dk_Typedef:
      case dk_Alias:
      case dk_Struct:
      case dk_Union:
      case dk_Enum:
      case dk_Primitive:
      case dk_String:
      case dk_Sequence:
      case dk_Array:
      case dk_Repository:
        _e = (DefinitionKind) __e;
        break;
      default:
        _CORBA_marshal_error();
    }
  }

  
  class IRObject;
  typedef IRObject *IRObject_ptr;
  typedef IRObject_ptr IRObjectRef;

  class IRObject {
  public:

    // Not implemented yet - used by IR

    virtual ~IRObject() {}

    virtual DefinitionKind def_kind() {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return dk_none;
    }

    virtual void destroy() {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
    }

    static IRObject_ptr _duplicate(IRObject_ptr obj) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }
      
    static IRObject_ptr _narrow(Object_ptr obj) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }

    static IRObject_ptr _nil() {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }

  protected:
    IRObject() { }

  private:    
    IRObject(const IRObject&);
    IRObject &operator=(const IRObject&);
  };
  
  class IDLType;
  typedef IDLType* IDLType_ptr;
  typedef IDLType_ptr IDLTypeRef;

  class TypeCode_var;

  class TypeCode_member {
  public:
    TypeCode_member() { _ptr = __nil_TypeCode; }

    inline TypeCode_member(TypeCode_ptr p) { _ptr = p; }

    inline TypeCode_member(const TypeCode_member& p) {
      if ((p._ptr)->NP_is_nil()) _ptr = __nil_TypeCode; 
      else _ptr = new TypeCode(*(p._ptr));
      }

    ~TypeCode_member() { CORBA::release(_ptr); }

    
    inline TypeCode_member& operator=(TypeCode_ptr p) {
    CORBA::release(_ptr);
    _ptr = p;
    return *this;
    }

    
    inline TypeCode_member& operator=(const TypeCode_member& p) {
      if (this != &p) {
	CORBA::release(_ptr);
      if ((p._ptr)->NP_is_nil()) _ptr = __nil_TypeCode; 
      else _ptr = new TypeCode(*(p._ptr));
      }
      return *this;
    }

    inline TypeCode_member& operator=(const TypeCode_var& p) {
      CORBA::release(_ptr);
      if ((p.pd_TC)->NP_is_nil()) _ptr = __nil_TypeCode;
      else _ptr = new TypeCode(*(p.pd_TC));
      return *this;
    }


    inline void operator>>=(NetBufferedStream& s) const { *_ptr >>= s; }
    inline void operator<<=(NetBufferedStream& s) {
	TypeCode_ptr _result = new TypeCode(tk_null);
	*_result <<= s;
	CORBA::release(_ptr);
	_ptr = _result;
      }

    inline void operator>>=(MemBufferedStream& s) const { *_ptr >>= s; }
    inline void operator<<=(MemBufferedStream& s) {
	TypeCode_ptr _result = new TypeCode(tk_null);
	*_result <<= s;
	CORBA::release(_ptr);
	_ptr = _result;
    }
      
    inline size_t NP_alignedSize(size_t initialoffset) const {
      return _ptr->NP_alignedSize(initialoffset);
    }

    
    inline TypeCode_ptr operator->() const { return (TypeCode_ptr) _ptr; }
    inline operator TypeCode_ptr() const { return _ptr; }


    TypeCode_ptr _ptr;
  };



  struct StructMember {    
    String_member name;
    TypeCode_member type;
    IDLType_ptr type_def;

    size_t NP_alignedSize(size_t initialoffset) const;
    void operator>>= (NetBufferedStream &) const;
    void operator<<= (NetBufferedStream &);
    void operator>>= (MemBufferedStream &) const;
    void operator<<= (MemBufferedStream &);
  };

  typedef _CORBA_ConstrType_Variable_Var<StructMember> StructMember_var;
  typedef _CORBA_Unbounded_Sequence<StructMember> StructMemberSeq;


  struct UnionMember {
    String_member name;
    Any label;
    TypeCode_member type;
    IDLType_ptr type_def;

    size_t NP_alignedSize(size_t initialoffset) const;
    void operator>>= (NetBufferedStream &) const;
    void operator<<= (NetBufferedStream &);
    void operator>>= (MemBufferedStream &) const;
    void operator<<= (MemBufferedStream &);
  };

  typedef _CORBA_ConstrType_Variable_Var<UnionMember> UnionMember_var;
  typedef _CORBA_Unbounded_Sequence<UnionMember> UnionMemberSeq;


  typedef _CORBA_Unbounded_Sequence<String_member> EnumMemberSeq;


////////////////////////////////////////////////////////////////////////
//                   PIDL Object                                      //
////////////////////////////////////////////////////////////////////////

  class Request;
  typedef Request *Request_ptr;
  typedef Request_ptr RequestRef;

  typedef _CORBA_Unbounded_Sequence_Octet ReferenceData;

  class ImplementationDef;
  typedef ImplementationDef* ImplementationDef_ptr;
  typedef ImplementationDef_ptr ImplementationDefRef;

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
    static const _CORBA_Char* repositoryID;

  private:
    omniObject *pd_obj;
  };


  class _nil_IDLType;

  static IDLType_ptr __nil_IDLType;

  class IDLType : public virtual IRObject, public virtual omniObject,
                  public virtual Object {
  public:

    // Not implemented yet - used by IR

    virtual ~IDLType() {}

    virtual TypeCode_ptr type() {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }

    static IDLType_ptr _duplicate(IDLType_ptr obj) {
      if (CORBA::is_nil(obj))
	return IDLType::_nil();
      else {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
      }
    }
    
    static IDLType_ptr _narrow(Object_ptr obj) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }
    
    static inline IDLType_ptr _nil()  {
      if (!CORBA::__nil_IDLType) {
	CORBA::__nil_IDLType = new _nil_IDLType;
      }
      return __nil_IDLType;
    }

    static inline size_t NP_alignedSize(IDLType_ptr obj,size_t initialoffset) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;      
    }

    static inline void marshalObjRef(IDLType_ptr obj,NetBufferedStream &s) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
    }

    static inline IDLType_ptr unmarshalObjRef(NetBufferedStream &s) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }

    static inline void marshalObjRef(IDLType_ptr obj,MemBufferedStream &s) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
    }

    static inline IDLType_ptr unmarshalObjRef(MemBufferedStream &s) {
	throw CORBA::NO_IMPLEMENT(0,CORBA::COMPLETED_NO);
	return 0;
    }

  protected:
    
    IDLType() {}
  };

  class _nil_IDLType : public virtual IDLType {
  public:
    _nil_IDLType() : omniObject(omniObject::nilObjectManager()) 
      { this->PR_setobj(0); }
    virtual ~_nil_IDLType() {}
    
    TypeCode_ptr type (){
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      TypeCode_ptr _result= 0;
      return _result;
    }
  };


////////////////////////////////////////////////////////////////////////
//                   PIDL TypeCode                                    //
////////////////////////////////////////////////////////////////////////

  struct PR_structMember {
    // Private omniORB type
    const char* name;
    TypeCode_ptr type;
  };

  struct PR_unionMember {
    // Private omniORB type
    const char* name;
    TypeCode_ptr type;
    _CORBA_ULong label;
  };
  
  
  class TypeCode {
  public:

    TypeCode(TCKind t, ULong maxLen = 0); // Predefined-type and string

    TypeCode(const char* repoId, const char* name);
                            // Object Reference

    TypeCode(const char* repoId, const char* name, 
	     const TypeCode& contentTC);
                            // Typedef
    TypeCode(TCKind t, ULong length, const TypeCode& contentTC); 
                                                  // Sequence, array
    TypeCode(TCKind t, const char* repoId, const char* name, 
	     const StructMemberSeq& members);          
                                                 // Struct, Exception
    TypeCode(const char* repoId, const char* name, 
 	     const EnumMemberSeq& members);            // Enum

    TypeCode(const char* repoId, const char* name, 
	     const TypeCode_ptr discriminatorTC, 
	     const UnionMemberSeq& members);           // Union

    // TypeCode interface for stubs:

    TypeCode(TCKind t, const char* repoId, const char* name, 
	     PR_structMember* members, ULong num_mems);          
                                                 // Struct, Exception
    TypeCode(const char* repoId, const char* name, 
 	     char** members, ULong num_mems);            // Enum

    TypeCode(const char* repoId, const char* name, 
	     const TypeCode& discriminatorTC, 
	     PR_unionMember* members, ULong num_mems, Long def = -1);     
                                                          // Union


    virtual ~TypeCode() { }                          // Destructor

    TypeCode(const TypeCode& tc);            // Copy Constructor
    TypeCode& operator=(const TypeCode& tc); 
 
    
    void operator>>= (NetBufferedStream& s) const;  // Marshalling and 
    void operator<<= (NetBufferedStream& s);  // unmarshalling member functions

    void operator>>= (MemBufferedStream& s) const;
    void operator<<= (MemBufferedStream& s);

    size_t NP_alignedSize(size_t initialoffset) const;
    Boolean NP_expandEqual(TypeCode_ptr, Boolean) const;
    void NP_fillInit(MemBufferedStream& mbuf) const;
    virtual Boolean NP_is_nil() const;
    TypeCode_ptr NP_aliasExpand() const;
    TypeCode_ptr NP_completeExpand() const;


    // OMG Interface:

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

    

    inline TCKind kind() const { return pd_tck; }
    inline Boolean equal(TypeCode_ptr TCp) const { 
      return NP_expandEqual(TCp,0); }

    virtual const char* id() const;
    virtual const char* name() const;
    
    virtual ULong member_count() const;
    virtual const char* member_name(ULong index) const;

    virtual TypeCode_ptr member_type(ULong index) const;

    virtual Any *member_label(ULong index) const;
    virtual TypeCode_ptr discriminator_type() const;
    virtual Long default_index() const;

    virtual ULong length() const;

    virtual TypeCode_ptr content_type() const;

    virtual Long param_count() const;
    virtual Any *parameter(Long) const;

    static TypeCode_ptr _duplicate(TypeCode_ptr t);
    static TypeCode_ptr _nil();

    friend class tcParseEngine;
    friend class Any;

  private:

    TCKind pd_tck;
    ULong  pd_maxLen;
    MemBufferedStream pd_param;

    Boolean PR_equal(TypeCode_ptr, Boolean expand) const;

    TypeCode();
  };



  class _nil_TypeCode : public virtual TypeCode {
  public:
    _nil_TypeCode() : TypeCode(tk_null) {  }
    virtual ~_nil_TypeCode() {}

    virtual Boolean NP_is_nil() const;

    // OMG interface:

    const char* id () const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      char * _result= 0;
      return _result;
    }

    const char* name () const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      char * _result= 0;
      return _result;
    }

    ULong member_count () const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      ULong _result = 0;
      return _result;
    }

    const char* member_name(ULong index) const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      char * _result= 0;
      return _result;
    }

    TypeCode_ptr member_type(ULong index) const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      TypeCode_ptr _result= 0;
      return _result;
    }

    Any* member_label(ULong index) const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      Any * _result= 0;
      return _result;
    }

    TypeCode_ptr discriminator_type() const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      TypeCode_ptr _result= 0;
      return _result;
    }

    Long  default_index() const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      Long _result = 0;
      return _result;
    }

    ULong length() const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      ULong _result = 0;
      return _result;
    }

    TypeCode_ptr content_type() const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      TypeCode_ptr _result= 0;
      return _result;
    }

    Long param_count() const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      Long _result = 0;
      return _result;
    }

    Any* parameter(Long index) const {
      throw CORBA::BAD_OPERATION(0,CORBA::COMPLETED_NO);
      // never reach here! Dummy return to keep some compilers happy.
      Any * _result= 0;
      return _result;
    }
  };



  class TypeCode_INOUT_arg;
  class TypeCode_OUT_arg;


  class TypeCode_var {
  public:
    inline TypeCode_var() { pd_TC = CORBA::TypeCode::_nil(); }

    inline TypeCode_var(TypeCode_ptr p) { pd_TC = p; }

    inline TypeCode_var(const TypeCode_var& p) { 
	pd_TC = CORBA::TypeCode::_duplicate(p.pd_TC); 
    }

    inline ~TypeCode_var() { CORBA::release(pd_TC); }

    TypeCode_var(const TypeCode_member& p) { 
      pd_TC = CORBA::TypeCode::_duplicate(p._ptr);
    }

    inline TypeCode_var& operator= (TypeCode_ptr p) {
      CORBA::release(pd_TC);
      pd_TC = p;
      return *this;
    }

    inline TypeCode_var& operator= (const TypeCode_var& p) {
      if (this != &p)
	{
	  CORBA::release(pd_TC);
	  pd_TC = CORBA::TypeCode::_duplicate(p.pd_TC);
	}
      return *this;
    }

    inline TypeCode_var& operator=(const TypeCode_member& p) {
      CORBA::release(pd_TC);
      pd_TC = CORBA::TypeCode::_duplicate(p._ptr);
      return *this;
    }
      

    inline TypeCode_ptr operator->() const { return (TypeCode_ptr) pd_TC; }
    
    inline operator TypeCode_ptr() const { return pd_TC; }

    friend class TypeCode_member;
    friend class TypeCode_INOUT_arg;
    friend class TypeCode_OUT_arg;

  private:
    TypeCode_ptr pd_TC;
  };



  class TypeCode_INOUT_arg {
  public:
    inline TypeCode_INOUT_arg(TypeCode_ptr& p) : _data(p) { }
    inline TypeCode_INOUT_arg(TypeCode_var& p) : _data(p.pd_TC) { }
    inline TypeCode_INOUT_arg(TypeCode_member& p) : _data(p._ptr) { }
    inline ~TypeCode_INOUT_arg() { }

    TypeCode_ptr& _data;

  private:
    TypeCode_INOUT_arg();
  };


  class TypeCode_OUT_arg {
  public:
    TypeCode_OUT_arg(TypeCode_ptr& p) : _data(p) { }
    TypeCode_OUT_arg(TypeCode_var& p) : _data(p.pd_TC) {
      p = CORBA::TypeCode::_nil();
    }
    inline TypeCode_OUT_arg(TypeCode_member& p) : _data(p._ptr) {
      p = CORBA::TypeCode::_nil();
    }
    inline ~TypeCode_OUT_arg() { }
    
    TypeCode_ptr& _data;

  private:
    TypeCode_OUT_arg();
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
  typedef BOA_ptr BOARef;

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
    // The argument <NonBlocking> is omniORB2 specific.
    // Calling this function will cause the BOA to start accepting requests
    // from other address spaces. 

    void impl_shutdown();
    // omniORB2 specific.
    // This is the reverse of impl_is_ready().
    // When this call returns, all the internal threads and network
    // connections will be shutdown. 
    // Any thread blocking on impl_is_ready is unblocked.
    // When this call returns, requests from other address spaces will not
    // be dispatched.
    // The BOA can be reactivated by impl_is_ready(), it will continue to use 
    // the existing network addresses when reactivated.
    //
    // Note: This function should not be called in the implementation of a
    //       CORBA interface. Otherwise, this call will be blocked 
    //       indefinitely waiting on itself to complete the request.


    void destroy();
    // omniORB2 specific.
    // Calling this function will destroy this BOA. The function will call
    // impl_shutdown() implicitly if it has not been called. When this call
    // returns, the network addresses (endpoints) where this BOA listens on
    // will be freed.
    // Note: After this call, the BOA should not be used directly or
    //       indirectly, otherwise the behaviour is undefined. If there is
    //       any object implementation still registered with the BOA when this
    //       function is called, the object implementation should not be called
    //       afterwards. This function does not call the dispose method of
    //       the implementations.
    // Note: Initialisation of another BOA using ORB::BOA_init() is not 
    //       supported. The behaviour of ORB::BOA_init() after this function 
    //       is called is undefined.

    void obj_is_ready(Object_ptr, ImplementationDef_ptr p=0);

    static BOA_ptr _duplicate(BOA_ptr);
    static BOA_ptr _nil();

    static BOA_ptr getBOA();
    // omniORB2 specific. Must be called after BOA_init is called.
    // Otherwise a CORBA::OBJ_ADAPTOR exception is raised.

    BOA();
    ~BOA();
  };

////////////////////////////////////////////////////////////////////////
//                   PIDL ORB                                         //
////////////////////////////////////////////////////////////////////////

  class ORB;
  typedef class ORB *ORB_ptr;
  typedef ORB_ptr ORBRef;

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

    TypeCode_ptr 
    create_struct_tc(const char* id, const char* name, 
				  const StructMemberSeq& members);

    TypeCode_ptr 
    create_union_tc(const char* id, const char* name, 
	      TypeCode_ptr discriminator_type, const UnionMemberSeq& members);

    
    TypeCode_ptr 
    create_enum_tc(const char* id, const char* name, 
				  const EnumMemberSeq& members);

    TypeCode_ptr 
    create_alias_tc(const char* id, const char* name, 
				  TypeCode_ptr original_type);

    TypeCode_ptr 
    create_exception_tc(const char* id, const char* name, 
				  const StructMemberSeq& members);

    TypeCode_ptr 
    create_interface_tc(const char* id, const char* name);

    TypeCode_ptr 
    create_string_tc(ULong bound);

    TypeCode_ptr 
    create_sequence_tc(ULong bound, TypeCode_ptr element_type);

    TypeCode_ptr 
    create_array_tc(ULong length, TypeCode_ptr element_type);

    TypeCode_ptr
    create_recursive_sequence_tc(ULong bound, ULong offset);


    static ORB_ptr _duplicate(ORB_ptr p);
    static ORB_ptr _nil();

    ORB();
    ~ORB();

  };

  typedef char *ORBid;
  static ORB_ptr ORB_init(int &argc, char **argv, const char *orb_identifier);

////////////////////////////////////////////////////////////////////////
//                   PIDL release and is_null                         //
////////////////////////////////////////////////////////////////////////

  static inline Boolean is_nil(TypeCode_ptr o) { return (o) ? o->NP_is_nil() 
						            : 1; }
  static Boolean is_nil(Environment_ptr);
  static Boolean is_nil(Context_ptr);
  static Boolean is_nil(Principal_ptr);
  static inline Boolean is_nil(Object_ptr o) { 
    if (o)
      return o->NP_is_nil(); 
    else {
      // omniORB2 does not use a nil pointer to represent a nil object 
      // reference. The program has passed in a pointer which has not
      // been initialised by CORBA::Object::_nil() or similar functions.
      // Some ORBs seems to be quite lax about this. We don't want to
      // break the applications that make this assumption. Just call
      // _CORBA_use_nil_ptr_as_nil_objref() to take note of this.
      return _CORBA_use_nil_ptr_as_nil_objref();
    }
  }
  static Boolean is_nil(BOA_ptr p);
  static Boolean is_nil(ORB_ptr p);
  static Boolean is_nil(NamedValue_ptr);
  static Boolean is_nil(NVList_ptr);
  static Boolean is_nil(Request_ptr);

  static inline void release(TypeCode_ptr o) {
    if (!CORBA::is_nil(o)) delete o;
  }

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


#include <omniORB2/omniORB.h>
#include <omniORB2/templates.h>
#include <omniORB2/proxyFactory.h>

// omniORB2 private functions
extern CORBA::Boolean _omni_defaultTransientExceptionHandler(void* cookie,
						   CORBA::ULong n_retries,
						   const CORBA::TRANSIENT& ex);

extern CORBA::Boolean _omni_defaultCommFailureExceptionHandler(void* cookie,
						CORBA::ULong n_retries,
						const CORBA::COMM_FAILURE& ex);

extern CORBA::Boolean _omni_defaultSystemExceptionHandler(void* cookie,
					    CORBA::ULong n_retries,
					    const CORBA::SystemException& ex);

extern CORBA::Boolean _omni_callTransientExceptionHandler(omniObject*,
						CORBA::ULong,
						const CORBA::TRANSIENT&);
extern CORBA::Boolean _omni_callCommFailureExceptionHandler(omniObject*,
						  CORBA::ULong,
						  const CORBA::COMM_FAILURE&);
extern CORBA::Boolean _omni_callSystemExceptionHandler(omniObject*,
					     CORBA::ULong,
					     const CORBA::SystemException&);

// Include the COSS Naming Service header:
#include <omniORB2/Naming.hh>

#endif // __CORBA_H__
