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

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <o2be_util.h>


#define CALL_DESC_PREFIX            "_0RL_cd_"
#define STD_PROXY_CALL_DESC_PREFIX  "omniStdCallDesc::"


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static unsigned long base_l;
static unsigned long base_h;
static int base_initialised = 0;
static unsigned long counter = 0;


static void initialise_base(const char* seed)
{
  base_initialised = 1;
  base_h = base_l = 0u;

  while( *seed ) {
    unsigned long tmp;
    tmp = (base_h & 0xfe000000) >> 25;
    base_h = (base_h << 7) ^ ((base_l & 0xfe000000) >> 25);
    base_l = (base_l << 7) ^ tmp;
    base_l ^= (unsigned long) *seed++;
  }
}


static char* generate_unique_name(const char* prefix)//DH
{
  static char chrmap[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
  };

  char* result = new char[1 + strlen(prefix) + 16 + 1 + 8];

  strcpy(result, prefix);
  char* s = result + strlen(result);

  for( unsigned i = 0 ; i < 8; i++ )
    *s++ = chrmap[(base_h >> i * 4) & 0xf];
  for( unsigned j = 0 ; j < 8; j++ )
    *s++ = chrmap[(base_l >> j * 4) & 0xf];

  *s++ = '_';
  for( unsigned k = 0 ; k < 8; k++ )
    *s++ = chrmap[(counter >> k * 4) & 0xf];
  *s = '\0';

  counter++;
  return result;
}


static SimpleStringMap callDescTable;
static int cdt_initialised = 0;


static void initialise_cdt()
{
  // These call descriptors already exist in the omniORB library.
  callDescTable.insert("void", STD_PROXY_CALL_DESC_PREFIX "void_call");
  callDescTable.insert("_cCORBA_mObject_i_cstring",
		       STD_PROXY_CALL_DESC_PREFIX "_cCORBA_mObject_i_cstring");

  cdt_initialised = 1;
}

//////////////////////////////////////////////////////////////////////
/////////////////////////// o2be_call_desc ///////////////////////////
//////////////////////////////////////////////////////////////////////

void
o2be_call_desc::produce_descriptor(std::fstream& s, o2be_operation& op)
{
  if( !base_initialised )  initialise_base(op._idname());
  if( !cdt_initialised )   initialise_cdt();

  const char* sig = op.mangled_signature();

  if( !callDescTable[sig] ) {
    const char* class_name = ::generate_unique_name(CALL_DESC_PREFIX);
    callDescTable.insert(sig, class_name);
    op.produce_call_desc(s, class_name);
  }
}


void
o2be_call_desc::produce_descriptor(std::fstream& s, o2be_attribute& attr)
{
  if( !base_initialised )  initialise_base(attr._idname());
  if( !cdt_initialised )   initialise_cdt();

  const char* sig = attr.mangled_read_signature();
  if( !callDescTable[sig] ) {
    const char* class_name = ::generate_unique_name(CALL_DESC_PREFIX);
    callDescTable.insert(sig, class_name);
    attr.produce_read_proxy_call_desc(s, class_name);
  }

  sig = attr.mangled_write_signature();
  if( !callDescTable[sig] ) {
    const char* class_name = ::generate_unique_name(CALL_DESC_PREFIX);
    callDescTable.insert(sig, class_name);
    attr.produce_write_proxy_call_desc(s, class_name);
  }
}


const char*
o2be_call_desc::descriptor_name(o2be_operation& op)
{
  return callDescTable[op.mangled_signature()];
}


const char*
o2be_call_desc::read_descriptor_name(o2be_attribute& attr)
{
  return callDescTable[attr.mangled_read_signature()];
}


const char*
o2be_call_desc::write_descriptor_name(o2be_attribute& attr)
{
  return callDescTable[attr.mangled_write_signature()];
}


char*
o2be_call_desc::generate_unique_name(const char* prefix)//DH
{
  if( !base_initialised )
    throw o2be_internal_error(__FILE__, __LINE__,
			      "generate_unique_name called before"
			      " initialisation");

  return ::generate_unique_name(prefix);
}
