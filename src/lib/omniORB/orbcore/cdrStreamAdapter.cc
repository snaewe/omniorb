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
// Revision 1.1.4.1  2003/03/23 21:02:24  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.1.2.4  2001/10/17 16:33:28  dpg1
// New downcast mechanism for cdrStreams.
//
// Revision 1.1.2.3  2001/06/13 20:12:32  sll
// Minor updates to make the ORB compiles with MSVC++.
//
// Revision 1.1.2.2  2001/05/10 15:03:50  dpg1
// Update cdrStreamAdapter to modified cdrStream interface.
//
// Revision 1.1.2.1  2001/01/09 17:17:00  dpg1
// New cdrStreamAdapter class to allow omniORBpy to intercept buffer
// management.
//

#include <omniORB4/CORBA.h>
#include <omniORB4/cdrStream.h>


OMNI_NAMESPACE_BEGIN(omni)

  class StreamAdapterStateCopier {
  public:
    inline StreamAdapterStateCopier(cdrStreamAdapter* s)
      : pd_s(s)
    {
      s->copyStateToActual();
    }
    inline ~StreamAdapterStateCopier()
    {
      pd_s->copyStateFromActual();
    }
  private:
    cdrStreamAdapter* pd_s;
  };

OMNI_NAMESPACE_END(omni)

OMNI_USING_NAMESPACE(omni)


void
cdrStreamAdapter::put_octet_array(const _CORBA_Octet* b, int size,
				  omni::alignment_t align)
{
  StreamAdapterStateCopier _c(this);
  pd_actual.put_octet_array(b, size, align);
}

void
cdrStreamAdapter::get_octet_array(_CORBA_Octet* b,int size,
				  omni::alignment_t align)
{
  StreamAdapterStateCopier _c(this);
  pd_actual.get_octet_array(b, size, align);
}

void
cdrStreamAdapter::skipInput(_CORBA_ULong size)
{
  StreamAdapterStateCopier _c(this);
  pd_actual.skipInput(size);
}

_CORBA_Boolean
cdrStreamAdapter::checkInputOverrun(_CORBA_ULong itemSize,
				    _CORBA_ULong nItems,
				    omni::alignment_t align)
{
  StreamAdapterStateCopier _c(this);
  return pd_actual.checkInputOverrun(itemSize, nItems, align);
}

_CORBA_Boolean
cdrStreamAdapter::checkOutputOverrun(_CORBA_ULong itemSize,
				     _CORBA_ULong nItems,
				     omni::alignment_t align)
{
  StreamAdapterStateCopier _c(this);
  return pd_actual.checkOutputOverrun(itemSize, nItems, align);
}

void
cdrStreamAdapter::copy_to(cdrStream& stream, int size, omni::alignment_t align)
{
  StreamAdapterStateCopier _c(this);
  pd_actual.copy_to(stream, size, align);
}

void
cdrStreamAdapter::fetchInputData(omni::alignment_t align,size_t required)
{
  StreamAdapterStateCopier _c(this);
  pd_actual.fetchInputData(align, required);
}

_CORBA_Boolean
cdrStreamAdapter::
reserveOutputSpaceForPrimitiveType(omni::alignment_t align, size_t required)
{
  StreamAdapterStateCopier _c(this);
  return pd_actual.reserveOutputSpaceForPrimitiveType(align, required);
}

_CORBA_Boolean
cdrStreamAdapter::
maybeReserveOutputSpace(omni::alignment_t align, size_t required)
{
  StreamAdapterStateCopier _c(this);
  return pd_actual.maybeReserveOutputSpace(align, required);
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

_CORBA_ULong
cdrStreamAdapter::completion()
{
  StreamAdapterStateCopier _c(this);
  return pd_actual.completion();
}

void*
cdrStreamAdapter::ptrToClass(int* cptr)
{
  if (cptr == &cdrStreamAdapter::_classid) return (cdrStreamAdapter*)this;
  if (cptr == &cdrStream       ::_classid) return (cdrStream*)       this;
  return 0;
}

int cdrStreamAdapter::_classid;
