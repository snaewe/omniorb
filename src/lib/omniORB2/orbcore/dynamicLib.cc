// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.cc              Created on: 15/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    Stubs for dynamic library 'hook' functions.
//

/*
  $Log$
  Revision 1.2.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.4  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.3  1999/10/14 16:22:08  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/27 11:01:11  djr
  Modifications to logging.

  Revision 1.1.2.1  1999/09/22 14:26:48  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB3/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <dynamicLib.h>
#include <exceptiondefs.h>


static void init();
static void deinit();
static size_t context_aligned_size(size_t initoffset, CORBA::Context_ptr cxtx,
				   const char*const* which, int how_many);
static void marshal_context(NetBufferedStream&, CORBA::Context_ptr cxtx,
			    const char*const* which, int how_many);
static void lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt);


static omniDynamicLib orbcore_ops = {
  init,
  deinit,
  context_aligned_size,
  marshal_context,
  lookup_id_lcfn
};

omniDynamicLib* omniDynamicLib::ops = &orbcore_ops;
omniDynamicLib* omniDynamicLib::hook = 0;


static void
init()
{
  omniORB::logs(2, "The omniDynamic library is not linked.");
}


static void
deinit()
{
}


static size_t
context_aligned_size(size_t initoffset, CORBA::Context_ptr cxtx,
		     const char*const* which, int how_many)
{
  omniORB::logs(1, "Attempt to marshal context, but omniDynamic library"
		" is not linked!");
  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
  return 0;
}


static void
marshal_context(NetBufferedStream&, CORBA::Context_ptr cxtx,
		const char*const* which, int how_many)
{
  omniORB::logs(1, "Attempt to marshal context, but omniDynamic library"
		" is not linked!");
  OMNIORB_THROW(NO_IMPLEMENT,0, CORBA::COMPLETED_NO);
}


static void
lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt)
{
  // Can't have a local call to a repository if the dynamic
  // library is not linked ...
  OMNIORB_ASSERT(0);
}
