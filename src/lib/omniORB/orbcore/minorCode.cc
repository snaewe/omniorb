// -*- Mode: C++; -*-
//                            Package   : omniORB
// minorCode.cc               Created on: 2 Aug 2001
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
  Revision 1.1.2.5  2004/12/20 18:09:48  dgrisby
  Remove entry for obsolete WRONG_TRANSACTION exception.

  Revision 1.1.2.4  2004/05/05 21:04:01  dgrisby
  Change COMMA define to avoid clashes with other systems.

  Revision 1.1.2.3  2001/10/17 16:47:09  dpg1
  New minor codes

  Revision 1.1.2.2  2001/09/24 10:41:10  dpg1
  Minor codes for Dynamic library and omniORBpy.

  Revision 1.1.2.1  2001/08/03 17:49:33  sll
  New minor code and lookup table for translating a minor code to its memonic
  names.

*/

#include <omniORB4/CORBA.h>
#include <omniORB4/minorCode.h>

OMNI_NAMESPACE_BEGIN(omni)

#define DeclareLookupEntry(name, value) { name, #name }

minorCodeLookup UNKNOWN_LookupTable[] = {
  DECLARE_UNKNOWN_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup BAD_PARAM_LookupTable[] = {
  DECLARE_BAD_PARAM_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup NO_MEMORY_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup IMP_LIMIT_LookupTable[] = {
  DECLARE_IMP_LIMIT_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup COMM_FAILURE_LookupTable[] = {
  DECLARE_COMM_FAILURE_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup INV_OBJREF_LookupTable[] = {
  DECLARE_INV_OBJREF_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup NO_PERMISSION_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INTERNAL_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup MARSHAL_LookupTable[] = {
  DECLARE_MARSHAL_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup INITIALIZE_LookupTable[] = {
  DECLARE_INITIALIZE_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 }   
};

minorCodeLookup NO_IMPLEMENT_LookupTable[] = {
  DECLARE_NO_IMPLEMENT_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup BAD_TYPECODE_LookupTable[] = {
  DECLARE_BAD_TYPECODE_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 }
};

minorCodeLookup BAD_OPERATION_LookupTable[] = { 
  DECLARE_BAD_OPERATION_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup NO_RESOURCES_LookupTable[] = { 
  DECLARE_NO_RESOURCES_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup NO_RESPONSE_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup PERSIST_STORE_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup BAD_INV_ORDER_LookupTable[] = {
  DECLARE_BAD_INV_ORDER_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup TRANSIENT_LookupTable[] = {
  DECLARE_TRANSIENT_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};


minorCodeLookup FREE_MEM_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INV_IDENT_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INV_FLAG_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INTF_REPOS_LookupTable[] = { 
  DECLARE_INTF_REPOS_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup BAD_CONTEXT_LookupTable[] = { 
  DECLARE_BAD_CONTEXT_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup OBJ_ADAPTER_LookupTable[] = {
  DECLARE_OBJ_ADAPTER_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup DATA_CONVERSION_LookupTable[] = {
  DECLARE_DATA_CONVERSION_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup OBJECT_NOT_EXIST_LookupTable[] = {
  DECLARE_OBJECT_NOT_EXIST_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};


minorCodeLookup TRANSACTION_REQUIRED_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup TRANSACTION_ROLLEDBACK_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INVALID_TRANSACTION_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup INV_POLICY_LookupTable[] = { 
  DECLARE_INV_POLICY_minors(DeclareLookupEntry,OMNI_COMMA),
  { 0, 0 } 
};

minorCodeLookup CODESET_INCOMPATIBLE_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup REBIND_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup TIMEOUT_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup TRANSACTION_UNAVAILABLE_LookupTable[] = { 
  { 0, 0 } 
};

minorCodeLookup TRANSACTION_MODE_LookupTable[] = { 	
  { 0, 0 } 
};

minorCodeLookup BAD_QOS_LookupTable[] = { 
  { 0, 0 } 
};


const char*
minorCode2String(const minorCodeLookup table[], CORBA::ULong code) {
  int i = 0;

  while (table[i].value != code) {
    if (table[i].name) 
      i++;
    else
      break;
  }
  return table[i].name;
}

OMNI_NAMESPACE_END(omni)
