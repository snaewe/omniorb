// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORB.h                  Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//      This is the public API of omniORB's extension to CORBA.
//      The API is intended to be used in application code.

/*
  $Log$
  Revision 1.5.2.1  2003/03/23 21:04:13  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.19  2002/10/14 20:06:03  dgrisby
  Per objref / per thread timeouts.

  Revision 1.2.2.18  2002/08/16 17:47:39  dgrisby
  Documentation, message updates. ORB tweaks to match docs.

  Revision 1.2.2.17  2002/01/09 11:39:22  dpg1
  New omniORB::setLogFunction() function.

  Revision 1.2.2.16  2001/09/19 17:30:04  dpg1
  New traceThreadId option to add omni_thread id to log messages.

  Revision 1.2.2.15  2001/09/12 19:42:35  sll
  Added back function to report max. GIOP message size.

  Revision 1.2.2.14  2001/08/17 17:03:23  sll
  Moved configuration parameters to orbParameters. Move old compatibility API
  to omniORBcompat.h.

  Revision 1.2.2.13  2001/08/16 16:11:48  sll
  Removed duplicate offerBidirectionalGIOP.

  Revision 1.2.2.12  2001/08/08 15:57:11  sll
  New options unixTransportDirectory & unixTransportPermission.

  Revision 1.2.2.11  2001/08/03 17:45:08  sll
  Moved OMNIORB_THROW so that it can be used in stub headers

  Revision 1.2.2.10  2001/08/01 10:08:20  dpg1
  Main thread policy.

  Revision 1.2.2.9  2001/07/31 16:10:38  sll
  Added GIOP BiDir support.

  Revision 1.2.2.8  2001/07/13 15:16:56  sll
  New configuration variables: oneCallPerConnection, threadPerConnectionPolicy,
  maxServerThreadPerConnection, maxInterleavedCallsPerConnection,
  maxServerThreadPoolSize, threadPerConnectionUpperLimit,
  threadPerConnectionLowerLimit. Removed maxNumOfAsyncThreads.

  Revision 1.2.2.7  2001/06/07 16:24:08  dpg1
  PortableServer::Current support.

  Revision 1.2.2.6  2001/05/11 14:25:54  sll
  Added operator for omniORB::logger to report system exception status and
  minor code.

  Revision 1.2.2.5  2001/04/18 17:50:44  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.2.2.4  2000/11/20 11:59:43  dpg1
  API to configure code sets.

  Revision 1.2.2.3  2000/11/03 18:58:47  sll
  Unbounded sequence of octet got a new type name.

  Revision 1.2.2.2  2000/09/27 17:09:16  sll
  New member maxGIOPVersion(), getInterceptors(), noFirewallNavigation,
  giopTargetAddressMode.
  LOCATION_FORWARD ctor takes an extra argument to indicate permanent
  location forwarding.

  Revision 1.2.2.1  2000/07/17 10:35:35  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:05  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.8  2000/05/24 17:11:18  dpg1
  Comments clarified.

  Revision 1.1.2.7  2000/03/07 18:07:32  djr
  Fixed user-exceptions when can't catch by base class.

  Revision 1.1.2.6  2000/03/03 14:29:17  djr
  Improvement to BOA skeletons (less generated code).

  Revision 1.1.2.5  1999/09/30 11:49:27  djr
  Implemented catching user-exceptions in GIOP_S for all compilers.

  Revision 1.1.2.4  1999/09/27 11:01:09  djr
  Modifications to logging.

  Revision 1.1.2.3  1999/09/24 17:11:10  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.1.2.2  1999/09/24 15:01:28  djr
  Added module initialisers, and sll's new scavenger implementation.

  Revision 1.1.2.1  1999/09/24 09:51:46  djr
  Moved from omniORB2 + some new files.

  Revision 1.21  1999/08/30 16:56:19  sll
  New API members: omniORB::scanGranularity and omniORB::callTimeOutPeriod.

  Revision 1.20  1999/06/26 17:56:39  sll
  New configuration variables: abortOnInternalError, verifyObjectExistAndType.

  Revision 1.19  1999/06/25 13:53:51  sll
  Renamed copyStringInAnyExtraction to omniORB_27_CompatibleAnyExtraction.

  Revision 1.18  1999/06/18 21:16:36  sll
  Updated copyright notice.

  Revision 1.17  1999/06/18 20:37:04  sll
  Replaced _LC_attr with _core_attr and _dyn_attr.
  New variable copyStringInAnyExtraction.

  Revision 1.16  1999/03/19 15:17:44  djr
  New option acceptMisalignedIndirections

  Revision 1.15  1999/02/18 15:19:54  djr
  New configuration function omniORB::enableLcdMode()

  Revision 1.14  1999/01/07 18:38:34  djr
  New configuration variable omniORB::diiThrowsSysExceptions.

  Revision 1.13  1998/08/26 21:50:22  sll
  Added omniORB::maxTcpConnectionPerServer to customise the maximum no. of
  outgoing connections per server.

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


struct omniOrbBoaKey {
  _CORBA_ULong hi;
  _CORBA_ULong med;
  _CORBA_ULong lo;
};

OMNI_NAMESPACE_BEGIN(omni)
class omniInterceptors;
OMNI_NAMESPACE_END(omni)

_CORBA_MODULE omniORB

_CORBA_MODULE_BEG


  ///////////////////////////////////////////////////////////////////////
  // Tracing level                                                     //
  //     level 0 - critical errors only                                //
  //     level 1 - informational messages only                         //
  //     level 2 - configuration information and warnings              //
  //     level 5 - the above plus report server thread creation and    //
  //               communication socket shutdown                       //
  //     level 10 - the above plus execution trace messages            //
  //     level 25 - output trace message per send or receive giop message
  //     level 30 - dump up to 128 bytes of a giop message             //
  //     level 40 - dump the complete giop message                     //
  //                                                                   //
  //    Valid values = (n >= 0)                                        //
  _CORBA_MODULE_VAR _core_attr _CORBA_ULong   traceLevel;              //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceLevel <n>, or by direct assignment to this   //
  //     variable.                                                     //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceInvocations;        //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceInvocations, or by direct assignment to this //
  //     variable.  If true, then each local and remote invocation     //
  //     will generate a trace message.                                //
  //                                                                   //
  _CORBA_MODULE_VAR _core_attr _CORBA_Boolean traceThreadId;           //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceThreadId, or by direct assignment to this    //
  //     variable.  If true, then the logging messages emmited due to  //
  //     the two options above will contain the thread id of the       //
  //     logging thread.                                               //
  ///////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // getInterceptors()                                                  //
  //   Only call this function after ORB_init().                        //
  //   The returned object contains all the ORB processing points where //
  //   interception functions can be added.                             //
  //   Calling this function before ORB_init() will result in a system  //
  //   exception.                                                       //
  _CORBA_MODULE_FN _OMNI_NS(omniInterceptors)* getInterceptors();       //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setMainThread()                                                    //
  //                                                                    //
  // POAs with the MAIN_THREAD policy dispatch calls on the "main"      //
  // thread. By default, omniORB assumes that the thread which          //
  // initialised the omnithread library is the "main" thread. To choose //
  // a different thread, call this function from the desired "main"     //
  // thread. The calling thread must have an omni_thread associated     //
  // with it. If it does not, throws CORBA::INITIALIZE.                 //
  //                                                                    //
  // Note that calls are only actually dispatched to the "main" thread  //
  // if ORB::run() or ORB::perform_work() is called from that thread.   //
  //                                                                    //
  _CORBA_MODULE_FN void setMainThread();                                //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setClientCallTimeout()                                             //
  // setClientThreadCallTimeout()                                       //
  // setClientThreadCallDeadline()                                      //
  //                                                                    //
  // Functions to set call timeouts at runtime.                         //
  //                                                                    //
  // setClientCallTimeout() either sets the global timeout, or the      //
  // timeout for the specfied object reference. The timeout is          //
  // specified in milliseconds.                                         //
  //                                                                    //
  // setClientThreadCallTimeout() sets the timeout for the calling      //
  // thread to the specified number of milliseconds. If the calling     //
  // thread is not an omni_thread, it throws CORBA::INITIALIZE.         //
  //                                                                    //
  // setClientThreadCall Deadline() sets an absolute deadline for all   //
  // calls in the calling thread. The seconds and nanoseconds should be //
  // acquired from omni_thread::get_time(). Again throws INITIALIZE if  //
  // the calling thread is not an omni_thread.                          //
  //                                                                    //
  _CORBA_MODULE_FN void setClientCallTimeout(CORBA::ULong millisecs);   //
  _CORBA_MODULE_FN void setClientCallTimeout(CORBA::Object_ptr obj,     //
    					     CORBA::ULong millisecs);   //
                                                                        //
  _CORBA_MODULE_FN void setClientThreadCallTimeout(CORBA::ULong millisecs);
                                                                        //
  _CORBA_MODULE_FN void setClientThreadCallDeadline(unsigned long secs, //
                                                    unsigned long ns);  //
                                                                        //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // giopMaxMsgSize()                                                   //
  //                                                                    //
  // Return the ORB-wide limit on the size of GIOP message (excluding   //
  // the header). This value is set by the ORB option giopMaxMsgSize.   //
  //                                                                    //
  _CORBA_MODULE_FN _CORBA_ULong giopMaxMsgSize();                       //
  ////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////
  /////////////////////////// omniORB::logger //////////////////////////
  //////////////////////////////////////////////////////////////////////

  static inline int trace(_CORBA_ULong tl) { return traceLevel >= tl; }

  typedef void (*logFunction)(const char*);

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // setLogFunction()                                                   //
  //                                                                    //
  // Set a logging function. If set to a non-zero function pointer, the //
  // provided function is called for all omniORB log messages.          //
  //                                                                    //
  _CORBA_MODULE_FN void setLogFunction(logFunction f);                  //
  ////////////////////////////////////////////////////////////////////////


  class logger {
  public:
    logger(const char* prefix = 0);  // prefix defaults to "omniORB: "
    ~logger();
    // The destructor flushes the message.

    logger& operator<<(char c);
    logger& operator<<(unsigned char c) { return (*this) << (char)c; }
    logger& operator<<(signed char c) { return (*this) << (char)c; }
    logger& operator<<(const char *s);
    logger& operator<<(const unsigned char *s) {
      return (*this) << (const char*)s;
    }
    logger& operator<<(const signed char *s) {
      return (*this) << (const char*)s;
    }
    logger& operator<<(const void *p);
    logger& operator<<(int n);
    logger& operator<<(unsigned int n);
    logger& operator<<(long n);
    logger& operator<<(unsigned long n);
    logger& operator<<(short n) {return operator<<((int)n);}
    logger& operator<<(unsigned short n) {return operator<<((unsigned int)n);}
#ifdef HAS_Cplusplus_Bool
    logger& operator<<(bool b) { return operator<<((int)b); }
#endif
#ifndef NO_FLOAT
    logger& operator<<(double n);
    logger& operator<<(float n) { return operator<<((double)n); }
#endif
    logger& operator<<(const omniLocalIdentity*);
    logger& operator<<(const omniIdentity*);
    logger& operator<<(omniObjKey&);

    logger& operator<<(const CORBA::SystemException&);

    class exceptionStatus {
    public:
      exceptionStatus(CORBA::CompletionStatus s, CORBA::ULong m) :
	status(s), minor(m), minor_string(0) {}

      exceptionStatus(CORBA::CompletionStatus s, const char* description) :
	status(s), minor(0), minor_string(description) {}

      CORBA::CompletionStatus status;
      CORBA::ULong            minor;
      const char*             minor_string;
    private:
      exceptionStatus();
    };

    logger& operator<<(const exceptionStatus&);

    void flush();
    // Flushes the logger -- it can then be re-used for another
    // message.

  private:
    logger(const logger&);
    logger& operator=(const logger&);

    inline void reserve(int n) { if( pd_end - pd_p - 1 < n )  more(n); }
    void more(int n);

    const char* pd_prefix;
    char*       pd_buf;
    char*       pd_p;      // assert(*pd_p == '\0')
    char*       pd_end;    // assert(pd_p < pd_end)
  };


  _CORBA_MODULE_FN void logf(const char* fmt ...);
  // Writes log message with prefix, and appends '\n'.

  _CORBA_MODULE_FN void do_logs(const char* msg);
  // internal

  _CORBA_MODULE_FN inline void logs(_CORBA_ULong tl, const char* msg) {
    if( traceLevel >= tl )  do_logs(msg);
  }
  // Writes log message with prefix, and appends '\n'.



  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a            //
  // CORBA::TRANSIENT exception may be raised. 				//
  //									//
  // One circumstance in which a TRANSIENT exception can arise is as    //
  // follows:                                                           //
  //                                                                    //
  //     1. The client invokes on an object reference.                  //
  //     2. The object replies with a LOCATION_FORWARD message.         //
  //     3. The client caches the new location and retries to the new   //
  //        location.                                                   //
  //     4. Time passes...                                              //
  //     5. The client tries to invoke on the object again, using the   //
  //        cached, forwarded location.                                 //
  //     6. The attempt to contact the object fails.                    //
  //     7. The ORB runtime resets the location cache and throws a      //
  //        TRANSIENT exception with minor code                         //
  //        TRANSIENT_FailedOnForwarded.                                //
  //                                                                    //
  // In this situation, the default TRANSIENT exception handler retries //
  // the request.                                                       //
  //                                                                    //
  // In all other circumstances, TRANSIENT exceptions are propagated to //
  // the application.                                                   //
  //									//
  // This retry behaviour can be overridden by installing an exception	//
  // handler.  An exception handler is a function with the same 	//
  // signature as omniORB::transientExceptionHandler_t. The handler 	//
  // will be called when a CORBA::TRANSIENT exception is caught by the 	//
  // ORB. The handler is passed three arguments: a <cookie>, the 	//
  // no. of retries <n_retries> and the value of the exception caught 	//
  // <ex>. The handler is expected to do whatever is appropriate and 	//
  // return a boolean value. If the return value is TRUE (1), the ORB 	//
  // would retry the operation again. If the return value is FALSE (0),	//
  // the CORBA::TRANSIENT exception would be re-throw and is expected   //
  // to be caught by the application code.			      	//
  //									//
  // The overloaded functions omniORB::installTransientExceptionHandler //
  // can be used to install the exception handlers for CORBA::TRANSIENT.//
  // Two overloaded forms are available. The first form installs an 	//
  // exception handler for all object references except for those which //
  // have an exception handler installed by the second form, which takes//
  // an addition argument <obj> to identify the target object reference.//
  // The argument <cookie> is an opaque pointer which will be passed 	//
  // on by the ORB when it calls the exception handler.			//
  //									//
  typedef CORBA::Boolean (*transientExceptionHandler_t)(void* cookie,	//
					CORBA::ULong n_retries, 	//
					const CORBA::TRANSIENT& ex);    //
  //								       	//
  _CORBA_MODULE_FN void installTransientExceptionHandler(void* cookie,  //
				 transientExceptionHandler_t fn);       //
  //									//
  _CORBA_MODULE_FN void installTransientExceptionHandler(               //
				 CORBA::Object_ptr obj,                 //
				 void* cookie,                          //
				 transientExceptionHandler_t fn);       //
    									//
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
					CORBA::ULong n_retries,         //
					const CORBA::COMM_FAILURE& ex); //
  //									//
  _CORBA_MODULE_FN void installCommFailureExceptionHandler(void* cookie,//
				 commFailureExceptionHandler_t fn);     //
  //									//
  _CORBA_MODULE_FN void installCommFailureExceptionHandler(             //
				   CORBA::Object_ptr obj,               //
				   void* cookie,		        //
				   commFailureExceptionHandler_t fn);   //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // When an operation is invoked via an object reference, a system	//
  // exception may be raised. If the exception is either                //
  // CORBA::TRANSIENT or CORBA::COMM_FAILURE, the handling of this      //
  // exception is as described above.			                //
  //									//
  // By default, if the exception is neither CORBA::TRANSIENT and	//
  // CORBA::COMM_FAILURE, the ORB will pass this exception on to the	//
  // application. 							//
  //									//
  // This behaviour can be overridden by installing an exception        //
  // handler.                                                           //
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
				   CORBA::ULong n_retries, 	        //
				   const CORBA::SystemException& ex);   //
  //									//
  _CORBA_MODULE_FN void installSystemExceptionHandler(void* cookie,	//
				    systemExceptionHandler_t fn);       //
  //									//
  _CORBA_MODULE_FN void installSystemExceptionHandler(                  //
				    CORBA::Object_ptr obj,              //
				    void* cookie,		        //
				    systemExceptionHandler_t fn);       //
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
    fatalException(const char *file,int line,const char *errmsg);       //
    inline ~fatalException() {}                                         //
    inline const char *file() const   { return pd_file;   }             //
    inline int line() const           { return pd_line;   }             //
    inline const char *errmsg() const { return pd_errmsg; }             //
  private:                                                              //
    const char *pd_file;                                                //
    int         pd_line;                                                //
    const char *pd_errmsg;                                              //
                                                                        //
    fatalException();                                                   //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // class LOCATION_FORWARD                                             //
  //  This may be thrown as an exception from object implementation     //
  // to pass a LOCATION_FORWARD message back to the client -- which     //
  // should retry the request on the object given by argument to the    //
  // constructor.  The reference is consumed.                           //
  // From GIOP 1.2 onwards, location forward can either be temporary    //
  // or permanent. This is indicated by the permanent flag.             //
  //  It is recommended that the facilities provided by the POA be used //
  // instead where possible.                                            //
  //                                                                    //
  class LOCATION_FORWARD {                                              //
  public:                                                               //
    inline LOCATION_FORWARD(CORBA::Object_ptr objref,                   //
			    CORBA::Boolean permanent)                   //
      : pd_objref(objref), pd_permanent(permanent) {}                   //
                                                                        //
    inline CORBA::Object_ptr get_obj() const { return pd_objref; }      //
    inline CORBA::Boolean is_permanent() const { return pd_permanent; } //
                                                                        //
    inline LOCATION_FORWARD(const LOCATION_FORWARD& l)                  //
      : pd_objref(l.pd_objref), pd_permanent(l.pd_permanent) {}         //
                                                                        //
  private:                                                              //
    CORBA::Object_ptr pd_objref;                                        //
    CORBA::Boolean    pd_permanent;                                     //
    LOCATION_FORWARD();                                                 //
    LOCATION_FORWARD& operator=(const LOCATION_FORWARD&);               //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

#define _INCLUDE_OMNIORBCOMPAT_
#include <omniORB4/omniORBcompat.h>
#undef  _INCLUDE_OMNIORBCOMPAT_

#ifndef HAS_Cplusplus_Namespace
  friend class omni;
  friend class CORBA;
private:
#endif

#ifndef HAS_Cplusplus_catch_exception_by_base
  // Internal omniORB class.  Used in the stubs to pass
  // user-defined exceptions to a lower level.

  class StubUserException {
  public:
    // This exception is thrown in the stubs to pass a
    // CORBA::UserException down.  It is needed because
    // gcc 2.7 cannot catch exceptions by base class.

    inline StubUserException(CORBA::Exception* e) : pd_e(e) {}

    // inline StubUserException(const StubUserException& e);
    // inline ~StubUserException() {}
    // The defaults will do.


    inline CORBA::Exception* ex() { return pd_e; }

  private:
    StubUserException();
    StubUserException& operator=(const StubUserException&);

    CORBA::Exception* pd_e;
  };
#endif

_CORBA_MODULE_END

#ifndef OMNIORB_NO_EXCEPTION_LOGGING

OMNI_NAMESPACE_BEGIN(omni)

class omniExHelper {
public:

#define OMNIORB_EX(name) \
  static void name(const char*, int, CORBA::ULong, CORBA::CompletionStatus);

  OMNIORB_FOR_EACH_SYS_EXCEPTION(OMNIORB_EX)

#undef OMNIORB_EX

  static const char* strip(const char*);
};

OMNI_NAMESPACE_END(omni)

#define OMNIORB_THROW(name, minor, completion) \
  _OMNI_NS(omniExHelper)::name(__FILE__, __LINE__, minor, completion)

#else


#define OMNIORB_THROW(name, minor, completion) \
  throw CORBA::name(minor, completion)

#endif


#endif // __OMNIORB_H__
