// -*- Mode: C++; -*-
//                            Package   : omniORB2
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
  Revision 1.7  1999/09/02 11:24:21  djr
  Minor bug in omniORB::logger::flush().

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

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <stdio.h>
#include <stdarg.h>

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

#ifndef NO_FLOAT
omniORB::logStream&
omniORB::logStream::operator<<(double n)
{
  fprintf(stderr,"%g",n);
  return *this;
}
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