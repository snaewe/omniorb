// -*- Mode: C++; -*-
//                            Package   : omniORB
// codeSets.cc                Created on: 18/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
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
//    Base implementation of code set conversion functions
//

/*
  $Log$
  Revision 1.1.2.1  2000/10/27 15:42:07  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#include <omniORB4/CORBA.h>
#include <codeSetUtil.h>
#include <initialiser.h>

//
// Code set id constants
//

const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_1  = 0x00010001;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_2  = 0x00010002;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_3  = 0x00010003;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_4  = 0x00010004;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_5  = 0x00010005;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_6  = 0x00010006;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_7  = 0x00010007;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_8  = 0x00010008;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_9  = 0x00010009;
const CONV_FRAME::CodeSetId omniCodeSet::ID_8859_10 = 0x0001000a;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UCS_2   = 0x00010100;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UCS_4   = 0x00010104;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UTF_8   = 0x05010001;
const CONV_FRAME::CodeSetId omniCodeSet::ID_UTF_16  = 0x00010109;


//
// GIOP version constants
//

const GIOP::Version omniCodeSetUtil::GIOP10 = { 1,0 };
const GIOP::Version omniCodeSetUtil::GIOP11 = { 1,1 };
const GIOP::Version omniCodeSetUtil::GIOP12 = { 1,2 };


//
// Linked lists of code set objects
//

static omniCodeSet::NCS_C* ncs_c_head = 0;
static omniCodeSet::NCS_W* ncs_w_head = 0;
static omniCodeSet::TCS_C* tcs_c_head = 0;
static omniCodeSet::TCS_W* tcs_w_head = 0;

static inline _CORBA_Boolean
versionMatch(GIOP::Version v1, GIOP::Version v2)
{
  return (v1.major == v2.major && v1.minor == v2.minor);
}

omniCodeSet::NCS_C*
omniCodeSet::getNCS_C(CONV_FRAME::CodeSetId id)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_c_head; cs; cs = cs->pd_next) {
    if (cs->id() == id)
      return (NCS_C*)cs;
  }
  return 0;
}

omniCodeSet::NCS_W*
omniCodeSet::getNCS_W(CONV_FRAME::CodeSetId id)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_w_head; cs; cs = cs->pd_next) {
    if (cs->id() == id)
      return (NCS_W*)cs;
  }
  return 0;
}

omniCodeSet::TCS_C*
omniCodeSet::getTCS_C(CONV_FRAME::CodeSetId id, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_C* tcs;
  for (cs = tcs_c_head; cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_C*)cs;
    if (tcs->id() == id && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::TCS_W*
omniCodeSet::getTCS_W(CONV_FRAME::CodeSetId id, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_W* tcs;
  for (cs = tcs_w_head; cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_W*)cs;
    if (tcs->id() == id && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::NCS_C*
omniCodeSet::getNCS_C(const char* name)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_c_head; cs; cs = cs->pd_next) {
    if (!strcmp(cs->name(), name))
      return (NCS_C*)cs;
  }
  return 0;
}

omniCodeSet::NCS_W*
omniCodeSet::getNCS_W(const char* name)
{
  omniCodeSet::Base* cs;
  for (cs = ncs_w_head; cs; cs = cs->pd_next) {
    if (!strcmp(cs->name(), name))
      return (NCS_W*)cs;
  }
  return 0;
}

omniCodeSet::TCS_C*
omniCodeSet::getTCS_C(const char* name, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_C* tcs;
  for (cs = tcs_c_head; cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_C*)cs;
    if (!strcmp(cs->name(), name) && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}

omniCodeSet::TCS_W*
omniCodeSet::getTCS_W(const char* name, GIOP::Version v)
{
  omniCodeSet::Base*  cs;
  omniCodeSet::TCS_W* tcs;
  for (cs = tcs_w_head; cs; cs = cs->pd_next) {
    tcs = (omniCodeSet::TCS_W*)cs;
    if (!strcmp(cs->name(), name) && versionMatch(tcs->giopVersion(), v))
      return tcs;
  }
  return 0;
}



void
omniCodeSet::registerNCS_C(omniCodeSet::NCS_C* cs)
{
  cs->pd_next = ncs_c_head;
  ncs_c_head  = cs;
}

void
omniCodeSet::registerNCS_W(omniCodeSet::NCS_W* cs)
{
  cs->pd_next = ncs_w_head;
  ncs_w_head  = cs;
}

void
omniCodeSet::registerTCS_C(omniCodeSet::TCS_C* cs)
{
  cs->pd_next = tcs_c_head;
  tcs_c_head  = cs;
}

void
omniCodeSet::registerTCS_W(omniCodeSet::TCS_W* cs)
{
  cs->pd_next = tcs_w_head;
  tcs_w_head  = cs;
}

//
// Code set base constructors
//

omniCodeSet::NCS_C::NCS_C(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind)

  : Base(id, name, kind)
{
}

omniCodeSet::TCS_C::TCS_C(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind,
			  GIOP::Version giopVersion)

  : Base(id, name, kind), pd_giopVersion(giopVersion)
{
}

omniCodeSet::NCS_W::NCS_W(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind)

  : Base(id, name, kind)
{
}

omniCodeSet::TCS_W::TCS_W(CONV_FRAME::CodeSetId id,
			  const char* name, Kind kind,
			  GIOP::Version giopVersion)

  : Base(id, name, kind), pd_giopVersion(giopVersion)
{
}

//
// Default implementations of fast marshalling functions just return false
//

_CORBA_Boolean
omniCodeSet::TCS_C::fastMarshalChar(cdrStream&    stream,
				    NCS_C*        ncs,
				    _CORBA_Char   c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastMarshalString(cdrStream&    stream,
				      NCS_C*        ncs,
				      _CORBA_ULong  bound,
				      const char*   s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastUnmarshalChar(cdrStream&    stream,
				      NCS_C*        ncs,
				      _CORBA_Char&  c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_C::fastUnmarshalString(cdrStream&    stream,
					NCS_C*        ncs,
					_CORBA_ULong  bound,
					_CORBA_ULong& length,
					char*&        s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastMarshalWChar(cdrStream&    stream,
				     NCS_W*        ncs,
				     _CORBA_WChar  c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastMarshalWString(cdrStream&          stream,
				       NCS_W*              ncs,
				       _CORBA_ULong        bound,
				       const _CORBA_WChar* s)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastUnmarshalWChar(cdrStream&    stream,
				       NCS_W*        ncs,
				       _CORBA_WChar& c)
{
  return 0;
}

_CORBA_Boolean
omniCodeSet::TCS_W::fastUnmarshalWString(cdrStream&     stream,
					 NCS_W*         ncs,
					 _CORBA_ULong   bound,
					 _CORBA_ULong&  length,
					 _CORBA_WChar*& s)
{
  return 0;
}


//
// Module initialiser
//

void
omniCodeSet::logCodeSets()
{
  omniCodeSet::Base* cs;

  {
    omniORB::logger    l;
    l << "Native char code sets:";
    for (cs = ncs_c_head; cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
  {
    omniORB::logger    l;
    l << "Transmission char code sets:";
    for (cs = tcs_c_head; cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
  {
    omniORB::logger    l;
    l << "Native wide char code sets:";
    for (cs = ncs_w_head; cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
  {
    omniORB::logger    l;
    l << "Transmission wide char code sets:";
    for (cs = tcs_w_head; cs; cs = cs->pd_next) l << " " << cs->name();
    l << ".\n";
  }
}


class omni_codeSet_initialiser : public omniInitialiser {
public:
  void attach() {
    if (omniORB::trace(15)) {
      omniCodeSet::logCodeSets();
    }
  }
  void detach() {}
};


static omni_codeSet_initialiser initialiser;

omniInitialiser& omni_codeSet_initialiser_ = initialiser;
