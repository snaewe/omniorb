// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORB.cc                 Created on: 15/6/99
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
//

/*
  $Log$
  Revision 1.2.2.9  2001/08/17 17:12:40  sll
  Modularise ORB configuration parameters.

  Revision 1.2.2.8  2001/08/08 15:57:12  sll
  New options unixTransportDirectory & unixTransportPermission.

  Revision 1.2.2.7  2001/08/03 17:41:23  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.6  2001/08/01 10:08:22  dpg1
  Main thread policy.

  Revision 1.2.2.5  2001/07/31 16:10:38  sll
  Added GIOP BiDir support.

  Revision 1.2.2.4  2001/07/13 15:30:12  sll
  New configuration variables.

  Revision 1.2.2.3  2001/06/07 16:24:10  dpg1
  PortableServer::Current support.

  Revision 1.2.2.2  2000/09/27 17:57:05  sll
  Changed include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:56  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.5  2000/06/22 10:40:16  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.4  1999/10/14 16:22:13  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.3  1999/09/30 12:25:59  djr
  Minor changes.

  Revision 1.1.2.2  1999/09/24 17:11:13  djr
  New option -ORBtraceInvocations and omniORB::traceInvocations.

  Revision 1.1.2.1  1999/09/22 14:26:58  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>
#include <orbParameters.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)


//////////////////////////////////////////////////////////////////////
/////////////////////// omniORB::fatalException //////////////////////
//////////////////////////////////////////////////////////////////////

omniORB::fatalException::fatalException(const char* file, int line,
					const char* errmsg)
  : pd_file(file), pd_line(line), pd_errmsg(errmsg)
{
  if( orbParameters::abortOnInternalError )  abort();
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void
omniORB::setMainThread()
{
  omni_thread* self = omni_thread::self();
  if (!self)
    OMNIORB_THROW(INITIALIZE, INITIALIZE_NotOmniThread, CORBA::COMPLETED_NO);

  omni::mainThreadId = self->id();
}
