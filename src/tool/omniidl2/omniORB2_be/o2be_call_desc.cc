// -*- Mode: C++; -*-
//                          Package   : omniidl2
// o2be_call_desc.cc        Created on: 12/1998
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
#include <o2be_stringbuf.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <string.h>


#define PROXY_CALL_DESC_PREFIX      "_0RL_pc_"
#define STD_PROXY_CALL_DESC_PREFIX  "OmniProxyCallWrapper::"

#define UP_CALL_DESC_PREFIX      "_0RL_uc_"
#define STD_UP_CALL_DESC_PREFIX  "OmniUpCallWrapper::"


//////////////////////////////////////////////////////////////////////
/////////////////////////// SimpleStringMap //////////////////////////
//////////////////////////////////////////////////////////////////////

class SimpleStringMap {
private:
  struct Entry {
    const char* key;
    const char* value;
  };

public:
  inline SimpleStringMap() {
    pd_entries_len = 30;
    pd_entries = new Entry[pd_entries_len];
    pd_nentries = 0;
  }
  inline ~SimpleStringMap() { delete[] pd_entries; }

  const char* operator [] (const char* key);
  void insert(const char* sig, const char* class_nm);
  // Insert the given pair into the table. Replaces an
  // existing entry.

private:
  void more_entries();

  Entry* pd_entries;
  unsigned pd_entries_len;
  unsigned pd_nentries;
};


const char*
SimpleStringMap::operator [] (const char* key)
{
  unsigned top = pd_nentries;
  unsigned bottom = 0;

  // Binary search to find entry.
  while( bottom < top ) {
    unsigned i = (bottom + top) / 2;
    int cmp = strcmp(key, pd_entries[i].key);

    if( cmp < 0 )       top = i;
    else if( cmp > 0 )  bottom = (bottom == i) ? i + 1 : i;
    else                return pd_entries[i].value;
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
    unsigned i = (bottom + top) / 2;
    int cmp = strcmp(key, pd_entries[i].key);

    if( cmp < 0 )       top = i;
    else if( cmp > 0 )  bottom = (bottom == i) ? i + 1 : i;
    else {
      pd_entries[i].value = value;
      return;
    }
  }

  // <bottom> is our insertion point.

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
//////////////////////////// CallDescTable ///////////////////////////
//////////////////////////////////////////////////////////////////////

class CallDescTable : private SimpleStringMap {
public:
  CallDescTable();

  const char* operator [] (const char* sig) {
    return SimpleStringMap::operator[](sig);
  }
  const char* insert(const char* sig, const char* op_idname);

  char* generate_unique_name(const char* prefix);//DH

  idl_bool doneProxyCallDesc(const char* sig);
  idl_bool doneUpCallDesc(const char* sig);
  void setDoneProxyCallDesc(const char* sig);
  void setDoneUpCallDesc(const char* sig);

private:
  void initialise_base(const char* sig);
  char* generate_class_name(const char* sig);

  unsigned long pd_base_l;
  unsigned long pd_base_h;
  idl_bool pd_base_initialised;
  unsigned long pd_index;

  SimpleStringMap pd_doneProxy;
  SimpleStringMap pd_doneUpCall;
};


CallDescTable::CallDescTable()
{
  pd_base_initialised = I_FALSE;
  pd_index = 0;

  // Insert the pre-defined call descriptor types.
  SimpleStringMap::insert("void", "void_call");
  SimpleStringMap::insert("_wvoid", "ow_void_call");

  pd_doneProxy.insert("void","done");
  pd_doneUpCall.insert("void","done");
  pd_doneProxy.insert("_wvoid","done");
  pd_doneUpCall.insert("_wvoid","done");
}


const char*
CallDescTable::insert(const char* sig, const char* op_idname)
{
  if( !pd_base_initialised )  initialise_base(op_idname);

  const char* class_name = generate_class_name(sig);
  SimpleStringMap::insert(sig, class_name);

  return class_name;
}


void
CallDescTable::initialise_base(const char* s)
{
  pd_base_initialised = I_TRUE;
  pd_base_h = pd_base_l = 0u;

  while( *s ) {
    unsigned long tmp;
    tmp = (pd_base_h & 0xfe000000) >> 25;
    pd_base_h = (pd_base_h << 7) ^ ((pd_base_l & 0xfe000000) >> 25);
    pd_base_l = (pd_base_l << 7) ^ tmp;
    pd_base_l ^= (unsigned long) *s++;
  }
}


char*
CallDescTable::generate_class_name(const char* sig)
{
  static char chrmap[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
  };

  char* result = new char[1 + 16 + 1 + 8];

  char* s = result;

  for( unsigned i = 0 ; i < 8; i++ )
    *s++ = chrmap[(pd_base_h >> i * 4) & 0xf];
  for( unsigned j = 0 ; j < 8; j++ )
    *s++ = chrmap[(pd_base_l >> j * 4) & 0xf];

  *s++ = '_';
  for( unsigned k = 0 ; k < 8; k++ )
    *s++ = chrmap[(pd_index >> k * 4) & 0xf];
  *s = '\0';

  pd_index++;
  return result;
}


char*
CallDescTable::generate_unique_name(const char* prefix)//DH
{
  if( !pd_base_initialised )
    throw o2be_internal_error(__FILE__, __LINE__,
			      "generate_unique_name called before"
			      " initialisation");

  static char chrmap[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
  };

  char* result = new char[1 + strlen(prefix) + 16 + 1 + 8];

  strcpy(result, prefix);
  char* s = result + strlen(result);

  for( unsigned i = 0 ; i < 8; i++ )
    *s++ = chrmap[(pd_base_h >> i * 4) & 0xf];
  for( unsigned j = 0 ; j < 8; j++ )
    *s++ = chrmap[(pd_base_l >> j * 4) & 0xf];

  *s++ = '_';
  for( unsigned k = 0 ; k < 8; k++ )
    *s++ = chrmap[(pd_index >> k * 4) & 0xf];
  *s = '\0';

  pd_index++;
  return result;
}


idl_bool
CallDescTable::doneProxyCallDesc(const char* sig)
{
  return (pd_doneProxy[sig] ? 1 : 0);
}

idl_bool
CallDescTable::doneUpCallDesc(const char* sig)
{
  return (pd_doneUpCall[sig] ? 1 : 0);
}

void
CallDescTable::setDoneProxyCallDesc(const char* sig)
{
  pd_doneProxy.insert(sig,"done");
}

void
CallDescTable::setDoneUpCallDesc(const char* sig)
{
  pd_doneUpCall.insert(sig,"done");
}

static CallDescTable callDescTable;

//////////////////////////////////////////////////////////////////////
/////////////////////////// o2be_call_desc ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
o2be_call_desc::produce_descriptor(std::fstream& s, o2be_operation& op)
{
  const char* sig = op.mangled_signature();

  if( !callDescTable.doneProxyCallDesc(sig) ) {
    (void) callDescTable.insert(sig, op._idname());
    callDescTable.setDoneProxyCallDesc(sig);
    op.produce_proxy_call_desc(s, descriptor_name(op));
  }
}


void
o2be_call_desc::produce_descriptor(std::fstream& s, o2be_attribute& attr)
{
  const char* sig = attr.mangled_read_signature();
  if( !callDescTable.doneProxyCallDesc(sig) ) {
    (void) callDescTable.insert(sig, attr._idname());
    callDescTable.setDoneProxyCallDesc(sig);
    attr.produce_read_proxy_call_desc(s, read_descriptor_name(attr));
  }

  if (attr.readonly()) return;

  sig = attr.mangled_write_signature();
  if( !callDescTable.doneProxyCallDesc(sig) ) {
    (void) callDescTable.insert(sig, attr._idname());
    callDescTable.setDoneProxyCallDesc(sig);
    attr.produce_write_proxy_call_desc(s, write_descriptor_name(attr));
  }
}


const char*
o2be_call_desc::descriptor_name(o2be_operation& op)
{
  const char* sig = op.mangled_signature();

  const char* prefix;
  if (strcmp(sig,"void") == 0)
    prefix = STD_PROXY_CALL_DESC_PREFIX;
  else if (strcmp(sig,"_wvoid") == 0)
    prefix = STD_PROXY_CALL_DESC_PREFIX;
  else
    prefix = PROXY_CALL_DESC_PREFIX;

  StringBuf class_name;
  class_name += prefix;
  class_name += callDescTable[sig];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


const char*
o2be_call_desc::read_descriptor_name(o2be_attribute& attr)
{
  StringBuf class_name;
  class_name += PROXY_CALL_DESC_PREFIX;
  class_name += callDescTable[attr.mangled_read_signature()];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


const char*
o2be_call_desc::write_descriptor_name(o2be_attribute& attr)
{
  StringBuf class_name;
  class_name += PROXY_CALL_DESC_PREFIX;
  class_name += callDescTable[attr.mangled_write_signature()];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


char*
o2be_call_desc::generate_unique_name(const char* prefix)//DH
{
  return callDescTable.generate_unique_name(prefix);
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// o2be_call_desc ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
o2be_upcall_desc::produce_descriptor(std::fstream& s, o2be_operation& op)
{
  const char* sig = op.mangled_signature();

  if( !callDescTable.doneUpCallDesc(sig) ) {
    if (!callDescTable[sig]) (void) callDescTable.insert(sig, op._idname());
    callDescTable.setDoneUpCallDesc(sig);
    op.produce_server_call_desc(s, descriptor_name(op));
  }
}


void
o2be_upcall_desc::produce_descriptor(std::fstream& s, o2be_attribute& attr)
{
  const char* sig = attr.mangled_read_signature();
  if( !callDescTable.doneUpCallDesc(sig) ) {
    if (!callDescTable[sig]) (void) callDescTable.insert(sig, attr._idname());
    callDescTable.setDoneUpCallDesc(sig);
    attr.produce_server_rd_call_desc(s, read_descriptor_name(attr));
  }

  if (attr.readonly()) return;

  sig = attr.mangled_write_signature();
  if( !callDescTable.doneUpCallDesc(sig) ) {
    if (!callDescTable[sig]) (void) callDescTable.insert(sig, attr._idname());
    callDescTable.setDoneUpCallDesc(sig);
    attr.produce_server_wr_call_desc(s, write_descriptor_name(attr));
  }
}


const char*
o2be_upcall_desc::descriptor_name(o2be_operation& op)
{
  StringBuf class_name;

  const char* sig = op.mangled_signature();

  const char* prefix;
  if (strcmp(sig,"void") == 0)
    prefix = STD_UP_CALL_DESC_PREFIX;
  else if (strcmp(sig,"_wvoid") == 0)
    prefix = STD_UP_CALL_DESC_PREFIX;
  else
    prefix = UP_CALL_DESC_PREFIX;

  class_name += prefix;
  class_name += callDescTable[op.mangled_signature()];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


const char*
o2be_upcall_desc::read_descriptor_name(o2be_attribute& attr)
{
  StringBuf class_name;
  class_name += UP_CALL_DESC_PREFIX;
  class_name += callDescTable[attr.mangled_read_signature()];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


const char*
o2be_upcall_desc::write_descriptor_name(o2be_attribute& attr)
{
  StringBuf class_name;
  class_name += UP_CALL_DESC_PREFIX;
  class_name += callDescTable[attr.mangled_write_signature()];
  char* result = new char[strlen(class_name)+1];
  strcpy(result,class_name);
  return result;
}


