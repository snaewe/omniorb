// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbParameters.h            Created on: 15/8/2001
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
  Revision 1.1.2.7  2005/08/23 11:45:06  dgrisby
  New maxSocketSend and maxSocketRecv parameters.

  Revision 1.1.2.6  2005/02/13 20:53:08  dgrisby
  Change threadPoolWatchConnection parameter to be an integer rather
  than a boolean. Value is the count of threads that can be handling a
  connection when one decides to watch it.

  Revision 1.1.2.5  2004/03/02 15:31:22  dgrisby
  Support for persistent server identifier.

  Revision 1.1.2.4  2002/10/14 20:06:41  dgrisby
  Per objref / per thread timeouts.

  Revision 1.1.2.3  2002/03/18 16:50:17  dpg1
  New threadPoolWatchConnection parameter.

  Revision 1.1.2.2  2001/08/21 11:02:12  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.1.2.1  2001/08/17 17:12:34  sll
  Modularise ORB configuration parameters.

*/

#ifndef __ORBPARAMETERS_H__
#define __ORBPARAMETERS_H__

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

OMNI_NAMESPACE_BEGIN(omni)

_CORBA_MODULE orbParameters

_CORBA_MODULE_BEG

struct timeValue {
  unsigned long secs;
  unsigned long nanosecs;
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                    ORB wide Parameters                                //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


_CORBA_MODULE_VAR _core_attr CORBA::Boolean dumpConfiguration;
//  Set to 1 to cause the ORB to dump the current set of configuration
//  parameters.
//
//  Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr GIOP::Version maxGIOPVersion;
//  Set the maximum GIOP version the ORB should support. The ORB tries
//  to match the <major>.<minor> version as specified. This function
//  should only be called before ORB_init(). Calling this function
//  after ORB_init() does not cause the ORB to change its maximum
//  supported version; in this case the ORB just returns its version
//  number in <major>.<minor>.
//
//  Valid values = 1.0 | 1.1 | 1.2

_CORBA_MODULE_VAR _core_attr CORBA::ULong giopMaxMsgSize;
//   This value defines the ORB-wide limit on the size of GIOP message 
//   (excluding the header). If this limit is exceeded, the ORB will
//   refuse to send or receive the message and raise a MARSHAL exception.
//
//   Valid values = (n >= 8192)
//

_CORBA_MODULE_VAR _core_attr size_t maxSocketSend;
_CORBA_MODULE_VAR _core_attr size_t maxSocketRecv;
//   These values set the maximum size that is used in an individual
//   socket send() or recv() call.
//
//   Valid values = (n >= 8192)


_CORBA_MODULE_VAR _core_attr omniCodeSet::NCS_C* nativeCharCodeSet;
//  set the native code set for char and string
//

_CORBA_MODULE_VAR _core_attr omniCodeSet::NCS_W* nativeWCharCodeSet;
//  set the native code set for wchar and wstring
//

_CORBA_MODULE_VAR _core_attr omniCodeSet::TCS_C* anyCharCodeSet;
//  set the preferred code set for char data inside anys
//

_CORBA_MODULE_VAR _core_attr omniCodeSet::TCS_W* anyWCharCodeSet;
//  set the preferred code set for wchar data inside anys
//

_CORBA_MODULE_VAR _core_attr CORBA::Boolean      lcdMode;
//  Set to 1 to enable 'Lowest Common Denominator' Mode.
//  This will disable various features of IIOP and GIOP which are
//  poorly supported by some ORBs, and disable warnings/errors when
//  certain types of erroneous message are received on the wire.
//
//  Valid values = 0 or 1
//


_CORBA_MODULE_VAR _core_attr CORBA::Boolean supportCurrent;
//  If the value of this variable is TRUE, per-thread information is
//  made available through the Current interfaces, e.g.
//  PortableServer::Current. If you do not need this information, you
//  can set the value to 0, resulting in a small performance
//  improvement.

_CORBA_MODULE_VAR _core_attr CORBA::Boolean strictIIOP;
//   Enable vigorous check on incoming IIOP messages
//
//   In some (sloppy) IIOP implementations, the message size value in
//   the header can be larger than the actual body size, i.e. there is
//   garbage at the end. As the spec does not say the message size
//   must match the body size exactly, this is not a clear violation
//   of the spec.
//
//   If this flag is non-zero, the incoming message is expected to
//   be well-behaved. Any messages that have garbage at the end will
//   be rejected.
//   
//   The default value of this flag is true, so invalid messages are
//   rejected. If you set it to zero, the ORB will silently skip the
//   unread part. The problem with this behaviour is that the header
//   message size may actually be garbage, caused by a bug in the
//   sender's code. The receiving thread may block forever on the
//   strand as it tries to read more data from it. In this case the
//   sender won't send any more as it thinks it has marshalled in all
//   the data.
//
//   Valid values = 0 or 1
//


_CORBA_MODULE_VAR _core_attr CORBA::ULong scanGranularity;
//  The granularity at which the ORB scans for idle connections.
//  This value determines the minimum value that inConScanPeriod or
//  outConScanPeriod can be implemented.
//
//  Valid values = (n >= 0 in seconds) 
//                  0 --> do not scan for idle connections.
//


_CORBA_MODULE_VAR _core_attr CORBA::ULong objectTableSize;
//  Hash table size of the Active Object Map. If this is zero, the ORB
//  uses a dynamically resized open hash table. This is normally the  
//  best option, but it leads to less predictable performance since   
//  any operation which adds or removes a table entry may trigger a   
//  resize. If you set this to a non-zero value, the hash table has   
//  the specified number of entries, and is never resized. Note that  
//  the hash table is open, so this does not limit the number of      
//  active objects, just how efficiently they can be located.
//
//  Valid values = (n >= 0)
//                 0 --> use a dynamically resized table.


_CORBA_MODULE_VAR _core_attr CORBA::Boolean abortOnInternalError;
//  If the value of this variable is TRUE then the ORB will abort
//  instead of throwing an exception when a fatal internal error is
//  detected. This is useful for debuging the ORB -- as the stack will
//  not be unwound by the exception handler, so a stack trace can be
//  obtained.
//  It is hoped that this will not often be needed by users of omniORB!
//
//  Valid values = 0 or 1


_CORBA_MODULE_VAR _core_attr CORBA::Boolean tcAliasExpand;
//   This flag is used to indicate whether TypeCodes associated with Anys
//   should have aliases removed. This functionality is included because
//   some ORBs will not recognise an Any containing a TypeCode with
//   aliases to be the same as the actual type contained in the Any. Note
//   that omniORB will always remove top-level aliases, but will not remove
//   aliases from TypeCodes that are members of other TypeCodes (e.g.
//   TypeCodes for members of structs etc.), unless tcAliasExpand is set to 1.
//   There is a performance penalty when inserting into an Any if 
//   tcAliasExpand is set to 1. The default value is 0 (i.e. aliases of
//   member TypeCodes are not expanded). Note that aliases won't be expanded
//   when one of the non-type-safe methods of inserting into an Any is
//   used (i.e. when the replace() member function or non - type-safe Any
//   constructor is used. )
//
//    Valid values = 0 or 1


_CORBA_MODULE_VAR _core_attr CORBA::Boolean useTypeCodeIndirections;
//   If true (the default), typecode indirections will be used. Set
//   this to false to disable that. Setting this to false might be
//   useful to interoperate with another ORB implementation that cannot
//   handle indirections properly.
//  
//   Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::Boolean  acceptMisalignedTcIndirections;
//   If true, try to fix a mis-aligned indirection in a typecode. This
//   could be used to work around some versions of Visibroker's Java ORB.
//  
//   Valid values = 0 or 1


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                    Client Side Parameters                             //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

_CORBA_MODULE_VAR _core_attr CORBA::Boolean verifyObjectExistsAndType;
//  If the value of this variable is 0 then the ORB will not
//  send a GIOP LOCATE_REQUEST message to verify the existence of
//  the object prior to the first invocation. Set this variable
//  if the other end is a buggy ORB that cannot handle GIOP
//  LOCATE_REQUEST. 
//
//  Valid values = 0 or 1


_CORBA_MODULE_VAR _core_attr CORBA::Boolean oneCallPerConnection;
//  1 means only one call can be in progress at any time per connection.
//
//  Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::ULong maxGIOPConnectionPerServer;
//  The ORB could open more than one connection to a server
//  depending on the number of concurrent invocations to the same
//  server. This variable decides the maximum number of connections 
//  to use per server. This variable is read only once at ORB_init.
//  If the number of concurrent invocations exceeds this number, the
//  extra invocations are blocked until the the outstanding ones
//  return.
//
//  Valid values = (n >= 1) 


_CORBA_MODULE_VAR _core_attr GIOP::AddressingDisposition giopTargetAddressMode;
//  On the client side, if it is to use GIOP 1.2 or above to talk to a 
//  server, use this Target Address Mode.
//
//  Valid values = 0 (GIOP::KeyAddr)
//                 1 (GIOP::ProfileAddr)
//                 2 (GIOP::ReferenceAddr)


_CORBA_MODULE_VAR _core_attr CORBA::Boolean offerBiDirectionalGIOP;
//   Applies to the client side. Set to 1 to indicates that the
//   ORB may choose to use a connection to do bidirectional GIOP
//   calls. Set to 0 means the ORB should never do bidirectional.
//
//   Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::Boolean  diiThrowsSysExceptions;
// If the value of this variable is 1 then the Dynamic Invacation Interface
// functions (Request::invoke, send_oneway, send_deferred, get_response,
// poll_response) will throw system exceptions as appropriate. Otherwise 
// the exception will be stored in the Environment pseudo object associated
// with the Request. By default system exceptions are passed through the 
// Environment object.
//
// Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::ULong outConScanPeriod;
//  Idle connections shutdown. The ORB periodically scans all the
//  incoming connections to detect if they are idle.
//  If no operation has passed through a connection for a scan period,
//  the ORB would treat this connection idle and shut it down.
//
//  Valid values = (n >= 0 in seconds) 
//                  0 --> do not close idle connections.

_CORBA_MODULE_VAR _core_attr timeValue clientCallTimeOutPeriod;
//   Call timeout. On the client side, if a remote call takes longer
//   than the timeout value, the ORB will shutdown the connection and
//   raise a COMM_FAILURE.
//
//   Valid values = (n >= 0 in seconds) 
//                   0 --> no timeout. Block till a reply comes back

_CORBA_MODULE_VAR _core_attr CORBA::Boolean supportPerThreadTimeOut;
//   If true, each thread may have a timeout associated with it. This
//   gives a performance hit due to accessing per-thread data.
//
//   Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::String_var bootstrapAgentHostname;
// Applies to the client side. Non-zero enables the use of Sun's bootstrap
// agent protocol to resolve initial references. The value is the host name
// where requests for initial references should be sent. Only uses this
// option to interoperate with Sun's javaIDL.

_CORBA_MODULE_VAR _core_attr CORBA::UShort bootstrapAgentPort;
// Applies to the client side. Use this port no. to contact the bootstrap 
// agent.


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                    Server Side Parameters                             //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


_CORBA_MODULE_VAR _core_attr CORBA::Boolean threadPerConnectionPolicy;
//   1 means the ORB should dedicate one thread per connection on the 
//   server side. 0 means the ORB should dispatch a thread from a pool
//   to a connection only when a request has arrived.
//
//  Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::ULong   threadPerConnectionUpperLimit;
//   If the one thread per connection is in effect, this number is
//   the max. no. of connections the server will allow before it
//   switch off the one thread per connection policy and move to
//   the thread pool policy.
//
//   Valid values = (n >= 1) 

_CORBA_MODULE_VAR _core_attr CORBA::ULong   threadPerConnectionLowerLimit;
//   If the one thread per connection was in effect and was switched
//   off because threadPerConnectionUpperLimit has been exceeded
//   previously, this number tells when the policy should be restored
//   when the number of connections drop.
//
//   Valid values = (n >= 1 && n < threadPerConnectionUpperLimit) 

_CORBA_MODULE_VAR _core_attr CORBA::ULong   maxServerThreadPerConnection;
//   The max. no. of threads the server will dispatch to server the
//   requests coming from one connection.
//
//   Valid values = (n >= 1) 

_CORBA_MODULE_VAR _core_attr CORBA::ULong   maxServerThreadPoolSize;
//   The max. no. of threads the server will allocate to do various
//   ORB tasks. This number does not include the dedicated thread
//   per connection when the threadPerConnectionPolicy is in effect
//
//   Valid values = (n >= 1) 

_CORBA_MODULE_VAR _core_attr CORBA::ULong   threadPoolWatchConnection;
//   After dispatching an upcall in thread pool mode, the thread that
//   has just performed the call can watch the connection for a short
//   time before returning to the pool. This leads to less thread
//   switching for a series of calls from a single client, but is less
//   fair if there are concurrent clients. The connection is watched
//   if the number of threads concurrently handling the connection is
//   <= the value of this parameter. i.e. if the parameter is zero,
//   the connection is never watched; if it is 1, the last thread
//   managing a connection watches it; if 2, the connection is still
//   watched if there is one other thread still in an upcall for the
//   connection, and so on.
//
//  Valid values = (n >= 0)

_CORBA_MODULE_VAR _core_attr CORBA::Boolean acceptBiDirectionalGIOP;
//   Applies to the server side. Set to 1 to indicates that the
//   ORB may choose to accept a clients offer to use bidirectional
//   GIOP calls on a connection. Set to 0 means the ORB should
//   never accept any bidirectional offer and should stick to normal
//   GIOP.
//
//   Valid values = 0 or 1

_CORBA_MODULE_VAR _core_attr CORBA::ULong   maxInterleavedCallsPerConnection;
//  No. of interleaved calls per connection the server is prepared
//  to accept. If this number is exceeded, the connection is closed.
//
//  Valid values = (n >= 1) 

_CORBA_MODULE_VAR _core_attr CORBA::ULong  inConScanPeriod;
//  Idle connections shutdown. The ORB periodically scans all the
//  outgoing connections to detect if they are idle.
//  If no operation has passed through a connection for a scan period,
//  the ORB would treat this connection idle and shut it down.
//
//   Valid values = (n >= 0 in seconds) 
//                   0 --> do not close idle connections.


_CORBA_MODULE_VAR _core_attr timeValue serverCallTimeOutPeriod;
//   Call timeout. On the server side, if the ORB cannot completely 
//   unmarshal a call's arguments in the defined timeout, it shutdown the
//   connection.
//
//   Valid values = (n >= 0 in milliseconds) 
//                   0 --> no timeout.

_CORBA_MODULE_VAR _core_attr CORBA::ULong poaHoldRequestTimeout;
//  This variable can be used to set a time-out for calls being held
//  in a POA which is in the HOLDING state.  It gives the time in
//  seconds after which a TRANSIENT exception will be thrown if the
//  POA is not transitioned to a different state.
//
//  Valid values = (n >= 0 in milliseconds) 
//                  0 --> no time-out.

_CORBA_MODULE_VAR _core_attr CORBA::String_var unixTransportDirectory;
//  Applies to the server side. Determine the directory in which
//  the unix domain socket is to be created.
//
//  Valid values = a valid pathname for a directory


_CORBA_MODULE_VAR _core_attr CORBA::UShort unixTransportPermission;
//  Applies to the server side. Determine the permission mode bits
//  the unix domain socket is set to.
//
//  Valid values = unix permission mode bits in octal radix (e.g. 0755)

_CORBA_MODULE_VAR _core_attr CORBA::Boolean supportBootstrapAgent;
//  Applies to the server side. 1 means enable the support for Sun's
//  bootstrap agent protocol.  This enables interoperability between omniORB
//  servers and Sun's javaIDL clients. When this option is enabled, an
//  omniORB server will response to a bootstrap agent request.

_CORBA_MODULE_VAR _core_attr _CORBA_Unbounded_Sequence_Octet persistentId;
//  Persistent identifier used to detect object references that should
//  be considered to be in this process.

_CORBA_MODULE_END

OMNI_NAMESPACE_END(omni)

#undef _core_attr

#endif // __ORBPARAMETERS_H__
