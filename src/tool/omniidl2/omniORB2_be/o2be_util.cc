// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_util.cc             Created on: 20/4/1999
//			    Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl2.
//
//  Omniidl2 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// Description:
//

#include <idl.hh>
#include <idl_extern.hh>
#include <o2be.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <o2be_util.h>
#include <ctype.h>


//////////////////////////////////////////////////////////////////////
////////////////////////////// StringBuf /////////////////////////////
//////////////////////////////////////////////////////////////////////

void
StringBuf::reserve(size_t n)
{
  size_t extra = STRING_BUF_INC;
  while( extra + free_space() < n )
    extra += STRING_BUF_INC;
  size_t newsize = (pd_end - pd_start) + extra;
  char* newptr = new char[newsize];
  strcpy(newptr, pd_start);
  pd_current = newptr + (pd_current - pd_start);
  pd_end = newptr + newsize;
  pd_start = newptr;
}


void
StringBuf::grab(char* s)
{
  if( s ) {
    delete[] pd_start;
    pd_start = s;
    pd_current = s + strlen(s);
    pd_end = pd_current + 1;
  }
  else {
    pd_current = pd_start;
    *pd_current = '\0';
  }
}


char*
StringBuf::release()
{
  char* p = new char[pd_current - pd_start + 1];
  memcpy(p, pd_start, pd_current - pd_start + 1);
  pd_current = pd_start;
  *pd_current = '\0';
  return p;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// SimpleStringMap //////////////////////////
//////////////////////////////////////////////////////////////////////

SimpleStringMap::SimpleStringMap()
{
  pd_entries_len = 30;
  pd_entries = new Entry[pd_entries_len];
  pd_nentries = 0;
}


SimpleStringMap::~SimpleStringMap()
{
  delete[] pd_entries;
}


const char*
SimpleStringMap::operator [] (const char* key)
{
  unsigned top = pd_nentries;
  unsigned bottom = 0;

  // Binary search to find entry.
  while( bottom < top ) {
    unsigned middle = (bottom + top) / 2;
    int cmp = strcmp(key, pd_entries[middle].key);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                return pd_entries[middle].value;
  }

  return 0;
}


void
SimpleStringMap::insert(const char* key, const char* value)
{
  unsigned top = pd_nentries;
  unsigned bottom = 0;

  // Binary search to find insertion point.
  while( bottom < top ) {
    unsigned middle = (bottom + top) / 2;
    int cmp = strcmp(key, pd_entries[middle].key);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else {
      pd_entries[middle].value = value;
      return;
    }
  }

  // bottom == top, and is our insertion point.

  if( pd_nentries == pd_entries_len )  more_entries();

  for( unsigned i = pd_nentries; i > bottom; i-- )
    pd_entries[i] = pd_entries[i - 1];

  pd_entries[bottom].key = key;
  pd_entries[bottom].value = value;
  pd_nentries++;
}


void
SimpleStringMap::more_entries()
{
  unsigned new_len = pd_entries_len * 3 / 2;
  Entry* new_entries = new Entry[new_len];

  for( unsigned i = 0; i < pd_nentries; i++ )
    new_entries[i] = pd_entries[i];

  delete[] pd_entries;
  pd_entries = new_entries;
  pd_entries_len = new_len;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& s, o2be_verbatim v)
{
  StringBuf sb;
  const char* p = v.data;

  while( *p ) {
    const char* e;

    if( (e = strchr(p, '\n')) ) {
      if( e == p )  s << '\n';
      else {
	IND(s);
	sb.clear();
	sb += StringBuf::extent(p, e);
	s << sb.c_str() << '\n';
      }
      p = e + 1;
    }
    else {
      if( *p )  IND(s);
      s << p;
      return s;
    }
  }

  return s;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& s, o2be_template t)
{
  StringBuf sb;
  const char* p = t.data;
  int start_of_line = 1;

  while( *p ) {

    const char* e = p;

    while( *e && *e != '\n' && !isalpha(*e) && *e != '_' )
      e++;

    if( start_of_line ) {
      if( *p != '\n' && *p != '#' )  IND(s);
      start_of_line = 0;
    }

    if( !*e ) {
      s << p;
      return s;
    }
    else if( *e == '\n' ) {
      sb.clear();
      sb += StringBuf::extent(p, e);
      s << sb.c_str() << '\n';
      p = e + 1;
      start_of_line = 1;
    }
    else {
      // isalpha(*e) || *e == '_'
      sb.clear();
      sb += StringBuf::extent(p, e);
      s << sb.c_str();

      p = e;
      do e++; while( isalnum(*e) || *e == '_' );

      sb.clear();
      sb += StringBuf::extent(p, e);
      const char* val = t.map[sb];
      if( val )  s << val;
      else       s << sb.c_str();
      p = e;

      // @ is used to concatenate ids, so skip.
      if( *p == '@' )  p++;
    }

  }

  return s;
}
