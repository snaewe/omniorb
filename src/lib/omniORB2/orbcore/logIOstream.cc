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
  Revision 1.11  2001/02/21 14:12:13  dpg1
  Merge from omni3_develop for 3.0.3 release.

  Revision 1.7.6.3  2000/08/07 15:34:35  dpg1
  Partial back-port of long long from omni3_1_develop.

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

// Implement omniORB::logStream using stderr.



// Macros to handle std namespace and streams header files

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <localIdentity.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

static omniORB::logStream _log;

#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the variables external linkage otherwise. Its a bug.
namespace omniORB {

logStream& log = _log;

}
#else

omniORB::logStream& omniORB::log = _log;

#endif


omniORB::logStream::logStream() : pd_state(0)
{
}

omniORB::logStream::~logStream()
{
}

omniORB::logStream& 
omniORB::logStream::operator<<(char c)
{
  fprintf(stderr,"%c",c);
  return *this;
}


omniORB::logStream&
omniORB::logStream::operator<<(const char *s)
{
  fprintf(stderr,"%s",s);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(const void *p)
{
  fprintf(stderr,"%p",p);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(int n)
{
  fprintf(stderr,"%d",n);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(unsigned int n)
{
  fprintf(stderr,"%u",n);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(long n)
{
  fprintf(stderr,"%ld",n);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(unsigned long n)
{
  fprintf(stderr,"%lu",n);
  return *this;
}

#ifdef HAS_LongLong
omniORB::logStream&
omniORB::logStream::operator<<(_CORBA_LONGLONG_DECL n)
{
  fprintf(stderr,"%lld",n);
  return *this;
}

omniORB::logStream&
omniORB::logStream::operator<<(_CORBA_ULONGLONG_DECL n)
{
  fprintf(stderr,"%llu",n);
  return *this;
}
#endif

#ifndef NO_FLOAT
omniORB::logStream&
omniORB::logStream::operator<<(double n)
{
  fprintf(stderr,"%g",n);
  return *this;
}

#ifdef HAS_LongDouble
omniORB::logStream&
omniORB::logStream::operator<<(_CORBA_LONGDOUBLE_DECL n)
{
  fprintf(stderr,"%Lg",n);
  return *this;
}
#endif

#endif

omniORB::logStream&
omniORB::logStream::flush()
{
  fflush(stderr);
  return *this;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// omniORB::logger //////////////////////////
//////////////////////////////////////////////////////////////////////

#ifdef INIT_BUF_SIZE
#undef INIT_BUF_SIZE
#endif
#define INIT_BUF_SIZE  256

#define PREFIX           "omniORB: "


omniORB::logger::logger(const char* prefix)
  : pd_prefix(prefix), pd_buf(new char[INIT_BUF_SIZE])
{
  if( !pd_prefix )  pd_prefix = PREFIX;

  strcpy(pd_buf, pd_prefix);
  pd_p = pd_buf + strlen(pd_prefix);
  pd_end = pd_buf + INIT_BUF_SIZE;
}


omniORB::logger::~logger()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) )
    fprintf(stderr, "%s", pd_buf);

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

#ifdef HAS_LongLong
omniORB::logger&
omniORB::logger::operator<<(_CORBA_LONGLONG_DECL n)
{
  reserve(60);
  sprintf(pd_p, "%lld", n);
  pd_p += strlen(pd_p);
  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(_CORBA_ULONGLONG_DECL n)
{
  reserve(60);
  sprintf(pd_p, "%llu", n);
  pd_p += strlen(pd_p);
  return *this;
}
#endif

#ifndef NO_FLOAT
omniORB::logger&
omniORB::logger::operator<<(double n)
{
  reserve(30);
  sprintf(pd_p, "%g", n);
  pd_p += strlen(pd_p);
  return *this;
}

#ifdef HAS_LongDouble
omniORB::logger&
omniORB::logger::operator<<(_CORBA_LONGDOUBLE_DECL n)
{
  reserve(60);
  sprintf(pd_p, "%Lg", n);
  pd_p += strlen(pd_p);
  return *this;
}
#endif

#endif


static void pp_key(omniORB::logger& l, const CORBA::Octet*, int);


omniORB::logger&
omniORB::logger::operator<<(omniLocalIdentity* id)
{
  OMNIORB_ASSERT(id);

  pp_key(*this, id->key(), id->keysize());
  if( !id->adapter() )  *this << " (not activated)";

  return *this;
}


omniORB::logger&
omniORB::logger::operator<<(omniIdentity* id)
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


void
omniORB::logger::flush()
{
  if( (size_t)(pd_p - pd_buf) != strlen(pd_prefix) )
    fprintf(stderr, "%s", pd_buf);

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
  size_t fmtlen = strlen(fmt) + sizeof(PREFIX) + 1;

  if( fmtlen > INLINE_BUF_SIZE )  buf = new char[fmtlen];

  strcpy(buf, PREFIX);
  strcpy(buf + sizeof(PREFIX) - 1, fmt);
  strcat(buf, "\n");

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, buf, args);
  va_end(args);

  if( buf != inlinebuf )  delete[] buf;
}
#endif


void
omniORB::do_logs(const char* fmt)
{
  char inlinebuf[INLINE_BUF_SIZE];
  char* buf = inlinebuf;
  size_t fmtlen = strlen(fmt) + sizeof(PREFIX) + 1;

  if( fmtlen > INLINE_BUF_SIZE )  buf = new char[fmtlen];

  strcpy(buf, PREFIX);
  strcpy(buf + sizeof(PREFIX) - 1, fmt);
  strcat(buf, "\n");

  fprintf(stderr, "%s", buf);

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
  char* ret = new char[keysize + 20];
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
  int idsize = kend - k;
  if( idsize == 4 ) {
    CORBA::ULong val;
    char* p = (char*) &val;
    *p++ = *k++;  *p++ = *k++;  *p++ = *k++;  *p++ = *k++;
    sprintf(s, "%lu", (unsigned long) val);
    s += strlen(s);
  }
  else {
    while( idsize-- )  { *s++ = isalnum(*k) ? *k : '.'; k++; }
  }

  *s++ = '>';
  *s++ = '\0';

  return ret;
}


static char cm[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };


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
  // output: key<key-in-hex>

  int size = 8 + keysize * 2;
  char* ret = new char[size];
  char* s = ret;
  strcpy(s, "key<0x");
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
