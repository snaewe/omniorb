// -*- Mode: C++; -*-
//                            Package   : omniORB
// callDescriptor.cc          Created on: 18/6/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Implementation of pre-defined call descriptors.
//

/*
 $Log$
 Revision 1.3  2000/07/13 15:25:59  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.4  2000/06/22 10:40:13  dpg1
 exception.h renamed to exceptiondefs.h to avoid name clash on some
 platforms.

 Revision 1.1.2.3  1999/10/14 16:22:05  djr
 Implemented logging when system exceptions are thrown.

 Revision 1.1.2.2  1999/10/04 17:08:31  djr
 Some more fixes/MSVC work-arounds.

 Revision 1.1.2.1  1999/09/22 14:26:43  djr
 Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB3/callDescriptor.h>
#include <exceptiondefs.h>

//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ULong
omniCallDescriptor::alignedSize(CORBA::ULong in)
{
  return in;
}


void
omniCallDescriptor::marshalArguments(GIOP_C&)
{
  // no-op
}


void
omniCallDescriptor::unmarshalReturnedValues(GIOP_C&)
{
  // no-op
}


void
omniCallDescriptor::userException(GIOP_C& giop_c, const char* repoId)
{
  // Server side returns a user-defined exception, but we seem
  // to think the operation has none.  The IDL used on each side
  // probably differs.

  if( omniORB::trace(1) ) {
    omniORB::logger l;
    l << "WARNING -- server returned user-defined exception for an\n"
      " operation which the client thinks has none declared.  Could the\n"
      " server and client have been compiled with different versions of\n"
      " the IDL?\n"
      "  Exception repository id: " << repoId << "\n";
    omniORB::log.flush();
  }

  giop_c.RequestCompleted(1);
  OMNIORB_THROW(MARSHAL,0, CORBA::COMPLETED_MAYBE);
}

//////////////////////////////////////////////////////////////////////
///////////// omniStdCallDesc::_cCORBA_mObject_i_cstring /////////////
//////////////////////////////////////////////////////////////////////

// NB. Copied from generated code.

CORBA::ULong omniStdCallDesc::_cCORBA_mObject_i_cstring::alignedSize(CORBA::ULong msgsize)
{
  msgsize = omni::align_to(msgsize, omni::ALIGN_4) + 4;
  msgsize += ((const char*) arg_0) ? strlen((const char*) arg_0) + 1 : 1;
  return msgsize;
}


void omniStdCallDesc::_cCORBA_mObject_i_cstring::marshalArguments(GIOP_C& giop_client)
{
  {
    CORBA::ULong _len = (((const char*) arg_0)? strlen((const char*) arg_0) + 1 : 1);
    _len >>= giop_client;
    if (_len > 1)
      giop_client.put_char_array((const CORBA::Char *)((const char*) arg_0),_len);
    else {
      if ((const char*) arg_0 == 0 && omniORB::traceLevel > 1)
        _CORBA_null_string_ptr(0);
      CORBA::Char('\0') >>= giop_client;
    }
  }
}


void omniStdCallDesc::_cCORBA_mObject_i_cstring::unmarshalReturnedValues(GIOP_C& giop_client)
{
  pd_result = CORBA::Object_Helper::unmarshalObjRef(giop_client);
}

//////////////////////////////////////////////////////////////////////
///////////////////// omniLocalOnlyCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

CORBA::ULong
omniLocalOnlyCallDescriptor::alignedSize(CORBA::ULong)
{
  OMNIORB_THROW(INV_OBJREF,0, CORBA::COMPLETED_NO);
  return 0;
}
