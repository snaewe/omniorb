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
 Revision 1.2.2.7  2001/08/03 17:41:17  sll
 System exception minor code overhaul. When a system exeception is raised,
 a meaning minor code is provided.

 Revision 1.2.2.6  2001/06/07 16:24:09  dpg1
 PortableServer::Current support.

 Revision 1.2.2.5  2001/04/18 18:18:11  sll
 Big checkin with the brand new internal APIs.

 Revision 1.2.2.4  2000/11/03 18:46:19  sll
 Moved string marshal functions into cdrStream.

 Revision 1.2.2.3  2000/10/06 16:36:51  sll
 Removed inline definition of the marshal method in the client and server
 marshallers.

 Revision 1.2.2.2  2000/09/27 17:50:28  sll
 Updated to use the new cdrStream abstraction.
 Added extra members for use in the upcalls on the server side.

 Revision 1.2.2.1  2000/07/17 10:35:50  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

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

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/minorCode.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>
#include <exceptiondefs.h>
#include <dynamicLib.h>
#include <omniCurrent.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
///////////////////////// omniCallDescriptor /////////////////////////
//////////////////////////////////////////////////////////////////////

void
omniCallDescriptor::initialiseCall(cdrStream&)
{
  // no-op
}

void
omniCallDescriptor::marshalArguments(cdrStream&)
{
  // no-op
}


void
omniCallDescriptor::unmarshalReturnedValues(cdrStream&)
{
  // no-op
}


void
omniCallDescriptor::userException(IOP_C& iop_c, const char* repoId)
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

  iop_c.RequestCompleted(1);
  OMNIORB_THROW(UNKNOWN,UNKNOWN_UserException,
		(CORBA::CompletionStatus)iop_c.getStream().completion());
}

void
omniCallDescriptor::unmarshalArguments(cdrStream&)
{
  // no-op
}


void
omniCallDescriptor::marshalReturnedValues(cdrStream&)
{
  // no-op
}



//////////////////////////////////////////////////////////////////////
///////////// omniStdCallDesc::_cCORBA_mObject_i_cstring /////////////
//////////////////////////////////////////////////////////////////////

// NB. Copied from generated code.

void omniStdCallDesc::_cCORBA_mObject_i_cstring::marshalArguments(cdrStream& s)
{
  s.marshalString(arg_0);
}


void omniStdCallDesc::_cCORBA_mObject_i_cstring::unmarshalReturnedValues(cdrStream& s)
{
  pd_result = CORBA::Object::_unmarshalObjRef(s);
}


void omniStdCallDesc::_cCORBA_mObject_i_cstring::unmarshalArguments(cdrStream& s)
{
  arg_0 = s.unmarshalString();
}


void omniStdCallDesc::_cCORBA_mObject_i_cstring::marshalReturnedValues(cdrStream& s)
{
  CORBA::Object::_marshalObjRef(pd_result,s);
}

//////////////////////////////////////////////////////////////////////
///////////////////// omniLocalOnlyCallDescriptor ////////////////////
//////////////////////////////////////////////////////////////////////

void omniLocalOnlyCallDescriptor::marshalArguments(cdrStream& s)
{
  OMNIORB_THROW(INV_OBJREF,INV_OBJREF_TryToInvokePseudoRemotely,
		(CORBA::CompletionStatus)s.completion());
}

