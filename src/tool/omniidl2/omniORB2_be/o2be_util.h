// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_util.h              Created on: 5/7/1999
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
//  Useful stuff.
//

/*
 $Log$
 Revision 1.1.2.1  1999/09/24 10:05:33  djr
 Updated for omniORB3.

*/

#ifndef __O2BE_UTIL_H__
#define __O2BE_UTIL_H__

#include <string.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
////////////////////////////// StringBuf /////////////////////////////
//////////////////////////////////////////////////////////////////////

#define STRING_BUF_INC  64


class StringBuf {
public:
  inline StringBuf(size_t size = STRING_BUF_INC) {
    pd_current = pd_start = new char[size];
    pd_end = pd_start + size;
    *pd_current = '\0';
  }
  inline ~StringBuf() { delete[] pd_start; }

  inline operator const char* () const { return pd_start; }
  inline const char* c_str() const     { return pd_start; }

  inline void operator += (const char* s) {
    size_t len = strlen(s);
    if( len > free_space() )  reserve(len);
    memcpy(pd_current, s, len + 1);
    pd_current += len;
  }

  inline void operator += (char c) {
    if( free_space() < 1 )  reserve(1);
    *pd_current++ = c;
    *pd_current = '\0';
  }

  inline void operator += (int i) {
    char s[20];
    sprintf(s, "%d", i);
    *this += s;
  }

  struct extent {
    inline extent(const char* start, const char* end) : s(start), e(end) {}
    const char *s, *e;
  };
  // Copies from <start> up to, but not including <end>.
  inline void operator += (extent e) {
    size_t len = e.e - e.s;
    if( len > free_space() )  reserve(len);
    memcpy(pd_current, e.s, len);
    pd_current += len;
    *pd_current = '\0';
  }

  inline void clear() { pd_current = pd_start; *pd_current = '\0'; }

  void reserve(size_t n);
  // Allocate enough buffer st. there are at least n bytes of free space.

  void grab(char*);
  // Sets contents to the given string (which is consumed).
  // If the string is zero, then this is the same as clear().

  char* release();
  // Return the string, and re-initialises this string to empty.
  // Caller takes responsibility for releasing the memory.

private:
  inline size_t free_space() { return pd_end - pd_current - 1; }

  char* pd_start;
  char* pd_current;
  char* pd_end;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////// o2be_iterator ///////////////////////////
//////////////////////////////////////////////////////////////////////

template <class T, AST_Decl::NodeType T_NodeType>
class o2be_iterator {
public:
  inline o2be_iterator(UTL_Scope* s)
    : pd_it(s, UTL_Scope::IK_decls), pd_item(0) {
    while( !pd_it.is_done() ) {
      AST_Decl* d = pd_it.item();
      if( d->node_type() == T_NodeType ) {
	pd_item = T::narrow_from_decl(d);
	break;
      }
      pd_it.next();
    }
  }

  inline void next() {
    pd_item = 0;
    if( pd_it.is_done() )  return;
    AST_Decl* d = 0;
    do {
      pd_it.next();
      if( pd_it.is_done() )  return;
      d = pd_it.item();
    } while( d->node_type() != T_NodeType );
    pd_item = T::narrow_from_decl(d);
  }

  inline T* item()          { return pd_item;         }
  inline idl_bool is_done() { return pd_item ? 0 : 1; }

private:
  UTL_ScopeActiveIterator pd_it;
  T*                      pd_item;
};

//////////////////////////////////////////////////////////////////////
/////////////////////////// SimpleStringMap //////////////////////////
//////////////////////////////////////////////////////////////////////

class SimpleStringMap {
public:
  SimpleStringMap();
  ~SimpleStringMap();

  const char* operator [] (const char* key);
  void insert(const char* key, const char* val);
  // Insert the given pair into the table. Replaces any
  // existing entry.  Assumes that <key> and <val> will
  // be around at least as long as this map.

private:
  void more_entries();

  struct Entry {
    const char* key;
    const char* value;
  };

  Entry*   pd_entries;
  unsigned pd_entries_len;
  unsigned pd_nentries;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct o2be_verbatim {
  inline o2be_verbatim(const char* a) : data(a) {}
  const char* data;
};

std::ostream& operator << (std::ostream&, o2be_verbatim);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct o2be_template {
  inline o2be_template(SimpleStringMap& m, const char* a=0)
    : data(a), map(m) {}
  const char* data;
  SimpleStringMap& map;
};

std::ostream& operator << (std::ostream&, o2be_template);


#endif  // __O2BE_UTIL_H__
