// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaString.cc             Created on: 20/9/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//      Implementation of the String interface
//	

/*
  $Log$
  Revision 1.1  1997/01/08 17:26:01  sll
  Initial revision

 */

#include <omniORB2/CORBA.h>

char *
CORBA::string_alloc(CORBA::ULong len)
{
  return new char[(int)len];
}

void
CORBA::string_free(char *p)
{
  delete [] p;
}

char *
CORBA::string_dup(const char *p)
{
  if (p) {
    char *q = CORBA::string_alloc((CORBA::ULong)(strlen(p)+1));
    if (q) {
      strcpy(q,p);
      return q;
    }
  }
  return 0;
}

CORBA::String_member&
CORBA::String_member::operator= (const CORBA::String_var &s)
{
  if (_ptr) {
    string_free(_ptr);
    _ptr = 0;
  }
  if ((const char *)s) {
    _ptr = string_alloc((ULong)(strlen(s)+1));
    strcpy(_ptr,s);
  }
  return *this;
}

CORBA::String_var::String_var(const CORBA::String_member &s)
{
  if ((const char*)s) {
    _data = string_alloc((ULong)(strlen(s)+1));
    strcpy(_data,s);
  }
  else
    _data = 0;
}


CORBA::String_var&
CORBA::String_var::operator= (const CORBA::String_member &s)
{
  if (_data) {
    string_free(_data);
    _data = 0;
  }
  if ((const char*)s) {
    _data = string_alloc((ULong)(strlen(s)+1));
    strcpy(_data,s);
  }
  return *this;
}


char &
CORBA::String_var::operator[] (CORBA::ULong index) 
{
  if (!_data || strlen(_data) < (int)index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}

char
CORBA::String_var::operator[] (CORBA::ULong index) const
{
  if (!_data || strlen(_data) < (int)index) {
    _CORBA_bound_check_error();	// never return
  }
  return _data[index];
}

void
CORBA::String_member::operator>>= (NetBufferedStream &s) const
{
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  CORBA::ULong _len = strlen((char *)_ptr)+1;
  _len >>= s;
  s.put_char_array((CORBA::Char *)_ptr,_len);
  return;
}

void
CORBA::String_member::operator<<= (NetBufferedStream &s)
{  
  CORBA::ULong _len;
  _len <<= s;
  if (!_len || s.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_ptr) {
    CORBA::string_free(_ptr);
    _ptr = 0;
  }
  _ptr = CORBA::string_alloc(_len);
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  s.get_char_array((CORBA::Char *)_ptr,_len);
  return;
}

void
CORBA::String_member::operator>>= (MemBufferedStream &s) const
{
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  CORBA::ULong _len = strlen((char *)_ptr)+1;
  _len >>= s;
  s.put_char_array((CORBA::Char *)_ptr,_len);
  return;
}

void
CORBA::String_member::operator<<= (MemBufferedStream &s)
{
  CORBA::ULong _len;
  _len <<= s;
  if (!_len || s.unRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  if (_ptr) {
    CORBA::string_free(_ptr);
    _ptr = 0;
  }
  _ptr = CORBA::string_alloc(_len);
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  s.get_char_array((CORBA::Char *)_ptr,_len);
  return;
}

size_t
CORBA::String_member::NP_alignedSize(size_t initialoffset) const
{
  if (!_ptr)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  size_t alignedsize = omniORB::align_to(initialoffset,omniORB::ALIGN_4);
  alignedsize += 4 + strlen((char *)_ptr) + 1;
  return alignedsize;
}

