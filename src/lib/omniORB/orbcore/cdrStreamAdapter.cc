// -*- Mode: C++; -*-
//                            Package   : omniORB
// cdrStreamAdapter.cc        Created on: 2001/01/09
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambrige
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
//	*** PROPRIETARY INTERFACE ***
//	

// $Log$
// Revision 1.1.2.1  2001/01/09 17:17:00  dpg1
// New cdrStreamAdapter class to allow omniORBpy to intercept buffer
// management.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/cdrStream.h>

void
cdrStreamAdapter::put_octet_array(const _CORBA_Octet* b, int size,
				  omni::alignment_t align)
{
  copyStateToActual();
  pd_actual.put_octet_array(b, size, align);
  copyStateFromActual();
}

void
cdrStreamAdapter::get_octet_array(_CORBA_Octet* b,int size,
				  omni::alignment_t align)
{
  copyStateToActual();
  pd_actual.get_octet_array(b, size, align);
  copyStateFromActual();
}

void
cdrStreamAdapter::skipInput(_CORBA_ULong size)
{
  copyStateToActual();
  pd_actual.skipInput(size);
  copyStateFromActual();
}

_CORBA_Boolean
cdrStreamAdapter::checkInputOverrun(_CORBA_ULong itemSize, 
				    _CORBA_ULong nItems,
				    omni::alignment_t align)
{
  copyStateToActual();
  return pd_actual.checkInputOverrun(itemSize, nItems, align);
}

_CORBA_Boolean
cdrStreamAdapter::checkOutputOverrun(_CORBA_ULong itemSize,
				     _CORBA_ULong nItems,
				     omni::alignment_t align)
{
  copyStateToActual();
  return pd_actual.checkOutputOverrun(itemSize, nItems, align);
}

void
cdrStreamAdapter::fetchInputData(omni::alignment_t align,size_t required)
{
  copyStateToActual();
  pd_actual.fetchInputData(align, required);
  copyStateFromActual();
}

size_t
cdrStreamAdapter::maxFetchInputData(omni::alignment_t align) const
{
  copyStateToActual();
  return pd_actual.maxFetchInputData(align);
}

_CORBA_Boolean
cdrStreamAdapter::reserveOutputSpace(omni::alignment_t align, size_t required)
{
  copyStateToActual();
  _CORBA_Boolean ret = pd_actual.reserveOutputSpace(align, required);
  copyStateFromActual();
  return ret;
}

size_t
cdrStreamAdapter::maxReserveOutputSpace(omni::alignment_t align) const
{
  copyStateToActual();
  return pd_actual.maxReserveOutputSpace(align);
}

_CORBA_ULong
cdrStreamAdapter::currentInputPtr() const
{
  copyStateToActual();
  return pd_actual.currentInputPtr();
}

_CORBA_ULong
cdrStreamAdapter::currentOutputPtr() const
{
  copyStateToActual();
  return pd_actual.currentOutputPtr();
}
