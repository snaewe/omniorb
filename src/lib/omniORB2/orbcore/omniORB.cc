// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORB.cc                 Created on: 15/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//

/*
  $Log$
  Revision 1.3.2.1  2000/09/27 17:13:08  djs
  Struct member renaming
  Added command line options
  Added CORBA::ValueBase (just to do reference counting)
  General refactoring

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/14 16:22:13  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.3  1999/09/30 12:25:59  djr
  Minor changes.

  Revision 1.1.2.2  1999/09/24 17:11:13  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.1.2.1  1999/09/22 14:26:58  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

#include <stdlib.h>
#if defined(UnixArchitecture) || defined(__VMS)
#include <sys/time.h>
#include <unistd.h>
#elif defined(NTArchitecture)
#include <sys/types.h>
#include <sys/timeb.h>
#include <process.h>
#endif

#ifdef __atmos__
#include <kernel.h>
#include <timelib.h>
#include <sys/time.h>
#endif


// Globals defined in class omniORB
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace omniORB {

int                      traceLevel = 1;
int                      traceInvocations = 0;
CORBA::Boolean           strictIIOP = 1;
char*                    serverName = 0;
CORBA::Boolean           tcAliasExpand = 0;
unsigned int             maxTcpConnectionPerServer = 5;
unsigned int             AMIMaxQueueSize = 0;
unsigned int             AMIMaxWorkerThreads = 5;
unsigned int             AMIWorkerTimeout = 30;
CORBA::Boolean           diiThrowsSysExceptions = 0;
CORBA::Boolean           abortOnInternalError = 0;
_CORBA_ULong             objectTableSize = 0;
CORBA::Boolean           useTypeCodeIndirections = 1;
CORBA::Boolean           acceptMisalignedTcIndirections = 0;
CORBA::Boolean           verifyObjectExistsAndType = 1;
int                      poaHoldRequestTimeout = 0;
objectKey                seed;
}

#else
int                      omniORB::traceLevel = 1;
int                      omniORB::traceInvocations = 0;
CORBA::Boolean           omniORB::strictIIOP = 1;
#if defined(HAS_Cplusplus_Namespace)
char*                    omniORB::serverName = 0;
#else
CORBA::String_var	 omniORB::serverName((const char*) "unknown");
#endif
CORBA::Boolean           omniORB::tcAliasExpand = 0;
unsigned int             omniORB::maxTcpConnectionPerServer = 5;
unsigned int             omniORB::AMIMaxQueueSize = 0;
unsigned int             omniORB::AMIMaxWorkerThreads = 5;
unsigned int             omniORB::AMIWorkerTimeout = 30;
CORBA::Boolean           omniORB::diiThrowsSysExceptions = 0;
CORBA::Boolean           omniORB::abortOnInternalError = 0;
_CORBA_ULong             omniORB::objectTableSize = 0;
CORBA::Boolean           omniORB::useTypeCodeIndirections = 1;
CORBA::Boolean           omniORB::acceptMisalignedTcIndirections = 0;
CORBA::Boolean           omniORB::verifyObjectExistsAndType = 1;
int                      omniORB::poaHoldRequestTimeout = 0;
omniORB::objectKey       omniORB::seed;
#endif


void
omniORB::enableLcdMode()
{
  omniORB::strictIIOP = 0;
  omniORB::tcAliasExpand = 1;
  omniORB::idleConnectionScanPeriod(omniORB::idleIncoming, 0);
  omniORB::idleConnectionScanPeriod(omniORB::idleOutgoing, 0);
  omniORB::useTypeCodeIndirections = 0;
  omniORB::verifyObjectExistsAndType = 0;
  omniORB::acceptMisalignedTcIndirections = 1;
}

//////////////////////////////////////////////////////////////////////
/////////////////////// omniORB::fatalException //////////////////////
//////////////////////////////////////////////////////////////////////

omniORB::fatalException::fatalException(const char* file, int line,
					const char* errmsg)
  : pd_file(file), pd_line(line), pd_errmsg(errmsg)
{
  if( abortOnInternalError )  abort();
}

//////////////////////////////////////////////////////////////////////
///////////////////////// omniORB::objectKey /////////////////////////
//////////////////////////////////////////////////////////////////////

static omni_tracedmutex key_lock;


void
omniORB::generateNewKey(omniORB::objectKey& k)
{
  static int first_time = 1;

  omni_tracedmutex_lock sync(key_lock);

  if ( first_time ) 
    {
      first_time = 0;

      // one-time initialisation of the seed value
      // initialise the seed of the objectKey generator
      // Guarantee that no two keys generated on the same machine are the same
      // ever.
#if !defined(__WIN32__) && !(defined(__VMS) && __VMS_VER < 70000000)
      // Use gettimeofday() to obtain the current time. Use this to
      // initialise the 32-bit field hi and med in the seed.
      // On unices, add the process id to med.
      // Initialise lo to 0.
      struct timeval v;
#ifndef __SINIX__
      gettimeofday(&v,0);
#else
      gettimeofday(&v);
#endif
      omniORB::seed.hi = v.tv_sec;
      omniORB::seed.med = (v.tv_usec << 12);
#if defined(UnixArchitecture) || defined(__VMS)
      omniORB::seed.med += getpid();
#else
      // without the process id, there is no guarantee that the keys generated
      // by this process do not clash with those generated by other processes.
#endif
      omniORB::seed.lo = 0;
#elif defined(__WIN32__)
      // Unique number on NT
      // Use _ftime() to obtain the current system time. 
#ifndef __BCPLUSPLUS__
      struct _timeb v;
      _ftime(&v);
      omniORB::seed.hi = v.time;
      omniORB::seed.med = v.millitm + _getpid();
      omniORB::seed.lo = 0;
#else
      struct timeb v;
      ftime(&v);
      omniORB::seed.hi = v.time;
      omniORB::seed.med = v.millitm + getpid();
      omniORB::seed.lo = 0;
#endif
#elif defined(__VMS) && __VMS_VER < 70000000
      // VMS systems prior to 7.0
      timeb v;
      ftime(&v);
      omniORB::seed.hi = v.time;
      omniORB::seed.med = v.millitm + getpid();
      omniORB::seed.lo = 0;
#endif
      // 
      // Make sure that the three fields are in little-endian, byte swap
      // if necessary.
      if (omni::myByteOrder) {
	// big endian
	omniORB::seed.hi =  ((((omniORB::seed.hi) & 0xff000000) >> 24) | 
			     (((omniORB::seed.hi) & 0x00ff0000) >> 8)  | 
			     (((omniORB::seed.hi) & 0x0000ff00) << 8)  | 
			     (((omniORB::seed.hi) & 0x000000ff) << 24));
	omniORB::seed.med =  ((((omniORB::seed.med) & 0xff000000) >> 24) | 
			     (((omniORB::seed.med) & 0x00ff0000) >> 8)  | 
			     (((omniORB::seed.med) & 0x0000ff00) << 8)  | 
			     (((omniORB::seed.med) & 0x000000ff) << 24));
      }
  }
  omniORB::seed.lo++;  // note: seed.lo is in native endian
  k = omniORB::seed;
  if (omni::myByteOrder) {
    // big endian
    k.lo =  ((((k.lo) & 0xff000000) >> 24) | 
	     (((k.lo) & 0x00ff0000) >> 8)  | 
	     (((k.lo) & 0x0000ff00) << 8)  | 
	     (((k.lo) & 0x000000ff) << 24));
  }
  return;
}


omniORB::objectKey
omniORB::nullkey()
{
  omniORB::objectKey n;
  n.hi = n.med = n.lo = 0;
  return n;
}


#if defined(HAS_Cplusplus_Namespace)
namespace omniORB {
#endif

  // Some compilers which support namespace cannot handle operator==
  // definition prefix by omniORB::. We therefore enclose the
  // operator definitions in namespace omniORB scoping to avoid the problem.

int 
operator==(const omniORB::objectKey &k1,const omniORB::objectKey &k2)
{
  return (k1.hi == k2.hi &&
	  k1.med == k2.med &&
	  k1.lo == k2.lo) ? 1 : 0;
}


int 
operator!=(const omniORB::objectKey &k1,const omniORB::objectKey &k2)
{
  return (k1.hi != k2.hi ||
	  k1.med != k2.med ||
	  k1.lo != k2.lo) ? 1 : 0;
}

#if defined(HAS_Cplusplus_Namespace)
}
#endif


omniORB::seqOctets* 
omniORB::keyToOctetSequence(const omniORB::objectKey &k1)
{
  omniORB::seqOctets* result = new omniORB::seqOctets;
  result->length(sizeof(omniORB::objectKey));
  const CORBA::Octet* p = (const CORBA::Octet*) &k1;
  for (unsigned int i=0; i< sizeof(omniORB::objectKey); i++) {
    result->operator[](i) = p[i];
  }
  return result;
}


omniORB::objectKey
omniORB::octetSequenceToKey(const omniORB::seqOctets& seq)
{
  if (seq.length() != sizeof(omniORB::objectKey)) {
    OMNIORB_THROW(MARSHAL,0,CORBA::COMPLETED_NO);
  }
  omniORB::objectKey result;
  CORBA::Octet* p = (CORBA::Octet*) &result;
  for (unsigned int i=0; i< sizeof(omniORB::objectKey); i++) {
    p[i] = seq[i];
  }
  return result;
}
