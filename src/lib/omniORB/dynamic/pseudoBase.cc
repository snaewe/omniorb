// -*- Mode: C++; -*-
//                            Package   : omniORB
// pseudoBase.cc              Created on: 9/1998
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
//   PseudoObjBase provides reference counting for the pseudo object
//   types.
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <pseudo.h>


//////////////////////////////////////////////////////////////////////
//////////////////////////// PseudoObjBase ///////////////////////////
//////////////////////////////////////////////////////////////////////

PseudoObjBase::~PseudoObjBase() {}


void
PseudoObjBase::decrRefCount()
{
  if( !pd_refCount ) {
    if( omniORB::traceLevel > 0 ) {
      omniORB::log <<
	"omniORB: WARNING -- CORBA::release() has been called too many times\n"
	" for a pseudo object.  The object has already been destroyed.\n";
      omniORB::log.flush();
    }
    return;
  }

  if( --pd_refCount == 0 )
    delete this;
}
