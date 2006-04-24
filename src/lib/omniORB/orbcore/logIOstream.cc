// -*- Mode: C++; -*-
//                            Package   : omniORB
// logIOstream.cc             Created on: 31/3/1998
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1998-1999 AT&T Laboratories Cambridge
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
//      
 
/*
  $Log$
  Revision 1.8.2.14  2006/04/24 15:10:18  dgrisby
  Make sure values are unsigned in isalnum, etc., to avoid stupid
  assertion failures in MSVC runtime.

  Revision 1.8.2.13  2005/09/19 14:23:56  dgrisby
  New traceFile configuration parameter.

  Revision 1.8.2.12  2004/03/05 14:20:00  dgrisby
  Better logging of object keys.

  Revision 1.8.2.11  2003/07/02 11:01:06  dgrisby
  Race condition in POA destruction.

  Revision 1.8.2.10  2002/10/30 16:49:03  dgrisby
  Log flush broken when using log function.

  Revision 1.8.2.9  2002/01/15 16:38:13  dpg1
  On the road to autoconf. Dependencies refactored, configure.ac
  written. No makefiles yet.

  Revision 1.8.2.8  2002/01/09 11:39:23  dpg1
  New omniORB::setLogFunction() function.

  Revision 1.8.2.7  2001/09/19 17:30:04  dpg1
  New traceThreadId option to add omni_thread id to log messages.

  Revision 1.8.2.6  2001/08/17 17:07:06  sll
  Remove the use of omniORB::logStream.

  Revision 1.8.2.5  2001/08/15 10:26:12  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.8.2.4  2001/08/03 17:41:22  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.8.2.3  2001/05/11 14:25:53  sll
  Added operator for omniORB::logger to report system exception status and
  minor code.

  Revision 1.8.2.2  2000/09/27 17:35:49  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.8.2.1  2000/07/17 10:35:55  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.9  2000/07/13 15:25:57  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.7.6.2  1999/09/28 10:54:33  djr
  Removed pretty-printing of object keys from object adapters.

  Revision 1.7.6.1  1999/09/22 14:26:54  djr
  Major rewrite of orbcore to support POA.

  Revision 1.6  1999/09/01 12:57:46  djr
  Added atomic logging class omniORB::logger, and methods logf() and logs().

  Revision 1.5  1999/03/11 16:25:54  djr
  Updated copyright notice

  Revision 1.4  1999/01/07 15:59:13  djr
  Corrected minor bug in fprintf format.

  Revision 1.3  1998/08/14 13:48:52  sll
  Added pragma hdrstop to control pre-compile header if the compiler feature
  is available.

  Revision 1.2  1998/04/18 10:11:47  sll
  Corrected typo (_log instead of log).

  Revision 1.1  1998/04/07 20:24:40  sll
  Initial revision

  */

// Implement omniORB::logger using stderr.



// Macros to handle std namespace and streams header files

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <objectTable.h>
#include <remoteIdentity.h>
#include <inProcessIdentity.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniORB::logger //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef INIT_BUF_SIZE
#undef INIT_BUF_SIZE
#endif
#define INIT_BUF_SIZE  256

#define PREFIX "omniORB: "

static FILE*             logfile = stderr;
static CORBA::String_var logfilename;


OMNI_USING_NAMESPACE(omni)


static inline omniORB::logFunction& logfunc()
{
  static omniORB::logFunction f = 0;
  return f;
}

void
omniORB::setLogFunction(omniORB::logFunction f)
{
  logfunc() = f;
}

void
omniORB::setLogFilename(const char* n)
{
  const char* existing = (const char*)logfilename;
  if (existing && !strcmp(existing, n)) {
    // Already using this file
    return;
  }
  FILE* f = fopen(n, "a");
  if (!f) {
    // Output an error to the existing logger
    if (omniORB::trace(1)) {
      omniORB::logger l;
      l << "Unable to open log file '" << n << "'.\n";
    }
    OMNIORB_THROW(INITIALIZE, INITIALIZE_CannotOpenLogFile,
		  CORBA::COMPLETED_NO);
  }
  if ((const char*)logfilename) {
    // Close existing file
    fclose(logfile);
  }
  logfile = f;
  logfilename = n;
}

const char*
omniORB::getLogFilename()
{
  return (const char*)logfilename;
}
  

omniORB::logger::logger(const char* prefix)
  : pd_prefix(prefix), pd_buf(new char[INIT_BUF_SIZE])
{
  if( !pd_prefix )  pd_prefix = PREFIX;

  strcpy(pd_buf, pd_prefix);
  pd_p = pd_buf + strlen(pd_prefix);
  pd_end = pd_buf + INIT_BUF_SIZE;

  if (omniORB::traceThreadId) {
    omni_thread* self = omni_thread::self();
    if (self)
      *this << "(" << self->id() << ") ";
    else
      *this << "(?) ";
  }
}


omniORB::logger::~logger()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) ) {
    if (logfunc())
      logfunc()(pd_buf);
    else
      fputs(pd_buf, logfile);
  }
  delete[] pd_buf;
}


omniORB::logger& 
omniORB::logger::operator<<(char c)
{
  reserve(1);
  *pd_p++ = c;
  *pd_p = '\0';
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const char *s)
{
  size_t len = strlen(s);
  reserve(len);
  strcpy(pd_p, s);
  pd_p += len;
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const void *p)
{
  reserve(30); // guess!
  sprintf(pd_p, "%p", p);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(int n)
{
  reserve(20);
  sprintf(pd_p, "%d", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(unsigned int n)
{
  reserve(20);
  sprintf(pd_p, "%u", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(long n)
{
  reserve(30);
  sprintf(pd_p, "%ld", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(unsigned long n)
{
  reserve(30);
  sprintf(pd_p, "%lu", n);
  pd_p += strlen(pd_p);
  return *this;
}


#ifndef NO_FLOAT
omniORB::logger&
omniORB::logger::operator<<(double n)
{
  reserve(30);
  sprintf(pd_p, "%g", n);
  pd_p += strlen(pd_p);
  return *this;
}
#endif


static void pp_key(omniORB::logger& l, const CORBA::Octet*, int);


omniORB::logger&
omniORB::logger::operator<<(const omniLocalIdentity* id)
{
  OMNIORB_ASSERT(id);

  pp_key(*this, id->key(), id->keysize());

  omniObjTableEntry* entry=omniObjTableEntry::downcast((omniLocalIdentity*)id);
  if (entry) {
    switch (entry->state()) {
    case omniObjTableEntry::ACTIVATING:    *this << " (activating)";     break;
    case omniObjTableEntry::ACTIVE:        *this << " (active)";         break;
    case omniObjTableEntry::DEACTIVATING:  *this << " (deactivating)";   break;
    case omniObjTableEntry::DEACTIVATING_OA:
                                           *this << " (deactivating OA)";break;
    case omniObjTableEntry::ETHEREALISING: *this << " (etherealising)";  break;
    case omniObjTableEntry::DEAD:          *this << " (dead)";           break;
    default:                               *this << " (???)";
    }
  }
  else
    *this << " (temp)";

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const omniIdentity* id)
{
  OMNIORB_ASSERT(id);

  pp_key(*this, id->key(), id->keysize());

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(omniObjKey& k)
{
  pp_key(*this, k.key(), k.size());

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const omniORB::logger::exceptionStatus& ex)
{
  switch (ex.status) {
  case CORBA::COMPLETED_YES:
    *this << "YES,";
    break;
  case CORBA::COMPLETED_NO:
    *this << "NO,";
    break;
  case CORBA::COMPLETED_MAYBE:
    *this << "MAYBE,";
    break;
  }
  if (ex.minor_string) {
    *this << ex.minor_string;
  }
  else {
    reserve(30);
    sprintf(pd_p, "0x%08x", (int)ex.minor);
    pd_p += strlen(pd_p);
  }
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(const CORBA::SystemException& ex)
{
  int sz;
  *this << ex._NP_repoId(&sz);
  switch (ex.completed()) {
  case CORBA::COMPLETED_YES:
    *this << ",YES,";
    break;
  case CORBA::COMPLETED_NO:
    *this << ",NO,";
    break;
  case CORBA::COMPLETED_MAYBE:
    *this << ",MAYBE,";
    break;
  }
  reserve(30);
  sprintf(pd_p, "0x%08x", (int)ex.minor());
  pd_p += strlen(pd_p);
  return *this;
}

void
omniORB::logger::flush()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) ) {
    if (logfunc())
      logfunc()(pd_buf);
    else
      fprintf(logfile, "%s", pd_buf);
  }
  pd_p = pd_buf + strlen(pd_prefix);
  *pd_p = '\0';
}


void
omniORB::logger::more(int n)
{
  int used = pd_p - pd_buf + 1;
  int size = pd_end - pd_buf;

  while( size - used < n )  size *= 2;

  char* newbuf = new char[size];
  strcpy(newbuf, pd_buf);
  char* newp = newbuf + (used - 1);
  delete[] pd_buf;
  pd_buf = newbuf;
  pd_p = newp;
  pd_end = pd_buf + size;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef INLINE_BUF_SIZE
#undef INLINE_BUF_SIZE
#endif
#define INLINE_BUF_SIZE  256

#ifndef __CIAO__
void
omniORB::logf(const char* fmt ...)
{
  char inlinebuf[INLINE_BUF_SIZE];
  char* buf = inlinebuf;
  size_t fmtlen = strlen(fmt) + sizeof(PREFIX) + 15;

  if( fmtlen > INLINE_BUF_SIZE )  buf = new char[fmtlen];

  if (traceThreadId) {
    omni_thread* self = omni_thread::self();
    if (self)
      sprintf(buf, "%s(%d) %s\n", PREFIX, self->id(), fmt);
    else
      sprintf(buf, "%s(?) %s\n", PREFIX, fmt);
  }
  else
    sprintf(buf, "%s%s\n", PREFIX, fmt);

  va_list args;
  va_start(args, fmt);

  if (logfunc()) {
    char  oinline[INLINE_BUF_SIZE * 4];
    char* obuf = oinline;
    int   obufsize = INLINE_BUF_SIZE * 4;
    int   nchars;
#ifdef HAVE_SNPRINTF
    while (1) {
      nchars = vsnprintf(obuf, obufsize, buf, args);
      if (nchars > -1 && nchars < obufsize)
	break;
      if (nchars > -1)
	obufsize = nchars + 1;
      else
	obufsize += 2;
      if (obuf != oinline) delete [] obuf;
      obuf = new char[obufsize];
    }
#else
    nchars = vsprintf(obuf, buf, args);
    OMNIORB_ASSERT(nchars >= 0 && nchars < obufsize);
#endif
    logfunc()(obuf);
    if (obuf != oinline) delete [] obuf;
  }
  else {
    vfprintf(logfile, buf, args);
  }
  va_end(args);

  if( buf != inlinebuf )  delete[] buf;
}
#endif


void
omniORB::do_logs(const char* mesg)
{
  char inlinebuf[INLINE_BUF_SIZE];
  char* buf = inlinebuf;
  size_t fmtlen = strlen(mesg) + sizeof(PREFIX) + 15;

  if( fmtlen > INLINE_BUF_SIZE )  buf = new char[fmtlen];

  if (traceThreadId) {
    omni_thread* self = omni_thread::self();
    if (self)
      sprintf(buf, "%s(%d) %s\n", PREFIX, self->id(), mesg);
    else
      sprintf(buf, "%s(?) %s\n", PREFIX, mesg);
  }
  else
    sprintf(buf, "%s%s\n", PREFIX, mesg);

  if (logfunc())
    logfunc()(buf);
  else
    fputs(buf, logfile);

  if( buf != inlinebuf )  delete[] buf;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// These should of course not be here ...
#define POA_NAME_SEP            '\xff'
#define POA_NAME_SEP_STR        "\xff"
#define TRANSIENT_SUFFIX_SEP    '\xfe'
#define TRANSIENT_SUFFIX_SIZE   8

static char cm[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };


static int is_poa_key(const CORBA::Octet* key, int keysize)
{
  const char* k = (const char*) key;
  const char* kend = k + keysize;

  if( *k != TRANSIENT_SUFFIX_SEP && *k != POA_NAME_SEP )  return 0;

  while( k < kend && *k == POA_NAME_SEP ) {
    k++;
    while( k < kend && *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      k++;
  }

  if( k == kend )  return 0;

  if( *k == TRANSIENT_SUFFIX_SEP )
    k += TRANSIENT_SUFFIX_SIZE + 1;
  if( k >= kend || *k )  return 0;

  return 1;
}


static int is_boa_key(const CORBA::Octet* key, int keysize)
{
  return keysize == sizeof(omniOrbBoaKey);
}


static char* pp_poa_key(const CORBA::Octet* key, int keysize)
{
  // output: root/poa/name<key>

  const char* k = (const char*) key;
  const char* kend = k + keysize;

  // We play safe with the size.  It can be slightly bigger than
  // the key because we prefix 'root', and the object id may be
  // pretty printed larger than its octet representation.
  char* ret = new char[keysize * 2 + 20];
  char* s = ret;

  strcpy(s, "root");  s += 4;

  while( k < kend && *k == POA_NAME_SEP ) {
    *s++ = '/';
    k++;

    while( *k && *k != POA_NAME_SEP && *k != TRANSIENT_SUFFIX_SEP )
      *s++ = *k++;
  }

  if( *k == TRANSIENT_SUFFIX_SEP )
    k += TRANSIENT_SUFFIX_SIZE + 1;

  k++;
  *s++ = '<';
  CORBA::ULong idsize = kend - k;
  if (idsize == 12) {
    // Persistent POA key (we hope)

    while (idsize > 4) {
      *s++ = cm[(unsigned char)*k >> 4];
      *s++ = cm[(unsigned char)*k & 0xf];
      k++;
      idsize--;
    }
    *s++ = '/';
  }
  if( idsize == 4 ) {
    CORBA::ULong val = 0;
    while (idsize--)
      val += ((CORBA::ULong)((unsigned char)*k++)) << (idsize * 8);

    sprintf(s, "%lu", (unsigned long) val);
    s += strlen(s);
  }
  else {
    while( idsize-- )  { *s++ = isalnum((unsigned char)*k) ? *k : '.'; k++; }
  }

  *s++ = '>';
  *s++ = '\0';

  return ret;
}



static char* pp_boa_key(const CORBA::Octet* key, int keysize)
{
  // output: boa<key-in-hex>

  int size = 8 + keysize * 2;
  char* ret = new char[size];
  char* s = ret;
  strcpy(s, "boa<0x");
  s += strlen(s);

  const unsigned char* k = (const unsigned char*) key;

  for( int i = 0; i < keysize; i++, k++ ) {
    *s++ = cm[*k >> 4];
    *s++ = cm[*k & 0xf];
  }
  *s++ = '>';
  *s++ = '\0';

  return ret;
}


static char* pp_key(const CORBA::Octet* key, int keysize)
{
  // output: key<keystring>

  int size = 8 + keysize;
  char* ret = new char[size];
  char* s = ret;
  strcpy(s, "key<");
  s += strlen(s);

  const char* k = (const char*) key;

  for( int i = 0; i < keysize; i++, k++ ) {
    *s++ = isalnum((unsigned char)*k) ? *k : '.';
  }
  *s++ = '>';
  *s++ = '\0';

  return ret;
}


static void pp_key(omniORB::logger& l, const CORBA::Octet* key, int keysize)
{
  char* p;

  if( is_poa_key(key, keysize) )
    p = pp_poa_key(key, keysize);
  else if( is_boa_key(key, keysize) )
    p = pp_boa_key(key, keysize);
  else
    p = pp_key(key, keysize);

  l << p;
  delete[] p;
}
