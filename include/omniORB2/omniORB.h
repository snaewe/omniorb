// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniORB.h                  Created on: 6/2/96
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
//        This is the public API of omniORB2's extension to CORBA.
//      The API is intended to be used in application code.

/*
  $Log$
  Revision 1.12  1998/04/07 19:52:31  sll
  Updated to use namespace if available.
  New class logStream to log all error messages.

  Revision 1.11  1998/03/04 14:48:47  sll
  Added omniORB::giopServerThreadWrapper.

 * Revision 1.10  1998/03/02  17:05:29  ewc
 * Removed scoping from objectKey in class loader (caused problems compiling
 * with MSVC++ 5.0)
 *
 * Revision 1.9  1998/02/25  20:34:59  sll
 * New omniORB::loader class for adding dynamic object loader.
 *
 * Revision 1.8  1998/01/27  16:07:58  ewc
 * Added -ORBtcAliasExpand flag
 *
  Revision 1.7  1997/12/12 18:47:16  sll
  New variable serverName.

  Revision 1.6  1997/12/09 20:36:05  sll
  Support for system exception handlers.
  Support for incoming and outgoing rope scavenger threads.

 * Revision 1.5  1997/05/06  16:09:39  sll
 * Public release.
 *
 */

#ifndef __OMNIORB_H__
#define __OMNIORB_H__

#ifdef _LC_attr
#error "A local CPP macro _LC_attr has already been defined."
#else
#define _LC_attr _OMNIORB_NTDLL_IMPORT
#endif

_CORBA_MODULE omniORB

_CORBA_MODULE_BEG

  ////////////////////////////////////////////////////////////////////////
  // serverName								//
  //									//
  // omniORB2's idea of the name of the server.  Set to argv[0] by      //
  // ORB_init or changed by command-line option -ORBserverName <name>.  //
  // This name is used in the host-based access control of the IIOP     //
  // gatekeeper.                                                        //
  //                                                                    //
  //									//
#ifndef HAS_Cplusplus_Namespace                                         //
  static CORBA::String_var serverName;		                        //
#else                                                                   //
  _CORBA_MODULE_VAR char* serverName;                            //
#endif                                                                  //
  ////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  // Tracing level                                                     //
  //     level 0 - turn off all tracing and informational messages     //
  //     level 1 - informational messages only                         //
  //     level 2 - configuration information                           //
  //     level 5 - the above plus report server thread creation and    //
  //               communication socket shutdown                       //
  //     level 10 - the above plus execution trace messages            //
  //     ...                                                           //
  _CORBA_MODULE_VAR CORBA::ULong   traceLevel;                  //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceLevel <n>, or by direct assignment to this   //
  //     variable.                                                     //
  ///////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  // strictIIOP flag                                                   //
  //   Enable vigorous check on incoming IIOP messages                 //
  //                                                                   //
  // In some (sloppy) IIOP implementation, the message size value in   //
  // the header can be larger than the actual body size, i.e. there is //
  // garbage at the end. As the spec does not say the message size     //
  // must match the body size exactly, this is not a clear violation   //
  // of the spec.                                                      //
  //                                                                   //
  // If this flag is non-zero, the incoming message is expected to     //
  // be well behaved. Any messages that have garbage at the end will   //
  // be rejected.                                                      //
  //                                                                   //
  // The default value of this flag is zero and the ORB would silently //
  // skip the unread part. The problem with this behaviour is that the //
  // header message size may actually be garbage value, caused by a bug//
  // in the sender's code. Th receiving thread may forever block on    //
  // the strand as it tries to read more data from it. In this case the//
  // sender won't send anymore as it thinks it has marshalled in all   //
  // the data.							       //
  _CORBA_MODULE_VAR CORBA::Boolean   strictIIOP;                //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBstrictIIOP <0|1>, or by direct assignment to this //
  //     variable.                                                     //
  ///////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  //  tcAliasExpand flag is used to indicate whether TypeCodes        //
  //              associated with anys should have aliases removed.This//
  //              functionality is included because some ORBs will not //
  //              recognise an Any containing a TypeCode containing    //
  //              aliases to be the same as the actual type contained  //
  //              in the Any. Note that omniORB will always remove     //
  //              top-level aliases, but will not remove aliases from  //
  //              TypeCodes that are members of other TypeCodes (e.g.  //
  //              TypeCodes for members of structs etc.), unless       //
  //              tcAliasExpand is set to 1. There is a performance    //
  //              penalty when inserting into an Any if tcAliasExpand  //
  //              is set to 1. The default value is 0 (i.e. aliases of //
  //              member TypeCodes are not expanded).                  //
  //              Note that aliases won't be expanded when one of the  //
  //              non - type-safe methods of inserting into an Any is  //
  //              used (i.e. when the replace() member function or     //
  //              non - type-safe Any constructor is used. )           //
  ///////////////////////////////////////////////////////////////////////
  _CORBA_MODULE_VAR CORBA::Boolean tcAliasExpand;               //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtcAliasExpand <0|1>, or by direct assignment to   //
  //     this variable.                                                //
  ///////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // objectKey is a data type that uniquely identify each object        //
  //           implementation in the same address space. Its actual     //
  //           implmentation is not public. The data type should only   //
  //           be processed in an application by the following utility  //
  //           functions.                                               //
  //                                                                    //
  //                                                                    //
  typedef omniObjectKey objectKey;                                      //
  //                                                                    //
  //  size of the hash table used by hash().                            //
  _CORBA_MODULE_VAR const unsigned int hash_table_size;          //
  //                                                                    //
  //  hash()                                                            //
  //    return the hash value of this key. The return value             //
  //    is between 0 - (hash_table_size - 1);                           //
  _CORBA_MODULE_FN int hash(objectKey& k);                          //
  //                                                                    //
  // generateNewKey()                                                   //
  //   generate a new key. The key is guaranteed to be temporally       //
  //   unique. On OSs that provide unique process IDs, e.g. unices,     //
  //   the key is guaranteed to be unique among all keys ever generated //
  //   on the same machine.                                             //
  _CORBA_MODULE_FN void generateNewKey(objectKey &k);               //
  //                                                                    //
  // Return a fixed key value that always hash to 0.                    //
  _CORBA_MODULE_FN objectKey nullkey();                             //
  //                                                                    //
  // Return non-zero if the keys are the same                           //
  _CORBA_MODULE_OP int operator==(const objectKey &k1,              //
                                      const objectKey &k2);             //
  //                                                                    //
  // Return non-zero if the keys are different                          //
  _CORBA_MODULE_OP int operator!=(const objectKey &k1,              //
                                      const objectKey &k2);             //
  //                                                                    //
  //                                                                    //
  typedef _CORBA_Unbounded_Sequence_Octet seqOctets;                    //
  // Convert a key to a sequence of octets.                             //
  _CORBA_MODULE_FN seqOctets* keyToOctetSequence(const objectKey &k1);//
  //                                                                    //
  // Convert a sequence of octets back to an object key.                //
  // This function may throw a CORBA::MARSHAL exception if the sequence //
  // is not an object key.                                              //
  _CORBA_MODULE_FN objectKey octetSequenceToKey(const seqOctets& seq);//
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // MaxMessageSize()                                                   //
  //                                                                    //
  // returns the ORB-wide limit on the size of GIOP message (excluding  //
  // the header).                                                       //
  //                                                                    //
  _CORBA_MODULE_FN size_t MaxMessageSize();                         //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // void MaxMessageSize(size_t newvalue)                               //
  //                                                                    //
  // Set the ORB-wide limit on the size of GIOP message (excluding      //
  // the header).                                                       //
  //                                                                    //
  _CORBA_MODULE_FN void MaxMessageSize(size_t newvalue);            //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  enum idleConnType { idleIncoming, idleOutgoing };                     //
  // Idle connections shutdown. The ORB periodically scans all the      //
  // incoming and outgoing connections to detect if they are idle.      //
  // If no operation has passed through a connection for two            //
  // consecutive scan periods, the ORB would treat this connection as   //
  // idle and shut it down.                                             //
  //                                                                    //
  // void idleConnectionScanPeriod() sets the scan period. The argument //
  // is in number of seconds. If the argument is zero, the scan for idle//
  // connection is disabled.                                            //
  _CORBA_MODULE_FN void idleConnectionScanPeriod(idleConnType direction,//
				       CORBA::ULong sec);               //
  // Note: This function is *non-thread safe*!!! The behaviour of       //
  //       concurrent calls to this function is undefined.              //
  //                                                                    //
  // CORBA::ULong idleConnectionScanPeriod()                            //
  //   Returns the current scan period                                  //
  _CORBA_MODULE_FN CORBA::ULong idleConnectionScanPeriod(idleConnType direction);//
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a            //
  // CORBA::TRANSIENT exception may be raised. 				//
  //									//
  // By default, the operation will be retried transparently. The ORB 	//
  // will retry indefinitely if the operation continues to raise the	//
  // CORBA::TRANSIENT exception. Successive retries will be delayed	//
  // progressively by multiples of					//
  // omniORB::defaultTransientRetryDelayIncment.  The value is in number//
  // of seconds. The delay will be limited to a maximum specified by	//
  // omniORB::defaultTransientRetryDelayMaximum.			//
  //									//
  // This retry behaviour can be overridden by installing an exception	//
  // handler.  An exception handler is a function with the same 	//
  // signature as omniORB::transientExceptionHandler_t. The handler 	//
  // will be called when a CORBA::TRANSIENT exception is caught by the 	//
  // ORB. The handler is passed with three arguments: a <cookie>, the 	//
  // no. of retries <n_retries> and the value of the exception caught 	//
  // <ex>. The handler is expected to do whatever is appropriate and 	//
  // returns a boolean value. If the return value is TRUE (1), the ORB 	//
  // would retry the operation again. If the return value is FALSE (0),	//
  // the CORBA::TRANSIENT exception would be re-throw and is expected to//
  // be caught by the application code.					//
  //									//
  // The overloaded functions omniORB::installTransientExceptionHandler //
  // can be used to install the exception handlers for CORBA::TRANSIENT.//
  // Two overloaded forms are available. The first form install an 	//
  // exception handler for all object references except for those which //
  // have an exception handler installed by the second form, which takes//
  // an addition argument <obj> to identify the target object reference.//
  // The argument <cookie> is an opaque pointer which will be passed 	//
  // on by the ORB when it calls the exception handler.			//
  //									//
  typedef CORBA::Boolean (*transientExceptionHandler_t)(void* cookie,	//
			 		      CORBA::ULong n_retries, 	//
                                              const CORBA::TRANSIENT& ex);
  //								       	//
  _CORBA_MODULE_FN void installTransientExceptionHandler(void* cookie, //
					       transientExceptionHandler_t fn);
  //									//
  _CORBA_MODULE_FN void installTransientExceptionHandler(CORBA::Object_ptr obj,//
					       void* cookie,            //
        				       transientExceptionHandler_t fn);
  //									//
  _CORBA_MODULE_VAR CORBA::ULong defaultTransientRetryDelayIncrement;//
  _CORBA_MODULE_VAR CORBA::ULong defaultTransientRetryDelayMaximum;//
  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a		//
  // CORBA::COMM_FAILURE exception may be raised. 			//
  //									//
  // By default, the ORB will pass this exception on to the application.//
  //									//
  // This behaviour can be overridden by installing an exception	//
  // handler. The function signature of the handler should be the same 	//
  // as omniORB::commFailureExceptionHandler_t. The handlers can be 	//
  // installed using the overloaded functions				//
  // omniORB::installCommFailureExceptionHandler. The use of these	//
  // functions is similar to those for the TRANSIENT exception. See 	//
  // above for details.							//
  //									//
  typedef CORBA::Boolean (*commFailureExceptionHandler_t)(void* cookie,	//
						CORBA::ULong n_retries, //
						const CORBA::COMM_FAILURE& ex);
  //									//
  _CORBA_MODULE_FN void installCommFailureExceptionHandler(void* cookie,//
						 commFailureExceptionHandler_t fn);
  //									//
  _CORBA_MODULE_FN void installCommFailureExceptionHandler(CORBA::Object_ptr obj,//
						 void* cookie,		//
						 commFailureExceptionHandler_t fn);
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a system	//
  // exception may be raised. If the exception is either CORBA::TRANSIENT//
  // and CORBA::COMM_FAILURE, the handling of this exception is described//
  // above.								//
  //									//
  // By default, if the exception is neither CORBA::TRANSIENT and	//
  // CORBA::COMM_FAILURE, the ORB will pass this exception on to the	//
  // application. 							//
  //									//
  // This behaviour can be overridden by installing an exception handler//
  // The function signature of the handler should be the same as	//
  // omniORB::systemExceptionHandler_t. The handlers can be installed   //
  // using the overloaded functions					//
  // omniORB::installSystemExceptionHandler.                            //
  // The use of these functions is similar to those for the TRANSIENT	//
  // exception. See above for details. Notice that the installed        //
  // exception handler will only be called when the system exception is //
  // neither CORBA::TRANSIENT nor CORBA::COMM_FAILURE.			//
  //									//
  typedef CORBA::Boolean (*systemExceptionHandler_t)(void* cookie,	//
					   CORBA::ULong n_retries, 	//
					   const CORBA::SystemException& ex);
  //									//
  _CORBA_MODULE_FN void installSystemExceptionHandler(void* cookie,	//
					    systemExceptionHandler_t fn);
  //									//
  _CORBA_MODULE_FN void installSystemExceptionHandler(CORBA::Object_ptr obj,//
					    void* cookie,		//
					    systemExceptionHandler_t fn);
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // An application can register a handler for loading objects          //
  // dynamically. The handler should have the signature:                //
  //                                                                    //
  //          omniORB::loader::mapKeyToObject_t                         //
  //                                                                    //
  // When the ORB cannot locate the target object in this address space,//
  // it calls the handler with the object key of the target.            //
  // The handler is expected to instantiate the object, either in       //
  // this address space or in another address space, and returns the    //
  // object reference to the newly instantiated object. The ORB will    //
  // then reply with a LOCATION_FORWARD message to instruct the client  //
  // to retry using the object reference returned by the handler.       //
  // When the handler returns, the ORB assumes ownership of the         //
  // returned value. It will call CORBA::release() on the returned      //
  // value when it has finished with it.                                //
  //                                                                    //
  // The handler may be called concurrently by multi-threads. Hence it  //
  // must be thread-safe.                                               //
  //                                                                    //
  // If the handler cannot load the target object, it should return     //
  // CORBA::Object::_nil(). The object will be treated as non-existing. //
  //                                                                    //
  // The application registers the handler with the ORB at runtime      //
  // using omniORB::loader::set(). This function is not thread-safe.    //
  // Calling this function again will replace the old handler with      //
  // the new one.                                                       //
  //                                                                    //
  class loader {                                                        //
  public:                                                               //
    typedef CORBA::Object_ptr (*mapKeyToObject_t) (                     //
                                       const objectKey& key);           //
                                                                        //
    static void set(mapKeyToObject_t NewKeyToObject);                   //
  };
  ////////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // class giopServerThreadWrapper                                      //
  //                                                                    //
  // At any time, a single instance of this class (a singleton) is      //
  // registered with the runtime.                                       //
  //                                                                    //
  // What is the function of this class?                                //
  //   The runtime uses a number of threads internally to process       //
  //   requests from other address spaces. Each thread starts by        //
  //   calling the run() method of the singleton. The thread            //
  //   will exit when run() returns. The run() method takes two         //
  //   arguments: a callback function <fn> and its argument <arg>.      //
  //   The run() method *MUST* call fn(arg) to pass the control back    //
  //   the runtime at some point. When fn() returns, the run() method   //
  //   should cleanup and returns asap.                                 //
  //                                                                    //
  //   Application can modify the behaviour of run() by installing      //
  //   another singleton using setGiopServerThreadWrapper(). The        //
  //   singleton should be an instance of a derived class of            //
  //   giopServerThreadWrapper. The derived class should overload the   //
  //   virtual function run() to customise its behaviour.               //
  //                                                                    //
  //   For example, to insert the fault handler code for ObjectStore    //
  //   a derived class ObjectStoreThreadWrapper is defined as follows:  //
  //									//
  //   class ObjectStoreThreadWrapper : omniORB::giopServerThreadWrapper//
  //   {								//
  //     public:							//
  //       void run(void (*fn)(void*),void* arg) {			//
  //         /* Setup the context to clean up the state attached by	//
  //            ObjectStore to this thread */				//
  //         OS_PSE_ESTABLISH_FAULT_HANDLER				//
  //         fn(arg);							//
  //         OS_PSE_END_FAILUT_HANDLER					//
  //       }								//
  //  }									//
  //									//
  //  And in the main()							//
  // 									//
  //  omniORB::setgiopServerThreadWrapper(new ObjectStoreThreadWrapper);//
  //									//
  class giopServerThreadWrapper {                                       //
  public:								//
    virtual void run(void (*fn)(void*), void* arg) { fn(arg); }		//
    virtual ~giopServerThreadWrapper() {}                               //
									//
  // Install a new singleton. The old singleton will be deleted by the  //
  // runtime. This function is not thread-safe and *SHOULD NOT* be used //
  // when the BOA::impl_is_ready() has been called.                     //
  // If the argument <p> is nil, the call will be siliently ignored.    //
  // 
    static void setGiopServerThreadWrapper(giopServerThreadWrapper* p);	//
    static giopServerThreadWrapper* getGiopServerThreadWrapper();       //
  };									//
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // class fatalException                                               //
  //                                                                    //
  // This exception is thrown if a bug inside the omniORB2 runtime is   //
  // detected. The exact location in the source where the exception is  //
  // thrown is indicated by file() and line().                          //
  //                                                                    //
  class fatalException {                                                //
  public:                                                               //
    fatalException(const char *file,int line,const char *errmsg) {      //
      pd_file = file;                                                   //
      pd_line = line;                                                   //
      pd_errmsg = errmsg;                                               //
    }                                                                   //
    ~fatalException() {}                                                //
    const char *file() const { return pd_file; }                        //
    int line() const { return pd_line; }                                //
    const char *errmsg() const { return pd_errmsg; }                    //
  private:                                                              //
    const char *pd_file;                                                //
    int         pd_line;                                                //
    const char *pd_errmsg;                                              //
                                                                        //
    fatalException();                                                   //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////


  class logStream {
  public:
    logStream();
    ~logStream();
    logStream& operator<<(char c);
    logStream& operator<<(unsigned char c) { return (*this) << (char)c; }
    logStream& operator<<(signed char c) { return (*this) << (char)c; }
    logStream& operator<<(const char *s);
    logStream& operator<<(const unsigned char *s) { 
      return (*this) << (const char*)s; 
    }
    logStream& operator<<(const signed char *s) { 
      return (*this) << (const char*)s; 
    }
    logStream& operator<<(const void *p);
    logStream& operator<<(int n);
    logStream& operator<<(unsigned int n);
    logStream& operator<<(long n);
    logStream& operator<<(unsigned long n);
    logStream& operator<<(short n) {return operator<<((int)n);}
    logStream& operator<<(unsigned short n) {return operator<<((unsigned int)n);}
#ifdef HAS_Cplusplus_Bool
    logStream& operator<<(bool b) { return operator<<((int)b); }
#endif
#ifndef NO_FLOAT
    logStream& operator<<(double n);
    logStream& operator<<(float n) { return operator<<((double)n); }
#endif
    logStream& flush();
  private:
    void* pd_state;
  };

  _CORBA_MODULE_VAR logStream& log;

#ifndef HAS_Cplusplus_Namespace
  friend class omni;
  friend class CORBA;
private:
#endif
  _CORBA_MODULE_VAR objectKey seed;

_CORBA_MODULE_END

#undef _LC_attr

#endif // __OMNIORB_H__
