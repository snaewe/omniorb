// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbOptions.h               Created on: 13/8/2001
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.2.9  2005/09/19 14:23:56  dgrisby
  New traceFile configuration parameter.

  Revision 1.1.2.8  2005/09/08 14:26:17  dgrisby
  New -ORBconfigFile command line argument.

  Revision 1.1.2.7  2003/08/21 14:57:38  dgrisby
  Really silly bug broke registry reading on Windows.

  Revision 1.1.2.6  2002/03/18 15:13:07  dpg1
  Fix bug with old-style ORBInitRef in config file; look for
  -ORBtraceLevel arg before anything else; update Windows registry
  key. Correct error message.

  Revision 1.1.2.5  2002/02/11 15:15:50  dpg1
  Things for ETS kernel.

  Revision 1.1.2.4  2001/08/21 11:02:12  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.3  2001/08/20 10:46:48  sll
  New orb configuration parsing now works with NT registry.

  Revision 1.1.2.2  2001/08/20 08:19:22  sll
  Read the new ORB configuration file format. Can still read old format.
  Can also set configuration parameters from environment variables.

  Revision 1.1.2.1  2001/08/17 17:12:34  sll
  Modularise ORB configuration parameters.

*/

#ifndef __ORBOPTIONS_H__
#define __ORBOPTIONS_H__

OMNI_NAMESPACE_BEGIN(omni)

class orbOptions;

class orbOptions {
 public:

  ////////////////////////////////////////////////////////////////////////
  class sequenceString;

  ////////////////////////////////////////////////////////////////////////
  class BadParam {
  public:
    BadParam(const char* k, const char* v, const char* y) :
      key(k),value(v),why(y) {}

    CORBA::String_var key;
    CORBA::String_var value;
    CORBA::String_var why;

    BadParam(const BadParam& b) : key(b.key), value(b.value), why(b.why) {}

  private:
    BadParam();
    BadParam& operator=(const BadParam&);
  };


  ////////////////////////////////////////////////////////////////////////
  class Unknown {
  public:
    Unknown(const char* k, const char* v) : key(k),value(v) {}

    CORBA::String_var key;
    CORBA::String_var value;

    Unknown(const Unknown& k) : key(k.key), value(k.value) {}

  private:
    Unknown();
    Unknown& operator=(const Unknown&);
  };

  ////////////////////////////////////////////////////////////////////////
  enum Source { fromFile, fromEnvironment, fromRegistry, fromArgv, 
		fromArray, fromInternal };


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  class Handler {
  public:

    const char* key() const { return key_; }
    const char* usage() const { return usage_; }
    const char* usageArgv() const { return argvUsage_; }
    CORBA::Boolean argvYes() const { return argvYes_; }
    CORBA::Boolean argvHasNoValue() const { return argvHasNoValue_; }

    virtual void visit(const char* value,Source source) throw (BadParam) = 0;
    virtual void dump(sequenceString& result) = 0;

  protected:
    Handler(const char* k, const char* u,
	    CORBA::Boolean yesorno, const char* arg_u,
	    CORBA::Boolean novalue = 0) :
      key_(k), usage_(u),
      argvYes_(yesorno), argvUsage_(arg_u), argvHasNoValue_(novalue) {}

    virtual ~Handler() {}

  private:

    const char*          key_;
    const char*          usage_;
    const CORBA::Boolean argvYes_;
    const char*          argvUsage_;
    const CORBA::Boolean argvHasNoValue_;

    Handler();
    Handler(const Handler&);
    Handler& operator=(const Handler&);
  };


  void registerHandler(Handler& h);
  // Register a handler for an option. The option's name is identified by
  // h->key().
  //
  // When subsequently this->visit() is called, h->visit() will be called
  // for each of the options that matches the option's name.
  //
  // h->argvYes() if true indicates that this option can be specified in
  // the ORB_init argument list. When this->extractInitOptions() is called,
  // the arguments will be searched and those that match the key of the
  // handler will be extracted.
  //
  // Any modules that have configuration options must call this method before
  // ORB_init is called. Typically this is done inside the ctor of the
  // initialiser singleton.
  //
  // Thread Safety preconditions:
  //    Not thread safe


  ////////////////////////////////////////////////////////////////////////
  void reset();
  // Remove any options that have been added previously with addOption() or
  // addOptions().

  ////////////////////////////////////////////////////////////////////////
  void visit() throw(BadParam);
  // Call this method will cause the object to walk through all the options
  // accumulated so far via addOption(). For each of these options, its
  // handler will be called.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void addOption(const char* key, const char* value, 
		 Source source=fromInternal) throw (Unknown,BadParam);
  // Add to the internal option list a <key,value> tuple.
  // Both arguments are copied.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void addOptions(const char* options[][2]) throw (Unknown,BadParam);
  // Add the option list. Each element of the variable size array is
  // a key, value pair. The array ends with a key, value pair that is both
  // nil(0) in value.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void extractInitOptions(int& argc, char** argv) throw (Unknown,BadParam);
  // Extract the ORB_init options from the argv list. Extract the arguments
  // from the argument list for those registered handlers that can accept
  // ORB_init arguments.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  void getTraceLevel(int argc, char** argv) throw (Unknown,BadParam);
  // Look for -ORBtraceLevel and -ORBtraceFile arguments very early
  // on, so the trace level can affect later option logging. Does not
  // remove the arguments -- that is done by extractInitOptions()
  // later.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  const char* getConfigFileName(int argc, char** argv, const char* fname)
    throw (Unknown,BadParam);
  // Look for an -ORBconfigFile argument before processing the config
  // file. Does not remove the arguments -- that is done by
  // extractInitOptions() later.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean importFromFile(const char* filename) throw (Unknown,BadParam);

#if defined(NTArchitecture) && !defined(__ETS_KERNEL__)
  ////////////////////////////////////////////////////////////////////////
  CORBA::Boolean importFromRegistry() throw (Unknown,BadParam);
#endif

  ////////////////////////////////////////////////////////////////////////
  void importFromEnv() throw (Unknown,BadParam);

  ////////////////////////////////////////////////////////////////////////
  sequenceString* usage() const;
  // Return the list of recognised options and their usage string.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  sequenceString* usageArgv() const;
  // Return the list of recognised options that can be specified as the
  // ORB_Init options.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  sequenceString* dumpSpecified() const;
  // Return the list of options entered by addOption() so far.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  sequenceString* dumpCurrentSet() const;
  // Return the list of available options and their current value.
  //
  // Caller is responsible for freeing the memory allocated to the
  // returned list.
  //
  // Thread Safety preconditions:
  //    Not thread safe


  ////////////////////////////////////////////////////////////////////////
  static orbOptions& singleton();
  // Returns the singleton orbOptions object. It is safe to call this
  // function in static initialisers. Typcially usage is to call
  // singleton()->registerHandler() in the ctor of initialiser singletons.
  //
  // Thread Safety preconditions:
  //    Not thread safe

  ////////////////////////////////////////////////////////////////////////
  // Helper functions for Handler classes to use.
  //    get* functions return True(1) if the value string can be parsed
  //                   correctly.
  //    addKV* functions append to the sequenceString a stringified value
  //                     of the key value pair.
  //
  static CORBA::Boolean getBoolean(const char* value, CORBA::Boolean& result);
  static CORBA::Boolean getULong(const char* value, CORBA::ULong& result);
  static void addKVBoolean(const char* key, CORBA::Boolean,sequenceString&);
  static void addKVULong(const char* key, CORBA::ULong,sequenceString&);
  static void addKVString(const char* key, const char* value, sequenceString&);

  static void move_args(int& argc,char **argv,int idx,int nargs);
  // Move the arguments at argv[idx--(idx+nargs-1)] to the end of
  // argv. Update argc to truncate the moved arguments from argv.

  static const char* expect_boolean_msg;
  static const char* expect_ulong_msg;
  static const char* expect_greater_than_zero_ulong_msg;

  ////////////////////////////////////////////////////////////////////////
  class sequenceString_var;

  class sequenceString : public _CORBA_Unbounded_Sequence_String {
  public:
    typedef sequenceString_var _var_type;
    inline sequenceString() {}
    inline sequenceString(const sequenceString& s)
      : _CORBA_Unbounded_Sequence_String(s) {}

    inline sequenceString(_CORBA_ULong _max)
      : _CORBA_Unbounded_Sequence_String(_max) {}
    inline sequenceString(_CORBA_ULong _max, _CORBA_ULong _len, char** _val, _CORBA_Boolean _rel=0)
      : _CORBA_Unbounded_Sequence_String(_max, _len, _val, _rel) {}



    inline sequenceString& operator = (const sequenceString& s) {
      _CORBA_Unbounded_Sequence_String::operator=(s);
      return *this;
    }
  };

  ////////////////////////////////////////////////////////////////////////
  class sequenceString_var {
  public:
    typedef sequenceString T;
    typedef sequenceString_var T_var;

    inline sequenceString_var() : _pd_seq(0) {}
    inline sequenceString_var(T* s) : _pd_seq(s) {}
    inline sequenceString_var(const T_var& s) {
      if( s._pd_seq )  _pd_seq = new T(*s._pd_seq);
      else             _pd_seq = 0;
    }
    inline ~sequenceString_var() { if( _pd_seq )  delete _pd_seq; }

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

    inline _CORBA_String_element operator [] (_CORBA_ULong s) {
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

  private:
    T* _pd_seq;
  };



#ifdef __GNUG__
  friend class _keep_gcc_quiet_;
#endif

  friend class omni_orbOptions_initialiser;

 private:

  omnivector<Handler*> pd_handlers;
  CORBA::Boolean       pd_handlers_sorted;
  Handler* findHandler(const char* k);
  void     sortHandlers();

  struct HandlerValuePair {

    HandlerValuePair(Handler* h, const char* v, Source s) :
      handler_(h),value_(v),source_(s) {}

    Handler*               handler_;
    CORBA::String_var      value_;
    Source                 source_;
  };
  omnivector<HandlerValuePair*>   pd_values;

  orbOptions();
  ~orbOptions();

  orbOptions(const orbOptions&);
  orbOptions& operator=(const orbOptions&);
};

OMNI_NAMESPACE_END(omni)

#endif // __ORBOPTIONS_H__
